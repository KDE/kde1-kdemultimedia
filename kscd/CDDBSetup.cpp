/**********************************************************************

	--- Dlgedit generated file ---

	File: CDDBSetup.cpp
	Last generated: Wed Dec 31 22:53:06 1997

 *********************************************************************/

#include "CDDBSetup.h"


#define Inherited CDDBSetupData

extern KApplication 	*mykapp;

CDDBSetup::CDDBSetup
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{



  QString temp;
  QString basedirdefault;


  basedir_edit->setText(temp.data());
  connect(basedir_edit,SIGNAL(textChanged(const char*)),
	  this,SLOT(basedir_changed(const char*)));

  connect(submit_edit,SIGNAL(textChanged(const char*)),
	  this,SLOT(submitaddress_changed(const char*)));

  connect(update_button,SIGNAL(clicked()),
	  this,SLOT(serverlist_update()));

  connect(defaults_button,SIGNAL(clicked()),
	  this,SLOT(set_defaults()));

  connect(help_button,SIGNAL(clicked()),
	  this,SLOT(help()));

  connect(server_listbox,SIGNAL(highlighted(int)),
	  this,SLOT(set_current_server(int)));

  connect(remote_cddb_cb,SIGNAL(toggled(bool)),
	  this,SLOT(enable_remote_cddb(bool)));

  //  submit_edit->setEnabled(false);


}


CDDBSetup::~CDDBSetup()
{
}


void CDDBSetup::set_current_server(int i){

  current_server_string = server_listbox->text(i);
  current_server_label->setText(current_server_string.data());
  emit updateCurrentServer();
}

void CDDBSetup::basedir_changed(const char* str){

  basedirstring = str;

}

void CDDBSetup::enable_remote_cddb(bool){

}
void CDDBSetup::submitaddress_changed(const char* str){

  submitaddressstring = str;

}

void CDDBSetup::help(){

  if(mykapp)
    mykapp->invokeHTMLHelp("kscd/kscd.html","");

}

void CDDBSetup::insertData(const QStrList& _serverlist,
			   const QString& _basedir,const QString& _submitaddress,
			   const QString& _current_server,
			   const bool& remote_enabled){

  server_listbox->setAutoUpdate(false);
  server_listbox->clear();
  server_listbox->insertStrList(&_serverlist, -1);
  server_listbox->setAutoUpdate(true);

  basedirstring = _basedir.copy();
  basedir_edit->setText(basedirstring);

  submitaddressstring = _submitaddress.copy();
  submit_edit->setText(submitaddressstring);
  current_server_string = _current_server.copy();
  current_server_label->setText(current_server_string.data());
  remote_cddb_cb->setChecked(remote_enabled);

}

void CDDBSetup::set_defaults(){

  server_listbox->setAutoUpdate(false);
  server_listbox->clear();
  server_listbox->insertItem("cddb.cddb.com cddbp 888 -", -1);
  server_listbox->setAutoUpdate(true);
  server_listbox->repaint();

  basedirstring = mykapp->kde_datadir().copy();
  basedirstring += "/kscd/cddb/";
  basedir_edit->setText(basedirstring);

  submitaddressstring = "xmcd-cddb@amb.org";
  submit_edit->setText(submitaddressstring);
  current_server_string = "cddb.cddb.com cddbp 888 -";
  current_server_label->setText(current_server_string.data());
  remote_cddb_cb->setChecked(true);
  emit updateCurrentServer();

}
void CDDBSetup::getData(QStrList& _serverlist,QString& _basedir,
			QString& _submitaddress, 
			QString& _current_server,
			bool& remote_enabled){

  _serverlist.clear();
  for(uint i = 0; i < server_listbox->count();i++){
  _serverlist.append(server_listbox->text(i));
  }
  _basedir = basedirstring.copy();
  _submitaddress = submitaddressstring.copy();

  _current_server = current_server_string.copy();
  remote_enabled = remote_cddb_cb->isChecked();

}

void CDDBSetup::getCurrentServer(QString& ser){

  ser = current_server_string.copy();

}
void CDDBSetup::serverlist_update(){

  emit    updateCDDBServers();

}

void CDDBSetup::insertServerList(QStrList& list){

  server_listbox->setAutoUpdate(false);
  server_listbox->clear();
  server_listbox->insertStrList(&list, -1);
  server_listbox->setAutoUpdate(true);
  server_listbox->repaint();
}

#include "CDDBSetup.moc"
#include "CDDBSetupData.moc"
