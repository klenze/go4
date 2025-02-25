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

#include "TGo4WinCond.h"

#include "TH1.h"

#include "TGo4WinCondPainter.h"
#include "TGo4Log.h"

TString TGo4WinCond::fgxURL_XLOW="xmin";
TString TGo4WinCond::fgxURL_XUP="xmax";
TString TGo4WinCond::fgxURL_YLOW="ymin";
TString TGo4WinCond::fgxURL_YUP="ymax";


// -----------------------------------------------
// Constructors
// -----------------------------------------------
TGo4WinCond::TGo4WinCond() :
   TGo4Condition(),
   fLow1(0),
   fUp1(0),
   fLow2(0),
   fUp2(0),
   fiSaveXMin(0),
   fiSaveXMax(0),
   fiSaveYMin(0),
   fiSaveYMax(0)
{
   GO4TRACE((15,"TGo4WinCond::TGo4WinCond()",__LINE__, __FILE__));
}
// -----------------------------------------------
TGo4WinCond::TGo4WinCond(const char *name, const char *title) :
   TGo4Condition(name,title),
   fLow1(0),
   fUp1(0),
   fLow2(0),
   fUp2(0),
   fiSaveXMin(0),
   fiSaveXMax(0),
   fiSaveYMin(0),
   fiSaveYMax(0)
{
GO4TRACE((15,"TGo4WinCond::TGo4WinCond(name,title)",__LINE__, __FILE__));
}

// -----------------------------------------------
TGo4WinCond::~TGo4WinCond()
{
GO4TRACE((15,"TGo4WinCond::~TGo4WinCond()",__LINE__, __FILE__));
}

// -----------------------------------------------
Bool_t TGo4WinCond::Test(Double_t v1, Double_t v2)
{
   IncCounts();
   if (!IsEnabled()) {
      if (FixedResult())
         IncTrueCounts();
      return FixedResult();
   }
   if (v1 < fLow1)
      return IsFalse();
   if (v1 >= fUp1)
      return IsFalse();
   if (v2 < fLow2)
      return IsFalse();
   if (v2 >= fUp2)
      return IsFalse();
   IncTrueCounts();
   return IsTrue();
}

// -----------------------------------------------
Bool_t TGo4WinCond::Test(Double_t v1)
{
   IncCounts();
   if (!IsEnabled()) {
      if (FixedResult())
         IncTrueCounts();
      return FixedResult();
   }
   if (v1 < fLow1)
      return IsFalse();
   if (v1 >= fUp1)
      return IsFalse();
   IncTrueCounts();
   return IsTrue();
}
// -----------------------------------------------
void TGo4WinCond::SetValues(Double_t low1, Double_t up1, Double_t low2, Double_t up2)
{
  fLow1 = low1;
  fUp1  = up1;
  fLow2 = low2;
  fUp2  = up2;
  SetDimension(2);
}

// -----------------------------------------------
void TGo4WinCond::SetValues(Double_t low1, Double_t up1)
{
   fLow1 = low1;
   fUp1  = up1;
   SetDimension(1);
}

// -----------------------------------------------
void TGo4WinCond::GetValues(Int_t & dim, Double_t & x1, Double_t & y1, Double_t & x2, Double_t & y2)
{
   x1  = fLow1;
   y1  = fUp1;
   x2  = fLow2;
   y2  = fUp2;
   dim = GetDimension();
}

// -----------------------------------------------
void TGo4WinCond::PrintCondition(Bool_t limits)
{
   TGo4Condition::PrintCondition();
   if (limits) {
      if (GetDimension() == 1)
         std::cout << TString::Format("[%8.2f,%8.2f]", fLow1, fUp1) << std::endl;
      else
         std::cout << TString::Format("[%8.2f,%8.2f][%8.2f,%8.2f]", fLow1, fUp1, fLow2, fUp2) << std::endl;
   }
}

