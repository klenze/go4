// $Id: TGo4HDF5SourceParameter.h 478 2009-10-29 12:26:09Z linev $
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

#ifndef TGO4HDF5SOURCEPARAMETER_H
#define TGO4HDF5SOURCEPARAMETER_H


#include "TGo4EventSourceParameter.h"

class TGo4HDF5SourceParameter : public TGo4EventSourceParameter {

   public:

      TGo4HDF5SourceParameter();

      TGo4HDF5SourceParameter(const char* name);

      virtual ~TGo4HDF5SourceParameter();

   ClassDef(TGo4HDF5SourceParameter,1)
};


#endif //TGO4FILESOURCEPARAMETER_H
