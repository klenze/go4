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

#ifndef TGO4MDIAREA_H
#define TGO4MDIAREA_H

#include <QMdiArea>

#include <QtCore/QEvent>

class TPad;
class TGo4ViewPanel;

/** @author S. Linev
  * @since 14.01.2014 */

class TGo4MdiArea : public QMdiArea {
   Q_OBJECT

   public:

      TGo4MdiArea(QWidget *parent = nullptr);
      virtual ~TGo4MdiArea();

      TGo4ViewPanel* GetActivePanel();

      TGo4ViewPanel *FindOtherPanel(TGo4ViewPanel *not_this);

      TPad* GetSelectedPad();
      void SetSelectedPad(TPad* pad);

      /** this method allows to set general properties for all go4 subwindows JAM*/
      QMdiSubWindow* AddGo4SubWindow(QWidget *widget, Qt::WindowFlags flags = Qt::Widget);

      void ResponseOnPanelEvent(int funcid, TGo4ViewPanel* panel, TPad* pad);

      static TGo4MdiArea* Instance();

   signals:
      void panelSignal(TGo4ViewPanel*, TPad*, int);

   protected slots:
      void subWindowActivatedSlot(QMdiSubWindow * window);

   private:
      TGo4ViewPanel*  fxActivePanel{nullptr};
      TPad*           fxActivePad{nullptr};
      TPad*           fxSelectedPad{nullptr};

      static TGo4MdiArea* gInstance;
};

#endif
