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

#include "TGo4DisplayDrawerTimer.h"

#include "TGo4Log.h"
#include "TGo4Task.h"
#include "TGo4Display.h"

TGo4DisplayDrawerTimer::TGo4DisplayDrawerTimer(TGo4Display *display, Long_t msperiod)
   : TTimer(msperiod, kTRUE), fxDisplay(display)
{
   TGo4Log::Debug(" Display drawing Timer is started with %ld ms period.", msperiod);
}

Bool_t TGo4DisplayDrawerTimer::Notify ()
{
   TObject* data = nullptr;
   TGo4Task* task = fxDisplay->GetTask();

   Int_t cnt = 100;

   while((data = task->NextObject(kFALSE)) != nullptr) {
      fxDisplay->DisplayData(data);
      if (cnt-- < 0) break;
   }

   Reset();
   return kFALSE;
}
