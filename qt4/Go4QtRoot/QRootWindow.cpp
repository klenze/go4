// $Id$
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "QRootWindow.h"

#include <iostream>

#include "TGFrame.h"
#include "TVirtualX.h"

#include <QMouseEvent>
#include <QCloseEvent>

#ifdef __GO4X11__
#include "TGo4LockGuard.h"
#else
#define TGo4LockGuard int
#endif

/** Pseudo root mainframe to wrap root composite frames like TGedEditor */
class TQRootFrame: public TGCompositeFrame {
   public:
      /** pretend a root frame for the externally created window of id wid */
      TQRootFrame(Window_t id) :
         TGCompositeFrame(gClient, id, new TGMainFrame(gClient->GetDefaultRoot(),100,100))
         {
         // we have to offer a real mainframe as top parent for root internal lists:
         // fParent=new TGMainFrame(gClient->GetDefaultRoot(),100,100);
         }

      virtual ~TQRootFrame()
      {
         delete fParent;
      }

      virtual void CloseWindow()
      {
         UnmapWindow();  // capture the close window
      }

      /*   virtual Bool_t HandleEvent(Event_t *event)
   {
      return TGCompositeFrame::HandleEvent(event);
   }
       */

};



///////////////////////////////////////////////////
// This is the widget to be embedded in qt:

QRootWindow::QRootWindow( QWidget *parent, const char *name, bool designermode) :
   QWidget(parent),
   fxRootwindow(nullptr),
   fbResizeOnPaint(kTRUE),fQtScalingfactor(1.0)
{
   setObjectName( name ? name : "QRootWindow");

   // set defaults
   setUpdatesEnabled( true );
   setMouseTracking(true);

   setFocusPolicy( Qt::TabFocus );
   setCursor( Qt::CrossCursor );

   if(!designermode) {
      // add the Qt::WinId to TGX11 interface
      fQtWinId = winId();
      fiWinid = gVirtualX->AddWindow((ULong_t)fQtWinId,145,400);
      //std::cout <<"QRootWindow ctor added window for "<<fQtWinId<<" with ROOT wid:"<<fiWinid<< std::endl;
      fxRootwindow = new TQRootFrame(gVirtualX->GetWindowID(fiWinid));
      fxRootwindow->Resize();
      if (parent) parent->installEventFilter( this );
      // JAM the following is pure empiric. hopefully default denominator won't change in future qt?
      fQtScalingfactor = (double) metric(QPaintDevice::PdmDevicePixelRatioScaled)/65536.;
   }
}

void QRootWindow::AddSubFrame(TGFrame* f, TGLayoutHints* l)
{
   fxRootwindow->AddFrame(f,l);
   fxRootwindow->MapWindow();
   fxRootwindow->MapSubwindows();
   fxRootwindow->Resize();
   ensurePolished();
   update();
   show();
}

void QRootWindow::SetEditable(bool on)
{
   fxRootwindow->SetEditable(on);
}

