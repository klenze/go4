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

#include "TGo4HisDrawOptions.h"

#include "TROOT.h"
#include "TPad.h"
#include "TColor.h"
#include "TAttLine.h"
#include "TAttFill.h"
#include "TAttMarker.h"

#include "TGo4Picture.h"
#include "TGo4ViewPanel.h"
#include "TGo4MdiArea.h"

#include <QColorDialog>

TGo4HisDrawOptions::TGo4HisDrawOptions( QWidget* parent, const char *name, Qt::WindowFlags fl )
    : QWidget( parent, fl )
{
   setObjectName( name ? name : "Go4HisDrawOptions");
   setupUi(this);

   QObject::connect(DrawOption, QOverload<int>::of(&QComboBox::activated), this, &TGo4HisDrawOptions::SetDrawOptions);
   QObject::connect(ErrorBars, QOverload<int>::of(&QComboBox::activated), this, &TGo4HisDrawOptions::SetErrorBars);
   QObject::connect(Coordinates, QOverload<int>::of(&QComboBox::activated), this, &TGo4HisDrawOptions::SetCoordinates);
   QObject::connect(XStyle, QOverload<int>::of(&QComboBox::activated), this, &TGo4HisDrawOptions::XaxisStyle);
   QObject::connect(YStyle, QOverload<int>::of(&QComboBox::activated), this, &TGo4HisDrawOptions::YaxisStyle);
   QObject::connect(ZStyle, QOverload<int>::of(&QComboBox::activated), this, &TGo4HisDrawOptions::ZaxisStyle);
   QObject::connect(AutoScaleBox, &QCheckBox::toggled, this, &TGo4HisDrawOptions::SetAutoScale);
   QObject::connect(LineColor, &QPushButton::clicked, this, &TGo4HisDrawOptions::SetLineColor);
   QObject::connect(FillColor, &QPushButton::clicked, this, &TGo4HisDrawOptions::SetFillColor);
   QObject::connect(MarkerColor, &QPushButton::clicked, this, &TGo4HisDrawOptions::SetMarkerColor);


   fbSettingPanelData = true;

   QObject::connect(TGo4MdiArea::Instance(), &TGo4MdiArea::panelSignal, this, &TGo4HisDrawOptions::panelSlot);

   UpdateView(view_Histo1);

   AutoScaleBox->setVisible(false);

   LineColor->setEnabled(false);
   FillColor->setEnabled(false);
   MarkerColor->setEnabled(false);

   fbSettingPanelData = false;
}

void TGo4HisDrawOptions::panelSlot(TGo4ViewPanel* panel, TPad* pad, int signalid)
{
   switch (signalid) {
      case QGo4Widget::panel_Modified:
      case QGo4Widget::panel_Updated:
         break;
      case QGo4Widget::panel_Activated:
      case QGo4Widget::panel_ActiveUpdated: {
         TGo4Picture* padopt = panel->GetPadOptions(pad);
         if (!padopt) break;

         int ndim = padopt->GetFullRangeDim();
         if (ndim <= 0) ndim = 1;

         const char *drawopt = nullptr;
         TObject *obj = panel->GetSelectedObject(pad, &drawopt);

         int viewtype = view_Histo1;

         TString buf(drawopt);
         buf.ToLower();

         if (obj && obj->InheritsFrom("TGraphPolar"))
            viewtype = view_Polar;
         else if (obj && (obj->InheritsFrom("TGraph") || obj->InheritsFrom("TMultiGraph")))
            viewtype = view_Graph;
         else if (ndim==1)
            viewtype = view_Histo1;
         else if (buf.Contains("lego") || buf.Contains("surf"))
            viewtype = view_Histo2ext;
         else
            viewtype = view_Histo2;

         fbSettingPanelData = true;

         if (fiLastView!=viewtype) UpdateView(viewtype, panel->IsWebCanvas());

         int ErrorStyle = 0, CoordStyle = 0, DrawStyle = 0;
         DecodeDrawOption(drawopt, ErrorStyle, CoordStyle, DrawStyle);

         Coordinates->setEnabled(!panel->IsWebCanvas());
         if (panel->IsWebCanvas()) CoordStyle = 0;

         DrawOption->setCurrentIndex(DrawStyle);
         ErrorBars->setCurrentIndex(ErrorStyle);
         Coordinates->setCurrentIndex(CoordStyle);
         XStyle->setCurrentIndex(padopt->GetLogScale(0));
         YStyle->setCurrentIndex(padopt->GetLogScale(1));
         ZStyle->setCurrentIndex(padopt->GetLogScale(2));
         ZStyle->setEnabled(ndim>1);
         AutoScaleBox->setChecked(padopt->IsAutoScale());

         LineColor->setEnabled(dynamic_cast<TAttLine*>(obj) != nullptr);
         FillColor->setEnabled(dynamic_cast<TAttFill*>(obj) != nullptr);
         MarkerColor->setEnabled(dynamic_cast<TAttMarker*>(obj) != nullptr);

         fbSettingPanelData = false;

         break; }
      default:
         break;
   }
}

