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

#ifndef TGO4OBJECTMANAGER_H
#define TGO4OBJECTMANAGER_H

#include "TGo4Slot.h"
#include "TGo4AccessWrapper.h"

#include "TObjArray.h"

class TFile;
class TDirectory;
class TTree;
class TFolder;
class TGo4Access;

class TGo4ObjectManager : public TGo4Slot, public TGo4AccessWrapper {
   friend class TGo4Access;

   public:
      TGo4ObjectManager();

      TGo4ObjectManager(const char *name, const char *title);

      virtual ~TGo4ObjectManager();

      void ProduceFullName(TString& name, TGo4Slot* toparent = nullptr) override;
      TGo4ObjectManager* GetOM() const override;

      void MakeFolder(const char *pathname);

      TGo4Slot* Add(const char *pathname, TObject *obj, Bool_t owner = kFALSE, Bool_t canrename = kFALSE);

      TGo4Slot* MakeObjSlot(const char *foldername, const char *name = nullptr, const char *title = nullptr);

      TGo4Slot* AddLink(TGo4Slot* source, const char *pathname, const char *linkname, const char *linktitle);

      TGo4Slot* AddLink(TGo4Slot* source, const char *pathname);

      TGo4Slot* AddLink(const char *sourcename, const char *pathname);

      TGo4Slot* GetLinked(TGo4Slot* link);

      void AddFile(const char *pathname, const char *filename);

      void CloseFiles(const char *pathname);

      void AddDir(const char *pathname, TDirectory* dir, Bool_t owner = kFALSE, Bool_t readright = kFALSE);

      void AddTree(const char *pathname, TTree* tree, Bool_t owner = kFALSE);

      void AddFolder(const char *pathname, TFolder* folder, Bool_t owner = kFALSE);

      void AddROOTFolder(const char *pathname, const char *foldername);

      void AddROOTFolders(const char *pathname, Bool_t selected = kTRUE);

      void AddProxy(const char *pathname, TGo4Proxy* cont, const char *name, const char *title = "title");
      TGo4Proxy* GetProxy(const char *name);

      void DeleteSlot(const char *pathname);

      void SaveDataToFile(TFile* f, Bool_t onlyobjs = kFALSE, TGo4Slot* startslot = nullptr);

      void ReadDataFromFile(TFile* f);

      void RegisterLink(TGo4Slot* source, TGo4Slot* target, Bool_t exapndchilds = kFALSE);
      void UnregisterLink(TGo4Slot* target);

      TGo4Access* ProvideAccess(const char *name = nullptr) override
          { return ProvideSlotAccess(name); }

      void Event(TGo4Slot* source, Int_t id, void* param = nullptr) override;
      void RetranslateEvent(TGo4Slot* source, Int_t id, void* param = nullptr);
      void RegisterObjectWith(TObject *obj, TGo4Slot* slot);
      void UnregisterObject(TObject *obj, TGo4Slot* slot);

      void PrintSlots();

      Int_t IterateSlots();

      Int_t RequestObject(const char *source, const char *targetslot, Int_t waittime_millisec = 0);

      void RecursiveRemove(TObject *obj) override;

   protected:

      virtual Bool_t AssignObject(const char *path, TObject *obj, Bool_t ownership);

      void RemoveFromLinks(const TGo4Slot* slot);

      TObjArray  fLinks;     //! list of links between slots

      TObjArray  fCleanups;  //! list of registered cleanups

   ClassDefOverride(TGo4ObjectManager,1);  // Object manager of all Go4 GUI objects
};

#endif
