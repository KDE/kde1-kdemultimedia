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

extern "C" {
#include "struct.h"
}

#include <qdir.h>
#include <kfm.h>

#include "docking.h"
#include "kscd.h"
#include "configdlg.h"
#include "version.h"
#include "config.h"
#include "cddb.h"
#include "inexact.h"
#include "CDDialog.h"
#include "CDDBSetup.h"
#include "bitmaps/playpaus.xbm"
#include "bitmaps/stop.xbm"
#include "bitmaps/repeat.xbm"
#include "bitmaps/nexttrk.xbm"
#include "bitmaps/prevtrk.xbm"
#include "bitmaps/ff.xbm"
#include "bitmaps/rew.xbm"
#include "bitmaps/info.xbm"
#include "bitmaps/poweroff.xbm"
#include "bitmaps/eject.xbm"
#include "bitmaps/db.xbm"
#include "bitmaps/logo.xbm"
#include "bitmaps/shuffle.xbm"
#include "bitmaps/options.xbm"

KApplication 	*mykapp;
KSCD 	         *k;
DockWidget*     dock_widget;

bool             debugflag = false;
char 		tmptime[100];
char 		*tottime;
//static void 	playtime (void);
void 		kcderror(char* title,char* message);
void 		kcdworker(int );

//void 		parseargs(char* buf, char** args);
extern QTime framestoTime(int frames);
extern void cddb_decode(QString& str);
extern void cddb_encode(QString& str, QStrList &returnlist);
extern void cddb_playlist_encode(QStrList& list,QString& playstr);
extern bool cddb_playlist_decode(QStrList& list, QString& str);

/************************************************************************
 new for workman: (BERND)
 *****************************************************************************/
extern "C" {
  int play_cd(int start,int pos,int end);
  int pause_cd();
  int cd_close();
  int stop_cd();
  int cd_status();
  int eject_cd();
  int cd_volume(int vol, int bal,int max); 
}

extern struct play *playlist ;
extern struct cdinfo_wm thiscd, *cd ;
extern char	*cd_device;
extern int have_new_cd;

extern int	cur_track;	/* Current track number, starting at 1 */
extern int	cur_index;	/* Current index mark */
extern int	cur_lasttrack ;	/* Last track to play in current chunk */
extern int	cur_firsttrack;	/* First track of current chunk */
int    cur_pos_abs;	/* Current absolute position in seconds */
int    cur_frame;	/* Current frame number */
int    cur_pos_rel;	/* Current track-relative position in seconds */
int    cur_tracklen;	/* Length in seconds of current track */
int    cur_cdlen;	/* Length in seconds of entire CD */
int    cur_ntracks;	/* Number of tracks on CD (= tracks + sections) */
int    cur_nsections;	/* Number of sections currently defined */
extern enum cd_modes	cur_cdmode;
int    cur_listno;	/* Current index into the play list, if playing */
char  *cur_artist;	/* Name of current CD's artist */
char  *cur_cdname;	/* Album name */
char  *cur_trackname;	/* Take a guess */
char   cur_contd;	/* Continued flag */
char   cur_avoid;	/* Avoid flag */

int cur_balance;
int info_modified;
int cur_stopmode;
int cur_playnew;
int mark_a, mark_b;

/****************************************************************************
				The GUI part 
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name ) :
  QWidget( parent, name ){

  cd_device_str        	= "";
  background_color 	= black;
  led_color 		= green;
  randomplay 		= false;
  looping 		= false;
  cddrive_is_ok 		= true;
  tooltips 		= true;
  cycle_flag		= false;
  cddb_remote_enabled   = true;
  cddb 			= 0L;
  setup 	       	= 0L;
  time_display_mode 	= TRACK_SEC;
  cddb_inexact_sentinel =false;
  revision		= 1;
  use_kfm 		= true;
  docking 		= true;
  updateDialog          = false;
  drawPanel();
  loadBitmaps();
  setColors();
  setToolTips();
  
  cddialog 	  = 0L;
  timer 	  = new QTimer( this );
  queryledtimer   = new QTimer( this );
  titlelabeltimer = new QTimer( this );
  initimer 	  = new QTimer( this );
  cycletimer      = new QTimer( this );
  
  connect( initimer, SIGNAL(timeout()),this,  SLOT(initCDROM()) );
  connect( queryledtimer, SIGNAL(timeout()),  SLOT(togglequeryled()) );
  connect( titlelabeltimer, SIGNAL(timeout()),  SLOT(titlelabeltimeout()) );
  connect( cycletimer, SIGNAL(timeout()),  SLOT(cycletimeout()) );
  connect( timer, SIGNAL(timeout()),  SLOT(cdMode()) );
  connect( playPB, SIGNAL(clicked()), SLOT(playClicked()) );
  connect( stopPB, SIGNAL(clicked()), SLOT(stopClicked()) );
  connect( prevPB, SIGNAL(clicked()), SLOT(prevClicked()) );
  connect( nextPB, SIGNAL(clicked()), SLOT(nextClicked()) );
  connect( fwdPB, SIGNAL(clicked()), SLOT(fwdClicked()) );
  connect( bwdPB, SIGNAL(clicked()), SLOT(bwdClicked()) );
  connect( quitPB, SIGNAL(clicked()), SLOT(quitClicked()) );	
  connect( replayPB, SIGNAL(clicked()), SLOT(loopClicked()) );
  connect( ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );
  connect( songListCB, SIGNAL(activated(int)), SLOT(trackSelected(int)));
  connect( volSB, SIGNAL(valueChanged(int)), SLOT(volChanged(int)));
  connect( aboutPB, SIGNAL(clicked()), SLOT(cycleplaytimemode()));
  //connect( aboutPB, SIGNAL(clicked()), SLOT(aboutClicked()));
  connect( optionsbutton, SIGNAL(clicked()), SLOT(aboutClicked()));
  connect( shufflebutton, SIGNAL(clicked()), SLOT(randomSelected()));
  connect( cddbbutton, SIGNAL(clicked()), SLOT(CDDialogSelected()));
  connect(mykapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(setColors()));
	
  readSettings();
  setColors();
  initWorkMan();

  setupPopups();
  volstartup = TRUE;
  volSB->setValue(volume);

  dock_widget = new DockWidget("dockw");  

  setFocusPolicy ( QWidget::NoFocus );
  srandom(time(0L));
  initimer->start(500,TRUE);

}

	
// Initialize the variables only in WorkMan
void KSCD::initWorkMan() {

  mark_a	= 0; 
  mark_b 	= 0;
  cur_balance 	= 10;
  fastin 	= FALSE;
  scmd 		= 0;
  tmppos 	= 0;
  save_track 	= 1;
  thiscd.trk 	= NULL;
  thiscd.lists 	= NULL;
  tottime 	= tmptime;

}
	

/* I am dropping this code for now. People seem to be having nothing
   but trouble with this code and it was of dubious value anyways.... 
#ifdef linux

// check if drive is mounted (from Mark Buckaway's cdplayer code)
void KSCD::checkMount() 
{
  	if ((fp = setmntent (MOUNTED, "r")) == NULL) {
		fprintf (stderr, "Couldn't open %s: %s\n", 
			 MOUNTED, strerror (errno));
		exit (1);
	}
	while ((mnt = getmntent (fp)) != NULL) {
		if (strcmp (mnt->mnt_type, "iso9660") == 0) {
			fputs ("CDROM already mounted. Operation aborted.\n", 
			       stderr);
			endmntent (fp);
			exit (1);
		}
	}
	endmntent (fp);
	
}


#elif defined (__FreeBSD__)

void KSCD::checkMount() 
{
      struct statfs *mnt;
      int i, n;

      n = getmntinfo(&mnt, MNT_WAIT);
      for (i=0; i<n; i++) {
              if (mnt[i].f_type == MOUNT_CD9660) {
                      fputs("CDROM already mounted. Operation aborted.\n",
                            stderr);
                      exit(1);
              }
      }
}     

#else

// TODO Can I do this for other platforms?
void KSCD::checkMount(){
}

#endif
*/

