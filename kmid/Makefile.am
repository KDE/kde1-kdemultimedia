
INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD = player/libplayer.a -lkdeui -lkdecore -lkfile -lkfm -lqt -lXext -lX11

kmid_SOURCES = kmidframe.cpp kmidclient.cpp kdisptext.cpp \
	 kslidertime.cpp midicfgdlg.cpp collectdlg.cpp kaskdlg.cpp \
	randomlist.cpp main.cpp 

noinst_HEADERS = *.h

PLAYEROBJECTS = midiout.o player.o track.o midimapper.o midfile.o dattypes.o midistat.o deviceman.o synthout.o

SRCMETA = kmidframe.moc kmidclient.moc kdisptext.moc kslidertime.moc \
	 midicfgdlg.moc collectdlg.moc kaskdlg.moc

DISTCLEANFILES = $(SRCMETA) ./.deps/* ./.deps/.P

bin_PROGRAMS = kmid

SUBDIRS = player toolbarxpm maps examples doc

applnk_DATA = kmid.kdelnk
applnkdir = $(kde_appsdir)/Multimedia

mime_DATA = x-karaoke.kdelnk
mimedir = $(kde_mimedir)/audio

icon_DATA = kmid.xpm
icondir = $(kde_icondir)

fmpatches_DATA = std.sb drums.sb std.o3 drums.o3
fmpatchesdir = $(kde_datadir)/kmid/fm

EXTRA_DIST = $(applnk_DATA) $(mime_DATA) $(icon_DATA) $(fmpatches_DATA) mini-kmid.xpm

%.moc: %.h
	$(MOC) $< -o $@

kmidclient.cpp:kmidclient.moc

kmidframe.cpp:kmidframe.moc

kaskdlg.cpp:kaskdlg.moc

collectdlg.cpp:collectdlg.moc

midicfgdlg.cpp:midicfgdlg.moc

kslidertime.cpp:kslidertime.moc

kdisptext.cpp:kdisptext.moc


messages:
	$(XGETTEXT) -C -ki18n $(kmid_SOURCES) && mv messages.po ../po/kmid.pot

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kmid.xpm $(kde_minidir)/kmid.xpm
	@echo "********************************************"
	@echo "Thanks for installing kmid 1.0 !"
	@echo "I have included some example songs that have been installed in"
	@echo $(kde_datadir)/kmid
	@echo ""
	@echo "If you find any bug, have any idea or only want to tell me"
	@echo "how much you like kmid :-), just send me an email to:"
	@echo "antlarr@arrakis.es"
	@echo ""
	@echo "I hope that you have fun with kmid !"
	@echo "Antonio Larrosa Jimenez (antlarr@arrakis.es)"
	@echo ""
	@echo "PS: I will not be liable for any flood during the use of kmid ;-)"
	@echo "*********************************************"
