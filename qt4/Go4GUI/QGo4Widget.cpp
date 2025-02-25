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

#include "QGo4Widget.h"

#include <QtCore/QTimer>
#include <QAction>
#include <QMenu>
#include <QtCore/QSignalMapper>

#include <QDragMoveEvent>
#include <QCloseEvent>
#include <QDragEnterEvent>

QGo4Widget::QGo4Widget(QWidget *parent, const char *name, Qt::WindowFlags f) :
   QWidget(parent, f),
   fWaitsForObjectCreation(false),
   fCanDestroyWidget(true),
   fResetWidgetShooted(false),
   fBlockUpdate(false),
   fBrowserProxy(nullptr)
{
   setObjectName(name);
   setAcceptDrops(true);
}

QGo4Widget::~QGo4Widget()
{
   RemoveLinksMasked("complete");
}

void QGo4Widget::ObjectCreatedByWidget(const char *itemname, TClass* cl)
{
   // this method calls by maingui when object created by one of editor

   if (fWaitsForObjectCreation) {
      fWaitsForObjectCreation = false;
      requestedObjectCreated(itemname, cl);
   }
}

void QGo4Widget::ResetWidget()
{
   // we set fResetWidgetShooted to prevent in any situation of ShootResetWidget() again
   fResetWidgetShooted = true;
   RemoveAllLinks();
   fWaitsForObjectCreation = false;
   fResetWidgetShooted = false;
}

void QGo4Widget::ShootResetWidget()
{
   if (fResetWidgetShooted) return;

   fResetWidgetShooted = true;

   QTimer::singleShot(0, this, &QGo4Widget::ResetWidget);
}

void QGo4Widget::ShootCloseWidget(bool closeparent)
{
   if (fResetWidgetShooted) return;

   fResetWidgetShooted = true;

   QTimer::singleShot(0, closeparent ? parentWidget() : this, &QWidget::close);
}

void QGo4Widget::CloseMDIParentSlot()
{
   ShootCloseWidget(true);
}

bool QGo4Widget::IsAcceptDrag(const char*, TClass*, int)
{
   return false;
}

void QGo4Widget::DropItem(const char*, TClass*, int)
{
}

void QGo4Widget::closeEvent(QCloseEvent* e)
{
   if (fCanDestroyWidget) {
      e->accept();
   } else {
      e->ignore();
   }
}

void QGo4Widget::dragEnterEvent(QDragEnterEvent *e)
{
   emit widgetService(this, service_DragEnter, nullptr, dynamic_cast<QDragMoveEvent *> (e));
}

void QGo4Widget::dragMoveEvent(QDragMoveEvent* e)
{
   emit widgetService(this, service_DragEnter, "", e);
}

void QGo4Widget::dropEvent(QDropEvent* e)
{
   emit widgetService(this, service_DropEvent, nullptr, e);
}

void QGo4Widget::AddLink(const char *itemname, const char *linkname)
{
   emit widgetService(this, service_AddEditorLink, linkname, (void*) itemname);
}

void QGo4Widget::AddLink(TGo4Slot* slot, const char *linkname)
{
   emit widgetService(this, service_AddDirectLink, linkname, (void*) slot);
}

TGo4Slot* QGo4Widget::AddLink(const char *itemname, TGo4Slot* parent)
{
   TGo4Slot* res = parent;
   emit widgetService(this, service_AddLinkInSlot, itemname, (void*) &res);
   return res;
}

TGo4Slot* QGo4Widget::GetTopSlot(bool force)
{
   TGo4Slot* res = nullptr;
   emit widgetService(this, service_GetTopSlot, force ? "force" : "normal", (void*) &res);
   return res;
}

TGo4Slot* QGo4Widget::AddSlot(const char *slotname)
{
   TGo4Slot* res =  nullptr;
   emit widgetService(this, service_AddEditorSlot, slotname, (void*) &res);
   return res;
}

void QGo4Widget::SetLinkedName(TGo4Slot* slot, const char *itemname)
{
   emit widgetService(this, service_SetLinkedName, itemname, (void*) slot);
}

const char *QGo4Widget::GetLinkedName(const char *linkname)
{
   const char *res = nullptr;

   emit widgetService(this, service_GetLinkedName, linkname, (void*) &res);

   return res;
}

const char *QGo4Widget::GetLinkedName(TGo4Slot* slot)
{
   void* res = slot;

   emit widgetService(this, service_GetLinkedName2, "", (void*) &res);

   return (const char*) res;
}


TObject* QGo4Widget::GetLinked(const char *linkname, int updatelevel)
{
   TObject* res = nullptr;
   int func = 0;
   switch (updatelevel) {
      case 0: func = service_GetLinked0; break;
      case 1: func = service_GetLinked1; break;
      case 2: func = service_GetLinked2; break;
      default: func = service_GetLinked1; break;
   }

   emit widgetService(this, func, linkname, (void*) &res);

   return res;
}