void KSCD::initCDROM(){

  initimer->stop();
  mykapp->processEvents();
  mykapp->flushX();

  cdMode();
  volstartup = FALSE;
  if(cddrive_is_ok)
    volChanged(volume);

  timer->start(1000);
  //  cdMode();
}  


QPushButton *KSCD::makeButton( int x, int y, int w, int h, const char *n ) 
{
  QPushButton *pb = new QPushButton( n, this );
  pb->setGeometry( x, y, w, h );
  pb->setFocusPolicy ( QWidget::NoFocus );
  return pb;
}
	
void KSCD::drawPanel() 
{
  int ix = 0;
  int iy = 0;
  //	const int WIDTH = 100;
  const int WIDTH = 90;
  //	const int HEIGHT = 29;
  const int HEIGHT = 27;
  //	const int SBARWIDTH = 180; //140
  const int SBARWIDTH = 220; //140
  
  setCaption( "kscd" );
  aboutPB = makeButton( ix, iy, WIDTH, 2 * HEIGHT, "About" );
  
  ix = 0;
  iy += 2 * HEIGHT;

  infoPB = makeButton( ix, iy, WIDTH/2, HEIGHT, "" );
  ejectPB = makeButton( ix + WIDTH/2, iy, WIDTH/2, HEIGHT, "" );
  
  iy += HEIGHT;
  quitPB = makeButton( ix, iy, WIDTH, HEIGHT, "Quit" );
	


  ix += WIDTH;
  iy = 0;
  
  backdrop = new QFrame(this);
  backdrop->setGeometry(ix,iy,SBARWIDTH -2, 2* HEIGHT + HEIGHT /2 -1);
  backdrop->setFocusPolicy ( QWidget::NoFocus );
  


  int D = 6;
  
  //ix += 2 * SBARWIDTH / 7;
  ix = WIDTH + 8;
  /*
    nLEDs = new QLabel(this);
    nLEDs->setGeometry(ix + 20,iy +5 + D, 100, 30);
    nLEDs->setFont( QFont( "Helvetica", 26, QFont::Bold ) );
    */
  
  for (int u = 0; u<5;u++){
    trackTimeLED[u] = new BW_LED_Number(this );	 
    trackTimeLED[u]->setGeometry( ix  + u*18, iy + D, 23 ,  30 );
    trackTimeLED[u]->setLEDoffColor(background_color);
    trackTimeLED[u]->setLEDColor(led_color,background_color);
  }
  
  QString zeros("--:--");
  setLEDs(zeros);
  
  artistlabel = new QLabel(this);
  artistlabel->setGeometry(WIDTH + 5, iy + 38 , SBARWIDTH -15, 13);
  artistlabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  artistlabel->setAlignment( AlignLeft );
  artistlabel->setText("");
  
  titlelabel = new QLabel(this);
  titlelabel->setGeometry(WIDTH + 5, iy + 50 , SBARWIDTH -15, 13);
  titlelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
  titlelabel->setAlignment( AlignLeft );
  titlelabel->setText("");


  statuslabel = new QLabel(this);
  statuslabel->setGeometry(WIDTH + 110, iy  +D, 50, 14);
  statuslabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
  statuslabel->setAlignment( AlignLeft );
  //	statuslabel->setText("Ready");

  queryled = new LedLamp(this);
  queryled->move(WIDTH + 200, iy  +D +1 );
  queryled->off();
  queryled->hide();
  
  volumelabel = new QLabel(this);
  volumelabel->setGeometry(WIDTH + 110, iy + 14 + D, 50, 14);
  volumelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
  volumelabel->setAlignment( AlignLeft );
  volumelabel->setText("Vol: --");
  
  tracklabel = new QLabel(this);
  tracklabel->setGeometry(WIDTH + 168, iy + 14 +D, 50, 14);
  tracklabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
  tracklabel->setAlignment( AlignLeft );
  tracklabel->setText("--/--");
  
  totaltimelabel = new QLabel(this);
  totaltimelabel->setGeometry(WIDTH + 168, iy  +D, 50, 14);
  totaltimelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
  totaltimelabel->setAlignment( AlignLeft );
  totaltimelabel->hide();

  /*
    trackTimeLED = new QLEDNumber( this );
    trackTimeLED->setGeometry( ix -10, iy+5, 100    , 2 * HEIGHT );// petit
    trackTimeLED->display("");
    trackTimeLED->setFrameStyle( QFrame::NoFrame );
    */
  
  ix = WIDTH;
  iy = HEIGHT + HEIGHT + HEIGHT/2;
  // Volume control here

  volSB = new QSlider( 0, 100, 5,  50, QSlider::Horizontal, this, "Slider" );
  volSB->setGeometry( ix, iy, SBARWIDTH, HEIGHT/2 );
  volSB->setFocusPolicy ( QWidget::NoFocus );

  iy += HEIGHT/2  +1 ;
  cddbbutton = new QPushButton( this );
  cddbbutton->setGeometry( ix , iy, SBARWIDTH/10 *2 , HEIGHT );
  cddbbutton->setFont( QFont( "helvetica", 12 ) );
  cddbbutton->setFocusPolicy ( QWidget::NoFocus );

  ix += SBARWIDTH/10*2;
  shufflebutton = new QPushButton( this );
  shufflebutton->setGeometry( ix , iy, SBARWIDTH/10 *2  , HEIGHT );
  shufflebutton->setFont( QFont( "helvetica", 12 ) );
  shufflebutton->setFocusPolicy ( QWidget::NoFocus );

  ix += SBARWIDTH/10*2;
  
  optionsbutton = new QPushButton( this );
  optionsbutton->setGeometry( ix, iy, SBARWIDTH/10 *2  , HEIGHT );
  optionsbutton->setFont( QFont( "helvetica", 12 ) );
  optionsbutton->setFocusPolicy ( QWidget::NoFocus );

  ix += SBARWIDTH/10*2;
  songListCB = new QComboBox( this );
  songListCB->setGeometry( ix, iy, SBARWIDTH/10*4, HEIGHT );
  songListCB->setFont( QFont( "helvetica", 12 ) );
  songListCB->setFocusPolicy ( QWidget::NoFocus );

  iy = 0;
  ix = WIDTH + SBARWIDTH;
  playPB = makeButton( ix, iy, WIDTH, HEIGHT, "Play/Pause" );
  
  iy += HEIGHT;
  stopPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Stop" );
  
  ix += WIDTH / 2;
  replayPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Replay" );
  
  ix = WIDTH + SBARWIDTH;
  iy += HEIGHT;
  bwdPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Bwd" );
  
  ix += WIDTH / 2;
  fwdPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Fwd" );
  
  ix = WIDTH + SBARWIDTH;
  iy += HEIGHT;
  prevPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Prev" );
  
  ix += WIDTH / 2;
  nextPB = makeButton( ix, iy, WIDTH / 2, HEIGHT, "Next" );
  
  this->adjustSize();
  this->setFixedSize(this->width(),this->height());

}

