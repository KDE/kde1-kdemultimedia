/*
 *
 * kscd -- A simple CD player for the KDE project           
 *
 * $Id$
 * 
 * Copyright (C) 1997 Bernd Johannes Wuebben 
 * wuebben@math.cornell.edu
 *
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <qtstream.h> 
#include <qfile.h>
#include <qdir.h>
#include <qfileinf.h> 
#include "cddb.h"
#include <sys/time.h>
#include <kapp.h>
#include "version.h"

#include <sys/utsname.h>

extern KApplication 	*mykapp;

void cddb_decode(QString& str);
void cddb_encode(QString& str, QStrList &returnlist);
bool cddb_playlist_decode(QStrList& playlist,QString&  value);
void cddb_playlist_encode(QStrList& playlist,QString&  value);

CDDB::CDDB(char *host, int _port,int _timeout){


  hostname  = host;
  port      = _port;
  connected = false;
  readonly  = false;
  timeout   = _timeout;

  QObject::connect(&starttimer,SIGNAL(timeout()),this,SLOT(cddb_connect_internal()));


  QObject::connect(&timeouttimer,SIGNAL(timeout()),this,SLOT(cddb_timed_out_slot()));

  sock = 0L;
  state = INIT;

}

CDDB::~CDDB(){

  if(sock){
    delete sock;
    sock = 0L;
  }

  timeouttimer.stop();
  starttimer.stop();

}

void CDDB::sighandler(int signum){

  signum = signum;
  /*
  if (signum == SIGALRM && connecting == true ){
    mykapp->processEvents();
    mykapp->flushX();
    signal( SIGALRM , CDDB::sighandler );
    setalarm();
    printf("SIGALRM\n");
  }
  */

}

void CDDB::setalarm(){

  struct itimerval  val1;
  struct timeval  tval1;
  struct timeval  tval2;

  tval1.tv_sec = 0;
  tval1.tv_usec = 100000; // 0.1 secs
  tval2.tv_sec = 0;
  tval2.tv_usec = 100000;

  val1.it_interval = tval1;
  val1.it_value    = tval2;

  setitimer(ITIMER_REAL, &val1, 0);
}

void CDDB::cddbgetServerList(QString& _server){

  char ser[1024];
  char por[1024];
  
  sscanf(_server.data(),"%s %s",ser,por);
  
  hostname  = ser;
  port      = atoi(por);

  starttimer.start(100,TRUE);
  mode = SERVERLISTGET;

}

void CDDB::cddb_connect(QString& _server){

  char ser[1024];
  char por[1024];
  
  sscanf(_server.data(),"%s %s",ser,por);
  
  hostname  = ser;
  port      = atoi(por);

  starttimer.start(100,TRUE);
  mode = REGULAR;

}

void CDDB::cddb_connect_internal(){

  starttimer.stop();
  timeouttimer.start(timeout*1000,TRUE);

  printf("CONNECTING ....\n");

   if(sock){
     delete sock;
     sock = 0L;
   }

   // signal( SIGALRM , CDDB::sighandler );
   // setalarm();

  sock = new KSocket(hostname.data(),port);

  //signal( SIGALRM , SIG_DFL );

  if( sock == 0L || sock->socket() < 0){

    timeouttimer.stop();
    emit cddb_failed();
    return;    

  }

  connected = true;
  respbuffer = "";
  state = INIT;

  connect(sock,SIGNAL(readEvent(KSocket*)),this,SLOT(cddb_read(KSocket*)));
  connect(sock,SIGNAL(closeEvent(KSocket*)),this,SLOT(cddb_close(KSocket*)));
  sock->enableRead(true);

  printf("CONNECTED\n");


}

void CDDB::cddb_timed_out_slot(){

  timeouttimer.stop();
  state = CDDB_TIMEDOUT;
  sock->enableRead(false);
  emit cddb_timed_out();
  printf("SOCKET CONNECTION TIMED OUT\n");
  cddb_close(sock);
}

int CDDB::getState(){

  return state;
}

void CDDB::cddb_close(KSocket *socket){

  timeouttimer.stop();
  disconnect(socket,SIGNAL(readEvent(KSocket*)),this,SLOT(cddb_read(KSocket*)));
  disconnect(socket,SIGNAL(closeEvent(KSocket*)),this,SLOT(cddb_close(KSocket*)));
  socket->enableRead(false);
  printf("SOCKET CONNECTION TERMINATED\n");
  connected = false;
  if(socket){
    delete socket;
    socket = 0L;
    sock = 0L;
  }
}

