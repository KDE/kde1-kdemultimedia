/**************************************************************************

    kmidclient.h  - The main client widget of KMid
    Copyright (C) 1997,98  Antonio Larrosa Jimenez

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
#ifndef _KMIDCLIENT_H
#define _KMIDCLIENT_H

#include "kslidertime.h"
#include "player/player.h"
#include "player/track.h"
#include <qtimer.h>
#include "kdisptext.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

// uncomment the next line to debug KMid
//#define KMidDEBUG

class DeviceManager;


class KApplication;
class KConfig;
class QLCDNumber;
class QLabel;

class kmidClient : public QWidget
{
    Q_OBJECT
private:
#ifdef KMidDEBUG
    long passcount;
#endif

    DeviceManager *Midi;
    player *Player;

    int playerProcessID;
    PlayerController *pctl;

    QTimer *timer4timebar;
    QTimer *timer4events;

    timeval begintv;
    ulong beginmillisec;
    ulong pausedatmillisec;
    SpecialEvent *spev;

    int	itsme;

    char *midifile_opened;

    int typeoftextevents;

    void extractFilename(const char *in,char *out);
public:
    kmidClient(QWidget *parent,const char *name=0);
    ~kmidClient();

    char *midiFileName(void) {return midifile_opened;};
    int isPlaying(void) {return pctl->playing;};

    int openFile(char *filename);
    int openURL(char *s);

    void repaintText(int typeoftextevents);
    void kmidOutput(void);

    void songType(int i);
    int ChooseTypeOfTextEvents(void);

    QFont *getFont(void);
    void fontChanged(void); // The new font is already in KConfig

protected:
    void resizeEvent(QResizeEvent *qre);


public slots:
    void file_Open();
    void help_Help();
    void help_About();
    void song_Play();
    void song_Pause();
    void song_Stop();
    void song_Rewind();
    void song_Forward();

    void timebarUpdate();
    void timebarChange(int i);

    void processSpecialEvent();

    DeviceManager *devman(void) {return Midi;};
    void setMidiDevice(int i);
    void setMidiMapFilename(char *mapfilename);
private:
    KSlider *timebar;
    KSliderTime *timetags;
    QLCDNumber *tempoLCD;
    KDisplayText *kdispt;
    QLabel *qlabelTempo;
};

#endif