void KSCD::loadBitmaps() {

  QBitmap playBmp( playpause_width, playpause_height, playpause_bits,TRUE );
  QBitmap stopBmp( stop_width, stop_height, stop_bits, TRUE );
  QBitmap prevBmp( prevtrk_width, prevtrk_height, prevtrk_bits, TRUE );
  QBitmap nextBmp( nexttrk_width, nexttrk_height, nexttrk_bits, TRUE );
  QBitmap replayBmp( repeat_width, repeat_height, repeat_bits, TRUE );
  QBitmap fwdBmp( ff_width, ff_height, ff_bits, TRUE );
  QBitmap bwdBmp( rew_width, rew_height, rew_bits, TRUE );
  QBitmap ejectBmp( eject_width, eject_height, eject_bits, TRUE );
  QBitmap infoBmp( info_width, info_height,info_bits, TRUE );
  QBitmap quitBmp( poweroff_width, poweroff_height, poweroff_bits, TRUE );
  QBitmap shuffleBmp( shuffle_width, shuffle_height, shuffle_bits, TRUE );
  QBitmap databaseBmp( db_width, db_height, db_bits, TRUE );
  QBitmap aboutBmp( logo_width, logo_height, logo_bits, TRUE );
  QBitmap optionsBmp( options_width, options_height, options_bits, TRUE );

  playPB->setPixmap( playBmp );
  stopPB->setPixmap( stopBmp );
  prevPB->setPixmap( prevBmp );
  nextPB->setPixmap( nextBmp );
  replayPB->setPixmap( replayBmp );
  fwdPB->setPixmap( fwdBmp );
  bwdPB->setPixmap( bwdBmp );
  ejectPB->setPixmap( ejectBmp );
  infoPB->setPixmap( infoBmp );
  quitPB->setPixmap( quitBmp );
  aboutPB->setPixmap( aboutBmp );
  shufflebutton->setPixmap( shuffleBmp );
  cddbbutton->setPixmap( databaseBmp );
  optionsbutton->setPixmap( optionsBmp );

}


void KSCD::setupPopups(){

  mainPopup   = new QPopupMenu ();
  perfPopup   = new QPopupMenu ();
  purchPopup   = new QPopupMenu ();
  infoPopup   = new QPopupMenu ();

  purchPopup->insertItem("CD Now");
  purchPopup->insertItem("CD Universe");

  perfPopup->insertItem("Tourdates.com");

  //  infoPopup->insertItem("Music Central");
  infoPopup->insertItem("Ultimate Bandlist");
  infoPopup->insertSeparator();
  infoPopup->insertItem("Deja News");
  infoPopup->insertItem("Excite");
  infoPopup->insertItem("HotBot");
  infoPopup->insertItem("Info Seek");
  infoPopup->insertItem("Lycos");
  infoPopup->insertItem("Magellan");
  infoPopup->insertItem("Yahoo!");


  mainPopup->insertItem (klocale->translate("Performances"), perfPopup);
  connect( perfPopup, SIGNAL(activated(int)), SLOT(performances(int)) );

  mainPopup->insertItem (klocale->translate("Purchases"), purchPopup);
  connect( purchPopup, SIGNAL(activated(int)), SLOT(purchases(int)) );

  mainPopup->insertItem (klocale->translate("Information"), infoPopup);
  connect( infoPopup, SIGNAL(activated(int)), SLOT(information(int)) );
  connect( infoPB, SIGNAL(clicked()), SLOT(showPopup()) );

}

void KSCD::showPopup(){

  QPoint point = this->mapToGlobal (QPoint(0,0));

  if(mainPopup)
    mainPopup->popup(point);

}


void KSCD::setToolTips() 
{
  if(tooltips){

    QToolTip::add( playPB, 		"Play/Pause" );
    QToolTip::add( stopPB, 		"Stop" );
    QToolTip::add( replayPB, 	"Loop CD" );
    QToolTip::add( songListCB, 	"Track Selection" );
    QToolTip::add( fwdPB, 		"30 Secs Forward" );
    QToolTip::add( bwdPB, 		"30 Secs Backward" );
    QToolTip::add( nextPB, 		"Next Track" );
    QToolTip::add( prevPB, 		"Previous Track" );
    QToolTip::add( quitPB, 		"Exit Kscd" );
    QToolTip::add( aboutPB, 	"Cycle Time Display" );
    QToolTip::add( optionsbutton, 	"Configure Kscd" );
    QToolTip::add( ejectPB, 	"Eject CD" );
    QToolTip::add( infoPB, 	"The Artist on the Web" );
    QToolTip::add( shufflebutton, 	"Random Play" );
    QToolTip::add( cddbbutton, 	"CDDB Dialog" );
    QToolTip::add( volSB, 		"CD Volume Control" );
  }	
  else{

    QToolTip::remove( playPB);
    QToolTip::remove( stopPB);
    QToolTip::remove( replayPB);
    QToolTip::remove( songListCB);
    QToolTip::remove( fwdPB );
    QToolTip::remove( bwdPB);
    QToolTip::remove( nextPB );
    QToolTip::remove( prevPB );
    QToolTip::remove( quitPB );
    QToolTip::remove( aboutPB );
    QToolTip::remove( optionsbutton );
    QToolTip::remove( ejectPB );
    QToolTip::remove( infoPB );
    QToolTip::remove( cddbbutton );
    QToolTip::remove( shufflebutton );
    QToolTip::remove( volSB );
  }

}

void KSCD::cleanUp(){

  if (thiscd.trk != NULL)
    delete thiscd.trk;
  signal (SIGINT, SIG_DFL);

}

