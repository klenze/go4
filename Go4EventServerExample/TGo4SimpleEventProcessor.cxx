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

#include "TGo4SimpleEventProcessor.h"

#include "TGo4Log.h"
#include "TGo4SimpleEvent.h"
#include "TGo4SimpleSubEvent.h"
#include "Go4EventServer.h"

TGo4SimpleEventProcessor::TGo4SimpleEventProcessor()
:TGo4EventProcessor("Go4 Standard SimpleEvent Processor")
{
GO4TRACE((14,"TGo4SimpleEventProcessor::TGo4SimpleEventProcessor(Int_t)",__LINE__, __FILE__));

}

TGo4SimpleEventProcessor::~TGo4SimpleEventProcessor()
{
GO4TRACE((14,"TGo4SimpleEventProcessor::TGo4SimpleEventProcessor(Int_t)",__LINE__, __FILE__));

}

void TGo4SimpleEventProcessor::BuildSimpleEvent(TGo4SimpleEvent* target)
{
   GO4TRACE((11,"TGo4SimpleEventProcessor::TGo4SimpleEventProcessor(Int_t)",__LINE__, __FILE__));
   TGo4MbsEvent *input = (TGo4MbsEvent *)GetInputEvent();

   if (input) {
      Short_t procid;
      input->ResetIterator();
      while (auto insub = input->NextSubEvent()) {
         // find out procid
         procid = insub->GetProcid();
         auto outsubix = target->GetSubEvent(procid);
         TGo4SimpleSubEvent *outsub = nullptr;
         if (outsubix) {
            outsub = outsubix;
         } else {
            // no such procid in array, add new one
            outsub = target->AddSubEvent(procid);
            if (!outsub) {
               TGo4Log::Error("simple event processor: error adding subevent");
               return;
            }

         } // if (outsubix)

         // copy subevent data of that procid into target struct
         outsub->Clear();
         Int_t fieldsize = (insub->GetDlen() - 2) * sizeof(Short_t) / sizeof(Int_t);
         outsub->fiFieldLen = fieldsize;
         void *sourcefield = (void *)(insub->GetDataField());
         void *destfield = (void *)&(outsub->fiD0);
         memcpy(destfield, sourcefield, fieldsize * sizeof(Int_t));

      } // while
      target->fiCount = input->GetCount();

   } // if(input)
   else {
      TGo4Log::Error("Simple Event Processor: no input event !");
   }
}
