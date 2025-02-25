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

#ifndef QROOTWINDOW_H
#define QROOTWINDOW_H

#include <QWidget>

#include "RtypesCore.h"

class QMouseEvent;
class QPaintEvent;
class TGLayoutHints;
class Event_t;
class TQRootFrame;
class TGFrame;
class TGCompositeFrame;

/** Qt widget wrapper for ROOT TGWindow class.
  * @author J.Adamczewski */

class QRootWindow : public QWidget {
   Q_OBJECT

   public:
      /** real ctor*/
      QRootWindow( QWidget *parent = nullptr, const char *name = nullptr, bool designermode = false);

      virtual ~QRootWindow();

      TGCompositeFrame* GetRootFrame();
      int GetRootWid() const { return fiWinid; }

      /** Add root subframe into this widget */
      void AddSubFrame(TGFrame* f, TGLayoutHints* l = nullptr);

      /** make this frame to current root master frame.*/
      void SetEditable(bool on=true);

      /** switch root frame resize mode on paint event:
       true=explicit resize, false=no resize of TQRootFrame*/
      void SetResizeOnPaint(bool on = true) { fbResizeOnPaint = on; }

      /** deliver width scaled with Qt5 screen scaling factor*/
      double ScaledWidth();

       /** deliver Height scaled with Qt5 screen scaling factor*/
      double ScaledHeight();

   protected:

      /** Event filter is used to pass Qt mouse events  root, after
        * translating them in MapQMouseEvent. Note that root grabs gui buttons
        * via x11 directly, thus this mechanism is not used for the regular
        *  root widgets! */
      bool eventFilter( QObject *, QEvent * ) override;
      void paintEvent( QPaintEvent *e ) override;
      void closeEvent( QCloseEvent * e) override;

      //virtual QPaintEngine * paintEngine () const {return nullptr;}

      /** translate Qt mouse event into root event structure. */
      Bool_t MapQMouseEvent(QMouseEvent *e, Event_t* rev);

      /** this is the pseudo root window embedded into this widget.
        * is used as parent for subwindows */
      TQRootFrame *fxRootwindow{nullptr};

      /** ROOT internal window index*/
      Int_t fiWinid{0};
      /** Qt window id as received from Qt (may change during runtime?)*/
      WId fQtWinId;

      /** switch if paint event does explicit resize of ROOT frame (default)
       * for embedded TGedEditor, we better disable it and resize editor from
       * parent widget*/
      Bool_t fbResizeOnPaint{kFALSE};

      double fQtScalingfactor{0};
};

#endif
