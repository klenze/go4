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

#include "TGo4Browser.h"

#include <iostream>

#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QApplication>
#include <QMenu>
#include <QHeaderView>
#include <QDrag>
#include <QMimeData>

#include "TClass.h"
#include "TClassTable.h"
#include "TROOT.h"
#include "TObjString.h"
#include "TCanvas.h"
#include "TGo4LockGuard.h"
#include "TGo4Slot.h"
#include "TGo4Iter.h"
#include "TGo4BrowserProxy.h"
#include "TGo4ServerProxy.h"
#include "TGo4QSettings.h"
#include "TGo4ViewPanel.h"

const int ColumnWidths[TGo4Browser::NColumns] = { 150, 40, 120, 90, 60, 100, 50 };
const char *ColumnNames[TGo4Browser::NColumns] = { "Name", "Flags", "Info", "Date", "Time", "Class", "Size" };
const int ColumnAllign[TGo4Browser::NColumns] = { Qt::AlignLeft, Qt::AlignLeft, Qt::AlignLeft, Qt::AlignLeft, Qt::AlignLeft, Qt::AlignLeft, Qt::AlignRight };

QTreeWidgetItem* nextSibling(QTreeWidgetItem* item)
{
   if (!item) return nullptr;

   QTreeWidgetItem* prnt = item->parent();
   if (!prnt) prnt = item->treeWidget()->invisibleRootItem();
   if (!prnt) return nullptr;

   int indx = prnt->indexOfChild(item) + 1;
   if (indx >= prnt->childCount()) return nullptr;
   return prnt->child(indx);
}


TGo4Browser::TGo4Browser(QWidget *parent, const char *name) :
   QGo4Widget(parent,name)
{
   setupUi(this);

   QObject::connect(ListView, &QGo4BrowserTreeWidget::itemDoubleClicked, this, &TGo4Browser::ListView_doubleClicked);
   QObject::connect(ListView, &QGo4BrowserTreeWidget::customContextMenuRequested, this, &TGo4Browser::ListView_customContextMenuRequested);

   setAcceptDrops(false);
   setCanDestroyWidget(false);

   fbUpdateTimerActive = false;

   ListView->setSortingEnabled(false);
   ListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
   ListView->setAcceptDrops(false);
   ListView->viewport()->setAcceptDrops(true);

   ListView->setRootIsDecorated(false);

   // ListView->setDragEnabled(true);

   for(int indx = 0; indx < NColumns; indx++) {
      int width = -1;
      if ((indx == 0) || (indx == 2)) width = ColumnWidths[indx];
      width = go4sett->getBrowserColumn(ColumnNames[indx], width);
      fVisibleColumns[indx] = width > 0;

      ListView->headerItem()->setText(indx, ColumnNames[indx]);

      ListView->header()->setSectionHidden(indx, ! fVisibleColumns[indx]);
      ListView->headerItem()->setTextAlignment(indx, ColumnAllign[indx]);

      ListView->header()->resizeSection(indx, width>0 ? width : ColumnWidths[indx]);
   }

   // not in .ui file while designer brakes this connection
   QObject::connect(ListView, &QGo4BrowserTreeWidget::RequestDragObject, this, &TGo4Browser::RequestDragObjectSlot);

//   QObject::connect(ListView, &QGo4BrowserTreeWidget::ItemDropAccept, this, &TGo4Browser::ItemDropAcceptSlot);

   QObject::connect(ListView, &QGo4BrowserTreeWidget::ItemDropProcess, this, &TGo4Browser::ItemDropProcessSlot);

   QObject::connect(ListView->header(), &QHeaderView::sectionResized, this, &TGo4Browser::HeaderSectionResizedSlot);

   QObject::connect(ListView->header(), &QHeaderView::customContextMenuRequested, this, &TGo4Browser::Header_customContextMenuRequested);

   ListView->header()->setToolTip(
     QString("You can change selected browser columns\n") +
             "by activating RMB. Flags has following meaning\n" +
             "m - monitored,\ns - static,\n" +
             "d - can be deleted,\np - protected against delete\n" +
             "r - can not be reset (read only),\nw - can be reset");

   ListView->setContextMenuPolicy(Qt::CustomContextMenu);
   ListView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
}


void TGo4Browser::StartWorking()
{
   AddLink("","Browser");
   ShootUpdateTimer();
}

