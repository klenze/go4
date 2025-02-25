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

#include "TGo4Condition.h"

#include "TH1.h"
#include "TROOT.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TVirtualPad.h"
#include "TDatime.h"

#include "TGo4Log.h"
#include "TGo4ConditionPainter.h"

const Double_t TGo4Condition::fgdUPDATEEPSILON = 0.01;

Bool_t TGo4Condition::fgbLABELDRAW=kTRUE;
Bool_t TGo4Condition::fgbLIMITSDRAW=kTRUE;
Bool_t TGo4Condition::fgbINTDRAW=kTRUE;
Bool_t TGo4Condition::fgbXMEANDRAW=kTRUE;
Bool_t TGo4Condition::fgbXRMSDRAW=kTRUE;
Bool_t TGo4Condition::fgbYMEANDRAW=kFALSE;
Bool_t TGo4Condition::fgbYRMSDRAW=kFALSE;
Bool_t TGo4Condition::fgbXMAXDRAW=kTRUE;
Bool_t TGo4Condition::fgbYMAXDRAW=kFALSE;
Bool_t TGo4Condition::fgbCMAXDRAW=kTRUE;
TString TGo4Condition::fgxNUMFORMAT="%.4E";

/** Following keywords should be used by condition editor in web browser: */
TString TGo4Condition::fgxURL_RESET="resetcounters";
TString TGo4Condition::fgxURL_RESULT="resultmode";
TString TGo4Condition::fgxURL_INVERT="invertmode";
TString TGo4Condition::fgxURL_VISIBLE="visible";
TString TGo4Condition::fgxURL_LABEL="labeldraw";
TString TGo4Condition::fgxURL_LIMITS="limitsdraw";
TString TGo4Condition::fgxURL_INTEGRAL="intdraw";
TString TGo4Condition::fgxURL_XMEAN="xmeandraw";
TString TGo4Condition::fgxURL_XRMS="xrmsdraw";
TString TGo4Condition::fgxURL_YMEAN="ymeandraw";
TString TGo4Condition::fgxURL_YRMS="yrmsdraw";
TString TGo4Condition::fgxURL_XMAX="xmaxdraw";
TString TGo4Condition::fgxURL_YMAX="ymaxdraw";
TString TGo4Condition::fgxURL_CMAX="cmaxdraw";

// -----------------------------------------------
// Constructors
// ---------------------------------------------------------
TGo4Condition::TGo4Condition() :
   TNamed(),
   TAttLine(),
   TAttFill(),
   fiIsChanged(0)
{
   GO4TRACE((15,"TGo4Condition::TGo4Condition()",__LINE__, __FILE__));
   fiDim = 0;
   fbEnabled = false;
   fbResult  = true;
   fbTrue    = true;
   fbFalse   = false;
   fbMarkReset = false;
   fiCounts  = 0;
   fiTrueCounts = 0;
   fbVisible = true;
   fbMultiEdit = true;
   fbHistogramLink = false;
   fdUpdateEpsilon = fgdUPDATEEPSILON;
   fbIsPainted = kFALSE;
   fbOwnedByEditor = kFALSE;
   fbStreamedCondition = kTRUE;
   InitLabelStyle();
}

// -----------------------------------------------
TGo4Condition::TGo4Condition(const char *name, const char *title) :
   TNamed(name, title),
   TAttLine(),
   TAttFill(),
   fiIsChanged(0)
{
   GO4TRACE((15,"TGo4Condition::TGo4Condition(const char*)",__LINE__, __FILE__));
   fiDim = 0;
   fbEnabled = false;
   fbResult = true;
   fbTrue = true;
   fbFalse = false;
   fbMarkReset = false;
   fiCounts = 0;
   fiTrueCounts = 0;
   fbVisible = true;
   fbMultiEdit = true;
   fbHistogramLink = false;
   fdUpdateEpsilon = fgdUPDATEEPSILON;
   fbIsPainted = kFALSE;
   fbOwnedByEditor = kFALSE;
   fbStreamedCondition = kFALSE;
   InitLabelStyle();
}

