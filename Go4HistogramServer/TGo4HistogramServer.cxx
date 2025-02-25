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

#include "TGo4HistogramServer.h"

#include <iostream>
#include "TFile.h"
#include "TBufferFile.h"

#include "TGo4Log.h"
#include "TGo4LockGuard.h"
#include "TGo4Socket.h"

#include "TGo4HisConnectorRunnable.h"
#include "TGo4ObjConnectorRunnable.h"
#include "TGo4ObjConnectorTimer.h"
#include "TGo4CommandInvoker.h"
#include "TGo4AnalysisClientImp.h"
#include "TGo4AnalysisImp.h"
#include "TGo4AnalysisObjectNames.h"
#include "TGo4Task.h"
#include "TGo4Thread.h"
#include "TGo4ThreadManager.h"
#include "TGo4ThreadHandler.h"
#include "TGo4TerminateException.h"
#include "TGo4TaskHandler.h"

extern "C" {
   #include "f_his_hist.h"
}

const char *TGo4HistogramServer::fgcCONTHREADNAME = "HISTOSERV-";
const char *TGo4HistogramServer::fgcSHUTDOWNNAME = "__HServLast__";
const char *TGo4HistogramServer::fgcCOMGETLIST = "__OServNamesList__";

const char *TGo4HistogramServer::fgcOBJTHREADNAME = "OBJECTSERV-";
const UInt_t TGo4HistogramServer::fguTIMERPERIOD = 200;          // time in ms (20)
const Int_t TGo4HistogramServer::fgiOPENWAITCYCLES = 100;        // wait cycles (100)
const UInt_t TGo4HistogramServer::fguOPENWAITCYCLETIME = 500;    // time in ms (20)
const Int_t TGo4HistogramServer::fgiCLOSEWAITCYCLES = 100;       // wait cycles (100)
const UInt_t TGo4HistogramServer::fguCLOSEWAITCYCLETIME = 500;   // time in ms (20)
const Int_t TGo4HistogramServer::fgiCONNECTWAITCYCLES = 20;      // wait cycles (20)
const UInt_t TGo4HistogramServer::fguCONNECTWAITCYCLETIME = 500; // time in ms (20)

TGo4HistogramServer::TGo4HistogramServer(TGo4AnalysisClient *owner, const char *servername, const char *password,
                                         Bool_t useobjectserver)
   : fxAnalysisClient(owner), fxThreadHandler(nullptr), fiServerPort(0), fxTransport(nullptr), fuObjectPort(0),
     fxConnectTransport(nullptr), fxDisConnectTransport(nullptr), fuConnectPort(0), fbConnectRequest(kFALSE),
     fbDisConnectRequest(kFALSE), fbConnectIsOpen(kFALSE), fbConnectIsDone(kFALSE), fbConnectIsClose(kTRUE),
     fxConnectorTimer(nullptr), fbUseObjectServer(useobjectserver)
{
   TGo4CommandInvoker::Instance(); // make sure we have an invoker instance!
   TGo4CommandInvoker::Register("HistogramServer", this);
   if (fbUseObjectServer)
      fxTransport = new TGo4Socket(kFALSE); // raw transport in server mode for object server
   // we use the histogram api instead of stccomm:
   Int_t result = f_his_server((CHARS *)servername, (CHARS *)password, &fiServerPort);
   if (result == COMM__SUCCESS) {
      fxServerName = servername;
      fxServerPass = password;
      TGo4Analysis::Instance()->Message(1, "Created Histogram server %s on port %d", servername, fiServerPort);
   } else {
      // std::cout <<"ERROR on creation of Histogram server: "<< result << std::endl;
      TGo4Analysis::Instance()->Message(3, "ERROR %d on creation of Histogram server", result);
   }
   // start connector thread:
   const char *ownername;
   if (fxAnalysisClient) {
      fxThreadHandler = fxAnalysisClient->GetThreadHandler();
      fxAnalysis = fxAnalysisClient->GetAnalysis();
      ownername = fxAnalysisClient->GetName();
   } else {
      ownername = "never-come-here";
   }

   if (fbUseObjectServer) {
      fxConnectorTimer = new TGo4ObjConnectorTimer(this, fguTIMERPERIOD);
      fxConnectorTimer->TurnOn();
   }
   if (fxThreadHandler) {
      // thread to serve gsi histogram api:
      fxConnectorName = fgcCONTHREADNAME;
      fxConnectorName += ownername;

      TGo4HisConnectorRunnable *crun = new TGo4HisConnectorRunnable(ConnectorName(), this);
      fxThreadHandler->NewThread(ConnectorName(), crun);
      fxThreadHandler->Start(ConnectorName());
      if (fbUseObjectServer) {

         // thread for root object server:
         fxObjectThreadName = fgcOBJTHREADNAME;
         fxObjectThreadName += ownername;

         TGo4ObjConnectorRunnable *orun = new TGo4ObjConnectorRunnable(ObjectThreadName(), this);
         fxThreadHandler->NewThread(ObjectThreadName(), orun);
         fxThreadHandler->Start(ObjectThreadName());
      }
   } else {
      std::cerr << " Histogram Server constructor FATAL ERROR: no threadmanager !!" << std::endl;
      throw TGo4RuntimeException();
   }
}

