/**************************************************************************

    kdisptext.cpp  - The widget that displays the karaoke/lyrics text 
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
#include "kdisptext.moc"
#include <qpainter.h>
#include <qfont.h>
#include <qfontmet.h>
#include <qscrbar.h>
#include <kapp.h>
#include <kconfig.h>
#include <kapp.h>

#define NUMPREVLINES 2

//#define KDISPTEXTDEBUG

KDisplayText::KDisplayText(QWidget *parent,const char *name) : QWidget(parent,name)
{
linked_list=NULL;
cursor_line=NULL;
first_line=NULL;
cursor=NULL;
textscrollbar=NULL;
nlines=0;
setBackgroundColor(QColor (110,110,110));
KConfig *kcfg=(KApplication::getKApplication())->getConfig();
kcfg->setGroup("KMid");
QFont *qtextfontdefault=new QFont("courier",22);
qtextfont=new QFont(kcfg->readFontEntry("KaraokeFont",qtextfontdefault));
delete qtextfontdefault;
qfmetr=new QFontMetrics(*qtextfont);
font_height=qfmetr->height();
qpaint=NULL;
nvisiblelines=height()/qfmetr->lineSpacing();
textscrollbar=new QScrollBar(1,1,1,2,1,QScrollBar::Vertical,this,"TheKaraokeScrollBar");
connect(textscrollbar,SIGNAL(valueChanged(int)),this,SLOT(ScrollText(int)));
};

KDisplayText::~KDisplayText()
{
RemoveLinkedList();
//if (textscrollbar!=NULL) delete textscrollbar;
};

void KDisplayText::PreDestroyer(void)
{
delete qfmetr;
delete qtextfont;
if (qpaint!=NULL) delete qpaint;
};

void KDisplayText::RemoveLine(kdispt_line *tmpl)
{
kdispt_ev *tmpe;
while (tmpl->ev!=NULL)
    {
    tmpe=tmpl->ev;
    tmpl->ev=tmpe->next;
    delete tmpe;
    };
};

void KDisplayText::RemoveLinkedList(void)
{
cursor=NULL;
cursor_line=NULL;
first_line=NULL;
nlines=0;

kdispt_line *tmpl;
while (linked_list!=NULL)
    {
    RemoveLine(linked_list);
    tmpl=linked_list;
    linked_list=linked_list->next;
    delete tmpl;
    };
};

void KDisplayText::ClearEv(void)
{
RemoveLinkedList();
repaint(TRUE);
};

int KDisplayText::IsLineFeed(char c,int type)
{
switch (type)
    {
    case (0) : if ((c==0)||(c==10)||(c==13)||(c=='\\')||(c=='/')||(c=='@')) return 1;break;
    case (1) : if (/*(c==0)||*/(c=='\\')||(c=='/')||(c=='@')) return 1;break;
    case (5) : if (/*(c==0)||*/(c==10)||(c==13)) return 1;break;
    };
return 0;
};

void KDisplayText::AddEv(SpecialEvent *ev)
{
if ((ev->type==1) || (ev->type==5))
    {
    if (linked_list==NULL)
        {
        linked_list=new kdispt_line;
        linked_list->next=NULL;
        linked_list->num=1;
        linked_list->ypos=30;
        linked_list->ev=new kdispt_ev;
        cursor_line=linked_list;
        cursor=cursor_line->ev;
        cursor->spev=ev;
        cursor->xpos=5;
        if (IsLineFeed(ev->text[0],ev->type))
             cursor->width=qfmetr->width(&ev->text[1]);
            else
             cursor->width=qfmetr->width(ev->text);
        cursor->next=NULL;    
        first_line=linked_list;
	nlines=1;
        }
       else
        {
        if (IsLineFeed(ev->text[0],ev->type))
            {
	    nlines++;
            cursor_line->next=new kdispt_line;
	    cursor_line=cursor_line->next;
	    cursor_line->num=nlines;
	    cursor_line->ypos=cursor_line->num*30;
	    cursor_line->ev=new kdispt_ev;
	    cursor_line->next=NULL;
	    cursor=cursor_line->ev;
	    cursor->xpos=5;
	    cursor->width=qfmetr->width(&ev->text[1]);
            }
 	   else
	    {
	    cursor->next=new kdispt_ev;
	    cursor->next->xpos=cursor->xpos+cursor->width;
	    cursor=cursor->next;
	    cursor->width=qfmetr->width(ev->text);
	    };
	cursor->spev=ev;
	cursor->next=NULL;
        };
    };
};

void KDisplayText::calculatePositions(void)
{
int typeoftextevents=1;
int fin=0;
kdispt_line *tmpl;
kdispt_ev *tmp;
int tmpx;
int tmpy;
while (!fin)
    {
    tmpl=linked_list;
    tmpy=30;
    while (tmpl!=NULL)
	{
	tmp=tmpl->ev;
	tmpx=5;
	while (tmp!=NULL)
	    {
	    if (tmp->spev->type==typeoftextevents)
		{
		if (IsLineFeed(tmp->spev->text[0],typeoftextevents))
		    {
		    tmpy+=30;
		    tmpx=5;
		    tmp->xpos=tmpx;
		    if (tmp->spev->text[0]!=0) tmp->width=qfmetr->width(&tmp->spev->text[1]);
		    }
		   else
		    {
		    tmp->xpos=tmpx;
		    tmp->width=qfmetr->width(tmp->spev->text);
		    };
		tmpx+=tmp->width;
		};
	    tmp=tmp->next;
	    };
	tmpl->ypos=tmpy;
	tmpl=tmpl->next;
	};
     
    if (typeoftextevents==1) typeoftextevents=5;
	else fin=1;
    };



};