// ---------------------------------------------------------
TGo4Condition::~TGo4Condition()
{
   GO4TRACE((15,"TGo4Condition::~TGo4Condition()",__LINE__, __FILE__));
   //UnDraw("reset");
   if(fxPainter) {
      delete fxPainter;
      fxPainter = nullptr;
   }

   if (fxUrlOptionArray) {
      fxUrlOptionArray->Delete();
      delete fxUrlOptionArray;
      fxUrlOptionArray = nullptr;
   }
}

// ---------------------------------------------------------
Bool_t TGo4Condition::Test()
{
   fiCounts++;
   return true;
}
// ---------------------------------------------------------

void TGo4Condition::AddCondition(TGo4Condition* next)
{
   GO4TRACE((14,"TGo4Condition::AddCondition(TGo4Condition*)",__LINE__, __FILE__));
}
// ---------------------------------------------------------
void TGo4Condition::IncTrueCounts()
{
   GO4TRACE((12,"TGo4Condition::IncTrueCounts()",__LINE__, __FILE__));
   fiTrueCounts++;
}
// ---------------------------------------------------------
void TGo4Condition::IncCounts()
{
   GO4TRACE((12,"TGo4Condition::IncCounts()",__LINE__, __FILE__));
   fiCounts++;
}
// ---------------------------------------------------------
Int_t TGo4Condition::Counts()
{
   GO4TRACE((12,"TGo4Condition::Counts()",__LINE__, __FILE__));
   return fiCounts;
}
// ---------------------------------------------------------
Int_t TGo4Condition::TrueCounts()
{
   GO4TRACE((12,"TGo4Condition::TrueCounts()",__LINE__, __FILE__));
   return fiTrueCounts;
}
// ---------------------------------------------------------
void TGo4Condition::ResetCounts()
{
   GO4TRACE((12,"TGo4Condition::ResetCounts()",__LINE__, __FILE__));
   fiTrueCounts = 0;
   fiCounts = 0;
}
// ---------------------------------------------------------
void TGo4Condition::SetCounts(Int_t truecounts, Int_t counts)
{
   GO4TRACE((12,"TGo4Condition::SetCounts()",__LINE__, __FILE__));
   fiTrueCounts = truecounts;
   fiCounts = counts;
}


// ---------------------------------------------------------
void TGo4Condition::Invert(Bool_t on)
{
   GO4TRACE((12,"TGo4Condition::Invert",__LINE__, __FILE__));
   fbTrue  = on ^ true;
   fbFalse = on ^ false;
}

// ---------------------------------------------------------
void TGo4Condition::PrintCondition(Bool_t full)
{
   std::cout << "Name:" << GetName() << " type:" << ClassName() << " title:" << GetTitle() << std::endl;
   if (fbHistogramLink)
      std::cout << "Connected to histogram " << fxHistoName << std::endl;

   Float_t perc = (fiCounts == 0) ? 0.0 : 100.0 / fiCounts * fiTrueCounts;

   TString line;
   if (fbEnabled)
      line = "Is Checked   ";
   else if (fbResult)
      line = "Always True  ";
   else
      line = "Always False ";
   line.Append(fbTrue ? "normal  " : "inverse ");
   line.Append(TString::Format(", tested: %8d true: %8d is %3.0f%s", fiCounts, fiTrueCounts, perc, "%"));
   std::cout << line << std::endl;
}

