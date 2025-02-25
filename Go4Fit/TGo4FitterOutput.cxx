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

#include "TGo4FitterOutput.h"

#include <iostream>

#include "TVirtualPad.h"

#include "TGo4FitterAbstract.h"

TGo4FitterOutput::TGo4FitterOutput() : TGo4FitterAction(), fxCommand(), fxOptions(), fxPad(nullptr)
{
}

TGo4FitterOutput::TGo4FitterOutput(const char *Command, const char *Options) :
    TGo4FitterAction(Command,"Output action for fitter"), fxCommand(Command), fxOptions(Options), fxPad(nullptr)
{
}

TGo4FitterOutput::~TGo4FitterOutput() {
}

void TGo4FitterOutput::DoAction(TGo4FitterAbstract* Fitter)
{
   if (!Fitter) return;
   if (fxCommand=="Print") Fitter->Print(fxOptions.Data()); else
   if (fxCommand=="Draw") {
     TVirtualPad* old = gPad;
     if (fxPad) fxPad->cd();
     Fitter->Draw(fxOptions.Data());
     if (fxPad) fxPad->Update();
     if (old) old->cd();
   }
}

void TGo4FitterOutput::Print(Option_t* option) const
{
   TGo4FitterAction::Print(option);
   std::cout << "  command: " << fxCommand;
   if (fxOptions.Length()>0) std::cout << "  options: " << fxOptions;
   std::cout << std::endl;
}