void QGo4Widget::RemoveLink(const char *linkname, bool blockreset)
{
   bool oldvalue = fBlockUpdate;
   if (blockreset) fBlockUpdate = true;
   emit widgetService(this, service_RemoveLink, linkname, nullptr);
   fBlockUpdate = oldvalue;
}

void QGo4Widget::RemoveAllLinks(bool blockreset)
{
   bool oldvalue = fBlockUpdate;
   if (blockreset) fBlockUpdate = true;
   emit widgetService(this, service_RemoveAllLinks, nullptr, nullptr);
   fBlockUpdate = oldvalue;
}

void QGo4Widget::RemoveLinksMasked(const char *startedwith, bool blockreset)
{
   bool oldvalue = fBlockUpdate;
   if (blockreset) fBlockUpdate = true;
   emit widgetService(this, service_RemoveAllLinks, startedwith, nullptr);
   fBlockUpdate = oldvalue;
}

bool QGo4Widget::BrowserItemRemote(const char *itemname)
{
   bool isremote = false;
   emit widgetService(this, service_BrowserItemRemote, itemname, (void*) &isremote);
   return isremote;
}

TGo4BrowserProxy* QGo4Widget::Browser()
{
   if (fBrowserProxy) return fBrowserProxy;

   TGo4BrowserProxy* br = nullptr;
   emit widgetService(this, service_Browser, "", (void*) &br);
   fBrowserProxy = br;
   return br;
}

void QGo4Widget::ConnectPad(TPad* pad)
{
   emit widgetService(this, service_ConnectPad, "", (void*) pad);
}

void QGo4Widget::CallPanelFunc(int func, TPad* pad)
{
   emit widgetService(this, func, "", (void*) pad);
}

void QGo4Widget::StatusMessage(const QString& message)
{
   emit widgetService(this, service_StatusMessage, message.toLatin1().constData(), nullptr);
}

void QGo4Widget::ProcessSignal(const char *linkname, bool assigned, TObject *obj, TGo4Slot* slot)
{
   if (assigned) {
      linkedUpdated(slot, obj);
      linkedObjectUpdated(linkname, obj);
   } else
   // inform widget only if deletion caused not by widget itself by RemoveAllLinks() call
   if (!IsUpdateBlocked()) {
      linkedRemoved(slot, obj);
      linkedObjectRemoved(linkname);
   }
}

void QGo4Widget::linkedObjectUpdated(const char */* linkname */, TObject* /* obj */)
{
}

void QGo4Widget::linkedObjectRemoved(const char */* linkname */)
{
   // reset editor if any watched object is removed
   ShootResetWidget();
}

void QGo4Widget::linkedUpdated(TGo4Slot* /* slot */, TObject* /* obj */ )
{
}

void QGo4Widget::linkedRemoved(TGo4Slot* /* slot */, TObject* /* obj */)
{
}

void QGo4Widget::AskToCreateObject(TClass* cl, int isremote)
{
   fWaitsForObjectCreation = (isremote >= 0);
   QString str = QString::number(isremote);
   emit widgetService(this, service_CreateItem, str.toLatin1().constData(), (void*) cl);
}

void QGo4Widget::InformThatObjectCreated(const char *itemname, TClass* cl)
{
   emit widgetService(this, service_ObjectCreated, itemname, cl);
}

// this function should be reimplemented in editor that asks to create object
void QGo4Widget::requestedObjectCreated(const char */* itemname */, TClass* /* cl */)
{
}

void QGo4Widget::ShowItemInfo(const QString& itemname)
{
   emit widgetService(this, service_ShowInfo, itemname.toLatin1().constData(), nullptr);
}

TGo4ViewPanel* QGo4Widget::CreateViewPanel(int ndiv)
{
   QString str = QString::number(ndiv);
   TGo4ViewPanel* res = nullptr;
   emit widgetService(this, service_CreateViewPanel, str.toLatin1().constData(), (void*)&res);
   return res;
}

TGo4ViewPanel* QGo4Widget::DrawItem(const QString& itemname, TGo4ViewPanel* panel, TPad* pad, bool activate, int updatelevel)
{
   void* res[4];
   res[0] = panel;
   res[1] = pad;
   res[2] = &activate;
   res[3] = &updatelevel;
   emit widgetService(this, service_DrawItem, itemname.toLatin1().constData(), res);
   return (TGo4ViewPanel*) res[0];
}

