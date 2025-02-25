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

#include "TGo4Display.h"

#include "TGo4Log.h"
#include "TGo4ThreadHandler.h"
#include "TGo4Task.h"
#include "TGo4DisplayDrawerTimer.h"
#include "TGo4DisplayLoggerTimer.h"
#include "TGo4Status.h"

#include "TGo4AnalysisProxy.h"
#include "TGo4Slot.h"


TGo4Display::TGo4Display(Bool_t isserver)
   : TGo4Master("Display",
                isserver, // servermode
                "dummy", //for clientmode only
                0), // negotiationport - 0 = root will find next free port automatically
     fxAnalysis(nullptr)
{
   // start gui timers instead of threads
   fxDrawTimer = new TGo4DisplayDrawerTimer(this, 30);
   fxLogTimer = new TGo4DisplayLoggerTimer(this, 500);
   fxDrawTimer->TurnOn();
   fxLogTimer->TurnOn();

    // Start the GUI Registry
   GetTask()->Launch();
}


TGo4Display::~TGo4Display()
{
   GO4TRACE((15,"TGo4Display::~TGo4Display()", __LINE__, __FILE__));
   //fxWorkHandler->CancelAll(); // make sure threads wont work on gui instance after it is deleted
   if(GetTask())
      GetTask()->GetWorkHandler()->CancelAll();
   delete fxDrawTimer;
   delete fxLogTimer;
   if (fxAnalysis) {
      fxAnalysis->DisplayDeleted(this); // will also clear back referenc to us
      TGo4Slot* pslot = fxAnalysis->ParentSlot();
      if (pslot) {
         //std::cout <<"TGo4Display dtor will delete analysis proxy parent slot" << std::endl;
         pslot->Delete();
      } else {
         //std::cout <<"TGo4Display dtor will delete analysis proxy directly" << std::endl;
         delete fxAnalysis; // regularly, we cleanup the analysis proxy.
      }
   }
   TGo4Log::Info("------- TGO4DISPLAY DESTRUCTOR FINISHED. ------");
}

void TGo4Display::DisplayData(TObject* data)
{
   if (fxAnalysis)
     fxAnalysis->ReceiveObject(dynamic_cast<TNamed*>(data));
   else
      delete data;
}

void TGo4Display::DisplayLog(TGo4Status * status)
{
   if (fxAnalysis)
      fxAnalysis->ReceiveStatus(status);
   else
      delete status;
}

Bool_t TGo4Display::DisconnectSlave(const char *name, Bool_t waitforslave)
{

   // Note: taskhandlerabortexception and shutdown of analysis server
   // both will schedule a TGo4ComDisconnectSlave into local command queue
   // of master task, calling TGo4Master::DisconnectSlave()
   // here we override this method to inform gui about this
   ///////////////////////////////////////////////////
   //
   // before disconnecting, gui might stop monitoring timers here....
   //...
   Bool_t rev = TGo4Master::DisconnectSlave(name, waitforslave); // should do all required things for disconnection
   // after disconnecting, gui might react on result by cleaning up browser, window caption etc.
   // for example:
   if (rev && fxAnalysis)
      fxAnalysis->DisplayDisconnected(this);

   return rev;
}

