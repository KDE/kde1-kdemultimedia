
 
APPSDIR = $(kde_appsdir)/Multimedia
INCLUDES= $(all_includes)

####### Files

SUBDIRS = pics docs
bin_PROGRAMS = 	kmedia
kmedia_SOURCES	= kmediaui.cpp mediaactions.cpp uiactions.cpp kmediawin.cpp prefs.cpp
noinst_HEADERS = kmediaui.h kmediawin.h prefs.h
kmedia_LDADD = -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 -lmediatool $(LIBCOMPAT) $(LIBSOCKET)

kmedia_LDFLAGS = $(all_libraries) $(KDE_RPATH)

BUILTSOURCES = kmediaui.moc kmediawin.moc prefs.moc
DISTCLEANFILES = kmediaui.moc kmediawin.moc prefs.moc

EXTRA_DIST = KMedia.kdelnk VERSION

kmediaui.cpp: kmediaui.moc
kmediaui.moc: kmediaui.h
	$(MOC) kmediaui.h -o kmediaui.moc

kmediawin.cpp: kmediawin.moc
kmediawin.moc: kmediawin.h
	$(MOC) kmediawin.h -o kmediawin.moc

prefs.cpp: prefs.moc
prefs.moc: prefs.h
	$(MOC) prefs.h -o prefs.moc

version.h: VERSION
	sed "s/0/#define APP_VERSION 0/" <VERSION >version.h

uiactions.cpp: version.h

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) KMedia.kdelnk $(APPSDIR)

uninstall-local:
	rm -f $(APPSDIR)/KMedia.kdelnk

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kmedia_SOURCES) && mv messages.po ../po/kmedia.pot

edit:
	xemacs $(kmedia_SOURCES) $(noinst_HEADERS) &
