#include <string.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kmediawin.h"
#include "kmediawin.moc"



// Pics
#include "pics/stop.xbm"
#include "pics/nexttrk.xbm"
#include "pics/prevtrk.xbm"
#include "pics/ff.xbm"
#include "pics/rew.xbm"
#include "pics/eject.xbm"
#include "pics/logo.xbm"


KIconLoader	*globalKIL;		// KIconLoader should be global
extern KApplication	*globalKapp;


KMediaWin::~KMediaWin()
{
  bool exitOK = removePlayer();
  if (!exitOK)
    fprintf(stderr,"Player does not quit!!!\n");

  delete mainmenu;
  delete statbar;
}


KMediaWin::KMediaWin(QWidget * /*parent*/, const char* /*name*/) // :   QWidget(parent,name)
{
  TimerAction = NOP;
  MaudioLaunched= false;

  InitialDelay = -1;
  RepeatSpeed  =  1;

  timer = new QTimer( this );
  timerFn = new QTimer( this );

  SuppKeysOld = ~0;
  status_old=~0;
  pos_old = max_old = 0;

  createPanel();
  loadBitmaps();

  fnameOld[0]=0;

  initMediatool(); // Init after kmw

  KmConfig=KApplication::getKApplication()->getConfig();
  prefDL = new Preferences(NULL);
  prefDL->kcfg = KApplication::getKApplication()->getConfig();
  prefDL->mixerCommand = KmConfig->readEntry( "MixerCommand", "kmix" );
  prefDL->mixerLE->setText(prefDL->mixerCommand);

  connect( timer, 	SIGNAL(timeout()), 	SLOT(SlaveStatusQuery()) );
  connect( timerFn,	SIGNAL(timeout()),	SLOT(TimerFunctions()) );
  connect( this, SIGNAL(lastWindowClosed()), SLOT(quitLW()));

  timer->start( 50 );
  timerFn->start( 100 );
  globalKapp->setMainWidget( this );
}



/******************************************************************************
 *
 * Function:	KMEDIA::createButton()
 *
 * Task:	Create a push button and place it. Set its name and an
 *		optional tooltip text
 *
 * in:		x,y,w,h		Position and size
 *		name		Nam of button
 *		TT		Tool-Tip text. If NULL, no tooltip is used.
 * 
 * out:		QPushButton	Created push button
 *
 *****************************************************************************/
QPushButton *KMediaWin::createButton( int x, int y, int w, int h, const char *name, const char *TT ) 
{
  QPushButton *pb = new QPushButton( Container,name );
  pb->setGeometry( x, y, w, h );
  //pb->setLineWidth( 1 );
  if (TT)
    QToolTip::add( pb, TT );
  return pb;
}

		
// Widgets have to be placed by manually......
void KMediaWin::createPanel() 
{
  // Init DnD: Set up drop zone and drop handler
  dropZone = new KDNDDropZone( this, DndURL );
  connect( dropZone, SIGNAL( dropAction( KDNDDropZone* )), 
	   SLOT( onDrop( KDNDDropZone*)));


  // Setup/Create the Menubar
  createMenu();
  mainmenu->setMenuBarPos(KMenuBar::Top);
  setMenu(mainmenu);


  // Setup/Create the status bar
  statbar = new KStatusBar(this);
  statbar->insertItem( "xxxxxxxxxxxxxx", 1 );
  statbar->insertItem( "", 2 );
  setStatusBar(statbar);


  // Create a big container containing every widget of this toplevel
  Container  = new QWidget(this);
  int ix=0,iy=0;
  int w = 7*WIDTH;

  // Create Slider (Position indicator)
  PosSB = new KSlider( 0, 0, 1, 5, KSlider::Horizontal, Container, "mediaslider" );
  QSize PosSBsize = PosSB->sizeHint();
  PosSB->setGeometry( ix, iy, w, PosSBsize.height() +4);
  //  PosSB->setTracking(false);
  //  PosSB->setEnabled(false);
  connect( PosSB, SIGNAL(valueChanged(int)), SLOT(PosChanged(int)));
  QToolTip::add( PosSB,	klocale->translate("Playback position" ));


  // Now setup the buttons
  ix  = 0;
  iy += PosSB->height();
  ejectPB = createButton( ix, iy, WIDTH, HEIGHT, "Eject", klocale->translate("Eject medium") );
  connect( ejectPB, SIGNAL(clicked()), SLOT(ejectClicked()) );

  ix += WIDTH;
  prevPB = createButton( ix, iy, WIDTH, HEIGHT, "Prev", klocale->translate("Previous track") );
  connect( prevPB, 	SIGNAL(pressed()), 	SLOT(prevClicked()) );
  connect( prevPB, 	SIGNAL(released()), 	SLOT(prevReleased()) );

  ix += WIDTH;
  bwdPB = createButton( ix, iy, WIDTH, HEIGHT, "Bwd", klocale->translate("Backward") );
  connect( bwdPB, 	SIGNAL(pressed()), 	SLOT(bwdClicked()) );
  connect( bwdPB, 	SIGNAL(released()), 	SLOT(bwdReleased()) );

  pllPB = createButton (2000, iy,   2,      2, "Pll",                  ("KDE"));
  connect(  pllPB,      SIGNAL(clicked()),      SLOT(pllClicked()) );

  ix += WIDTH;
  stopPB = createButton( ix, iy, WIDTH, HEIGHT, "Stop" , klocale->translate("Stop") );
  connect( stopPB, 	SIGNAL(clicked()), 	SLOT(stopClicked()) );

  ix += WIDTH;
  playPB = createButton( ix, iy, WIDTH, HEIGHT, "Play/Pause" , klocale->translate("Play/Pause") );
  connect( playPB, 	SIGNAL(clicked()), 	SLOT(playClicked()) );


  ix += WIDTH;
  fwdPB = createButton( ix, iy, WIDTH, HEIGHT, "Fwd" , klocale->translate("Forward" ));
  connect( fwdPB, 	SIGNAL(pressed()), 	SLOT(fwdClicked()) );
  connect( fwdPB, 	SIGNAL(released()), 	SLOT(fwdReleased()) );

  ix += WIDTH;
  nextPB = createButton( ix, iy, WIDTH, HEIGHT, "Next" , klocale->translate("Next track"));
  connect( nextPB, 	SIGNAL(pressed()), 	SLOT(nextClicked()) );
  connect( nextPB, 	SIGNAL(released()), 	SLOT(nextReleased()) );

  iy += HEIGHT;


  Container->setMinimumSize(w       , iy);
  Container->setMaximumSize(9999    , iy);
  // Hehe. Now fooling around for de. fi is perhaps still messy ;-)
  Container->resize        (w+WIDTH , iy );
  setView(Container);

  //setMinimumSize(w         , iy);
  //setMaximumSize(2000      , iy);
  resize        (w+WIDTH   , iy);

  show();
}


