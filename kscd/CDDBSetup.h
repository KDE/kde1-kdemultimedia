/**********************************************************************

	--- Dlgedit generated file ---

	File: CDDBSetup.h
	Last generated: Wed Dec 31 22:53:06 1997

 *********************************************************************/

#ifndef CDDBSetup_included
#define CDDBSetup_included

#include "CDDBSetupData.h"
#include <kapp.h>
#include <qstrlist.h>

class CDDBSetup : public CDDBSetupData
{
    Q_OBJECT

public:

    CDDBSetup
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~CDDBSetup();

public slots:
   void basedir_changed(const char* str);
   void submitaddress_changed(const char* str);
   void serverlist_update();
   void getData(QStrList& _serverlist,QString& _basedir,QString& _submitaddress,
		QString& _current_server, bool& _remote_enabled);
   void insertData(const QStrList& _serverlist,
		   const QString& _basedir,const QString& _submitaddress,
		   const QString& _current_server, 
		   const bool& _remote_enabled);
   void set_current_server(int i);
   void set_defaults();
   void help();
   void enable_remote_cddb(bool en);

signals:
   void updateCDDBServers();
   void updateCurrentServer();

public:
   void insertServerList(QStrList& list);
   void getCurrentServer(QString& ser);

private:

   QString basedirstring;
   QString submitaddressstring;
   QString current_server_string;

};
#endif // CDDBSetup_included
