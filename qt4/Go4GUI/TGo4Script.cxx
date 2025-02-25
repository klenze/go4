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

#include "TGo4Script.h"

#include <fstream>

#include "TString.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TStopwatch.h"

#include "TGo4QSettings.h"
#include "TGo4BrowserProxy.h"
#include "TGo4AnalysisProxy.h"
#include "TGo4DirProxy.h"
#include "TGo4HServProxy.h"
#include "TGo4DabcProxy.h"
#include "TGo4HttpProxy.h"
#include "TGo4Iter.h"

#include "TGo4AnalysisConfiguration.h"
#include "TGo4ConfigStep.h"
#include "TGo4AnalysisWindow.h"
#include "TGo4MdiArea.h"
#include "TGo4Picture.h"
#include "TGo4ViewPanel.h"
#include "TGo4MainWindow.h"

#include <QMdiSubWindow>


TGo4Script* TGo4Script::ScriptInstance()
{
   return dynamic_cast<TGo4Script*> (Instance());
}

TGo4Script::TGo4Script(TGo4MainWindow* mainwin) :
   TGo4AbstractInterface(),
   fiWaitForGUIReaction(0),
   fiWaitCounter(0),
   fStrBuf(),
   fMainWin(mainwin),
   fErrorFlag(kFALSE),
   fBlockConfigFlag(0)
{
   if (mainwin)
      Initialize(mainwin->OM(), mainwin->Browser());
}

TGo4Script::~TGo4Script()
{
}


Bool_t TGo4Script::StartScriptExecution(const char *fname)
{
   if (!fname || (strlen(fname) == 0)) return kFALSE;

   FinishExecution();

   if (!LoadHotStart(fname)) return kFALSE;

   fiWaitForGUIReaction = 0;
   fiWaitCounter = getCounts(0.1);

   doOutput("Start hotstart script execution");

   return kTRUE;
}

Int_t TGo4Script::getCounts(Double_t tm)
{
   return int(tm*1000./DelayMillisec());
}

void  TGo4Script::doOutput(const char *str)
{
   if (fMainWin)
     fMainWin->StatusMessage(str);
}

Bool_t TGo4Script::ContinueExecution()
{
   if (!fMainWin || !IsHotStart()) return kFALSE;

   switch (execGUICommands()) {
      case 0:
         return kFALSE;
      case 1:
         fiWaitForGUIReaction = 0;
         fiWaitCounter = 0;
         break;
      case 2:
         fiWaitCounter--;
         return kTRUE;
   }

   const char *nextcmd = NextHotStartCmd();
   if (!nextcmd) return kFALSE;

   Int_t error = 0;
   gROOT->ProcessLineSync(nextcmd, &error);

   if (error != 0) {
      doOutput(TString::Format("Error = %d. CMD: %s", error, nextcmd).Data());
      return kFALSE;
   }

   return kTRUE;
}

void TGo4Script::FinishExecution()
{
   FreeHotStartCmds();
   fErrorFlag = kFALSE;
   fiWaitForGUIReaction = 0;
   fiWaitCounter = 0;
}

Bool_t TGo4Script::IsWaitSomething()
{
   return fiWaitCounter>0;
}

void TGo4Script::DoPostProcessing()
{
   // do postprocessing of command only if normal script is executed
   if (IsHotStart()) return;

   // if error was before, nothing todo
   if (fErrorFlag || !fMainWin) return;

   TStopwatch watch;

   Int_t loopnumbers = 0;
   Double_t spenttime = 0;

   do {

      loopnumbers++;
      Bool_t first = kTRUE;

      do {
         watch.Start();
         if (first) first = kFALSE;
               else gSystem->Sleep(DelayMillisec());
         fMainWin->ProcessQtEvents();
         watch.Stop();
         spenttime += watch.RealTime();
      } while (spenttime < loopnumbers*0.001*DelayMillisec());

      switch (execGUICommands()) {
         case 0: // error
            fiWaitForGUIReaction = 0;
            fiWaitCounter = 0;
            fErrorFlag = kTRUE;
            return;
         case 1: // finish command
            fiWaitForGUIReaction = 0;
            fiWaitCounter = 0;
            return;
         case 2: // next turn
            fiWaitCounter--;
      }
   } while (loopnumbers < 1000000);
}

void TGo4Script::ProcessEvents(Int_t timeout)
{
   Wait(timeout <= 0 ? 0 : timeout*0.001);
}