void TGo4Browser::linkedObjectUpdated(const char *linkname, TObject *obj)
{
   if (strcmp(linkname,"Browser") == 0) {
      ShootUpdateTimer();
   } else if (strcmp(linkname,"TGo4Slot::evSubslotUpdated") == 0) {
      TGo4Slot* itemslot = dynamic_cast<TGo4Slot*> (obj);
      if (itemslot)
         SetViewItemProperties(itemslot, FindItemFor(itemslot));
   }
}


void TGo4Browser::RequestDragObjectSlot(QDrag** res)
{
   *res = nullptr;

   if (!ListView->currentItem()) return;

   QString fullname = FullItemName(ListView->currentItem());

   *res = new QDrag(this);
   QMimeData *mimeData = new QMimeData;
   mimeData->setText(fullname);

   (*res)->setMimeData(mimeData);
}

void TGo4Browser::ItemDropAcceptSlot(void* item, void* d, bool* res)
{
   *res = false;
   QString tgtname = FullItemName((QTreeWidgetItem*) item);
   const QMimeData* mime = (const QMimeData*) d;
   if (!mime->hasText()) return;

   QString dropname = mime->text();

   TGo4Slot* tgtslot = Browser()->ItemSlot(tgtname.toLatin1().constData());
   TGo4Slot* dropslot = Browser()->ItemSlot(dropname.toLatin1().constData());

   if (!tgtslot || !dropslot) return;
   if (Browser()->ItemKind(tgtslot) != TGo4Access::kndFolder) return;

   if (dropslot->GetParent() == tgtslot) return;

   *res = true;
}

void TGo4Browser::ItemDropProcessSlot(void* item, void* d)
{
   QString tgtname = FullItemName((QTreeWidgetItem*) item);
   const QMimeData* mime = (const QMimeData*) d;
   if (!mime->hasText()) return;
   Browser()->ProduceExplicitCopy(mime->text().toLatin1().constData(), tgtname.toLatin1().constData(), go4sett->getFetchDataWhenCopy());
}

void TGo4Browser::ResetWidget()
{
}

TGo4BrowserProxy* TGo4Browser::BrowserProxy()
{
   return GetLinkedCast<TGo4BrowserProxy>("Browser");
}

QString TGo4Browser::FullItemName(QTreeWidgetItem* item)
{
   QString name;
   if (item) {
      name = item->text(0);
      while (item->parent()) {
         item = item->parent();
         name = item->text(0) + "/" + name;
      }
   }
   return name;
}

QTreeWidgetItem* TGo4Browser::FindItemFor(TGo4Slot* slot)
{
   TGo4BrowserProxy* br = BrowserProxy();
   if (!br) return nullptr;

   TString itemname;
   if (!br->BrowserItemName(slot, itemname)) return nullptr;
   QString iname = itemname.Data();

   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it ) {
      QString fullname = FullItemName(*it);
      if (fullname == iname) return *it;
   }
   return nullptr;
}

void TGo4Browser::SetViewItemProperties(TGo4Slot* itemslot, QTreeWidgetItem* item)
{
   if (!itemslot || !item) return;

   TGo4BrowserProxy* br = BrowserProxy();

   bool mon = br->IsItemMonitored(itemslot);
   bool remote = br->IsItemRemote(itemslot);
   int kind = br->ItemKind(itemslot);
   int cando = br->ItemCanDo(itemslot);
   const char *iteminfo = TGo4BrowserProxy::ItemInfo(itemslot);
   const char *classname = TGo4BrowserProxy::ItemClassName(itemslot);

   bool visible = false;

   switch (br->GetItemFilter()) {
      case 1: visible = (itemslot->GetAssignedObject() != nullptr); break;
      case 2: visible = mon; break;
      default: visible = true;
   }

   if (TGo4BrowserProxy::CanDragItem(cando))
      item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
   else
      item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);

   item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
   if (kind==TGo4Access::kndFolder) {
      TGo4Slot* memslot = br->BrowserMemorySlot();
      if (itemslot->IsParent(memslot) || (itemslot==memslot))
        item->setFlags(item->flags() | Qt::ItemIsDropEnabled);
   }

   if (visible)
     item->setText(NColumns,"visible");
   else
     item->setText(NColumns,"hidden");

   // make visible all folders, where item is located
   if (visible && (br->GetItemFilter()>0)) {
      QTreeWidgetItem* parent = item->parent();
      while (parent) {
         parent->setText(NColumns, "visible");
         parent = parent->parent();
      }
   }

   QString flags;

