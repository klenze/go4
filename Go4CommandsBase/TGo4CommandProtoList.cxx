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

#include "TGo4CommandProtoList.h"

#include <iostream>

#include "TMutex.h"
#include "TObjArray.h"

#include "TGo4LockGuard.h"
#include "TGo4Log.h"
#include "TGo4RemoteCommand.h"

TGo4CommandProtoList::TGo4CommandProtoList(const char *name) :
   TNamed(name,"This is a TGo4CommandProtoList")
{
   GO4TRACE((14,"TGo4CommandProtoList::TGo4CommandProtoList(const char*) ctor",__LINE__, __FILE__));

   fxListMutex = new TMutex;
   fxCommandList = new TObjArray;
   AddCommand (new TGo4RemoteCommand);
}

TGo4CommandProtoList::~TGo4CommandProtoList()
{
   GO4TRACE((14,"TGo4CommandProtoList::~TGo4CommandProtoList() dtor",__LINE__, __FILE__));
   {
      TGo4LockGuard listguard(fxListMutex);
      fxCommandList->Delete();
      delete fxCommandList;
   }
   delete fxListMutex;
}

void TGo4CommandProtoList::ShowCommands()
{
   GO4TRACE((12,"TGo4CommandProtoList::ShowCommands()",__LINE__, __FILE__));

   TGo4LockGuard listguard(fxListMutex);
   TIter iter(fxCommandList);
   TGo4Log::Debug(" CommandProtoList Showing the known commands:");
   std::cout << " Name: \t| Description:"<<std::endl;
   while(auto com = (TGo4Command*) iter())
      std::cout << " "<< com->GetName()<<"\t| "<<com->What()<<std::endl;
}

TGo4Command* TGo4CommandProtoList::MakeCommand(const char *name)
{
   GO4TRACE((12,"TGo4CommandProtoList::MakeCommand(const char*)",__LINE__, __FILE__));
   TGo4Command* rev = nullptr;
   TGo4LockGuard listguard(fxListMutex);
   TObject *obj = fxCommandList->FindObject(name);
   if(!obj) {
      //no, do nothing
      GO4TRACE((10,"TGo4CommandProtoList::RemoveCommand(TGo4Command*) command not found in array",__LINE__, __FILE__));
   } else {
      // yes, create it it
      GO4TRACE((10,"TGo4CommandProtoList::RemoveCommand(TGo4Command*) cloning command",__LINE__, __FILE__));
      rev = (TGo4Command*) obj->Clone();
   }

   return rev;
}

TGo4Command* TGo4CommandProtoList::MakeCommand(TGo4RemoteCommand* remcon)
{
   if(!remcon) return nullptr;
   TGo4Command* com = MakeCommand(remcon->GetCommandName());
   if(com)
      com->Set(remcon); // copy optional parameters from remote command
   return com;
}


void TGo4CommandProtoList::RemoveCommand(const char *name)
{
   GO4TRACE((12,"TGo4CommandProtoList::RemoveCommand(const char*)",__LINE__, __FILE__));

   TGo4LockGuard listguard(fxListMutex);
   TObject *obj = fxCommandList->FindObject(name);
   if(!obj) {
      //no, do nothing
      GO4TRACE((10,"TGo4CommandProtoList::RemoveCommand(TGo4Command*) command not found in array",__LINE__, __FILE__));
   } else {
      // yes, remove it
      GO4TRACE((10,"TGo4CommandProtoList::RemoveCommand(TGo4Command*) removing command from array",__LINE__, __FILE__));
      fxCommandList->Remove(obj);
      fxCommandList->Compress();
      fxCommandList->Expand(fxCommandList->GetLast()+1);
      delete obj;
   }
}


void TGo4CommandProtoList::AddCommand(TGo4Command* com)
{
   GO4TRACE((12,"TGo4CommandProtoList::AddCommand(TGo4Command*)",__LINE__, __FILE__));
   TGo4LockGuard listguard(fxListMutex);
   if(!fxCommandList->FindObject(com)) {
      //no, add new command
      GO4TRACE((10,"TGo4CommandProtoList::AddCommand(TGo4Command*) Adding new go4 commandto array",__LINE__, __FILE__));
      fxCommandList->AddLast(com);
   } else {
      // yes, do nothing
      GO4TRACE((10,"TGo4CommandProtoList::AddCommand(TGo4Command*) command was already in array",__LINE__, __FILE__));
   }
}

TGo4CommandProtoList& TGo4CommandProtoList::operator+=(const TGo4CommandProtoList& two)
{
   if(this != &two) {
       TGo4LockGuard outerguard(two.fxListMutex);
       TIter iter(two.fxCommandList);
       while (auto com = dynamic_cast<TGo4Command*>(iter()))
           AddCommand(com);
       std::cout <<"CommandProtoList "<< GetName() <<"used operator += for adding list " << two.GetName() << std::endl;
   }
   return *this;
}
