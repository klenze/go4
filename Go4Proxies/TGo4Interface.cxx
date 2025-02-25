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

#include "TGo4Interface.h"

#include "TROOT.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TStopwatch.h"
#include "TTimer.h"

#include "TGo4AnalysisStatus.h"
#include "TGo4AnalysisStepStatus.h"

#include "TGo4UserSourceParameter.h"
#include "TGo4RevServParameter.h"
#include "TGo4FileStoreParameter.h"
#include "TGo4BackStoreParameter.h"
#include "TGo4UserStoreParameter.h"
#include "TGo4FileSourceParameter.h"
#include "TGo4MbsFileParameter.h"
#include "TGo4MbsStreamParameter.h"
#include "TGo4MbsTransportParameter.h"
#include "TGo4MbsEventServerParameter.h"
#include "TGo4MbsRandomParameter.h"

#include "TGo4ObjectManager.h"
#include "TGo4BrowserProxy.h"
#include "TGo4AnalysisProxy.h"
#include "TGo4RootBrowserProxy.h"

TGo4Interface::TGo4Interface() :
   TGo4AbstractInterface(),
   fRootBrowser(nullptr),
   fOMDataPath(),
   fOMBrowserPath(),
   fOMEditorsPath(),
   fCmdFinished(kTRUE),
   fCmdTimer(nullptr),
   fWaitCounter(0)
{
   // create object manager
   TGo4ObjectManager* om =
      new TGo4ObjectManager("GUI_OM","Gui object manager");
   fOMDataPath    = "data";
   fOMBrowserPath = "gui";
   fOMEditorsPath = "editors";

   // create mount point for all data sources
   om->MakeFolder(fOMDataPath.Data());

   // create entry for browser
   TGo4BrowserProxy* br =
      new TGo4BrowserProxy(fOMDataPath.Data(), fOMEditorsPath.Data(), kTRUE);
   om->AddProxy("", br, fOMBrowserPath.Data(), "Place for gui slots");
   br->CreateMemoryFolder();

   // create entry for gui browser
   fRootBrowser = new TGo4RootBrowserProxy(br);
   om->AddProxy("", fRootBrowser, fOMEditorsPath.Data(), "Proxy for ROOT browser");
   om->AddLink(br->BrowserTopSlot(), fOMEditorsPath.Data(), "Go4Browser", "Link to Go4 browser");

   Initialize(om, br);
}

TGo4Interface::~TGo4Interface()
{
   DisconnectAnalysis();
   delete fCmdTimer;
   fCmdTimer = nullptr;

   Int_t timecnt = 50;
   while (timecnt-->0) {
      if (!Analysis()) break;
      gSystem->ProcessEvents();
      gSystem->Sleep(100);
   }

   TGo4ObjectManager* om = OM();
   delete om;
}

void TGo4Interface::ProcessEvents(Int_t timeout)
{
   Wait(timeout <= 0 ? 0 : timeout*0.001);
}

void TGo4Interface::Wait(double tm_sec)
{
   TStopwatch watch;

   Double_t spenttime = 0.;
   Bool_t first = kTRUE;

   do {
      watch.Start();
      if (first) first = kFALSE;
            else gSystem->Sleep(50);
      gSystem->ProcessEvents();
      watch.Stop();
      spenttime += watch.RealTime();
   } while (spenttime <= tm_sec);
}

void TGo4Interface::Message(const char *msg)
{
   if (fRootBrowser)
      fRootBrowser->Message("Message","msg",5000);
}

void TGo4Interface::LaunchAnalysis()
{
   // do nothing while no parameters is specified
   // work in Qt GUI while parameters can be taken from the QSettings
}

TGo4AnalysisProxy* TGo4Interface::AddAnalysisProxy(Bool_t isserver)
{
   const char *analisysitem = "Analysis";

   TGo4Slot* analslot = Browser()->DataSlot(analisysitem);

   if (!analslot) {
      TGo4AnalysisProxy* anal = new TGo4AnalysisProxy(isserver);
      OM()->AddProxy(fOMDataPath.Data(), anal, analisysitem, "Analysis proxy");
      analslot = Browser()->DataSlot(analisysitem);
      anal->SetDefaultReceiver(OM(), TString("gui/") + analisysitem + "/");
   }

   TGo4AnalysisProxy* anal = !analslot ? nullptr :
      dynamic_cast<TGo4AnalysisProxy*> (analslot->GetProxy());

   if (anal)
     anal->SetAnalysisReady(kFALSE);

   if (anal && fRootBrowser) {
      OM()->AddLink(anal->RatemeterSlot(), fOMEditorsPath.Data(), "AnalRateMeter", "Analysis ratemeter");
      OM()->AddLink(anal->LoginfoSlot(), fOMEditorsPath.Data(), "AnalLoginfo", "Analysis loginfo");
   }

   return anal;
}


