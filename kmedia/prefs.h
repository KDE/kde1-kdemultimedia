#ifndef PREFS_H
#define PREFS_H

#include <stdio.h>

#include <qdialog.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <ktabctl.h>
#include "kconfig.h"
#include "kslider.h"

class Preferences : public QDialog 
{
      Q_OBJECT
   private:
      QPushButton	*buttonOk, *buttonApply, *buttonCancel;
      KTabCtl		*tabctl;
      QWidget		*page1, *page2;
   public:
      Preferences( QWidget *parent );
      QLineEdit		*mixerLE;
      QString	 	mixerCommand;
      KConfig		*kcfg;

signals:
	void optionsApply();
	
   public slots:
      void slotShow();
      void slotOk();
      void slotApply();
      void slotCancel();
};


#endif /* PREFS_H */