void CDDB::cddb_read(KSocket *socket){

  int n;
  char buffer[2024];
  QString myreturn;

  if(socket == 0L || socket->socket() < 0)
    return;

  memset(buffer,0,2024);
  n = read( socket->socket(), buffer, 2023 );
  buffer[n] = '\0';
  tempbuffer += buffer;

  //  printf("BUFFER: %s",buffer);

  // let's only add responses one line at a time.
  int newlinepos = tempbuffer.findRev('\n',-1,true);

  if(newlinepos != -1){
    respbuffer += tempbuffer.left(newlinepos +1 );
    tempbuffer  = tempbuffer.right(tempbuffer.length() - newlinepos -1);
    isolate_lastline();
  }
}



void CDDB::isolate_lastline(){

  int pos;
  pos = respbuffer.findRev('\n',respbuffer.length()-2,true);

  if(pos == -1)
    lastline = respbuffer.copy();
  else
    lastline = respbuffer.mid(pos+1,respbuffer.length());

  if(!lastline.isEmpty())
    do_state_machine();
  else
    printf("WARNING CDDB LASTLINE EMPTY\n");

};

void  CDDB::queryCD(unsigned long _magicID,QStrList& querylist){

  if(sock == 0L || sock->socket() < 0 )
    return;

  QString str;
  title = "";
  category = "";
  magicID = _magicID;

  str = str.sprintf("cddb query %08x %d ",magicID,querylist.count()-1);
  for(int i = 0; i <(int) querylist.count(); i++){
    str += querylist.at(i);
    str += " ";
  }
  str += "\n";

  write(sock->socket(),str.data(),str.length());
  //  printf("WROTE:%s\n",str.data());
  state  = QUERY;
  
}

void CDDB::query_exact(QString line){

      int category_start = 0;
      int category_end 	 = 0;
      int magic_start 	 = 0;
      int magic_end 	 = 0;

      QString magicstr;
      
      category_start = line.find(" ",0,true) + 1;
      category_end = line.find(" ",category_start,true);
      category = line.mid(category_start,category_end-category_start);

      magic_start = category_end + 1;
      magic_end = line.find(" ",magic_start,true);
      magicstr = line.mid( magic_start, magic_end - magic_start);

      title = line.mid(magic_end + 1,line.length());

      QString readstring;
      //      readstring.sprintf("cddb read %s %lx \n",category.data(),magicID);
      readstring.sprintf("cddb read %s %s \n",category.data(),magicstr.data());
      if(sock == 0L || sock ->socket() < 0){
	printf("sock = 0L!!!\n");
	return;
      }

      write(sock->socket(),readstring.data(),readstring.length());
      //printf("WROTE=%s\n",readstring.data());

      state = CDDB_READ;
      respbuffer = "";
      sock->enableRead(true);

}

