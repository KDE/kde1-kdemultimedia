//-*-C++-*-
#ifndef KMEDIA_H
#define KMEDIA_H

/*
	kmedia.h : Class declaration
*/

#include <qtimer.h>
#include <qmsgbox.h>
#include <qslider.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <signal.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <kurl.h>
#include <kapp.h>
#include <kmsgbox.h>

extern "C" {
#include <mediatool.h>
}

//#include <kiconloader.h>
#include "kmediawin.h"



class KMEDIA : public QWidget { //: public KTopLevelWidget {
  Q_OBJECT

public:
  KMEDIA( QWidget *parent = 0, const char *name = 0 );
  KMediaWin	*kmw;


};

#endif /* KMEDIA_H */