void KMediaWin::createMenu()
{

  QPopupMenu *Mfile = new QPopupMenu;
  CHECK_PTR( Mfile );
  Mfile->insertItem( klocale->translate("New view"),  this, SLOT(newviewClicked()), CTRL+Key_N );
  Mfile->insertItem( klocale->translate("Exit")    ,  this, SLOT(quitClicked())   , CTRL+Key_Q  );

  QPopupMenu *Moptions = new QPopupMenu;
  CHECK_PTR( Moptions );
  //  Moptions->insertItem( klocale->translate("Hide Menu"), this, SLOT(hideMenu()));
  Moptions->insertItem( klocale->translate("Mixer"), this, SLOT(launchMixer()));
  Moptions->insertSeparator();
  Moptions->insertItem( klocale->translate("Preferences"), this, SLOT(showOpts()));

  QPopupMenu *Mhelp = new QPopupMenu;
  CHECK_PTR( Mhelp );
  Mhelp->insertItem( klocale->translate("&Contents"), this, SLOT(launchHelp()), Key_F1);
  Mhelp->insertSeparator();
  Mhelp->insertItem( klocale->translate("&About"), this, SLOT(aboutClicked()));
  Mhelp->insertItem( klocale->translate("&About Qt..."), this, SLOT(aboutqt()));

  mainmenu = new KMenuBar( this /*Container*/ , "mainmenu" );
  CHECK_PTR( mainmenu );
  mainmenu->insertItem( klocale->translate("&File"), Mfile );
  mainmenu->insertItem( klocale->translate("&Options"), Moptions );
  mainmenu->insertSeparator();
  mainmenu->insertItem( klocale->translate("&Help"), Mhelp );
}




// Set up the pics for the buttons
void KMediaWin::loadBitmaps() {
  globalKIL  = globalKapp->getIconLoader();

  QBitmap stopBmp( stop_width, stop_height, stop_bits, TRUE );
  QBitmap prevBmp( prevtrk_width, prevtrk_height, prevtrk_bits, TRUE );
  QBitmap nextBmp( nexttrk_width, nexttrk_height, nexttrk_bits, TRUE );
  QBitmap fwdBmp( ff_width, ff_height, ff_bits, TRUE );
  QBitmap bwdBmp( rew_width, rew_height, rew_bits, TRUE );
  QBitmap ejectBmp( eject_width, eject_height, eject_bits, TRUE );
  QBitmap aboutBmp( logo_width, logo_height, logo_bits, TRUE );

  playpauseBmp      = globalKIL->loadIcon( "playpause.xbm" );
  playpause_playBmp = globalKIL->loadIcon( "playpause_play.xpm" );
  playpause_bothBmp = globalKIL->loadIcon( "playpause_both.xpm" );
 
  playPB->setPixmap( playpauseBmp );
  stopPB->setPixmap( stopBmp );
  prevPB->setPixmap( prevBmp );
  nextPB->setPixmap( nextBmp );
  fwdPB->setPixmap( fwdBmp );
  bwdPB->setPixmap( bwdBmp );
  ejectPB->setPixmap( ejectBmp );
}