void TGo4HisDrawOptions::UpdateView(int viewtype, bool webcanvas)
{
   int lines=30;
   ErrorBars->clear();
   DrawOption->clear();
   Coordinates->clear();

   switch(viewtype) {
      case view_Histo1:
         DrawOption->addItem( QIcon(":/icons/h1_t.png"),   " scatter");
         DrawOption->addItem( QIcon(""),           "AH   no axis");
         DrawOption->addItem( QIcon(""),           "*H   stars");
         DrawOption->addItem( QIcon(""),           "L    lines");
         DrawOption->addItem( QIcon(""),           "LF2  lines+fill");
         DrawOption->addItem( QIcon(""),           "C    curve");
         DrawOption->addItem( QIcon(""),           "B    barchart");
         DrawOption->addItem( QIcon(""),           "P    polymarkers");
         DrawOption->addItem( QIcon(""),           "P0   polymarkers");
         DrawOption->addItem( QIcon(""),           "9H   high resol");
         DrawOption->addItem( QIcon(""),           "][   no right");
         DrawOption->addItem( QIcon(""),           "TEXT digits b/w");
         DrawOption->addItem( QIcon(""),           "BAR  barchart");
         DrawOption->addItem( QIcon(":/icons/lego.png"),  " lego  b/w");
         DrawOption->addItem( QIcon(":/icons/lego1.png"), " lego1 shadow");
         DrawOption->addItem( QIcon(":/icons/lego2.png"), " lego2 color");
         DrawOption->addItem( QIcon(":/icons/surf.png"),  " mesh b/w");
         DrawOption->addItem( QIcon(":/icons/surf1.png"), " mesh color");
         DrawOption->addItem( QIcon(":/icons/surf2.png"), " surf c");
         DrawOption->addItem( QIcon(":/icons/surf3.png"), " mesh+contour");
         DrawOption->addItem( QIcon(":/icons/surf4.png"), " gourand");
         DrawOption->addItem( QIcon(":/icons/surf5.png"), " col contour");
         DrawOption->addItem( QIcon(""), "");
         lines=30;
         break;
      case view_Histo2:
      case view_Histo2ext:
         DrawOption->addItem( QIcon(":/icons/h1_t.png"), " scatter");
         DrawOption->addItem( QIcon(":/icons/col.png"),  " pixel c");
         DrawOption->addItem( QIcon(":/icons/cont0.png")," cont0 c");
         DrawOption->addItem( QIcon(":/icons/lego2.png")," lego2 color");
         DrawOption->addItem( QIcon(":/icons/surf2.png")," surf c");
         DrawOption->addItem( QIcon(":/icons/surf1.png")," mesh color");
         DrawOption->addItem( QIcon(":/icons/cont1.png")," cont1 c");
         DrawOption->addItem( QIcon(":/icons/cont1.png")," cont4");
         DrawOption->addItem( QIcon(":/icons/lego1.png")," lego1 shadow");
         DrawOption->addItem( QIcon(":/icons/lego.png"), " lego b/w");
         DrawOption->addItem( QIcon(":/icons/cont2.png")," cont2 dot b/w");
         DrawOption->addItem( QIcon(":/icons/cont3.png")," cont3 b/w");
         DrawOption->addItem( QIcon(":/icons/surf.png"), " mesh b/w");
         DrawOption->addItem( QIcon(":/icons/surf3.png")," mesh+contour");
         DrawOption->addItem( QIcon(":/icons/surf4.png")," gourand");
         DrawOption->addItem( QIcon(":/icons/surf5.png")," col contour");
         DrawOption->addItem( QIcon(""),         "ARR  arrow mode");
         DrawOption->addItem( QIcon(""),         "BOX  boxes");
         DrawOption->addItem( QIcon(""),         "TEXT content");
         lines=30;
#ifndef __NOGO4ASI__
         if (!webcanvas) DrawOption->addItem( QIcon(":/icons/asimage.png"), " ASImage");
         DrawOption->addItem( QIcon(""), "");
#else
         DrawOption->addItem( QIcon(""), "");
#endif
         break;
      case view_Graph:
         DrawOption->addItem("P: default");
         DrawOption->addItem("*: stars");
         DrawOption->addItem("L: line");
         DrawOption->addItem("F: fill");
         DrawOption->addItem("F1: fill 1");
         DrawOption->addItem("F2: fill 2");
         DrawOption->addItem("C: smooth");
         DrawOption->addItem("B: bar");
         DrawOption->addItem("LP: line + mark");
         DrawOption->addItem("L*: line + *");
         DrawOption->addItem("FP: fill + mark");
         DrawOption->addItem("F*: fill + *");
         DrawOption->addItem("CP: smooth + mark");
         DrawOption->addItem("C*: smooth + *");
         DrawOption->addItem("BP: smooth + mark");
         DrawOption->addItem("B*: smooth + *");
         lines=30;
         break;
      case view_Polar:
         DrawOption->addItem(": opt as is");
         DrawOption->addItem("P: polym");
         DrawOption->addItem("F: fill");
         DrawOption->addItem("PF: P+F");
         lines = 10;
         break;
   }

   DrawOption->setMaxVisibleItems(lines);

   switch(viewtype) {
      case view_Histo1:
         ErrorBars->addItem("No Errors");
         ErrorBars->addItem("E: simple");
         ErrorBars->addItem("E1: edges");
         ErrorBars->addItem("E2: rectangles");
         ErrorBars->addItem("E3: fill");
         ErrorBars->addItem("E4: contour");
         break;
      case view_Histo2:
         ErrorBars->addItem("No palette");
         ErrorBars->addItem(QIcon(":/icons/colz.png")," +scale");
         break;
      case view_Histo2ext:
         ErrorBars->addItem("No palette");
         ErrorBars->addItem(QIcon(":/icons/colz.png")," +scale");
         ErrorBars->addItem("    - front");
         ErrorBars->addItem("    - back");
         ErrorBars->addItem("    - fr & bk");
         ErrorBars->addItem(QIcon(":/icons/colz.png")," scale - fr");
         ErrorBars->addItem(QIcon(":/icons/colz.png"), " scale - bk");
         ErrorBars->addItem(QIcon(":/icons/colz.png"), " scale - fr & bk");
         break;
      case view_Graph:
         ErrorBars->addItem("errors as is");
         ErrorBars->addItem("X: no errors");
         ErrorBars->addItem(">: arrow");
         ErrorBars->addItem("|>: full arrow");
         ErrorBars->addItem("2: err opt 2");
         ErrorBars->addItem("3: err opt 3");
         ErrorBars->addItem("4: err opt 4");
         ErrorBars->addItem("[]: asym err");
         break;
      case view_Polar:
         ErrorBars->addItem(": err as is");
         ErrorBars->addItem("E: error bars");
         ErrorBars->addItem("N: no labels");
         ErrorBars->addItem("EN: E+N");
      break;
   }

   switch(viewtype) {
      case view_Histo1:
      case view_Histo2:
      case view_Histo2ext:
         Coordinates->addItem("Cartesian");
         Coordinates->addItem("Polar");
         Coordinates->addItem("Spheric");
         Coordinates->addItem("Rapidity");
         Coordinates->addItem("Cylindric");
         break;
      case view_Graph:
         Coordinates->addItem("A: norm axis");
         Coordinates->addItem("AI: supp. axis");
         Coordinates->addItem("AX+: top");
         Coordinates->addItem("AY+: right");
         Coordinates->addItem("AX+Y+: x & y");
         Coordinates->addItem("A1: ylow = ymin");
         break;
      case view_Polar:
         Coordinates->addItem(": axis as is");
         Coordinates->addItem("A: force axis");
         Coordinates->addItem("O: orthogonal");
         Coordinates->addItem("AO: A+O");
         break;
   }

   fiLastView = viewtype;
}