TGo4HistogramServer::TGo4HistogramServer()
   : fxAnalysisClient(nullptr), fxThreadHandler(nullptr), fiServerPort(0), fxTransport(nullptr), fuObjectPort(0), fxConnectTransport(nullptr),
     fxDisConnectTransport(nullptr), fcConnectHost(0), fuConnectPort(0), fbConnectRequest(kFALSE),
     fbDisConnectRequest(kFALSE), fbConnectIsOpen(kFALSE), fbConnectIsDone(kFALSE), fbConnectIsClose(kTRUE),
     fbUseObjectServer(kFALSE)
{
   TGo4CommandInvoker::Instance(); // make sure we have an invoker instance!
   TGo4CommandInvoker::Register("HistogramServer", this);
}

TGo4HistogramServer::~TGo4HistogramServer()
{
   // prepare stopped flag:
   if (fbUseObjectServer && fxThreadHandler)
      fxThreadHandler->Stop(ConnectorName());

   // for clean shutdown, we have to connect one last time to get out of the wait:
   INTS4 *pl_all = nullptr;
   s_his_head *ps_his_head = nullptr;
   INTS4 l_size = 0;
   f_his_gethis((char *)"localhost", fiServerPort, (char *)fxServerName.Data(), (char *)fxServerPass.Data(),
                (char *)fgcSHUTDOWNNAME, (s_his_head **)&ps_his_head, &pl_all, &l_size);

   if (fxThreadHandler) {
      fxThreadHandler->RemoveThread(ConnectorName());
      //// shutdown of object server thread:
      if (fbUseObjectServer) {
         fxThreadHandler->Stop(ObjectThreadName());
         // here we might connect last time to object server, to investigate!!
         /// remove and delete the objectserver thread:
         fxThreadHandler->RemoveThread(ObjectThreadName());
         // this will cancel thread and delete runnable
      }
   }
   delete fxConnectorTimer;
   f_his_close(); // this will delete api server for histograms

   if (fxTransport) {
      fxTransport->Close(); // close go4 server socket for object server
      delete fxTransport;
      fxTransport = nullptr;
   }

   TGo4CommandInvoker::UnRegister(this);
}

Int_t TGo4HistogramServer::ServeObjectClient()
{
   Int_t rev = ConnectObjectClient();
   if (rev < 0)
      return rev;
   if (rev == 0 && CheckLogin()) {
      HandleObjectRequest();
   } else {
   }
   DisconnectObjectClient();
   return rev;
}

void TGo4HistogramServer::DisconnectObjectClient()
{
   //
   SetDisConnect(fxTransport); // timer shall do the Close() of negotiation
   // TGo4Log::Debug(" HistogramServer: Waiting for timer Close() of client  ... ");
   WaitForClose(); // poll until timer has returned from close
   // TGo4Log::Debug(" HistogramServer: Client connection closed. ");
}

