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

#include "kmedia.h"


void KMediaWin::playClicked()
{
  /* play/pause/unpause */
  if ( KeysChunk->pause )
    {
      KeysChunk->pause = 0;
      EventCounterRaise(&(KeysChunk->play),1);
    }
  else
    {
      if  ( *StatStatPtr & MD_STAT_PLAYING )
	{
	  KeysChunk->pause = 1; 
	}
      else
	{
	  EventCounterRaise(&(KeysChunk->play),1);
 	}
    }
}

void KMediaWin::stopClicked()
{
  printf( "EventCounter Stop raised\n");
  EventCounterRaise(&(KeysChunk->stop),1);
}

void KMediaWin::prevClicked()
{
  EventCounterRaise(&(KeysChunk->prevtrack),1);
  TimerAction=PREVTRACK;
}

void KMediaWin::prevReleased()
{
  /* empty for now */
  TimerAction=NOP;
}

void KMediaWin::nextClicked()
{
  EventCounterRaise(&(KeysChunk->nexttrack),1);
  TimerAction=NEXTTRACK;
}

void KMediaWin::nextReleased()
{
  /* empty for now */
  TimerAction=NOP;
}

void KMediaWin::fwdClicked()
{
  EventCounterRaise(&(KeysChunk->forward),1);
  TimerAction=FF;
}

void KMediaWin::fwdReleased()
{
  /* empty for now */
  TimerAction=NOP;
}

void KMediaWin::bwdClicked()
{
  EventCounterRaise(&(KeysChunk->backward),1);
  TimerAction=REW;
}

void KMediaWin::bwdReleased()
{
  /* empty for now */
  TimerAction=NOP;
}

/* Martin Jones told me the CloseEvent trick. Thank you! */
void KMediaWin::closeEvent( QCloseEvent * )
{
  quitClicked();
}