void TGo4HisDrawOptions::ChangeDrawOptionForCurrentPanel(int kind, int value)
{
   if (fbSettingPanelData) return;

   TGo4ViewPanel* panel = TGo4MdiArea::Instance()->GetActivePanel();
   if (!panel) return;

   TPad* pad = panel->GetActivePad();

   TString buf;
   const char *drawopt = nullptr;

   if ((kind >= 0) && (kind <= 2)) {
      CodeDrawOptions(ErrorBars->currentIndex(), Coordinates->currentIndex(), DrawOption->currentIndex(), buf);
      if (buf.Length()>0) drawopt = buf.Data();
   }

   panel->ChangeDrawOption(kind, value, drawopt);
}

void TGo4HisDrawOptions::SetDrawOptions( int t )
{
   ChangeDrawOptionForCurrentPanel(0, t);
}

void TGo4HisDrawOptions::SetErrorBars( int t )
{
   ChangeDrawOptionForCurrentPanel(1, t);
}

void TGo4HisDrawOptions::SetCoordinates( int t )
{
   ChangeDrawOptionForCurrentPanel(2, t);
}

void TGo4HisDrawOptions::XaxisStyle( int t )
{
   ChangeDrawOptionForCurrentPanel(3, t);
}

void TGo4HisDrawOptions::YaxisStyle( int t )
{
    ChangeDrawOptionForCurrentPanel(4, t);
}

