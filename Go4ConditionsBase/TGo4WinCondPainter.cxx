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

#include "TGo4WinCondPainter.h"

#include "TVirtualPad.h"
#include "TROOT.h"

#include "TGo4WinCond.h"
#include "TGo4WinCondView.h"

TGo4WinCondPainter::TGo4WinCondPainter() :
   TGo4ConditionPainter(),
   fxBox(nullptr)
{
}

TGo4WinCondPainter::TGo4WinCondPainter(const char *name, const char *title) :
   TGo4ConditionPainter(name,title),
   fxBox(nullptr)
{
}

TGo4WinCondPainter::~TGo4WinCondPainter()
{
   UnPaintCondition();
   if (fxBox) {
      delete fxBox;
      fxBox = nullptr;
   }
}


void TGo4WinCondPainter::PaintCondition(Option_t* opt)
{
   if (!gPad)
      return;
   TObject *boxinpad = gPad->GetListOfPrimitives()->FindObject(fxBox);
   // std::cout<<"TGo4WinCondPainter::PaintCondition with fxBox= "<<(long) fxBox<<",
   // isatexecutemousevent="<<(fxBox?fxBox->IsAtExecuteMouseEvent():0) << std::endl;
   if (fxBox && boxinpad && fxBox->IsAtExecuteMouseEvent())
      return; // JAM suppress resetting coordinates during mouse modification of box
   double xpmin = 0, xpmax = 0, ypmin = 0, ypmax = 0;
   int dim = 0;
   TGo4WinCond *wconny = dynamic_cast<TGo4WinCond *>(fxCondition);
   if (wconny && wconny->IsVisible()) {
      wconny->GetValues(dim, xpmin, xpmax, ypmin, ypmax);

      // for loaded window conditions, we have to correct limits
      // otherwise, this would yield wrong statistics from work histogram
      // root would correct box limits only on first edge resize!
      if (xpmin > xpmax) {
         if (wconny->GetDimension() > 1) // keep dimension!
            wconny->SetValues(xpmax, xpmin, ypmin, ypmax);
         else
            wconny->SetValues(xpmax, xpmin);
      }
      if (ypmin > ypmax) {
         if (wconny->GetDimension() > 1) // keep dimension!
            wconny->SetValues(xpmin, xpmax, ypmax, ypmin);
      }

      //      std::cout <<"\nBefore toPad:"<< std::endl;
      //      std::cout <<" xpmin="<<xpmin << std::endl;
      //      std::cout <<" xpmax="<<xpmax << std::endl;
      //      std::cout <<" ypmin="<<ypmin << std::endl;
      //      std::cout <<" ypmax="<<ypmax << std::endl;

      if (dim == 1) {
         ypmin = gPad->PadtoY(gPad->GetUymin());
         ypmax = gPad->PadtoY(gPad->GetUymax());
      }

      // if(!fxBox || !boxinpad)
      // user might have deleted box from pad by mouse even if fxBox != nullptr
      // JAM 2016: this is a memory leak! for each condition update we get new condition view
      // better: suppress Delete in mouse menu in QtROOT interface (disregard plain ROOT browser here :-))
      if (!fxBox) {
         fxBox = new TGo4WinCondView(xpmin, ypmin, xpmax, ypmax);
         // std::cout <<"TGo4WinCondPainter::PaintCondition creates new fxBox." <<std::endl;
      } else {
         fxBox->SetX1(xpmin);
         fxBox->SetY1(ypmin);
         fxBox->SetX2(xpmax);
         fxBox->SetY2(ypmax);
      }
      fxBox->SetCondition(wconny);
      if (!strcmp(opt, "fixstyle")) {
         // reproduce condition colors always
         // this mode will prevent the user from changing box color interactively
         fxBox->SetLineWidth(wconny->GetLineWidth());
         fxBox->SetLineColor(wconny->GetLineColor());
         fxBox->SetLineStyle(wconny->GetLineStyle());
         fxBox->SetFillColor(wconny->GetFillColor());
         fxBox->SetFillStyle(wconny->GetFillStyle());
      }
      if (!boxinpad) {
         fxBox->SetLineWidth(wconny->GetLineWidth());
         fxBox->SetLineColor(wconny->GetLineColor());
         fxBox->SetLineStyle(wconny->GetLineStyle());
         fxBox->SetFillColor(wconny->GetFillColor());
         fxBox->SetFillStyle(wconny->GetFillStyle());
         fxBox->AppendPad(); // only append to pad if not already there
                             // this is necessary for the Pop() in array painter
      }
      fxBox->Paint(); // for condarrays

   } // if(wconny && wconny->IsVisible())
   else {
      UnPaintCondition();
   }
}

void TGo4WinCondPainter::UnPaintCondition(Option_t *opt)
{

   gROOT->GetListOfCanvases()->RecursiveRemove(fxBox);
   // we do not delete view, but restore graphics properties though invisible
   TString option(opt);
   if (option.Contains("reset")) {
      // case of reset option: discard old label geometry
      delete fxBox;
      fxBox = nullptr;
   }
}

void TGo4WinCondPainter::DisplayToFront(Option_t *opt)
{
   if (fxBox)
      fxBox->Pop();                       // box to front
   TGo4LabelPainter::DisplayToFront(opt); // label itself will be frontmost
}
