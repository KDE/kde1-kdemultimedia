
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
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
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
  
  if(data){
    configdata.background_color = data->background_color;
    configdata.led_color = data->led_color;
    configdata.tooltips = data->tooltips;
    configdata.cd_device = data->cd_device;
  }

  colors_changed = false;

  setCaption("Configure kscd");

  box = new QGroupBox(this, "box");
  box->setGeometry(10,10,320,260);


  label1 = new QLabel(this);
  label1->setGeometry(20,25,135,25);
  label1->setText("LED Color:");

  qframe1 = new QFrame(this);
  qframe1->setGeometry(155,25,30,25);	
  qframe1->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe1->setBackgroundColor(configdata.led_color);

  button1 = new QPushButton(this);
  button1->setGeometry(205,25,100,25);
  button1->setText("Change");
  connect(button1,SIGNAL(clicked()),this,SLOT(set_led_color()));

  label2 = new QLabel(this);
  label2->setGeometry(20,65,135,25);
  label2->setText("Background Color:");

  qframe2 = new QFrame(this);
  qframe2->setGeometry(155,65,30,25);	
  qframe2->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  qframe2->setBackgroundColor(configdata.background_color);

  button2 = new QPushButton(this);
  button2->setGeometry(205,65,100,25);
  button2->setText("Change");
  connect(button2,SIGNAL(clicked()),this,SLOT(set_background_color()));

  button3 = new QPushButton(this);
  button3->setGeometry(205,200,100,25);
  button3->setText("Help");
  connect(button3,SIGNAL(clicked()),this,SLOT(help()));

  label5 = new QLabel(this);
  label5->setGeometry(20,110,135,25);
  label5->setText("CDROM Device:");

  cd_device_edit = new QLineEdit(this);
  cd_device_edit->setGeometry(155,110,150,25);
  cd_device_edit->setText(configdata.cd_device);
  connect(cd_device_edit,SIGNAL(textChanged(const char*)),
	  this,SLOT(device_changed(const char*)));  

#if defined(sun) && defined(__sun__) && defined(__osf__) && defined(ultrix) && defined(__ultrix)

  label5->hide();
  cd_device_edit->hide();

#endif

  ttcheckbox = new QCheckBox("Show Tool Tips", this, "tooltipscheckbox");
  ttcheckbox->setGeometry(30,160,135,25);
  ttcheckbox->setChecked(configdata.tooltips);
  connect(ttcheckbox,SIGNAL(clicked()),this,SLOT(ttclicked()));

  
  
}


void ConfigDlg::okbutton() {



}

void ConfigDlg::device_changed(const char* dev) {

  configdata.cd_device = dev;
}

void ConfigDlg::ttclicked(){

  if(ttcheckbox->isChecked())
    configdata.tooltips = TRUE;
  else
    configdata.tooltips = FALSE;

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
  
  return &configdata;

}

#include "configdlg.moc"



