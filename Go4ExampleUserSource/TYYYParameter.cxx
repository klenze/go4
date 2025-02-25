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

#include "TYYYParameter.h"

#include "TGo4Log.h"
#include "TGo4Status.h"
#include "TGo4Fitter.h"

TYYYParameter::TYYYParameter() :
   TGo4Parameter("YYYParameter")
{
   frP1 = 0;
   frP2 = 0;

   for (Int_t ix = 0; ix < __ARRAYSIZE__; ix++) {
      fiDataArray[ix] = 0;
      fxFitArray[ix] = nullptr;
   }
}

TYYYParameter::TYYYParameter(const char *name) :
   TGo4Parameter(name)
{
   frP1 = 10;
   frP2 = 20;
   for (Int_t ix = 0; ix < __ARRAYSIZE__; ix++) {
      fiDataArray[ix] = ix + 30;
      fxFitArray[ix] = new TGo4Fitter();
   }
}

TYYYParameter::~TYYYParameter()
{
   for (Int_t ix = 0; ix < __ARRAYSIZE__; ++ix)
      if (fxFitArray[ix]) {
         delete fxFitArray[ix];
         fxFitArray[ix] = nullptr;
      }
}

void TYYYParameter::Print(Option_t*) const
{
  TGo4Status::PrintLine("Parameter %s:", GetName());
  TGo4Status::PrintLine(" P1 = %f", frP1);
  TGo4Status::PrintLine(" P2= %f", frP2);
  for (Int_t ix = 0; ix < __ARRAYSIZE__; ++ix) {
     TGo4Status::PrintLine("fiDataArray[%d] = %d", ix, fiDataArray[ix]);
     if (fxFitArray[ix])
        fxFitArray[ix]->PrintLines();
  }
}

Bool_t TYYYParameter::UpdateFrom(TGo4Parameter *source)
{
   auto from = dynamic_cast<TYYYParameter*> (source);
   if (!from) {
      TGo4Log::Error("Wrong parameter class: %s", source->ClassName());
      return kFALSE;
   }

   TGo4Log::Info("Parameter %s: P1=%f P2=%f", GetName(), frP1, frP2);
   frP1 = from->frP1;
   frP2 = from->frP2;
   for (Int_t ix = 0; ix < __ARRAYSIZE__; ++ix) {
      fiDataArray[ix] = from->fiDataArray[ix];
      // replace old fitters by copy of source fitter:
      if(fxFitArray[ix]) {
         delete fxFitArray[ix];
         fxFitArray[ix] = nullptr;
      }
      if (from->fxFitArray[ix])
         fxFitArray[ix] = (TGo4Fitter*) from->fxFitArray[ix]->Clone();
   }
   return kTRUE;
}
