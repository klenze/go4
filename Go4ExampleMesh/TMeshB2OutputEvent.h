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

#ifndef TMESHB2OUTPUTEVENT_H
#define TMESHB2OUTPUTEVENT_H

#include "TGo4EventElement.h"

class TMeshB2AnlProc;
class TGo4FileSource;

class TMeshB2OutputEvent : public TGo4EventElement {
   public:
      TMeshB2OutputEvent();
      TMeshB2OutputEvent(const char *name);
      virtual ~TMeshB2OutputEvent();
      Int_t Init() override;
      Int_t Fill() override;
      void  Clear(Option_t *t = "") override;

      Float_t frData[16];
   private:
      // this object is streamed. Therefore pointers must be excluded!
      // Otherwise these objects are streamed as well
      TMeshB2AnlProc     *fxProcessor{nullptr};  //! Don't put this to file
      TGo4FileSource     *fxFile{nullptr};  //! Don't put this to file

   ClassDefOverride(TMeshB2OutputEvent,1)
};

#endif //TMESHB12OUTPUTEVENT_H

