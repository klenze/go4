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

#include "TGo4TreeViewer.h"

#include "TObjArray.h"
#include "TObjString.h"
#include "TH1.h"
#include "TClass.h"

#include <QApplication>

#include "TGo4BrowserProxy.h"

TGo4TreeViewer::TGo4TreeViewer(QWidget *parent, const char *name)
   : QGo4Widget(parent, name)
{
   setupUi(this);

   QObject::connect(TreeDrawBtn, &QPushButton::clicked, this, &TGo4TreeViewer::TreeDrawBtn_clicked);
   QObject::connect(TreeClearBtn, &QPushButton::clicked, this, &TGo4TreeViewer::TreeClearBtn_clicked);
   QObject::connect(NewHistBtn, &QPushButton::clicked, this, &TGo4TreeViewer::NewHistBtn_clicked);
   QObject::connect(XFieldEdt, &QGo4LineEdit::textDropped, this, &TGo4TreeViewer::XFieldEdt_dropped);
   QObject::connect(XFieldEdt, &QGo4LineEdit::returnPressed, this, &TGo4TreeViewer::TreeDrawBtn_clicked);
   QObject::connect(YFieldEdt, &QGo4LineEdit::textDropped, this, &TGo4TreeViewer::YFieldEdt_dropped);
   QObject::connect(YFieldEdt, &QGo4LineEdit::returnPressed, this, &TGo4TreeViewer::TreeDrawBtn_clicked);
   QObject::connect(ZFieldEdt, &QGo4LineEdit::textDropped, this, &TGo4TreeViewer::ZFieldEdt_dropped);
   QObject::connect(ZFieldEdt, &QGo4LineEdit::returnPressed, this, &TGo4TreeViewer::TreeDrawBtn_clicked);
   QObject::connect(CutEdt, &QGo4LineEdit::textDropped, this, &TGo4TreeViewer::cutEdit_dropped);
   QObject::connect(CutEdt, &QGo4LineEdit::returnPressed, this, &TGo4TreeViewer::TreeDrawBtn_clicked);
   QObject::connect(HistNameEdt, &QGo4LineEdit::returnPressed, this, &TGo4TreeViewer::TreeDrawBtn_clicked);

   fxTreeName = "";
   TreeDrawBtn->setEnabled(false);
   setAcceptDrops(false);
   setToolTip("Tree is not selected. Drag and drop leaf item from browser to X, Y or Z field");
}

void TGo4TreeViewer::ResetWidget()
{
   QGo4Widget::ResetWidget();
   XFieldEdt->clear();
   YFieldEdt->clear();
   ZFieldEdt->clear();
   HistNameEdt->clear();
   CutEdt->clear();
   TreeDrawBtn->setEnabled(false);
   fxTreeName = "";
   setToolTip("Tree is not selected. Drag and drop leaf item from browser to X, Y or Z field");
}

void TGo4TreeViewer::WorkWithLeaf(const char *itemname)
{
   ResetWidget();
   XFieldEdt->setText(itemname);
   ProcessDropEvent(XFieldEdt, true);
}

void TGo4TreeViewer::XFieldEdt_dropped()
{
   ProcessDropEvent(XFieldEdt, true);
}

void TGo4TreeViewer::YFieldEdt_dropped()
{
   ProcessDropEvent(YFieldEdt, true);
}

void TGo4TreeViewer::ZFieldEdt_dropped()
{
   ProcessDropEvent(ZFieldEdt, true);
}

void TGo4TreeViewer::cutEdit_dropped()
{
   ProcessDropEvent(CutEdt, false);
}

void TGo4TreeViewer::ProcessDropEvent(QGo4LineEdit* edt, bool caninit)
{
   QString value = edt->text();

   TGo4BrowserProxy* br = Browser();
   if (!br) return;

   if (fxTreeName.isEmpty()) {
      if (!caninit) {
          StatusMessage("First drop something on X, Y or Z field");
          edt->setText("");
          return;
      }

      TString treename;
      if (!br->DefineTreeName(value.toLatin1().constData(), treename)) {
          StatusMessage(QString("Invalid tree ") + value);
          edt->setText("");
          return;
      }
      fxTreeName = treename.Data();
      setToolTip(QString("Selected tree: ") + fxTreeName);

      TreeDrawBtn->setEnabled(true);
      AddLink(treename.Data(), "Tree");
   }

   TString leafname;
   if (!br->DefineLeafName(value.toLatin1().constData(), fxTreeName.toLatin1().constData(), leafname)) {
      edt->setText("");
      StatusMessage(QString("Invalid leaf name ") + value);
      return;
   }
   // count number of [ and replace by []
   // this means, by default accumulate over all members
   TObjArray* it=leafname.Tokenize("[");
   leafname=((TObjString *)it->First())->GetName();
   for(Int_t i=1;i<it->GetEntriesFast();i++) leafname.Append("[]");
   edt->setText(leafname.Data());
   edt->setFocus();
   it->Delete();
}

void TGo4TreeViewer::TreeDrawBtn_clicked()
{
   if (fxTreeName.isEmpty()) {
      StatusMessage("No tree specified");
      return;
   }

   if(XFieldEdt->text().isEmpty()) {
      StatusMessage("Treedraw without x coordinate expression");
      return;
   }

   TGo4BrowserProxy* br = Browser();
   if (!br) return;

   QApplication::setOverrideCursor(Qt::WaitCursor);

   TString createdh;

   br->PerformTreeDraw(fxTreeName.toLatin1().constData(),
                       XFieldEdt->text().toLatin1().constData(),
                       YFieldEdt->text().toLatin1().constData(),
                       ZFieldEdt->text().toLatin1().constData(),
                       CutEdt->text().toLatin1().constData(),
                       HistNameEdt->text().toLatin1().constData(),
                       createdh);

    if (createdh.Length() > 0) {
       HistNameEdt->setText(createdh.Data());
       DrawItem(createdh.Data());
    } else {
        if (!WhereItemDrawn(HistNameEdt->text().toLatin1().constData()))
           DrawItem(HistNameEdt->text());
        else
           br->RequestBrowserObject(HistNameEdt->text().toLatin1().constData());
    }

   QApplication::restoreOverrideCursor();
}

void TGo4TreeViewer::TreeClearBtn_clicked()
{
   ResetWidget();
}

void TGo4TreeViewer::NewHistBtn_clicked()
{
   if (fxTreeName.length()>0)
     AskToCreateObject(TH1::Class(), BrowserItemRemote(fxTreeName.toLatin1().constData()) ? 1 : 0);
}

void TGo4TreeViewer::requestedObjectCreated(const char *itemname, TClass* cl)
{
   if (cl && cl->InheritsFrom(TH1::Class()))
      HistNameEdt->setText(itemname);
}