//   if (kind == TGo4Access::kndObject)
//     if (itemslot->GetAssignedObject())
//       flags += "c";
//     else
//       flags += "a";

   if (remote && (kind == TGo4Access::kndObject)) {
      if (mon)
         flags += "m";
      else
         flags += "s";
      Int_t delprot, clearprot;
      br->GetProtectionBits(itemslot, delprot, clearprot);

      switch (delprot) {
         case 0: flags+="d"; break;
         case 1: flags+="p"; break;
         default: flags+="-"; break;
      }

      switch (clearprot) {
         case 0: flags+="w"; break;
         case 1: flags+="r"; break;
         default: flags+="-"; break;
      }
   }
   item->setText(1, flags);
   item->setText(2, iteminfo);
   item->setText(3, br->ItemDate(itemslot));
   item->setText(4, br->ItemTime(itemslot));
   item->setText(5, classname);
   QString sizelbl = "";
   int sizeinfo = br->ItemSizeInfo(itemslot);
   int calcsize = br->GetCalcSize(itemslot);
   if (sizeinfo>0) sizelbl = QString::number(sizeinfo); else
   if (calcsize>0) sizelbl = QString("= ") + QString::number(calcsize);
   item->setText(6, sizelbl);
}

void TGo4Browser::updateListViewItems()
{
   TGo4LockGuard lock;

   fbUpdateTimerActive = false;

   // first update list of visible columns

   for (int indx = 0; indx < NColumns; indx++)
      ListView->header()->setSectionHidden(indx, !fVisibleColumns[indx]);

   TGo4BrowserProxy* br = BrowserProxy();
   if (!br) return;

   ServiceCall("UpdateGuiLayout");

   TGo4Slot* topslot = br->BrowserTopSlot();
   if (!topslot) return;

   checkVisisbilityFlags(true);

//   slot->Print("");

   QTreeWidgetItem* curfold = nullptr;
   QTreeWidgetItem* curitem = ListView->topLevelItem(0);
   QTreeWidgetItem* previtem = nullptr;

   TGo4Iter iter(topslot, kTRUE);

   TObjArray testedClasses;

   while (true) {
       Bool_t res = iter.next();

      // go to top folders and remove rest items
      Int_t levelchange = iter.levelchange();
      while (levelchange++ < 0) {

          while (curitem) {
            QTreeWidgetItem* next = nextSibling(curitem);
            delete curitem;
            curitem = next;
          }

          if (!curfold) break;

          curitem = nextSibling(curfold);
          previtem = curfold;
          curfold = curfold->parent();
      }

      if (!res) break;

      // delete all slots in folder, which has another name
      while (curitem && (strcmp(iter.getname(), curitem->text(0).toLatin1().constData()) != 0)) {
         QTreeWidgetItem* next = nextSibling(curitem);
         delete curitem;
         curitem = next;
      }

      TGo4Slot* curslot = iter.getslot();
      if (!curslot) {
         std::cerr << "************* ERROR in gui slots ****************** " << std::endl;
         return;
      }

      const char *classname = br->ItemClassName(curslot);
      Int_t itemkind = br->ItemKind(curslot);
      TClass* itemclass = nullptr;

      if (classname && (strlen(classname) > 0) && (testedClasses.FindObject(classname) == nullptr)) {

         itemclass = gROOT->GetClass(classname, kFALSE);

         // if dictionary existing (library is loaded) force creation of TClass object
         if (!itemclass && TClassTable::GetDict(classname))
            itemclass = gROOT->LoadClass(classname);

         if (!itemclass)
           testedClasses.Add(new TNamed(classname,""));
      }

      TString pixmap;
      int cando = TGo4BrowserProxy::DefineItemProperties(itemkind, itemclass, pixmap);
      if (pixmap.Length()>0) pixmap = TString(":/icons/") + pixmap;
      TGo4BrowserProxy::SetItemCanDo(curslot, cando);

      if (!curitem) {
         if (!curfold)
            curitem = new QTreeWidgetItem(ListView, previtem);
         else
            curitem = new QTreeWidgetItem(curfold, previtem);
      }

      curitem->setText(0, iter.getname());

      if ((pixmap.Length()>0) && curitem->icon(0).isNull())
        curitem->setIcon(0, QIcon(pixmap.Data()));

      if ((curitem->text(0).contains(".TGo4EventElement") && (itemkind==TGo4Access::kndTreeBranch)) ||
          ((curitem->text(0)=="TGo4EventElement") && (itemkind==TGo4Access::kndFolder))) {
         curitem->setHidden(go4sett->getHideTGo4EventElement());
      }

      SetViewItemProperties(curslot, curitem);

      if (iter.isfolder()) {
         curfold = curitem;
         curitem = curfold->child(0);
         previtem = nullptr;
      } else {
         // remove any substructures if any
         while (curitem->child(0))
           delete curitem->child(0);
         previtem = curitem;
         curitem = nextSibling(curitem);
      }
   }

   while (curitem) {
      QTreeWidgetItem* next = nextSibling(curitem);
      delete curitem;
      curitem = next;
   }

   if (br->GetItemFilter() > 0)
     checkVisisbilityFlags(false);

   ListView->update();
   ListView->viewport()->update();

   testedClasses.Delete();
}

