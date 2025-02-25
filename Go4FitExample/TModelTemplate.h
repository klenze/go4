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

#ifndef TMODELTEMPLATE_H
#define TMODELTEMPLATE_H

#include "TGo4FitModel.h"

class TModelTemplate : public TGo4FitModel {
   public:
      TModelTemplate(const char *iName, Int_t iNPars, Bool_t AddAmplitude = kFALSE);

   protected:
      Double_t UserFunction(Double_t* Coordinates, Double_t* Parameters) override;

   ClassDefOverride(TModelTemplate,1)
};

#endif // TMODELTEMPLATE_H
