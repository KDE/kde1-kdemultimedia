/**********************************************************************

	--- Qt Architect generated file ---

	File: CDDBSetupData.cpp
	Last generated: Fri Apr 17 16:39:07 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#include "CDDBSetupData.h"

#define Inherited QWidget

#include <qlabel.h>
#include <qgrpbox.h>

CDDBSetupData::CDDBSetupData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, 0 )
{
	QGroupBox* dlgedit_GroupBox_1;
	dlgedit_GroupBox_1 = new QGroupBox( this, "GroupBox_1" );
	dlgedit_GroupBox_1->setGeometry( 10, 10, 365, 420 );
	dlgedit_GroupBox_1->setMinimumSize( 10, 10 );
	dlgedit_GroupBox_1->setMaximumSize( 32767, 32767 );
	dlgedit_GroupBox_1->setFrameStyle( 49 );
	dlgedit_GroupBox_1->setTitle( "" );
	dlgedit_GroupBox_1->setAlignment( 1 );

	QLabel* dlgedit_Label_8;
	dlgedit_Label_8 = new QLabel( this, "Label_8" );
	dlgedit_Label_8->setGeometry( 20, 120, 160, 20 );
	dlgedit_Label_8->setMinimumSize( 10, 10 );
	dlgedit_Label_8->setMaximumSize( 32767, 32767 );
	dlgedit_Label_8->setText( "CDDB Base Directory" );
	dlgedit_Label_8->setAlignment( 289 );
	dlgedit_Label_8->setMargin( -1 );

	basedir_edit = new QLineEdit( this, "LineEdit_4" );
	basedir_edit->setGeometry( 20, 140, 340, 28 );
	basedir_edit->setMinimumSize( 10, 10 );
	basedir_edit->setMaximumSize( 32767, 32767 );
	basedir_edit->setText( "" );
	basedir_edit->setMaxLength( 32767 );
	basedir_edit->setEchoMode( QLineEdit::Normal );
	basedir_edit->setFrame( TRUE );

	QLabel* dlgedit_Label_9;
	dlgedit_Label_9 = new QLabel( this, "Label_9" );
	dlgedit_Label_9->setGeometry( 20, 180, 210, 20 );
	dlgedit_Label_9->setMinimumSize( 10, 10 );
	dlgedit_Label_9->setMaximumSize( 32767, 32767 );
	dlgedit_Label_9->setText( "Send CDDB submissions to:" );
	dlgedit_Label_9->setAlignment( 289 );
	dlgedit_Label_9->setMargin( -1 );

	submit_edit = new QLineEdit( this, "LineEdit_5" );
	submit_edit->setGeometry( 20, 200, 240, 28 );
	submit_edit->setMinimumSize( 10, 10 );
	submit_edit->setMaximumSize( 32767, 32767 );
	submit_edit->setText( "cddb-test@xmcd.com" );
	submit_edit->setMaxLength( 32767 );
	submit_edit->setEchoMode( QLineEdit::Normal );
	submit_edit->setFrame( TRUE );

	server_listbox = new QListBox( this, "ListBox_2" );
	server_listbox->setGeometry( 20, 290, 240, 135 );
	server_listbox->setMinimumSize( 10, 10 );
	server_listbox->setMaximumSize( 32767, 32767 );
	server_listbox->setFrameStyle( 51 );
	server_listbox->setLineWidth( 2 );
	server_listbox->insertItem( "www.cddb.com cddbp 8880 -" );
	server_listbox->insertItem( "wwcddb.moonsoft.com http 80 /~cddb/cddb.cgi" );
	server_listbox->setBottomScrollBar( TRUE );
	server_listbox->setMultiSelection( FALSE );

	QLabel* dlgedit_Label_10;
	dlgedit_Label_10 = new QLabel( this, "Label_10" );
	dlgedit_Label_10->setGeometry( 20, 240, 235, 20 );
	dlgedit_Label_10->setMinimumSize( 10, 10 );
	dlgedit_Label_10->setMaximumSize( 32767, 32767 );
	dlgedit_Label_10->setText( "CDDB server" );
	dlgedit_Label_10->setAlignment( 289 );
	dlgedit_Label_10->setMargin( -1 );

	update_button = new QPushButton( this, "PushButton_7" );
	update_button->setGeometry( 270, 295, 90, 25 );
	update_button->setMinimumSize( 10, 10 );
	update_button->setMaximumSize( 32767, 32767 );
	update_button->setText( "Update" );
	update_button->setAutoRepeat( FALSE );
	update_button->setAutoResize( FALSE );

	current_server_label = new QLabel( this, "Label_4" );
	current_server_label->setGeometry( 20, 260, 240, 28 );
	current_server_label->setMinimumSize( 10, 10 );
	current_server_label->setMaximumSize( 32767, 32767 );
	current_server_label->setFrameStyle( 51 );
	current_server_label->setLineWidth( 2 );
	current_server_label->setText( "" );
	current_server_label->setAlignment( 289 );
	current_server_label->setMargin( -1 );

	remote_cddb_cb = new QCheckBox( this, "CheckBox_1" );
	remote_cddb_cb->setGeometry( 25, 15, 230, 30 );
	remote_cddb_cb->setMinimumSize( 10, 10 );
	remote_cddb_cb->setMaximumSize( 32767, 32767 );
	remote_cddb_cb->setText( "Enable Remote CDDB" );
	remote_cddb_cb->setAutoRepeat( FALSE );
	remote_cddb_cb->setAutoResize( FALSE );

	defaults_button = new QPushButton( this, "PushButton_2" );
	defaults_button->setGeometry( 270, 345, 90, 25 );
	defaults_button->setMinimumSize( 10, 10 );
	defaults_button->setMaximumSize( 32767, 32767 );
	defaults_button->setText( "Defaults" );
	defaults_button->setAutoRepeat( FALSE );
	defaults_button->setAutoResize( FALSE );

	help_button = new QPushButton( this, "PushButton_3" );
	help_button->setGeometry( 270, 400, 90, 25 );
	help_button->setMinimumSize( 10, 10 );
	help_button->setMaximumSize( 32767, 32767 );
	help_button->setText( "Help" );
	help_button->setAutoRepeat( FALSE );
	help_button->setAutoResize( FALSE );

	cddb_http_cb = new QCheckBox( this, "CheckBox_2" );
	cddb_http_cb->setGeometry( 25, 40, 230, 30 );
	cddb_http_cb->setMinimumSize( 10, 10 );
	cddb_http_cb->setMaximumSize( 32767, 32767 );
	connect( cddb_http_cb, SIGNAL(toggled(bool)), SLOT(http_access_toggled(bool)) );
	cddb_http_cb->setText( "Use HTTP proxy to access CDDB" );
	cddb_http_cb->setAutoRepeat( FALSE );
	cddb_http_cb->setAutoResize( FALSE );

	proxy_port_ef = new QLineEdit( this, "LineEdit_6" );
	proxy_port_ef->setGeometry( 315, 75, 45, 30 );
	proxy_port_ef->setMinimumSize( 10, 10 );
	proxy_port_ef->setMaximumSize( 32767, 32767 );
	proxy_port_ef->setText( "" );
	proxy_port_ef->setMaxLength( 5 );
	proxy_port_ef->setEchoMode( QLineEdit::Normal );
	proxy_port_ef->setFrame( TRUE );

	proxy_host_ef = new QLineEdit( this, "LineEdit_7" );
	proxy_host_ef->setGeometry( 70, 75, 230, 30 );
	proxy_host_ef->setMinimumSize( 10, 10 );
	proxy_host_ef->setMaximumSize( 32767, 32767 );
	proxy_host_ef->setText( "" );
	proxy_host_ef->setMaxLength( 32767 );
	proxy_host_ef->setEchoMode( QLineEdit::Normal );
	proxy_host_ef->setFrame( TRUE );

	QLabel* dlgedit_Label_12;
	dlgedit_Label_12 = new QLabel( this, "Label_12" );
	dlgedit_Label_12->setGeometry( 25, 75, 45, 30 );
	dlgedit_Label_12->setMinimumSize( 10, 10 );
	dlgedit_Label_12->setMaximumSize( 32767, 32767 );
	dlgedit_Label_12->setText( "HTTP://" );
	dlgedit_Label_12->setAlignment( 289 );
	dlgedit_Label_12->setMargin( -1 );

	QLabel* dlgedit_Label_13;
	dlgedit_Label_13 = new QLabel( this, "Label_13" );
	dlgedit_Label_13->setGeometry( 305, 75, 10, 30 );
	dlgedit_Label_13->setMinimumSize( 10, 10 );
	dlgedit_Label_13->setMaximumSize( 32767, 32767 );
	dlgedit_Label_13->setText( ":" );
	dlgedit_Label_13->setAlignment( 289 );
	dlgedit_Label_13->setMargin( -1 );

	resize( 385,465 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
}


CDDBSetupData::~CDDBSetupData()
{
}
void CDDBSetupData::http_access_toggled(bool)
{
}
