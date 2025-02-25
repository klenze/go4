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

#include "TGo4UserException.h"

#include <stdarg.h>

#include "TGo4Log.h"

TGo4UserException::TGo4UserException (Int_t prio, const char *text,...) :
   TGo4RuntimeException(),
   fxMessage(),
   fiPriority(prio)
{
   GO4TRACE((14,"TGo4UserException:TGo4UserException (TGo4TaskHandler*)", __LINE__, __FILE__));
   fxDescription =  "!!!-- Go4 User Exception --!!!";
   // put potential printf arguments in text:
   char txtbuf[TGo4Log::fguMESLEN];
   va_list args;
   va_start(args, text);
   vsnprintf(txtbuf, TGo4Log::fguMESLEN, text, args);
   va_end(args);
   fxMessage = txtbuf;
}

TGo4UserException::~TGo4UserException()
{
   GO4TRACE((14,"TGo4UserException:~TGo4UserException", __LINE__, __FILE__));
}

TGo4UserException & TGo4UserException::operator= (const TGo4UserException & right)
{
   GO4TRACE((14,"TGo4UserException:operator=",__LINE__, __FILE__));
   if (&right != this) {
      TGo4RuntimeException::operator=(right); // copy base class members
      fxMessage = right.fxMessage;
      fiPriority = right.fiPriority;
   }
   return *this;
}

TGo4UserException::TGo4UserException(const TGo4UserException &right)
  :TGo4RuntimeException(right)
{
   GO4TRACE((14,"TGo4UserException:TGo4UserException(right)", __LINE__, __FILE__));
   fxMessage = right.fxMessage;
   fiPriority = right.fiPriority;
}

Int_t TGo4UserException::Handle()
{
   GO4TRACE((12,"TGo4UserException::Handle()", __LINE__, __FILE__));
   TGo4Log::Message(GetPriority(), "%s", GetMessage());

   return 0;
}
