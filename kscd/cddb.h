
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


#ifndef _CDDB_H_
#define _CDDB_H_

#include <qtimer.h>
#include <qstring.h>
#include <qstrlist.h>

#include <ksock.h>

#define DEFAULT_CDDB_SERVER "www.cddb.com cddbp 8880 -"
#define DEFAULT_CDDBHTTP_SERVER "cddb.moonsoft.com http 80 /~cddb/cddb.cgi"

class CDDB:public QObject
{

    Q_OBJECT

	public:

    CDDB(char *hostname=0, int port = 0, int timeout = 60);
    ~CDDB();

    enum { INIT, ERROR_INIT, HELLO, ERROR_HELLO, READY, QUERY, ERROR_QUERY,
	   CDDB_READ, CDDB_DONE, ERROR_CDDB_READ, CDDB_TIMEDOUT, INEX_READ,
	   SERVERLISTGET, REGULAR, REQUEST_SERVER_LIST, SERVER_LIST_WAIT,
	   HTTP_REQUEST };

    bool  isConnected() {return connected;};

    void getData(
	QString& data,
	QStrList& titlelist,
	QStrList& extlist, 
	QString& cat, 
	QStrList& discidlist,
	int& revision,
	QStrList& playlist
    );

    void  get_inexact_list(QStrList& inexact_list);
    void  query_exact(QString line);
    void  setPathList(QStrList& paths);
    bool  checkDir(unsigned long magicID, char* dir);
    bool  getValue(QString& key,QString& value, QString& data);
    void  serverList(QStrList& list);
    void  cddbgetServerList(QString& server);
    void  close_connection();

    static void  sighandler(int sig);
    static void  setalarm();

    void        queryCD(unsigned long magicID,QStrList& querylist);
    int		getState();

    bool local_query(
	unsigned long magicID,
	QString&  data,
	QStrList& titlelist,
	QStrList& extlist,
	QString&  category,
	QStrList& discidlist,
	int& revision,
	QStrList& playlist
    );
    void getCategoryFromPathName(char* pathname, QString& string);

    static bool normalize_server_list_entry(QString &entry);

    void setHTTPProxy(QString host, int port);
    void useHTTPProxy(bool flag);

    bool    useHTTPProxy();
    QString getHTTPProxyHost();
    int     getHTTPProxyPort();
   
 protected:
    void 	do_state_machine();
    void 	parse_serverlist();

    public slots:

    void	 cddb_connect(QString& server);
    void	 cddb_connect_internal();
    void         cddb_read(KSocket* sock);
    void         cddb_close(KSocket* sock);
    void         cddb_timed_out_slot();
    void	 isolate_lastline();

 signals:

    void        cddb_done();
    void	cddb_timed_out();
    void 	cddb_failed();
    void 	cddb_ready();
    void        cddb_inexact_read();
    void 	cddb_no_info();
    void	get_server_list_done();
    void        get_server_list_failed();

 private:

    QTimer 	starttimer;
    QTimer 	timeouttimer;
    QTimer      expecttimer;

    QStrList    pathlist;
    QString 	hostname;

    QString     proxyhost;
    int         proxyport;
    bool        use_http_proxy;
   
    QString     cgi;
    int         saved_state; // I was using stack here, but I guess it's overhead
   
    QString     respbuffer;
    QString     tempbuffer;
    QString     lastline;
    int		timeout;
    int     	port;
    bool    	connected;
    bool	readonly;
    KSocket	*sock;
    int	        state;
    int		mode;
    struct passwd* pw;     
    QString     category;
    QString	title;
    QStrList 	serverlist;
    unsigned long magicID;

    typedef enum {CDDBP,CDDBHTTP,SMTP,UNKNOWN} transport;

    transport protocol;
    transport decodeTransport(const char *);
};

#endif