void TGo4Interface::LaunchAnalysis(const char *ClientName,
                                   const char *ClientDir,
                                   const char *ClientExec,
                                   const char *ClientNode,
                                   Int_t ShellMode,
                                   Int_t TermMode,
                                   Int_t ExeMode,
                                   const char *UserArgs)
{
   TString launchcmd, killcmd;

   TGo4AnalysisProxy* anal = AddAnalysisProxy(kFALSE);
   if (!anal) return;

   anal->LaunchAsClient(launchcmd, killcmd,
                       (ShellMode==2),
                       TermMode==2 ? 2 : 3,
                       ClientName,
                       ClientNode,
                       ClientDir,
                       ClientExec,
                       ExeMode,
                       UserArgs);

   gSystem->ProcessEvents();

   ProcessEvents(200);
}

void TGo4Interface::ConnectAnalysis(const char *ServerNode,
                                    Int_t ServerPort,
                                    Int_t ControllerMode,
                                    const char *password)
{
   TGo4AnalysisProxy* anal = Analysis();
   if (!anal) anal = AddAnalysisProxy(kTRUE);

   if (!anal) return;

   anal->ConnectToServer(ServerNode,
                         ServerPort,
                         ControllerMode,
                         password);

   if (ControllerMode>0)
     anal->RequestAnalysisSettings();

//   gSystem->ProcessEvents();
   ProcessEvents(200);
}

void TGo4Interface::WaitAnalysis(Int_t delay_sec)
{
   if (IsHotStart()) {
      fWaitCounter = delay_sec*100;
      return;
   }

   TGo4AnalysisProxy* anal = Analysis();
   if (!anal) {
       Error("WaitAnalysis","Analysis not found");
       return;
   }

   delay_sec*=10;

   while (delay_sec-->0) {
      gSystem->ProcessEvents();
      gSystem->Sleep(100);
      gSystem->ProcessEvents();
      if (anal->IsAnalysisReady())
         return;
   }
}

void TGo4Interface::DisconnectAnalysis()
{
   TGo4AnalysisProxy* anal = Analysis();
   if (!anal) return;

   Browser()->ToggleMonitoring(0);

   anal->DisconnectAnalysis(5, kFALSE);

   ProcessEvents(200);
}

void TGo4Interface::ShutdownAnalysis()
{
   TGo4AnalysisProxy* anal = Analysis();
   if (!anal) return;

   Browser()->ToggleMonitoring(0);

   bool realshutdown = anal->IsAnalysisServer() &&
                       anal->IsConnected() &&
                       anal->IsAdministrator();

   anal->DisconnectAnalysis(30, realshutdown);
}

void TGo4Interface::SubmitAnalysisConfig(int tmout)
{
   TGo4ServerProxy* anal = Server();
   if (!anal) return;

   anal->SubmitAnalysisSettings();
   anal->RefreshNamesList();

   Int_t delay_sec = tmout > 0 ? tmout : 20;
   while (delay_sec-->0) {
      gSystem->ProcessEvents();
      gSystem->Sleep(1000);
      gSystem->ProcessEvents();
      if (anal->IsAnalysisSettingsReady())
        if (anal->NamesListReceived()) return;
   }
}

void TGo4Interface::StartAnalysis()
{
   TGo4ServerProxy* anal = Server();
   if (!anal) return;

   anal->StartAnalysis();
   anal->RefreshNamesList();
   anal->DelayedRefreshNamesList(4);

   gSystem->ProcessEvents();
   gSystem->Sleep(500);
   gSystem->ProcessEvents();
}

void TGo4Interface::StopAnalysis()
{
   TGo4ServerProxy* anal = Server();
   if (!anal) return;

   anal->StopAnalysis();

   gSystem->ProcessEvents();
   gSystem->Sleep(1000);
   gSystem->ProcessEvents();
}

void TGo4Interface::RefreshNamesList(int tmout)
{
   TGo4ServerProxy* anal = Server();
   if (!anal) return;

   anal->RefreshNamesList();

   Int_t delay_sec = tmout > 0 ? tmout : 10;
   while (delay_sec-->0) {
      gSystem->ProcessEvents();
      gSystem->Sleep(1000);
      gSystem->ProcessEvents();
      if (anal->NamesListReceived()) return;
   }
}


TGo4AnalysisStatus* TGo4Interface::GetAnalStatus()
{
   TGo4ServerProxy* anal = Server();
   if (!anal) return nullptr;
   if (!anal->SettingsSlot()) return nullptr;

   return dynamic_cast<TGo4AnalysisStatus*>
            (anal->SettingsSlot()->GetAssignedObject());
}

