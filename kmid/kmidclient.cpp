/**************************************************************************

    kmidclient.cpp  - The main client widget of KMid
    Copyright (C) 1997  Antonio Larrosa Jimenez

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Send comments and bug fixes to antlarr@arrakis.es
    or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain

***************************************************************************/
#include "kmidclient.moc"
//#include "kmidframe.h"

#include <qkeycode.h>
#include <stdio.h>
#include <qfiledlg.h>
#include <kapp.h>
#include <qstring.h>
#include <unistd.h>
#include "player/midimapper.h"
#include "player/track.h"
#include "player/midispec.h"
#include <sys/soundcard.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <kmsgbox.h>
#include <sys/signal.h>
#include <qlcdnum.h>
#include <kurl.h>
#include <qlabel.h>

kmidClient::kmidClient(QWidget *parent,const char *name)
	:QWidget(parent,name)
{
	itsme=0;
        timebar = new KSlider(0,240000,30000,60000,KSlider::Horizontal,this);
	timebar->setSteps(30000,60000);
	timebar->setValue(0);
	timebar->setGeometry(5,10,width()-5,15);
/*	timebar->setRange(0,240000);
	timebar->setSteps(30000,60000);
*/
//	timebar->move(5,10);
//	timebar->show();
	connect (timebar,SIGNAL(valueChanged(int)),this,SLOT(timebarChange(int)));
//	timebar->setTracking(FALSE);

	timetags = new KSliderTime(timebar,this);
	timetags->setGeometry(5,10+timebar->height(),width()-5,16);

	qlabelTempo= new QLabel("Tempo :",this,"tempolabel",QLabel::NoFrame);
	qlabelTempo->move(5,10+timebar->height()+timetags->height()+5);
	qlabelTempo->adjustSize();

	tempoLCD = new QLCDNumber(3,this,"TempoLCD");
	tempoLCD->setMode(QLCDNumber::DEC);
	tempoLCD->display(120);
//	tempoLCD->move(5,10+timebar->height()+timetags->height()+10);		
	tempoLCD->setGeometry(5+qlabelTempo->width()+5,10+timebar->height()+timetags->height()+5,65,28);

	typeoftextevents=1;
	kdispt=new KDisplayText(this,"KaraokeWindow");
	kdispt->move(5,10+timebar->height()+timetags->height()+5+tempoLCD->height()+10);		


	timer4timebar=new QTimer(this);	
	connect (timer4timebar,SIGNAL(timeout()),this,SLOT(timebarUpdate()));
	timer4events=new QTimer(this);	
	connect (timer4events,SIGNAL(timeout()),this,SLOT(processSpecialEvent()));


	int sharedmemid=shmget(getpid(),sizeof(PlayerController),0666 | IPC_CREAT);

	pctl=(PlayerController *)shmat(sharedmemid,NULL,0);
	pctl->playing=0;
	pctl->gm=1;

	Midi = new midiOut();
	Player= new player(Midi,pctl);

	KApplication *kappl;
	kappl=KApplication::getKApplication();
	KConfig *kconf=kappl->getConfig();
	kconf->setGroup("Midimapper");
	QString qs=kconf->readEntry("Loadfile");
	
	printf("readConfig: %s\n",(const char *)qs);
	MidiMapper *Map=new MidiMapper((char*)(const char *)qs);
	Midi->useMapper(Map);

	if (kappl->argc()>1) 
	    {
	    openFile((kappl->argv())[1]);
	    song_Play();
	    };
};

void kmidClient::resizeEvent(QResizeEvent *) 
{
//timebar->resize(width()-5,timebar->height());
timebar->setGeometry(5,10,width()-5,15);
timetags->setGeometry(5,10+timebar->height(),width()-5,16);
kdispt->setGeometry(5,10+timebar->height()+timetags->height()+5+tempoLCD->height()+10,width()-10,height()-(10+timebar->height()+timetags->height()+5+tempoLCD->height()+10));
};

kmidClient::~kmidClient()
{
if (pctl->playing==1)
    {
    song_Stop();
//    sleep(1);
    }; 
kill(playerProcessID,SIGINT);
waitpid(playerProcessID, NULL, WUNTRACED);
kdispt->PreDestroyer();
delete kdispt;
delete timer4timebar;
delete timer4events;
delete tempoLCD;
delete Player;
delete Midi;

};
/*
void kmidClient::readConfig(KConfig *kconf)
{
QString qs;
qs="Midimapper";
kconf->setGroup(qs);
qs=kconf->readEntry("Loadfile");

printf("readConfig: %s\n",(char *)(const char *)qs);
MidiMapper *Map=new MidiMapper((char*)(const char *)qs);
Midi->useMapper(Map);
};
*/

