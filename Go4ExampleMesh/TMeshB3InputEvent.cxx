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

#include "TMeshB3InputEvent.h"

#include "TGo4Log.h"
#include "TMeshUnpackProc.h"
#include "TGo4FileSource.h"

//***********************************************************
TMeshB3InputEvent::TMeshB3InputEvent()
  :TGo4EventElement("InputEvent3"),fxProcessor(0),fxFile(0)
{
}
//***********************************************************
TMeshB3InputEvent::TMeshB3InputEvent(const char *name)
  :TGo4EventElement(name),fxProcessor(0),fxFile(0)
{
}
//***********************************************************
TMeshB3InputEvent::~TMeshB3InputEvent()
{
}
//***********************************************************

//-----------------------------------------------------------
Int_t TMeshB3InputEvent::Init()
{
  Int_t rev = 0;
  //std::cout << "+++ Init event" << std::endl;
  Clear();
  // is it used by Unpack step as output?
  if(CheckEventSource("TMeshUnpackProc")){
    fxProcessor = dynamic_cast<TMeshUnpackProc*>(GetEventSource());
    TGo4Log::Info("TMeshB3InputEvent init for Unpack step");
  }
  // or is it used from provider step as input
  else if(CheckEventSource("TGo4FileSource")){
    fxFile = dynamic_cast<TGo4FileSource*> (GetEventSource());
    TGo4Log::Info("TMeshB3InputEvent init for file input");
  }
  else          rev=1;
  return rev;
}
//-----------------------------------------------------------
Int_t TMeshB3InputEvent::Fill()
{
   Int_t rev = 0;
   Clear();
   if(fxProcessor)
      fxProcessor->UnpackBranch3(this);
   else if(fxFile)
      fxFile->BuildEvent(this);
   else
      rev=1;
   return rev;
}

//-----------------------------------------------------------
void  TMeshB3InputEvent::Clear(Option_t *t)
{
   void* destfield = (void*) &fiCrate3[0];
   memset(destfield,0, sizeof(fiCrate3));
}
