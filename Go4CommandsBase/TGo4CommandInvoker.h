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

#ifndef TGO4COMMANDINVOKER_H
#define TGO4COMMANDINVOKER_H

/** @author M. Hemberger (2000),
  * J. Adamczewski (modified Apr/Jun-2005)
  * S.Linev (modified Jun-2005)
  */

#include "TObject.h"
#include "TGo4CommandReceiver.h"

class TMutex;
class TObjArray;
class TGo4Command;
class TGo4RemoteCommand;
class TGo4CommandProtoList;

/**
 * Class that knows Command Object; When Client gets the Command it is passed to the Invoker who
 * sets the CommandClient as receiver and calls its Execute method.
 * Decouples CommandClient and Command object circular dependencies
 */

class TGo4CommandInvoker : public TObject, public TGo4CommandReceiver {

   public:

      static TGo4CommandInvoker * Instance();

      static void Register(const char *name, TGo4CommandReceiver *p);

      static void UnRegister(TGo4CommandReceiver* p);

      virtual void Invoke(TGo4Command * com);

      virtual void Invoke();

      virtual ~TGo4CommandInvoker();

      /** Application may exchange standard command list by
        * own subclass with specialized commands, e.g. for analysis control */
      static void SetCommandList(TGo4CommandProtoList* list);

      /** Create real command from remote command and invoke it.
        * Used when remote command acts on this invoker as receiver */
      Int_t ExecuteFromRemote(TGo4RemoteCommand* remcom);

   protected:

      TGo4CommandInvoker();

      static TGo4CommandReceiver * Lookup(const char *name);

   private:

      /** @link aggregation */
      TGo4Command * fxCommand{nullptr};       //!

      /** For remote commands, invoker will create the requested
       * slave commands from own command list */
      static TGo4CommandProtoList* fxCommandList;  //!

      static TGo4CommandInvoker *fxInstance;  //!
      static TObjArray * fxArray;             //!
      static TMutex * fxMutex;                //!

   ClassDefOverride(TGo4CommandInvoker, 2)
};

#endif //TGO4COMMANDINVOKER_H
