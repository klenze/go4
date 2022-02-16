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

#include "TGo4AnalysisObjectNames.h"

#include <iostream>

#include "TROOT.h"
#include "TList.h"
#include "TFolder.h"
#include "TGo4Log.h"

TGo4AnalysisObjectNames::TGo4AnalysisObjectNames() :
   TGo4Status()
{
}

TGo4AnalysisObjectNames::TGo4AnalysisObjectNames(const char* name) :
   TGo4Status(name)
{
   GO4TRACE((15,"TGo4AnalysisObjectnames::TGo4AnalysisObjectnames(const char*)",__LINE__, __FILE__));
}

TGo4AnalysisObjectNames::~TGo4AnalysisObjectNames()
{
   GO4TRACE((15,"TGo4AnalysisObjectnames::~TGo4AnalysisObjectnames()",__LINE__, __FILE__));
   if(fxTopFolder) {
      delete fxTopFolder;
      fxTopFolder = nullptr;
   }
}

TList* TGo4AnalysisObjectNames::GetFolderList()
{
   if(fxTopFolder)
      return dynamic_cast<TList*> (fxTopFolder->GetListOfFolders());

   return nullptr;
}

TFolder* TGo4AnalysisObjectNames::GetNamesFolder(Bool_t chown)
{
   TFolder* reval=fxTopFolder;
   if(chown) fxTopFolder=0;
   return reval;
}


void TGo4AnalysisObjectNames::Print(Option_t *)
{
   GO4TRACE((12,"TGo4AnalysisObjectNames::PrintStatus()",__LINE__, __FILE__));
   //
   gROOT->SetDirLevel(0);
   PrintLine("G-OOOO-> Analysis Object Names Printout <-OOOO-G");
   PrintLine("G-OOOO-> ---------------------------------------------- <-OOOO-G");
   PrintFolder(fxTopFolder);
}

void TGo4AnalysisObjectNames::PrintFolder(TFolder* fold)
{
   if (!fold) return;
   GO4TRACE((12,"TGo4AnalysisObjectNames::PrintFolder()",__LINE__, __FILE__));
   //
   gROOT->IncreaseDirLevel();

   PrintLine("G-OOOO-> Status Folder %s Printout <-OOOO-G", fold->GetName());
   PrintLine("G-OOOO-> ---------------------------------------------- <-OOOO-G");

   TIter iter(fold->GetListOfFolders());
   TObject* entry = nullptr;

   while((entry = iter()) != nullptr) {
      if(entry->InheritsFrom(TFolder::Class())) {
         PrintFolder(dynamic_cast<TFolder*> (entry));
      } else if (entry->InheritsFrom(TGo4Status::Class())) {
         entry->Print();
      } else {
         entry->ls();
      }
   }

   PrintLine("G-OOOO-> ---------------------------------------------- <-OOOO-G");

   gROOT->DecreaseDirLevel();
}
