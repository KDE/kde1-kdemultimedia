Summary: Play midi/karaoke files and show lyrics. For X11/KDE
Name: kmid
Version: 0.4.1
Release: 1
Copyright: GPL
Group: X11/K Desktop Environment/Multimedia
Source: kmid-0.4.1.tgz
Requires: qt >= 1.3 , kdelibs

%description
KMid is a midi/karaoke file player, with configurable midi mapper,
real Session Management, drag & drop, customizable fonts, etc.
It has a very nice interface which let you easily follow the tune while
changing the color of the lyrics.

%prep
%setup

%build
export KDEDIR=/opt/kde
./configure
make all 

%install
export KDEDIR=/opt/kde
make install

%files
%doc kmid/README kmid/COPYING kmid/PEOPLE kmid/kmid-0.4.1.lsm kmid/ChangeLog

/opt/kde/bin/kmid
/opt/kde/share/doc/HTML/en/kmid
/opt/kde/share/icons/kmid.xpm
/opt/kde/share/applnk/Multimedia/kmid.kdelnk
/opt/kde/share/mimelnk/audio/x-karaoke.kdelnk
/opt/kde/share/apps/kmid/toolbar
/opt/kde/share/apps/kmid/maps/gm.map
/opt/kde/share/apps/kmid/maps/YamahaPSS790.map
/opt/kde/share/apps/kmid/maps/YamahaPSR500.map
/opt/kde/share/apps/kmid/UnBesoYUnaFlor.kar
/opt/kde/share/apps/kmid/AgainstAllOdds.kar
/opt/kde/share/apps/kmid/MoonlightShadow.kar