void TGo4Interface::AnalysisAutoSave(const char *filename,
                                         Int_t interval,
                                         Int_t compression,
                                         Bool_t enabled,
                                         Bool_t overwrite)
{
   TGo4AnalysisStatus* status  = GetAnalStatus();
   if (!status) return;

   status->SetAutoFileName(filename);
   status->SetAutoSaveInterval(interval);
   status->SetAutoSaveCompression(compression);
   status->SetAutoSaveOn(enabled);
   status->SetAutoSaveOverwrite(overwrite);
}

void TGo4Interface::AnalysisConfigName(const char *filename)
{
   TGo4AnalysisStatus* status  = GetAnalStatus();
   if (status)
     status->SetConfigFileName(filename);
}

TGo4AnalysisStepStatus* TGo4Interface::GetStepStatus(const char *stepname)
{
   TGo4AnalysisStatus* status = GetAnalStatus();
   return !status ? nullptr : status->GetStepStatus(stepname);
}

void TGo4Interface::ConfigStep(const char *stepname,
                                   Bool_t enableprocess,
                                   Bool_t enablesource,
                                   Bool_t enablestore)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   step->SetSourceEnabled(enablesource);
   step->SetStoreEnabled(enablestore);
   step->SetProcessEnabled(enableprocess);
}

void TGo4Interface::StepFileSource(const char *stepname,
                                       const char *sourcename,
                                       int timeout)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4FileSourceParameter par(sourcename);
   par.SetTimeout(timeout);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepMbsFileSource(const char *stepname,
                                      const char *sourcename,
                                      int timeout,
                                      const char *TagFile,
                                      int start,
                                      int stop,
                                      int interval)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4MbsFileParameter par(sourcename);
   par.SetTimeout(timeout);

   if(!TagFile) par.SetTagName("GO4-NOLMDTAG");
           else par.SetTagName(TagFile);
   par.SetStartEvent(start);
   par.SetStopEvent(stop);
   par.SetEventInterval(interval);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepMbsStreamSource(const char *stepname,
                                        const char *sourcename,
                                        int timeout,
                                        int start,
                                        int stop,
                                        int interval)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4MbsStreamParameter par(sourcename);
   par.SetTimeout(timeout);
   par.SetStartEvent(start);
   par.SetStopEvent(stop);
   par.SetEventInterval(interval);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepMbsTransportSource(const char *stepname,
                                        const char *sourcename,
                                        int timeout,
                                        int start,
                                        int stop,
                                        int interval)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4MbsTransportParameter par(sourcename);
   par.SetTimeout(timeout);
   par.SetStartEvent(start);
   par.SetStopEvent(stop);
   par.SetEventInterval(interval);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepMbsEventServerSource(const char *stepname,
                                          const char *sourcename,
                                          int timeout,
                                          int start,
                                          int stop,
                                          int interval)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4MbsEventServerParameter par(sourcename);
   par.SetTimeout(timeout);
   par.SetStartEvent(start);
   par.SetStopEvent(stop);
   par.SetEventInterval(interval);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepMbsRevServSource(const char *stepname,
                                      const char *sourcename,
                                      int timeout,
                                      int port,
                                      int start,
                                      int stop,
                                      int interval)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4RevServParameter par(sourcename);
   par.SetTimeout(timeout);
   par.SetPort(port);
   par.SetStartEvent(start);
   par.SetStopEvent(stop);
   par.SetEventInterval(interval);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepRandomSource(const char *stepname,
                                  const char *sourcename,
                                  int timeout)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4MbsRandomParameter par(sourcename);
   par.SetTimeout(timeout);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepUserSource(const char *stepname,
                                const char *sourcename,
                                int timeout,
                                int port,
                                const char *expr)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4UserSourceParameter par(sourcename);
   par.SetTimeout(timeout);
   par.SetPort(port);
   par.SetExpression(expr);
   step->SetSourcePar(&par);
}

void TGo4Interface::StepFileStore(const char *stepname,
                               const char *storename,
                               bool overwrite,
                               int bufsize,
                               int splitlevel,
                               int compression,
                               int autosaveperiod)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4FileStoreParameter par(storename);
   par.SetOverwriteMode(overwrite);
   par.SetBufsize(bufsize);
   par.SetSplitlevel(splitlevel);
   par.SetCompression(compression);
   par.SetAutosaveSize(autosaveperiod);
   step->SetStorePar(&par);
}

void TGo4Interface::StepBackStore(const char *stepname,
                               const char *storename,
                               int bufsize,
                               int splitlevel)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4BackStoreParameter par(storename);
   par.SetBufsize(bufsize);
   par.SetSplitlevel(splitlevel);
   step->SetStorePar(&par);
}

void TGo4Interface::StepUserStore(const char *stepname,
                   const char *storename)
{
   TGo4AnalysisStepStatus* step = GetStepStatus(stepname);
   if (!step) return;

   TGo4UserStoreParameter par(storename);
   step->SetStorePar(&par);
}

