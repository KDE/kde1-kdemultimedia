#include <qwidget.h>
#include <kledlamp.h>

class RhythmView : public QWidget
{
    int num,den;

    KLedLamp **lamps;
    
public:
    RhythmView (QWidget *parent, char *name);
    ~RhythmView();

    void setRhythm(int numerator,int denominator);

    void Beat(int i); // Sets the beat number

    void Beat(void); // Just increase the beat

private:
    void resizeEvent(QResizeEvent *);

};