void KSCD::playClicked(){


  qApp->processEvents();
  qApp->flushX();

#ifdef NEW_BSD_PLAYCLICKED

  if (cur_cdmode == STOPPED || cur_cdmode == UNKNOWN  || cur_cdmode == BACK) {

#else

  if (cur_cdmode == STOPPED || cur_cdmode == UNKNOWN ) {

#endif
    
    statuslabel->setText( "Playing" );
    songListCB->clear();
    setLEDs( "00:00" );

    for (int i = 0; i < cur_ntracks; i++){
      songListCB->insertItem( QString( 0 ).sprintf( "Track %02d", i + 1 ) );
    }

    qApp->processEvents();
    qApp->flushX();

    if(playlist.count()>0){
      if(playlistpointer >=(int) playlist.count())
	playlistpointer = 0;
      play_cd (atoi(playlist.at(playlistpointer)), 0, 
	       atoi(playlist.at(playlistpointer)) + 1);
      save_track = cur_track = atoi(playlist.at(playlistpointer));
    }
    else{
      play_cd (save_track, 0, cur_ntracks + 1);
    }
    
  } else 
    
    if (cur_cdmode == PLAYING || cur_cdmode == PAUSED) {
	
      switch (cur_cdmode) {
	
      case PLAYING:
	statuslabel->setText( "Pause" );
	break;
	
      case PAUSED:
	if(randomplay){
	  statuslabel->setText( "Random" );
	}
	else{
	  statuslabel->setText( "Playing" );
	}
	break;
	
      default:
	statuslabel->setText( "Strange...." );
	break;
	
      }
	
      pause_cd ();
      qApp->processEvents();
      qApp->flushX();
    }
    
  cdMode();
}

void KSCD::stopClicked(){

  looping = FALSE;
  randomplay = FALSE;
  statuslabel->setText("Stopped");
  setLEDs("--:--");
  qApp->processEvents();
  qApp->flushX();
  
  save_track = cur_track = 1;
  playlistpointer = 0;
  stop_cd ();

}

void KSCD::prevClicked(){

  setLEDs("00:00");
  qApp->processEvents();
  qApp->flushX();
  
  if(playlist.count()>0){
    playlistpointer--;
    if(playlistpointer < 0 )
      playlistpointer = playlist.count() -1;
    cur_track = atoi(playlist.at(playlistpointer));
  }
  else{
  cur_track--;
  if (cur_track < 1)
    cur_track = cur_ntracks;
  }
  if(randomplay){
      play_cd (cur_track, 0, cur_track + 1);
  }
  else{
    play_cd (cur_track, 0, cur_ntracks + 1);
  }

}

void KSCD::nextClicked(){


  setLEDs("00:00");
  qApp->processEvents();
  qApp->flushX();

  if(randomplay){

    int j = randomtrack();
    QString str;
    str.sprintf("%02d/%02d",j,cd->ntracks );
    tracklabel->setText(str.data());
    
    if(j < (int)tracktitlelist.count()){
      titlelabel->setText(tracktitlelist.at(j));
      artistlabel->setText(tracktitlelist.at(0));
    }
    qApp->processEvents();
    qApp->flushX();
    
    play_cd( j, 0, j + 1 );
    
  }
  else if(playlist.count() > 0){
    if(playlistpointer < (int)playlist.count() - 1)
      playlistpointer++;
    else
      playlistpointer = 0;

    play_cd (atoi(playlist.at(playlistpointer)), 
	     0, atoi(playlist.at(playlistpointer)) + 1);    
    cur_track = atoi(playlist.at(playlistpointer));

  }
  else{
    if (cur_track == cur_ntracks)
      cur_track = 0;
    play_cd (cur_track + 1, 0, cur_ntracks + 1);
  }
}

void KSCD::fwdClicked(){

  qApp->processEvents();
  qApp->flushX();
  
  if (cur_cdmode == PLAYING) {
    tmppos = cur_pos_rel + 30;
    if (tmppos < thiscd.trk[cur_track - 1].length) {
      if(randomplay || playlist.count() > 0)
	play_cd (cur_track, tmppos, cur_track + 1);		  
      else
	play_cd (cur_track, tmppos, cur_ntracks + 1);
    }
  }
}

void KSCD::bwdClicked(){

  qApp->processEvents();
  qApp->flushX();
  
  if (cur_cdmode == PLAYING) {
    tmppos = cur_pos_rel - 30;
    if(randomplay || playlist.count() > 0)
      play_cd (cur_track, tmppos > 0 ? tmppos : 0, cur_track + 1);
    else
      play_cd (cur_track, tmppos > 0 ? tmppos : 0, cur_ntracks + 1);
    
  }
  cdMode();
}

void KSCD::quitClicked(){

  randomplay = FALSE;
  statuslabel->setText("");
  setLEDs( "--:--" );
  
  qApp->processEvents();
  qApp->flushX();
  
  stop_cd ();

  cd_status();
  cd_status();

  cleanUp();
  writeSettings();
  qApp->quit();
}

void KSCD::closeEvent( QCloseEvent *e ){


  if(docking){
    dock_widget->dock();
    this->hide();
  }
  else{
    e->ignore();
  }

};

void KSCD::loopClicked(){

  randomplay = FALSE;

  if(looping){
    looping  = FALSE;
  }
  else{
    if(cur_cdmode == PLAYING){
      looping = TRUE;
      statuslabel->setText("Loop");
    }    
  }
}

void KSCD::ejectClicked(){

  if(!cddrive_is_ok)
    return;

  looping = FALSE;
  randomplay = FALSE;
  statuslabel->setText("Ejecting");
  qApp->processEvents();
  qApp->flushX();
  artistlabel->setText("");
  titlelabel->setText("");
  tracktitlelist.clear();
  extlist.clear();
  
  stop_cd();
  //  timer->stop();
  eject_cd(1);

}

void KSCD::randomSelected(){

  looping = FALSE;
  if(randomplay == TRUE){
    randomplay = FALSE;
  }
  else{

    statuslabel->setText("Random");
    randomplay = TRUE;
    
    int j = randomtrack();
    QString str;
    str.sprintf("%02d/%02d",j,cd->ntracks );
    tracklabel->setText(str.data());
    if(j < (int)tracktitlelist.count()){
      titlelabel->setText(tracktitlelist.at(j));
      artistlabel->setText(tracktitlelist.at(0));
    }

    qApp->processEvents();
    qApp->flushX();

    play_cd( j, 0, j + 1 );
    cur_track = j;

  }
}

void KSCD::trackSelected( int trk ){

  randomplay = false;
  QString str;
  str.sprintf("%02d/%02d",trk + 1,cd->ntracks );
  tracklabel->setText(str.data());

  if(trk+1 < (int)tracktitlelist.count()){
    titlelabel->setText(tracktitlelist.at(trk+1));
    artistlabel->setText(tracktitlelist.at(0));
  }

  setLEDs("00:00");
  qApp->processEvents();
  qApp->flushX();
  
  cur_track = trk + 1;
  //  pause_cd();
  play_cd( cur_track, 0, cur_ntracks + 1 );

}

void KSCD::aboutClicked(){

  QString server_copy;
  server_copy = current_server.copy();

  QTabDialog * tabdialog;

  tabdialog = new QTabDialog(0,"tabdialog",TRUE);
  tabdialog->setCaption( "kscd Configuraton" );
  tabdialog->resize( 400, 450 );
  tabdialog->setCancelButton();

  QWidget *about = new QWidget(tabdialog,"about");

  QGroupBox *box = new QGroupBox(about,"box");
  QLabel  *label = new QLabel(box,"label");
  box->setGeometry(10,10,365,360);
  label->setGeometry(170,30,165,300);
  label->setAlignment( AlignCenter);
  QString labelstring;
  labelstring = "kscd "KSCDVERSION"\n"\
		 "Copyright (c) 1997 \nBernd Johannes Wuebben\n"\
		 "wuebben@math.cornell.edu\n"\
		 "wuebben@kde.org\n\n"\
		 "\nkscd  contains code from:\n"
                 "workman 1.4 beta 3\n"
                 "Copyright (c) Steven Grimm \n"\
                 "koreth@hyperion.com\n\n"\
                 "Special thanks to Ti Kan and "\
                 "Steve Scherf, the inventors of "\
                 "the CDDB database concept. "\
                 "Visit http://www.cddb.com for "\
                 "more information on CDDB.";

  label->setAlignment(AlignLeft|WordBreak|ExpandTabs);
  label->setText(labelstring.data());
  
  QString pixdir = mykapp->kdedir() + QString("/share/apps/kscd/pics/"); 

  QPixmap pm((pixdir + "kscdlogo.xpm").data());
  QLabel *logo = new QLabel(box);
  logo->setPixmap(pm);
  logo->setGeometry(40, 50, pm.width(), pm.height());

  ConfigDlg* dlg;
  struct configstruct config;
  config.background_color = background_color;
  config.led_color = led_color;
  config.tooltips = tooltips;
  config.cd_device = cd_device;
  config.mailcmd = mailcmd.copy();
  config.browsercmd = browsercmd.copy();
  config.use_kfm    = use_kfm;
  config.docking    = docking;

  dlg = new ConfigDlg(tabdialog,&config,"configdialg");

  setup = new CDDBSetup(tabdialog,"cddbsetupdialog");
  connect(setup,SIGNAL(updateCDDBServers()),this,SLOT(getCDDBservers()));
  connect(setup,SIGNAL(updateCurrentServer()),this,SLOT(updateCurrentCDDBServer()));
  setup->insertData(cddbserverlist,cddbbasedir,
		    submitaddress,current_server,cddb_remote_enabled);


  tabdialog->addTab(setup,"CDDB");
  tabdialog->addTab(dlg,"Kscd Options");
  tabdialog->addTab(about,"About");


  
  if(tabdialog->exec() == QDialog::Accepted){
    
    background_color = dlg->getData()->background_color;
    led_color = dlg->getData()->led_color;
    tooltips = dlg->getData()->tooltips;
    mailcmd = dlg->getData()->mailcmd.copy();

    browsercmd = dlg->getData()->browsercmd.copy();
    use_kfm = dlg->getData()->use_kfm;
    docking = dlg->getData()->docking;

    if( (QString)cd_device != dlg->getData()->cd_device){
      cd_device_str = dlg->getData()->cd_device;
      cd_close();
      cd_device = cd_device_str.data();
      
    }
    cddrive_is_ok = true;
      
    setup->getData(cddbserverlist,cddbbasedir,submitaddress,
		   current_server,cddb_remote_enabled);
    setColors();
    setToolTips();

  }
  else{
    // reset the current server in case we played with it ...

    current_server = server_copy.copy();
    if(debugflag) printf("RESETTING SERVER TO: %s\n",current_server.data());
  }

  disconnect(setup,SIGNAL(updateCDDBServers()),this,SLOT(getCDDBservers()));
  disconnect(setup,SIGNAL(updateCurrentServer()),this,SLOT(updateCurrentCDDBServer()));

  delete dlg;
  dlg = 0L;
  delete setup;
  setup = 0L;
  delete about;
  delete tabdialog;  

}

void KSCD::updateCurrentCDDBServer(){

  if(setup){
    setup->getCurrentServer(current_server);
    if(debugflag) printf("SET SERVER TO: %s\n",current_server.data());
  }
}

void KSCD::volChanged( int vol ){

  if(volstartup)
    return;

  QString str;
  str.sprintf("Vol: %02d%%",vol);
  volumelabel->setText(str.data());
  cd_volume(vol, 10, 100); // 10 -> right == left balance
  volume = vol;

}

int KSCD::randomtrack(){

  if(playlist.count() > 0){

    int j;
    j=(int) (((double) playlist.count()  ) * rand()/(RAND_MAX+1.0)); 
    playlistpointer = j;
    return atoi(playlist.at(j));
  }
  else{

    int j;
    j=1+(int) (((double)cur_ntracks) *rand()/(RAND_MAX+1.0)); 
    return j;
  }
}

void KSCD::cdMode(){

  static char *p = new char[10];
  static bool damn = TRUE;
  QString str;

  sss = cd_status();

  if(sss < 0){
    if(cddrive_is_ok){
      statuslabel->setText( "Error" );
      cddrive_is_ok = false;
      QString errstring;
      errstring.sprintf("CDROM read or access error.\n"\
			"Please make sure you have access permissions to:\n%s",cd_device);
      QMessageBox::warning(this,"Error",errstring.data());
    }
    return;
  }
  cddrive_is_ok = true; // cd drive ok
  
  switch (cur_cdmode) {
  case -1:         /* UNKNOWN */
    cur_track = save_track = 1;
    statuslabel->setText( "" ); // TODO how should I properly handle this
    damn = TRUE;
    break;
    
  case 0:         /* TRACK DONE */
    if( randomplay){
      
      int j = randomtrack();
      play_cd( j, 0, j + 1 );
      
    }
    else if (playlist.count() > 0){
      if(playlistpointer < (int)playlist.count() - 1)
	playlistpointer++;
      else
	playlistpointer = 0;
      play_cd(atoi(playlist.at(playlistpointer)),0,atoi(playlist.at(playlistpointer))+1);
    }
    else if (looping){
      if (cur_track == cur_ntracks){
	cur_track = 0;
	play_cd (1, 0, cur_ntracks + 1);
      }
      
    }
    else{  
      cur_track = save_track = 1;
      statuslabel->setText( "" ); // TODO how should I properly handle this
      damn = TRUE;
    }

    break;
    
  case 1:         /* PLAYING */
    playtime ();
    if(randomplay)
      statuslabel->setText( "Random" );
    else if(looping)
      statuslabel->setText("Loop");
    else
      statuslabel->setText( "Playing" );
    
    sprintf( p, "%02d  ", cur_track );
    if(songListCB->count() == 0){
      // we are in here when we start kscd and 
      // the cdplayer is already playing.	

      for (int i = 0; i < cur_ntracks; i++){
	songListCB->insertItem( QString( 0 ).sprintf("Track %02d",i + 1 ) );
      }
      songListCB->setCurrentItem( cur_track - 1 );
      have_new_cd = false;
      get_cddb_info(false); // false == do not update dialog if open
    }
    else{
      songListCB->setCurrentItem( cur_track - 1 );
    }
    str.sprintf("%02d/%02d",cur_track,cd->ntracks );
    tracklabel->setText(str.data());
    
    if((cur_track < (int)tracktitlelist.count()) && (cur_track >= 0)){
      titlelabel->setText(tracktitlelist.at(cur_track));
      artistlabel->setText(tracktitlelist.at(0));
    }
    
    setLEDs( tmptime );
    damn = TRUE;
    break;

  case 2: /*FORWARD*/
    break;
    
  case 3:         /* PAUSED */
    statuslabel->setText( "Pause" );
    damn = TRUE;
    break;

  case 4:         /* STOPPED */
    int i;
    if (damn) {

      statuslabel->setText( "Ready" );
      setLEDs( "--:--" );
      songListCB->clear();

      for (i = 0; i < cur_ntracks; i++)
	songListCB->insertItem( QString( 0 ).sprintf( "Track %02d", i + 1 ) );

      int w = ((cur_track >= 0) ? cur_track : 1);

      str.sprintf("%02d/%02d",cur_track >=0? cur_track :1 ,cd->ntracks );
      tracklabel->setText(str.data());
      
      if( w < (int)tracktitlelist.count()){
	titlelabel->setText(tracktitlelist.at( w ));
	artistlabel->setText(tracktitlelist.at(0));
      }
      
    }
    damn = FALSE;
    if(have_new_cd){

      //      timer->stop(); 
      have_new_cd = false;
      // timer must be restarted when we are doen
      // with getting the cddb info
      get_cddb_info(false); // false == do not update dialog if open
		  
    }

    break;

  case 5:         /* CDEJECT */
    statuslabel->setText( "Ejected" );
    songListCB->clear();
    setLEDs( "--:--" );	
    tracklabel->setText( "--/--" );
    titlelabel->setText("");
    artistlabel->setText("");
    totaltimelabel->setText("");
    totaltimelabel->lower();
    damn = TRUE;

    break;
  }
}

void KSCD::setLEDs(QString symbols){

  // nLEDs->setText(symbols);

  if(symbols.length() < 5){
    return;
  }

   for(int i=0;i<5;i++){
    trackTimeLED[i]->display(symbols.data()[i]);
  }


}


void KSCD::setColors(){

  backdrop->setBackgroundColor(background_color);
  
  QColorGroup colgrp( led_color, background_color, led_color,led_color , led_color,
		      led_color, white );

  titlelabel ->setPalette( QPalette(colgrp,colgrp,colgrp) );
  artistlabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
  volumelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
  statuslabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
  tracklabel ->setPalette( QPalette(colgrp,colgrp,colgrp) );
  totaltimelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
  // nLEDs->setPalette( QPalette(colgrp,colgrp,colgrp) );


  queryled->setPalette( QPalette(colgrp,colgrp,colgrp) );

  for (int u = 0; u< 5;u++){
    trackTimeLED[u]->setLEDoffColor(background_color);
    trackTimeLED[u]->setLEDColor(led_color,background_color);
  }

  titlelabel ->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  artistlabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  volumelabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  statuslabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  tracklabel ->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  totaltimelabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );
  artistlabel->setFont( QFont( "Helvetica", 10, QFont::Bold) );

}


