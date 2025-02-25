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

#ifndef TSTOREEVENT_H
#define TSTOREEVENT_H

#include "TGo4EventElement.h"

class TXXXEvent : public TGo4EventElement {
   public:
      TXXXEvent();
      TXXXEvent(const char *name);
      virtual ~TXXXEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="") override;

      Float_t fCrate1[8];
      Float_t fCrate2[8];

   ClassDefOverride(TXXXEvent,1)
};

#endif //TSTOREEVENT_H
