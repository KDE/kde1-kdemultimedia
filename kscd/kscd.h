/*   
   Kscd - A simple cd player for the KDE Project

   $Id$
 
   Copyright (c) 1997 Bernd Johannes Wuebben math.cornell.edu
   
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



#ifndef __KSCD__
#define __KSCD__

#include "bwlednum.h"

#include <qpushbt.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qapp.h>
#include <qtimer.h>
#include <qbitmap.h>
#include <qcombo.h>
#include <qmsgbox.h>
#include <qscrbar.h>
#include <qslider.h>
#include <qtabdlg.h>
#include <qtooltip.h> 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/* this is for glibc 2.x which the ust structure in ustat.h not stat.h */
#ifdef __GLIBC__
#include <sys/ustat.h>
#endif

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

#ifdef linux
#include <mntent.h>
#endif

#include <kapp.h>


struct configstruct{
  QColor led_color;
  QColor background_color;
  bool   tooltips;
};


class KSCD : public QDialog {
	Q_OBJECT
private:
	// Buttons
	QPushButton	*playPB;
	QPushButton	*stopPB;
	QPushButton	*prevPB;
	QPushButton	*nextPB;
	QPushButton	*fwdPB;
	QPushButton	*bwdPB;
	QPushButton	*quitPB;
	QPushButton	*replayPB;
	QPushButton	*ejectPB;
	QPushButton	*aboutPB;
	// LED 

	QColor background_color;
	QColor led_color;
	BW_LED_Number	*trackTimeLED[6];
	QLabel *statuslabel;
	QLabel *titlelabel;
	QLabel *volumelabel;
	QLabel *tracklabel;

//	QPushButton *databasebutton;
	QPushButton *optionsbutton;
	QPushButton *shufflebutton;
	QPushButton		*volLA;
	// Timer to update the screen 
	QTimer		*timer;
	QTimer		*initimer;
	// Song List
	QComboBox	*songListCB;
	// Volume Control
	QSlider		*volSB;
	// CD volume
	int		volChnl;
	int		mixerFd;
	int 	        volume;
	QFrame 	*backdrop;
	KConfig *config;
	bool tooltips;
	bool randomplay ;
	bool looping;
	bool volstartup;
	bool cddrive_is_ok;
   // Private functions
	QPushButton     *makeButton( int, int, int, int, const char * );

	void		initWorkMan();
//	void		checkMount();
	void		drawPanel();
	void		cleanUp();
	void		loadBitmaps();
	void 	 	setLEDs(QString symbols);
	int 	        randomtrack();

//TODO get rid of the mixe stuff 
	void		initMixer( const char *mixer = "/dev/mixer" );

	// These are the variables from workbone
	int 		sss;
	int 		sel_stat;
	int 		dly;
	int 		fastin;
	int 		scmd;
	int 		tmppos;
	int 		save_track;
	struct timeval 	mydelay;
	struct mntent 	*mnt;
	FILE 		*fp;

public:
	KSCD( QWidget *parent = 0, const char *name = 0 );


protected:
	void closeEvent( QCloseEvent *e );

public slots:

	void 	       setToolTips();
	void 	       randomSelected();
	void 	        readSettings();
        void            writeSettings();
	void 		setColors();
	void		playClicked();
	void		initCDROM();
	void		stopClicked();
	void		prevClicked();
	void		nextClicked();
	void		fwdClicked();
	void		bwdClicked();
	void		quitClicked();
	void		loopClicked();
	void 		cdMode();
	void		ejectClicked();
	void		trackSelected( int );
	void		aboutClicked();
	void		volChanged( int );
};

#endif

