// $Id$
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum f�r Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#ifndef TGO4COMMANDRECEIVER_H
#define TGO4COMMANDRECEIVER_H

/** @author M. Hemberger */

#include "Rtypes.h"

class TGo4CommandReceiver {
   public:

      virtual ~TGo4CommandReceiver();

      TGo4CommandReceiver();

   ClassDef(TGo4CommandReceiver, 1)
};

#endif //TGO4COMMANDRECEIVER_H
