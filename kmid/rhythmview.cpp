#include "rhythmview.h"

RhythmView::RhythmView (QWidget *parent, char *name) : QWidget (parent,name)
{
    lamps=NULL;
    setRhythm(4,4);
}

RhythmView::~RhythmView()
{
    if(lamps!=NULL)
    {
        for (int i=0;i<num;i++) delete lamps[i];
        delete lamps;
        lamps=NULL;
    }
}

void RhythmView::setRhythm(int numerator,int denominator)
{
    int i;
    if(lamps!=NULL)
    {
        for (i=0;i<num;i++) delete lamps[i];
        delete lamps;
        lamps=NULL;
    }
    num=numerator;
    den=denominator;
    
    lamps=new (KLedLamp *)[num];
    int w=width()/num;
    int x=0;

    for (i=0;i<num;i++)
    {
        lamps[i]=new KLedLamp(this);
        lamps[i]->setState(KLedLamp::Off);
        lamps[i]->setGeometry(x+2,0,w-4,height());
        lamps[i]->show();
        x+=w;
    }
//    lamps[0]->setState(KLedLamp::On);

}

void RhythmView::Beat(int j)
{
    if (j>num) setRhythm(j,4); // This is a preventive case
    for (int i=0;i<num;i++)
    {
        lamps[i]->setState(KLedLamp::Off);
    }
    lamps[j-1]->setState(KLedLamp::On);
}

void RhythmView::Beat(void)
{
    
}

void RhythmView::resizeEvent(QResizeEvent *)
{
    int w=width()/num;
    int x=0;

    for (int i=0;i<num;i++)
    {
        lamps[i]->setGeometry(x+2,0,w-4,height());
        x+=w;
    }
}