Int_t TGo4HistogramServer::ConnectObjectClient()
{

   static Bool_t isfirsttime = kTRUE;
   // we delegate the actual TSocket open to the taskconnector timer:
   SetConnect(fxTransport, "Server mode does not need hostname", 0); // for portscan
   Int_t waitresult = WaitForOpen();                                 // wait for the server Open() call by timer
   if (waitresult == -2)
      return waitresult; // return in termination mode
   if (waitresult < 0) {
      // open timeout
      TGo4Log::Debug(" HistogramServer: Negotiation channel open TIMEOUT");
      std::cerr << " HistogramServer TIMEOUT ERROR opening socket connection !!! Terminating..." << std::endl;
      throw TGo4TerminateException(fxAnalysisClient->GetTask());
   }
   Int_t count = 0;
   while (GetObjPort() == 0) {
      TGo4Task *task = fxAnalysisClient->GetTask();
      if (count > TGo4TaskHandler::Get_fgiPORTWAITCYCLES()) {
         TGo4Log::Debug(" HistogramServer: Negotiation port getter TIMEOUT");
         std::cerr << " HistogramServer TIMEOUT ERROR retrieving port number  !!! Terminating..." << std::endl;
         throw TGo4TerminateException(fxAnalysisClient->GetTask());
      } else if (!task || task->IsTerminating()) {
         return -1;
      } else {
         TGo4Thread::Sleep(TGo4TaskHandler::Get_fguPORTWAITTIME());
         ++count;
      }
   }
   //   TGo4Log::Debug(" HistogramServer is waiting to serve object client request on port %d ... ",
   //            fuObjectPort);
   if (isfirsttime) {
      // only tell gui the first time the port number; performance!
      TGo4Analysis::Instance()->Message(1, "Object Server %s is waiting on port %d", fxServerName.Data(), fuObjectPort);
      isfirsttime = kFALSE;
   } else {
   }
   Int_t connectwaitseconds = WaitForConnection(); // timer tells us by flag when the transport is opened
   if (connectwaitseconds < 0) {
      // case of threadmanager termination:
      // connector runnable shall stop on return from ServeClient method
      return connectwaitseconds;
   } else {
      // just proceed to the client server negotiations
   }

   return 0;
}

Bool_t TGo4HistogramServer::CheckLogin()
{
   ///////// check connected client:
   // check for basename:
   // std::cout <<"##### check login " << std::endl;
   const char *recvchar = fxTransport->RecvRaw("dummy");
   if (recvchar && !strcmp(recvchar, fxServerName.Data())) {
      fxTransport->Send(TGo4TaskHandler::Get_fgcOK()); // handshake to assure the client
   } else {
      // TGo4Analysis::Instance()->Message(2,
      //       "Object server connection attempt with wrong basename");
      std::cerr << "##### check login with wrong base" << std::endl;
      fxTransport->Send(TGo4TaskHandler::Get_fgcERROR());
      DisconnectObjectClient();
      return kFALSE;
   }
   // check for password:
   recvchar = fxTransport->RecvRaw("dummy");
   if (recvchar && !strcmp(recvchar, fxServerPass.Data())) {
      fxTransport->Send(TGo4TaskHandler::Get_fgcOK()); // handshake to assure the client
   } else {
      //         TGo4Analysis::Instance()->Message(2,
      //                "Object server connection attempt with wrong password");
      std::cerr << "##### check login with wrong passwd" << std::endl;
      fxTransport->Send(TGo4TaskHandler::Get_fgcERROR());
      DisconnectObjectClient();
      return kFALSE;
   }
   return kTRUE;
}

