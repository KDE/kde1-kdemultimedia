/**************************************************************************

    channel.cpp  - The KMidChannel widget (with pure virtual members)
    Copyright (C) 1998  Antonio Larrosa Jimenez

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

#include "channel.h"
#include <qpainter.h>
#include <stdlib.h>
#include <kapp.h>
#include <kcombo.h>
#include "instrname.h"

KMidChannel::KMidChannel(int chn,QWidget *parent) : QWidget (parent)
{
    channel=chn;
    int i;
    for (i=0;i<128;i++) pressed[i]=0;
    KApplication *kappl;
    kappl=KApplication::getKApplication();
    keyboard=new QPixmap((const char *)(kapp->kde_datadir()+"/kmid/pics/keyboard.xpm"));
    penB = new QPen (black);
    penW = new QPen (white);
    penT = new QPen (colorGroup().background());

    setBackgroundColor(colorGroup().background());

    KConfig *kcfg=KApplication::getKApplication()->getConfig();
    kcfg->setGroup("KMid");
    QFont *qtextfontdefault=new QFont("lucida",18,QFont::Bold,TRUE);
    qcvfont=new QFont(kcfg->readFontEntry("ChannelViewFont",qtextfontdefault));
    delete qtextfontdefault;

    instrumentCombo = new KCombo(this,"instr");
    instrumentCombo->setGeometry(160,2,200,20);

    for (i=0;i<128;i++)
        instrumentCombo->insertItem(i18n(instrumentName[i]),i);
    
    
};

KMidChannel::~KMidChannel()
{
    delete keyboard;
    delete penB;
    delete penW;
};

void KMidChannel::paintEvent( QPaintEvent * )
{

    QPainter *qpaint=new QPainter(this);


    char tmp[110];
    sprintf(tmp,"%s %d",i18n("Channel"),channel);
    qpaint->setFont(*qcvfont);
    qpaint->setPen(QColor(00,00,00));
    qpaint->drawText(2,20,tmp);
    qpaint->setPen(QColor(255,255,255));
    qpaint->drawText(0,18,tmp);

    drawKeyboard(qpaint);
    drawPressedKeys(qpaint);
    
    delete qpaint;
};

void KMidChannel::drawKeyboard(QPainter *qpaint)
{
    int x=1;
    for (int i=0;(i<12) && (x<width());i++,x+=63)
    {
        qpaint->drawPixmap(x,KEYBOARDY,*keyboard);
    };
    qpaint->setPen(*penB);
    qpaint->drawLine(0,KEYBOARDY,0,KEYBOARDY+44);

    qpaint->setPen(*penT);
    qpaint->drawLine(0,KEYBOARDY+45,x+63,KEYBOARDY+45);


};

void KMidChannel::drawPressedKeys(QPainter *qpaint)
{
    for (int i=0;i<128;i++)
        if (pressed[i]) drawKey(qpaint,i);
};

void KMidChannel::drawKey(QPainter *qpaint,int key)
{
    int octave=key/12;
    int note=key%12;
    int x=octave*63+1;

    switch (note)
    {
    case (0) :  drawDo   (qpaint,x,pressed[key]);break;
    case (1) :  drawDo__ (qpaint,x,pressed[key]);break;
    case (2) :  drawRe   (qpaint,x,pressed[key]);break;
    case (3) :  drawRe__ (qpaint,x,pressed[key]);break;
    case (4) :  drawMi   (qpaint,x,pressed[key]);break;
    case (5) :  drawFa   (qpaint,x,pressed[key]);break;
    case (6) :  drawFa__ (qpaint,x,pressed[key]);break;
    case (7) :  drawSol  (qpaint,x,pressed[key]);break;
    case (8) :  drawSol__(qpaint,x,pressed[key]);break;
    case (9) :  drawLa   (qpaint,x,pressed[key]);break;
    case (10) : drawLa__ (qpaint,x,pressed[key]);break;
    case (11) : drawSi   (qpaint,x,pressed[key]);break;
    };
//    qpaint->flush();
};

void KMidChannel::noteOn(int key)
{
    pressed[key]=1;
    QPainter *qpaint=new QPainter(this);
    drawKey(qpaint,key);
    delete qpaint;
};

void KMidChannel::noteOff(int key)
{
    pressed[key]=0;
    QPainter *qpaint=new QPainter(this);
    drawKey(qpaint,key);
    delete qpaint;
};

void KMidChannel::changeInstrument(int pgm)
{
    instrumentCombo->setCurrentItem(pgm);
};


void KMidChannel::reset(void)
{
    for (int i=0;i<128;i++) pressed[i]=0;
    instrumentCombo->setCurrentItem(0);
    repaint(FALSE);
};

void KMidChannel::saveState(bool *p,int *pgm)
{
    for (int i=0;i<128;i++) p[i]=pressed[i];
    *pgm=instrumentCombo->currentItem();
};

void KMidChannel::loadState(bool *p,int *pgm)
{
    for (int i=0;i<128;i++) pressed[i]=p[i];
    instrumentCombo->setCurrentItem(*pgm);
    repaint(FALSE);
};