Int_t TGo4Script::execGUICommands()
{
   switch (fiWaitForGUIReaction) {
      case 0:
        return 1;

      case 1: { // execute launch client method
         TGo4ServerProxy* anal = Server();

         if(anal && anal->IsAnalysisReady()) {
             doOutput("// Disconnect old analysis first");
             fMainWin->DisconnectAnalysis(false);
             fiWaitForGUIReaction = 3;
             fiWaitCounter = getCounts(10.);
             return 2;
         }

         fMainWin->LaunchClient(false);

         doOutput("// Start analysis client");

         // wait 5 sec for analysis ready
         fiWaitForGUIReaction = 2;
         fiWaitCounter = getCounts(5.);

         return 2;
      }

      case 2: {  // check if analysis is ready, can continue if not ok
         TGo4ServerProxy* anal = Server();
         if (anal && anal->IsAnalysisReady()) return 1;
         return (fiWaitCounter<2) ? 1 : 2;
      }

      case 3: {   // wait while analysis is disconnected
         TGo4ServerProxy* anal = Server();
         if(!anal || !anal->IsAnalysisReady()) {
            fiWaitForGUIReaction = 4;
            fiWaitCounter = getCounts(5.);
            return 2;
         }
         return (fiWaitCounter<2) ? 0 : 2;
      }

      case 4: { // wait time and lunch analysis again
        if (fiWaitCounter<5)
          fiWaitForGUIReaction = 1;
        return 2;
      }

      case 5: {  // check if analysis is ready, abort if not ok
         TGo4ServerProxy* anal = Server();
         if (!anal) return 0;
         if (anal->IsAnalysisReady()) return 1;
         return (fiWaitCounter<2) ? 0 : 2;
      }

      case 10: { // wait until submitted settings are set
        TGo4ServerProxy* anal = Server();
        if (anal && anal->IsAnalysisSettingsReady()) {
          fiWaitForGUIReaction = 11;
          // fiWaitCounter = getCounts(20.); // counter is for complete operation
          return 2;
        }
        return (fiWaitCounter<2) ? 0 : 2;
      }

      case 11: { // wait until remote browser refresh it's data
         TGo4ServerProxy* anal = Server();
         if (anal && anal->NamesListReceived()) return 1;
         return (fiWaitCounter<2) ? 0 : 2;
      }

      case 12: { // wait until analysis will be disconnected
         TGo4ServerProxy* anal = Server();
         if (!anal) return 1;
         return (fiWaitCounter<2) ? 0 : 2;
      }

      case 15: { // connect to running server
         fMainWin->ConnectServer(false, fStrBuf.Data());

         // wait 5 sec for analysis ready,
         fiWaitForGUIReaction = 2;
         fiWaitCounter = getCounts(5.);

         return 2;
      }

      case 111: { // just wait time and take next command
         return fiWaitCounter < 2 ? 1 : 2;
      }
   } //switch

   return 2;
}

// ******************* Functions to be used in GUI script *************

void TGo4Script::Wait(double tm_sec)
{
   fiWaitForGUIReaction = 111;
   fiWaitCounter = getCounts(tm_sec);
   if (fiWaitCounter<0) fiWaitCounter = 1;

   DoPostProcessing();
}


void TGo4Script::Message(const char *msg)
{
   if (fMainWin)
     fMainWin->StatusMessage(msg);
}

void TGo4Script::HotStart(const char *filename)
{
   fMainWin->HotStart(filename);
}

void TGo4Script::LaunchAnalysis()
{
   fiWaitForGUIReaction = 1;
   fiWaitCounter = getCounts(1.);

   DoPostProcessing();
}

void TGo4Script::LaunchAnalysis(const char *ClientName,
                                const char *ClientDir,
                                const char *ClientExec,
                                const char *ClientNode,
                                Int_t ShellMode,
                                Int_t TermMode,
                                Int_t ExeMode,
                                const char *UserArgs)
{
   go4sett->setClientName(ClientName);
   go4sett->setClientDir(ClientDir);
   go4sett->setClientExeMode(ExeMode);
   go4sett->setClientExec(ClientExec);
   go4sett->setClientArgs(UserArgs);
   const char *separ = strrchr(ClientNode, ':');
   if (!separ) {
      go4sett->setClientNode(ClientNode);
      go4sett->setClientIsServer(0);
   } else {
      TString node;
      node.Append(ClientNode, separ - ClientNode);
      go4sett->setClientNode(node.Data());
      go4sett->setClientIsServer(2);
      go4sett->setClientPort(TString(separ+1).Atoi());
   }

   go4sett->setClientShellMode(ShellMode);
   go4sett->setClientTermMode(TermMode);
   LaunchAnalysis();
}

void TGo4Script::ConnectAnalysis(const char *ServerNode,
                                 Int_t ServerPort,
                                 Int_t ControllerMode,
                                 const char *password)
{
   go4sett->setClientNode(ServerNode);
   go4sett->setClientPort(ServerPort);
   go4sett->setClientControllerMode(ControllerMode);
   go4sett->setClientDefaultPass(!password || (*password == 0));
   fiWaitForGUIReaction = 15;
   fStrBuf = password;
   fiWaitCounter = getCounts(1.);

   DoPostProcessing();
}

