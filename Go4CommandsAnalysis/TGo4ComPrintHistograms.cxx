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

#include "TGo4ComPrintHistograms.h"

#include "TGo4Log.h"
#include "TGo4AnalysisClientImp.h"
#include "TGo4AnalysisImp.h"

TGo4ComPrintHistograms::TGo4ComPrintHistograms()
:TGo4AnalysisCommand("ANPrhis","Printout of Histogram statistics")
{
   GO4TRACE((12,"TGo4ComPrintHistograms::TGo4ComPrintHistograms() ctor",__LINE__, __FILE__));
   SetReceiverName("AnalysisClient");  // this command needs client as receiver
                                       // override default receiver
}

Int_t TGo4ComPrintHistograms::ExeCom()
{
   GO4TRACE((12,"TGo4ComPrintHistograms::ExeCom() dtor",__LINE__, __FILE__));

   TGo4AnalysisClient* cli=dynamic_cast<TGo4AnalysisClient*> (fxReceiverBase);
   if (cli)   {
      GO4TRACE((11,"TGo4ComPrintHistograms::ExeCom() - found valid receiver",__LINE__, __FILE__));
         TGo4Analysis* ana= TGo4Analysis::Instance();
         if(ana)
            {
               ana->PrintHistograms();
               cli->SendStatusMessage(1, kFALSE, TString::Format(
                     "Analysis %s prints out histogram statistics.",ana->GetName()));
            }
         else
            {
                    cli->SendStatusMessage(3, kTRUE, TString::Format(
                          " %s ERROR no analysis ",GetName()));
            } // if(ana)



      }
   else
      {
      GO4TRACE((11,"TGo4ComPrintHistograms::ExeCom() - no receiver specified ERROR!",__LINE__, __FILE__));
         TGo4Log::Debug(" !!! ''%s'': NO RECEIVER ERROR!!!",GetName());
         return 1;
      }

   return -1;
}