void KMediaWin::resizeEvent(QResizeEvent *e)
{
  KTopLevelWidget::resizeEvent(e);
  PosSB->resize     ( width(), PosSB->height());
}






// It runs every 1/10 sec checking the status of the media slave and 
// updates the appropriate data.
void KMediaWin::SlaveStatusQuery()
{

  static const char   *modetextExited = NULL, *modetextInit = NULL, *modetextBusy= NULL, \
    *modetextPlaying= NULL, *modetextPausing= NULL, *modetextReady= NULL, \
    *modetextUnknown=NULL;

  // This is for NLS support. Look up texts only once
  if (!modetextExited)	modetextExited=klocale->translate("Exited");
  if (!modetextInit)	modetextInit=klocale->translate("Init");
  if (!modetextBusy)	modetextBusy=klocale->translate("Busy");
  if (!modetextPlaying)	modetextPlaying=klocale->translate("Playing");
  if (!modetextPausing)	modetextPausing=klocale->translate("Paused");
  if (!modetextReady)	modetextReady=klocale->translate("Ready");
  if (!modetextUnknown)	modetextUnknown=klocale->translate("Undefined");

  status = *StatStatPtr;

  if (status != status_old)
    {
      if ( status & MD_STAT_EXITED )
	{
	   modetext = modetextExited;
	   playPB->setPixmap( playpauseBmp );
	}
      else if ( status & MD_STAT_INIT )
	{
	   modetext = modetextInit;
           playPB->setPixmap( playpauseBmp );
        }
      else if ( status & MD_STAT_BUSY )
	{
	   modetext = modetextBusy;
           playPB->setPixmap( playpauseBmp );
        }
      else if ( status & MD_STAT_PLAYING )
	{
	   modetext = modetextPlaying;
           playPB->setPixmap( playpause_playBmp );
        }
      else if ( status & MD_STAT_READY )
	{
	   modetext = modetextReady;
           playPB->setPixmap( playpauseBmp );
        }
      else if ( status & MD_STAT_PAUSING )
	{
	   modetext = modetextPausing;
           playPB->setPixmap( playpause_bothBmp );
        }
      else modetext = modetextUnknown;

      /* Display changes in mode now. */
      statbar->changeItem((char*)modetext,1);
      playPB->show();

      status_old = status;
    }

  /* Update slider range and position */
  if ( StatChunk->pos_current != pos_old || StatChunk->pos_max != max_old )
    {
      PosSB->setRange(0,StatChunk->pos_max);
      PosSB->setValue(StatChunk->pos_current);
      pos_old = StatChunk->pos_current;
      max_old = StatChunk->pos_max;
    }

  /* Update status field, which shows the status of the slave */
  status = *StatStatPtr;
  if( ( status && MD_STAT_PLAYING )
    && (strcmp (fnameOld, StatChunk->songname) != 0) )
    {
      strcpy (fnameOld, StatChunk->songname);
      statbar->changeItem(StatChunk->songname,2);
    }

  /* Update key field. (De)activate keys, reflecting the status of the 
   * supp_keys field
   */
  if ( SuppKeysOld != StatChunk->supp_keys )
    {
      SuppKeysOld = supp_keys_now = StatChunk->supp_keys;
      /* supp_keys field has changed. */
      playPB->setEnabled  (supp_keys_now&MD_KEY_PLAY);
      stopPB->setEnabled  (supp_keys_now&MD_KEY_STOP);
      ejectPB->setEnabled (supp_keys_now&MD_KEY_EJECT);
      prevPB->setEnabled  (supp_keys_now&MD_KEY_PREVTRACK);
      nextPB->setEnabled  (supp_keys_now&MD_KEY_NEXTTRACK);
      fwdPB->setEnabled   (supp_keys_now&MD_KEY_FORWARD);
      bwdPB->setEnabled   (supp_keys_now&MD_KEY_BACKWARD);
      ejectPB->setEnabled (supp_keys_now&MD_KEY_EJECT);
    }
}
