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

#include "TGo4ConnectorRunnable.h"

#include "TGo4Thread.h"
#include "TGo4ServerTask.h"
#include "TGo4TaskManager.h"

TGo4ConnectorRunnable::TGo4ConnectorRunnable(const char *name, TGo4ServerTask* man) :
   TGo4Runnable(name,man)
{
   fxTaskManager = man->GetTaskManager();
}

Int_t TGo4ConnectorRunnable::Run(void* ptr)
{
   // create new taskhandler and connect server to  client:
   Int_t rev = fxTaskManager->ServeClient();
   // stop runnable for termination
   if (rev<-1) fxGo4Thread->Stop();
   return rev;
}