void CDDB::do_state_machine(){

  switch (state){

  case INIT:
    if((lastline.left(3) == QString("201")) ||(lastline.left(3) == QString("200")) ){

      QString hellostr;
      QString username;

      if(lastline.left(3) == QString("201"))
	readonly = true;
      else
	readonly = false;

      struct utsname uts;
      uname(&uts);
      printf("LOCAL NODE: %s\n",uts.nodename);

      QString domainname;
      domainname = uts.nodename;
      domainname.detach();

      if(domainname.isEmpty())
	domainname = "somemachine.nowhere.org";

      pw = getpwuid(getuid());

      if (pw)
	username = pw->pw_name;
      else
	username = "anonymous";

      // cddb hello username hostname clientname version
      hellostr.sprintf("cddb hello %s %s Kscd ",username.data(),
		        domainname.data());
      hellostr += KSCDVERSION;

      hellostr += " \n";

      write(sock->socket(),hellostr.data(),hellostr.length());
      state = HELLO;

    }
    else{
      state = ERROR_INIT;	
      cddb_close(sock);
      emit cddb_failed();
    }

    respbuffer = "";
    break;

  case HELLO:

    if(lastline.left(3) == QString("200")){

      state = READY;
      if(mode == REGULAR)
	emit cddb_ready();
      else{
	write(sock->socket(),"sites\n",6);
	state = SERVER_LIST_WAIT;
      }

    }
    else{
      state = ERROR_HELLO;
      cddb_close(sock);
      emit cddb_failed();
    }

    respbuffer = "";
    break;

  case QUERY:

    if(respbuffer.left(3) == QString("200")){
      
      query_exact(lastline);
      respbuffer = "";
    }
    else if(respbuffer.left(3) == QString("211")){

      if(lastline.left(1) == QString(".")){

	state = CDDB_DONE;
	respbuffer.detach();
	emit cddb_inexact_read();

      }
      else{
	state = INEX_READ;

      }
    }
    else if(respbuffer.left(3) == QString("202")){

      state = CDDB_DONE;
      respbuffer.detach();
      respbuffer = "";
      cddb_close(sock);
      emit cddb_no_info();

    }
    else{

      state = ERROR_QUERY;
      cddb_close(sock);
      emit cddb_failed();
      respbuffer = "";

    }

    break;

  case INEX_READ:

    if(lastline.left(1) == QString(".")){

      state = CDDB_DONE;
      respbuffer.detach();
      emit cddb_inexact_read();

    }
    break;

  case CDDB_READ:

    if(lastline.left(1) == QString(".")){

      QString readstring;
      readstring.sprintf("quit \n",category.data(),magicID);

      write(sock->socket(),readstring.data(),readstring.length());

      state = CDDB_DONE;

      respbuffer.detach();
      // Let's strip the first line and the trainling \r.\n\r
      int nl = respbuffer.find("\n",0,true);
      respbuffer = respbuffer.mid(nl+1,respbuffer.length()- nl -4);
      emit cddb_done();
      
    }    
    if(lastline.left(1) == "4"){

      state = ERROR_CDDB_READ;
      cddb_close(sock);
      emit cddb_failed();
    }

    break;

  case SERVER_LIST_WAIT:
    if(lastline.left(1) == QString(".")){

      respbuffer.detach();
      // Let's strip the first line and the trainling \r.\n\r
      int nl = respbuffer.find("\n",0,true);
      respbuffer = respbuffer.mid(nl+1,respbuffer.length()- nl -4);
    }
    parse_serverlist();
    write(sock->socket(),"quit\n",6);
    emit get_server_list_done();
    state = CDDB_DONE;
    break;

  default:
    break;

  }

}


void CDDB::serverList(QStrList& list){

  list = serverlist;
}

void CDDB::parse_serverlist(){

  
  int pos1,pos2;
  char po[1024];
  char serv[1024];

  serverlist.clear();
  // get rid of the first line
  respbuffer = respbuffer.replace( QRegExp("\r"), "" );

  pos1 = respbuffer.find("\n",0,true);
  respbuffer = respbuffer.mid(pos1 + 1,respbuffer.length());

  pos1 = 0;

  while((pos2 = respbuffer.find("\n",pos1  ,true)) != -1){
    
    if(pos2-pos1  > 0){
      QString tempstr1;
      QString tempstr2;
      tempstr1 = respbuffer.mid(pos1 ,pos2-pos1  );
      sscanf(tempstr1.data(),"%s %s",serv,po);
      tempstr2 = tempstr2.sprintf("%s %s",serv,po);
      serverlist.append(tempstr2.data());
    }

    pos1 = pos2 + 1;
  }

}

void CDDB::get_inexact_list(QStrList& inexact_list){

  respbuffer = respbuffer.mid(0,respbuffer.length() - 3);
  respbuffer = respbuffer.replace( QRegExp("\r"), "" );

  int pos1,pos2;

  // get rid of the first line

  pos1 = respbuffer.find("\n",0,true);
  respbuffer = respbuffer.mid(pos1 + 1,respbuffer.length());

  pos1 = 0;

  while((pos2 = respbuffer.find("\n",pos1  ,true)) != -1){
    
    if(pos2-pos1  > 0){
      inexact_list.append(respbuffer.mid(pos1 ,pos2-pos1  ));
    }

    pos1 = pos2 + 1;
  }
}

bool CDDB::local_query(
		       unsigned  long magicID,
		       QString&  data,
		       QStrList& titlelist,
		       QStrList& extlist,
		       QString&  _category,
		       QStrList& discidlist,
		       int&	 revision,
		       QStrList& playlist
		       ){

  if(pathlist.count() == 0)
    return false;

  for(int i = 0 ; i <(int) pathlist.count(); i++){

    printf("Checking %s\n",pathlist.at(i));

    if(checkDir(magicID,pathlist.at(i))){
      getCategoryFromPathName(pathlist.at(i),category);
      category.detach();
      getData(data,titlelist,extlist,_category,discidlist,revision,playlist);
      return true;
    }
  }

  return false;

}