void TGo4Browser::checkVisisbilityFlags(bool showall)
{
   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it ) {
      QTreeWidgetItem* item = *it;
      if (showall || !item->parent())
        item->setHidden(false);
      else
        item->setHidden(item->text(NColumns)!="visible");
   }
}

void TGo4Browser::DisplaySelectedItems()
{

   TGo4BrowserProxy* br = BrowserProxy();

   int npads = 0;

   {
      QTreeWidgetItemIterator it(ListView);
      for ( ; *it; ++it )
       if ((*it)->isSelected() &&
           canDrawItem(*it)) npads++;
   }

   if (npads == 0) return;

   TGo4ViewPanel* newpanel = CreateViewPanel(npads);
   TPad* subpad = nullptr;

   int cnt = 0;
   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it )
     if ( (*it)->isSelected() && canDrawItem(*it)) {
        QString itemname = FullItemName(*it);

        subpad = newpanel->GetSubPad(newpanel->GetCanvas(), cnt++, true);

        DrawItem(itemname, newpanel, subpad, false);
     }
   newpanel->ShootRepaintTimer();
//   newpanel->SetActivePad(subpad);
}


void TGo4Browser::SuperImposeSelectedItems()
{
   TGo4ViewPanel* newpanel = nullptr;

   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it )
     if ( (*it)->isSelected() && canDrawItem(*it)) {
        if (!newpanel) {
           newpanel = CreateViewPanel();
           newpanel->SetPadSuperImpose(newpanel->GetCanvas(), true);
        }

        QString itemname = FullItemName(*it);

        DrawItem(itemname, newpanel, newpanel->GetCanvas(), false);
     }
   if (newpanel)
     newpanel->ShootRepaintTimer();
}

void TGo4Browser::ListView_doubleClicked(QTreeWidgetItem* item, int ncol)
{
   if (!item) return;

   QString fullname = FullItemName(item);

   TGo4BrowserProxy* br = BrowserProxy();

   int cando = br->ItemCanDo(fullname.toLatin1().constData());
   TGo4Slot* itemslot = br->ItemSlot(fullname.toLatin1().constData());

   if (go4sett->getDrawOnceFlag()) {
      TGo4ViewPanel* panel = WhereItemDrawn(fullname.toLatin1().constData());

      QWidget* mdi = panel ? panel->parentWidget() : nullptr;

      if (mdi)  {
         if (mdi->isMinimized()) mdi->showNormal();
         mdi->activateWindow();
         mdi->raise();
         mdi->show();
         mdi->setFocus();
      }

      if (panel) return;
   }

   if (TGo4BrowserProxy::CanDrawItem(cando))
     DrawItem(fullname, nullptr, nullptr, true);
   else if (TGo4BrowserProxy::CanEditItem(cando))
      EditItem(fullname);
   else if (TGo4BrowserProxy::CanExpandItem(cando)) {
      item->setExpanded(true);
      ExpandItem(fullname);
   } else if (TGo4BrowserProxy::CanExecuteItem(cando)) {
      ExecuteItem(fullname);
   }
}

void TGo4Browser::Header_customContextMenuRequested(const QPoint & pos)
{
   QMenu menu;

   for(int indx = 1; indx < NColumns; indx++)
      QObject::connect(CreateChkAction(&menu, ColumnNames[indx], fVisibleColumns[indx]),
                        &QAction::toggled, [this,indx]() { ColumnToggled(indx); });

   menu.exec(ListView->header()->mapToGlobal(pos));
}


