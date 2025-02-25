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

#include "TGo4TreeSource.h"

#include <iostream>

#include "TTree.h"
#include "TBranch.h"

#include "TGo4Log.h"
#include "TGo4MainTree.h"
#include "TGo4TreeSourceParameter.h"

TGo4TreeSource::TGo4TreeSource(const char *name)
: TGo4EventSource(name)
{
   GO4TRACE((15,"TGo4TreeSource::TGo4TreeSource(const char*)",__LINE__, __FILE__));
   Open();
}

TGo4TreeSource::TGo4TreeSource(TGo4TreeSourceParameter* par)
: TGo4EventSource(par->GetName())
{
   GO4TRACE((15,"TGo4TreeSource::TGo4TreeSource(TGo4TreeSourceParameter*)",__LINE__, __FILE__));
   Open();
}


TGo4TreeSource::TGo4TreeSource()
: TGo4EventSource("Go4TreeSource")
{
   GO4TRACE((15,"TGo4TreeSource::TGo4TreeSource()",__LINE__, __FILE__));
}

TGo4TreeSource::~TGo4TreeSource()
{
   GO4TRACE((15,"TGo4TreeSource::~TGo4TreeSource()",__LINE__, __FILE__));
   // we have to readout rest of branch into memory before whole tree is written again
   Int_t current = fxSingletonTree->GetCurrentIndex();
   Int_t max = fxSingletonTree->GetMaxIndex();
   Int_t z = 0;
   for (Int_t ix = current; ix < max; ++ix) {
      z++;
      if (fxBranch->GetEntry(ix) == 0) {
         std::cout << "reached end of branch after " << z << " dummy event retrieves" << std::endl;
         break;
      }
   }
   std::cout << "treesource " << GetName() << " is destroyed after " << z << "dummy retrieves." << std::endl;
}

Int_t TGo4TreeSource::Open()
{
   GO4TRACE((15,"TGo4TreeSource::Open()",__LINE__, __FILE__));

   // for branches containing the same event structure
   TString buffer = TString::Format("%s.",GetName());

   fxSingletonTree = TGo4MainTree::Instance();
   fxTree = fxSingletonTree->GetTree();
   fxBranch = fxTree->GetBranch(buffer.Data());
   if(fxBranch)
      TGo4Log::Debug(" TreeSource: Found existing branch %s ", buffer.Data());
   else
      ThrowError(77,0, "!!! ERROR: Branch %s  not found!!!", buffer.Data());

   return 0;
}



Bool_t TGo4TreeSource::BuildEvent(TGo4EventElement* dest)
{
   GO4TRACE((12,"TGo4TreeSource::BuildEvent(TGo4EventElement*)",__LINE__, __FILE__));
   //
   Bool_t rev=kTRUE;
   if(!dest) ThrowError(0,22,"!!! ERROR BuildEvent: no destination event!!!");
   if(!fxBranch) ThrowError(0,23,"!!! ERROR BuildEvent: branch was not initialized !!!");
   fxBranch->SetAddress(&dest);
   Int_t current=fxSingletonTree->GetCurrentIndex();
   if(fxBranch->GetEntry(current) == 0) {
      ThrowError(0,24,"!!! ERROR BuildEvent: getting branch entry failed !!!");
   } else {
      rev=kTRUE;
   }
   return rev;
}