Bool_t QRootWindow::MapQMouseEvent(QMouseEvent *e, Event_t* rev)
{
   if(!e || !rev) return kFALSE;

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
   rev->fX = e->x();
   rev->fY = e->y();
   rev->fXRoot = e->globalX();
   rev->fYRoot = e->globalY();
#else
   rev->fX = e->position().x();
   rev->fY = e->position().y();
   rev->fXRoot = e->globalPosition().x();
   rev->fYRoot = e->globalPosition().y();
#endif

   // translate Qt event type:
   if(e->type() == QEvent::MouseButtonPress) rev->fType = kButtonPress;
   else if(e->type() == QEvent::MouseButtonRelease) rev->fType = kButtonRelease;
   else if(e->type() == QEvent::MouseButtonDblClick) rev->fType = kButtonDoubleClick;
   else if(e->type() == QEvent::MouseMove) rev->fType = kMotionNotify;
   else if(e->type() == QEvent::KeyPress) rev->fType = kGKeyPress;
   else if(e->type() == QEvent::KeyRelease) rev->fType = kKeyRelease;
   else rev->fType = kOtherEvent;

   // translate Qt state bits:
   rev->fState = 0;
   if(e->buttons() & Qt::LeftButton)
      rev->fState |= kButton1Mask;
   if(e->buttons() & Qt::RightButton)
      rev->fState |= kButton3Mask;
   if(e->buttons() & Qt::MiddleButton)
      rev->fState |= kButton2Mask;
   if(e->buttons() & Qt::MouseButtonMask)
      rev->fState |= kButton1Mask;

   if(e->modifiers() & Qt::ShiftModifier)
      rev->fState |= kKeyShiftMask;
   if(e->modifiers() & Qt::ControlModifier)
      rev->fState |= kKeyControlMask;
   if(e->modifiers() & Qt::AltModifier)
      rev->fState |= kKeyMod1Mask;
   if(e->modifiers() & Qt::MetaModifier)
      rev->fState |= kKeyMod1Mask;
   //if(e->buttons() & KeyButtonMask)
   //         rev->fState |= ShiftMask;
   //if(e->buttons() & Keypad)
   //         rev->fState |= ShiftMask;

   rev->fCode      = Qt::NoButton;    // button code
   if(e->button() == Qt::LeftButton)
      rev->fCode |= kButton1Mask;
   if(e->button() == Qt::RightButton)
      rev->fCode |= kButton3Mask;
   if(e->button() == Qt::MiddleButton)
      rev->fCode |= kButton2Mask;

   rev->fUser[0] = 0;
   rev->fWindow  = gVirtualX->GetWindowID(fiWinid); // we refer signals to this window
   rev->fSendEvent = 0;
   rev->fTime = 0; // this might cause problems with root doubleclick treatment?

   return kTRUE;
}



void QRootWindow::paintEvent( QPaintEvent * e)
{
   //TGo4LockGuard threadlock;
   //(void) threadlock; // suppress compiler warnings

   if(fxRootwindow) {
      WId nxid = winId();
      if(fQtWinId != nxid) {
         TGo4LockGuard threadlock;
         (void) threadlock; // suppress compiler warnings
         // may happen when this window is embedded to Qt workspace...
         std::cout <<"Warning: QRootWindow::paintEvent finds changed X window id: "<< (ULong_t) winId()<<", previous:"<<fQtWinId << std::endl;
         delete fxRootwindow; // should also remove old x windows!
         fQtWinId = nxid;
         fiWinid = gVirtualX->AddWindow((ULong_t) fQtWinId, width(), height());
         fxRootwindow = new TQRootFrame(gVirtualX->GetWindowID(fiWinid));
      }
      if(fbResizeOnPaint) {
         TGo4LockGuard threadlock;
         (void) threadlock; // suppress compiler warnings
         //std::cout <<"QRootWindow::paintEvent does TGCompositeFrame Resize ..." << std::endl;
         fxRootwindow->Resize(width(),height());
         gVirtualX->Update(1); // Xsync/flus
      }
   }
   QWidget::paintEvent(e);
}

bool QRootWindow ::eventFilter( QObject *o, QEvent *e )
{
   TGo4LockGuard threadlock;
   (void) threadlock; // suppress compiler warnings

   QMouseEvent* me = dynamic_cast<QMouseEvent*>(e);

   Event_t root_evnt;

   if (MapQMouseEvent(me, &root_evnt)) {
      if(fxRootwindow) fxRootwindow->HandleEvent(&root_evnt);
      return false;
   }

   if ( e->type() == QEvent::Close) {  // close
      delete fxRootwindow;
      fxRootwindow = nullptr;
      return false;
   }

   // standard event processing
   return QWidget::eventFilter( o, e );
}

void QRootWindow::closeEvent(QCloseEvent *e)
{
   if (fxRootwindow) {
      delete fxRootwindow;
      fxRootwindow = nullptr;
   }
   e->accept();
}

QRootWindow::~QRootWindow()
{
   if (fxRootwindow) {
      delete fxRootwindow;
      fxRootwindow = nullptr;
   }
}

TGCompositeFrame* QRootWindow::GetRootFrame()
{
   return fxRootwindow;
}

double QRootWindow::ScaledWidth()
{
    return fQtScalingfactor * width();
}

double QRootWindow::ScaledHeight()
{
    return fQtScalingfactor * height();
}
