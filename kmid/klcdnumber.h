/**************************************************************************

    klcdnumber.h  - The KLCDNumber widget (displays a lcd number)
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
#include <qwidget.h>

class QPainter;
class KTriangleButton;

class KLCDNumber : public QWidget
{
    Q_OBJECT
protected:
    struct digit {
        bool up;  
        bool nw;  
        bool ne;
        bool md;
        bool sw;
        bool se;
        bool bt;
    };
    /*
        up
        ---
     nw|   |ne
       |___|<------ md
       |   |
     sw|___|se
        bt
     */
    

    KLCDNumber::digit Digit[11];
/*
={
        / 0 /    {true,true,true,false,true,true,true},
        / 1 /    {false,false,true,false,false,true,false},
        / 2 /    {true,false,true,true,true,false,true},
        / 3 /    {true,false,true,true,false,true,true},
        / 4 /    {false,true,true,true,false,true,false},
        / 5 /    {true,true,false,true,false,true,true},
        / 6 /    {true,true,false,true,true,true,true},
        / 7 /    {true,false,true,false,false,true,false},
        / 8 /    {true,true,true,true,true,true,true},
        / 9 /    {true,true,true,true,false,true,true},
        /   /    {false,false,false,false,false,false,false}
    };*/
    
    int numDigits;
    bool setUserChangeValue;

    int value;

    void drawVerticalBar(QPainter *qpaint,int x,int y,int w,int h,int d);
    void drawHorizBar(QPainter *qpaint,int x,int y,int w,int h,int d);
    void drawDigit(QPainter *qpaint,int x,int y,int w,int h,digit d);

    void initDigits(void);

public:
    KLCDNumber(int _numDigits,QWidget *parent,char *name);
    KLCDNumber(bool _setUserChangeValue,int _numDigits,QWidget *parent,char *name);
    
    void setValue(int v);
    int getValue(void) { return value; };

    void display (int v);
    void display (double v) { display((int)v); };

protected:

    virtual void paintEvent ( QPaintEvent *e );
    virtual void resizeEvent ( QResizeEvent *e);
    
    KTriangleButton *downBtn;
    KTriangleButton *upBtn;
    
public slots:

    void decreaseValue();
    void increaseValue();
    void decreaseValueFast();
    void increaseValueFast();

signals:

    void valueChanged(int v);
    
};
