/*
   Copyright (c) 1997-98 Christian Esken (esken@kde.org)

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
*/

// $Id$
// A GUI for the mediatool players. Serves as mediatool master

#include "kmedia.h"
#include "kmedia.moc"

#include <sys/types.h> 
#include <sys/wait.h> 


#include <klocale.h>

KApplication	*globalKapp;



/****************************************************************************
	      W A R N I N G : THE CODE IS (STILL) SOMEWHAT UGLY
         I guess this is due to the fact, that I cannot program C++.
                     But I am currently learning fast. :-)
 ****************************************************************************/

void sigchld_handler(int /*signal*/)
{
 waitpid(-1, NULL, WNOHANG);
}

int main ( int argc, char *argv[] )
{
  int i;
  struct sigaction act;

  globalKapp = new KApplication( argc, argv, "kmedia" );
  KMEDIA *kmedia;

  act.sa_handler = sigchld_handler;
  sigemptyset(&(act.sa_mask));
  sigaddset(&(act.sa_mask), SIGCHLD);                                       
  act.sa_flags = SA_NOCLDSTOP;
  sigaction( SIGCHLD, &act, NULL);

  // globalKIL = new KIconLoader();
  kmedia  = new KMEDIA;

  /* Create initial Playlist from command line */
  for ( i=1 ; i<argc; i++ ) {
    if ( argv[i][0] != '-' ) {
      /*
	PlaylistAdd(kmedia->Playlist, argv[i],-1);
      */
      KURL *url = new KURL( argv[i] );
      kmedia->kmw->launchPlayer(url->path()); // Will be launched only one time
      // OK, this cast is umh ... not-so-good. But it is the low-level
      // protocol, only kmedia and the media players speak lowlevel protocol.
      FileNameSet( kmedia->kmw->FnamChunk, (char*)url->path());
      delete url;
      break;
    }
  }
  globalKapp->connect( globalKapp, SIGNAL(lastWindowClosed()), kmedia->kmw, SLOT( quitAll()));

  return globalKapp->exec();
}



KMEDIA::KMEDIA( QWidget *parent, const char *name ) :  QWidget(parent,name)
{
  kmw = new KMediaWin(this, " ");
  // Window title
  setCaption( globalKapp->getCaption() );
}


void KMediaWin::baseinitMediatool()
{
  static bool InitDone=false;
  if (!InitDone)
  {
     MdConnectInit();
     InitDone=true;
  }
}


// call this for every new connection
void KMediaWin::initMediatool() 
{
  baseinitMediatool();
  MdConnectNew(&m);
  if ( m.shm_adr == NULL )
    dtfatal("Failed creating media connection.\n");

//  printf("The media id (talk id) for today is %i.\n", m.talkid);

  Playlist = PlaylistNew();
  if ( !Playlist )
    printf("Warning: Playlist not available. Please debug.\n");

  KeysChunk   = (MdCh_KEYS*)FindChunkData(m.shm_adr, "KEYS");
  if (!KeysChunk) dtfatal("No KEYS chunk.\n");
  StatChunk   = (MdCh_STAT*)FindChunkData(m.shm_adr, "STAT");
  if (!StatChunk) dtfatal("No STAT chunk.\n");
  StatStatPtr =  &(StatChunk->status);
  FnamChunk   = (MdCh_FNAM*)FindChunkData(m.shm_adr, "FNAM");
  if (!FnamChunk) dtfatal("No FNAM chunk.\n");
  IhdrChunk   = (MdCh_IHDR*)FindChunkData(m.shm_adr, "IHDR");
  if (!IhdrChunk) dtfatal("No IHDR chunk.\n");
}


void KMediaWin::dtfatal(char *text)
{
  fprintf(stderr, "%s", text);
  exit(-1);
}


void KMediaWin::showOpts()
{
  prefDL->slotShow();
}