void TGo4HisDrawOptions::ZaxisStyle( int t )
{
   ChangeDrawOptionForCurrentPanel(5, t);
}

void TGo4HisDrawOptions::SetAutoScale( bool on )
{
   ChangeDrawOptionForCurrentPanel(10, on ? 1 : 0);
}

void TGo4HisDrawOptions::DecodeDrawOption(const char *drawopt,
                                          int& HisErrorStyle,
                                          int& HisCoordStyle,
                                          int& HisDrawStyle)
{
   HisDrawStyle = 0;
   HisErrorStyle = 0;
   HisCoordStyle = 0;
   if (fiLastView==view_Histo2) HisDrawStyle = 1;

   if (!drawopt || (*drawopt == 0)) return;

   TString buf(drawopt);
   buf.ToLower();

// check for error options in string:

   if (fiLastView==view_Polar) {
      if (buf.Contains("p") && buf.Contains("f")) HisDrawStyle = 3; else
      if (buf.Contains("f")) HisDrawStyle = 2; else
      if (buf.Contains("p")) HisDrawStyle = 1; else HisDrawStyle = 0;

      if (buf.Contains("a") && buf.Contains("o")) HisCoordStyle = 3; else
      if (buf.Contains("o")) HisCoordStyle = 2; else
      if (buf.Contains("a")) HisCoordStyle = 1; else HisCoordStyle = 0;

      if (buf.Contains("e") && buf.Contains("n")) HisErrorStyle = 3; else
      if (buf.Contains("n")) HisErrorStyle = 2; else
      if (buf.Contains("e")) HisErrorStyle = 1; else HisErrorStyle = 0;

      return;
   }

   if (fiLastView==view_Graph) {

      if (buf.Contains("b") && buf.Contains("*")) HisDrawStyle = 15; else
      if (buf.Contains("b") && buf.Contains("p")) HisDrawStyle = 14; else
      if (buf.Contains("c") && buf.Contains("*")) HisDrawStyle = 13; else
      if (buf.Contains("c") && buf.Contains("p")) HisDrawStyle = 12; else
      if (buf.Contains("f") && buf.Contains("*")) HisDrawStyle = 11; else
      if (buf.Contains("f") && buf.Contains("p")) HisDrawStyle = 10; else
      if (buf.Contains("l") && buf.Contains("*")) HisDrawStyle = 9; else
      if (buf.Contains("l") && buf.Contains("p")) HisDrawStyle = 8; else
      if (buf.Contains("b")) HisDrawStyle = 7; else
      if (buf.Contains("c")) HisDrawStyle = 6; else
      if (buf.Contains("f2")) HisDrawStyle = 5; else
      if (buf.Contains("f1")) HisDrawStyle = 4; else
      if (buf.Contains("f")) HisDrawStyle = 3; else
      if (buf.Contains("l")) HisDrawStyle = 2; else
      if (buf.Contains("*")) HisDrawStyle = 1;

      if (buf.Contains("x+") && buf.Contains("y+")) HisCoordStyle = 4; else
      if (buf.Contains("y+")) HisCoordStyle = 3; else
      if (buf.Contains("x+")) HisCoordStyle = 2; else
      if (buf.Contains("a1")) HisCoordStyle = 5; else
      if (buf.Contains("ai")) HisCoordStyle = 1; else
      if (buf.Contains("a")) HisCoordStyle = 0; else HisCoordStyle = 1;

      if (buf.Contains("x") && (HisCoordStyle!=4) && (HisCoordStyle!=2)) HisErrorStyle = 1; else
      if (buf.Contains("|>")) HisErrorStyle = 3; else
      if (buf.Contains(">")) HisErrorStyle = 2; else
      if (buf.Contains("2") && (HisDrawStyle != 5)) HisErrorStyle = 4; else
      if (buf.Contains("3")) HisErrorStyle = 5; else
      if (buf.Contains("4")) HisErrorStyle = 6; else
      if (buf.Contains("[]")) HisErrorStyle = 7;

      return;
   }

   if (fiLastView==view_Histo1) {
      if(buf.Contains("e1")) HisErrorStyle = 2; else
      if(buf.Contains("e2")) HisErrorStyle = 3; else
      if(buf.Contains("e3")) HisErrorStyle = 4; else
      if(buf.Contains("e4")) HisErrorStyle = 5; else
      if(buf.Contains("e") && !buf.Contains("text") && !buf.Contains("lego") && !buf.Contains("image")) HisErrorStyle = 1;
   } else { // Histo2
      if (buf.Contains("lego") || buf.Contains("surf")) {
         if (buf.Contains("fb") && buf.Contains("bb") && buf.Contains("z")) HisErrorStyle = 7; else
         if (buf.Contains("bb") && buf.Contains("z")) HisErrorStyle = 6; else
         if (buf.Contains("fb") && buf.Contains("z")) HisErrorStyle = 5; else
         if (buf.Contains("fb") && buf.Contains("bb")) HisErrorStyle = 4; else
         if (buf.Contains("bb")) HisErrorStyle = 3; else
         if (buf.Contains("fb")) HisErrorStyle = 2; else
         if (buf.Contains("z")) HisErrorStyle = 1;
      } else
         if (buf.Contains("z")) HisErrorStyle = 1;
   }

   // check coordinate specs in string:
   if(buf.Contains(",pol")) HisCoordStyle = 1; else
   if(buf.Contains(",sph")) HisCoordStyle = 2; else
   if(buf.Contains(",psr")) HisCoordStyle = 3; else
   if(buf.Contains(",cyl")) HisCoordStyle = 4;

   // check draw options itself:

   if (fiLastView==view_Histo1) {
      if(buf.Contains("surf5")) HisDrawStyle = 21; else
      if(buf.Contains("surf4")) HisDrawStyle = 20; else
      if(buf.Contains("surf3")) HisDrawStyle = 19; else
      if(buf.Contains("surf2")) HisDrawStyle = 18; else
      if(buf.Contains("surf1")) HisDrawStyle = 17; else
      if(buf.Contains("surf"))  HisDrawStyle = 16; else
      if(buf.Contains("lego2")) HisDrawStyle = 15; else
      if(buf.Contains("lego1")) HisDrawStyle = 14; else
      if(buf.Contains("lego"))  HisDrawStyle = 13; else
      if(buf.Contains("lego3")) HisDrawStyle = 13; else // from gededitor
      if(buf.Contains("bar"))   HisDrawStyle = 12; else
      if(buf.Contains("text"))  HisDrawStyle = 11; else
      if(buf.Contains("]["))    HisDrawStyle = 10; else
      if(buf.Contains("9h"))    HisDrawStyle =  9; else
      if(buf.Contains("p0"))    HisDrawStyle =  8; else
      if(*drawopt == 'p')       HisDrawStyle =  7; else
      if(*drawopt == 'P')       HisDrawStyle =  7; else
      if(*drawopt == 'b')       HisDrawStyle =  6; else
      if(*drawopt == 'B')       HisDrawStyle =  6; else
      if((*drawopt == 'c') && !buf.Contains("nostack")) HisDrawStyle =  5; else
      if((*drawopt == 'C') && !buf.Contains("nostack")) HisDrawStyle =  5; else
      if(buf.Contains("lf2"))   HisDrawStyle =  4; else
      if(*drawopt == 'l')       HisDrawStyle =  3; else
      if(*drawopt == 'L')       HisDrawStyle =  3; else
      if(buf.Contains("*h"))    HisDrawStyle =  2; else
      if(buf.Contains("ah"))    HisDrawStyle =  1;

      if ((HisDrawStyle >= 16) && ((HisCoordStyle == 0) || (HisCoordStyle == 2)))
         HisCoordStyle = 1;
   } else { // Histo2
      if(buf.Contains("hist"))  HisDrawStyle =  0; else
      if(buf.Contains("col"))   HisDrawStyle =  1; else
      if(buf.Contains("lego3")) HisDrawStyle =  9; else // from gededitor
      if(buf.Contains("lego1")) HisDrawStyle =  8; else
      if(buf.Contains("lego2")) HisDrawStyle =  3; else
      if(buf.Contains("lego"))  HisDrawStyle =  9; else
      if(buf.Contains("cont0")) HisDrawStyle =  2; else
      if(buf.Contains("cont1")) HisDrawStyle =  6; else
      if(buf.Contains("cont2")) HisDrawStyle = 10; else
      if(buf.Contains("cont3")) HisDrawStyle = 11; else
      if(buf.Contains("cont4")) HisDrawStyle =  7; else
      if(buf.Contains("cont"))  HisDrawStyle =  2; else
      if(buf.Contains("surf1")) HisDrawStyle =  5; else
      if(buf.Contains("surf2")) HisDrawStyle =  4; else
      if(buf.Contains("surf3")) HisDrawStyle = 13; else
      if(buf.Contains("surf4")) HisDrawStyle = 14; else
      if(buf.Contains("surf5")) HisDrawStyle = 15; else
      if(buf.Contains("surf"))  HisDrawStyle = 12; else
      if(buf.Contains("arr"))   HisDrawStyle = 16; else
      if(buf.Contains("box"))   HisDrawStyle = 17; else
      if(buf.Contains("text"))  HisDrawStyle = 18; else
      if(buf.Contains("asimage")) HisDrawStyle = 19;
   }
   //std::cout << *drawopt << " Decode " << buf << " Error " << HisErrorStyle
   //     << " Coord " << HisCoordStyle  << " Draw " <<HisDrawStyle<< std::endl;
}