void kmidClient::extractFilename(const char *in,char *out)
{
char *p=(char *)in;
char *filename=(char *)in;
while (*p!=0)
    {
    if (*p=='/') filename=p+1;
    p++;
    };
while (*filename!=0)
    {
    *out=*filename;
    out++;
    filename++;
    };
*out=0;
};

void kmidClient::file_Open()
{
printf("file Open\n");
char name[200];
name[0]=0;
QString filename;
filename=QFileDialog::getOpenFileName(0,"*.*",this,name); 
if (!filename.isNull()) 
	{
	openFile((char *)(const char *)filename);
	char *fn=new char[strlen(filename)+20];
        extractFilename((const char *)filename,fn);
	char *capt=new char[strlen(fn)+20];
	sprintf(capt,"KMid - %s",fn);
	topLevelWidget()->setCaption(capt);
	delete capt;
	};
};

void kmidClient::openFile(char *filename)
{
pctl->message|=PLAYER_HALT;
song_Stop();
Player->loadSong(filename);
#ifdef KMidDEBUG
printf("TOTAL TIME : %g milliseconds\n",Player->Info()->millisecsTotal);
#endif
timebar->setRange(0,(int)(Player->Info()->millisecsTotal));
timetags->repaint(TRUE);
kdispt->ClearEv();
spev=Player->takeSpecialEvents();
while (spev!=NULL)
    {
    if ((spev->type==1) || (spev->type==5)) 
	{
	kdispt->AddEv(spev);
	};
    spev=spev->next;
    };

kdispt->calculatePositions();
kdispt->CursorToHome();
kdispt->repaint(TRUE);
printf("file opened\n");
timebar->setValue(0);
};

void kmidClient::openURL(char *s)
{
KURL kurldropped(s);
if (kurldropped.isMalformed()) return;
if (strcmp(kurldropped.protocol(),"file")!=0) return;

char *filename=kurldropped.path();
QString qsfilename(filename);
KURL::decodeURL(qsfilename);
filename=(char *)(const char *)qsfilename;
if (filename!=NULL) 
	{
	openFile(filename);
	char *fn=new char[strlen(filename)+20];
        extractFilename(filename,fn);
	char *capt=new char[strlen(fn)+20];
	sprintf(capt,"KMid - %s",fn);
	topLevelWidget()->setCaption(capt);
	delete capt;
	};
};

void kmidClient::help_Help()
{
KApplication *kappp=KApplication::getKApplication();
kappp->invokeHTMLHelp("kmid.html",NULL);
};

void kmidClient::help_About()
{
printf("KMid 0.2 Copyright (C) 1997 Antonio Larrosa Jimenez. Malaga (Spain)\n");
printf("KMid comes with ABSOLUTELY NO WARRANTY; for details view file COPYING\n");
printf("This is free software, and you are welcome to redistribute it\n");
printf("under certain conditions");
KMsgBox::message( 0, "About KMid", "KMid 0.2\n(C) 1997 by Antonio Larrosa Jimenez\n(antlarr@arrakis.es)\nMalaga (Spain)", KMsgBox::INFORMATION, "Close" );
};