void TGo4Script::WaitAnalysis(Int_t delay_sec)
{
   fiWaitForGUIReaction = 5;
   fiWaitCounter = getCounts(delay_sec);

   DoPostProcessing();
}

void TGo4Script::DisconnectAnalysis()
{
    fMainWin->DisconnectAnalysis(false);

    fiWaitForGUIReaction = 12;
    fiWaitCounter = getCounts(20.);

    DoPostProcessing();
}

void TGo4Script::ShutdownAnalysis()
{
   fMainWin->ShutdownAnalysis(false);

   fiWaitForGUIReaction = 12;
   fiWaitCounter = getCounts(20.);

   DoPostProcessing();
}

void TGo4Script::SubmitAnalysisConfig(int tmout)
{
   if (CanConfigureAnalysis())
      if (fMainWin->SubmitAnalysisSettings()) {
         fiWaitForGUIReaction = 10;
         fiWaitCounter = getCounts(tmout);
      }

   DoPostProcessing();
}

void TGo4Script::StartAnalysis()
{
   if (CanConfigureAnalysis()) {
      fMainWin->StartAnalysisSlot();
      if (dynamic_cast<TGo4HttpProxy*>(Server()))
         Wait(0.1);
      else
         Wait(1.);
   } else {
      fBlockConfigFlag = -1; // from this command blocking is disabled
   }
}

void TGo4Script::StopAnalysis()
{
   if (CanConfigureAnalysis()) {
      fMainWin->StopAnalysisSlot();
      if (dynamic_cast<TGo4HttpProxy*>(Server()))
         Wait(0.1);
      else
         Wait(2.);
   } else {
      fBlockConfigFlag = -1; // from this command blocking is disabled
   }
}

void TGo4Script::RefreshNamesList(int tmout)
{
   TGo4ServerProxy* anal = Server();
   if (anal) {
      anal->RefreshNamesList();
      fiWaitForGUIReaction = 11;
      fiWaitCounter = getCounts(tmout > 0 ? tmout : 10);
   }

   DoPostProcessing();
}

Bool_t TGo4Script::CanConfigureAnalysis()
{
   if (fBlockConfigFlag == 0) {
      // once a session check if analysis was not launched from the GUI, we should not try to configure it
      TGo4HttpProxy* serv = dynamic_cast<TGo4HttpProxy*> (Server());
      if (serv && !serv->IsAnalysisLaunched()) fBlockConfigFlag = 1;
   }

   return fBlockConfigFlag <= 0;
}


void TGo4Script::SetAnalysisTerminalMode(int mode)
{
   TGo4AnalysisWindow* gui_ = fMainWin->FindAnalysisWindow();
   QWidget* gui = gui_ ? gui_->parentWidget() : nullptr;
   if (!gui) return;

   if (mode < 0)
      gui->hide();
   else if (mode == 0)
      gui->showMinimized();
   else
      gui->showNormal();
}

void TGo4Script::SetAnalysisConfigMode(int mode)
{
   TGo4AnalysisConfiguration* gui_ = fMainWin->FindAnalysisConfiguration();
   QWidget* gui = gui_ ? gui_->parentWidget() : nullptr;
   if (!gui) return;

   if (mode < 0)
      gui->hide();
   else if (mode == 0)
      gui->showMinimized();
   else
      gui->showNormal();
}

void TGo4Script::AnalysisAutoSave(const char *filename,
                                       Int_t interval,
                                       Int_t compression,
                                       Bool_t enabled,
                                       Bool_t overwrite)
{
   TGo4AnalysisConfiguration* gui = fMainWin->FindAnalysisConfiguration();
   if(gui && CanConfigureAnalysis())
     gui->SetAutoSaveConfig(filename, interval, compression, enabled, overwrite);
}

void TGo4Script::AnalysisConfigName(const char *filename)
{
   TGo4AnalysisConfiguration* gui = fMainWin->FindAnalysisConfiguration();
   if(gui && CanConfigureAnalysis())
      gui->SetAnalysisConfigFile(filename);
}

TGo4ConfigStep* TGo4Script::GetStepGUI(const char *stepname)
{
   // in cannot configure analysis - do not return step gui pointer
   if (!CanConfigureAnalysis()) return nullptr;

   TGo4AnalysisConfiguration* gui = fMainWin->FindAnalysisConfiguration();

   return !gui ? nullptr : gui->FindStepConfig(stepname);
}

void TGo4Script::ConfigStep(const char *stepname,
                            Bool_t enableprocess,
                            Bool_t enablesource,
                            Bool_t enablestore)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step)
      step->SetStepControl(enableprocess, enablesource, enablestore);
}

void TGo4Script::StepFileSource(const char *stepname,
                                const char *sourcename,
                                int timeout)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetFileSource();
      step->SetSourceWidgets(sourcename, timeout);
   }
}