void KDisplayText::paintEvent(QPaintEvent *)
{
if (linked_list==NULL) return;
if (nlines>nvisiblelines)
    textscrollbar->setRange(1,nlines-nvisiblelines+1);
   else
    textscrollbar->setRange(1,1);

//DebugText();
if (qpaint==NULL)
    {
    qpaint=new QPainter(this);
    qpaint->setFont(*qtextfont);
    };
int i=0;
qpaint->setPen(QColor(255,255,0));
int colorplayed=1;
if (cursor==NULL) colorplayed=0; // Thus, the program doesn't change the color
kdispt_line *tmpl=first_line;
kdispt_ev *tmp;
KConfig *kcfg=KApplication::getKApplication()->getConfig();
kcfg->setGroup("KMid");
typeoftextevents=kcfg->readNumEntry("TypeOfTextEvents",5);
#ifdef KDISPTEXTDEBUG
printf("events displayed %d\n",typeoftextevents);
#endif
while ((i<nvisiblelines)&&(tmpl!=NULL))
    {
    tmpl->ypos=(tmpl->num-first_line->num+1)*qfmetr->lineSpacing();
    tmp=tmpl->ev;
    while ((tmp!=NULL)&&(tmp->spev->type!=typeoftextevents)) tmp=tmp->next;
    while (tmp!=NULL)
        {
	if ( colorplayed && 
	(tmp->spev->absmilliseconds>=cursor->spev->absmilliseconds))
		{
		qpaint->setPen(QColor(0,0,0));
		colorplayed=0;
		};	
	
        if (IsLineFeed(tmp->spev->text[0],tmp->spev->type))
            qpaint->drawText(tmp->xpos,tmpl->ypos,&tmp->spev->text[1]);
           else
            qpaint->drawText(tmp->xpos,tmpl->ypos,tmp->spev->text);
	tmp=tmp->next;
        while ((tmp!=NULL)&&(tmp->spev->type!=typeoftextevents)) tmp=tmp->next;
        };
    i++;
    tmpl=tmpl->next;
    };
qpaint->setPen(QColor(255,255,0));
};

void KDisplayText::resizeEvent(QResizeEvent *)
{
nvisiblelines=height()/qfmetr->lineSpacing();

textscrollbar->setGeometry(width()-16,0,16,height());

};

void KDisplayText::CursorToHome(void)
{
if (linked_list==NULL)
    {
    cursor_line=NULL;
    cursor=NULL;
    first_line=NULL;
    }
   else
    {
    cursor_line=linked_list;
    cursor=cursor_line->ev;
    first_line=linked_list;
    };
textscrollbar->setValue(1);

}; 

void KDisplayText::PaintIn(void)
{
if ((cursor==NULL)||(cursor_line==NULL))
    {
    printf("KDispT : cursor == NULL !!!\n");
    return;
    };
if ((cursor_line->num>=first_line->num)&&(cursor_line->num<first_line->num+nvisiblelines))
   { 
   if (cursor->spev->type==typeoftextevents)
      if (IsLineFeed(cursor->spev->text[0],cursor->spev->type))
        qpaint->drawText(cursor->xpos,cursor_line->ypos,&cursor->spev->text[1]);
       else
        qpaint->drawText(cursor->xpos,cursor_line->ypos,cursor->spev->text);
   };
cursor=cursor->next;
while ((cursor==NULL)&&(cursor_line!=NULL))
    {
    cursor_line=cursor_line->next;
    if (cursor_line!=NULL) 
	{
	cursor=cursor_line->ev;
        if ((cursor_line->num>first_line->num+NUMPREVLINES)
	   &&(cursor_line->num<first_line->num+nvisiblelines+1))
		 textscrollbar->setValue(textscrollbar->value()+1);
	};
    };
};

void KDisplayText::ScrollText(int i)
{
kdispt_line *tmpl=linked_list;
while ((tmpl!=NULL)&&(tmpl->num<i)) tmpl=tmpl->next;
first_line=tmpl;
repaint();
};

void KDisplayText::gotomsec(ulong i)
{
if (linked_list==NULL) return;
   else
    {
    cursor_line=linked_list;
    cursor=cursor_line->ev;
    first_line=linked_list;
    };
while ((cursor_line!=NULL)&&(cursor->spev->absmilliseconds<i))
    {
    cursor=cursor->next;
    while ((cursor==NULL)&&(cursor_line!=NULL))
        {
        cursor_line=cursor_line->next;
        if (cursor_line!=NULL) 
	    {
	    cursor=cursor_line->ev;
            if ((cursor_line->num>first_line->num+NUMPREVLINES)
	       &&(cursor_line->num<first_line->num+nvisiblelines+1))
	        if ((first_line!=NULL)&&(first_line->num+nvisiblelines<=nlines)) first_line=first_line->next;
	    };
        };
    };
textscrollbar->setValue(first_line->num);
repaint();
};

QFont *KDisplayText::getFont(void)
{
return qtextfont;
};

void KDisplayText::fontChanged(void)
{
KConfig *kcfg=(KApplication::getKApplication())->getConfig();
kcfg->setGroup("KMid");
QFont *qtextfontdefault=new QFont(*qtextfont);
delete qtextfont;
qtextfont=new QFont(kcfg->readFontEntry("KaraokeFont",qtextfontdefault));
delete qtextfontdefault;
qfmetr=new QFontMetrics(*qtextfont);
font_height=qfmetr->height();
if (qpaint!=NULL) qpaint->setFont(*qtextfont);
calculatePositions();
nvisiblelines=height()/qfmetr->lineSpacing();
repaint(TRUE);
};
