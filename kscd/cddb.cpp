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
#include <qregexp.h> 
#include "cddb.h"
#include <sys/time.h>
#include <kapp.h>
#include "version.h"

#include <sys/utsname.h>

#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

extern KApplication 	*mykapp;

void cddb_decode(QString& str);
void cddb_encode(QString& str, QStrList &returnlist);
bool cddb_playlist_decode(QStrList& playlist,QString&  value);
void cddb_playlist_encode(QStrList& playlist,QString&  value);
extern bool debugflag;

CDDB::CDDB(char *host, int _port,int _timeout)
{
    hostname  = host;
    port      = _port;
    connected = false;
    readonly  = false;
    timeout   = _timeout;

    QObject::connect(&starttimer,SIGNAL(timeout()),this,SLOT(cddb_connect_internal()));


    QObject::connect(&timeouttimer,SIGNAL(timeout()),this,SLOT(cddb_timed_out_slot()));

    sock = 0L;
    state = INIT;

    use_http_proxy=false;

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
    char proto[1024];
    char extra[1024];
  
    sscanf(_server.data(),"%s %s %s %s",ser,proto,por,extra);
  
    hostname  = ser;
    port      = atoi(por);
    cgi       = extra;

    protocol=decodeTransport(proto);

    starttimer.start(100,TRUE);
    mode = SERVERLISTGET;
    if(debugflag) printf("GETTING SERVERLIST\n");

}

void CDDB::cddb_connect(QString& _server){

    char ser[1024];
    char por[1024];
    char proto[1024];
    char extra[1024];
  
    sscanf(_server.data(),"%s %s %s %s",ser,proto,por,extra);
  
    hostname  = ser;
    port      = atoi(por);
    cgi       = extra;

    protocol=decodeTransport(proto);
  
    if(protocol==CDDBP)
    {
	starttimer.start(100,TRUE);
	mode = REGULAR;
    } 
    // Do nothing otherwise. We do not connect immediately for HTTP
    // We rather connect per request.
}

void CDDB::cddb_connect_internal(){

    starttimer.stop();
    timeouttimer.start(timeout*1000,TRUE);

    if(sock){
	delete sock;
	sock = 0L;
    }

    // signal( SIGALRM , CDDB::sighandler );
    // setalarm();

    //if(debugflag) printf("PROTOCOL %d\n",(int)protocol);
    if(protocol==CDDBHTTP && use_http_proxy)
    {
	if(debugflag) printf("CONNECTING TO %s:%d ....\n",proxyhost.data(),proxyport);
	sock = new KSocket(proxyhost.data(),proxyport);
    }
    else
    {
	if(debugflag) printf("CONNECTING TO %s:%d ....\n",hostname.data(),port);
	sock = new KSocket(hostname.data(),port);
    }
   
    //signal( SIGALRM , SIG_DFL );

    if( sock == 0L || sock->socket() < 0){

	timeouttimer.stop();

	if(debugflag) printf("CONNECT FAILED\n");

	if( mode == REGULAR )
	    emit cddb_failed();      
	else // mode == SERVERLISTGET
	    emit get_server_list_failed();

	return;    

    }

    connected = true;
    respbuffer = "";

    connect(sock,SIGNAL(readEvent(KSocket*)),this,SLOT(cddb_read(KSocket*)));
    connect(sock,SIGNAL(closeEvent(KSocket*)),this,SLOT(cddb_close(KSocket*)));
    sock->enableRead(true);

    if(protocol==CDDBHTTP)
    {
	struct utsname uts;
	uname(&uts);
      
	QString domainname;
	domainname = uts.nodename;
	domainname.detach();
      
	if(domainname.isEmpty())
	    domainname = "somemachine.nowhere.org";
      
	pw = getpwuid(getuid());
	QString username;
	if (pw)
	    username = pw->pw_name;
	else
	    username = "anonymous";
      
	QString prt;
	prt.setNum(port);
	QString base  = "http://"+hostname+":"+prt;
	QString param = cgi+"?cmd=sites&hello="+username+"+"+domainname+"+Kscd+"+KSCDVERSION+"&proto=3";
	QString request;
	if(use_http_proxy)
	    request="GET "+base+param+" HTTP/1.0\r\n\r\n";
	else
	    request="GET "+param+"\r\n";

	if(debugflag) fprintf(stderr,"Sending HTTP request: %s",request.data());
      
	write(sock->socket(),request.data(),request.length());
	if(use_http_proxy)
	{
	    saved_state=SERVER_LIST_WAIT;
	    state=HTTP_REQUEST;
	} else
	    state = SERVER_LIST_WAIT;
    } else
	state = INIT;
  
    if (debugflag) printf("CONNECTED\n");
}