void TGo4Script::StepMbsFileSource(const char *stepname,
                                   const char *sourcename,
                                   int timeout,
                                   const char *TagFile,
                                   int start,
                                   int stop,
                                   int interval)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetMbsFileSource(TagFile);
      step->SetSourceWidgets(sourcename, timeout);
      step->SetMbsSourceWidgets(start, stop, interval);
   }
}

void TGo4Script::StepMbsStreamSource(const char *stepname,
                                     const char *sourcename,
                                     int timeout,
                                     int start,
                                     int stop,
                                     int interval)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetMbsStreamSource();
      step->SetSourceWidgets(sourcename, timeout);
      step->SetMbsSourceWidgets(start, stop, interval);
   }
}

void TGo4Script::StepMbsTransportSource(const char *stepname,
                                        const char *sourcename,
                                        int timeout,
                                        int start,
                                        int stop,
                                        int interval)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetMbsTransportSource();
      step->SetSourceWidgets(sourcename, timeout);
      step->SetMbsSourceWidgets(start, stop, interval);
   }
}

void TGo4Script::StepMbsEventServerSource(const char *stepname,
                                          const char *sourcename,
                                          int timeout,
                                          int start,
                                          int stop,
                                          int interval)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetMbsEventServerSource();
      step->SetSourceWidgets(sourcename, timeout);
      step->SetMbsSourceWidgets(start, stop, interval);
   }
}

void TGo4Script::StepMbsRevServSource(const char *stepname,
                                      const char *sourcename,
                                      int timeout,
                                      int port,
                                      int start,
                                      int stop,
                                      int interval)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetMbsRevServSource(port);
      step->SetSourceWidgets(sourcename, timeout);
      step->SetMbsSourceWidgets(start, stop, interval, port);
   }
}

void TGo4Script::StepMbsSelection(const char *stepname,
                                  int start,
                                  int stop,
                                  int interval)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step)
      step->SetMbsSourceWidgets(start, stop, interval);
}

void TGo4Script::StepRandomSource(const char *stepname,
                                  const char *sourcename,
                                  int timeout)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetRandomSource();
      step->SetSourceWidgets(sourcename, timeout);
   }
}

void TGo4Script::StepMbsPort(const char *stepname,
                             int port)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) step->SetMbsPort(port);
}

void TGo4Script::StepMbsRetryCnt(const char *stepname,
                                 int cnt)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) step->SetMbsRetryCnt(cnt);
}

void TGo4Script::StepUserSource(const char *stepname,
                                const char *sourcename,
                                int timeout,
                                int port,
                                const char *expr)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step) {
      step->SetUserSource(port, expr);
      step->SetSourceWidgets(sourcename, timeout);
   }
}


void TGo4Script::StepHDF5Source(const char *stepname,
                                const char *sourcename,
                                int timeout)
{
    TGo4ConfigStep* step = GetStepGUI(stepname);
     if (step) {
        step->SetHDF5Source(sourcename);
        step->SetSourceWidgets(sourcename, timeout);
     }
}

void TGo4Script::StepFileStore(const char *stepname,
                               const char *storename,
                               bool overwrite,
                               int bufsize,
                               int splitlevel,
                               int compression,
                               int autosaveperiod)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step)
      step->SetFileStore(storename, overwrite, bufsize, splitlevel, compression, autosaveperiod);
}

void TGo4Script::StepBackStore(const char *stepname,
                               const char *storename,
                               int bufsize,
                               int splitlevel)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step)
      step->SetBackStore(storename, bufsize, splitlevel);
}

void TGo4Script::StepUserStore(const char *stepname,
                               const char *storename)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step)
      step->SetUserStore(storename);
}

void TGo4Script::StepHDF5Store(const char *stepname,
                               const char *storename,
                               int flags)
{
   TGo4ConfigStep* step = GetStepGUI(stepname);
   if (step)
      step->SetHDF5Store(storename, flags);
}

void TGo4Script::SetMainWindowState(int qtversion, const char *val)
{
   if (qtversion==4)
      fMainWin->restoreState(QByteArray::fromHex(val));
}

void TGo4Script::SetMainWindowGeometry(int qtversion, const char *val)
{
   if (qtversion==4)
      fMainWin->restoreGeometry(QByteArray::fromHex(val));
}

ViewPanelHandle TGo4Script::StartViewPanel()
{
   return fMainWin->MakeNewPanel(1);
}

