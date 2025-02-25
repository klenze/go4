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

#include "TGo4EventEndException.h"

#include "TGo4Log.h"

TGo4EventEndException::TGo4EventEndException (TGo4EventSource* eventsource)
: TGo4EventSourceException(eventsource)
{
   GO4TRACE((16,"TGo4EventEndException::TGo4EventEndException (TGo4EventSource*)", __LINE__, __FILE__));
   fxDescription= "Go4 EventEndException ";

}

TGo4EventEndException::TGo4EventEndException(const TGo4EventEndException &right)
  :TGo4EventSourceException(right)
{
   GO4TRACE((16,"TGo4EventEndException::TGo4EventEndException(right)", __LINE__, __FILE__));   ;
}

TGo4EventEndException & TGo4EventEndException::operator= (const TGo4EventEndException & right)
{
   GO4TRACE((16, "TGo4EventEndException::operator=", __LINE__, __FILE__));
   if (&right != this)
      TGo4EventSourceException::operator=(right); // copy base class members
   return *this;
}

TGo4EventEndException::~TGo4EventEndException()
{
   GO4TRACE((16,"TGo4EventEndException::~TGo4EventEndException()", __LINE__, __FILE__));
}
