/*
 *        kmidi
 *
 * $Id$
 *            Copyright (C) 1997  Bernd Wuebben
 *                 wuebben@math.cornel.edu 
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include "log.moc"


myMultiEdit::myMultiEdit(QWidget *parent, const char *name)
  : QMultiLineEdit(parent, name){
}

myMultiEdit::~myMultiEdit(){
}

void myMultiEdit::insertChar(char c){
  QMultiLineEdit::insertChar( c);
}

void myMultiEdit::newLine(){
  QMultiLineEdit::newLine();
}


LogWindow::LogWindow(QWidget *parent, const char *name)
  : QDialog(parent, name, FALSE)
{
  setCaption("Info Window");

  text_window = new myMultiEdit(this,"logwindow");
  text_window->setGeometry(2,5,500, 300);
  //  text_window->setReadOnly(FALSE);

  statuslabel = new QLabel( this, "statuslabel" );
  
  statuslabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statuslabel->setText( "" );
  statuslabel->setAlignment( AlignLeft|AlignVCenter );
  statuslabel->setGeometry(2, 307, 500, 20);
  //statusPageLabel->setFont( QFont("helvetica",12,QFont::Normal) );

  dismiss = new QPushButton(this,"dismissbutton");
  dismiss->setGeometry(330,340,70,30);
  dismiss->setText("Dismiss");

  connect(dismiss,SIGNAL(clicked()),SLOT(hideit()));
 
  stringlist = new QStrList(TRUE); // deep copies
  stringlist->setAutoDelete(TRUE);
  /*  fline = new QFrame(this,"line");
  fline->setFrameStyle(QFrame::HLine |QFrame::Sunken);
  fline->setGeometry(2,332,398,5);*/
  adjustSize();
  setMinimumSize(width(),height());
  
  sltimer = new QTimer(this);
  connect(sltimer,SIGNAL(timeout()),this,SLOT(updatewindow()));
  timerset = false;

}


LogWindow::~LogWindow() {
}


void LogWindow::hideit(){

  this->hide();

}

void LogWindow::updatewindow(){

  timerset = false;


  //printf("in updatewindow\n");

  if (stringlist->count() != 0){

    /*    printf("inserting .. %d\n", stringlist->count());*/
    text_window->setAutoUpdate(FALSE);
    
    for(stringlist->first();stringlist->current();stringlist->next()){
      text_window->insertLine(stringlist->current(),-1);
    }

    text_window->setAutoUpdate(TRUE);
    text_window->setCursorPosition(text_window->numLines(),0,FALSE);


    text_window->update();
    stringlist->clear();

  }

}

void LogWindow::insertStr(char* str){


  QString string = str;

  if(string.find("Lyric:",0,TRUE) != -1)
    return;
  
  if(string.find("MIDI file",0,TRUE) != -1){
    stringlist->append(" ");
  }
 

  int index;
  //  char newl = '\n';
  if((index = string.find(";",0,TRUE)) != -1){
    string.replace(index,1,"\n");
  }



  stringlist->append(string.data());
 
  if(!timerset){
    sltimer->start(500,TRUE); // sinlge shot TRUE
    timerset = true;
  }

}
void LogWindow::clear(){

  if(text_window){
    
    text_window->clear();

  }

}
		  
void LogWindow::insertchar(char c) {

  QString stuff;

  if(c == '\r' || c == '\n') {
    if(c == '\n') {
      text_window->newLine();
    } 
  }
  else{
    text_window->insertChar(c);
  }
}


void LogWindow::statusLabel(const char *n) {

  statuslabel->setText(n);

}



/*
void LogWindow::keyPressEvent(QKeyEvent *k) {
}

*/

void LogWindow::resizeEvent(QResizeEvent* ){

  int w = width() ;
  int h = height();

  text_window->setGeometry(2,5,w - 2 ,h - 63);
  statuslabel->setGeometry(2, h - 56 , w -2 , 20);
  dismiss->setGeometry(w - 72 , h - 32, 70, 30);
  
}

void LogWindow::enter() {

  char character[3];
  character = "\r\n";

  text_window->append(character);

}