ViewPanelHandle TGo4Interface::StartViewPanel()
{
   return StartViewPanel(10,10, 500, 300, 1, nullptr);
}

ViewPanelHandle TGo4Interface::StartViewPanel(int x, int y, int width, int height, int mode, TGo4Picture* pic)
{
   static Int_t cancounter = 0;

   TString cname = "Panel";
   cname+=cancounter++;

   TCanvas* c = new TCanvas(cname.Data(), TString("Drawing of ") + cname, width, height);

   fRootBrowser->DrawPicture("", pic, c);

   c->Update();

   return (ViewPanelHandle) c;
}

TString TGo4Interface::GetViewPanelName(ViewPanelHandle handle)
{
   TCanvas* c = (TCanvas*) handle;
   return TString(c ? c->GetName() : "");
}

ViewPanelHandle TGo4Interface::FindViewPanel(const char *name)
{
   return (ViewPanelHandle) gROOT->GetListOfCanvases()->FindObject(name);
}

Bool_t TGo4Interface::SetViewPanelName(ViewPanelHandle handle, const char *newname)
{
   TCanvas* c = (TCanvas*) handle;
   if (!c || !newname || (strlen(newname) == 0)) return kFALSE;

   if (gROOT->GetListOfCanvases()->FindObject(newname)) {
      Message(TString::Format("Canvas with name %s already exists",newname).Data());
      return kFALSE;
   }

   c->SetName(newname);

   c->Update();

   return kTRUE;
}

ViewPanelHandle TGo4Interface::GetActiveViewPanel()
{
   return (ViewPanelHandle) (gPad ? gPad->GetCanvas() : nullptr);
}

void TGo4Interface::RedrawPanel(ViewPanelHandle handle)
{
   TCanvas* c = (TCanvas*) handle;
   if (c) {
      c->Modified();
      c->Update();
   }
}

void TGo4Interface::DivideViewPanel(ViewPanelHandle handle, Int_t numX, Int_t numY)
{
    TCanvas* c = (TCanvas*) handle;
    if (c) c->Divide(numX, numY);
}

TPad* TGo4Interface::SelectPad(ViewPanelHandle handle, Int_t number)
{
    TCanvas* c = (TCanvas*) handle;
    if (c) return (TPad*) c->cd(number);
    return nullptr;
}

Bool_t TGo4Interface::DrawItem(const char *itemname, ViewPanelHandle handle, const char *drawopt)
{
    if (!handle) handle = StartViewPanel();

    TObject *obj = GetObject(itemname, 5000);

    if (obj) obj->Draw(drawopt);

    return obj != nullptr;
}

void TGo4Interface::RedrawItem(const char *itemname)
{
   TGo4AbstractInterface::RedrawItem(itemname);

   TIter next(gROOT->GetListOfCanvases());
   while (auto pad = (TPad*) next()) {
      pad->Modified();

      TVirtualPad* subpad = nullptr;
      Int_t number = 0;
      while ((subpad = pad->GetPad(number++)) != nullptr)
        subpad->Modified();

      pad->Update();
   }
}

Bool_t TGo4Interface::HandleTimer(TTimer* timer)
{
   // if there is no commands remains, just exit
   if (!IsHotStart()) return kTRUE;

   if (timer!=fCmdTimer) return kFALSE;

   // for the moment, only WaitAnalysis requries that strange waiting
   // during hotstart. In normal script simple ProcessEvents and Sleep works

   if (fWaitCounter>0) {
       fWaitCounter--;
       TGo4AnalysisProxy* anal = Analysis();
       if (anal && anal->IsAnalysisReady())
          fWaitCounter = 0;
       else
          if (fWaitCounter<2) FreeHotStartCmds(); // if cannot wait analysis, just break execution
   } else
      ProcessHotStart();

   if (IsHotStart()) fCmdTimer->Start(10, kTRUE);
   return kTRUE;
}


void TGo4Interface::HotStart(const char *filename)
{
   if (!filename || (strlen(filename) == 0)) return;

   FreeHotStartCmds();

   fCmdFinished = kTRUE;

   if (!LoadHotStart(filename)) return;

   if (!fCmdTimer)
      fCmdTimer = new TTimer(this, 10, kTRUE);

   fCmdTimer->Start(10, kTRUE);
}

void TGo4Interface::ProcessHotStart()
{
   if (!fCmdFinished) {
       Error("ProcessHotStart","Internal problem");
       return;
   }

   const char *nextcmd = NextHotStartCmd();

   if (!nextcmd || !IsHotStart()) return;

   Int_t error = 0;
   fCmdFinished = kFALSE;
   gROOT->ProcessLineSync(nextcmd, &error);
   fCmdFinished = kTRUE;

   if (error != 0)
      Error("ProcessHotStart", "ProcessLine(\"%s\") error %d", nextcmd, error);
}
