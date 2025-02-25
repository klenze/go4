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

#ifndef QROOTCANVAS_H
#define QROOTCANVAS_H

#include <QWidget>

#ifdef __GO4DESIGNER__
#include <QtDesigner/QDesignerExportWidget>
#define GO4_WIDGET_EXPORT QDESIGNER_WIDGET_EXPORT
#else
#define GO4_WIDGET_EXPORT
#endif

#include "RtypesCore.h"
#include "Buttons.h"
#include "TVirtualX.h"

class TObject;
class TMethod;
class TObjLink;
class TVirtualPad;
class TPad;
class TCanvas;
class TBrowser;
class TContextMenu;
class TList;
class TVirtualPadEditor;
class TH1;

class QSignalMapper;
class QMenu;
class QAction;
class QTimer;
class QRootWindow;
class QFrame;
class QStatusBar;

/** This canvas uses Qt eventloop to handle user input
  *   @short Graphic Qt Widget based Canvas
  *
  * @authors Denis Bertini <d.bertini(at)gsi.de>
  * @version 2.0
  */

class GO4_WIDGET_EXPORT QRootCanvas : public QWidget {

   Q_OBJECT

   enum { act_Update = 1, act_Resize = 2 };

   public:
      QRootCanvas(QWidget *parent = nullptr);
      virtual ~QRootCanvas();

      TCanvas*          getCanvas() { return fCanvas; }
      int               getRootWid() { return fRootWindowId; }
      void              setMaskDoubleClick(bool on=true) { fMaskDoubleClick = on; }

      bool              showEventStatus() const;
      void              setShowEventStatus(bool s);

      void              setEditorFrame(QFrame *fr) { fEditorFrame = fr; }
      bool              isEditorAllowed();
      bool              isEditorVisible();
      void              toggleEditor();
      void              resizeEditor();
      void              activateEditor(TPad *pad, TObject *obj);
      void              cleanupEditor();

      void              setStatusBar(QStatusBar *bar) { fStatusBar = bar; }
      void              showStatusMessage(const char *msg);
      void              setStatusBarVisible(bool flag);
      bool              isStatusBarVisible();

   signals:
      /** signal which will be emitted when root selected pad is changed
        * via clicking the mid-mouse button (M. al-Turany) */
      void              SelectedPadChanged(TPad*);

      /** signal emitted when mouse clicks on pad  */
      void              PadClicked(TPad*, int, int);

      /** signal emitted when user produce left mouse double-click on pad  */
      void              PadDoubleClicked(TPad*, int, int);

      void              MenuCommandExecuted(TObject*, const char*);

      void              CanvasStatusEvent(const char*);

      void              CanvasDropEvent(QDropEvent*, TPad*);

      void              CanvasLeaveEvent();

      void              CanvasUpdated();

   public slots:

