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

#include "kscd.h"
#include "configdlg.h"
#include "version.h"
#include "config.h"

#include "bitmaps/playpaus.xbm"
#include "bitmaps/stop.xbm"
#include "bitmaps/repeat.xbm"
#include "bitmaps/nexttrk.xbm"
#include "bitmaps/prevtrk.xbm"
#include "bitmaps/ff.xbm"
#include "bitmaps/rew.xbm"
#include "bitmaps/poweroff.xbm"
#include "bitmaps/eject.xbm"
#include "bitmaps/logo.xbm"
#include "bitmaps/shuffle.xbm"
#include "bitmaps/options.xbm"

KApplication 	*mykapp;
char 		tmptime[100];
char 		*tottime;
static void 	playtime (void);
void 		kcderror(char* title,char* message);
void 		kcdworker(int );


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

extern int	cur_track;	/* Current track number, starting at 1 */
extern int	cur_index;	/* Current index mark */
extern int	cur_lasttrack ;	/* Last track to play in current chunk */
extern int	cur_firsttrack;	/* First track of current chunk */
int	cur_pos_abs;	/* Current absolute position in seconds */
int	cur_frame;	/* Current frame number */
int	cur_pos_rel;	/* Current track-relative position in seconds */
int	cur_tracklen;	/* Length in seconds of current track */
int	cur_cdlen;	/* Length in seconds of entire CD */
int	cur_ntracks;	/* Number of tracks on CD (= tracks + sections) */
int	cur_nsections;	/* Number of sections currently defined */
extern  enum cd_modes	cur_cdmode;
int	cur_listno;	/* Current index into the play list, if playing */
char *	cur_artist;	/* Name of current CD's artist */
char *	cur_cdname;	/* Album name */
char *	cur_trackname;	/* Take a guess */
char	cur_contd;	/* Continued flag */
char	cur_avoid;	/* Avoid flag */

int cur_balance;
int info_modified;
int cur_stopmode;
int cur_playnew;
int mark_a, mark_b;

/****************************************************************************
				The GUI part 
*****************************************************************************/

KSCD::KSCD( QWidget *parent, const char *name ) :
	QDialog( parent, name )
{
        cd_device_str = "";
        background_color = black;
	led_color = green;
	randomplay = false;
	looping = false;
	cddrive_is_ok = true;
	tooltips = true;

	drawPanel();
	loadBitmaps();
	setColors();
	setToolTips();

	timer = new QTimer( this );
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
	connect( aboutPB, SIGNAL(clicked()), SLOT(aboutClicked()));
	connect( optionsbutton, SIGNAL(clicked()), SLOT(aboutClicked()));
	connect( shufflebutton, SIGNAL(clicked()), SLOT(randomSelected()));

	connect(mykapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(setColors()));

	readSettings();
	setColors();
	initWorkMan();

	volstartup = TRUE;
	volSB->setValue(volume);

	initimer = new QTimer;
	srandom(time(0L));
	connect( initimer, SIGNAL(timeout()),this,  SLOT(initCDROM()) );
	initimer->start(500,TRUE);

}

	
// Initialize the variables only in WorkMan
void KSCD::initWorkMan() {

  mark_a= 0; 
  mark_b = 0;
  cur_balance = 10;
  fastin = FALSE;
  scmd = 0;
  tmppos = 0;
  save_track = 1;
  thiscd.trk = NULL;
  thiscd.lists = NULL;
  tottime = tmptime;

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

}  


QPushButton *KSCD::makeButton( int x, int y, int w, int h, const char *n ) 
{
	QPushButton *pb = new QPushButton( n, this );
	pb->setGeometry( x, y, w, h );
	return pb;
}
	
