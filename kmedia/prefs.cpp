
#include "prefs.h"
#include "prefs.moc"
#include "kmedia.h"

Preferences::Preferences( QWidget *parent ) :
   QDialog( parent )
{
  int tabwidth = 300, tabheight = 200;

   tabctl = new KTabCtl( this );
   
   page1 = new QWidget( tabctl );
   page2 = new QWidget( tabctl );
   tabctl->addTab( page1, "Players" );
   tabctl->addTab( page2, "Options" );
   tabctl->setGeometry( 10, 10, tabwidth, tabheight );

   int x=10,y=10;
   // Define page 1
   QLabel    *mixerLB = new QLabel("Mixer",page1);
   mixerLB->setGeometry( 10, y, mixerLB->width(), mixerLB->height() );
   x += mixerLB->width()+4;

   mixerLE = new QLineEdit(page1,"Mixer");
   QFontMetrics qfm = fontMetrics();
   mixerLE->setGeometry(x,y, tabctl->width()-x-10, mixerLE->height() );

   // Define page 2
   QButtonGroup *grpbox1 = new QButtonGroup( "Play in", page2 );
   grpbox1->setGeometry( 10, 10, tabctl->width()/2-15, 100 );
   QRadioButton *rb2_1 = new QRadioButton( "Stereo", grpbox1 );
   rb2_1->move( 10, 15 );   
   QRadioButton *rb2_2 = new QRadioButton( "Mono", grpbox1 );
   rb2_2->move( 10, 40 );

   QButtonGroup *grpbox2 = new QButtonGroup( "Quality", page2 );
   grpbox2->setGeometry( tabctl->width()/2+5, 10, tabctl->width()/2-15, 100 );
   QRadioButton *rb2_3 = new QRadioButton( "Best", grpbox2 );
   rb2_3->move( 10, 15 );   
   QRadioButton *rb2_4 = new QRadioButton( "Medium", grpbox2 );
   rb2_4->move( 10, 40 );
   QRadioButton *rb2_5 = new QRadioButton( "Low", grpbox2 );
   rb2_5->move( 10, 65 );
   QLabel *lbl2_1 = new QLabel( "NOT IMPLEMENTED YET!!!",
				page2 );
   lbl2_1->setAlignment( AlignLeft );
   lbl2_1->adjustSize();
   lbl2_1->move( (tabctl->width()-lbl2_1->width())/2, 120 );
   

   buttonOk = new QPushButton( "Ok", this );
   buttonOk->setGeometry( tabwidth-250, tabheight+20, 80, 25 );
   connect( buttonOk, SIGNAL(clicked()), this, SLOT(slotOk()));

   buttonApply = new QPushButton( "Apply", this );
   buttonApply->setGeometry( tabwidth-160, tabheight+20, 80, 25 );
   connect( buttonApply, SIGNAL(clicked()), this, SLOT(slotApply()));

   buttonCancel = new QPushButton( "Cancel", this );
   buttonCancel->setGeometry( tabwidth-70, tabheight+20, 80, 25 );
   connect( buttonCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
   
   setFixedSize( tabwidth + 20, tabheight + 55 );
   setCaption( "KMedia Preferences" );
}

void Preferences::slotShow()
{
   show();
}

void Preferences::slotOk()
{
  slotApply();
  hide();
}

void Preferences::slotApply()
{
  mixerCommand = mixerLE->text();
  kcfg->writeEntry( "MixerCommand", mixerCommand, true );
  kcfg->sync();
  emit optionsApply();
}

void Preferences::slotCancel()
{
  hide();
}