// ---------------------------------------------------------
void TGo4Condition::PrintBar()
{
   Float_t perc = (fiCounts == 0) ? 0. : 100.0/fiCounts*fiTrueCounts;
   if (perc < 0)
      perc = 0;
   else if (perc>100)
      perc = 100;

   char num[64];
   strncpy(num, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", sizeof(num));
   char *pc = num + (Int_t)perc/2;
   *pc = 0;
   std::cout << TString::Format("%-24s %8d %3.0f%s |%-50s|",GetName(),fiCounts,perc,"%",num) << std::endl;
}

// -----------------------------------------------
void TGo4Condition::Print(Option_t* opt) const
{
   //std::cout <<"MyPrint:"<<GetName() << std::endl;
   TGo4Condition* localthis=const_cast<TGo4Condition*>(this);
   TString option = opt;
   option.ToLower();
   if (option.IsNull() || option == "*") {
      // old default: we print bar graphics to std::cout
      localthis->PrintBar();
   } else {
      // new printout of condition with different options:
      TString textbuffer = "\nCondition ";
      textbuffer.Append(GetName());
      if (localthis->IsPolygonType()) {
         textbuffer.Append(" (Polygon type, 2-dim)");
      } else {
         // JAM2019: use this kludge to avoid changing the baseclass api...
         if (InheritsFrom("TGo4ListCond")) {
            textbuffer.Append(" (Whitelist type, 1-dim)");
         } else {

            textbuffer.Append(" (Window type,");
            if (localthis->GetActiveCondition()->GetDimension() > 1)
               textbuffer.Append(" 2-dim)");
            else
               textbuffer.Append(" 1-dim)");
         }
      }

      // textbuffer+="\n";
      if (option.Contains("limits"))
         textbuffer.Append(
            TString::Format("\n!  Xlow: \t\tXup: \t\tYlow: \t\tYup:\n   %.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t",
                            localthis->GetXLow(), localthis->GetXUp(), localthis->GetYLow(), localthis->GetYUp()));

      if (option.Contains("flags"))
         textbuffer.Append(TString::Format(
            "\n!  Status:\n!  Enab.: \tVis.: \tRes.: \tTrue: \tCnts: \tTrueCnts:\n   %d\t\t%d\t%d\t%d\t%d\t%d",
            localthis->fbEnabled, localthis->IsVisible(), localthis->fbResult, localthis->fbTrue, localthis->Counts(),
            localthis->TrueCounts()));

      if (option.Contains("stats")) {
         // output of region statistics

         textbuffer.Append("\n!  with");
         TH1 *hist = localthis->GetWorkHistogram();
         if (hist) {
            textbuffer.Append(" histogram: ");
            textbuffer.Append(hist->GetName());
            textbuffer.Append(
               TString::Format("\n!   Int:\t\tXmax:\t\tYmax:\t\tCmax:\t\tXmean:\t\tYmean:\t\tXrms:\t\tYrms:\n    "
                               "%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f",
                               localthis->GetIntegral(hist), localthis->GetXMax(hist), localthis->GetYMax(hist),
                               localthis->GetCMax(hist), localthis->GetMean(hist, 1), localthis->GetMean(hist, 2),
                               localthis->GetRMS(hist, 1), localthis->GetRMS(hist, 2)));
         } else {
            textbuffer.Append("out histogram");
         }
      }
      // now check output mode:
      if (option.Contains("go4log")) {
         TGo4Log::Message(1, "%s", textbuffer.Data());
      } else {
         std::cout << textbuffer.Data() << std::endl;
      }
   } // if(option.IsNull())
}

// ---------------------------------------------------------
Bool_t TGo4Condition::UpdateFrom(TGo4Condition * cond, Bool_t counts)
{
    fbTrue    = cond->TGo4Condition::IsTrue();
    fbFalse   = cond->TGo4Condition::IsFalse();
    fbResult  = cond->TGo4Condition::FixedResult();
    fbEnabled = cond->TGo4Condition::IsEnabled();
    fbMarkReset = cond->fbMarkReset;
    fbVisible=cond->TGo4Condition::IsVisible();
    fbLabelDraw=cond->TGo4Condition::IsLabelDraw();
    fbLimitsDraw=cond->TGo4Condition::IsLimitsDraw();
    fbMultiEdit=cond->TGo4Condition::IsMultiEdit();
    fbIntDraw= cond->TGo4Condition::IsIntDraw();
    fbXMeanDraw= cond->TGo4Condition::IsXMeanDraw();
    fbXRMSDraw= cond->TGo4Condition::IsXRMSDraw();
    fbYMeanDraw= cond->TGo4Condition::IsYMeanDraw();
    fbYRMSDraw= cond->TGo4Condition::IsYRMSDraw();
    fbXMaxDraw= cond->TGo4Condition::IsXMaxDraw();
    fbYMaxDraw= cond->TGo4Condition::IsYMaxDraw();
    fbCMaxDraw= cond->TGo4Condition::IsCMaxDraw();
    fbHistogramLink=cond->TGo4Condition::IsHistogramLink();
    fxHistoName=cond->fxHistoName;
    fdUpdateEpsilon= cond->fdUpdateEpsilon;
    fiDim=cond->GetDimension();
    if(counts){
      fiCounts = cond->TGo4Condition::Counts();
      fiTrueCounts = cond->TGo4Condition::TrueCounts();
      fbMarkReset = false;
    }
    if(fbMarkReset){
       ResetCounts();
       fbMarkReset = false;
    }

    return kTRUE;
}

void TGo4Condition::BuildUrlOptionArray(const char *rest_url_opt)
{
  if(fxUrlOptionArray) {
    fxUrlOptionArray->Delete();
    delete fxUrlOptionArray;
    fxUrlOptionArray = nullptr; // bad implementation of Tokenize, many memory leak dangers!
  }
  TString options = rest_url_opt;
  fxUrlOptionArray = options.Tokenize("&");
}

Bool_t TGo4Condition::UrlOptionHasKey(const char *key)
{
   TObjArrayIter iter(fxUrlOptionArray);
   TObject *cursor = nullptr;
   while ((cursor = iter.Next()) != nullptr) {
      TObjString *curopt = dynamic_cast<TObjString *>(cursor);
      if (curopt) {
         TString theOption = curopt->GetString();
         if (theOption.Contains(key)) {
            return kTRUE;
         }
      }
   } // while
   return kFALSE;
}

TString TGo4Condition::GetUrlOptionAsString(const char *key, TString def_value)
{
   TObjArrayIter iter(fxUrlOptionArray);
   TObject *cursor = nullptr;
   TObjArray *valuearray;
   while ((cursor = iter.Next()) != nullptr) {
      TObjString *curopt = dynamic_cast<TObjString *>(cursor);
      if (curopt) {
         TString theOption = curopt->GetString();
         if (theOption.Contains(key)) {
            valuearray = theOption.Tokenize("=");
            TString theValue = valuearray->Last()->GetName();
            valuearray->Delete();
            delete valuearray; // bad implementation of Tokenize, many memory leak dangers!
            return theValue;
         }
      }
   } // while
   return def_value;
}

Int_t TGo4Condition::GetUrlOptionAsInt(const char *key, Int_t def_value)
{
   TString valstring = GetUrlOptionAsString(key, "");
   if (valstring.IsNull())
      return def_value;
   else
      return valstring.Atoi();
}

Double_t TGo4Condition::GetUrlOptionAsDouble(const char *key, Double_t def_value)
{
  TString valstring=GetUrlOptionAsString(key,"");
   if(valstring.IsNull())
     return def_value;
   else
     return valstring.Atof();
}


Bool_t TGo4Condition::UpdateFromUrl(const char *rest_url_opt)
{
  TString message;
  message.Form("TGo4Condition::UpdateFromUrl - condition %s: with url:%s", GetName(), rest_url_opt);
  TGo4Log::Message(1, "%s", message.Data());
  BuildUrlOptionArray(rest_url_opt); // split option string into separate key value entries


  // all keywords are defined as static class variables of condition class

  Int_t resetcounters=GetUrlOptionAsInt(TGo4Condition::fgxURL_RESET.Data(), -1);

  Int_t resultmode = GetUrlOptionAsInt(TGo4Condition::fgxURL_RESULT.Data(), -1);
  Int_t invertmode = GetUrlOptionAsInt(TGo4Condition::fgxURL_INVERT.Data(), -1);
  Int_t visible = GetUrlOptionAsInt(TGo4Condition::fgxURL_VISIBLE.Data(), -1);
  Int_t labeldraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_LABEL.Data(), -1);
  Int_t limitsdraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_LIMITS.Data(), -1);
  Int_t integraldraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_INTEGRAL.Data(), -1);
  Int_t xmeandraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_XMEAN.Data(), -1);
  Int_t xrmsdraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_XRMS.Data(), -1);
  Int_t ymeandraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_YMEAN.Data(), -1);
  Int_t yrmsdraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_YRMS.Data(), -1);
  Int_t xmaxdraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_XMAX.Data(), -1);
  Int_t ymaxdraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_YMAX.Data(), -1);
  Int_t cmaxdraw = GetUrlOptionAsInt(TGo4Condition::fgxURL_CMAX.Data(), -1);

  message.Form("Set condition %s:", GetName());

  if (resetcounters > 0) {
     ResetCounts();
     message.Append(TString::Format(", resetcounters=%d", resetcounters));
  }

  if (resultmode >= 0) {
     // same as in Go4 GUI condition editor:
     switch (resultmode) {
        case 0: Enable(); break;
        case 1: Disable(kTRUE); break;
        case 2: Disable(kFALSE); break;
        default: Enable(); break;
     };
     message.Append(TString::Format(", resultmode=%d", resultmode));
  }

  if (invertmode >= 0) {
     // same as in Go4 GUI condition editor:
     Invert(invertmode == 1);
     message.Append(TString::Format(", invertmode=%d", invertmode));
  }

  if (visible >= 0) {
     SetVisible(visible == 1);
     message.Append(TString::Format(", visible=%d", visible));
  }
  if (labeldraw >= 0) {
     SetLabelDraw(labeldraw == 1);
     message.Append(TString::Format(", labeldraw=%d", labeldraw));
  }
  if (limitsdraw >= 0) {
     SetLimitsDraw(limitsdraw == 1);
     message.Append(TString::Format(", limitsdraw=%d", limitsdraw));
  }
  if (integraldraw >= 0) {
     SetIntDraw(integraldraw == 1);
     message.Append(TString::Format(", intdraw=%d", integraldraw));
  }
  if (xmeandraw >= 0) {
     SetXMeanDraw(xmeandraw == 1);
     message.Append(TString::Format(", xmeandraw=%d", xmeandraw));
  }
  if (xrmsdraw >= 0) {
     SetXRMSDraw(xrmsdraw == 1);
     message.Append(TString::Format(", xrmsdraw=%d", xrmsdraw));
  }
  if (ymeandraw >= 0) {
     SetYMeanDraw(ymeandraw == 1);
     message.Append(TString::Format(", ymeandraw=%d", ymeandraw));
  }
  if (yrmsdraw >= 0) {
     SetYRMSDraw(yrmsdraw == 1);
     message.Append(TString::Format(", yrmsdraw=%d", yrmsdraw));
  }
  if (xmaxdraw >= 0) {
     SetXMaxDraw(xmaxdraw == 1);
     message.Append(TString::Format(", xmaxdraw=%d", xmaxdraw));
  }
  if (ymaxdraw >= 0) {
     SetYMaxDraw(ymaxdraw == 1);
     message.Append(TString::Format(", ymaxdraw=%d", ymaxdraw));
  }
  if (cmaxdraw >= 0) {
     SetCMaxDraw(cmaxdraw == 1);
     message.Append(TString::Format(", cmaxdraw=%d", cmaxdraw));
  }

  TGo4Log::Message(1, "%s", message.Data());

  return kTRUE;
}