void TGo4HisDrawOptions::CodeDrawOptions(int HisErrorStyle,
                                         int HisCoordStyle,
                                         int HisDrawStyle,
                                         TString& buf)
{
   buf = "";

   if (fiLastView==view_Polar) {
      switch (HisDrawStyle) {
         case 0: buf = ""; break;
         case 1: buf = "P"; break;
         case 2: buf = "F"; break;
         case 3: buf = "PF"; break;
      }

      switch (HisErrorStyle) {
         case 0: break;
         case 1: buf.Append("E"); break;
         case 2: buf.Append("N"); break;
         case 3: buf.Append("EN"); break;
      }

      switch (HisCoordStyle) {
         case 0: break;
         case 1: buf.Append("A"); break;
         case 2: buf.Append("O"); break;
         case 3: buf.Append("AO"); break;
      }
      return;
   }

   if (fiLastView == view_Graph) {

      switch (HisDrawStyle) {
         case 0: buf = "P"; break;
         case 1: buf = "*"; break;
         case 2: buf = "L"; break;
         case 3: buf = "F"; break;
         case 4: buf = "F1"; break;
         case 5: buf = "F2"; break;
         case 6: buf = "C"; break;
         case 7: buf = "B"; break;
         case 8: buf = "LP"; break;
         case 9: buf = "L*"; break;
         case 10: buf = "FP"; break;
         case 11: buf = "F*"; break;
         case 12: buf = "CP"; break;
         case 13: buf = "C*"; break;
         case 14: buf = "BP"; break;
         case 15: buf = "B*"; break;
         default: buf = "P";
      }

      switch (HisErrorStyle) {
         case 0: break;
         case 1: buf.Append("X"); break;
         case 2: buf.Append(">"); break;
         case 3: buf.Append("|>"); break;
         case 4: buf.Append("2"); break;
         case 5: buf.Append("3"); break;
         case 6: buf.Append("4"); break;
         case 7: buf.Append("[]"); break;
      }

      switch (HisCoordStyle) {
         case 0: buf.Append("A"); break;
         case 1: buf.Append("AI"); break;
         case 2: buf.Append("AX+"); break;
         case 3: buf.Append("AY+"); break;
         case 4: buf.Append("AX+Y+"); break;
         case 5: buf.Append("A1"); break;
      }

      return;
   }

   if (fiLastView==view_Histo1) {

    switch (HisDrawStyle) {
       case  0: buf="";      break;
       case  1: buf="ah";    break;
       case  2: buf="*h";    break;
       case  3: buf="l";     break;
       case  4: buf="lf2";   break;
       case  5: buf="c";     break;
       case  6: buf="b";     break;
       case  7: buf="p";     break;
       case  8: buf="p0";    break;
       case  9: buf="9h";    break;
       case 10: buf="][";    break;
       case 11: buf="text";  break;
       case 12: buf="bar";   break;
       case 13: buf="lego3"; break; // for geditor, "lego"
       case 14: buf="lego1"; break;
       case 15: buf="lego2"; break;
       case 16: buf="surf";  break;
       case 17: buf="surf1"; break;
       case 18: buf="surf2"; break;
       case 19: buf="surf3"; break;
       case 20: buf="surf4"; break;
       case 21: buf="surf5"; break;
    }
    if ((HisDrawStyle >= 16) && ((HisCoordStyle == 0) || (HisCoordStyle == 2)))
       HisCoordStyle = 1;
   } else // Histo2

      switch (HisDrawStyle) {
      case 0: buf = "hist"; break;
      case 1: buf = "col"; break;
      case 9: buf = "lego3"; break;
      case 8: buf = "lego1"; break;
      case 3: buf = "lego2"; break;
      case 2: buf = "cont0"; break;
      case 6: buf = "cont1"; break;
      case 10: buf = "cont2"; break;
      case 11: buf = "cont3"; break;
      case 7: buf = "cont4"; break;
      case 12: buf = "surf"; break;
      case 5: buf = "surf1"; break;
      case 4: buf = "surf2"; break;
      case 13: buf = "surf3"; break;
      case 14: buf = "surf4"; break;
      case 15: buf = "surf5"; break;
      case 16: buf = "arr"; break;
      case 17: buf = "box"; break;
      case 18: buf = "text"; break;
#ifndef __NOGO4ASI__
       case 19: buf="asimage"; break;
#else
       case 19: buf=""; break;
#endif
    }

    switch (HisCoordStyle) {
       case 0:  break;
       case 1:  buf.Append(",pol"); break;
       case 2:  buf.Append(",sph"); break;
       case 3:  buf.Append(",psr"); break;
       case 4:  buf.Append(",cyl"); break;
    }

    if (fiLastView==view_Histo1)
       switch (HisErrorStyle) {
          case 0:  break;
          case 1:  buf.Append("E"); break;
          case 2:  buf.Append("E1"); break;
          case 3:  buf.Append("E2"); break;
          case 4:  buf.Append("E3"); break;
          case 5:  buf.Append("E4"); break;
       }
    else
       switch (HisErrorStyle) {
          case 0:  break;
          case 1:  buf.Append("Z"); break;
          case 2:  buf.Append("FB"); break;
          case 3:  buf.Append("BB"); break;
          case 4:  buf.Append("BBFB"); break;
          case 5:  buf.Append("ZFB"); break;
          case 6:  buf.Append("ZBB"); break;
          case 7:  buf.Append("ZBBFB"); break;
       }
}


