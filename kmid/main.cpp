/**************************************************************************

    main.cpp  - The main function for KMid
    Copyright (C) 1997  Antonio Larrosa Jimenez

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Send comments and bug fixes to antlarr@arrakis.es
    or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain

***************************************************************************/
#include "kmidframe.h"
#include <stdlib.h>
#include <string.h>
//#include <qapp.h>
// Why can't I include qapp before kapp ?
#include <kapp.h>
#include <qwidget.h>

int main(int argc, char **argv)
{
    printf("KMid 0.2 Copyright (C) 1997 Antonio Larrosa Jimenez. Malaga (Spain)\n");
    printf("KMid comes with ABSOLUTELY NO WARRANTY; for details view file COPYING\n"
);
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions\n");

    KApplication *app=new KApplication(argc,argv);
    kmidFrame *kmidframe=new kmidFrame("KMid");

    app->setMainWidget ( kmidframe );
//    KConfig *kconf=app->getConfig();
//    kmidframe->readConfig(kconf);
//    app->restoreTopLevelGeometry();
// I have deleted provisionally this part, so you cannot pass a midi file in
//  the command line, but it will be better implemented in a few days
/*    if (argc>1) 
	{
	kmidframe->openFile(argv[1]);
	kmidframe->song_Play();
	};
*/
    kmidframe->show();

    int ret= app->exec();
    delete kmidframe;
    delete app;
    return ret;
};