void TGo4Condition::GetValues(Int_t & dim, Double_t & xmin, Double_t & xmax, Double_t & ymin, Double_t & ymax)
{
   xmin  = GetXLow();
   xmax  = GetXUp();
   ymin  = GetYLow();
   ymax  = GetYUp();
   dim = GetDimension();

}

Int_t TGo4Condition::GetMemorySize()
{
   Int_t size = sizeof(*this);
   if (GetName()) size += strlen(GetName());
   if (GetTitle()) size += strlen(GetTitle());
   return size;
}

void TGo4Condition::MarkReset(Bool_t on)
{
   fbMarkReset = on;
}

void TGo4Condition::Clear(Option_t *)
{
   ResetCounts();
}

void TGo4Condition::GetFlags(Bool_t *enabled, Bool_t *lastresult, Bool_t *markreset, Bool_t *result, Bool_t *truevalue,
                             Bool_t *falsevalue)
{
   *enabled = fbEnabled;
   *lastresult = fbLastResult;
   *markreset = fbMarkReset;
   *result = fbResult;
   *truevalue = fbTrue;
   *falsevalue = fbFalse;
}

void TGo4Condition::SetFlags(Bool_t enabled, Bool_t lastresult, Bool_t markreset, Bool_t result, Bool_t truevalue,
                             Bool_t falsevalue)
{
   fbEnabled = enabled;
   fbLastResult = lastresult;
   fbMarkReset = markreset;
   fbResult = result;
   fbTrue = truevalue;
   fbFalse = falsevalue;
}

