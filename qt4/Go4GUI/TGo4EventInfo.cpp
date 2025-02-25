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

#include "TGo4EventInfo.h"
#include "TTree.h"
#include "TGo4ServerProxy.h"
#include "TGo4Slot.h"
#include "TGo4BrowserProxy.h"

const char *defMbsEventName = "MbsEvent101";

TGo4EventInfo::TGo4EventInfo(QWidget *parent, const char *name)
   : QGo4Widget(parent, name)
{
   setupUi(this);
   QObject::connect(RefreshButton, &QPushButton::clicked, this, &TGo4EventInfo::RefreshClicked);
   QObject::connect(PrintEventButton, &QPushButton::clicked, this, &TGo4EventInfo::PrintEventClicked);
   QObject::connect(MbsButton, &QPushButton::clicked, this, &TGo4EventInfo::MbsButton_clicked);
   setWindowTitle("Event info");
   ResetWidget();
}

bool TGo4EventInfo::IsAcceptDrag(const char *itemname, TClass* cl, int kind)
{
   return kind == TGo4Access::kndEventElement;
}

void TGo4EventInfo::DropItem(const char *itemname, TClass* cl, int kind)
{
   if (kind == TGo4Access::kndEventElement)
      WorkWithEvent(itemname);
}

void TGo4EventInfo::WorkWithEvent(const char *itemname)
{
   EventLbl->setText(itemname);

   bool ismbs = QString(itemname).contains(defMbsEventName);

   MbsPrintBox->setEnabled(ismbs);
   TString mbsitem = Browser()->FindItemInAnalysis(defMbsEventName);
   MbsButton->setEnabled(!ismbs && (mbsitem.Length() > 0));
   setFocus();
}

void TGo4EventInfo::ResetWidget()
{
   QGo4Widget::ResetWidget();
   EventLbl->setText("");
   MbsButton->setEnabled(true);
//   if (Browser()) {
//     TString mbsitem = Browser()->FindItemInAnalysis(defMbsEventName);
//     MbsButton->setEnabled(mbsitem.Length()>0);
//   } else
//     MbsButton->setEnabled(false);
   MbsPrintBox->setEnabled(false);
}

void TGo4EventInfo::linkedObjectUpdated(const char *linkname, TObject *obj)
{
   TTree* tr = dynamic_cast<TTree*>(obj);
   if (tr) {
      tr->Show(0);
      return;
   }

   if (obj) {
      obj->Print();
      return;
   }
}

void TGo4EventInfo::linkedObjectRemoved(const char * /*linkname*/)
{
}


void TGo4EventInfo::RefreshClicked()
{
   RemoveAllLinks();

   if (EventLbl->text().isEmpty()) return;
   QString evname = EventLbl->text();
   bool isremote = RemoteButton->isChecked();
   bool istree = TreeButton->isChecked();

   TGo4Slot* tgtslot = isremote ? nullptr : AddSlot("Event");

   TGo4BrowserProxy* br = Browser();
   if (!br) return;

   TString objname;
   TGo4ServerProxy* an = br->DefineAnalysisObject(evname.toLatin1().constData(), objname);
   if (an) an->RequestEventStatus(objname.Data(), istree, tgtslot);
}

void TGo4EventInfo::PrintEventClicked()
{
   if (EventLbl->text().isEmpty()) return;

   TString folder, name;
   TGo4Slot::ProduceFolderAndName(EventLbl->text().toLatin1().constData(), folder, name);

   TGo4ServerProxy* anal = GetAnalysis(EventLbl->text().toLatin1().constData());

   if (anal)
     anal->RemotePrintEvent(name.Data(),
                            PrintEventNum->value(),
                            PrintEventSid->value(),
                            HexBox->isChecked(),
                            LongBox->isChecked());
}

void TGo4EventInfo::MbsButton_clicked()
{
   TString itemname = Browser()->FindItemInAnalysis(defMbsEventName);

   // printf("Search item %s  found %s\n", defMbsEventName, itemname.Data());

   if (itemname.Length() > 0)
     WorkWithEvent(itemname.Data());
}
