 
APPSDIR = $(kde_appsdir)/Multimedia
INCLUDES= $(all_includes)

####### Files

SUBDIRS = pics docs
bin_PROGRAMS = 	kmedia
kmedia_SOURCES	= kmedia.cpp mediaactions.cpp uiactions.cpp kmediawin.cpp prefs.cpp
noinst_HEADERS = kmedia.h kmediawin.h prefs.h
kmedia_LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 -lmediatool
kmedia_LDFLAGS = $(LDFLAGS) $(KDE_RPATH)

BUILTSOURCES = kmedia.moc kmediawin.moc prefs.moc
DISTCLEANFILES = kmedia.moc kmediawin.moc prefs.moc



kmedia.cpp: kmedia.moc
kmedia.moc: kmedia.h
	$(MOC) kmedia.h -o kmedia.moc

kmediawin.cpp: kmediawin.moc
kmediawin.moc: kmediawin.h
	$(MOC) kmediawin.h -o kmediawin.moc

prefs.cpp: prefs.moc
prefs.moc: prefs.h
	$(MOC) prefs.h -o prefs.moc

version.h: VERSION
	sed "s/0/#define APP_VERSION 0/" <VERSION >version.h

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) KMedia.kdelnk $(APPSDIR)

uninstall-local:
	rm -f $(APPSDIR)/KMedia.kdelnk

messages:
	@XGETTEXT@ -C -ki18n *.cpp -x../../kdelibs/po/kde.pot && mv messages.po ../po/kmedia.pot

edit:
	xemacs $(kmedia_SOURCES) $(noinst_HEADERS) &
