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

#include "TGo4CompositeEvent.h"

#include "TTree.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TDirectory.h"
#include "TBranch.h"

#include "TGo4Log.h"

TGo4CompositeEvent::TGo4CompositeEvent() :
   TGo4EventElement(),
   fNElements(0),
   fEventElements(nullptr),
   fMaxIndex(0)
{
}

TGo4CompositeEvent::TGo4CompositeEvent(const char*aName, const char *aTitle, Short_t aBaseCat) :
   TGo4EventElement(aName,aTitle, aBaseCat),
   fNElements(0),
   fEventElements(nullptr),
   fMaxIndex(0)
{
}

TGo4CompositeEvent::~TGo4CompositeEvent()
{
   if(fEventElements) {

      fEventElements->Delete();

      delete fEventElements;

      fEventElements = nullptr;
   }
}

TGo4EventElement* TGo4CompositeEvent::GetChild(const char *name)
{
   TGo4EventElement* res = TGo4EventElement::GetChild(name);

   if (!res && fEventElements)
      res = dynamic_cast<TGo4EventElement*> (fEventElements->FindObject(name));

   return res;
}


void TGo4CompositeEvent::makeBranch(TBranch *parent)
{
   if (fEventElements)
      for (Int_t i = 0; i <= fEventElements->GetLast(); i++) {
         TGo4EventElement **par = (TGo4EventElement **)&((*fEventElements)[i]);
         if (par && *par) {
            TBranch *b = parent->GetTree()->TTree::Branch(TString::Format("%s.", (*par)->GetName()).Data(),
                                                          (*par)->ClassName(), par, 4000, 99);
            (*par)->makeBranch(b);
         }
      }

   TGo4EventElement::makeBranch(parent);
}

Int_t TGo4CompositeEvent::activateBranch(TBranch *branch, Int_t init, TGo4EventElement** var_ptr)
{
   // first read event itself
   TGo4EventElement::activateBranch(branch, init, var_ptr);

   if (!branch) return fNElements;

   // we need to call GetEntry here to get value of fNElements  and fMaxIndex
   branch->GetEntry(0);

   if (fDebug)
      TGo4Log::Debug("##### TGo4CompositeEvent::activateBranch called from obj:%s",this->GetName());

   TTree* tree = branch->GetTree();
   TObjArray *br = tree->GetListOfBranches();

   Int_t i = init;
   Int_t all_branches = fNElements;

   // expand object array to final size before setting branch addresses:
   ProvideArray();


   if (fDebug)
      TGo4Log::Debug("-I-TGo4CompositeEvent::activateBranch from obj:%s bname:%s Elements:%d  index:%d",
                     GetName(), branch->GetName(), fNElements, init);

   while (i < init + all_branches) {
      i++;
      TBranch* b = (TBranch*) br->At(i);
      Bool_t readentry = kFALSE;
      if (!b) continue;

      TString sub = b->GetName();
      sub.Remove(sub.Length()-1);
      TGo4EventElement* par = getEventElement(sub.Data());

      if (fDebug)
         TGo4Log::Debug("-I TGo4CompositeEvent::activateBranch use subbranch %s", b->GetName());

      if (!par) {

         TClass* cl = gROOT->GetClass(b->GetClassName());
         if (!cl) {
            TGo4Log::Debug("-I class %s cannot be reconstructed", b->GetClassName());
            continue;
         }

         par = (TGo4EventElement*) cl->New();
         // need to set correct object name:
         par->SetName(sub.Data());
         if(fDebug)
              TGo4Log::Debug("-I Created new instance of class %s, name:%s", cl->GetName(), par->GetName());

         if (!par) {
            TGo4Log::Error("-I class %s instance cannot be created", b->GetClassName());
            continue;
         }

         readentry = !par->isComposed();
      }

      // TODO: could we put element in the elements array BEFORE we activate branch
      //  in this case activate branch will set correct address from the beginning
      Int_t offset = par->activateBranch(b, i, &par);

      if (fDebug)
         TGo4Log::Debug("-I activate from obj:%s elems:%d index:%d adding:%s",
               this->GetName(), init+fNElements, i, par->GetName());

      // we need to getentry only when new object was created to get its id
      if (readentry) b->GetEntry(0);

      if (fDebug)
         TGo4Log::Debug("Add branch %s event %s offset %d", b->GetName(), par->GetName(), offset);

      if (addEventElement(par, kTRUE)) {
         TGo4EventElement** par_ptr = (TGo4EventElement**) &((*fEventElements)[par->getId()]);
         tree->SetBranchAddress(b->GetName(), par_ptr);
      }

      i+=offset;
      all_branches+=offset;
      if (fDebug)
         TGo4Log::Debug("-I from obj:%s activate indexing after offset:%d index:%d max:%d",
               this->GetName(), offset, i, init+all_branches);
   } //!while

   // FIXME: do we need clear method here ????
   // Clear();

   if (fDebug)
      TGo4Log::Debug("-I activate return value from obj:%s offset:%i", GetName(), all_branches);

   return all_branches;
}

void TGo4CompositeEvent::Clear(Option_t *opt)
{
   //Clears the data in the event (i.e. clears the internal buffers...)

   TGo4EventElement::Clear(opt);

   TIter next(fEventElements);
   while (auto ev = (TGo4EventElement *)next())
      ev->Clear(opt);
}

