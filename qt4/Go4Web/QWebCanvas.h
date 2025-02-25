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

#ifndef QWEBCANVAS_H
#define QWEBCANVAS_H

#include <QWidget>
#include <QWebEngineView>

class TCanvas;
class TPad;
class TObject;
class QTimer;

class QWebCanvas : public QWidget {

   Q_OBJECT

public:
   QWebCanvas(QWidget *parent = nullptr);
   virtual ~QWebCanvas();

   /// returns canvas shown in the widget
   TCanvas *getCanvas() { return fCanvas; }

   bool isStatusBarVisible();
   bool isEditorVisible();

   void Modified();
   void Update();

signals:

   void CanvasDropEvent(QDropEvent*, TPad*);

   void CanvasUpdated();

   void SelectedPadChanged(TPad*);

   void PadClicked(TPad*,int,int);

   void PadDblClicked(TPad*,int,int);

public slots:

   void activateEditor(TPad *pad = nullptr, TObject *obj = nullptr);
   void activateStatusLine();

   void setStatusBarVisible(bool flag = true);

   void setEditorVisible(bool flag = true);

protected slots:

   void dropView(QDropEvent* event);

   void processRepaintTimer();

protected:

   void resizeEvent(QResizeEvent *event) override;

   void dropEvent(QDropEvent* event) override;

   double scaledPosition(int p) { return (double) p * fQtScalingfactor; }

   void SetPrivateCanvasFields(bool on_init);

   void ProcessCanvasUpdated() { emit CanvasUpdated(); }

   void ProcessActivePadChanged(TPad *pad) { emit SelectedPadChanged(pad); }

   void ProcessPadClicked(TPad *pad, int x, int y) { emit PadClicked(pad,x,y); }

   void ProcessPadDblClicked(TPad *pad, int x, int y) { emit PadDblClicked(pad,x,y); }

   QWebEngineView *fView{nullptr};  ///< qt webwidget to show

   TCanvas *fCanvas{nullptr};

   QTimer *fRepaintTimer{nullptr};

   double fQtScalingfactor{0};
};

#endif
