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
#include <sys/shm.h>
#include <sys/wait.h>
#include <kmsgbox.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include "kmidclient.h"
#include <kurl.h>
#include <drag.h>
#include <kfontdialog.h>
#include "midicfgdlg.h"

kmidFrame::kmidFrame(const char *name)
	:KTopLevelWidget(name)
{
	kmidclient=new kmidClient(this,"KMidClient");
	kmidclient->songType(1);
	kmidclient->show();
	setView(kmidclient,FALSE);

	m_file = new QPopupMenu;
	m_file->insertItem( "&Open", this, SLOT(file_Open()), CTRL+Key_O );
	m_file->insertSeparator();
	m_file->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );
	m_song = new QPopupMenu;
	m_song->insertItem( "&Play", kmidclient, SLOT(song_Play()) , Key_Space);
	m_song->insertItem( "P&ause", this, SLOT(song_Pause()) );
	m_song->insertSeparator();
	m_song->insertItem( "&Stop", kmidclient, SLOT(song_Stop()) );

	m_options = new QPopupMenu;
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

	m_options->insertItem( "&Automatic Text chooser", this, SLOT(options_AutomaticText()) );
	m_options->setId(5,5);
	m_options->setItemChecked(5,TRUE);
	m_options->insertSeparator();
	m_options->insertItem( "&Font Change ...", this, SLOT(options_FontChange()));
	m_options->insertSeparator();
	m_options->insertItem( "&Midi Setup ...", this, SLOT(options_MidiSetup()));


	m_help = new QPopupMenu;
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

	m_options->setItemChecked(5,(kcfg->readNumEntry("AutomaticTextEventChooser",1)==1)?TRUE:FALSE);

	KDNDDropZone * dropZone = new KDNDDropZone( this , DndURL);
	connect( dropZone, SIGNAL( dropAction( KDNDDropZone *) ),
		this, SLOT( slotDropEvent( KDNDDropZone *) ) );
/*
	if (kapp->isRestored())
	    {
	    if (KTopLevelWidget::canBeRestored(1)) restore(1);
	    }
*/

        if ((kapp->argc()>1)&& (kapp->argv()[1][0]!='-') )
            {
	    kmidclient->openURL((kapp->argv())[1]);
	    if ((kcfg->readNumEntry("AutomaticTextEventChooser",1))==1)
	      {
	      if (kmidclient->ChooseTypeOfTextEvents()==1)
	            options_Text();
	          else
	            options_Lyrics();
	      }
            kmidclient->song_Play();
            };
};

kmidFrame::~kmidFrame()
{
delete kmidclient;
delete m_file;
delete m_song;
delete m_options;
delete m_help;
delete menu;
delete toolbar;
};

void kmidFrame::file_Open()
{
kmidclient->file_Open();

KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
if ((kcfg->readNumEntry("AutomaticTextEventChooser",1))==1)
    {
    if (kmidclient->ChooseTypeOfTextEvents()==1)
	  options_Text();
	else
	  options_Lyrics();  
    }
};

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

void kmidFrame::options_AutomaticText()
{
printf("Automatic Text chooser\n");
KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
int automatic=kcfg->readNumEntry("AutomaticTextEventChooser",1);
automatic=(automatic==1) ? 0 : 1; 
kcfg->writeEntry("AutomaticTextEventChooser",automatic);
m_options->setItemChecked(5,(automatic==1) ? TRUE : FALSE);
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
  if (kmidclient->openURL(s)==-1)
	{
	return;
	};
  KConfig *kcfg=KApplication::getKApplication()->getConfig();
  kcfg->setGroup("KMid");
  if ((kcfg->readNumEntry("AutomaticTextEventChooser",1))==1)
      {
      if (kmidclient->ChooseTypeOfTextEvents()==1)
            options_Text();
          else
            options_Lyrics();
      }
  kmidclient->song_Play();
};  

void kmidFrame::saveProperties(KConfig *kcfg)
{
int play;
if ((play=kmidclient->isPlaying())==1) kmidclient->song_Stop();
kcfg->writeEntry("File",kmidclient->midiFileName());
kcfg->writeEntry("Playing",play);
};

void kmidFrame::readProperties(KConfig *kcfg)
{
char *c=new char [strlen((const char *)kcfg->readEntry("File",NULL))+1];
strcpy(c,(const char *)kcfg->readEntry("File",NULL));
kmidclient->openURL(c);

KConfig *kconfig=KApplication::getKApplication()->getConfig();
kconfig->setGroup("KMid");
if ((kconfig->readNumEntry("AutomaticTextEventChooser",1))==1)
    {
    if (kmidclient->ChooseTypeOfTextEvents()==1)
	  options_Text();
	else
	  options_Lyrics();  
    }

if (kcfg->readNumEntry("Playing",0)==1) kmidclient->song_Play();
};

void kmidFrame::options_MidiSetup()
{
printf("MidiSetup\n");
if (kmidclient->devman()->checkInit()<0) return;
MidiConfigDialog *dlg;

dlg=new MidiConfigDialog(kmidclient->devman(),NULL,"MidiDialog");
if (dlg->exec() == QDialog::Accepted) 
    {
    printf("Accept\n");
    printf("Device selected : %d\n",MidiConfigDialog::selecteddevice);
    KConfig *kcfg=KApplication::getKApplication()->getConfig();
    kcfg->setGroup("KMid");
    kcfg->writeEntry("MidiPortNumber",MidiConfigDialog::selecteddevice);
    kmidclient->setMidiDevice(MidiConfigDialog::selecteddevice);
//    if (MidiConfigDialog::selectedmap!=NULL)
//	{
	printf("Midi map : %s\n",MidiConfigDialog::selectedmap);
	kcfg->setGroup("Midimapper"); 
        kcfg->writeEntry("LoadFile",(const char*)
	(MidiConfigDialog::selectedmap==NULL)? "":MidiConfigDialog::selectedmap);
	kmidclient->setMidiMapFilename(MidiConfigDialog::selectedmap);
//	};
    }
 else printf("Reject\n");
};

/*
void kmidFrame::closeEvent(QCloseEvent *e)
{
e->accept();
};
*/