ViewPanelHandle TGo4Script::StartViewPanel(int x, int y, int width, int height, int mode, TGo4Picture* pic)
{
   TGo4ViewPanel* panel = fMainWin->MakeNewPanel(1);
   if (!panel) return (ViewPanelHandle) nullptr;

   if (mode == 0)
      panel->showMinimized();
   else if (mode == 2)
      panel->showMaximized();
   else {
      panel->showNormal();
      panel->parentWidget()->move(QPoint(x, y));
      panel->parentWidget()->resize(QSize(width, height));
   }

   if (pic) {
      panel->ProcessPictureRedraw("", panel->GetCanvas(), pic);
      if (TString(DefaultPicTitle()) != pic->GetTitle())
         panel->SetFreezedTitle(pic->GetTitle());

      panel->ShootRepaintTimer();
   }

   return (ViewPanelHandle) panel;
}

TString TGo4Script::GetViewPanelName(ViewPanelHandle handle)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (!panel) return TString();
   return TString(panel->objectName().toLatin1().constData());
}

ViewPanelHandle TGo4Script::FindViewPanel(const char *name)
{
   return (ViewPanelHandle) fMainWin->FindViewPanel(name);
}

Bool_t TGo4Script::SetViewPanelName(ViewPanelHandle handle, const char *newname)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (!handle || !newname || (strlen(newname) == 0)) return kFALSE;

   if (fMainWin->FindViewPanel(newname)) {
      Message(TString::Format("Viewpanel with name %s already exists",newname).Data());
      return kFALSE;
   }

   panel->SetPanelName(newname);

   panel->UpdatePanelCaption();

   return kTRUE;
}

ViewPanelHandle TGo4Script::GetActiveViewPanel()
{
   return (ViewPanelHandle) TGo4MdiArea::Instance()->GetActivePanel();
}

void TGo4Script::RedrawPanel(ViewPanelHandle handle)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (panel) panel->RedrawPanel(panel->GetCanvas(), true);
}

void TGo4Script::DivideViewPanel(ViewPanelHandle handle, Int_t numX, Int_t numY)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (panel) panel->Divide(numX, numY);
}

TPad* TGo4Script::SelectPad(ViewPanelHandle handle, Int_t number)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (panel) {
      TPad* pad = panel->GetSubPad(nullptr, number, false);
      if (!pad) pad = panel->GetCanvas();
      panel->SetActivePad(pad);
      ProcessEvents();
      return pad;
   }
   return nullptr;
}

TGo4Picture* TGo4Script::GetPadOptions(ViewPanelHandle handle, Int_t padnumber)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (!panel) return nullptr;

   TPad* pad = panel->GetSubPad(nullptr, padnumber, false);
   if (!pad) pad = panel->GetCanvas();
   return panel->GetPadOptions(pad);
}

void TGo4Script::SetSuperimpose(ViewPanelHandle handle, Bool_t on)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (panel) panel->SetPadSuperImpose(panel->GetActivePad(), on);
}

void TGo4Script::SetApplyToAll(ViewPanelHandle handle, Bool_t on)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;
   if (panel) panel->SetApplyToAllFlag(on);
}

Bool_t TGo4Script::DrawItem(const char *itemname, ViewPanelHandle handle, const char *drawopt)
{
   TGo4ViewPanel *panel = (TGo4ViewPanel *)handle;

   panel = fMainWin->DisplayBrowserItem(itemname, panel, nullptr, true, -1, drawopt);

   if (panel) ProcessEvents();

   return panel != nullptr;
}

const char *TGo4Script::GetDrawnItemName(ViewPanelHandle handle, int cnt)
{
   TGo4ViewPanel* panel = (TGo4ViewPanel*) handle;

   return panel ? panel->GetDrawItemName(cnt) : nullptr;
}

void TGo4Script::StartFitPanel()
{
   fMainWin->StartFitPanel();
}

TGo4ServerProxy* TGo4Script::ConnectHttp(const char *servername, const char *account, const char *pass)
{
   if (!servername || (*servername == 0)) return nullptr;

   TGo4HttpProxy* proxy = new TGo4HttpProxy();
   if(account) proxy->SetAccount(account, pass);
   if (!proxy->Connect(servername)) {
      delete proxy;
      return nullptr;
   }

   const char *slotname = servername;
   if (strncmp(slotname,"http://",7) == 0) slotname+=7; else
   if (strncmp(slotname,"https://",8) == 0) slotname+=8;
   TString sname(slotname);
   Int_t len = sname.Index("/");
   if ((len>1) && (len<sname.Length())) sname.Resize(len);

   Browser()->AddServerProxy(proxy, sname.Data(), "ROOT http server");

   return proxy;
}


// ***************************** Generation *********************