void TGo4Browser::ListView_customContextMenuRequested(const QPoint& pos)
{
   if (ListView->header()->logicalIndexAt(pos) != 0) {
      Header_customContextMenuRequested(pos);
      return;
   }

   QMenu menu;

   QTreeWidgetItem* item = ListView->itemAt(pos);
   TGo4BrowserProxy* br = BrowserProxy();
   TGo4Slot* memslot = br->BrowserMemorySlot();
   TGo4Slot* analslot = br->FindServerSlot(false, 1);

   bool istopmemory = false;

   int nitems = 0, nmemory = 0, nclose = 0, ndraw = 0, nsuperimpose = 0, si_kind = -1,
       nremote = 0, nanalysis = 0, nmonitor = 0, nclear = 0, nclearlocal = 0, nclearproton = 0,
       nclearprotoff = 0, ndelprotoff = 0, nobjects = 0, nfolders = 0, nedits = 0, ninfo = 0,
       nexport = 0, ndelete = 0, nassigned = 0, nexpand = 0, nexecute = 0;

   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it )
      if ((*it)->isSelected()) {
         QString fullname = FullItemName(*it);
         TGo4Slot* itemslot = br->ItemSlot(fullname.toLatin1().constData());
         if (!itemslot) continue;
         nitems++;

         int cando = br->ItemCanDo(itemslot);
         int kind = br->ItemKind(itemslot);
         const char *itemclassname = br->ItemClassName(itemslot);

         bool ismemitem = itemslot->IsParent(memslot);

         if (br->IsCanDelete(itemslot) || ismemitem)
           ndelete++;

         if (kind==TGo4Access::kndObject)
            nobjects++;

         if (kind==TGo4Access::kndFolder)
           nfolders++;

         if (TGo4BrowserProxy::CanExpandItem(cando)) nexpand++;

         if (TGo4BrowserProxy::CanExecuteItem(cando)) nexecute++;

         if (TGo4BrowserProxy::CanDrawItem(cando)) {
           ndraw++;
           TClass* cl = gROOT->GetClass(itemclassname);
           if (cl) {
              if (cl->InheritsFrom("TH1")) {
                 if (!cl->InheritsFrom("TH2") && !cl->InheritsFrom("TH3")) {
                    if ((si_kind < 0) || (si_kind == 1)) {
                       si_kind=1;
                       nsuperimpose++;
                    }
                 }
              } else
              if (cl->InheritsFrom("TGraph")) {
                if ((si_kind<0) || (si_kind==2)) {
                   si_kind=2;
                   nsuperimpose++;
                }
              }
           // JAM test for TF1:
              else
                if (cl->InheritsFrom("TF1")) {
                  if ((si_kind<0) || (si_kind==3)) {
                    si_kind=3;
                    nsuperimpose++;
                  }
                }
           }
         }

         if (TGo4BrowserProxy::CanEditItem(cando))
           nedits++;

         if (TGo4BrowserProxy::CanCloseItem(cando))
           nclose++;

         if (TGo4BrowserProxy::CanInfoItem(cando))
           ninfo++;

         istopmemory = (itemslot==memslot);

         if (ismemitem || istopmemory)
           nmemory++;

         if (ismemitem && TGo4BrowserProxy::CanClearItem(cando))
            nclearlocal++;

         if ((kind==TGo4Access::kndObject) && itemslot->GetAssignedObject()) {
            nassigned++;
            if (TGo4BrowserProxy::CanExportItem(cando))
              nexport++;
         }

         bool isanalysisitem = (itemslot==analslot) || itemslot->IsParent(analslot);

         if (isanalysisitem) nanalysis++;

         if (itemclassname && (strcmp(itemclassname,"TGo4DabcProxy") == 0)) nremote++;

         if (itemclassname && (strcmp(itemclassname,"TGo4ServerProxy") == 0)) nremote++;

         if (br->IsItemRemote(itemslot)) {
            nremote++;

            if (isanalysisitem) {

               if (kind==TGo4Access::kndObject) {
                  Int_t delprot, clearprot;
                  br->GetProtectionBits(itemslot, delprot, clearprot);
                  if (clearprot == 1)
                     nclearproton++;
                  else if (clearprot == 0)
                     nclearprotoff++;
                  if (delprot == 0)
                     ndelprotoff++;
               }

               if (TGo4BrowserProxy::CanClearItem(cando) || (kind==TGo4Access::kndFolder))
                 nclear++;
            }
         }

         if (br->IsItemMonitored(itemslot)) nmonitor++;
      }

   auto addAct = [this, &menu](const QString& iconname, const QString& text, int id, bool enabled = true, QMenu *submenu = nullptr) {
      auto act = iconname.isEmpty() ?  new QAction(text, &menu)
                                    :  new QAction(QIcon(QString(":/icons/") + iconname), text, &menu);
      act->setEnabled(enabled);
      if (submenu)
         submenu->addAction(act);
      else
         menu.addAction(act);
      QObject::connect(act, &QAction::triggered, [this, id]() { ContextMenuActivated(id); });
   };

   addAct("chart.png", "Plot", 11, ndraw > 0);

   addAct("superimpose.png", "Superimpose",  12, (ndraw>1) && (nsuperimpose==ndraw));

   if (nexpand > 0)
       addAct("zoomlim.png", "Expand", 28);

   if (nexecute == 1)
      addAct("zoomlim.png", "Execute", 29);

   addAct("right.png", "Fetch item(s)",  18, (nfolders > 0) || (nobjects > 0));

   addAct("saveall.png", "Save selected...",  13, (nobjects > 0) || ((nfolders == 1) && (nitems == 1)));

   // organize export submenu
   if (nexport > 0) {

      QMenu* expmenu = menu.addMenu(QIcon(":/icons/export.png"), "Export to");

      addAct("", "ASCII", 141, true, expmenu);
      addAct("", "Radware", 142, true, expmenu);

   } else {
      addAct("export.png", "Export to", 14, false);
   }

   addAct("info.png", "Info...",  15, (ninfo > 0));

   addAct("control.png", "Edit...", 16, (nedits > 0));

   QString dellabel = "Delete item";
   QString delbutton = "delete.png";
   if ((nclose > 0) && (ndelete == 0)) {
      dellabel = "Close item";
      if (nclose > 1)
         dellabel += "s";
      delbutton = "close.png";
   } else if ((nclose == 0) && (ndelete > 0)) {
      dellabel = "Delete item";
      if (ndelete > 1)
         dellabel += "s";
   } else if ((nclose > 0) && (ndelete > 0)) {
      dellabel = "Close/delete items";
   }

   addAct(delbutton, dellabel, 17, (nclose > 0) || (ndelete > 0));

   addAct("copyws.png", "Copy to Workspace", 19, (nobjects > 0) || ((nfolders == 1) && (nitems == 1)));

   addAct("editcopy.png", "Copy to clipboard", 20, (nobjects>0) || (nfolders>0));

   if ((nremote > 0) || (nanalysis > 0)) {

       menu.addSeparator();

       addAct("monitor.png", "Monitor item(s)",  21, ((nobjects>0) && (nremote>0) && (nmonitor<nremote)) || ((nfolders==1) && (nitems==1)));

       addAct("Stop.png", "Stop item(s) monitoring", 22, ((nobjects>0) && (nremote>0) && (nmonitor>0)) || ((nfolders==1) && (nitems==1)));

       addAct("clear.png", "Clear (Reset to 0)", 23, (nclear > 0));

       addAct("clear_nok.png", "Set Clear protection", 24, (nclearprotoff > 0));

       addAct("clear_ok.png", "Unset Clear protection", 25, (nclearproton > 0));

       addAct("delete.png", "Delete from analysis", 26, (ndelprotoff > 0));

       addAct("refresh.png", "Refresh namelist", 27);
   }

   if ((nmemory > 0) && (nmemory == nitems)) {
       menu.addSeparator();

       addAct("crefolder.png", "Create folder",  41, (nmemory==1) && (nfolders==1));

       addAct("rename.png", "Rename object",  42, (nmemory==1) && !istopmemory);

       addAct("clear.png", "Clear object(s)", 44, (nclearlocal > 0));

       addAct(":/icons/editpaste.png", "Paste from clipboard",  43, br->IsClipboard() && (nmemory==1) && (nfolders==1));
   }

   menu.exec(ListView->viewport()->mapToGlobal(pos));
}