Bool_t TGo4HistogramServer::HandleObjectRequest()
{
   char objectname[TGo4ThreadManager::fguTEXTLENGTH];
   // get object name
   char *recvchar = fxTransport->RecvRaw("dummy");
   if (!recvchar) {
      std::cerr << "-----Object server received null character for object request!" << std::endl;
      return kFALSE;
   }
   strncpy(objectname, recvchar, TGo4ThreadManager::fguTEXTLENGTH - 1); // get the client name
   // check here if object is requested or nameslist? :
   TObject *object = nullptr;
   if (!strcmp(objectname, fgcCOMGETLIST)) {
      // get analysis nameslist object
      TGo4LockGuard mainguard; // protect creation of new nameslist
      fxAnalysis->UpdateNamesList();
      object = fxAnalysis->GetNamesList();
   } else {
      // get object from analysis
      object = fxAnalysis->GetObject(objectname);
   }
   return SendObject(object);
}

Bool_t TGo4HistogramServer::SendObject(TObject *object)
{
   Bool_t retval = kTRUE;
   // stream object into TBuffer:
   TBuffer *rootbuffer = nullptr;
   if (object) {
      fxTransport->Send(TGo4TaskHandler::Get_fgcOK()); // let client know the object exists
      TGo4LockGuard mainguard;
      rootbuffer = new TBufferFile(TBuffer::kWrite);
      TFile *filsav = gFile;
      gFile = nullptr;
      rootbuffer->WriteObject(object);
      gFile = filsav;
      fxTransport->SendBuffer(rootbuffer);
      delete rootbuffer;
   } else {
      // std::cout <<"Error: object not found in analysis!" << std::endl;
      fxTransport->Send(TGo4TaskHandler::Get_fgcERROR());
      retval = kFALSE;
   }
   char *recvchar = fxTransport->RecvRaw("dummy"); // get exit message
   if (!recvchar) {
      TGo4Log::Debug(" HistogramServer: null character on finishing object client channel ");
      retval = kFALSE;
   } else if (strcmp(recvchar, TGo4TaskHandler::Get_fgcOK())) {
      TGo4Log::Debug(" HistogramServer: ERROR on finishing object client channel ");
      retval = kFALSE;
   } else {
      // std::cout <<"##### send object is finished with ok." << std::endl;
   }
   return retval;
}

void TGo4HistogramServer::SetConnect(TGo4Socket *trans, const char *host, UInt_t port)
{
   GO4TRACE((12, "TGo4HistogramServer::SetConnect(TGo4Socket *)", __LINE__, __FILE__));
   fxConnectTransport = trans;
   fcConnectHost = host;
   fuConnectPort = port;
   fbConnectRequest = kTRUE;
}

void TGo4HistogramServer::SetDisConnect(TGo4Socket *trans)
{
   GO4TRACE((12, "TGo4HistogramServer::SetDisConnect(TGo4Socket *)", __LINE__, __FILE__));
   fxDisConnectTransport = trans;
   fbDisConnectRequest = kTRUE;
}