void TGo4Script::ProduceScript(const char *filename, TGo4MainWindow* main)
{
   TGo4AnalysisConfiguration *confgui = main->FindAnalysisConfiguration();

   TGo4AnalysisWindow *termgui = main->FindAnalysisWindow();

   TGo4BrowserProxy *br = main->Browser();

   TGo4AnalysisProxy *anal = br->FindAnalysis();

   // this is either HTTP or normal http proxy
   TGo4ServerProxy *serv = br->FindServer();

   // ignore server which is was not launched
   if (serv && (serv != anal) && (serv->IsAnalysisLaunched() <= 0))
      serv = nullptr;

   std::ofstream fs(filename);
   if (!fs.is_open()) return;

   fs << "// Automatically generated startup script" << std::endl;
   fs << "// Do not change it!" << std::endl << std::endl;

   fs << "go4->SetMainWindowState(4, \"" << main->saveState().toHex().data() << "\");" << std::endl;

   fs << "go4->SetMainWindowGeometry(4, \"" << main->saveGeometry().toHex().data() << "\");" << std::endl;

   ProduceLoadLibs(fs);

   TObjArray prlist;
   br->MakeFilesList(&prlist);
   for (Int_t n = 0; n <= prlist.GetLast(); n++) {
      TGo4DirProxy *pr = (TGo4DirProxy *)prlist.At(n);
      fs << "go4->OpenFile(\"" << pr->GetFileName() << "\");" << std::endl;
   }

   prlist.Clear();
   br->MakeHServerList(&prlist);
   for (Int_t n = 0; n <= prlist.GetLast(); n++) {
      TGo4HServProxy *pr = (TGo4HServProxy *)prlist.At(n);
      fs << "go4->ConnectHServer(\""
         << pr->GetServerName() << "\", "
         << pr->GetPortNumber() << ", \""
         << pr->GetBaseName() << "\", \""
         << pr->GetUserPass() << "\", \""
         << pr->GetFilter() << "\");" << std::endl;
   }

   prlist.Clear();
   br->MakeDabcList(&prlist);
   for (Int_t n = 0; n <= prlist.GetLast(); n++) {
      TGo4DabcProxy *pr = (TGo4DabcProxy *)prlist.At(n);
      fs << "go4->ConnectDabc(\"" << pr->GetServerName() << "\");" << std::endl;
   }

   fs << std::endl;

   bool isanalysis = anal && anal->IsAnalysisReady() && !anal->IsAnalysisServer();
   if (!isanalysis && serv && (serv != anal)) isanalysis = true;

   // start analysis configuration
   if (isanalysis) {
   fs << "go4->LaunchAnalysis(\"" << go4sett->getClientName().toLatin1().constData() << "\", \""
                                  << go4sett->getClientDir().toLatin1().constData() <<  "\", \""
                                  << go4sett->getClientExec().toLatin1().constData() << "\", \""
                                  << go4sett->getClientNode().toLatin1().constData();

   if (go4sett->getClientIsServer() == 2) {
      // add port number for http server
      fs << ":";
      fs << go4sett->getClientPort();
   }

   fs << "\", ";

   if (go4sett->getClientShellMode() == Go4_rsh) fs << "Go4_rsh, "; else
   if (go4sett->getClientShellMode() == Go4_ssh) fs << "Go4_ssh, "; else fs << "Go4_sh, ";

   if (go4sett->getClientTermMode() == Go4_xterm) fs << "Go4_xtrem, "; else
   if (go4sett->getClientTermMode() == Go4_konsole) fs << "Go4_konsole, "; else fs << "Go4_qt, ";

   if (go4sett->getClientExeMode() == Go4_lib) fs << "Go4_lib"; else fs << "Go4_exe";

//                                  << go4sett->getClientShellMode() << ", "
//                                  << go4sett->getClientTermMode() <<  ", "
//                                  << go4sett->getClientExeMode();
   if (go4sett->getClientArgs().length()>0)
      fs << ", \"" << go4sett->getClientArgs().toLatin1().constData() << "\"";
   fs  << ");" << std::endl;

   fs << "go4->WaitAnalysis(300);" << std::endl << std::endl;

   if (!confgui) return;
   fs << "// configuration of analysis" << std::endl;

   QString fname;
   int interval, compression;
   bool asenabled, asoverwrite;

   confgui->GetAutoSaveConfig(fname, interval, compression, asenabled, asoverwrite);
   fs << "go4->AnalysisAutoSave(\"" << fname.toLatin1().constData() << "\", "
                                    << interval << ", "
                                    << compression << ", "
                                    << (asenabled ? "kTRUE" : "kFALSE") << ", "
                                    << (asoverwrite ? "kTRUE" : "kFALSE") << ");" << std::endl;

   confgui->GetAnalysisConfigFile(fname);
   fs << "go4->AnalysisConfigName(\"" << fname.toLatin1().constData() << "\");" << std::endl << std::endl;

   for (int nstep = 0; nstep < confgui->GetNumSteps(); nstep++) {
      TGo4ConfigStep *stepconf = confgui->GetStepConfig(nstep);
      fs << "// step " << stepconf->GetStepName().toLatin1().constData() << std::endl;

      bool process, source, store;
      stepconf->GetStepControl(process, source, store);
      fs << "go4->ConfigStep(\"" << stepconf->GetStepName().toLatin1().constData() << "\", "
                                 << (process ? "kTRUE" : "kFALSE") << ", "
                                 << (source ? "kTRUE" : "kFALSE") << ", "
                                 << (store ? "kTRUE" : "kFALSE") << ");" << std::endl;

      QString srcname;
      int timeout = 0, start = 0, stop = 0, interval = 0, nport = 0, nretry = 0;
      int nsrc = stepconf->GetSourceSetup(srcname, timeout, start, stop, interval, nport, nretry);

      TString srcargs;
      srcargs.Form("(\"%s\", \"%s\", %d", stepconf->GetStepName().toLatin1().constData(), srcname.toLatin1().constData(), timeout);

      switch(nsrc) {
         case 0:
           fs << "go4->StepFileSource" << srcargs;
           break;
         case 1: {
           QString TagFile;
           stepconf->GetMbsFileSource(TagFile);
           fs << "go4->StepMbsFileSource" << srcargs << ", \""
                                          << TagFile.toLatin1().constData() << "\"";
           break;
         }
         case 2:
           fs << "go4->StepMbsStreamSource" << srcargs;
           break;
         case 3:
           fs << "go4->StepMbsTransportSource" << srcargs;
           break;
         case 4:
           fs << "go4->StepMbsEventServerSource" << srcargs;
           break;
         case 5:
            fs << "go4->StepMbsRevServSource" << srcargs;
            break;
         case 6:
            fs << "go4->StepRandomSource" << srcargs;
            break;
         case 7: {
           int port;
           QString expr;
           stepconf->GetUserSource(port, expr);
           fs << "go4->StepUserSource" << srcargs << ", " << port << ", \""
                                       << expr.toLatin1().constData() << "\"";
           break;
         }
         case 8:
           fs << "go4->StepHDF5Source" << srcargs;
           break;


      } //  switch(nsrc)
      fs << ");" << std::endl;

      if ((start != 0) || (stop != 0) || (interval > 1)) {
         srcargs.Form("(\"%s\", %d, %d ,%d)",stepconf->GetStepName().toLatin1().constData(), start, stop, interval);
         fs << "go4->StepMbsSelection" << srcargs << ";" << std::endl;
      }

      if (nport>0) {
         srcargs.Form("(\"%s\", %d)",stepconf->GetStepName().toLatin1().constData(), nport);
         fs << "go4->StepMbsPort" << srcargs << ";" << std::endl;
      }

      if (nretry>0) {
         srcargs.Form("(\"%s\", %d)",stepconf->GetStepName().toLatin1().constData(), nretry);
         fs << "go4->StepMbsRetryCnt" << srcargs << ";" << std::endl;
      }

      QString storename;
      int nstore = stepconf->GetStoreSetup(storename);
      switch(nstore) {
         case 0: {
            bool overwrite;
            int bufsize, splitlevel, compression, autosave;
            stepconf->GetFileStore(overwrite, bufsize, splitlevel, compression, autosave);
            fs << "go4->StepFileStore(\"" << stepconf->GetStepName().toLatin1().constData() << "\", \""
                                          << storename.toLatin1().constData() << "\", "
                                          << (overwrite ? "kTRUE" : "kFALSE") << ", "
                                          << bufsize << ", "
                                          << splitlevel << ", "
                                          << compression << ", "
                                          << autosave << ");" << std::endl;
            break;
         }

         case 1: {
            int bufsize, splitlevel;
            stepconf->GetBackStore(bufsize, splitlevel);
            fs << "go4->StepBackStore(\"" << stepconf->GetStepName().toLatin1().constData() << "\", \""
                                          << storename.toLatin1().constData() << "\", "
                                          << bufsize << ", "
                                          << splitlevel << ");" << std::endl;
            break;
         }

         case 2: {
            fs << "go4->StepUserStore(\"" << stepconf->GetStepName().toLatin1().constData() << "\", \""
                                          << storename.toLatin1().constData() << "\");" << std::endl;
            break;
         }

         case 3: {
           int flags = 0;
           stepconf->GetHDF5Store(flags);
           fs << "go4->StepHDF5Store(\"" << stepconf->GetStepName().toLatin1().constData() << "\", \""
                                         << storename.toLatin1().constData() << "\", "
                                         << flags << ");" << std::endl;
           break;
         }

      } // switch

      fs << std::endl;
   }

   if ((anal && anal->IsAnalysisSettingsReady()) || (serv && (serv != anal)))
      fs << "go4->SubmitAnalysisConfig(20);" << std::endl << std::endl;

   int mode = 1;
   QWidget* mdi = confgui ? confgui->parentWidget() : nullptr;
   if (mdi) {
      if (mdi->isHidden()) mode = -1; else
      if (mdi->isMinimized()) mode = 0;
   }
   fs << "go4->SetAnalysisConfigMode(" << mode << ");" << std::endl;

   mode = 1;
   mdi = termgui ? termgui->parentWidget() : nullptr;
   if (mdi) {
      if (mdi->isHidden()) mode = -1; else
      if (mdi->isMinimized()) mode = 0;
   }
   fs << "go4->SetAnalysisTerminalMode(" << mode << ");" << std::endl << std::endl;
// end analysis configuration
   } else
   if (anal && anal->IsAnalysisReady() && anal->IsAnalysisServer()) {
      fs << "go4->ConnectAnalysis(\""
         << go4sett->getClientNode().toLatin1().constData() << "\", "
         << go4sett->getClientPort() << ", "
         << go4sett->getClientControllerMode() << ", ";
      if (go4sett->getClientDefaultPass())
        fs << "0);" << std::endl;
      else
        fs << "\"" << main->LastTypedPassword() << "\");" << std::endl;
      fs << "go4->WaitAnalysis(10);" << std::endl << std::endl;
   } else {
      fs << "go4->DisconnectAnalysis();" << std::endl;
   }

   // connect to other http server after main analysis configuration - otherwise they could replace it in control elements
   prlist.Clear();
   br->MakeHttpList(&prlist);
   for (Int_t n = 0; n <= prlist.GetLast(); n++) {
      TGo4HttpProxy *pr = (TGo4HttpProxy *)prlist.At(n);
      if (pr == serv) continue;
      fs << "go4->ConnectHttp(\"" << pr->GetServerName() << "\");" << std::endl;
   }

   if ((anal && anal->IsAnalysisRunning() && !anal->IsAnalysisServer()) ||
       (serv && (serv!=anal) && serv->IsAnalysisRunning())) {
      fs << "go4->StartAnalysis();" << std::endl;
      fs << std::endl;
      fs << "// in some analysis one requires to wait several seconds before new histograms appears" << std::endl;
      fs << "// go4->Wait(1);" << std::endl;
      fs << "go4->RefreshNamesList();" << std::endl;
   } else
   if (anal || serv) {
      fs << std::endl;
      fs << "go4->RefreshNamesList();" << std::endl;
   }

   int npanel = 0;

   QList<QMdiSubWindow *> windows = TGo4MdiArea::Instance()->subWindowList();
   for (int i = 0; i < windows.count(); ++i) {
      TGo4ViewPanel *panel = dynamic_cast<TGo4ViewPanel *>(windows.at(i)->widget());
      if (!panel) continue;

      TString picname = TString::Format("pic%d", ++npanel);
      TGo4Picture pic(picname.Data(), DefaultPicTitle());

      panel->MakePictureForPad(&pic, panel->GetCanvas(), true);

      fs << std::endl;

      pic.SavePrimitive(fs,"");

      QWidget* mdi = panel->parentWidget();

      QPoint pos = mdi->pos();
      mdi->mapToParent(pos);
      QSize size = mdi->size();

      const char *mode = "Go4_normal";
      if (mdi->isHidden()) mode = "Go4_hidden"; else
      if (mdi->isMinimized()) mode = "Go4_minimized"; else
      if (mdi->isMaximized()) mode = "Go4_maximized";

      fs << "go4->StartViewPanel("
         << pos.x() << ", "
         << pos.y() << ", "
         << size.width() << ", "
         << size.height() << ", "
         << mode << ", " << picname << ");" << std::endl;

      fs << "delete " << picname << ";" << std::endl;
   }

   fs << std::endl;

   TGo4Iter iter(br->BrowserTopSlot(), kFALSE);
   Bool_t goinside = kTRUE;

   while (iter.next(goinside)) {
      TGo4Slot* subslot = iter.getslot();
      goinside = kTRUE;

      if (br->ItemKind(subslot)==TGo4Access::kndFolder) {
         // check if any subitem not monitored
         Int_t ncheck = 0, ncount = 0;
         TGo4Iter subiter(subslot, kTRUE);
         while (subiter.next()) {
            TGo4Slot* subsubslot = subiter.getslot();
            if (br->ItemKind(subsubslot)!=TGo4Access::kndObject) continue;
            ncount++;
            if (!br->IsItemMonitored(subsubslot)) ncheck++;
                                          else break;
         }
         goinside = (ncount == 0) || (ncheck != ncount);
      } else
         goinside = br->IsItemMonitored(subslot);

      if (!goinside) {
         TString sbuf;
         if (br->BrowserItemName(subslot, sbuf))
           fs << "go4->MonitorItem(\"" << sbuf << "\", kFALSE);" << std::endl;
      }

   }

   Int_t mperiod = br->MonitoringPeriod();
   if (mperiod > 0)
     fs << "go4->StartMonitoring(" << mperiod/1000 << ");" << std::endl;
}