void TGo4Condition::Disable(Bool_t result)
{
   fbEnabled = kFALSE;
   fbResult = result;
}

void TGo4Condition::Enable()
{
   fbEnabled=kTRUE;
}

void TGo4Condition::SetHistogram(const char *name)
{
   if (!name || (*name == 0)) {
      fxHistoName = "";
      fbHistogramLink = false;
   } else {
      fxHistoName = name;
      fbHistogramLink = true;
   }
}

const char *TGo4Condition::GetLinkedHistogram()
{
   return fxHistoName.Data();
}

void TGo4Condition::SetPainter(TGo4ConditionPainter*)
{
// delete old painter, replace by the new one
// overwritten method in subclass may check if painter is correct type
}


void TGo4Condition::Paint(Option_t* opt)
{
   /////// check for streamed canvas markers that were not Draw()n:
   if (fbStreamedCondition) {
      SetPainted(kTRUE);
      fbStreamedCondition = kFALSE;
   }

   if (!IsPainted())
      return;
   if (!fxPainter)
      fxPainter = CreatePainter();
   // condition subclass may not provide a real painter, then we skip painting:
   if (fxPainter) {
      fxPainter->SetCondition(this); // JAM2016
      fxPainter->PaintCondition(opt);
      fxPainter->PaintLabel(opt);
   }
}