Int_t TGo4HistogramServer::TimerConnect()
{
   GO4TRACE((12, "TGo4HistogramServer::TimerConnect()", __LINE__, __FILE__));
   Int_t rev = 0;
   ///////////////////////////////////////
   //// handle the disconnection first:
   if (fbDisConnectRequest) {
      GO4TRACE((15, "TGo4HistogramServer::TimerConnect()--DisConnectRequest", __LINE__, __FILE__));
      if (fxDisConnectTransport) {
         // we have a transport instance to disconnect
         fxDisConnectTransport->Close();
         fbConnectIsClose = kTRUE;
         fbDisConnectRequest = kFALSE; // we served the request, reset it
         rev += 1;
      } else {
         // error, zero pointer given
         rev += 32;
      }
   } else //// if(fbDisConnectRequest)
   {
      GO4TRACE((15, "TGo4HistogramServer::TimerConnect()--NO DisConnectRequest", __LINE__, __FILE__));
      // no open request, continue
      rev += 2;
   }

   /////////////////////////////////////
   //// then handle the connection:
   if (fbConnectRequest) {
      GO4TRACE((15, "TGo4HistogramServer::TimerConnect()--ConnectRequest", __LINE__, __FILE__));
      // timer shall open a transport as server
      if (fxConnectTransport) {
         if (!fxConnectTransport->IsOpen()) {
            GO4TRACE((10, "TGo4HistogramServer::TimerConnect()--transport is not open", __LINE__, __FILE__));
            // transport is not open, so do it
            fbConnectIsOpen = kTRUE; // tell connector thread that we try to open
            // std::cout <<"TimerConnect: connect before open" << std::endl;
            Int_t result = fxConnectTransport->Open(ConnectHost(), fuConnectPort, kTRUE);
            if (result == 0) {
               fbConnectIsDone = kTRUE;   // tell connector thread we returned from open
               fbConnectRequest = kFALSE; // we served the request, reset it
               rev += 4;
            } else {
               rev = -4;
               // open was not finished, we poll once again...
            }
         } else {
            GO4TRACE((10, "TGo4HistogramServer::TimerConnect()--transport already open", __LINE__, __FILE__));
            // transport was already open, do nothing
            rev += 8;
         }
      }
      else {
         GO4TRACE((10, "TGo4HistogramServer::TimerConnect()--no transport specified", __LINE__, __FILE__));
         rev += 64;
      }
   } //// if(fbConnectRequest)
   else {
      GO4TRACE((15, "TGo4HistogramServer::TimerConnect()--NO ConnectRequest", __LINE__, __FILE__));
      // no open request, continue
      rev += 16;
   }
   //   std::cout <<"TimerConnect: before return" << std::endl;
   return rev;
}

Int_t TGo4HistogramServer::WaitForOpen()
{
   GO4TRACE((12, "TGo4HistogramServer::WaitForOpen()", __LINE__, __FILE__));
   Int_t count = 0;
   while (!fbConnectIsOpen) {
      TGo4Task *task = fxAnalysisClient->GetTask();
      if (count > TGo4HistogramServer::fgiOPENWAITCYCLES) {
         count = -1; // timeout
         break;
      } else if (!task || task->IsTerminating()) {
         // note: task == nullptr is case of shutdown of analysis server!
         count = -2;
         break;
      } else {
         TGo4Thread::Sleep(TGo4HistogramServer::fguOPENWAITCYCLETIME);
         ++count;
      }
      // std::cout << "*****WaitForOpen()"<< std::endl;
   }
   fbConnectIsOpen = kFALSE; //  reset for next time
   return count;
}

Int_t TGo4HistogramServer::WaitForClose()
{
   GO4TRACE((12, "TGo4HistogramServer::WaitForClose()", __LINE__, __FILE__));
   Int_t count = 0;
   while (!fbConnectIsClose) {
      // std::cout <<"Waiting for close..." << std::endl;
      if (count > TGo4HistogramServer::fgiCLOSEWAITCYCLES) {
         // std::cout <<"reached closewaitcycles "<< count << std::endl;
         count = -1; // timeout
         break;
      } else {
         TGo4Thread::Sleep(TGo4HistogramServer::fguCLOSEWAITCYCLETIME);
         ++count;
      }
      // std::cout << "*****WaitForClose() "<<count<< std::endl;
   }
   fbConnectIsClose = kFALSE; //  reset for next time
   return count;
}

Int_t TGo4HistogramServer::WaitForConnection()
{
   GO4TRACE((12, "TGo4HistogramServer::WaitForConnection()", __LINE__, __FILE__));
   Int_t count = 0;
   while (!fbConnectIsDone) {
      TGo4Task *task = fxAnalysisClient->GetTask();
      if (!task || task->IsTerminating()) {
         // note: task == nullptr is case of shutdown of analysis server!
         count = -2; // termination mode
         break;
      } else {
         TGo4Thread::Sleep(TGo4HistogramServer::fguCONNECTWAITCYCLETIME);
         ++count;
      }
      // std::cout << "*****WaitForConnection()"<< std::endl;
   }
   fbConnectIsDone = kFALSE; //  reset for next time
   return count;
}

UInt_t TGo4HistogramServer::GetObjPort()
{
   if (fxTransport) {
      fuObjectPort = fxTransport->GetPort();
   }
   return fuObjectPort;
}
