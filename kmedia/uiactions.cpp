/*
   Copyright (c) 1997 Christian Esken (chris@magicon.prima.ruhr.de)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 	kmedia.cpp : the GUI part of the media master
		     The protocol resides in libmediatool.a
*/

#include <unistd.h>
#include "kmedia.h"

extern KApplication	*globalKapp;


void KMediaWin::quit()
{
  exit(0);
}

void KMediaWin::quitLW()
{
  quit();
}

void KMediaWin::quitClicked()
{

      /*
      bool exitOK = removePlayer();
      if (!exitOK)
	fprintf(stderr,"Player does not quit!!!\n");
	*/

      timer->stop();
      //delete this;

      quit(); 
      /* Quit should be done by the "lastWindowClosed SIGNAL handler. But
       * it does not make it through KApplication:-(
       */
}


bool KMediaWin::removePlayer()
{
  if (!MaudioLaunched)
    return true;



  /* Press the exit "button". This tells the slave to quit */
  EventCounterRaise(&(KeysChunk->exit),1);
  /* Await end of player */
  for (int i=0; i<40; i++)
    {
      if (*StatStatPtr == MD_STAT_EXITED)
	break;
      else
	{
	  usleep(100*1000);
	  //QApplication::processEvents();
	}
    }
  if (*StatStatPtr == MD_STAT_EXITED)
    {
      MaudioLaunched = false;
      return true;
    }
  else
    return false;
}


void KMediaWin::ejectClicked()
{

}

void KMediaWin::trackSelected( int trk )
{
  fprintf(stderr, "Track selected: %i\n",trk);
}




void KMediaWin::onDrop( KDNDDropZone* _zone )
{
  QStrList strlist;
  KURL *url;
  char *str;

  fprintf(stderr, "URLs dropped on KMedia!\n");
  strlist = _zone->getURLList();

  url = new KURL( strlist.first() );
  fprintf(stderr, "%s\n",url->path());
  FileNameSet( FnamChunk, url->path());

  launchPlayer(url->path());

  delete url;
  return;

  /* commented out !!! */
  fprintf(stderr, "URLs dropped on KMedia:\n");

  if ( strlist.count()==1 )
    {
      // Open first URL
      url = new KURL( strlist.first() );
      FileNameSet( FnamChunk, url->path());
      delete url;
      fprintf(stderr,"Set Filename completed.\n");
    }
  else
    {
      for( str=strlist.first(); str!=0; str=strlist.next() )
	{
	  fprintf(stderr, "%s\n", str);
	  PlaylistAdd(Playlist, str, -1);
	}
    }

  printf("Playlist is now:\n");
  PlaylistShow(Playlist);
}


void KMediaWin::aboutClicked()
{
  QString msg;
  char TalkIdStr[10];

  sprintf(TalkIdStr,"%i", m.talkid);

  msg = \
    "Kmedia and the mediatool protocol are written\n" \
    "and (C)1997 by Christian Esken. This program is\n"\
    "in the GPL. Please send comments and bug reports\n"\
    "to chris@magicon.prima.ruhr.de\n" \
    "The talk id is: ";
  msg += TalkIdStr;

  KMsgBox::message(0, "About Kmedia 0.39",\
			msg,
			KMsgBox::INFORMATION, "OK" );
}




void KMediaWin::TimerFunctions()
{
  if (TimerAction == NOP)
    {
      /* If nothing is to be done: Reset everything to default values */
      InitialDelay = -1;
      return;
    }

  if ( InitialDelay < 0 )
    {
      /* Here we come only, when TimerAction!=NOP, and last time it was NOP */
      InitialDelay =  4;
      RepeatSpeed  =  1;
      return;
    }
  else if ( InitialDelay > 0 )
    {
      /* Here we come, when we are in "delay" mode. */
      InitialDelay --;
      return;
    }

  RepeatSpeed--;
  if ( RepeatSpeed >0 )
    return;

  /* Default repeat speed */
  RepeatSpeed = 1;

  switch (TimerAction)
    {
    case FF:
      EventCounterRaise(&(KeysChunk->forward),1);
      break;
    case REW:
      EventCounterRaise(&(KeysChunk->backward),1);
      break;
    case NEXTTRACK:
      EventCounterRaise(&(KeysChunk->nexttrack),1);
      RepeatSpeed  =  1;
      break;
    case PREVTRACK:
      EventCounterRaise(&(KeysChunk->prevtrack),1);
      RepeatSpeed  =  1;
      break;
    default:
      break;
    }

}