void kmidClient::song_Play()
{
#ifdef KMidDEBUG
printf("******************************+\n");
#endif
if (!Player->isSongLoaded())
    {
    KMsgBox::message(this,"Warning","You must load the file before playing it");
    return;
    };
if (pctl->playing==1)
    {
    KMsgBox::message(this,"Warning","A song is already being played");
    return;
    };
kdispt->CursorToHome();
pctl->message=0;
pctl->playing=0;
pctl->error=0;
//pctl->playing=1;
pctl->SPEVplayed=0;
pctl->SPEVprocessed=0;
#ifdef KMidDEBUG
passcount=0;
#endif
gettimeofday(&begintv, NULL);
beginmillisec=begintv.tv_sec*1000+begintv.tv_usec/1000;


if ((playerProcessID=fork())==0)
    {
#ifdef KMidDEBUG
    printf("PlayerProcessID : %d\n",getpid());
#endif
    Player->play(0,(void (*)(void))kmidOutput);
//    Player->play((void (*)(event *))kmidOutput);
//   I haven't been able to pass an event as a parameter
#ifdef KMidDEBUG
    printf("End of child process\n");
#endif
    _exit(0);
    };
pctl->millisecsPlayed=0;


spev=Player->takeSpecialEvents();
#ifdef KMidDEBUG
printf("writing SPEV\n");
Player->writeSPEV();
printf("writing SPEV(END)\n");
#endif

while ((pctl->playing==0)&&(pctl->error==0)) ;

if (pctl->error==1) return;

timer4timebar->start(1000);

//printf(" FIRSTFIRST Event in %ld milliseconds\n",spev->absmilliseconds);

if ((spev!=NULL)&&(spev->type!=0))
   timer4events->start(spev->absmilliseconds,TRUE);

#ifdef KMidDEBUG
printf("PlayerProcess : %d . ParentProcessID : %d\n",playerProcessID,getpid());
printf("******************************-\n");
#endif
};

void kmidClient::timebarUpdate()
{
itsme=1;
if (pctl->playing==0)
    timer4timebar->stop();
timeval tv;
gettimeofday(&tv, NULL);
ulong currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
pctl->millisecsPlayed=(currentmillisec-beginmillisec);

timebar->setValue((int)(pctl->millisecsPlayed));
itsme=0;
};

void kmidClient::timebarChange(int i)
{
if (itsme==1) return;
//if (timebar->draggingSlider()==TRUE) return;
if (pctl->playing==0) 
	{
	itsme=1;
	timebar->setValue(0);
	itsme=0;
	return;
	};
kill(playerProcessID,SIGINT);

printf("change Time : %d\n",i);
pctl->OK=0;
pctl->gotomsec=i;
pctl->message|=PLAYER_SETPOS;

if ((playerProcessID=fork())==0)
    {
    printf("PlayerProcessID : %d\n",getpid());
    
    Player->play(0,(void (*)(void))kmidOutput);
    
    printf("End of child process\n");
    _exit(0);
    };

timer4events->stop();
spev=Player->takeSpecialEvents();
//kdispt->CursorToHome();
while ((spev!=NULL)&&(spev->absmilliseconds<(ulong)i))
     {
//     if ((spev->type==1) || (spev->type==5)) kdispt->PaintIn();
     spev=spev->next;
     };
kdispt->gotomsec(i);
while (pctl->OK==0) ;
pctl->OK=0;
tempoLCD->display(tempoToMetronomeTempo(pctl->tempo));

timeval tv;
gettimeofday(&tv, NULL);
ulong currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
beginmillisec=currentmillisec-i;
if (spev==NULL) return;
ulong delaymillisec=spev->absmilliseconds-(currentmillisec-beginmillisec);
timer4events->start(delaymillisec,TRUE);

};


void kmidClient::song_Pause()
{
timeval tv;
if (pctl->playing==0) return;
printf("song Pause\n");
if (pctl->paused==0)
    {
    kill(playerProcessID,SIGINT);
    pausedatmillisec=(ulong)pctl->millisecsPlayed;
////    pctl->message|=PLAYER_DOPAUSE;
////    while (pctl->paused==0) ;
////    gettimeofday(&tv, NULL);
////    pausedatmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
    pctl->paused=1;
    timer4timebar->stop(); 
    timer4events->stop();
//    kill(playerProcessID,SIGSTOP);
//   The previous line doesn't work because it stops the two processes (!?)
    }
  else
    {
pctl->OK=0;
pctl->gotomsec=pausedatmillisec;
pctl->message|=PLAYER_SETPOS;

if ((playerProcessID=fork())==0)
    {
    printf("PlayerProcessID : %d\n",getpid());
    
    Player->play(0,(void (*)(void))kmidOutput);
    
    printf("End of child process\n");
    _exit(0);
    };
while (pctl->OK==0) ;
pctl->OK=0;

/////    pctl->OK=0;
    pctl->paused=0;
//    printf("proc : %d\n",playerProcessID);
//    int kf=kill(playerProcessID,SIGCONT);
//    printf("kf:%u",kf);
//////    while (pctl->OK==0) ;


    if ((spev!=NULL)&&(spev->type!=0))
        {
        gettimeofday(&tv, NULL);
        ulong currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
//        beginmillisec=beginmillisec+(currentmillisec-pausedatmillisec);
        beginmillisec=currentmillisec-pausedatmillisec;
        ulong delaymillisec=spev->absmilliseconds-(currentmillisec-beginmillisec);
        timer4events->start(delaymillisec,TRUE);
	};
    timer4timebar->start(1000);
    };
};


