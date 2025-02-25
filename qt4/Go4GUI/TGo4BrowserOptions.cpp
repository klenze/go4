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

#include "TGo4BrowserOptions.h"

#include "TGo4BrowserProxy.h"
#include "TGo4ServerProxy.h"
#include "TGo4AnalysisProxy.h"
#include "TGo4DabcProxy.h"
#include "TGo4HttpProxy.h"

TGo4BrowserOptions::TGo4BrowserOptions(QWidget *parent, const char *name) :
   QGo4Widget(parent,name)
{
   setupUi(this);
   QObject::connect(RefreshBtn, &QToolButton::clicked, this, &TGo4BrowserOptions::RefreshBtn_clicked);
   QObject::connect(ClearBtn, &QToolButton::clicked, this, &TGo4BrowserOptions::ClearBtn_clicked);
   QObject::connect(StartMonitorBtn, &QToolButton::clicked, this, &TGo4BrowserOptions::StartMonitorBtn_clicked);
   QObject::connect(StopMonitorBtn, &QToolButton::clicked, this, &TGo4BrowserOptions::StopMonitorBtn_clicked);
   QObject::connect(FilterBox, QOverload<int>::of(&QComboBox::activated), this, &TGo4BrowserOptions::FilterBox_activated);

}

void TGo4BrowserOptions::StartWorking()
{
   AddLink("","Browser");
   UpdateView();
}

void TGo4BrowserOptions::UpdateView()
{
   int interval = Browser()->MonitoringPeriod()/1000;
   StartMonitorBtn->setEnabled(interval <= 0);
   StopMonitorBtn->setEnabled(interval > 0);

   if ((interval>1) && (IntervalSpin->value()!=interval))
      IntervalSpin->setValue(interval);
}

void TGo4BrowserOptions::linkedObjectUpdated(const char *linkname, TObject *obj)
{
   if (strcmp(linkname,"Browser") == 0)
      UpdateView();
}

void TGo4BrowserOptions::RefreshBtn_clicked()
{
   TGo4ServerProxy* an = Browser()->FindServer();
   if (an) an->RefreshNamesList();

   TObjArray prlist;
   Browser()->MakeDabcList(&prlist);
   for (Int_t n = 0; n <= prlist.GetLast(); n++) {
      TGo4DabcProxy *pr = dynamic_cast<TGo4DabcProxy *>(prlist.At(n));
      if (pr) pr->RefreshNamesList();
   }

   prlist.Clear();
   Browser()->MakeHttpList(&prlist);
   for (Int_t n = 0; n <= prlist.GetLast(); n++) {
      TGo4HttpProxy *pr = dynamic_cast<TGo4HttpProxy *>(prlist.At(n));
      if (pr) pr->RefreshNamesList();
   }

   Browser()->UpdateVisibleAnalysisObjects(false);
}

void TGo4BrowserOptions::ClearBtn_clicked()
{
   TGo4ServerProxy* an = Browser()->FindServer();
   if (an) {
      an->ClearAllAnalysisObjects();
   } else {
      TGo4ServerProxy* root_serv = Browser()->FindServer(0, kFALSE);
      if (root_serv) {
         TString cmd = root_serv->FindCommand("Clear");
         if (cmd.Length()>0) root_serv->SubmitCommand(cmd);
      }
   }
   Browser()->UpdateVisibleAnalysisObjects(false);
}


void TGo4BrowserOptions::StartMonitorBtn_clicked()
{
   int period = IntervalSpin->value();

   Browser()->ToggleMonitoring(period*1000);

   UpdateView();
}

void TGo4BrowserOptions::StopMonitorBtn_clicked()
{
   Browser()->ToggleMonitoring(0);

   UpdateView();
}

void TGo4BrowserOptions::FilterBox_activated(int indx)
{
   Browser()->SetItemsFilter(indx);
}

