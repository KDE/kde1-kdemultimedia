
INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD = player/libplayer.a -lkdeui -lkdecore -lqt -lXext -lX11 

kmid_SOURCES = kmidframe.cpp kmidclient.cpp kdisptext.cpp \
	kslidertime.cpp midicfgdlg.o main.o

PLAYEROBJECTS = midiout.o player.o track.o midimapper.o midfile.o dattypes.o midistat.o deviceman.o synthout.o

SRCMETA = kmidframe.moc kmidclient.moc kdisptext.moc kslidertime.moc midicfgdlg.moc 

DISTCLEANFILES = $(SRCMETA)

bin_PROGRAMS = kmid

SUBDIRS = player toolbarxpm maps examples

applnkdir = $(kde_appsdir)/Multimedia
applnk_DATA = kmid.kdelnk

mimedir = $(kde_mimedir)
mime_DATA = x-karaoke.kdelnk

docsdir = $(kde_htmldir)/en/kmid
docs_DATA = kmid.html

icondir = $(kde_icondir)
icon_DATA = kmid.xpm

