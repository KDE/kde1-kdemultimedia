#include "midicfgdlg.moc"
#include <qpushbt.h>
#include <qlistbox.h>
#include <qlabel.h> 
#include <qfiledlg.h>
#include <kapp.h>
#include "player/deviceman.h"

MidiConfigDialog::MidiConfigDialog(DeviceManager *dm, QWidget *parent,const char *name) : QDialog(parent,name,TRUE)
{
setCaption("Configure Midi devices");
setMinimumSize(360,240);
setMaximumSize(360,240);
ok=new QPushButton("OK",this);
ok->setGeometry(140,200,100,30);
connect(ok,SIGNAL(clicked()),SLOT(accept()) );
cancel=new QPushButton("Cancel",this);
cancel->setGeometry(250,200,100,30);
connect(cancel,SIGNAL(clicked()),SLOT(reject()) );

label=new QLabel("Select the midi device you want to use :",this);
label->adjustSize();
label->move(10,10);
mididevices=new QListBox(this,"midideviceslist");
mididevices->setGeometry(10,30,340,60);
connect(mididevices,SIGNAL(highlighted(int)),SLOT(deviceselected(int)));
devman=dm;
char temp[200];
for (int i=0;i<devman->numberOfMidiPorts()+devman->numberOfSynthDevices();i++)
    {
    if (strcmp(devman->type(i),"")!=0)    
        sprintf(temp,"%s - %s",devman->name(i),devman->type(i));
       else
        sprintf(temp,"%s",devman->name(i));

    mididevices->insertItem(temp,i);
    };
mididevices->setCurrentItem(devman->getDefaultDevice());

label2=new QLabel("Use the midi map :",this);
label2->adjustSize();
label2->move(10,100);


if (selectedmap!=NULL) delete selectedmap;
if (strcmp(devman->getMidiMapFilename(),"")==0)
    {
    selectedmap=NULL;
    }
   else
    {
    selectedmap=new char[strlen(devman->getMidiMapFilename())+1];
    strcpy(selectedmap,devman->getMidiMapFilename());
    };

if (selectedmap!=NULL) maplabel=new QLabel(selectedmap,this);
   else maplabel=new QLabel("None",this);

maplabel->adjustSize();
maplabel->move(10,100+label2->height());

mapbrowse=new QPushButton("Browse...",this);
mapbrowse->adjustSize();
mapbrowse->move(170,140);
connect(mapbrowse,SIGNAL(clicked()),SLOT(browseMap()) );

mapnone=new QPushButton("None",this);
mapnone->adjustSize();
mapnone->move(170+mapbrowse->width()+10,140);
connect(mapnone,SIGNAL(clicked()),SLOT(noMap()) );

};

void MidiConfigDialog::deviceselected(int idx)
{
selecteddevice=idx;
};

void MidiConfigDialog::browseMap()
{
QString filename;
char *path;
path=new char[strlen(KApplication::kde_datadir())+20];
sprintf(path,"%s/kmid/maps",(const char *)KApplication::kde_datadir());
filename=QFileDialog::getOpenFileName((const char *)path,"*.map",this); 
if (filename.isNull()) return;

if (selectedmap) delete selectedmap;
selectedmap=new char[strlen(filename)+1];
strcpy(selectedmap,filename);
maplabel->setText(selectedmap);
maplabel->adjustSize();

};

void MidiConfigDialog::noMap()
{
if (selectedmap!=NULL) {delete selectedmap;selectedmap=NULL;}; 
maplabel->setText("None");
maplabel->adjustSize();
};

int MidiConfigDialog::selecteddevice=0;
char *MidiConfigDialog::selectedmap=NULL;