bool CDDB::checkDir(unsigned long magicID, char* dir){

  QString mag;
  mag.sprintf("%s/%08x",dir,magicID);

  QFileInfo info(mag.data());
  if(!info.isReadable())
    return false;

  respbuffer = "";

  QFile file(mag.data());

  if( !file.open( IO_ReadOnly )) {
    return false;
  }

  QTextStream t(&file);

  while ( !t.eof() ) {

    QString s = t.readLine() + "\n";
    if(!t.eof())
      respbuffer += s;
  }  


  file.close();
  respbuffer.detach();
  return true;

}


void CDDB::setPathList(QStrList& _paths){

  pathlist = _paths; // automatically makes deep copies is _paths has deep copies

}

// scan the relevant parts of the cddba database entry in the the provied structures
void CDDB::getData(
		   QString& data,
		   QStrList& titles, 
		   QStrList& extlist,
		   QString& categ,
		   QStrList& discidlist, 
		   int& revision,
		   QStrList& playlist
		   ){

  respbuffer = respbuffer.replace( QRegExp("\r"), "" );

  data = "";
  titles.clear();
  extlist.clear();
  discidlist.clear();
  categ      = category.copy();
  data 	     = respbuffer;
  data.detach();
  
  int pos1,pos2,pos3,pos4 = 0;

  int revtmp = data.find("Revision:",0,true);
  if(revtmp == -1)
    revision = 1;
  else{
    QString revstr;
    int revtmp2;
    revtmp2 = data.find("\n",revtmp,true);
    if(revtmp2 - revtmp - 9 >=0)
      revstr = data.mid(revtmp +9,revtmp2 - revtmp -9);
    revstr.stripWhiteSpace();
    bool ok;
    revision = revstr.toInt(&ok);
    if(!ok)
      revision = 1;
    printf("REVISION %d\n",revision);
  }
    
  // lets get all DISCID's in the data. Remeber there can be many DISCID's on
  // several lines separated by commas on each line
  //
  // DISCID= 47842934,4h48393,47839492
  // DISCID= 47fd2934,4h48343,47839492,43879074

  while((pos3 = data.find("DISCID=",pos4,true))!= -1){

    pos1 = pos3;
    pos2 = data.find("\n",pos1,true);

    QString discidtemp;
    QString temp3;

    if( pos2 - pos1 -7 >= 0){
      discidtemp = data.mid(pos1 + 7,pos2- pos1 -7);
    }
    else{
      printf("ANOMALY 1\n");
    }

    printf("DISCDID %s\n",discidtemp.data());

    pos1 = 0;
    while((pos2 = discidtemp.find(",",pos1,true)) != -1){


      if( pos2 - pos1 >= 0){
	temp3 = discidtemp.mid(pos1,pos2-pos1);
      }
      else{
	printf("ANOMALY 2\n");
      }

      temp3 = temp3.stripWhiteSpace();

      if(!temp3.isEmpty())
	discidlist.append(temp3.data());

      pos1 = pos2 + 1;
    }

    temp3 = discidtemp.mid(pos1,discidtemp.length());
    temp3.stripWhiteSpace();

    if(!temp3.isEmpty())
      discidlist.append(temp3.data());

    pos4 = pos3 + 1;

  }// end get DISCID's

  printf("FOUND %d DISCID's\n",discidlist.count());

  // Get the DTITLE

  QString value;
  QString key;
  key = "DTITLE=";

  getValue(key,value,data);
  titles.append(value);


  int counter = 0;
  key = key.sprintf("TTITLE%d=",counter);
  while(getValue(key,value,data)){
      titles.append(value);
      key = key.sprintf("TTITLE%d=",++counter);
  }

  key = "EXTD=";
  getValue(key,value,data);
  extlist.append(value);

  counter = 0;
  key = key.sprintf("EXTT%d=",counter);
  while(getValue(key,value,data)){
      extlist.append(value);
      key = key.sprintf("EXTT%d=",++counter);
  }
   
  key = "PLAYORDER=";
  getValue(key,value,data);
  cddb_playlist_decode(playlist, value);
  

}

