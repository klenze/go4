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

#ifndef TGO4ANALYSISSNIFFER_H
#define TGO4ANALYSISSNIFFER_H

#include "Rtypes.h"

class TGo4Ratemeter;
class TString;

/** Object to sniff Go4 analysis
 * It can obtain different events like log messages, ratemeter changes and so on
 * Used in dabc plugin to catch different go4 events
 */

class TGo4AnalysisSniffer {
   public:
      TGo4AnalysisSniffer() {}
      virtual ~TGo4AnalysisSniffer() {}

      /**Method called by analysis when ratemeter is updated */
      virtual void RatemeterUpdate(TGo4Ratemeter*) {}

      virtual void StatusMessage(int level, Bool_t printout, const TString&) {}

      /** Hook function to execute action in context of analysis thread */
      virtual void ProcessSnifferEvents() {}

   ClassDef(TGo4AnalysisSniffer, 1);
};

#endif
