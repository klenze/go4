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

#include "TGo4AbstractInterface.h"

#include <iostream>
#include <fstream>

#include "TROOT.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "TObjString.h"

#include "TGo4Log.h"
#include "TGo4Iter.h"
#include "TGo4BrowserProxy.h"
#include "TGo4ServerProxy.h"

TGo4AbstractInterface* TGo4AbstractInterface::fgInstance = nullptr;

TString TGo4AbstractInterface::fInitSharedLibs = "";


TGo4AbstractInterface* TGo4AbstractInterface::Instance()
{
   return fgInstance;
}

void TGo4AbstractInterface::DeleteInstance()
{
   if (fgInstance) {
      delete fgInstance;
      fgInstance = nullptr;
   }
}

const char *TGo4AbstractInterface::FileExtension()
{
   return ".hotstart";
}

const char *TGo4AbstractInterface::DefaultPicTitle()
{
   return "temporary object to setup viewpanel";
}

Int_t TGo4AbstractInterface::DelayMillisec()
{
   return 20;
}

void TGo4AbstractInterface::SetInitSharedLibs(const char *libs)
{
   fInitSharedLibs = libs ? libs : gInterpreter->GetSharedLibs();
}

TGo4AbstractInterface::TGo4AbstractInterface() :
   TObject(),
   fOM(nullptr),
   fBrowser(nullptr),
   fxCommands()
{
   fgInstance = this;
}

void TGo4AbstractInterface::Initialize(TGo4ObjectManager* om, TGo4BrowserProxy* br)
{
   fOM = om;
   fBrowser = br;

   Int_t error = 0;
   gROOT->ProcessLineSync("TGo4AbstractInterface* go4 = TGo4AbstractInterface::Instance();", &error);
   gROOT->ProcessLineSync("TGo4ObjectManager* om = TGo4AbstractInterface::Instance()->OM();", &error);
   gROOT->ProcessLineSync("TGo4BrowserProxy* br = TGo4AbstractInterface::Instance()->Browser();", &error);
   gROOT->ProcessLineSync(TString::Format(".x %s", TGo4Log::subGO4SYS("macros/go4macroinit.C").Data()).Data());
}

TGo4AbstractInterface::~TGo4AbstractInterface()
{
   FreeHotStartCmds();

   gROOT->ProcessLine(TString::Format(".x %s", TGo4Log::subGO4SYS("macros/go4macroclose.C").Data()).Data());

   gInterpreter->DeleteGlobal(fBrowser);
   gInterpreter->DeleteGlobal(fOM);
   gInterpreter->DeleteGlobal(this);

   fgInstance = nullptr;

}

TGo4AnalysisProxy* TGo4AbstractInterface::Analysis()
{
   return Browser() ? Browser()->FindAnalysis() : nullptr;
}

TGo4ServerProxy* TGo4AbstractInterface::Server()
{
   return Browser() ? Browser()->FindServer() : nullptr;
}

void TGo4AbstractInterface::LoadLibrary(const char *fname)
{
   if (!fname || (strlen(fname) == 0)) return;

   TString libs = gInterpreter->GetSharedLibs();

   const char *token = strtok((char*) libs.Data(), " ,\t\n");

   while(token != nullptr) {
      if (strcmp(token, fname) == 0) return;
      token = strtok(nullptr, " ,\t\n");
   }

   gSystem->Load(fname);
}

void TGo4AbstractInterface::OpenFile(const char *fname)
{
   Browser()->OpenFile(fname);
   ProcessEvents(-1);
}


Bool_t TGo4AbstractInterface::SaveToFile(const char *itemname,
                                         const char *filename,
                                         const char *filetitle)
{
   return Browser()->SaveBrowserToFile(filename, kFALSE, itemname, filetitle);
}

Bool_t TGo4AbstractInterface::ExportToFile(const char *itemname,
                                           const char *dirpath,
                                           const char *format,
                                           const char *filetitle)
{
    TGo4Slot* topslot = Browser()->ItemSlot(itemname);
    if (!topslot) return kFALSE;

    TObjArray items;

    TGo4Iter iter(topslot, kTRUE);

    TString buf;

    while (iter.next())
       if (Browser()->BrowserItemName(iter.getslot(), buf))
          items.Add(new TObjString(buf));

    Browser()->ExportItemsTo(&items, kFALSE, "null", dirpath, format, filetitle);

    items.Delete();

    return kTRUE;
}


void TGo4AbstractInterface::ConnectHServer(const char *servername,
                                       Int_t portnumber,
                                       const char *basename,
                                       const char *userpass,
                                       const char *filter)
{
   Browser()->ConnectHServer(servername,
                             portnumber,
                             basename,
                             userpass,
                             filter);
   ProcessEvents(200);
}

void TGo4AbstractInterface::ConnectDabc(const char *servername)
{
   Browser()->ConnectDabc(servername);
}

Bool_t TGo4AbstractInterface::IsAnalysisConnected()
{
   return !Server() ? kFALSE : Server()->IsConnected();
}

