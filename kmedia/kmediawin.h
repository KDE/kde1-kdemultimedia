//-*-C++-*-
#ifndef KMEDIAWIN_H
#define KMEDIAWIN_H

const int	WIDTH  = 25;
const int	HEIGHT = 30;

#include <qpushbt.h>
#include <qlabel.h>
#include <qpopmenu.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktopwidget.h>
#include "kslider.h"
#include <kapp.h>

#include "prefs.h"
extern "C" {
#include <mediatool.h>
}
enum { NOP, FF, REW, NEXTTRACK, PREVTRACK };


class KMediaWin : public KTopLevelWidget
{
  Q_OBJECT
public:
  KMediaWin(QWidget *parent, const char* name);

  MdCh_STAT	*StatChunk;
  uint32	*StatStatPtr;
  void resizeEvent(QResizeEvent *);


  void		initMediatool();
  void		dtfatal(char *text);


  int32		InitialDelay;
  int32		RepeatSpeed;
  MediaCon	m;
  bool		MaudioLaunched;


  void		baseinitMediatool();
  void		launchPlayer(char *filename);
  bool		removePlayer();

  // Mediatool
  MdPlaylist	*Playlist;
  int32		key;
  int32		Position;

  // Mediatool chunks
  MdCh_KEYS	*KeysChunk;
  MdCh_FNAM	*FnamChunk;
  MdCh_IHDR	*IhdrChunk;

  int		TimerAction;
  Preferences	*prefDL;
  KConfig	*KmConfig;

signals:
  void		lastWindowClosed();

public slots:
  void		playClicked();
  void		stopClicked();
  void		prevClicked();
  void		prevReleased();
  void		nextClicked();
  void		nextReleased();
  void		fwdClicked();
  void		fwdReleased();
  void		bwdClicked();
  void		bwdReleased();
  void		quitClicked();
  void		TimerFunctions();
  void		ejectClicked();
  void		trackSelected( int );
  void		launchHelp();
  void		launchMixer();
  void		hideMenu();
  void		PosChanged( int new_pos ); 
  void		aboutClicked();
  void		aboutqt();
  void		newviewClicked();
  void		showOpts();
  void		SlaveStatusQuery();

  void		quit();
  void		quitLW();
  void		closeEvent( QCloseEvent * );
  void		onDrop( KDNDDropZone* _zone );

private:
  QTimer	*timer;		// Timer to update the time
  QTimer	*timerFn;	// Timer based functions
  uint32	status_old;
  uint32	status, supp_keys_now;
  uint32	pos_old, max_old;
  QPixmap	playpauseBmp, playpause_playBmp, playpause_bothBmp;
  int32		SuppKeysOld;
  const char   	*modetext;
  char		fnameOld[LEN_FNAME];

  QPushButton	*makeButton( int, int, int, int, const char *, const char * );
  void		createPanel();
  void		createMenu();
  QPushButton   *createButton( int x, int y, int w, int h, const char *name, const char *TT );
  void		loadBitmaps();
  QWidget	*Container;	// Container for everything
  QPushButton	*playPB;	// Play/Pause button
  QPushButton	*stopPB;	// Stop button
  QPushButton	*prevPB;	// Previous track
  QPushButton	*nextPB;	// Next track
  QPushButton	*fwdPB;		// "Fast" forward
  QPushButton	*bwdPB;		// "Fast" rewind
  QPushButton	*quitPB;	// Quit button
  QPushButton	*ejectPB;	// Eject button

  QPopupMenu	*Mfile;
  QPopupMenu	*Moptions;
  QPopupMenu	*Mhelp;
  KMenuBar	*mainmenu;
  KStatusBar	*statbar;
  QLabel	*label;
  KSlider	*PosSB;
  QLabel	*playstat;
  QLabel	*mediatitle;
  KDNDDropZone	*dropZone;
};

#endif