Bool_t TGo4CompositeEvent::addEventElement(TGo4EventElement* aElement, Bool_t reading)
{
   // if Identifiers  are needed for fast retrieval of elements
   // one should use:
   // >>> fEventElements->AddAtAndExpand(aElement,aElement->getId());
   // Note: When reading from file, adding elements should not be
   // incremented

   // when trying to add same element second time do nothing
   if (reading && fEventElements && fEventElements->FindObject(aElement)) return kTRUE;

   if (getEventElement(aElement->GetName(),1)) {
      TGo4Log::Error("<TGo4CompositeEvent::addEventElement> object:%s already in structure => not added !",
            aElement->GetName());
      return kFALSE;
   }

   if ( aElement->getId() < 0 ) {
      TGo4Log::Error("<TGo4CompositeEvent::addEventElement> object:%s with invalid Id:%d   => not added !",
            aElement->GetName(), aElement->getId());
      return kFALSE;
   }

   if (getEventElement(aElement->getId()) != nullptr) {
      TGo4Log::Error("<TGo4CompositeEvent::addEventElement> object:%s with Id:%d already used => not added !",
            aElement->GetName(), aElement->getId());
      return kFALSE;
   }

   ProvideArray();

   if (fDebug)
      TGo4Log::Debug("-I adding element in :%s :%p of id:%i",GetName(),aElement, aElement->getId());

   fEventElements->AddAtAndExpand(aElement,aElement->getId());
   if(!reading) fNElements++;
   if(aElement->getId()>fMaxIndex) fMaxIndex=aElement->getId();
   if (fDebug)
              TGo4Log::Debug("-I fNElements:%d fMaxIndex:%d",fNElements, fMaxIndex);

   return kTRUE;
}


TGo4EventElement* TGo4CompositeEvent::getEventElement(Int_t idx)
{
   // Returns a pointer to the partial event with array location idx.

   if (!fEventElements || (idx < 0) || (idx > fEventElements->GetLast()))
      return nullptr;
   return (TGo4EventElement *)fEventElements->At(idx);
}

TGo4EventElement* TGo4CompositeEvent::getEventElement(const char *name, Int_t final_element)
{
   TIter next(fEventElements);
   while (auto ev = (TGo4EventElement *)next()) {
      if(strcmp(name,ev->GetName()) == 0) return ev;
      if (ev->isComposed()) {
         TGo4EventElement* inter= ((TGo4CompositeEvent*) ev)->getEventElement(name,1);
         if (inter) return inter;
      }
   }
   if(final_element == 0)
      TGo4Log::Debug("TGo4CompositeEvent => Element:%s not found in Composite:%s", name, GetName());
   return nullptr;
}

void TGo4CompositeEvent::deactivate()
{
   TGo4EventElement::deactivate();

   TIter next(fEventElements);

   while (auto ev = (TGo4EventElement *)next())
      ev->deactivate();
}


void TGo4CompositeEvent::activate()
{
   TGo4EventElement::activate();

   TIter next(fEventElements);
   while (auto ev = (TGo4EventElement*)next())
      ev->activate();
}


TObjArray* TGo4CompositeEvent::getListOfComposites(Bool_t toplevel)
{
   TObjArray *comp = new TObjArray(12);
   if (toplevel) comp->Add(this);

   TIter next(fEventElements);
   //-- Add top level composite
   while (auto ev = (TGo4EventElement *)next()) {
      if (ev->isComposed()) {
         comp->Add( ev );
         TObjArray* dump = ((TGo4CompositeEvent *) ev)->getListOfComposites(kFALSE);

         comp->AddAll(dump);

         // SL: memory leak, list should be removed
         delete dump;
      }
   }
   return comp;
}


TGo4EventElement& TGo4CompositeEvent::operator[]( Int_t i )
{
   if (!fEventElements || (i < 0) || (i > fEventElements->GetLast())) {
      TGo4Log::Error("Wrong index %d in TGo4CompositeEvent::operator[]", i);
      return *this;
   }

   return * ((TGo4EventElement*) (fEventElements->At(i)));
}


void TGo4CompositeEvent::ProvideArray()
{
   if (!fEventElements) {
      Int_t size = fMaxIndex+1;
      fEventElements = new TObjArray(size);
      if (fDebug)
         TGo4Log::Debug("-I creating TObjArray of size %i",size);
   }
   if(fMaxIndex+1 >fEventElements->GetSize()) {
      fEventElements->Expand(fMaxIndex+1);
      if (fDebug)
         TGo4Log::Debug("-I Expanded component array to size %i",fEventElements->GetSize());
   }
}


TTree* TGo4CompositeEvent::CreateSampleTree(TGo4EventElement** sample)
{
   TDirectory* filsav = gDirectory;
   gROOT->cd();
   if (sample) delete *sample;
   TGo4CompositeEvent* clone = (TGo4CompositeEvent*) Clone();
   TTree* thetree = new TTree(clone->GetName(), "Single Event Tree");
   thetree->SetDirectory(nullptr);
   if (sample) *sample = clone;
   TBranch *topbranch =
      thetree->Branch("Go4EventSample", clone->ClassName(), sample ? (TGo4CompositeEvent**) sample : &clone, 64000, 99);
   clone->makeBranch(topbranch);
   thetree->Fill();
   filsav->cd();
   if (!sample) delete clone;
   return thetree;
}