void KSCD::readSettings(){

  config = mykapp->getConfig();
  
  config->setGroup("GENERAL");
  volume = config->readNumEntry("Volume",40);
  tooltips  = (bool) config->readNumEntry("ToolTips",1);
  randomplay = (bool) config->readNumEntry("RandomPlay", 0);
  use_kfm   = (bool) config->readNumEntry("USEKFM", 1);
  docking   = (bool) config->readNumEntry("DOCKING", 1);

  cd_device_str = config->readEntry("CDDevice",DEFAULT_CD_DEVICE);
  cd_device = cd_device_str.data();

  mailcmd = config->readEntry("UnixMailCommand","mail -s \"%s\" ");
  browsercmd = config->readEntry("CustomBroserCmd","netscape \"%s\" ");

  QColor defaultback = black;
  QColor defaultled = QColor(226,224,255);
  background_color = config->readColorEntry("BackColor",&defaultback);	
  led_color = config->readColorEntry("LEDColor",&defaultled);

  config->setGroup("CDDB");
  QString basedirdefault;
  basedirdefault = mykapp->kdedir().copy();
  basedirdefault += "/share/apps/kscd/cddb/";

  cddbbasedir = config->readEntry("LocalBaseDir",basedirdefault.data());
  cddb_remote_enabled = (bool) config->readNumEntry("CDDBRemoteEnabled",
						    (int)true);
  current_server = config->readEntry("CurrentServer","cddb.cddb.com 888");
  submitaddress = config->readEntry("CDDBSubmitAddress","xmcd-cddb@amb.org");
  int num;
  num = config->readListEntry("SeverList",cddbserverlist,',');
  if (num == 0)
    cddbserverlist.append("cddb.cddb.com 888");
}

