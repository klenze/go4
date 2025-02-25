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

#include "QFitItem.h"

#include "TGo4FitPanel.h"

#include <QtCore/QString>


QFitItem::QFitItem(TGo4FitPanel* panel, QTreeWidgetItem* parent, TObject* iObj, int iObjectType, int iWidgetType, int iPopupMenuType, int iGraphType, int iTag ) :
   QTreeWidgetItem()
{
   setText(0, QString());
   setText(1, QString());

   fxPanel = panel;
   fxObj = iObj;
   fiObjectType = iObjectType;
   fiWidgetType = iWidgetType;
   fiPopupMenuType = iPopupMenuType;
   fiGraphType = iGraphType;
   fiTag = iTag;

   parent->addChild(this);

   if (fxPanel) fxPanel->UpdateItem(this, false);
}

QFitItem::~QFitItem()
{
   if (fxPanel) fxPanel->FitItemDeleted(this);
}

QFitItem* QFitItem::DefineWidgetItem()
{
   QFitItem* item = this;

   while (item) {
     if (item->WidgetType()!=-1) return item;
     item = item->Parent();
   }
   return nullptr;
}

QFitItem* QFitItem::DefineGraphItem()
{
  QFitItem* item = this;

  while (item) {
    if (item->GraphType()!=-1) return item;
    item = item->Parent();
  }
  return nullptr;
}

bool QFitItem::FindInParents(QFitItem* item)
{
   QFitItem* i = this;
   i = i->Parent();

   while (i) {
     if (i==item) return true;
     i = i->Parent();
   }

  return false;
}