void CDDB::cddb_timed_out_slot(){

    timeouttimer.stop();

    sock->enableRead(false);

    if( mode == REGULAR )
	emit cddb_timed_out();
    else // mode == SERVERLISTGET
	emit get_server_list_failed();

    state = CDDB_TIMEDOUT;
    if (debugflag) printf("SOCKET CONNECTION TIMED OUT\n");
    cddb_close(sock);
}

int CDDB::getState(){

    return state;
}

// called externally if we want to close or interrupt the cddb connection
void CDDB::close_connection(){
 
    if(sock)
	cddb_close(sock);
    sock = 0L;

}
void CDDB::cddb_close(KSocket *socket){

    timeouttimer.stop();
    disconnect(socket,SIGNAL(readEvent(KSocket*)),this,SLOT(cddb_read(KSocket*)));
    disconnect(socket,SIGNAL(closeEvent(KSocket*)),this,SLOT(cddb_close(KSocket*)));
    socket->enableRead(false);
    if (debugflag) printf("SOCKET CONNECTION TERMINATED\n");
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
	if (debugflag) printf("WARNING CDDB LASTLINE EMPTY\n");

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

    timeouttimer.stop();
    timeouttimer.start(timeout*1000,TRUE);
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
	if (debugflag) printf("sock = 0L!!!\n");
	return;
    }

    timeouttimer.stop();
    timeouttimer.start(timeout*1000,TRUE);

    write(sock->socket(),readstring.data(),readstring.length());
    //printf("WROTE=%s\n",readstring.data());

    state = CDDB_READ;
    respbuffer = "";
    sock->enableRead(true);

}

void CDDB::do_state_machine()
{
    switch (state)
    {
    case HTTP_REQUEST:
	//Parse responce and check numeric code.
	char proto[1024];
	char code[1024];
	sscanf(respbuffer.data(),"%s %s",proto,code);
	if(strcmp(code,"200")==0)
	{
	    state=saved_state;
	    if(debugflag)
		fprintf(stderr,"HTTP request is OK. Moving on.\n");
	}
	else {
	    if(debugflag)
		fprintf(stderr,"HTTP error: %s\n",lastline.data());
	    if(saved_state==SERVER_LIST_WAIT)
		emit get_server_list_failed();
          
	    state=CDDB_DONE; //TODO: some error state
	}
	break;
      
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
	    if (debugflag) printf("LOCAL NODE: %s\n",uts.nodename);

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
	    if(debugflag) printf("ERROR_INIT\n");
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
	    if(debugflag) printf("ERROR_HELLO\n");
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
		timeouttimer.stop();
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
	    if(debugflag) printf("ERROR_QUERY\n");
	    emit cddb_failed();
	    respbuffer = "";

	}

	break;

    case INEX_READ:

	if(lastline.left(1) == QString(".")){

	    state = CDDB_DONE;
	    respbuffer.detach();
	    timeouttimer.stop();
	    emit cddb_inexact_read();

	}
	break;

    case CDDB_READ:

	if(lastline.left(1) == QString(".")){


	    respbuffer.detach();
	    // Let's strip the first line and the trainling \r.\n\r
	    int nl = respbuffer.find("\n",0,true);
	    respbuffer = respbuffer.mid(nl+1,respbuffer.length()- nl -4);

	    QString readstring;
	    readstring.sprintf("quit \n");

	    write(sock->socket(),readstring.data(),readstring.length());

	    state = CDDB_DONE;

	    emit cddb_done();
      
	}    
	if(lastline.left(1) == "4"){

	    state = ERROR_CDDB_READ;
	    if(debugflag) printf("ERROR_CDDB_READ\n");
	    cddb_close(sock);
	    emit cddb_failed();
	}

	break;

    case SERVER_LIST_WAIT:
	if(lastline.left(1) == QString("."))
	{
	    respbuffer.detach();
	    if(protocol==CDDBHTTP && use_http_proxy)
	    {
		// Let's strip HTTP header
		int hl = respbuffer.find("\r\n\r\n",0,true);
		respbuffer = respbuffer.right(respbuffer.length()-hl-4);
	    }
	    // Let's strip the first line and the trainling \r.\n\r
	    int nl = respbuffer.find("\n",0,true);
	    respbuffer = respbuffer.mid(nl+1,respbuffer.length()- nl -4);
	    parse_serverlist();
	    if(debugflag) printf("GOT SERVERLIST\n");
	    write(sock->socket(),"quit\n",6);
	    cddb_close(sock);
	    emit get_server_list_done();
	    state = CDDB_DONE;
	}

	break;

    default:
	break;

    }

}


