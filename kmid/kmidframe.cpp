/**************************************************************************

    kmidframe.cpp  - The main widget of KMid
    Copyright (C) 1997  Antonio Larrosa Jimenez

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
#include "kmidframe.moc"
//#include "kmidframe.h"

//#include <kapp.h>
#include <qkeycode.h>
#include <stdio.h>
#include <qfiledlg.h>
#include <kapp.h>
#include <qstring.h>
#include <unistd.h>
//#include "player/midimapper.h"
//#include "player/track.h"
//#include "player/midispec.h"
#include <sys/soundcard.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <kmsgbox.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include "kmidclient.h"
#include <kurl.h>
#include <drag.h>
#include <kfontdialog.h>

kmidFrame::kmidFrame(const char *name)
	:KTopLevelWidget(name)
{
	kmidclient=new kmidClient(this,"KMidClient");
	kmidclient->songType(1);
	kmidclient->show();
	setView(kmidclient,FALSE);

/*	QPopupMenu */m_file = new QPopupMenu;
	m_file->insertItem( "&Open", kmidclient, SLOT(file_Open()), CTRL+Key_O );
	m_file->insertSeparator();
	m_file->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );
/*	QPopupMenu */m_song = new QPopupMenu;
	m_song->insertItem( "&Play", kmidclient, SLOT(song_Play()) , Key_Space);
	m_song->insertItem( "P&ause", this, SLOT(song_Pause()) );
	m_song->insertSeparator();
	m_song->insertItem( "&Stop", kmidclient, SLOT(song_Stop()) );

/*	QPopupMenu */m_options = new QPopupMenu;
	m_options->setCheckable(TRUE);
	m_options->insertItem( "&General Midi file", this, SLOT(options_GM()) );
	m_options->setId(0,0);
	m_options->setItemChecked(0,TRUE);
	m_options->insertItem( "&MT-32 file", this, SLOT(options_MT32()) );
	m_options->setId(1,1);
	m_options->setItemChecked(1,FALSE);
	m_options->insertSeparator();
	m_options->insertItem( "See &Text events", this, SLOT(options_Text()) );
	m_options->setId(3,3);
	m_options->setItemChecked(3,TRUE);
	m_options->insertItem( "See &Lyrics events", this, SLOT(options_Lyrics()) );
	m_options->setId(4,4);
	m_options->setItemChecked(4,FALSE);
	m_options->insertSeparator();
	m_options->insertItem( "&Font Change ...", this, SLOT(options_FontChange()));
/*	QPopupMenu */m_help = new QPopupMenu;
	m_help->insertItem( "Help", kmidclient, SLOT(help_Help()), Key_F1 );
	m_help->insertSeparator();
	m_help->insertItem( "About", kmidclient, SLOT(help_About()), CTRL+Key_H );

	menu = new KMenuBar(this);
	menu->insertItem("&File",m_file);
	menu->insertItem("&Song",m_song);
	menu->insertItem("&Options",m_options);
	menu->insertSeparator();
	menu->insertItem("&Help",m_help);
	menu->show();
	setMenu(menu);

	toolbar=new KToolBar(this);
	KIconLoader *loader = (KApplication::getKApplication())->getIconLoader(); 

	toolbar->insertButton(loader->loadIcon("kmid_prev.xpm"),1,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Previous song (Not Implemented)");
	toolbar->insertButton(loader->loadIcon("kmid_frewind.xpm"),2,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Rewind");
	toolbar->insertButton(loader->loadIcon("kmid_stop.xpm"),3,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Stop");
	toolbar->insertButton(loader->loadIcon("kmid_pause.xpm"),4,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Pause");
	toolbar->setToggle(4,TRUE);
	toolbar->insertButton(loader->loadIcon("kmid_play.xpm"),5,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Play");
	toolbar->insertButton(loader->loadIcon("kmid_fforward.xpm"),6,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Forward");
	toolbar->insertButton(loader->loadIcon("kmid_next.xpm"),7,SIGNAL(clicked(int)),this,SLOT(buttonClicked(int)),TRUE,"Next song (Not Implemented)");

//	toolbar->setFullWidth();
	toolbar->setBarPos(KToolBar::Top);
	toolbar->enable(KToolBar::Show);
	addToolBar(toolbar);

	KConfig *kcfg=(KApplication::getKApplication())->getConfig();
	kcfg->setGroup("KMid");
	if (kcfg->readNumEntry("TypeOfTextEvents",5)==5)
		options_Lyrics();
	    else
		options_Text();
	if (kcfg->readNumEntry("TypeOfMidiFile",0)==0)
		options_GM();
	    else
		options_MT32();
	

//	updateRects();
	KDNDDropZone * dropZone = new KDNDDropZone( this , DndURL);
	connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
		this, SLOT( slotDropEvent( KDNDDropZone *) ) );
};

kmidFrame::~kmidFrame()
{
delete m_file;
delete m_song;
delete m_options;
delete m_help;
delete kmidclient;
delete menu;
delete toolbar;
};

/*void kmidFrame::readConfig(KConfig *kconf)
{
kmidclient->readConfig(kconf);
};
*/
void kmidFrame::song_Pause()
{
toolbar->toggleButton(4);
kmidclient->song_Pause();
};

void kmidFrame::buttonClicked(int i)
{
switch (i)
    {
    case (1) : printf("This is not implemented yet !\n");break;
    case (2) : kmidclient->song_Rewind();break;
    case (3) : kmidclient->song_Stop();break;
    case (4) : kmidclient->song_Pause();break;
    case (5) : kmidclient->song_Play();break;
    case (6) : kmidclient->song_Forward();break;
    case (7) : printf("This is not implemented yet !\n");
//		kmidclient->timebarChange(4946);
//		kmidclient->timebarChange(191733);
		break;
    };


};

void kmidFrame::options_GM()
{
KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
kcfg->writeEntry("TypeOfMidiFile",0);
kmidclient->songType(1);
m_options->setItemChecked(0,TRUE);
m_options->setItemChecked(1,FALSE);
};
void kmidFrame::options_MT32()
{
KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
kcfg->writeEntry("TypeOfMidiFile",1);
kmidclient->songType(0);
m_options->setItemChecked(0,FALSE);
m_options->setItemChecked(1,TRUE);
};
void kmidFrame::options_Text()
{
KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
kcfg->writeEntry("TypeOfTextEvents",1);
kcfg->sync();
m_options->setItemChecked(3,TRUE);
m_options->setItemChecked(4,FALSE);
kmidclient->repaintText(1);
};
void kmidFrame::options_Lyrics()
{
KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
kcfg->writeEntry("TypeOfTextEvents",5);
kcfg->sync();
m_options->setItemChecked(3,FALSE);
m_options->setItemChecked(4,TRUE);
kmidclient->repaintText(5);
};
void kmidFrame::options_FontChange()
{
KFontDialog *kfd=new KFontDialog(this);
QFont font;
font=*kmidclient->getFont();
kfd->getFont(font);
delete kfd;
KConfig *config=(KApplication::getKApplication())->getConfig();
config->setGroup("KMid");
config->writeEntry("KaraokeFont",font);
config->sync();
kmidclient->fontChanged();
};

void kmidFrame::slotDropEvent( KDNDDropZone * _dropZone )
{
  QStrList & list = _dropZone->getURLList();

  char *s=list.first();
  kmidclient->openURL(s);
/*  KURL *kurldropped=new KURL(s);
  if (kurldropped->isMalformed()) return;
  if (strcmp(kurldropped->protocol(),"file")!=0) return;
 
  kmidclient->openFile(kurldropped->path()); 
  delete kurldropped;*/
};  