void KSCD::drawPanel() 
{
	int ix = 0;
	int iy = 0;
	const int WIDTH = 100;
	const int HEIGHT = 29;
	const int SBARWIDTH = 180; //140

	setCaption( "kscd" );
	aboutPB = makeButton( ix, iy, WIDTH, 2 * HEIGHT, "About" );

	ix = 0;
	iy += 2 * HEIGHT;
	ejectPB = makeButton( ix, iy, WIDTH, HEIGHT, "Eject" );

	iy += HEIGHT;
	quitPB = makeButton( ix, iy, WIDTH, HEIGHT, "Quit" );
	


	ix += WIDTH;
	iy = 0;

	backdrop = new QFrame(this);
	backdrop->setGeometry(ix,iy,SBARWIDTH -2, 2* HEIGHT + HEIGHT /2 -1);


	int D = 6;

	//ix += 2 * SBARWIDTH / 7;
	ix = WIDTH + 8;
	for (int u = 0; u<5;u++){
	  trackTimeLED[u] = new BW_LED_Number(this );	 
	  trackTimeLED[u]->setGeometry( ix  + u*20, iy+8 +D, 23 ,  7*HEIGHT/5 );
	  trackTimeLED[u]->setLEDoffColor(background_color);
	  trackTimeLED[u]->setLEDColor(led_color,background_color);
	}
	
	QString zeros("--:--");
	setLEDs(zeros);


	/*
	titlelabel = new QLabel(this);
	titlelabel->setGeometry(WIDTH + 10, iy + 50 , WIDTH + 60, 14);
	titlelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
        titlelabel->setAlignment( AlignLeft );
	titlelabel->setText("Sting / Ten Summoners");
	*/

	statuslabel = new QLabel(this);
	statuslabel->setGeometry(WIDTH + 120, iy + 7 +D, 50, 14);
	statuslabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
        statuslabel->setAlignment( AlignLeft );
	//	statuslabel->setText("Ready");

	volumelabel = new QLabel(this);
	volumelabel->setGeometry(WIDTH + 120, iy + 21+D, 50, 14);
	volumelabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
        volumelabel->setAlignment( AlignLeft );
	volumelabel->setText("Vol: --");

	tracklabel = new QLabel(this);
	tracklabel->setGeometry(WIDTH + 120, iy + 35 +D, 50, 14);
	tracklabel->setFont( QFont( "Helvetica", 10, QFont::Bold ) );
        tracklabel->setAlignment( AlignLeft );
	tracklabel->setText("Trk: --");
			      
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


	iy += HEIGHT/2  +1;


	optionsbutton = new QPushButton( this );
	optionsbutton->setGeometry( ix, iy, SBARWIDTH/4 +1 , HEIGHT );
	optionsbutton->setFont( QFont( "helvetica", 12 ) );

	ix += SBARWIDTH/4;
	shufflebutton = new QPushButton( this );
	shufflebutton->setGeometry( ix +1, iy, SBARWIDTH/4-1 , HEIGHT );
	shufflebutton->setFont( QFont( "helvetica", 12 ) );


	ix += SBARWIDTH/4;
	songListCB = new QComboBox( this );
	songListCB->setGeometry( ix, iy, SBARWIDTH/2, HEIGHT );
	songListCB->setFont( QFont( "helvetica", 12 ) );

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
	QBitmap playBmp( playpause_width, playpause_height, playpause_bits,
			 TRUE );
	QBitmap stopBmp( stop_width, stop_height, stop_bits, TRUE );
	QBitmap prevBmp( prevtrk_width, prevtrk_height, prevtrk_bits, TRUE );
	QBitmap nextBmp( nexttrk_width, nexttrk_height, nexttrk_bits, TRUE );
	QBitmap replayBmp( repeat_width, repeat_height, repeat_bits, TRUE );
	QBitmap fwdBmp( ff_width, ff_height, ff_bits, TRUE );
	QBitmap bwdBmp( rew_width, rew_height, rew_bits, TRUE );
	QBitmap ejectBmp( eject_width, eject_height, eject_bits, TRUE );
	QBitmap quitBmp( poweroff_width, poweroff_height, poweroff_bits, 
			 TRUE );

	QBitmap shuffleBmp( shuffle_width, shuffle_height, shuffle_bits, TRUE );
	/*	QBitmap databaseBmp( db_width, db_height, db_bits, TRUE );*/

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
	quitPB->setPixmap( quitBmp );
	aboutPB->setPixmap( aboutBmp );
	shufflebutton->setPixmap( shuffleBmp );
	/*	databasebutton->setPixmap( databaseBmp );*/
	optionsbutton->setPixmap( optionsBmp );
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
	//	QToolTip::add( aboutPB, 	"Configure Kscd" );
	QToolTip::add( optionsbutton, 	"Configure Kscd" );
	QToolTip::add( ejectPB, 	"Eject CD" );
	QToolTip::add( shufflebutton, 	"Random Play" );
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
	//	QToolTip::remove( aboutPB );
	QToolTip::remove( optionsbutton );
	QToolTip::remove( ejectPB );
	QToolTip::remove( shufflebutton );
	QToolTip::remove( volSB );
  }

}