void TGo4Condition::Draw(Option_t* opt)
{
   // std::cout<<"TGo4Condition::Draw of instance:"<<(long) this  << " with visible="<< TGo4Condition::IsVisible()<<
   // std::endl;
   if (TGo4Condition::IsVisible()) {
      if (gPad && !gPad->GetListOfPrimitives()->FindObject(this)) {

         // UnDraw(); // JAM2016: do we need this? for switching condition between different pads...? no!
         AppendPad(opt);
      }
      SetPainted(kTRUE);
   } else {
      // std::cout<<"TGo4Condition::Draw does undraw"<< std::endl;
      UnDraw(opt);
   }
}

void TGo4Condition::UnDraw(Option_t* opt)
{
   SetPainted(kFALSE);
   gROOT->GetListOfCanvases()->RecursiveRemove(this);
   //std::cout<<"TGo4Condition::UnDraw of instance:"<<(long) this  << std::endl;
   if(!fxPainter) fxPainter = CreatePainter();
   // condition subclass may not provide a real painter, then we skip unpainting:
   if(fxPainter) {
      fxPainter->UnPaintCondition(opt);
      //if(strcmp(opt,"keeplabel"))
      fxPainter->UnPaintLabel();
   }
}

void TGo4Condition::Pop()
{
   if(fxPainter) fxPainter->DisplayToFront();
}