void TGo4AbstractInterface::ExecuteLine(const char *remotecmd)
{
   TGo4ServerProxy* anal = Server();
   if (anal && remotecmd) {
      anal->ExecuteLine(remotecmd);
      TGo4Log::Message(1, "Exec: %s", remotecmd);
   }
}

void TGo4AbstractInterface::RequestAnalysisConfig()
{
   TGo4ServerProxy* anal = Server();
   if (anal)
     anal->RequestAnalysisSettings();
}

void TGo4AbstractInterface::MonitorItem(const char *itemname, Bool_t on)
{
   TGo4Slot* itemslot = Browser()->BrowserSlot(itemname);
   if (itemslot)
     Browser()->SetItemMonitored(itemslot, on);
}

void TGo4AbstractInterface::StartMonitoring(Int_t period)
{
   Browser()->ToggleMonitoring(period*1000);
}

void TGo4AbstractInterface::StopMonitoring()
{
   Browser()->ToggleMonitoring(-1);
}

TString TGo4AbstractInterface::FindItem(const char *objname)
{
   // if itemname is specified, return as is
   if (Browser()->ItemSlot(objname)) return TString(objname);

   return Browser()->FindItem(objname);
}

Bool_t TGo4AbstractInterface::CopyItem(const char *itemname)
{
   return Browser()->ProduceExplicitCopy(itemname);
}

Bool_t TGo4AbstractInterface::DeleteItem(const char *itemname)
{
   return Browser()->DeleteDataSource(Browser()->ItemSlot(itemname));
}

void TGo4AbstractInterface::FetchItem(const char *itemname, Int_t wait_time)
{
   Browser()->FetchItem(itemname, wait_time);
}

void TGo4AbstractInterface::RedrawItem(const char *itemname)
{
   Browser()->RedrawItem(itemname);
}

TObject* TGo4AbstractInterface::GetObject(const char *itemname, Int_t updatelevel)
{
   if (!itemname || (strlen(itemname) == 0)) return nullptr;

   return Browser()->GetBrowserObject(itemname, updatelevel);
}

TString TGo4AbstractInterface::SaveToMemory(const char *path, TObject *obj, Bool_t ownership)
{
   TString res = Browser()->SaveToMemory(path, obj, ownership, kTRUE);

   ProcessEvents();

   return res;
}

Bool_t TGo4AbstractInterface::LoadHotStart(const char *filename)
{
   FreeHotStartCmds();

   std::ifstream f(filename);

   if (!f.good()) return kFALSE;

   char buf[10000];

   while (!f.eof()) {
      f.getline(buf, 10000);
      fxCommands.Add(new TObjString(buf));
   }

   if (fxCommands.IsEmpty()) return kFALSE;

   // Add empty string, which will be deleted first
   fxCommands.AddFirst(new TObjString(""));
   return kTRUE;
}

Bool_t TGo4AbstractInterface::IsHotStart()
{
   return !fxCommands.IsEmpty();
}

const char *TGo4AbstractInterface::NextHotStartCmd()
{
   if (fxCommands.IsEmpty()) return nullptr;
   TObject *obj = fxCommands.First();
   fxCommands.Remove(obj);
   delete obj;
   const char *res = nullptr;
   do {
      if (fxCommands.IsEmpty()) return nullptr;
      TObjString* str = (TObjString*) fxCommands.First();
      res = str->GetName();
      if (!res || (strlen(res) == 0)) {
         res = nullptr;
         fxCommands.Remove(str);
         delete str;
      }

   } while (!res);
   return res;
}

void TGo4AbstractInterface::FreeHotStartCmds()
{
   fxCommands.Delete();
}

void TGo4AbstractInterface::ProduceLoadLibs(std::ostream& fs)
{
   TString rootsys;
   if (gSystem->Getenv("ROOTSYS") != nullptr) {
      rootsys = gSystem->Getenv("ROOTSYS");
      if (rootsys[rootsys.Length()-1] != '/') rootsys+="/";
   }

   TString go4sys = TGo4Log::GO4SYS();

   TString libs = gInterpreter->GetSharedLibs();
   const char *token = strtok((char*) libs.Data(), " ,\t\n");
   while(token != nullptr) {
      if ((fInitSharedLibs.Index(token) == kNPOS)
//           !strstr(token,"libGX11.") &&
//           !strstr(token,"libGX11TTF.") &&
           && !strstr(token,"libHistPainter.")) {
              fs << "go4->LoadLibrary(\"";
              if ((go4sys.Length() > 0) && strstr(token, go4sys.Data()) == token)
                 fs << "$GO4SYS/" << (token + go4sys.Length());
              else
              if ((rootsys.Length() > 0) && strstr(token, rootsys.Data()) == token)
                 fs << "$ROOTSYS/" << (token + rootsys.Length());
              else
                 fs << token;

              fs << "\");" << std::endl;
          }
      token = strtok(nullptr, " ,\t\n");
   }
}