void KSCD::writeSettings(){
		
  config = mykapp->getConfig();

  config->setGroup("GENERAL");	
  if(tooltips)
    config->writeEntry("ToolTips", 1);
  else
    config->writeEntry("ToolTips", 0);
  
  if(randomplay)
    config->writeEntry("RandomPlay", 1);
  else
    config->writeEntry("RandomPlay", 0); 
  config->writeEntry("USEKFM", (int)use_kfm); 
  config->writeEntry("DOCKING", (int)docking); 
  config->writeEntry("CDDevice",cd_device);
  config->writeEntry("CustomBroserCmd",browsercmd);
  config->writeEntry("Volume", volume);
  config->writeEntry("BackColor",background_color);
  config->writeEntry("LEDColor",led_color);
  config->writeEntry("UnixMailCommand",mailcmd);


  config->setGroup("CDDB");
  config->writeEntry("CDDBRemoteEnabled",(int)cddb_remote_enabled);
  config->writeEntry("LocalBaseDir",cddbbasedir);
  config->writeEntry("SeverList",cddbserverlist);
  config->writeEntry("CDDBSubmitAddress",submitaddress);
  config->writeEntry("CurrentServer",current_server);
  config->sync();
	
}

void KSCD::CDDialogSelected(){

  if(cddialog)
    return;

  cddialog = new CDDialog();

  cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
		  revision,playlist,pathlist,mailcmd,submitaddress);

  connect(cddialog,SIGNAL(cddb_query_signal(bool)),this,SLOT(get_cddb_info(bool)));
  connect(cddialog,SIGNAL(dialog_done()),this,SLOT(CDDialogDone()));
  connect(cddialog,SIGNAL(play_signal(int)),this,SLOT(trackSelected(int)));
  cddialog->show();

}

void KSCD::CDDialogDone(){
  
  disconnect(cddialog,SIGNAL(play_signal(int)),this,SLOT(trackSelected(int)));
  disconnect(cddialog,SIGNAL(dialog_done()),this,SLOT(CDDialogDone()));
  disconnect(cddialog,SIGNAL(cddb_query_signal(bool)),this,SLOT(get_cddb_info(bool)));
  delete cddialog;
  cddialog = 0L;

}

void KSCD::getCDDBservers(){

  led_on();
  if(cddb == 0L)
    cddb = new CDDB();

  connect(cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
  connect(cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));

  cddb->cddbgetServerList(current_server);
  
}

