//-----------------------------------------------------------------------------
// KSlider control V1.3
//
// Copyright (C) Martin R. Jones 1997
//
// KSlider now maintained by Christian Esken (chris@magicon.prima.ruhr.de)
// Revision information.
// 1.0 KSlider by Martin R. Jones
// 1.1 All changes now by Christian Esken: sanity checks ( checkWidth() ).
// 1.2 Implemented rangeChange()
// 1.3 Reworked drawArrow(). For avoiding paint problems with some servers, I
// am now painting the arrow with "precise" lines.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpainter.h>
#include <qdrawutl.h>

#include "kslider.h"
#include "kslider.moc"

#define ARROW_LENGTH	13

KSlider::KSlider( QWidget *parent, const char *name )
  : QSlider( parent, name )
{
  setBackgroundMode(PaletteBackground);
  //setSteps(1,1);
}

KSlider::KSlider( Orientation o, QWidget *parent, const char *name )
  : QSlider( (QSlider::Orientation)o, parent, name )
{
  setBackgroundMode(PaletteBackground);
  //setSteps(1,1);
}

KSlider::KSlider( int _minValue, int _maxValue, int _Step, int _value,
		  Orientation o, QWidget *parent, const char *name )
  : QSlider( _minValue, _maxValue, _Step, _value, (QSlider::Orientation)o,
             parent, name )
{
  setBackgroundMode(PaletteBackground);
  setSteps(_Step,_Step); // !!!
}



void KSlider::drawShadeLine( QPainter *painter )
{
  if ( orientation() == Vertical )
    qDrawShadeLine(painter, 5, 3, 5, height()-3, colorGroup(), true, 1, 2);
  else
    qDrawShadeLine(painter, 3, 5, width()-3, 5, colorGroup(), true, 1, 2);
}


void KSlider::paintSlider(QPainter *painter, const QRect &rect )
{
  QPoint pos;

  pos = calcArrowPos( prevValue() );
  drawArrow( painter, false, pos );

  drawShadeLine(painter);
  drawTickMarks(painter);

  pos = calcArrowPos( value() );
  drawArrow( painter, true, pos );
}

void KSlider::drawArrow( QPainter *painter, bool show, const QPoint &pos )
{
  QPen        arrowPen;
  QPointArray array(5);

  // Select Horizontal or Vertical Polygon Array
  if ( orientation() == Vertical ) {
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    array.setPoint( 1, pos.x()-4, pos.y()+4 );
    array.setPoint( 2, pos.x()-ARROW_LENGTH, pos.y()+4 );
    array.setPoint( 3, pos.x()-ARROW_LENGTH, pos.y()-4 );
    array.setPoint( 4, pos.x()-4, pos.y()-4 );
  }
  else {
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    array.setPoint( 1, pos.x()+4, pos.y()-4 );
    array.setPoint( 2, pos.x()+4, pos.y()-ARROW_LENGTH );
    array.setPoint( 3, pos.x()-4, pos.y()-ARROW_LENGTH );
    array.setPoint( 4, pos.x()-4, pos.y()-4 );
  }

  // Select a base pen, then change parameters
  if ( show )
    arrowPen = QPen( colorGroup().light() );
  else
    arrowPen = QPen( backgroundColor() );

  arrowPen.setWidth(1);		// Yup, we REALLY want width 1, not 0 here !!
  painter->setPen(arrowPen);
  painter->setBrush( backgroundColor() );
  // painter->setRasterOp ( CopyROP );

  painter->drawPolygon( array );

  if ( show )
    {
      arrowPen = QPen( colorGroup().dark() );
      arrowPen.setWidth(1);	// Yup, we REALLY want width 1, not 0, here !!
      painter->setPen(arrowPen);
      painter->drawPolyline( array, 0, 3);

      // !!! This fixes a problem with a missing point! Qt Bug?
      // !!! I will wait for Qt1.3 to see the results
      //painter.drawPoint(array[1]);
    }
}


QSize KSlider::sizeHint() const
{
  QSize size;

  if ( orientation() == Vertical ) {
    size.setWidth( ARROW_LENGTH + 5 + 1 );
    size.setHeight( -1 );
  }
  else {
    size.setWidth( -1 );
    size.setHeight( ARROW_LENGTH + 5 + 1 );
  }
  return size;
}



QPoint KSlider::calcArrowPos( int val )
{
  QPoint p;
  int diffMaxMin = checkWidth();	// sanity check, Christian Esken

  if ( orientation() == Vertical ) {
    p.setY( height() - ( (height()-10) * ( val - minValue() )
				  / diffMaxMin + 5 ) );
    p.setX( ARROW_LENGTH );
  }
  else {
    p.setX( ( (width()-10) * ( val - minValue() )
	      / diffMaxMin + 5 ) );
    p.setY( ARROW_LENGTH );
  }
  return p;
}


void KSlider::drawTickMarks(QPainter *painter)
{
  QPen tickPen = QPen( colorGroup().dark() );
  tickPen.setWidth(1);	// Yup, we REALLY want width 1, not 0 here !!
  painter->setPen(tickPen);

  int i;
  int diffMaxMin = checkWidth();	// sanity check, Christian Esken
  if ( orientation() == Vertical ) {
    // first clear the tickmark area
    painter->fillRect(ARROW_LENGTH+1,0, ARROW_LENGTH + 6, height()-1, colorGroup().background()  );

    // draw ruler marks
    for ( i = 0; i <= maxValue() - minValue(); i += lineStep() ) {
      int pos = (height()-10) * i / diffMaxMin + 5;
      painter->drawLine( ARROW_LENGTH+1, pos, ARROW_LENGTH + 4, pos );
    }
    for ( i = 0; i <= maxValue() - minValue(); i += pageStep() ) {
      int pos = (height()-10) * i / diffMaxMin + 5;
      painter->drawLine( ARROW_LENGTH+1, pos, ARROW_LENGTH + 6, pos );
    }
  }
  else {
    // first clear the tickmark area
    painter->fillRect(0, ARROW_LENGTH+1, width()-1, ARROW_LENGTH + 6, colorGroup().background() );
    // draw ruler marks
    for ( i = 0; i <= maxValue() - minValue(); i += lineStep() ) {
      int pos = (width()-10) * i / diffMaxMin + 5;
      painter->drawLine( pos, ARROW_LENGTH+1, pos, ARROW_LENGTH + 4 );
    }
    for ( i = 0; i <= maxValue() - minValue(); i += pageStep() ) {
      int pos = (width()-10) * i / diffMaxMin + 5;
      painter->drawLine( pos, ARROW_LENGTH+1, pos, ARROW_LENGTH + 6 );
    }
  }
}

void KSlider::paintEvent( QPaintEvent * )
{
  QPainter painter;

  painter.begin( this );
  QRect rect(x(),y(),width(),height());
  paintSlider(&painter, rect);
  painter.end();
}

void KSlider::rangeChange()
{
  QSlider::rangeChange();

  //QPainter painter;
  //painter.begin( this );
  //drawTickMarks(&painter);  // when range changes, only tickmarks must be repainted  
  //painter.end();
  paintEvent(NULL);
}

int KSlider::checkWidth()
{
  int diff = maxValue() - minValue();
  if ( diff == 0)
    // If (max - min) has no "secure" value, set it to lineStep().  Christian Esken
    diff=lineStep();
  return diff;
}