TGo4ViewPanel* QGo4Widget::WhereItemDrawn(const char *itemname)
{
   TGo4ViewPanel* res = nullptr;
   emit widgetService(this, service_WhereItemDrawn, itemname, &res);
   return res;
}

void QGo4Widget::UndrawItem(const char *itemname)
{
   emit widgetService(this, service_UndrawItem, itemname, nullptr);
}

void QGo4Widget::HelpWindow(const char *filename, const char *msg)
{
   emit widgetService(this, service_HelpWindow, filename, (void*)msg);
}

void QGo4Widget::StartHotstart(const char *filename)
{
   emit widgetService(this, service_HotStart, filename, nullptr);
}

TGo4ViewPanel* QGo4Widget::LastActivePanel()
{
   TGo4ViewPanel* res = nullptr;
   emit widgetService(this, service_LastActivePanel, "", (void*)&res);
   return res;
}

void QGo4Widget::EditItem(const QString& itemname)
{
   emit widgetService(this, service_EditItem, itemname.toLatin1().constData(), nullptr);
}

void QGo4Widget::EditObjectInSlot(TGo4Slot* slot)
{
   emit widgetService(this, service_EditInSlot, "", (void*) slot);
}

QString QGo4Widget::SaveObjectInMemory(const char *foldername, TObject *obj)
{
   void* par = obj;
   emit widgetService(this, service_SaveToMemory, foldername, (void*) &par);
   QString itemname = "";
   if ((par != obj) && par) {
      QString* res = (QString*) par;
      itemname = *res;
      delete res;
   }
   return itemname;
}

bool QGo4Widget::SaveItemToFile(const char *itemname, const char *subfolder)
{
   char buf[1000];
   if (!subfolder)
      strcpy(buf, "");
   else
      strncpy(buf, subfolder, sizeof(buf));
   emit widgetService(this, service_SaveItem, itemname, (void*) buf);
   return buf[0] != 0;
}

bool QGo4Widget::UpdateItemInAnalysis(const char *itemname, TObject *obj)
{
   TObject* res = obj;
   emit widgetService(this, service_UpdateAnalysisItem, itemname, (void*) &res);
   return res != nullptr;
}

TGo4ServerProxy* QGo4Widget::GetAnalysis(const char *itemname)
{
   TGo4ServerProxy* res = nullptr;
   emit widgetService(this, service_GetAnalysis, itemname, (void*) &res);
   return res;
}

void QGo4Widget::CallServiceFunc(int func, const char *str, void* par)
{
   emit widgetService(this, func, str, par);
}

void QGo4Widget::ServiceCall(const char *name, void* par)
{
   CallServiceFunc(service_General, name, par);
}

QAction* QGo4Widget::CreateChkAction(QMenu* menu, const QString& text, bool checked, bool enabled)
{
   QAction* act = new QAction(text, menu);
   act->setCheckable(true);
   act->setChecked(checked);
   act->setEnabled(enabled);
   menu->addAction(act);
   return act;
}

QAction* QGo4Widget::AddIdAction(QMenu* menu, QSignalMapper* map,
                                 const QString& text, int id, int enabled, int checked)
{
   auto oldact = map->mapping(id);
   if (oldact) map->removeMappings(oldact);

   QAction* act = new QAction(text, menu);
   if (checked!=-1) {
      act->setCheckable(true);
      act->setChecked(checked > 0);
   }
   if (enabled!=-1)
      act->setEnabled(enabled > 0);

   QObject::connect(act, &QAction::triggered, [id, map]() {
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
      map->mapped(id);
#else
      map->mappedInt(id);
#endif
   });

   menu->addAction(act);
   map->setMapping(act, id);
   return act;
}

QAction* QGo4Widget::SetIdAction(QSignalMapper* map, int id, int enabled, int checked)
{
   QAction* act = (QAction*) map->mapping(id);
   if (!act) return nullptr;
   if (checked!=-1) {
      act->setCheckable(true);
      act->setChecked(checked > 0);
   }
   if (enabled!=-1)
      act->setEnabled(enabled > 0);
   return act;
}

QAction* QGo4Widget::AddIdAction(QMenu* menu, QSignalMapper* map,
                                 const QIcon& icon, const QString& text, int id, int enabled, int checked)
{
   auto oldact = map->mapping(id);
   if (oldact) map->removeMappings(oldact);

   QAction* act = new QAction(icon, text, menu);
   if (checked != -1) {
     act->setCheckable(true);
     act->setChecked(checked > 0);
   }
   if (enabled!=-1)
     act->setEnabled(enabled > 0);

   QObject::connect(act, &QAction::triggered, [id, map]() {
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
      map->mapped(id);
#else
      map->mappedInt(id);
#endif
   });

   menu->addAction(act);
   map->setMapping(act, id);
   return act;
}
