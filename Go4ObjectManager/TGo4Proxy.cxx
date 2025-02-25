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

#include "TGo4Proxy.h"

#include "TROOT.h"

#include "TGo4ObjectManager.h"


Bool_t TGo4Access::GetObject(TObject* &obj, Bool_t &owner) const
{
   obj = nullptr;
   owner = kFALSE;
   return kFALSE;
}


const char *TGo4Access::GetObjectName() const
{
   return nullptr;
}

TClass* TGo4Access::GetObjectClass() const
{
   return TGo4Proxy::GetClass(GetObjectClassName());
}

const char *TGo4Access::GetObjectClassName() const
{
   return nullptr;
}

Int_t TGo4Access::AssignObjectTo(TGo4ObjectManager* rcv, const char *path)
{
   if (!rcv || IsRemote()) return 0;
   TObject *obj = nullptr;
   Bool_t owner = kFALSE;
   if (!GetObject(obj, owner)) return 0;
   DoObjectAssignement(rcv, path, obj, owner);
   return 1;
}

Int_t TGo4Access::AssignObjectToSlot(TGo4Slot* slot)
{
   if (!slot) return 0;

   TString tgtname;
   slot->ProduceFullName(tgtname);
   return AssignObjectTo(slot->GetOM(), tgtname.Data());
}

void TGo4Access::DoObjectAssignement(TGo4ObjectManager* mgr,
                                     const char *path,
                                     TObject *obj,
                                     Bool_t owner)
{
   mgr->AssignObject(path, obj, owner);
}

//------------------------------------------------------------------------------

TClass* TGo4Proxy::GetClass(const char *classname, Bool_t load)
{
   // Wrapper for TROOT::GetClass() method.
   // While ROOT does not automatically create TClass instance,
   // do it for known ROOT and Go4 classes
   // General idea of such method - avoid automatic load of custom libraries into the GUI

   if (!classname || (*classname == 0)) return nullptr;

   TClass* cl = (TClass*) gROOT->GetListOfClasses()->FindObject(classname);

   if (cl) return cl;

   if (!load)
      load = (strstr(classname,"TGo4")==classname) ||
             (strstr(classname,"TH1")==classname) ||
             (strstr(classname,"TH2")==classname) ||
             (strstr(classname,"TH3")==classname) ||
             (strstr(classname,"TGraph")==classname) ||
             !strcmp(classname,"TProfile") ||
             !strcmp(classname,"TProfile2D") ||
             !strcmp(classname,"TCanvas") ||
             !strcmp(classname,"TTree") ||
             !strcmp(classname,"THStack");

   return load ? gROOT->GetClass(classname, kTRUE, kTRUE) : nullptr;
}