void CDDB::getCategoryFromPathName(char* pathname, QString& category){
  
  QString path = pathname;
  path = path.stripWhiteSpace();

  while(path.right(1) == QString("/")){
    path = path.left(path.length() - 1);
  }

  int pos = 0;
  pos  = path.findRev("/",-1,true);
  if(pos == -1)
    category = path.copy();
  else
    category = path.mid(pos+1,path.length());

  category.detach();

}

bool CDDB::getValue(QString& key,QString& value, QString& data){

  bool found_one = false;
  int pos1 = 0;
  int pos2 = 0;

  value = "";

  while((  pos1 = data.find(key.data(),pos1,true)) != -1){
    found_one = true;
    pos2 = data.find("\n",pos1,true);
    if( (pos2 - pos1 - (int)key.length()) >= 0){
      value += data.mid(pos1 + key.length(), pos2 - pos1 - key.length());
    }
    else{
      printf("GET VALUE ANOMALY 1\n");
    }
    pos1 = pos1 + 1;
  }

  if(value.isNull())
    value = "";

  cddb_decode(value);
  return found_one;
}

void cddb_playlist_encode(QStrList& list,QString& playstr){

  playstr = "";
  for( uint i = 0; i < list.count(); i++){
    playstr += list.at(i);
    if(i < list.count() -1)
      playstr += ",";
  }
}


bool cddb_playlist_decode(QStrList& list, QString& str){
 
  bool isok = true;
  int pos1, pos2;
  pos1 = 0;
  pos2 = 0;

  list.clear();

  while((pos2 = str.find(",",pos1,true)) != -1){

    if(pos2 > pos1){
      list.append(str.mid(pos1,pos2 - pos1));
    }
    
    pos1 = pos2 + 1;
  }
  
  if(pos1 <(int) str.length())
    list.append(str.mid(pos1,str.length()));

  QString check;
  bool 	ok1;
  int   num;

  for(uint i = 0; i < list.count(); i++){
    check = list.at(i);
    check = check.stripWhiteSpace();

    if(check.isEmpty()){
      list.remove(i);
      i--;
      continue;
    }

    if(check == QString (",")){
      list.remove(i);
      i--;
      continue;
    }
    
    num = check.toInt(&ok1);
    if(!ok1 || num < 1){
      list.remove(i);
      i--;
      isok = false;
      continue;
    }
    
    list.remove(i);
    list.insert(i, check);

  }

  /*  for(uint i = 0; i < list.count(); i++){
    printf("playlist %d=%s\n",i,list.at(i));
  }*/
  return isok;
}

void cddb_decode(QString& str){

  int pos1 = 0;
  int pos2 = 0;

  while((pos2 = str.find("\\n",pos1,true)) !=-1  ){
    if(pos2>0){
      if(str.mid(pos2-1,3) == QString("\\\\n")){
	pos1 = pos2 + 2;
	continue;
      }
    }
    str.replace(pos2 , 2 , "\n");
    pos1 = pos2 + 1;
  }

  pos1 = 0;
  pos2 = 0;

  while((pos2 = str.find("\\t",pos1,true)) !=-1){
    if(pos2>0){
      if(str.mid(pos2-1,3) == QString("\\\\t")){
	pos1 = pos2 + 2;
	continue;
      }
    }
    str.replace(pos2 , 2 , "\t");
    pos1 = pos2 + 1;
  }

  pos1 = 0;
  pos2 = 0;

  while((pos2 = str.find("\\\\",pos1,true)) !=-1){
    str.replace(pos2 , 2 , "\\");
    pos1 = pos2 + 1;
  }


}

void cddb_encode(QString& str, QStrList &returnlist){

  returnlist.clear();

  int pos1 = 0;
  int pos2 = 0;

  while((pos2 = str.find("\\",pos1,true)) !=-1){
    str.replace(pos2 , 1 , "\\\\");
    pos1 = pos2 + 1;
  }

  pos1 = 0;
  pos2 = 0;

  while((pos2 = str.find("\n",pos1,true)) !=-1){
    str.replace(pos2 , 1 , "\\n");
    pos1 = pos2 + 1;
  }

  pos1 = 0;
  pos2 = 0;

  while((pos2 = str.find("\t",pos1,true)) !=-1){
    str.replace(pos2 , 1 , "\\t");
    pos1 = pos2 + 1;
  }

  while(str.length() > 70){
    returnlist.append(str.left(70));
    str = str.mid(70,str.length());
  }

  returnlist.append(str);

}
#include "cddb.moc"