void KSCD::cleanUp() 
{
	if (thiscd.trk != NULL)
    		delete thiscd.trk;
  	signal (SIGINT, SIG_DFL);

}

void KSCD::playClicked()
{

        qApp->processEvents();
	qApp->flushX();

#ifdef NEW_BSD_PLAYCLICKED
	if (cur_cdmode == STOPPED || cur_cdmode == UNKNOWN  || cur_cdmode == BACK) {
#else
	if (cur_cdmode == STOPPED || cur_cdmode == UNKNOWN ) {
#endif

		int i;

		statuslabel->setText( "Playing" );
		songListCB->clear();
		setLEDs( "00:00" );
		for (i = 0; i < cur_ntracks; i++)
			songListCB->insertItem( 
			 QString( 0 ).sprintf( "Track %02d", i + 1 ) );
		qApp->processEvents();
		qApp->flushX();
		play_cd (save_track, 0, cur_ntracks + 1);
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

void KSCD::stopClicked()
{
        looping = FALSE;
	randomplay = FALSE;
        statuslabel->setText("Stopped");
        setLEDs("--:--");
        qApp->processEvents();
	qApp->flushX();

	save_track = cur_track = 1;
	stop_cd ();
}

void KSCD::prevClicked()
{
  //        statuslabel->setText("Previous");

        setLEDs("00:00");
        qApp->processEvents();
	qApp->flushX();

	cur_track--;
	if (cur_track < 1)
		cur_track = cur_ntracks;
	if(randomplay)
	  play_cd (cur_track, 0, cur_track + 1);
	else
	  play_cd (cur_track, 0, cur_ntracks + 1);
}

void KSCD::nextClicked()
{
  //    statuslabel->setText("Next");

        setLEDs("00:00");
        qApp->processEvents();
	qApp->flushX();

	if(randomplay){

	  int j = randomtrack();
	  QString str;
	  str.sprintf("Trk: %02d",j );
	  tracklabel->setText(str.data());
	  qApp->processEvents();
	  qApp->flushX();

	  play_cd( j, 0, j + 1 );

	}
	else{
	  if (cur_track == cur_ntracks)
	    cur_track = 0;
	  play_cd (cur_track + 1, 0, cur_ntracks + 1);
	}
}

void KSCD::fwdClicked()
{
  //        statuslabel->setText("Forward");


        qApp->processEvents();
	qApp->flushX();

	if (cur_cdmode == PLAYING) {
		tmppos = cur_pos_rel + 30;
		if (tmppos < thiscd.trk[cur_track - 1].length) {
		    if(randomplay)
		      play_cd (cur_track, tmppos, cur_track + 1);		  
		    else
		      play_cd (cur_track, tmppos, cur_ntracks + 1);
		}

	}
}

void KSCD::bwdClicked(){

  //    statuslabel->setText("Backward");
        qApp->processEvents();
	qApp->flushX();

	if (cur_cdmode == PLAYING) {
		tmppos = cur_pos_rel - 30;
		if(randomplay)
		play_cd (cur_track, tmppos > 0 ? tmppos : 0, cur_track + 1);
		else
		play_cd (cur_track, tmppos > 0 ? tmppos : 0, cur_ntracks + 1);

	}
	cdMode();
}

void KSCD::quitClicked()
{
	randomplay = FALSE;
        statuslabel->setText("");
        setLEDs( "--:--" );

        qApp->processEvents();
	qApp->flushX();

	stop_cd ();
	// calling cd_status() after stop_cd() before exiting
	//  keeps my cd_drive from locking up !!!
	cd_status();
	cd_status();
	cleanUp();
	writeSettings();
	qApp->quit();
}

void KSCD::closeEvent( QCloseEvent *e ){

  (void) e;

  writeSettings();
  cleanUp();
  qApp->quit();

};

void KSCD::loopClicked()
{

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

void KSCD::ejectClicked()
{
  if(!cddrive_is_ok)
    return;

  looping = FALSE;
  randomplay = FALSE;
  statuslabel->setText("Ejecting");
  qApp->processEvents();
  qApp->flushX();
  
  stop_cd();
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
    str.sprintf("Trk: %02d",j );
    tracklabel->setText(str.data());
    qApp->processEvents();
    qApp->flushX();

    play_cd( j, 0, j + 1 );

  }
}

void KSCD::trackSelected( int trk )
{
  randomplay = false;
  QString str;
  str.sprintf("Trk: %02d",trk + 1);
  tracklabel->setText(str.data());
  setLEDs("00:00");
  qApp->processEvents();
  qApp->flushX();
  
  cur_track = trk + 1;
  //  pause_cd();
  play_cd( cur_track, 0, cur_ntracks + 1 );
}

void KSCD::aboutClicked()
{

  QTabDialog * tabdialog;

  tabdialog = new QTabDialog(0,"tabdialog",TRUE);
  tabdialog->setCaption( "kscd Configuraton" );
  tabdialog->resize( 350, 350 );
  tabdialog->setCancelButton();

  QWidget *about = new QWidget(tabdialog,"about");

  QGroupBox *box = new QGroupBox(about,"box");
  QLabel  *label = new QLabel(box,"label");
  box->setGeometry(10,10,320,260);
  label->setGeometry(130,30,165,200);
  label->setAlignment( AlignCenter);
  QString labelstring;
  labelstring = "kscd "KSCDVERSION"\n"\
		 "Copyright (c) 1997\nBernd Johannes Wuebben\n"\
		 "wuebben@math.cornell.edu\n"\
		 "wuebben@kde.org\n\n"\
		 "\nkscd  contains code from:\n"
                 "workman 1.4 beta 3\n"
                 "Copyright (c) Steven Grimm \n"\
                 "koreth@hyperion.com\n"

;

  label->setAlignment(AlignLeft|WordBreak|ExpandTabs);
  label->setText(labelstring.data());
  
  QString pixdir = mykapp->kdedir() + QString("/share/apps/kscd/pics/"); 

  QPixmap pm((pixdir + "kscdlogo.xpm").data());
  QLabel *logo = new QLabel(box);
  logo->setPixmap(pm);
  logo->setGeometry(20, 50, pm.width(), pm.height());

  ConfigDlg* dlg;
  struct configstruct config;
  config.background_color = background_color;
  config.led_color = led_color;
  config.tooltips = tooltips;
  config.cd_device = cd_device;

  dlg = new ConfigDlg(tabdialog,&config,"configdialg");
 
  tabdialog->addTab(dlg,"Configure");
  tabdialog->addTab(about,"About");
  
  if(tabdialog->exec() == QDialog::Accepted){
    
    background_color = dlg->getData()->background_color;
    led_color = dlg->getData()->led_color;
    tooltips = dlg->getData()->tooltips;

    if( (QString)cd_device != dlg->getData()->cd_device){
      cd_device_str = dlg->getData()->cd_device;
      cd_close();
      cd_device = cd_device_str.data();
  }
    cddrive_is_ok = true;
      

    setColors();
    setToolTips();
  }

  delete dlg;
  delete about;
  delete tabdialog;
  

}


void KSCD::volChanged( int vol )
{
  if(volstartup)
    return;
  QString str;
  str.sprintf("Vol: %02d%%",vol);
  volumelabel->setText(str.data());
  cd_volume(vol, 10, 100); // 10 -> right == left balance
  volume = vol;

}

int KSCD::randomtrack(){

  int j;
  j=1+(int) (((double)cur_ntracks) *rand()/(RAND_MAX+1.0)); 
  return j;
}

void KSCD::cdMode()
{
	static char *p = new char[10];
	static bool damn = TRUE;
	QString str;

	sss = cd_status();
	//	printf("status %d\n",sss);
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
		  for (int i = 0; i < cur_ntracks; i++)

		    // we are in here when we start kscd and 
		    // the cdplayer is already playing.

                        songListCB->insertItem( QString( 0 ).sprintf( 
					    "Track %02d",i + 1 ) );
		  songListCB->setCurrentItem( cur_track - 1 );
		}
		else{
		  songListCB->setCurrentItem( cur_track - 1 );
		}
		str.sprintf("Trk: %02d",cur_track );
		tracklabel->setText(str.data());

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
			//			trackLED->display( "" );
			setLEDs( "--:--" );
			songListCB->clear();
			for (i = 0; i < cur_ntracks; i++)
                        songListCB->insertItem( 
			   QString( 0 ).sprintf( "Track %02d", i + 1 ) 
			   );
			str.sprintf("Trk: %02d", cur_track >= 0 ? cur_track: 1);
			tracklabel->setText(str.data());
		}
		damn = FALSE;
		break;
	case 5:         /* CDEJECT */
		statuslabel->setText( "Ejected" );
		songListCB->clear();
		setLEDs( "--:--" );	
		tracklabel->setText( "Trk: --" );
		damn = TRUE;
		break;
	}
}

