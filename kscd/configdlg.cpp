
/*
 *
 * kscd -- A simple CD player for the KDE project
 *
 * $Id$
 * 
 * Copyright (C) 1997 Bernd Johannes Wuebben 
 * wuebben@math.cornell.edu
 *
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */



#include "configdlg.h"


extern KApplication *mykapp;


ConfigDlg::ConfigDlg(QWidget *parent=0, struct configstruct *data = 0,const char *name=0)
  : QDialog(parent, name)
{

  configdata.background_color = black;
  configdata.led_color = green;
  configdata.tooltips = true;
  configdata.cd_device ="/dev/cdrom";
  configdata.cd_device ="mail -s \"%s\" ";
  configdata.docking = true;

  if(data){
    configdata.background_color = data->background_color;
    configdata.led_color = data->led_color;
    configdata.tooltips = data->tooltips;
    configdata.cd_device = data->cd_device;
    configdata.mailcmd = data->mailcmd;
    configdata.browsercmd = data->browsercmd;
    configdata.use_kfm = data->use_kfm;
    configdata.docking = data->docking;
  }

  colors_changed = false;

  setCaption(klocale->translate("Configure kscd"));

  box = new QGroupBox(this, "box");
  box->setGeometry(10,10,365,360);


  label1 = new QLabel(this);
  label1->setGeometry(20,25,135,25);
  label1->setText("LED Color:");

  qframe1 = new QFrame(this);
  qframe1->setGeometry(155,25,30,25);	
  qframe1->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe1->setBackgroundColor(configdata.led_color);

  button1 = new QPushButton(this);
  button1->setGeometry(255,25,100,25);
  button1->setText(klocale->translate("Change"));
  connect(button1,SIGNAL(clicked()),this,SLOT(set_led_color()));

  label2 = new QLabel(this);
  label2->setGeometry(20,65,135,25);
  label2->setText(klocale->translate("Background Color:"));

  qframe2 = new QFrame(this);
  qframe2->setGeometry(155,65,30,25);	
  qframe2->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe2->setBackgroundColor(configdata.background_color);

  button2 = new QPushButton(this);
  button2->setGeometry(255,65,100,25);
  button2->setText(klocale->translate("Change"));
  connect(button2,SIGNAL(clicked()),this,SLOT(set_background_color()));

  button3 = new QPushButton(this);
  button3->setGeometry(255,330,100,25);
  button3->setText(klocale->translate("Help"));
  connect(button3,SIGNAL(clicked()),this,SLOT(help()));

  label5 = new QLabel(this);
  label5->setGeometry(20,110,135,25);
  label5->setText(klocale->translate("CDROM Device:"));

  cd_device_edit = new QLineEdit(this);
  cd_device_edit->setGeometry(155,110,200,25);
  cd_device_edit->setText(configdata.cd_device);
  connect(cd_device_edit,SIGNAL(textChanged(const char*)),
	  this,SLOT(device_changed(const char*)));  

#if defined(sun) && defined(__sun__) && defined(__osf__) && defined(ultrix) && defined(__ultrix)

  label5->hide();
  cd_device_edit->hide();

#endif

  label6 = new QLabel(this);
  label6->setGeometry(20,150,135,25);
  label6->setText(klocale->translate("Unix mail command:"));

  mail_edit = new QLineEdit(this);
  mail_edit->setGeometry(155,150,200,25);
  mail_edit->setText(configdata.mailcmd);
  connect(mail_edit,SIGNAL(textChanged(const char*)),
	  this,SLOT(mail_changed(const char*)));  

  browserbox = new  QButtonGroup(klocale->translate("WWW-Browser"),this,"wwwbox");
  browserbox->setGeometry(20,190,338,120);

  kfmbutton = new QRadioButton(klocale->translate("Use kfm as Browser"),
			       browserbox,"kfmbutton");
  kfmbutton->move(10,20);
  kfmbutton->adjustSize();
  kfmbutton->setChecked(configdata.use_kfm);
  connect(kfmbutton,SIGNAL(clicked()),this,SLOT(kfmbutton_clicked()));

  custombutton = new QRadioButton(klocale->translate("Use Custom Browser:"),
				  browserbox,"custombutton");
  custombutton->move(10,50);
  custombutton->adjustSize();
  custombutton->setChecked(!configdata.use_kfm);
  connect(custombutton,SIGNAL(clicked()),this,SLOT(custombutton_clicked()));

  custom_edit = new QLineEdit(browserbox,"customedit");
  custom_edit->setText(data->browsercmd);
  custom_edit->setEnabled(!configdata.use_kfm);
  custom_edit->setGeometry(30,80,198,28);

  ttcheckbox = new QCheckBox(klocale->translate("Show Tool Tips"), 
			     this, "tooltipscheckbox");
  ttcheckbox->setGeometry(30,315,135,20);
  ttcheckbox->setChecked(configdata.tooltips);
  connect(ttcheckbox,SIGNAL(clicked()),this,SLOT(ttclicked()));

  dockcheckbox = new QCheckBox(klocale->translate("Enable KPanel Docking"), 
			       this, "dockcheckbox");
  dockcheckbox->setGeometry(30,340,160,20);
  dockcheckbox->setChecked(configdata.docking);
  connect(dockcheckbox,SIGNAL(clicked()),this,SLOT(dockclicked()));
  
}


void ConfigDlg::custombutton_clicked(){

    configdata.use_kfm = false;
    custom_edit->setEnabled(!configdata.use_kfm);
}


void ConfigDlg::kfmbutton_clicked(){

    configdata.use_kfm = true;
    custom_edit->setEnabled(!configdata.use_kfm);
}


void ConfigDlg::okbutton() {



}

void ConfigDlg::device_changed(const char* dev) {

  configdata.cd_device = dev;
}

void ConfigDlg::mail_changed(const char* dev) {

  configdata.mailcmd = dev;
}

void ConfigDlg::ttclicked(){

  if(ttcheckbox->isChecked())
    configdata.tooltips = TRUE;
  else
    configdata.tooltips = FALSE;

}

void ConfigDlg::dockclicked(){

  if(dockcheckbox->isChecked())
    configdata.docking = TRUE;
  else
    configdata.docking = FALSE;

}
void ConfigDlg::help(){

  if(mykapp)
    mykapp->invokeHTMLHelp("kscd/kscd.html","");
}

void ConfigDlg::cancelbutton() {
  reject();
}

void ConfigDlg::set_led_color(){


  KColorDialog::getColor(configdata.led_color);
  qframe1->setBackgroundColor(configdata.led_color);


}

void ConfigDlg::set_background_color(){

  KColorDialog::getColor(configdata.background_color);
  qframe2->setBackgroundColor(configdata.background_color);

}

struct configstruct * ConfigDlg::getData(){

  configdata.browsercmd = custom_edit->text();
  return &configdata;

}

#include "configdlg.moc"