void kmidClient::song_Stop()
{
if (pctl->playing==0) return;
printf("song Stop\n");
kill(playerProcessID,SIGINT);
pctl->playing=0;
////////pctl->OK=0;
////////pctl->message|=PLAYER_HALT;
timer4timebar->stop();
timer4events->stop();
//pctl->playing=0;
//pctl->paused=0;
////////while (pctl->OK==0) ;
};

void kmidClient::song_Rewind()
{
timebar->subtractPage();
timebarChange(timebar->value());
};

void kmidClient::song_Forward()
{
timebar->addPage();
timebarChange(timebar->value());
};


void kmidClient::kmidOutput(void)
{
// Should do nothing
/*
Midi_event *ev=pctl->ev;

timeval tv;
gettimeofday(&tv, NULL);
ulong currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;

if ((ev->command==MIDI_SYSTEM_PREFIX)&&((ev->command|ev->chn)==META_EVENT))
    {
    if ((ev->d1==5)||(ev->d1==1))
	{
	char *text=new char[ev->length+1];
	strncpy(text,(char *)ev->data,ev->length);
	text[ev->length]=0;
	printf("%s , played at : %ld\n",text,currentmillisec-beginmillisec);
        }
       else if (ev->d1==ME_SET_TEMPO)
	    {
	    int tempo=(ev->data[0]<<16)|(ev->data[1]<<8)|(ev->data[2]);
//	    printf("Change tempo : %d , %g, played at :%ld\n",tempo,tempoToMetronomeTempo(tempo),currentmillisec-beginmillisec);
	    };

    }; 
*/
};


void kmidClient::processSpecialEvent()
{
if (spev==NULL)
   {
   printf("SPEV == NULL !!!!!\n");
   return;
   };
#ifdef KMidDEBUG
printf(":::: %ld",passcount++);

printf("%d %s %ld",spev->type,spev->text,spev->absmilliseconds);
#endif

int processNext=1;
long delaymillisec=~0;
while (processNext)
    { 
/*    timeval tv;
    gettimeofday(&tv, NULL);
    ulong currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
*/
    if ((spev->type==1) || (spev->type==5))
       {
//       printf("%s , showed at : %ld , %ld\n",spev->text,currentmillisec-beginmillisec,spev->absmilliseconds);
       kdispt->PaintIn();
       }
      else
       {
//       printf("Change Tempo : %d , showed at %ld , %ld\n",spev->tempo,currentmillisec-beginmillisec,spev->absmilliseconds);
	tempoLCD->display(tempoToMetronomeTempo(spev->tempo));
       };
    pctl->SPEVprocessed++;
    processNext=0;

    spev=spev->next;


    if ((spev!=NULL)&&(spev->type!=0))
        {
#ifdef KMidDEBUG
        timeval tv;
        gettimeofday(&tv, NULL);
        ulong currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
        printf("{{:%ld\n",currentmillisec-beginmillisec);
        printf("}}:%d\n",(tv.tv_sec-begintv.tv_sec)*1000+((tv.tv_usec-begintv.tv_usec)/1000));
        printf("():%ld\n",spev->absmilliseconds-(currentmillisec-beginmillisec));
#else
        timeval tv;
        ulong currentmillisec;
#endif
        gettimeofday(&tv, NULL);
        currentmillisec=tv.tv_sec*1000+tv.tv_usec/1000;
	delaymillisec=spev->absmilliseconds-(currentmillisec-beginmillisec);
        if (delaymillisec<10) processNext=1;

        }
       else
	delaymillisec=~0;
    };

    if (delaymillisec!=~(long)0) timer4events->start(delaymillisec,TRUE);

}; 

void kmidClient::repaintText(int type)
{
//kdispt->ChangeTypeOfTextEvents(type);
typeoftextevents=type;
kdispt->repaint(TRUE);
};

void kmidClient::songType(int i)
{
pctl->gm=i;
};


QFont * kmidClient::getFont(void)
{
return kdispt->getFont();
};

void  kmidClient::fontChanged(void)
{
kdispt->fontChanged();
};