void KSCD::getCDDBserversFailed(){

  led_off();
  if(cddb){
   disconnect(cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
   disconnect(cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
  }
  titlelabel->setText("Unable to get CDDB server list.");
  artistlabel->setText("");
  titlelabeltimer->start(10000,TRUE); // 10 secs
}

void KSCD::getCDDBserversDone(){

  led_off();
  if(cddb){
   disconnect(cddb,SIGNAL(get_server_list_done()),this,SLOT(getCDDBserversDone()));
   disconnect(cddb,SIGNAL(get_server_list_failed()),this,SLOT(getCDDBserversFailed()));
    cddb->serverList(cddbserverlist);
    if(setup)
      setup->insertServerList(cddbserverlist);
  }

}

void KSCD::get_cddb_info(bool _updateDialog){

  updateDialog = _updateDialog;

  QTime dml;
  dml = dml.addSecs(cd->length);

  QString fmt;
  if(dml.hour() > 0)
    fmt.sprintf("%02d:%02d:%02d",dml.hour(),dml.minute(),dml.second());
  else
    fmt.sprintf("%02d:%02d",dml.minute(),dml.second());

  totaltimelabel->setText(fmt.data());

  if(cddb == 0L)
    cddb = new CDDB();

  get_pathlist(pathlist);
  cddb->setPathList(pathlist);

  connect(cddb,SIGNAL(cddb_ready()),this,SLOT(cddb_ready()));
  connect(cddb,SIGNAL(cddb_failed()),this,SLOT(cddb_failed()));
  connect(cddb,SIGNAL(cddb_done()),this,SLOT(cddb_done()));
  connect(cddb,SIGNAL(cddb_timed_out()),this,SLOT(cddb_timed_out()));
  connect(cddb,SIGNAL(cddb_inexact_read()),this,SLOT(mycddb_inexact_read()));
  connect(cddb,SIGNAL(cddb_no_info()),this,SLOT(cddb_no_info()));

  led_on();

  bool res = cddb->local_query(
			       cd->magicID,
			       xmcd_data,
			       tracktitlelist,
			       extlist,
			       category,
			       discidlist,
			       revision,
			       playlist
			       );

  if(!res && !cddb_remote_enabled){
    //    have_new_cd = false;
    cddb_no_info();
    return;
  }

  if(!res){

    if (debugflag) printf("STARTING REMOTE QUERY\n");
    cddb->cddb_connect(current_server);
  }
  else{

    if (debugflag) printf("FOUND RECORD LOCALLY\n");
    if((int)tracktitlelist.count() != (cd->ntracks + 1)){
      if(debugflag) printf(
	"WARNING LOCAL QUERY tracktitleslist.count = %d != cd->ntracks +1 = %d\n",
	tracktitlelist.count(),cd->ntracks + 1
	);
    }

    if((int)extlist.count() != (cd->ntracks + 1)){
      if (debugflag )printf("WARNING LOCAL QUERYextlist.count = %d != cd->ntracks +1 = %d\n",
	     extlist.count(),cd->ntracks + 1);
    }

    if(tracktitlelist.count() > 1){
      titlelabel->setText(tracktitlelist.at(1));
      artistlabel->setText(tracktitlelist.at(0));
    }

    led_off();
    timer->start(1000);

    if(cddialog && updateDialog)
      cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
			revision,playlist,pathlist,mailcmd,submitaddress);
    playlistpointer = 0;
  }

  //  have_new_cd = false;

}


void KSCD::cddb_ready(){

  if(!cddb || !cd)
    return;

  querylist.clear();
  tracktitlelist.clear();
  extlist.clear();
  discidlist.clear();

  QString num;

  for(int i = 0 ; i < cd->ntracks; i++){
    querylist.append(num.sprintf("%d",cd->cddbtoc[i].absframe));
  }

  querylist.append(num.sprintf("%d",cd->cddbtoc[cd->ntracks].absframe/75));
  cddb_inexact_sentinel =false;  
  cddb->queryCD(cd->magicID,querylist);

}

void KSCD::cddb_no_info(){

  titlelabel->setText("No matching CDDB entry found.");
  artistlabel->setText("");
  titlelabeltimer->start(10000,TRUE); // 10 secs

  tracktitlelist.clear();
  for(int i = 0 ; i <= cd->ntracks; i++)
    tracktitlelist.append("");

  extlist.clear();
  for(int i = 0 ; i <= cd->ntracks; i++)
    extlist.append("");

  discidlist.clear();

  timer->start(1000);
  led_off();
  cddb_inexact_sentinel =false;

}

void KSCD::cddb_failed(){

  // TODO differentiate between those casees where the communcition really 
  // failed and those where we just couldn't find anything

  tracktitlelist.clear();
  for(int i = 0 ; i <= cd->ntracks; i++)
    tracktitlelist.append("");

  extlist.clear();
  for(int i = 0 ; i <= cd->ntracks; i++)
    extlist.append("");

  discidlist.clear();

  titlelabel->setText("No matching CDDB entry found.");
  artistlabel->setText("");
  titlelabeltimer->start(10000,TRUE); // 10 secs

  timer->start(1000);
  led_off();
  cddb_inexact_sentinel =false;
}

void KSCD::cddb_timed_out(){

  tracktitlelist.clear();
  for(int i = 0 ; i <= cd->ntracks; i++)
    tracktitlelist.append("");

  extlist.clear();
  for(int i = 0 ; i <= cd->ntracks; i++)
    extlist.append("");

  discidlist.clear();

  titlelabel->setText("CDDB query timed out.");
  artistlabel->setText("");
  titlelabeltimer->start(10000,TRUE); // 10 secs

  timer->start(1000);
  led_off();
  cddb_inexact_sentinel =false;
}

void KSCD::mycddb_inexact_read(){

  if(cddb_inexact_sentinel == true)
    return;

  cddb_inexact_sentinel = true;
  QStrList inexact_list;
  cddb->get_inexact_list(inexact_list);

  InexactDialog *dialog;
  dialog = new InexactDialog(0,"inexactDialog",true);
  dialog->insertList(inexact_list);

  if(dialog->exec() != QDialog::Accepted){
    cddb->close_connection();
    timer->start(1000);
    led_off();
    return;
  }

  QString pick;
  dialog->getSelection(pick);
  delete dialog;


  if(pick.isEmpty()){
    timer->start(1000);
    led_off();
    return;
  }

  pick = "200 " + pick;
  cddb->query_exact(pick);

}

void KSCD::cddb_done(){

  cddb_inexact_sentinel =false;  
  if(!cddb)
    return;

 cddb->getData(xmcd_data,tracktitlelist,extlist,category,discidlist,revision,playlist);
 playlistpointer = 0;

 if((int)tracktitlelist.count() != (cd->ntracks + 1)){
   if(debugflag) printf("WARNING tracktitleslist.coutn = %d != cd->ntracks +1 = %d\n",
	  tracktitlelist.count(),cd->ntracks + 1);
 }

 if((int)extlist.count() != (cd->ntracks + 1)){
   if (debugflag )printf("WARNING extlist.coutn = %d != cd->ntracks +1 = %d\n",
	  extlist.count(),cd->ntracks + 1);
 }

 if(tracktitlelist.count() > 1){
   titlelabel->setText(tracktitlelist.at(1));
   artistlabel->setText(tracktitlelist.at(0));
 }

 if(cddialog && updateDialog)
   cddialog->setData(cd,tracktitlelist,extlist,discidlist,xmcd_data,category,
		  revision,playlist,pathlist,mailcmd,submitaddress);

  led_off();
  timer->start(1000);

}

void KSCD::led_off(){

  queryledtimer->stop();
  queryled->off();
  queryled->hide();
  totaltimelabel->raise();
  totaltimelabel->show();

}

void KSCD::led_on(){

  totaltimelabel->hide();
  totaltimelabel->lower();
  queryledtimer->start(800);
  queryled->off();
  queryled->show();
  mykapp->processEvents();
  mykapp->flushX();

}

void KSCD::togglequeryled(){

  queryled->show();
  queryled->toggle();

}

void KSCD::titlelabeltimeout(){

  // clear the cddb error message on the title label.
  titlelabeltimer->stop();
  titlelabel->setText("");

}

void KSCD::playtime()
{
  static int mymin;
  static int mysec;
  int tmp = 0;
  
  switch(time_display_mode){

  case TRACK_REM:

    tmp = cur_tracklen - cur_pos_rel;
    mysec = tmp % 60;
    mymin = tmp / 60;
    break;

  case TOTAL_SEC:
    
    mysec = cur_pos_abs % 60;
    mymin = cur_pos_abs / 60;
    break;

  case TOTAL_REM:

    tmp = cur_cdlen - cur_pos_abs;
    mysec = tmp % 60;
    mymin = tmp / 60;

    break;
    
  case TRACK_SEC:
  default:

    if (cur_pos_rel > 0 && (tmp = cur_pos_rel % 60) == mysec)
      return;
    mysec = tmp;
    mymin = cur_pos_rel / 60;

    break;
  }
  	

  sprintf( tmptime, "%02d:%02d", mymin, mysec );
  return;

}

void KSCD::cycleplaytimemode(){

  cycletimer->stop();

  if (++time_display_mode > 3)
    time_display_mode = 0;
  playtime();
  setLEDs( tmptime );  


  switch(time_display_mode){

  case TRACK_REM:
    volumelabel->setText("Tra Rem");
    break;

  case TOTAL_SEC:
    volumelabel->setText("Tot Sec");
    break;

  case TOTAL_REM:
    volumelabel->setText("Tot Rem");
    break;
    
  case TRACK_SEC:
  default:
    volumelabel->setText("Tra Sec");
    break;
  }  

  cycletimer->start(3000,TRUE);

}

void KSCD::cycletimeout(){
  
  cycletimer->stop();
  QString str;
  str.sprintf("Vol: %02d%%",volume);
  volumelabel->setText(str.data());

}


bool  KSCD::getArtist(QString& artist){

  if((int)tracktitlelist.count()== 0){
    return false;
  }

  artist = tracktitlelist.at(0);

  int pos;
  pos = artist.find("/",0,true);
  if(pos != -1)
    artist = artist.left(pos);

  artist = artist.stripWhiteSpace();
  return true;

}

void  KSCD::performances(int i){

  if (debugflag )printf("preformances %d\n",i);

  QString artist;
  QString str;

  if(!getArtist(artist))
     return;

  // primitive incomplete http encoding TODO fix!
  artist = artist.replace( QRegExp(" "), "+" );

  switch(i){
  case 0:
      str = str.sprintf(
      "http://www.tourdates.com/cgi-bin/search.cgi?type=Artist&search=%s",
      artist.data());
      startBrowser(str.data());

    break;

  default:
    break;
  }

}

void  KSCD::purchases(int i){

  if (debugflag) printf("purchases %d\n",i);

  QString artist;
  QString str;

  if(!getArtist(artist))
     return;

  // primitive incomplete http encoding TODO fix!
  artist = artist.replace( QRegExp(" "), "+" );

  switch(i){
  case 0:
      str = str.sprintf(
      "http://cdnow.com/switch/from=sr-288025/target=buyweb_products/artfs=%s",
      artist.data());
      startBrowser(str.data());

    break;
  case 1:
      str = str.sprintf(
      "http://www.cduniverse.com/cgi-bin/cdubin.exe/rlinka/ean=%s",
      artist.data());
      startBrowser(str.data());

    break;

  default:
    break;
  }

}

void KSCD::information(int i){

  if (debugflag) printf("Information %d\n",i);

  QString artist;
  QString str;

  if(!getArtist(artist))
     return;

  // primitive incomplete http encoding TODO fix!
  artist = artist.replace( QRegExp(" "), "+" );

  switch(i){
    /*  case 0:
      str = str.sprintf(
"http://musiccentral.msn.com/MCGen/ReEntry?theurl=http%%3A//musiccentral.msn.com/Search/DoFullSearch%%3FArtists%3Don%%26Albums%3D%%26Songs%%3D%%26ArticleTitles%3D%%26FullText%3D%%26strInput%%3D%s",
      artist.data());
      startBrowser(str.data());
      printf("%s\n",str.data());

    break;*/
  case 0:
      str = str.sprintf(
      "http://www.ubl.com/find/form?SEARCH=%s",
      artist.data());
      startBrowser(str.data());

    break;


  case 2:
      str = str.sprintf(
"http://x8.dejanews.com/dnquery.xp?QRY=%s&defaultOp=AND&svcclass=dncurrent&maxhits=20&ST=QS&format=terse&DBS=2",

      artist.data());
      startBrowser(str.data());

    break;

  case 3:
      str = str.sprintf(
			"http://www.excite.com/search.gw?c=web&search=%s&trace=a",
      artist.data());
      startBrowser(str.data());

    break;

  case 4:
      str = str.sprintf(
"http://www.search.hotbot.com/hResult.html?SW=web&SM=MC&MT=%s&DC=10&DE=2&RG=NA&_v=2",
      artist.data());
      startBrowser(str.data());

    break;

  case 5:
      str = str.sprintf(
      "http://www.infoseek.com/Titles?qt=%s&col=WW&sv=IS&lk=ip-noframes&nh=10",
      artist.data());
      startBrowser(str.data());

    break;

  case 6:
      str = str.sprintf(
      "http://www.lycos.com/cgi-bin/pursuit?cat=lycos&query=%s",
      artist.data());
      startBrowser(str.data());

    break;

  case 7:
      str = str.sprintf(
      "http://www.mckinley.com/search.gw?search=%s&c=web&look=magellan",
      artist.data());
      startBrowser(str.data());

    break;

  case 8:
      str = str.sprintf(
			"http://search.yahoo.com/bin/search?p=%s",
      artist.data());
      startBrowser(str.data());

    break;

  default:
    break;
  }

}

void KSCD::startBrowser(char* querystring){

  if(use_kfm){

    KFM* kfm = new KFM();
    kfm->openURL( querystring );
    delete kfm;

  }
  else{

    char* shell = 0L;

    QString bw = browsercmd.copy();
    QString cmd;
    cmd = cmd.sprintf(bw.data(),querystring);

    if (!shell){
    if ( ( shell = getenv("SHELL") ) && *shell ) // make sure SHELL is not empty
      shell = qstrdup(getenv("SHELL"));
    else
      shell = "/bin/sh";
    }
   
    KProcess proc;
    proc.setExecutable(shell);
    proc << "-c" << cmd;
    proc.start(KProcess::DontCare);

  }
  
}

void KSCD::get_pathlist(QStrList& _pathlist){
  
   QDir d;
   QStrList list;

   d.setFilter( QDir::Dirs);
   d.setSorting( QDir::Size);
   d.setPath(cddbbasedir.data());

   _pathlist.clear();
   list = *d.entryList();

   for(uint i = 0; i < list.count(); i++){
     if(QString(list.at(i)) != QString (".") && QString(list.at(i)) != QString (".."))
       _pathlist.append( QString (cddbbasedir + "/" +  list.at(i)));

   }
}


int main( int argc, char *argv[] ){

  mykapp = new KApplication( argc, argv,"kscd" );
  k = new KSCD; 

  cur_track = 1;
  debugflag = false;

  for(int i = 0; i < argc; i++){

    if(strcmp(argv[i],"-d") == 0){
       debugflag = true;
       printf("DEBUG MODE ON\n");
    }

    if(strcmp(argv[i],"-h") == 0){
      printf("KSCD "KSCDVERSION\
	     " Copyright 1997-98 Bernd Johannes Wuebben wuebben@kde.org\n");
      printf("-h: display commandline options\n");
      printf("-d: enable debugging output.\n");
      exit(0);
    }
  }
    
  mykapp->enableSessionManagement(true);
  mykapp->setWmCommand(argv[0]);      
  mykapp->setTopWidget(k);
  mykapp->setMainWidget( k );
  k->show();

  return mykapp->exec();
}



void kcderror(char* title,char* message){

  QMessageBox::message(title,message);

}

#include "kscd.moc"

/*
void parseargs(char* buf, char** args){

  while(*buf != '\0'){
    
    // Strip whitespace. Use nulls, so that the previous argument is terminated 
    // automatically.
     
    while ((*buf == ' ' ) || (*buf == '\t' ) || (*buf == '\n' ) )
      *buf++ ='\0';
    
    // save the argument
    if(*buf != '\0')
    *args++ = buf;
    
    while ((*buf != '\0') && (*buf != '\n') && (*buf != '\t') && (*buf != ' '))
      buf++;
    
  }
 
  *args ='\0';;

}
*/
/* this is how you do real http encoding TODO rewrite for qstrings!!
STATIC void
cddb_http_xlat(char *s1, char *s2)
{
	char	*p,
		*q;

	for (p = s1, q = s2; *p != '\0'; p++) {
		switch (*p) {
		case '?':
		case '=':
		case '+':
		case '&':
		case ' ':
		case '%':
			(void) sprintf(q, "%%%02X", *p);
			q += 3;
			break;
		default:
			*q = *p;
			q++;
			break;
		}
	}
	*q = '\0';
}

*/
