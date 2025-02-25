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

/************************************************************************
* This example shows how to use the TGo4ObjectClient class
* to receive any registered root object from a running Go4 analysis.
* The instance of TGo4ObjectClient stores all information required to
* connect, like hostname, port,  name of the database, password.
* This information is set in the class constructor, but can be changed
* with methods
*       SetHost(const char*), SetPort(Int_t)
*       SetBase(const char*), SetPasswd(const char*)
* without destroying the client object.
* The connection to Go4 object server is established anew at any
* time an object or the list of names is requested from the server,
* and disconnected again after the object is received.
* Method
*      TGo4AnalysisObjectNames* TGo4ObjectClient::RequestNamesList()
* requests the go4 nameslist object from the specified object server
+ and delivers pointer to the object when received. Nameslist object
* has to be deleted by the user of the client object afterwards.
* Returns 0 in case of error.
* Method
*      TObject* TGo4ObjectClient::RequestObject(const char*)
* requests a root object by name from the specified object server
+ and delivers pointer to the object when received. Object
* has to be deleted by the user of the client afterwards.
* Returns 0 in case of error or object not found. Name string may
* contain the full path to the object in the go4 folder structure
*********************
* The example expects the client parameters host,port,base,passwd,
* and a (in this case) dummy command string.
* as command line parameters. Then a command input loop starts.
* Typing the object name will request object and draw it on the
* canvas, if histogram. Other objects are just printed.
* Typing "dir" will request and printout the list of objects
* available on the server.
* Typing "exit" will finish the object client.
**********************************************************************/

#include <iostream>

#include "TROOT.h"
#include "TApplication.h"
#include "TCanvas.h"

#include "TGo4Log.h"
#include "TGo4ObjClient.h"
#include "TGo4AnalysisObjectNames.h"

int main(int argc, char **argv)
{
   if (argc < 6) {
      std::cout << "usage: MainGo4ObjectClient hostname port base passwd command "<<std::endl;
      return 0;
   }

   TApplication theApp("App", 0, nullptr);
   TCanvas *mycanvas = nullptr;
   TGo4Log::Instance(); // init logger object
   TGo4Log::SetIgnoreLevel(0); // set this to 1 to suppress detailed debug output
                               // set this to 2 to get warnings and errors only
                               // set this to 3 to get errors only
   TGo4Log::LogfileEnable(kFALSE); // will enable or disable logging all messages to file

   const char *hostname = argv[1];
   const char *connector = argv[2];
   Int_t port = atoi(connector);
   const char *base = argv[3];
   const char *pass = argv[4];
   const char *comm = argv[5];
   std::cout << "Host:" << hostname << ",\tPort:" << port << ",\tBase:" << base << ",\tPass:" << pass
             << ",\tComm:" << comm << std::endl;
   TGo4ObjClient myclient("TestClient", base, pass, hostname, port);
   // testing repeated access:

   std::string inputline;
   while (1) {
      std::cout << "Go4> Object client command:" << std::endl;
      getline(std::cin, inputline);
      comm = inputline.c_str();

      if (!strcmp(comm, "exit")) {
         std::cout << "exit command..." << std::endl;
         gApplication->Terminate();
      } else if (!strcmp(comm, "dir")) {
         std::cout << "getting nameslist" << std::endl;
         TGo4AnalysisObjectNames *list = myclient.RequestNamesList();

         if (list)
            list->Print();
         else
            std::cout << "got zero list!!!" << std::endl;
      } else {
         std::cout << "getting object " << comm << std::endl;
         TObject *ob = myclient.RequestObject(comm);
         if (ob) {
            ob->Print();

            if (ob->InheritsFrom("TH1")) {
               if (!mycanvas) mycanvas = gROOT->MakeDefCanvas();

               mycanvas->cd();
               ob->Draw();
               mycanvas->Modified();
               mycanvas->Update();
            }

         } else {
            std::cout << "got zero object!!!" << std::endl;
         }
      } // if(!strcmp(comm,"dir"))
   }    // while(inputline...
   theApp.Run();
   return 0;
}
