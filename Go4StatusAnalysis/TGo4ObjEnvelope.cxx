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

#include "TGo4ObjEnvelope.h"

TGo4ObjEnvelope::TGo4ObjEnvelope() :
   TNamed(),
   fxObject(nullptr),
   fxObjName(),
   fxObjFolder(),
   fbOwner(kTRUE)
{
}

TGo4ObjEnvelope::TGo4ObjEnvelope(TObject *obj, const char *name, const char *folder) :
   TNamed("Go4Envelope",""),
   fxObject(obj),
   fxObjName(name),
   fxObjFolder(folder),
   fbOwner(kFALSE)
{
}

TGo4ObjEnvelope::~TGo4ObjEnvelope()
{
   if (fbOwner && fxObject) {
      delete fxObject;
   }
}

TObject* TGo4ObjEnvelope::TakeObject()
{
   TObject* res = fxObject;
   fxObject = nullptr;
   fbOwner = kFALSE;
   return res;
}
