Summary: Play midi/karaoke files and show lyrics. Support External synths, AWE, GUS, and FM devices. For X11/KDE
Name: kmid
Version: 1.0
Release: 1
Copyright: GPL
Group: X11/K Desktop Environment/Multimedia
Source: kmid-1.0.tgz
Requires: qt >= 1.3 , kdelibs

%description
KMid is a midi/karaoke file player, with configurable midi mapper,
real Session Management, drag & drop, customizable fonts, etc.
It has a very nice interface which let you easily follow the tune while
changing the color of the lyrics.
It supports output through external synthesizers, AWE, FM and GUS cards.

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
%doc kmid/README kmid/COPYING kmid/PEOPLE kmid/kmid.lsm kmid/ChangeLog

/opt/kde/bin/kmid
/opt/kde/share/doc/HTML/en/kmid
/opt/kde/share/doc/HTML/es/kmid
/opt/kde/share/icons/kmid.xpm
/opt/kde/share/icons/mini/kmid.xpm
/opt/kde/share/applnk/Multimedia/kmid.kdelnk
/opt/kde/share/mimelnk/audio/x-karaoke.kdelnk
/opt/kde/share/apps/kmid/toolbar
/opt/kde/share/apps/kmid/fm
/opt/kde/share/apps/kmid/maps/gm.map
/opt/kde/share/apps/kmid/maps/YamahaPSS790.map
/opt/kde/share/apps/kmid/maps/YamahaPSR500.map
/opt/kde/share/apps/kmid/TakingCareOfBusiness.kar
/opt/kde/share/apps/kmid/AlwaysLookOnTheBrightSideOfLife.kar
/opt/kde/share/apps/kmid/UnVeleroLlamadoLibertad.kar

