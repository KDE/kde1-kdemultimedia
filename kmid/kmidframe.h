/**************************************************************************

    kmidframe.h  - The main widget of KMid
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
#ifndef _KMIDFRAME_H
#define _KMIDFRAME_H

//#include <kapp.h>
//#include <qwidget.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kslider.h>
#include "player/midiout.h"
#include "player/player.h"
#include "player/track.h"
//#include <pthread/mit/pthread.h>
#include <qtimer.h>
//#include "kdisptext.h"

// uncomment the next line to debug KMid
//#define KMidDEBUG

class KApplication;
class KConfig;
class kmidClient;
class KToolBar;
class KDNDDropZone;

class kmidFrame : public KTopLevelWidget
{
    Q_OBJECT
private:
#ifdef KMidDEBUG
    long passcount;
#endif

    midiOut *Midi;
    player *Player;

    int playerProcessID;
    PlayerController *pctl;

    QPopupMenu *m_file;
    QPopupMenu *m_song;
    QPopupMenu *m_options;
    QPopupMenu *m_help;

protected:

    virtual void saveProperties(KConfig *kcfg);
    virtual void readProperties(KConfig *kcfg);


//    virtual void closeEvent(QCloseEvent *e);
public:
    kmidFrame(const char *name=0);
    ~kmidFrame();

//    void readConfig(KConfig *kconf);

public slots:

    void buttonClicked(int i);
    void file_Open();
    void song_Pause();

    void options_GM();
    void options_MT32();
    void options_Text();
    void options_Lyrics();
    void options_AutomaticText();
    void options_FontChange();
    void options_MidiSetup();

    void slotDropEvent( KDNDDropZone * _dropZone );
 


private:
    KMenuBar *menu;
    kmidClient *kmidclient;
    KToolBar *toolbar;
};

#endif
