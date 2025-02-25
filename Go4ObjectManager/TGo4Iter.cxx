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

#include "TGo4Iter.h"

#include "TGo4Slot.h"

TGo4Iter::TGo4Iter() :
   TGo4AccessWrapper(),
   fSlot(nullptr),
   fLevels(),
   fStatus(-1),
   fFullName(),
   fLevelChange(0)
{
}

TGo4Iter::TGo4Iter(TGo4Slot* slot, Bool_t onlyslots) :
   TGo4AccessWrapper(),
   fSlot(slot),
   fOnlySlots(onlyslots),
   fLevels(),
   fStatus(-1),
   fFullName(),
   fLevelChange(0)
{
}

TGo4Iter::~TGo4Iter()
{
   fLevels.Delete();
}

Bool_t TGo4Iter::next(Bool_t goesinto)
{
   fLevelChange = 0;

   if (fStatus == 1) return kFALSE;

   Int_t lastlevel = level();

   TGo4LevelIter* iter = nullptr;

   Bool_t wasfolder = kFALSE;

   if (fStatus==-1) {
      iter = !fSlot ? nullptr : fSlot->MakeLevelIter();
      if (iter) fLevels.Add(iter);
   } else {
      iter = (TGo4LevelIter*) fLevels.Last();

      wasfolder = fOnlySlots ? iter->isslotsfolder() : iter->isfolder();

      if (iter && wasfolder && goesinto) {
         TGo4LevelIter* subiter = iter->subiterator();
         if (subiter) {
            fLevels.Add(subiter);
            iter = subiter;
         }
      }
   }

   while (iter && !iter->next()) {
      fLevels.Remove(iter);
      delete iter;
      iter = (TGo4LevelIter*) fLevels.Last();
   }

   fFullName = "";

   if (iter)
      for (int n = 0; n <= fLevels.GetLast(); n++) {
         TGo4LevelIter *it = (TGo4LevelIter *)fLevels.At(n);
         if (n>0) fFullName.Append("/");
         fFullName.Append(it->name());
      }

   fStatus = iter ? 0 : 1;

   fLevelChange = level() - lastlevel;

   // Bool_t isfolder = kFALSE;
   // if (iter) isfolder = fOnlySlots ? iter->isslotsfolder() : iter->isfolder();
   // if (isfolder && wasfolder) fLevelChange--;

   if (wasfolder) fLevelChange--;

   return fStatus == 0;
}

Bool_t TGo4Iter::nextobj()
{
   while (next())
      if (!isfolder()) return kTRUE;
   return kFALSE;
}

Bool_t TGo4Iter::isfolder()
{
   TGo4LevelIter* iter = currentiter();
   return !iter ? kFALSE : (fOnlySlots ? iter->isslotsfolder() : iter->isfolder());
}

const char *TGo4Iter::getname()
{
   TGo4LevelIter* iter = currentiter();
   return !iter ? nullptr : iter->name();
}

const char *TGo4Iter::getinfo()
{
   TGo4LevelIter* iter = currentiter();
   return !iter ? nullptr : iter->info();
}

Int_t TGo4Iter::getsizeinfo()
{
   TGo4LevelIter* iter = currentiter();
   return !iter ? 0 : iter->sizeinfo();
}

Int_t TGo4Iter::getflag(const char *flagname)
{
   TGo4LevelIter* iter = currentiter();
   return (!iter || !flagname) ? -1 : iter->getflag(flagname);
}

Int_t TGo4Iter::getkindofitem()
{
   TGo4LevelIter* iter = currentiter();
   return !iter ? -1 : iter->GetKind();
}

const char *TGo4Iter::getclassname()
{
   TGo4LevelIter* iter = currentiter();
   return !iter ? nullptr : iter->GetClassName();
}

const char *TGo4Iter::getfullname()
{
   return fStatus != 0 ? nullptr : fFullName.Data();
}

Int_t TGo4Iter::level() const
{
   return fStatus != 0 ? -1 : fLevels.GetLast();
}

TGo4Access* TGo4Iter::ProvideAccess(const char *name)
{
   return fStatus != 0 ? nullptr : fSlot->ProvideSlotAccess(fFullName.Data());
}

TGo4Slot* TGo4Iter::getslot() const
{
   if (fStatus != 0) return nullptr;
   TGo4LevelIter* iter = (TGo4LevelIter*) fLevels.Last();
   return !iter ? nullptr : iter->getslot();
}