      void              cd(Int_t subpadnumber = 0);
      virtual void      Browse(TBrowser *b);
      void              Clear(Option_t *option="");
      void              Close(Option_t *option="");
      virtual void      Draw(Option_t *option="");
      virtual TObject  *DrawClone(Option_t *option="");
      virtual TObject  *DrawClonePad();
      virtual void      EditorBar();
      void              EnterLeave(TPad *prevSelPad, TObject *prevSelObj);
      void              FeedbackMode(Bool_t set);
      void              Flush();
      void              UseCurrentStyle();
      void              ForceUpdate();
      const char       *GetDISPLAY();
      TContextMenu     *GetContextMenu();
      Int_t             GetDoubleBuffer();
      Int_t             GetEvent();
      Int_t             GetEventX();
      Int_t             GetEventY();
      Color_t           GetHighLightColor();
      TVirtualPad      *GetPadSave();
      TObject          *GetSelected();
      Option_t         *GetSelectedOpt();
      TVirtualPad      *GetSelectedPad();
      Bool_t            GetShowEventStatus();
      Bool_t            GetAutoExec();
      Size_t            GetXsizeUser();
      Size_t            GetYsizeUser();
      Size_t            GetXsizeReal();
      Size_t            GetYsizeReal();
      Int_t             GetCanvasID();
      Int_t             GetWindowTopX();
      Int_t             GetWindowTopY();
      UInt_t            GetWindowWidth();
      UInt_t            GetWindowHeight();
      UInt_t            GetWw();
      UInt_t            GetWh();
      virtual void      GetCanvasPar(Int_t &wtopx, Int_t &wtopy, UInt_t &ww, UInt_t &wh);
      virtual void      HandleInput(EEventType button, Int_t x, Int_t y);
      Bool_t            HasMenuBar();
      void              Iconify();
      Bool_t            IsBatch();
      Bool_t            IsRetained();
      virtual void      ls(Option_t *option="");
      void              Modified(Bool_t=1);
      void              MoveOpaque(Int_t set=1);
      Bool_t            OpaqueMoving();
      Bool_t            OpaqueResizing();
      virtual void      Paint(Option_t *option="");
      virtual TPad     *Pick(Int_t px, Int_t py, TObjLink *&pickobj);
      virtual TPad     *Pick(Int_t px, Int_t py, TObject *prevSelObj);
      virtual void      Resize(Option_t *option="");
      void              ResizeOpaque(Int_t set=1);
      void              SaveSource(const char *filename="", Option_t *option="");
      virtual void      SetCursor(ECursor cursor);
      virtual void      SetDoubleBuffer(Int_t mode=1);
      void              SetWindowPosition(Int_t x, Int_t y);
      void              SetWindowSize(UInt_t ww, UInt_t wh);
      void              SetCanvasSize(UInt_t ww, UInt_t wh);
      void              SetHighLightColor(Color_t col);
      void              SetSelected(TObject *obj);
      void              SetSelectedPad(TPad *pad);
      void              Show();
      virtual void      Size(Float_t xsizeuser = 0, Float_t ysizeuser = 0);
      void              SetBatch(Bool_t batch=kTRUE);
      void              SetRetained(Bool_t retained=kTRUE);
      void              SetTitle(const char *title="");
      virtual void      ToggleEventStatus();
      virtual void      ToggleAutoExec();
      virtual void      Update();

      void              buildEditorWindow();
      void              executeMenu(int id);
      void              processRepaintTimer();

   protected:

      void      dropEvent( QDropEvent *Event ) override;
      void      dragEnterEvent( QDragEnterEvent *e ) override;
      void      mousePressEvent( QMouseEvent *e ) override;
      void      mouseReleaseEvent( QMouseEvent *e ) override;
      void      resizeEvent( QResizeEvent *e ) override;
      void      paintEvent( QPaintEvent *e ) override;
      void      mouseDoubleClickEvent(QMouseEvent* e ) override;
      void      mouseMoveEvent(QMouseEvent *e) override;
      void      wheelEvent( QWheelEvent* e) override;
      void      leaveEvent(QEvent *e) override;
      void      closeEvent( QCloseEvent * e) override;


      /** returns scaled point coordinate, for high dpi case*/
      double scaledPosition(int p) { return (double) p * fQtScalingfactor; }

      QPoint scaledMousePoint(QMouseEvent *ev);

      QPaintEngine * paintEngine () const override { return nullptr; }

      void              methodDialog(TObject* object, TMethod* method);
      QAction*          addMenuAction(QMenu* menu, QSignalMapper* map, const QString& text, int id);

      void              activateRepaint(int act);

      TCanvas*          fCanvas{nullptr};
      Int_t             fRootWindowId{0};
      WId               fQtWindowId; // current id of embedded canvas
      QTimer*           fRepaintTimer{nullptr}; // do not draw canvas immediately, postpone this on few miliseconds
      int               fRepaintMode{0}; // 0 - inactive, 1 - paint, 2 - resize, -1 - skip first repaint event

      QFrame*           fEditorFrame{nullptr};        // frame to show editor
      TVirtualPadEditor* fxPeditor{nullptr};          // ROOT editor
      QRootWindow*      fxRooteditor{nullptr};        // QtRoot window to embed ROOT editor
      TH1*              fDummyHisto{nullptr};         // dummy histogram used for editor cleanup

      QStatusBar       *fStatusBar{nullptr};

   private:
      bool              fMaskDoubleClick{false};
      double            fMousePosX{0};    // mouse position in user coordinate when activate menu
      double            fMousePosY{0};    // mouse position in user coordinate when activate menu

      TObject*          fMenuObj{nullptr};      // object use to fill menu
      TList*            fMenuMethods{nullptr};  // list of menu methods
      bool              fxShowEventStatus{false};

      double            fQtScalingfactor{0};
};

#endif