TGo4ConditionPainter* TGo4Condition::CreatePainter()
{
   return nullptr;
}

void TGo4Condition::SaveLabelStyle()
{
   TGo4Condition::fgbLABELDRAW = fbLabelDraw;
   TGo4Condition::fgbLIMITSDRAW = fbLimitsDraw;
   TGo4Condition::fgbINTDRAW = fbIntDraw;
   TGo4Condition::fgbXMEANDRAW = fbXMeanDraw;
   TGo4Condition::fgbXRMSDRAW = fbXRMSDraw;
   TGo4Condition::fgbYMEANDRAW = fbYMeanDraw;
   TGo4Condition::fgbYRMSDRAW = fbYRMSDraw;
   TGo4Condition::fgbXMAXDRAW = fbXMaxDraw;
   TGo4Condition::fgbYMAXDRAW = fbYMaxDraw;
   TGo4Condition::fgbCMAXDRAW = fbCMaxDraw;
   TGo4Condition::fgxNUMFORMAT = fxNumFormat;
}

void TGo4Condition::InitLabelStyle()
{
   fbLabelDraw = TGo4Condition::fgbLABELDRAW;
   fbLimitsDraw = TGo4Condition::fgbLIMITSDRAW;
   fbIntDraw = TGo4Condition::fgbINTDRAW;
   fbXMeanDraw = TGo4Condition::fgbXMEANDRAW;
   fbXRMSDraw = TGo4Condition::fgbXRMSDRAW;
   fbYMeanDraw = TGo4Condition::fgbYMEANDRAW;
   fbYRMSDraw = TGo4Condition::fgbYRMSDRAW;
   fbXMaxDraw = TGo4Condition::fgbXMAXDRAW;
   fbYMaxDraw = TGo4Condition::fgbYMAXDRAW;
   fbCMaxDraw = TGo4Condition::fgbCMAXDRAW;
   fxNumFormat = TGo4Condition::fgxNUMFORMAT;
}

void TGo4Condition::SetGlobalStyle(Bool_t LABELDRAW, Bool_t LIMITSDRAW, Bool_t INTDRAW,
                           Bool_t XMEANDRAW, Bool_t YMEANDRAW, Bool_t XRMSDRAW, Bool_t YRMSDRAW,
                           Bool_t XMAXDRAW, Bool_t YMAXDRAW, Bool_t CMAXDRAW, const char *NUMFORMAT)
{
   TGo4Condition::fgbLABELDRAW = LABELDRAW;
   TGo4Condition::fgbLIMITSDRAW = LIMITSDRAW;
   TGo4Condition::fgbINTDRAW = INTDRAW;
   TGo4Condition::fgbXMEANDRAW = XMEANDRAW;
   TGo4Condition::fgbYMEANDRAW = YMEANDRAW;
   TGo4Condition::fgbXRMSDRAW = XRMSDRAW;
   TGo4Condition::fgbYRMSDRAW = YRMSDRAW;
   TGo4Condition::fgbXMAXDRAW = XMAXDRAW;
   TGo4Condition::fgbYMAXDRAW = YMAXDRAW;
   TGo4Condition::fgbCMAXDRAW = CMAXDRAW;
   TGo4Condition::fgxNUMFORMAT = NUMFORMAT;
}