// -----------------------------------------------
Double_t TGo4WinCond::GetIntegral(TH1* histo, Option_t* opt)
{
   if(!histo) return 0;
   SetHistogramRanges(histo);
   Double_t result = histo->Integral(opt);
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetMean(TH1* histo, Int_t axis)
{
   if(!histo) return 0;
   SetHistogramRanges(histo);
   Double_t result = histo->GetMean(axis);
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetRMS(TH1* histo, Int_t axis)
{
   if(!histo) return 0;
   SetHistogramRanges(histo);
   Double_t result = histo->GetRMS(axis);
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetSkewness(TH1* histo, Int_t axis)
{
   if(!histo) return 0;
   SetHistogramRanges(histo);
   Double_t result = histo->GetSkewness(axis);
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetCurtosis(TH1* histo, Int_t axis)
{
   if(!histo) return 0;
   SetHistogramRanges(histo);
   Double_t result = histo->GetKurtosis(axis);
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetXMax(TH1* histo)
{
   if(!histo) return 0;
   Double_t result = 0.;
   SetHistogramRanges(histo);
   TAxis* xax=histo->GetXaxis();
   Int_t maxbin=histo->GetMaximumBin();
   if (histo->GetDimension() == 1) {
      result = xax->GetBinCenter(maxbin);
   } else if (histo->GetDimension() == 2) {
      Int_t xmaxbin = maxbin % (histo->GetNbinsX() + 2);
      result = xax->GetBinCenter(xmaxbin);
   } else {
      result = 0; // no support for 3d histos at the moment!
   }
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetYMax(TH1* histo)
{
   if(!histo) return 0;
   Double_t result = 0;
   SetHistogramRanges(histo);
   if (histo->GetDimension() == 1) {
      result = histo->GetMaximum();
   } else if (histo->GetDimension() == 2) {
      TAxis *yax = histo->GetYaxis();
      Int_t maxbin = histo->GetMaximumBin();
      Int_t maxybin = maxbin / (histo->GetNbinsX() + 2);
      result = yax->GetBinCenter(maxybin);
   } else {
      result = 0; // no support for 3d histos at the moment!
   }
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
Double_t TGo4WinCond::GetCMax(TH1* histo)
{
   if(!histo) return 0;
   SetHistogramRanges(histo);
   Double_t result = histo->GetMaximum();
   RestoreHistogramRanges(histo);
   return result;
}

// -----------------------------------------------
void TGo4WinCond::SetHistogramRanges(TH1* histo)
{
   if(!histo) return;
   Double_t xmin = fLow1;
   Double_t xmax = fUp1;
   Double_t ymin = fLow2;
   Double_t ymax = fUp2;
   TAxis *xax = histo->GetXaxis();
   fiSaveXMin = xax->GetFirst();
   fiSaveXMax = xax->GetLast();
   Int_t xminbin = xax->FindBin(xmin);
   Int_t xmaxbin = xax->FindBin(xmax);
   Int_t yminbin = 0;
   Int_t ymaxbin = 0;
   TAxis *yax = histo->GetYaxis();
   if (yax && histo->GetDimension() > 1) {
      fiSaveYMin = yax->GetFirst();
      fiSaveYMax = yax->GetLast();
      yminbin = yax->FindBin(ymin);
      ymaxbin = yax->FindBin(ymax);
   }
   // set histo range to condition limits
   xax->SetRange(xminbin, xmaxbin);
   if (yax && histo->GetDimension() > 1)
      yax->SetRange(yminbin, ymaxbin);
}

// -----------------------------------------------
void TGo4WinCond::RestoreHistogramRanges(TH1* histo)
{
   if(!histo) return;
   TAxis* xax=histo->GetXaxis();
   TAxis* yax=histo->GetYaxis();
   xax->SetRange(fiSaveXMin,fiSaveXMax);
   if(yax&& histo->GetDimension()>1)
      yax->SetRange(fiSaveYMin,fiSaveYMax);
}

// -----------------------------------------------
Bool_t TGo4WinCond::UpdateFrom(TGo4Condition * cond, Bool_t counts)
{
   if(!TGo4Condition::UpdateFrom(cond,counts)) return kFALSE;
   if(!cond->InheritsFrom(TGo4WinCond::Class())) {
      std::cout << "Cannot update " << GetName() << " from " << cond->ClassName() << std::endl;
      return kFALSE;
   }
   Int_t dimension = 0;
   ((TGo4WinCond*)cond)->GetValues(dimension,fLow1,fUp1,fLow2,fUp2);  // get limits from source
   SetDimension(dimension);
   return kTRUE;
}

// -----------------------------------------------

Bool_t TGo4WinCond::UpdateFromUrl(const char *rest_url_opt)
{
  if (!TGo4Condition::UpdateFromUrl(rest_url_opt))
    return kFALSE;
  TGo4Log::Message(1, "TGo4WinCond::UpdateFromUrl - condition %s: with url:%s", GetName(), rest_url_opt);
  TString message;
  if (UrlOptionHasKey(TGo4WinCond::fgxURL_XLOW)) {
    Double_t xmin = GetUrlOptionAsDouble(TGo4WinCond::fgxURL_XLOW, GetXLow());
    Double_t xmax = GetUrlOptionAsDouble(TGo4WinCond::fgxURL_XUP, GetXUp());
    Double_t ymin = GetUrlOptionAsDouble(TGo4WinCond::fgxURL_YLOW, GetYLow());
    Double_t ymax = GetUrlOptionAsDouble(TGo4WinCond::fgxURL_YUP, GetYUp());
    message.Form("Set Window condition %s:", GetName());
    Int_t dim = GetDimension();
    switch (dim) {
      case 1:
        SetValues(xmin, xmax);
        message.Append(TString::Format(", set limits to (%f, %f)", xmin, xmax));
        break;
      case 2:
        SetValues(xmin, xmax, ymin, ymax);
        message.Append(TString::Format(", set limits to (%f, %f) (%f, %f)", xmin, xmax, ymin, ymax));
        break;
      default:
        message.Append(TString::Format(" !wrong condition dimension %d, NEVER COME HERE", dim));
        break;
    };
  } else {
    std::cout << "DEBUG- no limits to change received" << std::endl;
  }
  if (message.Length() > 0)
     TGo4Log::Message(1, "%s", message.Data());
  return kTRUE;
}

void TGo4WinCond::SetPainter(TGo4ConditionPainter* painter)
{
   // delete old painter, replace by the new one
   // overwritten method in subclass may check if painter is correct type
   if(!painter) return;
   if(painter->InheritsFrom(TGo4WinCondPainter::Class())) {
      if(fxPainter) delete fxPainter;
      fxPainter = painter;
      fxPainter->SetCondition(this);
   } else {
      TGo4Log::Warn("Could not set painter of class %s for TGo4WinCond %s",
            painter->ClassName(),GetName());
   }
}

TGo4ConditionPainter* TGo4WinCond::CreatePainter()
{
   TGo4ConditionPainter* painter=new TGo4WinCondPainter(GetName());
   painter->SetCondition(this);
   return painter;
}

Int_t TGo4WinCond::GetMemorySize()
{
   Int_t size = sizeof(*this);
   if (GetName()) size += strlen(GetName());
   if (GetTitle()) size += strlen(GetTitle());
   return size;
}

void TGo4WinCond::SavePrimitive(std::ostream& out, Option_t* opt)
{
   static int cnt = 0;

   TString varname = MakeScript(out, TString::Format("wincond%d", cnt++), opt);

   Int_t dim;
   Double_t  xl,xu,yl,yu;
   GetValues(dim,xl,xu,yl,yu);

   if(dim==1) out << TString::Format("   %s->SetValues(%f, %f);", varname.Data(), xl, xu) << std::endl;
         else out << TString::Format("   %s->SetValues(%f, %f, %f, %f);", varname.Data(), xl, xu, yl, yu) << std::endl;
}