void KSCD::setLEDs(QString symbols){

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
	//	titlelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
	volumelabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
	statuslabel->setPalette( QPalette(colgrp,colgrp,colgrp) );
	tracklabel->setPalette( QPalette(colgrp,colgrp,colgrp) );

	for (int u = 0; u< 5;u++){
	  trackTimeLED[u]->setLEDoffColor(background_color);
	  trackTimeLED[u]->setLEDColor(led_color,background_color);
	}

}


void KSCD::readSettings(){

	config = mykapp->getConfig();

	volume = config->readNumEntry("Volume",40);
	tooltips  = (bool) config->readNumEntry("ToolTips",1);
	randomplay = (bool) config->readEntry("RandomPlay", 0);
	cd_device_str = config->readEntry("CDDevice",DEFAULT_CD_DEVICE);
	cd_device = cd_device_str.data();

	QColor defaultback = black;
	QColor defaultled = QColor(226,224,255);
	background_color = config->readColorEntry("BackColor",&defaultback);	
	led_color = config->readColorEntry("LEDColor",&defaultled);


}

void KSCD::writeSettings(){
		
	config = mykapp->getConfig();
	
	///////////////////////////////////////////////////

	if(tooltips)
	  config->writeEntry("ToolTips", 1);
	else
	  config->writeEntry("ToolTips", 0);

	if(randomplay)
	  config->writeEntry("RandomPlay", 1);
	else
	  config->writeEntry("RandomPlay", 0);

	config->writeEntry("CDDevice",cd_device);
	config->writeEntry("Volume", volume);
	config->writeEntry("BackColor",background_color);
	config->writeEntry("LEDColor",led_color);
	config->sync();

	
}

int main ( int argc, char *argv[] )
{
	mykapp = new KApplication( argc, argv,"kscd" );
	KSCD 		*k = new KSCD; 

	cur_track = 1;
	mykapp->enableSessionManagement(true);
	mykapp->setWmCommand(argv[0]);      
	mykapp->setTopWidget(k);
	//	mykapp->setMainWidget( k );
	k->show();
	mykapp->exec();
}


void playtime()
{
  	static int mymin;
  	static int mysec;
  	int tmp = 0;

  	if (cur_pos_rel > 0 && (tmp = cur_pos_rel % 60) == mysec)
    		return;
  	mysec = tmp;
  	mymin = cur_pos_rel / 60;
	sprintf( tmptime, "%02d:%02d", mymin, mysec );

  	return;
}

void kcderror(char* title,char* message){

  QMessageBox::message(title,message);

}

/*
void kcdworker(int ){
  
  usleep(100000);
  printf("working\n");

}
*/

#include "kscd.moc"