void TGo4Condition::GetGlobalStyle(Bool_t &LABELDRAW, Bool_t &LIMITSDRAW, Bool_t &INTDRAW,
                                   Bool_t &XMEANDRAW, Bool_t &YMEANDRAW, Bool_t &XRMSDRAW, Bool_t &YRMSDRAW,
                                   Bool_t &XMAXDRAW, Bool_t &YMAXDRAW, Bool_t &CMAXDRAW, TString &NUMFORMAT)
{
   LABELDRAW = TGo4Condition::fgbLABELDRAW;
   LIMITSDRAW = TGo4Condition::fgbLIMITSDRAW;
   INTDRAW = TGo4Condition::fgbINTDRAW;
   XMEANDRAW = TGo4Condition::fgbXMEANDRAW;
   YMEANDRAW = TGo4Condition::fgbYMEANDRAW;
   XRMSDRAW = TGo4Condition::fgbXRMSDRAW;
   YRMSDRAW = TGo4Condition::fgbYRMSDRAW;
   XMAXDRAW = TGo4Condition::fgbXMAXDRAW;
   YMAXDRAW = TGo4Condition::fgbYMAXDRAW;
   CMAXDRAW = TGo4Condition::fgbCMAXDRAW;
   NUMFORMAT = TGo4Condition::fgxNUMFORMAT;
}


void TGo4Condition::ResetLabel(Option_t* opt)
{
   if(fxPainter) {
      fxPainter->UnPaintLabel(opt);
      fxPainter->PaintLabel();
   }
}

void TGo4Condition::DeletePainter()
{
   if (fxPainter) {
      delete fxPainter;
      fxPainter = nullptr;
   }
}

const char *TGo4Condition::MakeScript(std::ostream& out, const char *varname, Option_t* opt, const char *arrextraargs)
{
   Bool_t savemacro = opt && strstr(opt,"savemacro");
   Bool_t saveprefix = savemacro;

   const char *subname = strstr(opt, "name:");
   if (subname) { varname = subname + 5; saveprefix = kFALSE; }

   if (saveprefix) {
      out << TString::Format("   %s* %s = (%s*) go4->GetAnalysisCondition(\"%s\",\"%s\");",
                   ClassName(), varname, ClassName(), GetName(), ClassName()) << std::endl;
      out << TString::Format("   if (!%s) {", varname) << std::endl;
      out << TString::Format("      TGo4Log::Error(\"Could not find condition %s of class %s\");", GetName(), ClassName()) << std::endl;
      out << TString::Format("      return;") << std::endl;
      out << TString::Format("   }") << std::endl << std::endl;
      out << TString::Format("   TGo4Log::Info(\"Set condition %s as saved at %s\");",
                         GetName(),TDatime().AsString()) << std::endl << std::endl;
   } else
   if (!savemacro && (!opt || !strstr(opt, "nocreate"))) {
      out << TString::Format("   %s* %s = new %s(\"%s\"%s);", ClassName(), varname, ClassName(), GetName(), (arrextraargs ? arrextraargs : "")) << std::endl << std::endl;
   }

   if (!arrextraargs) {

      Bool_t enabled,last,mark,result,vtrue,vfalse;
      GetFlags(&enabled, &last, &mark, &result, &vtrue, &vfalse);

      out << "   // SetFlags(enabled,last,mark,result,vtrue,vfalse);" << std::endl;

      out << TString::Format("   %s%s->SetFlags(%s, %s, %s, %s, %s, %s);",
                  savemacro ? "if (flags) " : "", varname,
                  enabled ? "kTRUE" : "kFALSE",
                  last ? "kTRUE" : "kFALSE",
                  mark ? "kTRUE" : "kFALSE",
                  result ? "kTRUE" : "kFALSE",
                  vtrue ? "kTRUE" : "kFALSE",
                  vfalse ? "kTRUE" : "kFALSE") << std::endl;

      out << TString::Format("   %s%s->SetCounts(%d, %d);",
                 savemacro ? "if (counters) " : "", varname,
                 TrueCounts(), Counts()) << std::endl;

      if (savemacro)
         out << TString::Format("   if (reset) %s->ResetCounts();", varname) << std::endl;
   }

   return varname;
}