void TGo4HisDrawOptions::SetLineColor()
{
   ChangeColor(0);
}

void TGo4HisDrawOptions::SetFillColor()
{
   ChangeColor(1);
}

void TGo4HisDrawOptions::SetMarkerColor()
{
   ChangeColor(2);
}

void TGo4HisDrawOptions::ChangeColor(int kind)
{
   TGo4ViewPanel* panel = TGo4MdiArea::Instance()->GetActivePanel();
   if (!panel) return;

   TPad* pad = panel->GetActivePad();
   TObject *obj = panel->GetSelectedObject(pad, 0);

   Color_t icol = 0;

   switch (kind) {
      case 0: {
        TAttLine* att = dynamic_cast<TAttLine*> (obj);
        if (!att) return;
        icol = att->GetLineColor();
        break;
      }
      case 1: {
        TAttFill* att = dynamic_cast<TAttFill*> (obj);
        if (!att) return;
        icol = att->GetFillColor();
        break;
      }
      case 2: {
        TAttMarker* att = dynamic_cast<TAttMarker*> (obj);
        if (!att) return;
        icol = att->GetMarkerColor();
        break;
      }
   }

   TColor* rcol = gROOT->GetColor(icol);

   QColor qcol;

   if (rcol)
      qcol.setRgb(rcol->GetRed(), rcol->GetGreen(), rcol->GetBlue());

   QColor c = QColorDialog::getColor(qcol);
   if (!c.isValid()) return;

   icol = TColor::GetColor(c.red(), c.green(), c.blue());

   switch (kind) {
      case 0: {
        TAttLine* att = dynamic_cast<TAttLine*> (obj);
        if (att) att->SetLineColor(icol);
        break;
      }
      case 1: {
        TAttFill* att = dynamic_cast<TAttFill*> (obj);
        if (att) att->SetFillColor(icol);
        break;
      }
      case 2: {
        TAttMarker* att = dynamic_cast<TAttMarker*> (obj);
        if (att) att->SetMarkerColor(icol);
        break;
      }
   }

   panel->MarkPadModified(pad);

   panel->ShootRepaintTimer(pad);

   panel->ActivateInGedEditor(obj);
}