void TGo4Browser::ColumnToggled(int indx)
{
   if ((indx <= 0) || (indx >= NColumns)) return;
   fVisibleColumns[indx] = !fVisibleColumns[indx];

   HeaderSectionResizedSlot(0, 0, 0);

   ShootUpdateTimer();
}

void TGo4Browser::HeaderSectionResizedSlot(int, int, int)
{
   int ncolumn = 0;
   for (int indx = 0; indx < NColumns; indx++) {
      int width = -1;
      if (fVisibleColumns[indx]) {
         width = ListView->columnWidth(ncolumn++);
         if (width == 0) width = ColumnWidths[indx];
      }
      go4sett->setBrowserColumn(ColumnNames[indx], width);
   }
}

void TGo4Browser::ContextMenuActivated(int id)
{
   switch (id) {
      case 11: DisplaySelectedItems(); return;
      case 12: SuperImposeSelectedItems(); return;
      case 13: SaveSelectedItems(); return;
      case 141: ExportSelectedItems("ASCII format"); return;
      case 142: ExportSelectedItems("Radware format"); return;
   }

   TGo4BrowserProxy* br = BrowserProxy();

   TGo4ServerProxy* anrefresh = nullptr;
   TGo4ServerProxy* servrefresh = nullptr;

   if (id == 20) br->ClearClipboard();

   if (id == 19)
     QApplication::setOverrideCursor(Qt::WaitCursor);

   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it )
      if ((*it)->isSelected()) {
         QString itemname = FullItemName(*it);
         TGo4Slot* itemslot = br->ItemSlot(itemname.toLatin1().constData());
         if (!itemslot) continue;
         int cando = br->ItemCanDo(itemslot);
         int kind = br->ItemKind(itemslot);

         switch(id) {
            case 15: {
               if (TGo4BrowserProxy::CanInfoItem(cando)) {
                  ShowItemInfo(itemname);
                  return;
               }
               break;
            }

            case 16: {
               if (TGo4BrowserProxy::CanEditItem(cando)) {
                  EditItem(itemname);
                  return;
               }
               break;
            }

            case 17: {
               br->DeleteDataSource(itemslot);
               break;
            }

            case 18: { // fetch item from the data source
                br->FetchItem(itemname.toLatin1().constData());
                break;
            }

            case 19: {
               br->ProduceExplicitCopy(itemname.toLatin1().constData(), 0, go4sett->getFetchDataWhenCopy());
               break;
            }

            case 20: {
               br->AddToClipboard(itemname.toLatin1().constData());
               break;
            }

            case 21:    // toggle monitor on
            case 22: {  // toggle monitor off
               br->SetItemMonitored(itemslot, id==21);
               ShootUpdateTimer();
               break;
            }

            case 23: {  // clear
               TString objname;
               TGo4ServerProxy* an = br->DefineAnalysisObject(itemname.toLatin1().constData(), objname);
               if (an) {
                  an->ClearAnalysisObject(objname.Data());
                  // if clear folder, request all objects which were requested before
                  if (kind==TGo4Access::kndFolder) {
                     TGo4Iter iter(itemslot, kTRUE);
                     while (iter.next()) {
                        TGo4Slot* subslot = iter.getslot();
                        if (subslot->GetAssignedObject())
                           subslot->Update(kFALSE);
                     }
                  } else
                    if (itemslot->GetAssignedObject())
                       itemslot->Update(kFALSE);
               }
               break;
            }

            case 24:     // set clear protect
            case 25: {   // unset clear protect
               TString objname;
               TGo4ServerProxy* an = br->DefineAnalysisObject(itemname.toLatin1().constData(), objname);
               if (an) {
                  an->ChageObjectProtection(objname.Data(), (id == 24 ? "+C" : "-C"));
                  anrefresh = an;
               }
               break;
            }

            case 26: {   // delete remote object
               TString objname;
               TGo4ServerProxy* an = br->DefineAnalysisObject(itemname.toLatin1().constData(), objname);
               if (an) {
                  an->RemoveObjectFromAnalysis(objname.Data());
                  anrefresh = an;
               }
               break;
            }

            case 27: { // refresh
               TString objname;
               TGo4ServerProxy* an = br->DefineAnalysisObject(itemname.toLatin1().constData(), objname);
               if (an) anrefresh = an;
               TGo4ServerProxy* serv = br->DefineServerProxy(itemname.toLatin1().constData());
               if (serv) servrefresh = serv;
               break;
            }

            case 28: { // expand
               if (TGo4BrowserProxy::CanExpandItem(cando)) {
                  (*it)->setExpanded(true);
                  ExpandItem(itemname);
               }
               break;
            }

            case 29: { // execute
               if (TGo4BrowserProxy::CanExecuteItem(cando))
                  ExecuteItem(itemname);
               break;
            }

            case 41: { // create folder in memory
              bool ok = false;
              QString folder =
                QInputDialog::getText(this,
                                      "Create folder in workspace",
                                      "Input folder name",
                                      QLineEdit::Normal,
                                      QString(),
                                      &ok);
               if (ok) br->CreateMemorySubfolder(itemname.toLatin1().constData(), folder.toLatin1().constData());
               break;
            }

            case 42: {  // rename memory item
              bool ok = false;
              QString shortitemname=itemname.section("/",-1);
              QString newname =
                QInputDialog::getText(this,
                                      "Rename item in workspace",
                                      "Input new item name",
                                      QLineEdit::Normal,
                                      shortitemname,
                                      &ok);
               if (ok) br->RenameMemoryItem(itemname.toLatin1().constData(), newname.toLatin1().constData());
               break;
            }

            case 43: {  // paste items from clipboard
               br->CopyClipboard(itemname.toLatin1().constData(), go4sett->getFetchDataWhenCopy());
               br->ClearClipboard();
               break;
            }

            case 44: { // clear memory item
               br->ClearMemoryItem(itemname.toLatin1().constData());
               break;
            }
         }
      }

   if (anrefresh)
      anrefresh->RefreshNamesList();

   if (servrefresh)
      servrefresh->RefreshNamesList();

   if (id == 19)
      QApplication::restoreOverrideCursor();
}