void CDDB::serverList(QStrList& list){

    list = serverlist;
}

void CDDB::parse_serverlist(){

  
    int  pos1,pos2;
    char serv  [1024];
    char po    [1024];
    char proto [1024];
    char extra [1024];
  
    serverlist.clear();
    // get rid of the first line
    respbuffer = respbuffer.replace( QRegExp("\r"), "" );

    pos1 = respbuffer.find("\n",0,true);
    respbuffer = respbuffer.mid(pos1 + 1,respbuffer.length());

    pos1 = 0;

    while((pos2 = respbuffer.find("\n",pos1  ,true)) != -1)
    {
	if(pos2-pos1 > 0)
	{
	    QString tempstr1;
	    QString tempstr2;
	    tempstr1 = respbuffer.mid(pos1 ,pos2-pos1  );
	    if(protocol==CDDBP)
	    {
		// for direct connections assuming CDDB protocol level 1
		sscanf(tempstr1.data(),"%s %s",serv,po);
		tempstr2 = tempstr2.sprintf("%s cddbp %s -",serv,po);
		serverlist.append(tempstr2.data());
	    } else
	    {
		// for http connections assuming CDDB protocol level 3
		sscanf(tempstr1.data(),"%s %s %s %s",serv,proto,po,extra);
		tempstr2 = tempstr2.sprintf("%s %s %s %s",serv,proto,po,extra);
		transport tr=decodeTransport(proto);
		if(tr==CDDBP || tr==CDDBHTTP)
		    serverlist.append(tempstr2.data());
		// ignore otherwsie - unksuported protocol.
	    }
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

	if (debugflag) printf("Checking %s\n",pathlist.at(i));

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
	if (debugflag) printf("REVISION %d\n",revision);
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
	    if (debugflag) printf("ANOMALY 1\n");
	}

	if (debugflag) printf("DISCDID %s\n",discidtemp.data());

	pos1 = 0;
	while((pos2 = discidtemp.find(",",pos1,true)) != -1){


	    if( pos2 - pos1 >= 0){
		temp3 = discidtemp.mid(pos1,pos2-pos1);
	    }
	    else{
		if (debugflag) printf("ANOMALY 2\n");
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

    if (debugflag) printf("FOUND %d DISCID's\n",discidlist.count());

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
	    if (debugflag) printf("GET VALUE ANOMALY 1\n");
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

// This function converts server list entry from "Server Port" format
// To "Server Protocol Port Address".
//     The fields are as follows:
//         site:
//             The Internet address of the remote site.
//         protocol:
//             The transfer protocol used to access the site.
//         port:
//             The port at which the server resides on that site.
//         address:
//             Any additional addressing information needed to access the
//             server. For example, for HTTP protocol servers, this would be
//             the path to the CDDB server CGI script. This field will be
//             "-" if no additional addressing information is needed.
//
// Returns 'true' if format have been converted.
bool CDDB::normalize_server_list_entry(QString &entry)
{
    char serv [1024];
    char proto[1024];
    char po   [1024];
    char extra[1024];
    
    if(sscanf(entry.data(),"%s %s %s %s",serv,proto,po,extra)==2) 
    {
	// old format
	sprintf(extra,"%s cddbp %s -",serv, proto);
	entry=extra;
	return true;
    } else
    {
	// Otherwise let us leave the item unchanged.
	return false;
    }
}

void CDDB::setHTTPProxy(QString host, int port)
{
    proxyhost=host;
    proxyport=port;
}

void CDDB::useHTTPProxy(bool flag)
{
    use_http_proxy=flag;
}

bool    CDDB::useHTTPProxy()
{
    return use_http_proxy;
}

int     CDDB::getHTTPProxyPort()
{
    return proxyport;
}

QString CDDB::getHTTPProxyHost()
{
    return proxyhost;
}

CDDB::transport CDDB::decodeTransport(const char *proto)
{
    if(strcasecmp(proto,"cddbp")==0)
        return CDDBP;
    else
        if(strcasecmp(proto,"http")==0)
            return CDDBHTTP;
        else
            if(strcasecmp(proto,"smtp")==0)
                return SMTP;
            else
                return UNKNOWN;
}

#include "cddb.moc"





