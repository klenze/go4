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

#include "TGo4DrawCloneProxy.h"

#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TString.h"

#include "TGo4LockGuard.h"
#include "TGo4ObjectManager.h"
#include "TGo4Picture.h"
#include "TGo4BrowserProxy.h"
#include "TGo4ViewPanel.h"


TGo4DrawCloneProxy::TGo4DrawCloneProxy(TGo4Slot* slot, TGo4ViewPanel* panel) :
   TGo4LinkProxy(slot),
   fClone(nullptr),
   fPanel(panel),
   fParentSlot(nullptr)
{
}

TGo4DrawCloneProxy::~TGo4DrawCloneProxy()
{
   if (fClone) delete fClone;
}

Bool_t TGo4DrawCloneProxy::AssignClone(TObject *obj, TGo4Slot* slot)
{
   TGo4LockGuard lock;

   CleanupClone(slot);
   if (!obj) return kFALSE;

   if (!obj->InheritsFrom(TH1::Class()) &&
       !obj->InheritsFrom(TGraph::Class())) return kFALSE;

   fClone = obj->Clone();
   if (fClone->InheritsFrom(TH1::Class()))
     ((TH1*) fClone)->SetDirectory(nullptr);
   TGo4ObjectManager* om = slot->GetOM();
   if (om && fClone)
     om->RegisterObjectWith(fClone, slot);

   return kTRUE;
}

void TGo4DrawCloneProxy::CleanupClone(TGo4Slot* slot)
{
   if (!fClone) return;
   TGo4ObjectManager* om = slot->GetOM();
   if (om) {
      om->RecursiveRemove(fClone);
      om->UnregisterObject(fClone, slot);
   }
   delete fClone;
   fClone = nullptr;
}

void TGo4DrawCloneProxy::ChangeTitle(TObject *obj)
{
   TNamed* src = dynamic_cast<TNamed*> (obj);
   TNamed* tgt = dynamic_cast<TNamed*> (fClone);

   TGo4Picture* padopt = fPanel->GetPadOptions(fParentSlot->GetParent());

   if (!tgt || !src || !padopt) return;
   TString title = src->GetTitle();

   const char *stime = TGo4BrowserProxy::ItemTime(GetLink());
   const char *sdate = TGo4BrowserProxy::ItemDate(GetLink());
   const char *itemname = TGo4BrowserProxy::GetLinkedName(fParentSlot);

   if (stime && padopt->IsTitleTime()) {
      title+= "  ";
      title+=stime;
   }
   if (sdate && padopt->IsTitleDate()) {
      title+= "  ";
      title+=sdate;
   }
   if (itemname && padopt->IsTitleItem()) {
      title+= "  ";
      title+=itemname;
   }
   title.ReplaceAll(";","#semicolon");
   tgt->SetTitle(title.Data());
}

void TGo4DrawCloneProxy::UpdateTitle()
{
   if (GetLink())
      ChangeTitle(GetLink()->GetAssignedObject());
}

Bool_t TGo4DrawCloneProxy::RemoveRegisteredObject(TObject *obj)
{
   if (obj==fClone) fClone = nullptr;
   return kFALSE;
}

void TGo4DrawCloneProxy::Initialize(TGo4Slot* slot)
{
   TGo4LinkProxy::Initialize(slot);
   if (fClone) {
      delete fClone;
      Error("Initialize"," Problem in TGo4DrawCloneProxy");
   }
   fClone = nullptr;
   fParentSlot = slot;

   if (GetLink()) {
      TObject *obj = GetLink()->GetAssignedObject();
      if (obj) AssignClone(obj, slot);
   }
}

void TGo4DrawCloneProxy::Finalize(TGo4Slot* slot)
{
   CleanupClone(slot);
   TGo4LinkProxy::Finalize(slot);
   fParentSlot = nullptr;
}

TObject* TGo4DrawCloneProxy::GetAssignedObject()
{
   return fClone ? fClone : TGo4LinkProxy::GetAssignedObject();
}

Bool_t TGo4DrawCloneProxy::ProcessEvent(TGo4Slot* slot, TGo4Slot* source, Int_t id, void* param)
{
   if (id==TGo4Slot::evObjAssigned) {
      TObject *obj = GetLink()->GetAssignedObject();
      AssignClone(obj, slot);
      ChangeTitle(obj);
   } else if ((id==TGo4Slot::evObjUpdated) || (id==TGo4Slot::evContAssigned)) {
      bool res = kFALSE;
      TObject *obj = GetLink()->GetAssignedObject();
      if (obj) {
        Int_t rebinx = 0, rebiny = 0;
        if (fClone)
          res = TGo4BrowserProxy::UpdateObjectContent(fClone, obj, &rebinx, &rebiny);
        if (!res) {
           res = AssignClone(obj, slot);
        } else {
           if (rebinx>1) fParentSlot->SetIntPar("::HasRebinX",rebinx);
                    else fParentSlot->RemovePar("::HasRebinX");
           if (rebiny>1) fParentSlot->SetIntPar("::HasRebinY",rebiny);
                    else fParentSlot->RemovePar("::HasRebinY");
        }
      }

      if (!res) CleanupClone(slot);
           else ChangeTitle(obj);
   }

   return TGo4LinkProxy::ProcessEvent(slot, source, id, param);
}

void TGo4DrawCloneProxy::PerformRebin()
{
   if (!fClone) return;

   Int_t rebinx = 0, rebiny = 0;
   fParentSlot->GetIntPar("::DoRebinX", rebinx);
   fParentSlot->RemovePar("::DoRebinX");
   fParentSlot->GetIntPar("::DoRebinY", rebiny);
   fParentSlot->RemovePar("::DoRebinY");

   if ((rebinx<2) && (rebiny<2)) return;

   if (rebinx == 0) rebinx = 1;
   if (rebiny == 0) rebiny = 1;

   TH2* h2 = dynamic_cast<TH2*> (fClone);
   if (h2) {
      h2->Rebin2D(rebinx, rebiny);
      return;
   }

   TH1* h1 = dynamic_cast<TH1*> (fClone);
   if (h1) h1->Rebin(rebinx);
}