bool TGo4Browser::canDrawItem(QTreeWidgetItem* item)
{
   if (!item) return false;
   int cando = BrowserProxy()->ItemCanDo(FullItemName(item).toLatin1().constData());
   return TGo4BrowserProxy::CanDrawItem(cando);
}

void TGo4Browser::ShootUpdateTimer()
{
   if (fbUpdateTimerActive) return;

   fbUpdateTimerActive = true;

   QTimer::singleShot(1, this, &TGo4Browser::updateListViewItems);
}

void TGo4Browser::SaveSelectedItems()
{
   QFileDialog fd(this, "Save selected objects to file", QString(),
                  "ROOT (*.root);;ROOT XML (*.xml)");
   fd.setFileMode( QFileDialog::AnyFile);
   fd.setAcceptMode(QFileDialog::AcceptSave);

   if (fd.exec() != QDialog::Accepted) return;

   QStringList flst = fd.selectedFiles();
   if (flst.isEmpty()) return;

   QString fname = flst[0];
   QString title;

   if (fd.selectedNameFilter() == "ROOT (*.root)") {
      bool ok = false;
      title = QInputDialog::getText(this,
         "Save slected objects to file", "Provide file title",
         QLineEdit::Normal, QString(), &ok);
      if (!ok) return;
      if (fname.indexOf(".root", 0, Qt::CaseInsensitive)<0) fname+=".root";
   } else {
      if (fname.indexOf(".xml", 0, Qt::CaseInsensitive)<0) fname+=".xml";
   }

   ExportSelectedItems(fname.toLatin1().constData(),
                       QFileInfo(fname).absolutePath().toLatin1().constData(),
                       fd.selectedNameFilter().toLatin1().constData(),
                       title.toLatin1().constData());
}