void KMediaWin::PosChanged( int new_pos )
{
  fprintf(stderr,"New Position is %i.\n", new_pos);
}


void KMediaWin::newviewClicked()
{
  KMediaWin *newkmedia;
  newkmedia = new KMediaWin(NULL,"Window 2");
  newkmedia->show();
}




void KMediaWin::launchPlayer(char *filename)
{
  
  char* Opts[10];
  char MaudioText[]  ="maudio";
  char MmidiText[]   ="mplaymidi";
  char MtrackerText[]="mtracker";
  char MediaText[]   ="-media";
  char MediaText2[]  ="-m";
  char TempString[256];

  enum {myAudio,myMidi,myTracker,myOther};
  int  ftype=myAudio;

  if (MaudioLaunched)
    {
      // already launched? then wait for termination of old player first
      bool exitOK = removePlayer();
      if (!exitOK)
	fprintf(stderr,"Player does not quit!!!\n");
      else
	MaudioLaunched = false;
    }


  // Do a stupid file type recognition
  int fnlen = strlen(filename);
  char *filename_start;
  filename_start = strrchr(filename,'/');
  if (filename_start==NULL)
    filename_start = filename;
  else
    filename_start ++;

  ftype = myOther;
  if ( fnlen > 4)
   {
      if (! strcasecmp(filename+fnlen-4, ".mid") )
	ftype=myMidi;
      else if ( (strcasecmp(filename+fnlen-4, ".mod")==0) || (strcasecmp(filename+fnlen-7, ".mod.gz")==0) || (strncasecmp(filename_start, "mod.",4)==0))
	ftype=myTracker;
      else
	ftype=myAudio;
    }
  pid_t myPid = fork();

  if ( myPid == 0 )
    { /* Child process */
      sprintf(TempString, "%i", IhdrChunk->ref);
      switch (ftype)
	{
	case myMidi:	Opts[0]=MmidiText   ; Opts[1]=MediaText2; break;
	case myTracker:	Opts[0]=MtrackerText; Opts[1]=MediaText ; break;
	case myAudio:	Opts[0]=MaudioText  ; Opts[1]=MediaText ; break;
	default:	KMsgBox::message(0, "Kmedia", "Unknown media type", KMsgBox::INFORMATION, "OK" );
	}
      Opts[2]=TempString;
      Opts[3]=NULL;
      execvp( Opts[0], /*(char *const )*/ Opts);

      fprintf(stderr,"Could not start maudio!\n");
      exit( 1 );
    }
  else
    {
      if (myPid>0)
	/* Process that forked the child */
	MaudioLaunched=true;
      else
	{
	  fprintf(stderr,"Could not fork!\n");
	  MaudioLaunched=false;
	}
    }    

}



void KMediaWin::launchHelp()
{
  globalKapp->invokeHTMLHelp("kmedia/kmedia.html", "");
}

void KMediaWin::launchMixer()
{
  if( prefDL->mixerCommand.isEmpty() )
    {
      KMsgBox::message(0, "Error",\
		       "Please set mixer in preferences first.",
		       KMsgBox::INFORMATION, "OK" );
    }
  else
    {
      if ( fork() == 0 )
	{
	  execlp( prefDL->mixerCommand, prefDL->mixerCommand, 0 );
	  fprintf(stderr,"Error: Starting mixer failed.\n");
	  exit( 1 );
	}
    }
}

void KMediaWin::hideMenu()
{
  static bool x=0;
  x=!x;

  /*   if (mainmenu->isVisible()) */
  if (x)
    {
      /* mainmenu->hide(); */
      printf("only container\n");
    }
  else
    {
      /* mainmenu->show(); */
      printf("both\n");
    }
}

