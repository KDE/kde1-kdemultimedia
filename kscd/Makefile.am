# $Id$

MIMEDIR = $(kde_mimedir)/text/
PICSDIR  = $(kde_datadir)/kscd/pics/
APPSDIR = $(kde_appsdir)/Multimedia
CDDBDIR = $(kde_datadir)/kscd/cddb/

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD   = $(LIBCDROM) $(LIBCDAUDIO) $(LIBFPE) -lkfm -lkdeui \
	-lkdecore -lqt  -lX11 -lXext $(LIBSOCKET)
INCLUDES= $(all_includes)

####### Files

noinst_HEADERS = kscd.h struct.h bwlednum.h ledlamp.h inexact.h \
	CDDialog.h CDDialogData.h CDDBSetup.h CDDBSetupData.h mgconfdlg.h

kscd_SOURCES = kscd.cpp configdlg.cpp cddb.cpp ledlamp.cpp CDDialog.cpp \
	CDDialogData.cpp CDDBSetup.cpp CDDBSetupData.cpp docking.cpp \
	inexact.cpp cdinfo.c mgconfdlg.cpp \
	plat_sun.c plat_hpux.c plat_linux.c plat_news.c \
	plat_bsd386.c plat_ultrix.c cdrom.c scsi.c drv_toshiba.c drv_sony.c \
	plat_svr4.c plat_freebsd.c plat_osf1.c plat_irix.c bwlednum.cpp

cddaslave_SOURCES = plat_sun_audio.c plat_sun_cdda.c cddaslave.c  

SOURCES3=kscdmagic/bitmap.cpp  kscdmagic/core.cpp kscdmagic/main.cpp \
	kscdmagic/sound.cpp kscdmagic/xlibwrap.cpp kscdmagic/xlib.c

OBJECTS3=kscdmagic/bitmap.o  kscdmagic/core.o kscdmagic/main.o \
	kscdmagic/sound.o kscdmagic/xlibwrap.o kscdmagic/xlib.o

DISTCLEANFILES = kscd.moc bwlednum.moc configdlg.moc cddb.moc ledlamp.moc \
	 inexact.moc CDDialog.moc CDDialogData.moc CDDBSetup.moc \
	CDDBSetupData.moc docking.moc mgconfdlg.moc

bin_PROGRAMS = kscd cddaslave 
TARGET3 = kscdmagic

config.h: kscd-script 
	$(SHELL) ./kscd-script

SUBDIRS = docs kscdmagic

CDDB_DIRS = rock blues classical jazz newage soundtrack \
	reggae folk country misc data

bin_SCRIPTS = workman2cddb.pl

install-data-local:
	for i in $(CDDB_DIRS); do \
	if test ! -d $(CDDBDIR)/$$i; then \
		$(mkinstalldirs) $(CDDBDIR)/$$i ;\
		chmod 0777 $(CDDBDIR)/$$i ;\
	fi ;\
	done 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kscd.xpm $(kde_icondir)
	$(INSTALL_DATA) bitmaps/cd.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kscd.xpm $(kde_minidir)/kscd.xpm
	$(mkinstalldirs) $(PICSDIR)
	$(INSTALL_DATA) kscdlogo.xpm $(PICSDIR)
	$(INSTALL_DATA) bitmaps/cdsmall.xpm $(PICSDIR)
	$(mkinstalldirs) $(MIMEDIR)
	$(INSTALL_DATA) xmcd.kdelnk $(MIMEDIR)/xmcd.kdelnk
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kscd.kdelnk $(APPSDIR)/kscd.kdelnk

uninstall:
	rm -f $(APPSDIR)/kscd.kdelnk
	rm -f $(PICSDIR)/kscdlogo.xpm
	rm -f $(kde_icondir)/kscd.xpm
	rm -f $(kde_minidir)/kscd.xpm
	rm -f $(kde_icondir)/cd.xpm
	rm -f $(MIMEDIR)/xmcd.kdelnk

messages:
	@XGETTEXT@ -C --keyword=translate *.cpp -x../../kdelibs/po/kde.pot && mv messages.po ../po/kscd.pot   

####### Explicit dependencies

CDDBSetup.cpp: CDDBSetup.moc
CDDBSetup.cpp: CDDBSetupData.moc
CDDialog.cpp: CDDialog.moc
CDDialog.cpp: CDDialogData.moc
bwlednum.cpp: bwlednum.moc
cddb.cpp: cddb.moc
configdlg.cpp: configdlg.moc
docking.cpp: docking.moc
inexact.cpp: inexact.moc
kscd.cpp: kscd.moc config.h
ledlamp.cpp: ledlamp.moc
mgconfdlg.cpp: mgconfdlg.moc

####### Meta objects

kscd.moc: kscd.h
	$(MOC) kscd.h -o kscd.moc

bwlednum.moc: bwlednum.h
	$(MOC) bwlednum.h -o bwlednum.moc

configdlg.moc: configdlg.h
	$(MOC) configdlg.h -o configdlg.moc

cddb.moc: cddb.h
	$(MOC) cddb.h -o cddb.moc

ledlamp.moc: ledlamp.h
	$(MOC) ledlamp.h -o ledlamp.moc

inexact.moc: inexact.h
	$(MOC) inexact.h -o inexact.moc

CDDialog.moc: CDDialog.h
	$(MOC) CDDialog.h -o CDDialog.moc

CDDBSetup.moc: CDDBSetup.h
	$(MOC) CDDBSetup.h -o CDDBSetup.moc

CDDialogData.moc: CDDialogData.h
	$(MOC) CDDialogData.h -o CDDialogData.moc

CDDBSetupData.moc: CDDBSetupData.h
	$(MOC) CDDBSetupData.h -o CDDBSetupData.moc

docking.moc: docking.h
	$(MOC) docking.h -o docking.moc

mgconfdlg.moc: mgconfdlg.h
	$(MOC) mgconfdlg.h -o mgconfdlg.moc