void TGo4Browser::ExportSelectedItems(const char *filtername)
{
   QFileDialog fd(this, QString("Select directory to export to ") + filtername);
   fd.setOption(QFileDialog::ShowDirsOnly, true);

   if (fd.exec() != QDialog::Accepted) return;

   QStringList flst = fd.selectedFiles();
   if (flst.isEmpty()) return;

   ExportSelectedItems("null",
                       flst[0].toLatin1().constData(),
                       filtername,
                       "Export of selected items");
}

void TGo4Browser::ExportSelectedItems(const char *filename, const char *filedir, const char *format, const char *description)
{
   TObjArray items;
   QTreeWidgetItemIterator it(ListView);
   for ( ; *it; ++it )
      if ((*it)->isSelected()) {
         QString fullname = FullItemName(*it);
         items.Add(new TObjString(fullname.toLatin1().constData()));
      }

   BrowserProxy()->ExportItemsTo(&items, go4sett->getFetchDataWhenSave(), filename, filedir, format, description);

   items.Delete();
}

void TGo4Browser::ExpandItem(const QString& itemname)
{
   BrowserProxy()->GetBrowserObject(itemname.toLatin1().constData(), 100);
}

void TGo4Browser::ExecuteItem(const QString& itemname)
{
   TString objname, arg1, arg2, arg3;

   TGo4ServerProxy* serv = BrowserProxy()->DefineServerObject(itemname.toLatin1().constData(), &objname, kFALSE);

   if (!serv || objname.IsNull()) return;

   Int_t nargs = serv->NumCommandArgs(objname);
   if (nargs < 0) return;

   for (Int_t n = 0; n < nargs; n++) {
      bool ok = false;
      QString value =
            QInputDialog::getText(0, "Input command arguments",
                                 QString("Arg%1:").arg(n+1), QLineEdit::Normal, "", &ok);
      if (!ok) return;
      if (n == 0)
         arg1 = value.toLatin1().constData();
      else if (n == 1)
         arg2 = value.toLatin1().constData();
      else if (n == 2)
         arg3 = value.toLatin1().constData();
   }

   Bool_t res = serv->SubmitCommand(objname, 3, (arg1.Length() > 0 ? arg1.Data() : nullptr), (arg2.Length() > 0 ? arg2.Data() : nullptr), (arg3.Length() > 0 ? arg3.Data() : nullptr));

   StatusMessage(QString(" Command execution:") + objname + QString("  result = ") + (res ? "TRUE" : "FALSE"));
}
