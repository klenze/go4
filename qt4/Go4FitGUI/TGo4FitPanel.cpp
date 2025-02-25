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

#include "TGo4FitPanel.h"

#include <iostream>

#include <QtCore/QSignalMapper>

#include <QMenu>
#include <QMenuBar>
#include <QDropEvent>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>

#include "TStyle.h"
#include "TObjArray.h"
#include "TString.h"
#include "TClass.h"
#include "TMath.h"
#include "Buttons.h"
#include "TH1.h"
#include "TCutG.h"
#include "TCanvas.h"
#include "TPaveStats.h"
#include "TLegend.h"
#include "TFile.h"

#include "TGo4LockGuard.h"
#include "TGo4Slot.h"
#include "TGo4Fitter.h"
#include "TGo4FitterAction.h"
#include "TGo4FitterConfig.h"
#include "TGo4FitterOutput.h"
#include "TGo4FitMinuit.h"
#include "TGo4FitAmplEstimation.h"
#include "TGo4FitPeakFinder.h"
#include "TGo4FitParsList.h"
#include "TGo4FitParameter.h"
#include "TGo4FitModelPolynom.h"
#include "TGo4FitModelGauss1.h"
#include "TGo4FitModelGauss2.h"
#include "TGo4FitModelGaussN.h"
#include "TGo4FitModelFormula.h"
#include "TGo4FitModelFunction.h"
#include "TGo4FitModelFromData.h"
#include "TGo4FitDataHistogram.h"
#include "TGo4FitDataGraph.h"
#include "TGo4FitDataProfile.h"
#include "TGo4FitDataRidge.h"
#include "TGo4FitDependency.h"
#include "TGo4FitMinuitResult.h"
#include "TGo4FitLinearTrans.h"
#include "TGo4FitMatrixTrans.h"
#include "TGo4Log.h"
#include "TGo4ViewPanel.h"
#include "TGo4MdiArea.h"
#include "TGo4QSettings.h"
#include "QFitItem.h"
#include "QFitRangeWidget.h"
#include "QFitRangeCutWidget.h"
#include "QFitParWidget.h"
#include "QFitDependencyWidget.h"
#include "QFitParCfgWidget.h"
#include "QFitDataWidget.h"
#include "QFitModelPolynomWidget.h"
#include "QFitModelGauss1Widget.h"
#include "QFitModelGauss2Widget.h"
#include "QFitModelGaussNWidget.h"
#include "QFitModelFormulaWidget.h"
#include "QFitModelFunctionWidget.h"
#include "QFitAmplEstimWidget.h"
#include "QFitPeakFinderWidget.h"
#include "QFitOutputActionWidget.h"
#include "QFitMinuitWidget.h"
#include "QFitMinuitResWidget.h"
#include "QFitSlotWidget.h"
#include "QFitterWidget.h"
#include "QFitTableWidget.h"
#include "QFitPrintWidget.h"
#include "QFitLinearTransWidget.h"
#include "QFitMatrixTransWidget.h"
#include "TGo4FitGuiTypes.h"
#include "TGo4FitGuiArrow.h"

TGo4FitPanel::TGo4FitPanel(QWidget *parent, const char *name) : QGo4Widget(parent, name)
{
   setupUi(this);

   QObject::connect(FitList, &QTreeWidget::customContextMenuRequested, this,
                    &TGo4FitPanel::FitList_customContextMenuRequested);
   QObject::connect(FitList, &QTreeWidget::currentItemChanged, this, &TGo4FitPanel::FitList_currentItemChanged);
   QObject::connect(Wiz_AddDataBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_AddDataBtn_clicked);
   QObject::connect(Wiz_DelDataBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_DelDataBtn_clicked);
   QObject::connect(Wiz_AddModelBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_AddModelBtn_clicked);
   QObject::connect(Wiz_DelModelBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_DelModelBtn_clicked);
   QObject::connect(Wiz_CloneModelBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_CloneModelBtn_clicked);
   QObject::connect(Wiz_ShowAllMod, &QCheckBox::toggled, this, &TGo4FitPanel::Wiz_ShowAllMod_toggled);
   QObject::connect(Wiz_FitFuncCmb, QOverload<int>::of(&QComboBox::activated), this,
                    &TGo4FitPanel::Wiz_FitFuncCmb_activated);
   QObject::connect(Wiz_FitNameEdt, &QGo4LineEdit::textChanged, this, &TGo4FitPanel::Wiz_FitNameEdt_textChanged);
   QObject::connect(Wiz_ParTable, &QTableWidget::cellChanged, this, &TGo4FitPanel::Wiz_ParTable_valueChanged);
   QObject::connect(Wiz_ModelList, &QListWidget::itemDoubleClicked, this, &TGo4FitPanel::Wiz_ModelList_doubleClicked);
   QObject::connect(Wiz_ModelList, &QListWidget::itemChanged, this, &TGo4FitPanel::Wiz_ModelList_itemChanged);
   QObject::connect(Wiz_RebuildDataBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_RebuildDataBtn_clicked);
   QObject::connect(Wiz_DataBufChk, &QCheckBox::toggled, this, &TGo4FitPanel::Wiz_DataBufChk_toggled);
   QObject::connect(Wiz_UseAmplEstimChk, &QCheckBox::toggled, this, &TGo4FitPanel::Wiz_UseAmplEstimChk_toggled);
   QObject::connect(Wiz_MigradIterSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                    &TGo4FitPanel::Wiz_MigradIterSpin_valueChanged);
   QObject::connect(Wiz_DataSlotsTable, &QTableWidget::customContextMenuRequested, this,
                    &TGo4FitPanel::Wiz_DataSlotsTable_contextMenuRequested);
   QObject::connect(Wiz_DataUseRangeBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_DataUseRangeBtn_clicked);
   QObject::connect(Wiz_DataClearRangesBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_DataClearRangesBtn_clicked);
   QObject::connect(Wiz_DrawDataBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_DrawDataBtn_clicked);
   QObject::connect(MainAttachBtn, &QPushButton::clicked, this, &TGo4FitPanel::MainAttachBtn_clicked);
   QObject::connect(MainFitBtn, &QPushButton::clicked, this, &TGo4FitPanel::MainFitBtn_clicked);
   QObject::connect(MainDrawBtn, &QPushButton::clicked, this, &TGo4FitPanel::MainDrawBtn_clicked);
   QObject::connect(MainFindBtn, &QPushButton::clicked, this, &TGo4FitPanel::MainFindBtn_clicked);
   QObject::connect(Wiz_DataList, &QListWidget::itemDoubleClicked, this, &TGo4FitPanel::Wiz_DataList_doubleClicked);
   QObject::connect(Wiz_DataList, &QListWidget::itemClicked, this, &TGo4FitPanel::Wiz_DataListSelect);
   QObject::connect(Wiz_DataList, &QListWidget::itemChanged, this, &TGo4FitPanel::Wiz_DataListSelect);
   QObject::connect(Wiz_DataList, &QListWidget::itemPressed, this, &TGo4FitPanel::Wiz_DataListSelect);
   QObject::connect(Wiz_ModelList, &QListWidget::currentItemChanged, this, &TGo4FitPanel::Wiz_ModelListSelect);
   QObject::connect(MainParsBtn, &QPushButton::clicked, this, &TGo4FitPanel::MainParsBtn_clicked);
   QObject::connect(LineParsChk, &QCheckBox::toggled, this, &TGo4FitPanel::LineParsChk_toggled);
   QObject::connect(ParsTable, &QTableWidget::cellChanged, this, &TGo4FitPanel::ParsTable_valueChanged);
   QObject::connect(Wiz_ModelBufChk, &QCheckBox::toggled, this, &TGo4FitPanel::Wiz_ModelBufChk_toggled);
   QObject::connect(Wiz_PFSetupBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_PFSetupBtn_clicked);
   QObject::connect(Wiz_PFUsePolynChk, &QCheckBox::toggled, this, &TGo4FitPanel::Wiz_PFUsePolynChk_toggled);
   QObject::connect(Wiz_PFPolynSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                    &TGo4FitPanel::Wiz_PFPolynSpin_valueChanged);
   QObject::connect(PF_MinWidthEdt, &QGo4LineEdit::textChanged, this, &TGo4FitPanel::PF_MinWidthEdt_textChanged);
   QObject::connect(PF_MaxWidthEdt, &QGo4LineEdit::textChanged, this, &TGo4FitPanel::PF_MaxWidthEdt_textChanged);
   QObject::connect(PF_AmplSlider, &QSlider::valueChanged, this, &TGo4FitPanel::PF_AmplSlider_valueChanged);
   QObject::connect(PF_AmplSlider, &QSlider::sliderReleased, this, &TGo4FitPanel::PF_AmplSlider_sliderReleased);
   QObject::connect(PF_WidthEdit, &QGo4LineEdit::textChanged, this, &TGo4FitPanel::PF_WidthEdit_textChanged);
   QObject::connect(PF_RelNoiseSlider, &QSlider::valueChanged, this, &TGo4FitPanel::PF_RelNoiseSlider_valueChanged);
   QObject::connect(PF_RelNoiseSlider, &QSlider::sliderReleased, this, &TGo4FitPanel::PF_RelNoiseSlider_sliderReleased);
   QObject::connect(PF_MinNoiseEdit, &QGo4LineEdit::textChanged, this, &TGo4FitPanel::PF_MinNoiseEdit_textChanged);
   QObject::connect(PF_SumUpSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                    &TGo4FitPanel::PF_SumUpSpin_valueChanged);
   QObject::connect(Smp_PolynomBtn, &QPushButton::clicked, this, &TGo4FitPanel::Smp_PolynomBtn_clicked);
   QObject::connect(Smp_GaussianBtn, &QPushButton::clicked, this, &TGo4FitPanel::Smp_GaussianBtn_clicked);
   QObject::connect(Smp_LorenzBtn, &QPushButton::clicked, this, &TGo4FitPanel::Smp_LorenzBtn_clicked);
   QObject::connect(Smp_ExponentBtn, &QPushButton::clicked, this, &TGo4FitPanel::Smp_ExponentBtn_clicked);
   QObject::connect(Smp_ClearBtn, &QPushButton::clicked, this, &TGo4FitPanel::Smp_ClearBtn_clicked);
   QObject::connect(Wiz_MinSetupBtn, &QPushButton::clicked, this, &TGo4FitPanel::Wiz_MinSetupBtn_clicked);
   QObject::connect(FindersTab, &QTabWidget::currentChanged, this, &TGo4FitPanel::FindersTab_currentChanged);
   QObject::connect(Wiz_BackgroundChk, &QCheckBox::toggled, this, &TGo4FitPanel::Wiz_BackgroundChk_toggled);
   QObject::connect(PF_MinWidthEdt, &QGo4LineEdit::returnPressed, this, &TGo4FitPanel::PF_MinWidthEdt_returnPressed);
   QObject::connect(PF_MaxWidthEdt, &QGo4LineEdit::returnPressed, this, &TGo4FitPanel::PF_MaxWidthEdt_returnPressed);
   QObject::connect(PF_WidthEdit, &QGo4LineEdit::returnPressed, this, &TGo4FitPanel::PF_WidthEdit_returnPressed);
   QObject::connect(PF_MinNoiseEdit, &QGo4LineEdit::returnPressed, this, &TGo4FitPanel::PF_MinNoiseEdit_returnPressed);

   fbFreezeMode = false;
   fiPanelMode = FitGui::pm_Wizard;
   fbParsWidgetShown = false;

   fbNeedConfirmation = go4sett->getBool("/FitPanel/NeedConfirmation", false);
   fbShowPrimitives = go4sett->getBool("/FitPanel/ShowPrimitives", true);
   fbSaveWithReferences = go4sett->getBool("/FitPanel/SaveWithReferences", false);
   fbUseCurrentRange = go4sett->getBool("/FitPanel/UseCurrentRange", true);
   fbDrawModels = go4sett->getBool("/FitPanel/DrawModels", true);
   fbDrawComponents = go4sett->getBool("/FitPanel/DrawComponents", false);
   fbUseSamePanelForDraw = go4sett->getBool("/FitPanel/UseSamePanelForDraw", true);
   fbDrawBackground = go4sett->getBool("/FitPanel/DrawBackground", false);
   fbDrawInfoOnPad = go4sett->getBool("/FitPanel/DrawInfoOnPad", false);
   fbRecalculateGaussWidth = go4sett->getBool("/FitPanel/RecalculateGaussWidth", true);
   fiIntegralMode = go4sett->getInt("/FitPanel/IntegralMode", 0);
   fiBuffersUsage = go4sett->getInt("/FitPanel/BuffersUsage", 1);

   FitList->setContextMenuPolicy(Qt::CustomContextMenu);

   // fit options part
   fxWizDataName = "";
   fxWizModelName = "";
   fbWizShowAllModels = true;
   fiWizPageIndex = 0;
   fbUseAmplEstim = false;
   fiNumMigradIter = 0;

   fxCurrentItem = nullptr;
   fxCurrentItemWidget = nullptr;

   fxActivePanel = nullptr;
   fxActivePad = nullptr;

   fxDrawNewPanel = nullptr;
   fbDrawPanelCreation = false;

   QTableWidgetItem *item = new QTableWidgetItem;
   item->setFlags(item->flags() & ~Qt::ItemIsEditable);
   Wiz_DataSlotsTable->setItemPrototype(item);
   Wiz_DataSlotsTable->setContextMenuPolicy(Qt::CustomContextMenu);

   QObject::connect(TGo4MdiArea::Instance(), &TGo4MdiArea::panelSignal, this, &TGo4FitPanel::panelSlot);

   MenuBar = new QMenuBar(MenuFrame);
   MenuBar->setMinimumWidth(100);
   // MenuBar->setFrameShape(QMenuBar::NoFrame);
   MenuBar->setNativeMenuBar(kFALSE); // disable putting this to screen menu. for MAC style WMs

   FitterMap = new QSignalMapper(this);
   FitterMenu = MenuBar->addMenu("&Fitter");
   QObject::connect(FitterMenu, &QMenu::aboutToShow, this, &TGo4FitPanel::AboutToShowFitterMenu);

   ViewMap = new QSignalMapper(this);
   ViewMenu = MenuBar->addMenu("&Tools");
   QObject::connect(ViewMenu, &QMenu::aboutToShow, this, &TGo4FitPanel::AboutToShowViewMenu);

   SettMap = new QSignalMapper(this);
   SettMenu = MenuBar->addMenu("&Settings");
   QObject::connect(SettMenu, &QMenu::aboutToShow, this, &TGo4FitPanel::AboutToShowSettMenu);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
   auto signal = QOverload<int>::of(&QSignalMapper::mapped);
#else
   auto signal = &QSignalMapper::mappedInt;
#endif

   QObject::connect(FitterMap, signal, this, &TGo4FitPanel::FitterMenuItemSelected);
   QObject::connect(ViewMap, signal, this, &TGo4FitPanel::ChangeViewType);
   QObject::connect(SettMap, signal, this, &TGo4FitPanel::ChangeSettings);

   AddIdAction(SettMenu, SettMap, "&Confirmation", 1);
   AddIdAction(SettMenu, SettMap, "&Show primitives", 2);
   AddIdAction(SettMenu, SettMap, "&Freeze mode", 3);
   AddIdAction(SettMenu, SettMap, "&Save with objects", 4);

   SettMenu->addSeparator();

   AddIdAction(SettMenu, SettMap, "&Use current range", 10);
   AddIdAction(SettMenu, SettMap, "&Draw model", 11);
   AddIdAction(SettMenu, SettMap, "Draw &background", 14);
   AddIdAction(SettMenu, SettMap, "Dra&w components", 12);
   AddIdAction(SettMenu, SettMap, "Draw on same &pad", 13);
   AddIdAction(SettMenu, SettMap, "Draw &info on pad", 15);

   SettMenu->addSeparator();

   AddIdAction(SettMenu, SettMap, "&No &integral", 17);
   AddIdAction(SettMenu, SettMap, "&Counts", 18);
   AddIdAction(SettMenu, SettMap, "&Integral", 19);
   AddIdAction(SettMenu, SettMap, "&Gauss integral", 20);

   AddIdAction(SettMenu, SettMap, "Recalculate gauss &widths", 16);

   SettMenu->addSeparator();

   AddIdAction(SettMenu, SettMap, "&Do not use buffers", 21);
   AddIdAction(SettMenu, SettMap, "&Only for data", 22);
   AddIdAction(SettMenu, SettMap, "&For data and models", 23);
   AddIdAction(SettMenu, SettMap, "&Individual settings", 24);

   ItemMenu = nullptr;
   CurrFitItem = nullptr;

   Wiz_DataSlotsTable->horizontalHeader()->setStretchLastSection(true);
   Wiz_ParTable->horizontalHeader()->setStretchLastSection(true);

   fxWizPars = new TObjArray();
   fxWizSlots = new TObjArray();
   fxParsTableList = new TObjArray();

   fiPaintMode = 0;

   gridLayout->setContentsMargins(3, 3, 3, 3);
   gridLayout->setSpacing(1);
   UpdateActivePage();

   MenuBar->setMinimumHeight(MenuFrame->height());
}

TGo4FitPanel::~TGo4FitPanel()
{
   // discards messages from deleted items
   fbFillingWidget = true;

   fxCurrentItem = nullptr;
   FitList->clear();

   delete fxParsTableList;
   delete fxWizSlots;
   delete fxWizPars;

   RemovePrimitives();
   RemoveDrawObjects();
   CloseDrawPanel();

   ClearObjectReferenceInSlots();
}

// *********************** overloaded methods of QGo4Widget **************************

bool TGo4FitPanel::IsAcceptDrag(const char *itemname, TClass *cl, int kind)
{
   if (!cl)
      return false;

   return (cl->InheritsFrom(TGo4Fitter::Class())) || (cl->InheritsFrom(TH1::Class())) ||
          (cl->InheritsFrom(TGraph::Class()));
}

void TGo4FitPanel::DropOnPanel(QDropEvent *event, const char *itemname, TClass *cl, int kind)
{
   if (!cl)
      return;

   if (cl->InheritsFrom(TGo4Fitter::Class())) {
      WorkWithFitter(itemname, nullptr, nullptr);
      return;
   }

   if (!event)
      return;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
   QPoint pos = event->pos();
#else
   QPoint pos = event->position().toPoint();
#endif

   QWidget *w = childAt(pos);

   if (w == Wiz_DataSlotsTable) {
      QPoint pnt = Wiz_DataSlotsTable->mapFrom(this, pos);
      QTableWidgetItem *item = Wiz_DataSlotsTable->itemAt(pnt);

      int nrow = item ? item->row() : -1;

      if ((nrow < 0) || (nrow > fxWizSlots->GetLast()))
         return;

      TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(fxWizSlots->At(nrow));

      if (!slot)
         return;
      if (!slot->IsSuitableClass(cl))
         return;

      CreateFitSlotLink(slot, itemname);

      UpdateActivePage();
   } else if (w == (QWidget *)FitList->viewport()) {
      QPoint pnt = FitList->viewport()->mapFrom(this, pos);
      QFitItem *item = dynamic_cast<QFitItem *>(FitList->itemAt(pnt));
      if (!item || (item->ObjectType() != FitGui::ot_slot))
         return;

      TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(item->Object());
      if (!slot)
         return;
      if (!slot->IsSuitableClass(cl))
         return;

      CreateFitSlotLink(slot, itemname);

      UpdateActivePage();
   }
}

void TGo4FitPanel::linkedObjectUpdated(const char *linkname, TObject *obj)
{
   UpdateObjectReferenceInSlots();

   UpdateActivePage();
}

void TGo4FitPanel::RemoveFitterLink()
{
   RemoveLink("Fitter");
}

void TGo4FitPanel::linkedObjectRemoved(const char *linkname)
{
   if (strcmp(linkname, "Fitter") == 0) {
      RemoveAllLinks();
      fxActivePanel = nullptr;
      fxActivePad = nullptr;
   } else {
      UpdateObjectReferenceInSlots();
   }

   UpdateActivePage();
}

void TGo4FitPanel::linkedRemoved(TGo4Slot *slot, TObject *obj) {}

void TGo4FitPanel::WorkWithFitter(const char *itemname, TGo4ViewPanel *panel, TPad *pad)
{
   RemovePrimitives();
   RemoveDrawObjects();
   CloseDrawPanel();
   ClearObjectReferenceInSlots();

   fxActivePanel = panel;
   fxActivePad = pad;

   RemoveFitterLink();
   AddLink(itemname, "Fitter");

   // just take object
   GetLinked("Fitter", 2);

   UpdateObjectReferenceInSlots();

   UpdateActivePage();
}

void TGo4FitPanel::WorkWithFitterInSlot(TGo4Slot *slot)
{
   RemovePrimitives();
   RemoveDrawObjects();
   CloseDrawPanel();
   ClearObjectReferenceInSlots();

   fxActivePanel = nullptr;
   fxActivePad = nullptr;

   RemoveFitterLink();
   AddLink(slot, "Fitter");

   GetLinked("Fitter");

   UpdateObjectReferenceInSlots();

   UpdateActivePage();
}

// **************************** interface to MainGUI ************************

void TGo4FitPanel::panelSlot(TGo4ViewPanel *panel, TPad *pad, int signalid)
{
   switch (signalid) {
   case panel_Activated: {
      if (!panel || !pad)
         return;

      if (fbDrawPanelCreation)
         return;
      if ((panel == fxDrawNewPanel) || fbDrawPanelCreation)
         return;

      bool update = fbFreezeMode;

      if (!update && GetFitter() && (GetFitter()->GetNumData() > 1))
         update = true;

      if (WorkingWithPanel() && (panel == ActivePanel()) && (pad == ActivePad()))
         update = true;

      if (update) {
         UpdateObjectReferenceInSlots();
         if (fiPanelMode == FitGui::pm_Expert)
            UpdateItemsOfType(FitGui::ot_slot);
         if (fiPanelMode == FitGui::pm_Wizard)
            UpdateWizStackWidget();
         return;
      }

      RemovePrimitives();
      RemoveDrawObjects();
      CloseDrawPanel();
      ClearObjectReferenceInSlots();

      RemoveFitterLink();

      fxActivePanel = panel;
      fxActivePad = pad;

      if (panel && pad) {
         TGo4Slot *slot = panel->GetDrawObjectSlot(pad, "::Fitter");
         AddLink(slot, "Fitter");
      }

      UpdateObjectReferenceInSlots();

      UpdateActivePage();

      break;
   }

   case panel_Modified: {
      if (!WorkingWithPanel() || (panel != ActivePanel()))
         return;
      RemovePrimitives();
      CloseDrawPanel();
      UpdateObjectReferenceInSlots();
      if (fiPanelMode == FitGui::pm_Wizard)
         UpdateWizStackWidget();
      if (fiPanelMode == FitGui::pm_Expert)
         UpdateItemsOfType(FitGui::ot_slot);
      break;
   }

   case panel_Deleted: {
      if (!panel)
         return;
      if (panel == fxDrawNewPanel) {
         fxDrawNewPanel = nullptr;
         return;
      }
      if (WorkingWithPanel() && (panel == ActivePanel())) {
         ClearObjectReferenceInSlots();
         RemoveFitterLink();
         fxActivePanel = nullptr;
         fxActivePad = nullptr;
      }

      UpdateActivePage();
      break;
   }
   }
}

// ************************ functionality of fitslots classes *****************************

bool TGo4FitPanel::WorkingWithPanel()
{
   return fxActivePanel && fxActivePad;
}

bool TGo4FitPanel::WorkingWithOnlyPad()
{
   return WorkingWithPanel() ? !ActivePanel()->IsPadHasSubPads(ActivePad()) : false;
}

TGo4ViewPanel *TGo4FitPanel::ActivePanel()
{
   return fxActivePanel;
}

TPad *TGo4FitPanel::ActivePad()
{
   return fxActivePad;
}

TGo4Fitter *TGo4FitPanel::GetFitter()
{
   return GetLinkedCast<TGo4Fitter>("Fitter");
}

void TGo4FitPanel::SetFitter(TGo4Fitter *fitter)
{
   ClearObjectReferenceInSlots();
   RemoveFitterLink();
   if (!WorkingWithPanel()) {
      if (fitter) {
         QString itemname = SaveObjectInMemory("FitPanel", fitter);
         AddLink(itemname.toLatin1().constData(), "Fitter");
      }
   } else {
      ActivePanel()->DeleteDrawObject(ActivePad(), "::Fitter");
      TGo4Slot *slot =
         ActivePanel()->AddDrawObject(ActivePad(), TGo4ViewPanel::kind_Fitter, "::Fitter", fitter, true, 0);
      if (slot)
         AddLink(slot, "Fitter");
   }
   UpdateObjectReferenceInSlots();
   fbParsWidgetShown = false;
}

TGo4Fitter *TGo4FitPanel::CloneFitter()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return nullptr;

   fitter->SetSaveFlagForObjects(fbSaveWithReferences);

   return dynamic_cast<TGo4Fitter *>(fitter->Clone());
}

void TGo4FitPanel::Fitter_New()
{
   if (WorkingWithPanel()) {
      ClearObjectReferenceInSlots();
      fxActivePanel = nullptr;
      fxActivePad = nullptr;
   }

   TGo4Fitter *fitter = new TGo4Fitter("Fitter", "Fitter object");
   SetFitter(fitter);

   UpdateActivePage();
}

void TGo4FitPanel::Fitter_NewForActivePad(bool overwrite)
{
   TGo4ViewPanel *panel = LastActivePanel();
   if (!panel || (panel == fxDrawNewPanel))
      return;
   TPad *pad = panel->GetActivePad();

   ClearObjectReferenceInSlots();

   RemoveFitterLink();

   fxActivePanel = panel;
   fxActivePad = pad;

   if (panel && pad) {
      TGo4Slot *fitterslot = panel->GetDrawObjectSlot(pad, "::Fitter");

      if (!overwrite && fitterslot)
         AddLink(fitterslot, "Fitter");
   }

   if (overwrite || !GetFitter())
      SetFitter(CreateFitterFor(panel, pad, "Fitter"));

   UpdateObjectReferenceInSlots();
   UpdateActivePage();
}

bool TGo4FitPanel::checkConfirm(const QString &title, const QString &msg)
{
   if (!fbNeedConfirmation)
      return false;
   return QMessageBox::information(nullptr, title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ==
          QMessageBox::No;
}

void TGo4FitPanel::Fitter_Delete()
{
   if (checkConfirm("Delete fitter", "Are you sure?"))
      return;

   RemovePrimitives();
   RemoveDrawObjects();
   CloseDrawPanel();

   ClearObjectReferenceInSlots();

   SetFitter(nullptr);

   if (WorkingWithPanel()) {
      fxActivePanel = nullptr;
      fxActivePad = nullptr;
   }

   UpdateActivePage();
}

void TGo4FitPanel::Fitter_UseWorkspace()
{
   RemovePrimitives();
   CloseDrawPanel();
   ClearObjectReferenceInSlots();

   fxActivePanel = nullptr;
   fxActivePad = nullptr;

   UpdateObjectReferenceInSlots();
   UpdateActivePage();
}

void TGo4FitPanel::Fitter_UpdateReferences()
{
   UpdateObjectReferenceInSlots();
   UpdateActivePage();
}

void TGo4FitPanel::Fitter_SaveToBrowser()
{
   TGo4LockGuard lock;

   TGo4Fitter *clone = CloneFitter();

   if (clone)
      SaveObjectInMemory("", clone);
}

void TGo4FitPanel::Fitter_PrintParameters()
{
   QTableWidget *tab = ParsTable;
   if (!tab)
      return;

   TGo4Log::Info("Printout of fitter parameters table");

   QString str("Name");
   for (int ncol = 0; ncol < tab->columnCount(); ncol++) {
      str += "\t";
      str += tab->horizontalHeaderItem(ncol)->text();
   }
   TGo4Log::Info(str.toLatin1().constData());

   for (int nrow = 0; nrow < tab->rowCount(); nrow++) {
      str = tab->verticalHeaderItem(nrow)->text();
      for (int ncol = 0; ncol < tab->columnCount(); ncol++) {
         str += "\t";
         str += tab->item(nrow, ncol)->text();
      }
      TGo4Log::Info(str.toLatin1().constData());
   }
}

void TGo4FitPanel::Fitter_RollbackParameters()
{
   TGo4Fitter *fitter = GetFitter();
   fitter->RememberPars();

   UpdateStatusBar();
   UpdateActivePage();
}

void TGo4FitPanel::Button_WorkWithPanel()
{
   TGo4LockGuard lock;

   TGo4Fitter *fitter = GetFitter();
   if (fitter) {
      TGo4ViewPanel *panel = LastActivePanel();
      if (!panel || (panel == fxDrawNewPanel))
         return;
      if ((panel != ActivePanel()) || (panel->GetActivePad() != ActivePad())) {
         TGo4Fitter *clone = (TGo4Fitter *)fitter->Clone();
         fxActivePanel = panel;
         fxActivePad = panel->GetActivePad();
         SetFitter(clone);
         UpdateObjectReferenceInSlots();
         UpdateActivePage();
      }
   } else
      Fitter_NewForActivePad(false);
}

void TGo4FitPanel::Button_SimpleFit(int nmodel)
{
   if (!WorkingWithOnlyPad())
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   if (fitter->GetNumData() != 1) {
      std::cout << " Incorrect data" << std::endl;
      return;
   }

   if (!UpdateObjectReferenceInSlots())
      return;

   TGo4FitData *data = fitter->GetData(0);
   if (!data)
      return;

   data->ClearRanges();
   data->SetRange(0, ActivePad()->GetUxmin(), ActivePad()->GetUxmax());

   fitter->SetMemoryUsage(1);

   int NumDim = data->DefineDimensions();

   TGo4FitModel *model = nullptr;

   switch (nmodel) {
   case 0: fitter->AddPolynoms(data->GetName(), "Pol", Smp_PolOrderSpin->value(), NumDim); break;

   case 1: model = CreateModel(20, 0, fitter, data); break;

   case 2: model = CreateModel(24, 0, fitter, data); break;

   case 3: model = CreateModel(25, 0, fitter, data); break;
   }

   if (model && ((nmodel == 1) || (nmodel == 2)))
      LocateModel(model, data, false);

   fbUseAmplEstim = true;
   fiNumMigradIter = 500;

   Button_PerformFit();
}

void TGo4FitPanel::Button_SimpleClear()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   fitter->DeleteAllModels();

   RemovePrimitives();
   RemoveDrawObjects();
}

void TGo4FitPanel::Button_PeakFinder()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   TGo4FitPeakFinder *finder = GetPeakFinder(true);
   if (!finder)
      return;

   TGo4FitData *data = 0;
   if (fiPanelMode == FitGui::pm_Wizard)
      data = Wiz_SelectedData();
   else
      data = fitter->GetData(0);
   if (!data)
      return;

   RemovePrimitives();
   RemoveDrawObjects();

   if (fbUseCurrentRange)
      Wiz_TakeCurrentRange();

   finder->SetDataName(data->GetName());

   finder->SetClearModels(kTRUE);

   fitter->SetMemoryUsage(fiBuffersUsage);

   UpdateStatusBar("Start peak finder ... ");

   fitter->DoAction(finder);

   fbUseAmplEstim = true;
   fiNumMigradIter = 300;

   UpdateStatusBar();

   fiPaintMode = 1;
   UpdateActivePage();
}

void TGo4FitPanel::Button_PerformFit()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   if (fiPanelMode != FitGui::pm_Expert) {

      if (fbUseAmplEstim || (fiNumMigradIter > 0)) {
         TGo4FitPeakFinder *finder = GetPeakFinder(false);
         Int_t n = 0;
         while (n < fitter->GetNumActions()) {
            TGo4FitterAction *action = fitter->GetAction(n);
            if (action == finder)
               n++;
            else
               fitter->DeleteAction(action);
         }

         if (fbUseAmplEstim)
            fitter->AddAmplEstimation();
         if (fiNumMigradIter > 0) {
            TGo4FitMinuit *minuit = new TGo4FitMinuit("Minuit");
            QString cmd("MIGRAD ");
            cmd += QString::number(fiNumMigradIter);
            cmd += " 1";
            minuit->AddCommand(cmd.toLatin1().constData());
            fitter->AddAction(minuit);
         }
      }

      if (fitter->GetNumActions() == 0)
         fitter->AddStandardActions();
   }

   if (fbUseCurrentRange)
      Wiz_TakeCurrentRange();

   fitter->SetMemoryUsage(fiBuffersUsage);

   fitter->MemorizePars();

   UpdateStatusBar("Start fitting ... ");

   fitter->DoActions();

   Button_FitterDraw(0);

   if (fiPanelMode == FitGui::pm_Simple) {
      RemovePrimitives();
      if (!fbShowPrimitives)
         return;
      TGo4FitData *data = fitter->GetData(0);
      PaintModelsFor(fitter, data, 0, true);
      UpdateStatusBar();
   } else
      UpdateActivePage();
}

void TGo4FitPanel::Button_FitterDraw(TGo4FitData *selecteddata)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter || (!fbDrawModels && !fbDrawComponents && !fbDrawBackground && !fbDrawInfoOnPad)) {
      RemoveDrawObjects();
      return;
   }

   bool UseSamePads = fbUseSamePanelForDraw;
   if (UseSamePads)
      for (Int_t n = 0; n < fitter->GetNumData(); n++) {
         TGo4FitData *data = fitter->GetData(n);
         if (selecteddata && (data != selecteddata))
            continue;
         if (data->IsAnyDataTransform()) {
            UseSamePads = false;
         }

         if (!FindPadWhereData(data))
            UseSamePads = false;
      }

   int NeedPads = 1;
   if (!UseSamePads) {
      if (!fxDrawNewPanel) {
         fbDrawPanelCreation = kTRUE;
         fxDrawNewPanel = CreateViewPanel();
         fbDrawPanelCreation = kFALSE;
         if (!fxDrawNewPanel)
            return;
      } else
         fxDrawNewPanel->ClearCanvas();

      if (!selecteddata)
         NeedPads = fitter->GetNumData();
      else
         NeedPads = 1;

      if (NeedPads > 1) {
         int DivX = (int)TMath::Sqrt(NeedPads * 1.);
         int DivY = DivX;
         while (DivX * DivY < NeedPads)
            DivY++;
         fxDrawNewPanel->Divide(DivX, DivY);
      }
   }

   for (Int_t n = 0; n < fitter->GetNumData(); n++) {
      TGo4FitData *data = fitter->GetData(n);

      if (selecteddata && (data != selecteddata))
         continue;

      const char *drawopt = nullptr;
      if (data->GetDataType() == TGo4FitData::dtGraph)
         drawopt = "L";

      TGo4ViewPanel *panel = nullptr;
      TPad *pad = nullptr;

      if (UseSamePads) {
         panel = ActivePanel();
         pad = FindPadWhereData(data);
      } else {
         panel = fxDrawNewPanel;
         pad = fxDrawNewPanel->GetSubPad(nullptr, n, true);
      }
      if (!panel || !pad)
         continue;

      panel->DeleteDrawObjects(pad, TGo4ViewPanel::kind_FitModels);

      // disable autoscaling while THStack is not able to automatically
      // scale only visible region
      if (UseSamePads)
         panel->SetAutoScale(false, pad);

      // panel->DeleteDrawObjects(pad, TGo4ViewPanel::kind_FitInfo);
      // panel->SetPadSuperImpose(pad, true);

      panel->ShootRepaintTimer();

      if (!UseSamePads) {
         TObject *obj = fitter->CreateDrawObject("Data", data->GetName(), kFALSE);
         panel->AddDrawObject(pad, TGo4ViewPanel::kind_FitModels, "::Data", obj, true, drawopt);
      }

      if (!fbDrawInfoOnPad)
         panel->DeleteDrawObjects(pad, TGo4ViewPanel::kind_FitInfo);
      else {
         Double_t x2 = 0.98;
         Double_t y2 = 0.5;
         Double_t x1 = x2 - gStyle->GetStatW();
         Double_t y1 = y2 - gStyle->GetStatH();

         if (LineParsChk->isChecked()) {
            x1 = 0.6;
            x2 = 0.95;
            y1 = 0.8;
            y2 = 0.88;
         }

         TPaveStats *stats = dynamic_cast<TPaveStats *>(pad->GetPrimitive("stats"));
         if (stats) {
            x1 = stats->GetX1NDC();
            x2 = stats->GetX2NDC();
            y2 = stats->GetY1NDC() - 0.03;
            y1 = y2 - (stats->GetY2NDC() - stats->GetY1NDC());
            if (y1 < 0.02)
               y1 = 0.02;
         }

         TLegend *leg = dynamic_cast<TLegend *>(pad->GetPrimitive("fitlegend"));
         if (leg) {
            x1 = leg->GetX1NDC();
            x2 = leg->GetX2NDC();
            y2 = leg->GetY1NDC() - 0.03;
            y1 = y2 - (leg->GetY2NDC() - leg->GetY1NDC());
            if (y1 < 0.02)
               y1 = 0.02;
         }

         TPaveStats *info = dynamic_cast<TPaveStats *>(panel->GetDrawObject(pad, "::FitterPars"));

         bool infocreated = false;

         if (info)
            info->Clear("*");
         else {
            infocreated = true;

            info = new TPaveStats(x1, y1, x2, y2, "brNDC");
            info->SetFillColor(gStyle->GetStatColor());
            info->SetFillStyle(gStyle->GetStatStyle());
            info->SetBorderSize(gStyle->GetStatBorderSize());
            info->SetTextFont(gStyle->GetStatFont());
            if (gStyle->GetStatFont() % 10 > 2)
               info->SetTextSize(gStyle->GetStatFontSize());
            info->SetFitFormat(gStyle->GetFitFormat());
            info->SetStatFormat(gStyle->GetStatFormat());
            info->SetTextColor(gStyle->GetStatTextColor());
            info->SetTextAlign(12);
            info->SetName("fitinfo");
         }

         info->AddText(data->GetName());
         info->AddLine(0, 0, 0, 0);

         if (LineParsChk->isChecked())
            info->AddText(TString::Format("Line | Ampl | Pos & %s", (fbRecalculateGaussWidth ? "FWHM" : "Width")));

         for (Int_t m = 0; m < fitter->GetNumModel(); m++) {
            TGo4FitModel *model = fitter->GetModel(m);
            if (!model->IsAssignTo(data->GetName()))
               continue;

            if (LineParsChk->isChecked()) {
               Double_t ampl, pos, width;

               ampl = model->GetAmplValue();
               if (!model->GetPosition(0, pos))
                  pos = 0.;
               if (!model->GetWidth(0, width))
                  width = 0.;

               bool recalculatew = (model->InheritsFrom(TGo4FitModelGauss1::Class()) ||
                                    model->InheritsFrom(TGo4FitModelGauss2::Class()) ||
                                    model->InheritsFrom(TGo4FitModelGaussN::Class())) &&
                                   fbRecalculateGaussWidth;
               double widthk = recalculatew ? 2.3548 : 1.0;

               TString tt = TString::Format("%7s | %s%s | %s%s %s%s", model->GetName(), "%", gStyle->GetStatFormat(),
                                            "%", gStyle->GetStatFormat(), "%", gStyle->GetStatFormat());

               info->AddText(TString::Format(tt.Data(), ampl, pos, width * widthk).Data());
            } else {
               int maxparlen = 7;
               for (Int_t np = 0; np < model->NumPars(); np++) {
                  int len = strlen(model->GetParFullName(np));
                  if (len > maxparlen)
                     maxparlen = len;
               }
               TString tt = TString::Format("%s%ds = %s%s", "%", maxparlen, "%", gStyle->GetStatFormat());

               for (Int_t np = 0; np < model->NumPars(); np++)
                  info->AddText(
                     TString::Format(tt.Data(), model->GetParFullName(np), model->GetPar(np)->GetValue()).Data());
            }
         }

         if (infocreated)
            panel->AddDrawObject(pad, TGo4ViewPanel::kind_FitInfo, "::FitterPars", info, true, nullptr);
      }

      if (fbDrawModels) {
         TObject *model = fitter->CreateDrawObject("Model", data->GetName(), kTRUE);
         panel->AddDrawObject(pad, TGo4ViewPanel::kind_FitModels, "::DataModel", model, true, drawopt);
      }

      if (fbDrawBackground) {
         TObject *model = fitter->CreateDrawObject("Background", data->GetName(), kTRUE, "Background");
         panel->AddDrawObject(pad, TGo4ViewPanel::kind_FitModels, "::Background", model, true, drawopt);
      }

      if (fbDrawComponents)
         for (Int_t m = 0; m < fitter->GetNumModel(); m++) {
            TGo4FitModel *model = fitter->GetModel(m);
            if (!model->IsAssignTo(data->GetName()))
               continue;
            if (model->GetGroupIndex() == 0)
               continue;
            TObject *comp = fitter->CreateDrawObject(model->GetName(), data->GetName(), kTRUE, model->GetName());
            if (!comp)
               continue;

            panel->AddDrawObject(pad, TGo4ViewPanel::kind_FitModels, "::FitComponent", comp, true, drawopt);
         }
   } // data
}

void TGo4FitPanel::Cmd_CreateFitter()
{
   TGo4Fitter *fitter = new TGo4Fitter("Fitter", "Fitter object");
   SetFitter(fitter);

   UpdateActivePage();
}

void TGo4FitPanel::Cmd_CreateAppropriateFitter()
{
   TGo4Fitter *fitter = CreateFitterFor(ActivePanel(), ActivePad(), "Fitter");
   if (fitter) {
      SetFitter(fitter);
      UpdateActivePage();
   }
}

void TGo4FitPanel::Cmd_DeleteFitter()
{
   if (checkConfirm("Delete fitter", "Are you sure?"))
      return;

   RemovePrimitives();

   SetFitter(nullptr);

   UpdateActivePage();
}

void TGo4FitPanel::Cmd_ClearFitter()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   if (checkConfirm("Clear fitter", "Are you sure?"))
      return;

   fitter->Clear();

   UpdateActivePage();
}

void TGo4FitPanel::Cmd_SaveFitter(bool ask)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   QString fname(fitter->GetName());
   fname += ".root";

   if (ask) {
      fname = QFileDialog::getSaveFileName(this, "Save fitter dialog", fname, "Root file (*.root)");
      if (fname.isNull())
         return;
   }

   fitter->SetSaveFlagForObjects(fbSaveWithReferences);

   TFile f(fname.toLatin1().constData(), "recreate");
   fitter->Write();
}

void TGo4FitPanel::Cmd_ItemPrint(QFitItem *item)
{
   TObject *obj = item->Object();
   if (!obj)
      return;

   QString str = "*";
   bool ok = true;
   if (obj->InheritsFrom(TGo4Fitter::Class()))
      str = QInputDialog::getText(this, QString("Go4 Fit Panel"), QString("Enter parameters for Print() command"),
                                  QLineEdit::Normal, str, &ok);
   if (!ok)
      return;

   RemoveItemWidget();
   QFitPrintWidget *widget = new QFitPrintWidget(0, (QString("Print ") + obj->GetName()).toLatin1().constData());

   fxCurrentItemWidget = widget;

   ListStack->addWidget(widget);
   widget->SetDrawOption(str);
   widget->SetItem(this, item);
   ListStack->setCurrentWidget(widget);
}

void TGo4FitPanel::Cmd_DrawData(QFitItem *item)
{
   TGo4FitData *data = dynamic_cast<TGo4FitData *>(item->Object());
   if (data)
      Button_FitterDraw(data);
}

void TGo4FitPanel::Cmd_DeleteData(QFitItem *item)
{
   TGo4FitData *data = dynamic_cast<TGo4FitData *>(item->Object());
   TGo4Fitter *fitter = GetFitter();

   if (!data || !fitter)
      return;

   if (checkConfirm("Delete data", QString("Are you sure to delete ") + data->GetName()))
      return;

   fitter->RemoveData(data->GetName(), kTRUE);

   UpdateObjectReferenceInSlots();

   UpdateItemsOfType(FitGui::ot_allslots);

   UpdateItem(item->Parent(), true);

   UpdateStatusBar();
}

void TGo4FitPanel::Cmd_DeleteAllData(QFitItem *item)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   if (checkConfirm("Delete all data", "Are you sure to delete all data objects"))
      return;

   fitter->DeleteAllData();

   UpdateObjectReferenceInSlots();

   UpdateItemsOfType(FitGui::ot_allslots);

   UpdateItem(item, true);

   UpdateStatusBar();
}

void TGo4FitPanel::Cmd_DeleteAssosiatedModels(QFitItem *item)
{
   if (!item || !item->Parent())
      return;
   TGo4FitData *data = dynamic_cast<TGo4FitData *>(item->Object());
   TGo4Fitter *fitter = GetFitter();

   if (!data || !fitter)
      return;

   if (checkConfirm("Delete models", QString("Are you sure to delete models, associated to ") + data->GetName()))
      return;

   fitter->DeleteModelsAssosiatedTo(data->GetName());

   QFitItem *mlist = FindItem(0, FitGui::ot_modellist, 0);
   if (mlist)
      UpdateItem(mlist, true);

   UpdateItemsOfType(FitGui::ot_allslots);

   ShowItem(item, false);

   UpdateStatusBar();
}

void TGo4FitPanel::Cmd_DeleteModel(QFitItem *item)
{
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());

   Cmd_RemoveModel(model);
}

void TGo4FitPanel::Cmd_RemoveModel(TGo4FitModel *model)
{
   TGo4Fitter *fitter = GetFitter();
   if (!model || !fitter)
      return;

   if (checkConfirm("Delete model", QString("Are you sure to delete ") + model->GetName()))
      return;

   fitter->RemoveModel(model->GetName(), kTRUE);

   UpdateObjectReferenceInSlots();

   UpdateActivePage();
}

void TGo4FitPanel::Cmd_DeleteModels(QFitItem *item)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   if (checkConfirm("Delete all models", "Are you sure to delete all models objects"))
      return;

   fitter->DeleteAllModels();

   UpdateObjectReferenceInSlots();

   UpdateItem(item, true);
   UpdateItemsOfType(FitGui::ot_allslots, 0);
   UpdateStatusBar();
}

void TGo4FitPanel::Cmd_CloneModel(QFitItem *item)
{
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());

   TGo4Fitter *fitter = GetFitter();
   if (!fitter || !model)
      return;

   fitter->CloneModel(model->GetName());

   UpdateObjectReferenceInSlots();

   UpdateActivePage();
}

void TGo4FitPanel::Cmd_AddNewData(QFitItem *item, int id)
{
   if (item->ObjectType() != FitGui::ot_datalist)
      item = item->Parent();
   if (item->ObjectType() != FitGui::ot_datalist)
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   TGo4FitData *data = CreateData(id, 0);
   if (!data)
      return;

   fitter->AddData(data);

   UpdateObjectReferenceInSlots();

   UpdateItem(item, true);

   UpdateItemsOfType(FitGui::ot_allslots);

   QFitItem *curr = FindItem(data, FitGui::ot_data, item);
   if (curr)
      FitList->setCurrentItem(curr, QItemSelectionModel::Select);
   UpdateStatusBar();
}

void TGo4FitPanel::Cmd_AddNewModel(QFitItem *item, int id)
{
   if (item->ObjectType() != FitGui::ot_modellist)
      item = item->Parent();
   if (item->ObjectType() != FitGui::ot_modellist)
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   TGo4FitModel *model = CreateModel(id, 0, fitter, 0);
   LocateModel(model, 0, true);

   UpdateObjectReferenceInSlots();

   UpdateItem(item, true);

   UpdateItemsOfType(FitGui::ot_allslots);

   QFitItem *curr = FindItem(model, FitGui::ot_model, item);
   if (curr)
      FitList->setCurrentItem(curr, QItemSelectionModel::Select);
   UpdateStatusBar();
}

void TGo4FitPanel::Cmd_ClearAssigment(QFitItem *item)
{
   if (!item)
      return;

   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Parent()->Object());
   if (!model)
      return;

   model->ClearAssignmentTo(item->text(0).toLatin1().constData());

   UpdateItemsOfType(FitGui::ot_parslist, item->Parent()->Parent());

   UpdateItem(item->Parent(), true);
}

void TGo4FitPanel::Cmd_ClearAssigments(QFitItem *item)
{
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
   if (!model)
      return;

   model->ClearAssignments();

   UpdateItemsOfType(FitGui::ot_parslist, item->Parent());

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_AssignModelToAllData(QFitItem *item)
{
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
   TGo4Fitter *fitter = GetFitter();
   if (!model || !fitter)
      return;

   fitter->AssignModelTo(model->GetName(), 0);

   UpdateItemsOfType(FitGui::ot_parslist, item->Parent());

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_AssignModelToData(QFitItem *item, int id)
{
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
   TGo4Fitter *fitter = GetFitter();
   if (!model || !fitter)
      return;

   fitter->AssignModelTo(model->GetName(), fitter->GetDataName(id));

   UpdateItemsOfType(FitGui::ot_parslist, item->Parent());

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_RemoveRangeCondition(QFitItem *item)
{
   if (!item)
      return;

   Int_t typ = item->ObjectType();

   if ((typ != FitGui::ot_range) && (typ != FitGui::ot_rangecut))
      return;

   int num = item->Tag();

   if (num < 0)
      return;

   if (checkConfirm("Remove condition", QString("Are you sure to delete condition ") + item->text(0)))
      return;

   QFitItem *parent = item->Parent();
   TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(parent->Object());

   if (typ == FitGui::ot_range)
      comp->RemoveRangeCondition(num);
   else
      comp->RemoveRangeCut(num);

   UpdateItem(parent, true);
}

void TGo4FitPanel::Cmd_RemoveRangeConditions(QFitItem *item)
{
   if (!item || (item->ObjectType() != FitGui::ot_rangelist))
      return;

   if (checkConfirm("Delete conditions", "Are you sure to delete all conditions"))
      return;

   TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(item->Object());
   comp->ClearRanges();

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_AddRangeCondition(QFitItem *item, int id)
{
   if (!item || (item->ObjectType() != FitGui::ot_rangelist))
      return;

   TGo4FitData *data = nullptr;

   if (item->Object()->InheritsFrom(TGo4FitData::Class())) {
      data = dynamic_cast<TGo4FitData *>(item->Object());
   } else {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
      TGo4Fitter *fitter = GetFitter();
      if (model && fitter)
         for (Int_t n = 0; n < model->NumAssigments(); n++) {
            data = fitter->FindData(model->AssignmentName(n));
            if (data)
               break;
         }
   }
   Double_t minx = 0., maxx = 100., miny = 0., maxy = 100;

   TPad *pad = FindPadWhereData(data);
   if (pad) {
      minx = pad->GetUxmin();
      maxx = pad->GetUxmax();
      miny = pad->GetUymin();
      maxy = pad->GetUymax();
   } else if (data) {
      data->DefineScaleMinMax(0, minx, maxx);
      data->DefineScaleMinMax(1, miny, maxy);
      Double_t ddx = 0.1 * (maxx - minx);
      minx += ddx;
      maxx -= ddx;
      Double_t ddy = 0.1 * (maxy - miny);
      miny += ddy;
      maxy -= ddy;
   }

   TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(item->Object());

   if (id == 0)
      comp->SetRange(0, minx, maxx);
   else if (id == 1)
      comp->ExcludeRange(0, minx, maxx);
   else if (id == 2)
      comp->SetRangeMin(0, minx);
   else if (id == 3)
      comp->SetRangeMax(0, maxx);
   else if (id == 4) {
      TString name = TString::Format("Cut%d", comp->GetNumRangeCut());
      TCutG *cut = new TCutG(name, 5);
      cut->SetTitle("range cut of X and Y axis");
      cut->SetPoint(0, minx, miny);
      cut->SetPoint(1, maxx, miny);
      cut->SetPoint(2, maxx, maxy);
      cut->SetPoint(3, minx, maxy);
      cut->SetPoint(4, minx, miny);
      comp->AddRangeCut(cut);
   }

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_DeleteAction(QFitItem *item)
{
   if (!item)
      return;
   TGo4FitterAction *action = dynamic_cast<TGo4FitterAction *>(item->Object());
   TGo4Fitter *fitter = GetFitter();

   if (action && fitter) {
      if (checkConfirm("Delete action", QString("Are you sure to delete ") + item->text(0)))
         return;

      fitter->DeleteAction(action);
      UpdateItem(item->Parent(), true);
   }
}

void TGo4FitPanel::Cmd_MoveAction(QFitItem *item, int dir)
{
   if (!item || !item->Parent())
      return;

   TGo4FitterAction *action = dynamic_cast<TGo4FitterAction *>(item->Object());
   TGo4Fitter *fitter = GetFitter();

   if (action && fitter) {
      fitter->ReplaceAction(action, dir);
      QFitItem *parent = item->Parent();
      UpdateItem(parent, true);
      item = FindItem(action, -1, 0);
      if (item)
         FitList->setCurrentItem(item, QItemSelectionModel::Select);
   }
}

void TGo4FitPanel::Cmd_ExecuteAction(QFitItem *item)
{
   TGo4Fitter *fitter = GetFitter();
   if (!item || !fitter)
      return;

   TGo4FitterAction *action = dynamic_cast<TGo4FitterAction *>(item->Object());
   if (!action)
      return;

   fitter->DoAction(action);

   if (action->CanChangeFitter()) {
      UpdateActivePage();
   } else {
      UpdateItem(item, true);
      UpdateItemsOfType(FitGui::ot_par);
   }
}

void TGo4FitPanel::Cmd_ExecuteActions(QFitItem *item, bool expert)
{
   TGo4Fitter *fitter = GetFitter();

   if (!item || !fitter)
      return;

   fitter->MemorizePars();

   fitter->DoActions(expert);

   if (expert) {
      UpdateActivePage();
   } else {
      UpdateItem(item, true);
      UpdateItemsOfType(FitGui::ot_par);
   }
}

void TGo4FitPanel::Cmd_DeleteOutputActions(QFitItem *item)
{
   TGo4Fitter *fitter = GetFitter();
   if (!item || (item->ObjectType() != FitGui::ot_actlist) || !fitter)
      return;

   if (checkConfirm("Delete output actions", QString("Are you sure to delete all output actions") + item->text(0)))
      return;

   fitter->DeleteOutputActions();
   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_DeleteActions(QFitItem *item)
{
   TGo4Fitter *fitter = GetFitter();
   if (!item || (item->ObjectType() != FitGui::ot_actlist) || !fitter)
      return;

   if (checkConfirm("Delete all actions", "Are you sure to delete all actions?"))
      return;

   fitter->DeleteActions();
   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_AddNewAction(QFitItem *item, int id)
{
   TGo4Fitter *fitter = GetFitter();
   if (!item || (item->ObjectType() != FitGui::ot_actlist) || !fitter)
      return;

   switch (id) {
   case 0: fitter->AddAction(new TGo4FitterConfig("Config", "Fitter configuration")); break;
   case 1: fitter->AddAmplEstimation(); break;
   case 2: fitter->AddSimpleMinuit(); break;
   case 3: fitter->AddOutputAction("Print", "*"); break;
   case 4: fitter->AddAction(new TGo4FitPeakFinder("Finder")); break;
   }
   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_DeleteDependency(QFitItem *item)
{
   if (!item || (item->ObjectType() != FitGui::ot_depend))
      return;

   TGo4FitDependency *depen = dynamic_cast<TGo4FitDependency *>(item->Object());
   TObjArray *lst = dynamic_cast<TObjArray *>(item->Parent()->Object());

   if (!depen || !lst)
      return;

   if (checkConfirm("Delete dependency item", QString("Are you sure to delete ") + item->text(0)))
      return;

   lst->Remove(depen);
   lst->Compress();
   delete depen;

   UpdateItem(item->Parent(), true);
}

void TGo4FitPanel::Cmd_DeleteDependencies(QFitItem *item)
{
   if (!item)
      return;
   TObjArray *lst = dynamic_cast<TObjArray *>(item->Object());
   if (!lst)
      return;

   if (checkConfirm("Delete all", QString("Are you sure to delete all items from ") + item->text(0)))
      return;

   lst->Clear();

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_AddDependency(QFitItem *item)
{
   if (!item)
      return;

   TObjArray *lst = dynamic_cast<TObjArray *>(item->Object());
   if (!lst)
      return;

   if (item->ObjectType() == FitGui::ot_reslist)
      lst->Add(new TGo4FitDependency(0, 0.0));
   else
      lst->Add(new TGo4FitDependency("Par1", 1.0));

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_MemorizePars(QFitItem *item)
{
   if (!item)
      return;

   TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Object());
   if (!pars)
      return;

   pars->MemorizePars();
}

void TGo4FitPanel::Cmd_RememberPars(QFitItem *item)
{
   if (!item)
      return;

   TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Object());
   if (!pars)
      return;

   pars->RememberPars();

   UpdateItemsOfType(FitGui::ot_par, item);

   ShowItem(item, false);
}

void TGo4FitPanel::Cmd_DeletePars(QFitItem *item)
{
   if (!item)
      return;
   TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Object());
   if (!pars)
      return;

   if (checkConfirm("Delete all", "Are you sure to delete all parameters"))
      return;

   pars->ClearPars();

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_MemorizePar(QFitItem *item)
{
   if (!item)
      return;
   TGo4FitParameter *par = dynamic_cast<TGo4FitParameter *>(item->Object());
   if (!par)
      return;

   par->MemorizeValue();
}

void TGo4FitPanel::Cmd_RememberPar(QFitItem *item)
{
   if (!item)
      return;
   TGo4FitParameter *par = dynamic_cast<TGo4FitParameter *>(item->Object());
   if (!par)
      return;

   par->RememberValue();

   SetItemText(item, true);

   ShowItem(item, false);
}

void TGo4FitPanel::Cmd_AddNewPar(QFitItem *item)
{
   if (!item)
      return;

   TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Object());
   if (!pars)
      return;

   TString name;
   int num = 0;
   do {
      name = TString::Format("Par%d", num++);
   } while (pars->FindPar(name.Data()));
   pars->CreatePar(name.Data(), "additional parameter", 0.);

   UpdateItem(item, true);
}

void TGo4FitPanel::Cmd_DeletePar(QFitItem *item)
{
   if (!item)
      return;

   TGo4FitParameter *par = dynamic_cast<TGo4FitParameter *>(item->Object());
   TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Parent()->Object());
   if (!pars || !par)
      return;

   if (checkConfirm("Delete parameter", QString("Are you sure to delete ") + par->GetName()))
      return;

   pars->RemovePar(par);

   UpdateItem(item->Parent(), true);
}

void TGo4FitPanel::Cmd_DeleteMinuitResult(QFitItem *item)
{
   if (!item)
      return;

   TGo4FitMinuitResult *res = dynamic_cast<TGo4FitMinuitResult *>(item->Object());
   TGo4FitMinuit *minuit = dynamic_cast<TGo4FitMinuit *>(item->Parent()->Object());
   if (!res || !minuit)
      return;

   if (checkConfirm("Delete result", QString("Are you sure to delete ") + res->GetName()))
      return;

   minuit->RemoveResult(res);
   delete res;

   UpdateItem(item->Parent(), true);
}

void TGo4FitPanel::Cmd_UpdateAllSlots(QFitItem *item)
{
   if (!item)
      return;

   UpdateObjectReferenceInSlots();

   for (int n = 0; n < item->childCount(); n++)
      UpdateItem(dynamic_cast<QFitItem *>(item->child(n)), true);
}

// *************************** fitslots  ends ******************************

void TGo4FitPanel::FitList_customContextMenuRequested(const QPoint &pnt)
{
   QFitItem *fititem = dynamic_cast<QFitItem *>(FitList->itemAt(pnt));
   if (!fititem)
      return;

   QMenu menu;

   if (FillPopupForItem(fititem, &menu)) {
      CurrFitItem = fititem;
      menu.exec(FitList->viewport()->mapToGlobal(pnt));
      CurrFitItem = nullptr;
   }
}

void TGo4FitPanel::FitList_currentItemChanged(QTreeWidgetItem *curr, QTreeWidgetItem *)
{
   if (fbFillingWidget)
      return;

   QFitItem *fititem = dynamic_cast<QFitItem *>(curr);
   if (!fititem)
      return;

   ShowItem(fititem, false);
   fxCurrentItem = fititem;

   UpdateItemMenu();
}

void TGo4FitPanel::AboutToShowViewMenu()
{
   ViewMenu->clear();

   AddIdAction(ViewMenu, ViewMap, "&Simple", FitGui::pm_Simple);
   AddIdAction(ViewMenu, ViewMap, "&Wizard", FitGui::pm_Wizard);
   AddIdAction(ViewMenu, ViewMap, "&Expert", FitGui::pm_Expert);
   SetIdAction(ViewMap, fiPanelMode, true, true);
}

void TGo4FitPanel::ChangeViewType(int id)
{
   if (id != FitGui::pm_None)
      fiPanelMode = id;

   fbParsWidgetShown = false;
   UpdateActivePage();
}

void TGo4FitPanel::ShowPanelPage(int id)
{
   QWidget *w = PageEmpty;
   switch (id) {
   case 1: w = PageSimple; break;
   case 2: w = PageWizard; break;
   case 3: w = PageExtended; break;
   case 100: w = PageParameters; break;
   }
   if (w != WidgetStack->currentWidget())
      WidgetStack->setCurrentWidget(w);
}

void TGo4FitPanel::UpdateStatusBar(const char *info)
{
   QString status;
   TGo4Fitter *fitter = GetFitter();

   if (info) {
      status = info;
   } else {
      if (WorkingWithPanel()) {
         status = QString("Active: ") + ActivePad()->GetName();
         if (!WorkingWithOnlyPad())
            status += ". Muiltifit";

      } else
         status = "Workspace";
      if (!fitter)
         status += ".  No fitter";
      else
         status = status + ". Fitter: " + fitter->GetName();
   }
   MainStatusLbl->setText(status);
   MainStatusLbl->update();
   MainStatusLbl->repaint();

   MainAttachBtn->setEnabled(true);

   if (fbParsWidgetShown) {
      MainParsBtn->setText("Back");
      MainParsBtn->setToolTip("Back to last selected page");
   } else {
      MainParsBtn->setText("Pars");
      MainParsBtn->setToolTip("Show list of fitter parameters");
   }

   if (!fitter) {
      MainFitBtn->setEnabled(false);
      MainDrawBtn->setEnabled(false);
      MainFindBtn->setEnabled(false);
      MainParsBtn->setEnabled(false);
   } else {
      MainFitBtn->setEnabled((fitter->GetNumData() > 0) &&
                             ((fitter->GetNumModel() > 0) || (fiPanelMode == FitGui::pm_Simple)));
      MainDrawBtn->setEnabled(fitter->GetNumData() > 0);
      MainFindBtn->setEnabled((fitter->GetNumData() > 0) && (fiPanelMode == FitGui::pm_Wizard) && GetPeakFinder(false));
      MainParsBtn->setEnabled(true);
   }
}

void TGo4FitPanel::UpdateItemMenu()
{
   bool showitem = fiPanelMode == FitGui::pm_Expert;

   QString itemtext;

   if (showitem) {
      QFitItem *item = dynamic_cast<QFitItem *>(FitList->currentItem());
      if (!item || (item->ObjectType() == FitGui::ot_fitter))
         showitem = false;
      else
         itemtext = item->text(0);
   }

   if (showitem) {
      if (!ItemMenu) {
         ItemMenu = MenuBar->addMenu(itemtext);
         QObject::connect(ItemMenu, &QMenu::aboutToShow, this, &TGo4FitPanel::AboutToShowItemMenu);
      } else
         ItemMenu->setTitle(itemtext);
      MenuBar->adjustSize();
   } else if (ItemMenu) {
      delete ItemMenu;
      ItemMenu = nullptr;
   }
}

void TGo4FitPanel::AboutToShowSettMenu()
{
   SetIdAction(SettMap, 1, true, fbNeedConfirmation);
   SetIdAction(SettMap, 2, true, fbShowPrimitives);
   SetIdAction(SettMap, 3, true, fbFreezeMode);
   SetIdAction(SettMap, 4, (fiPanelMode == FitGui::pm_Expert) || (fiPanelMode == FitGui::pm_Wizard),
               fbSaveWithReferences);

   SetIdAction(SettMap, 10, true, fbUseCurrentRange);
   SetIdAction(SettMap, 11, true, fbDrawModels);
   SetIdAction(SettMap, 12, true, fbDrawComponents);
   SetIdAction(SettMap, 13, true, fbUseSamePanelForDraw);
   SetIdAction(SettMap, 14, true, fbDrawBackground);
   SetIdAction(SettMap, 15, true, fbDrawInfoOnPad);
   SetIdAction(SettMap, 16, true, fbRecalculateGaussWidth);

   SetIdAction(SettMap, 17, true, fiIntegralMode == 0);
   SetIdAction(SettMap, 18, true, fiIntegralMode == 1);
   SetIdAction(SettMap, 19, true, fiIntegralMode == 2);
   SetIdAction(SettMap, 20, true, fiIntegralMode == 3);

   SetIdAction(SettMap, 21, true, fiBuffersUsage == 0);
   SetIdAction(SettMap, 22, true, fiBuffersUsage == 1);
   SetIdAction(SettMap, 23, true, fiBuffersUsage == 2);
   SetIdAction(SettMap, 24, true, fiBuffersUsage == 3);
}

void TGo4FitPanel::ChangeSettings(int id)
{
   switch (id) {
   case 1: fbNeedConfirmation = !fbNeedConfirmation; break;
   case 2: fbShowPrimitives = !fbShowPrimitives; break;
   case 3: fbFreezeMode = !fbFreezeMode; break;
   case 4: fbSaveWithReferences = !fbSaveWithReferences; break;
   case 10: fbUseCurrentRange = !fbUseCurrentRange; break;
   case 11: fbDrawModels = !fbDrawModels; break;
   case 12: fbDrawComponents = !fbDrawComponents; break;
   case 13: fbUseSamePanelForDraw = !fbUseSamePanelForDraw; break;
   case 14: fbDrawBackground = !fbDrawBackground; break;
   case 15: fbDrawInfoOnPad = !fbDrawInfoOnPad; break;
   case 16:
      fbRecalculateGaussWidth = !fbRecalculateGaussWidth;
      UpdateActivePage();
      break;
   case 17:
      fiIntegralMode = 0;
      UpdateActivePage();
      break;
   case 18:
      fiIntegralMode = 1;
      UpdateActivePage();
      break;
   case 19:
      fiIntegralMode = 2;
      UpdateActivePage();
      break;
   case 20:
      fiIntegralMode = 3;
      UpdateActivePage();
      break;
   case 21: fiBuffersUsage = 0; break;
   case 22: fiBuffersUsage = 1; break;
   case 23: fiBuffersUsage = 2; break;
   case 24: fiBuffersUsage = 3; break;
   }

   go4sett->setBool("/FitPanel/NeedConfirmation", fbNeedConfirmation);
   go4sett->setBool("/FitPanel/ShowPrimitives", fbShowPrimitives);
   go4sett->setBool("/FitPanel/SaveWithReferences", fbSaveWithReferences);
   go4sett->setBool("/FitPanel/UseCurrentRange", fbUseCurrentRange);
   go4sett->setBool("/FitPanel/DrawModels", fbDrawModels);
   go4sett->setBool("/FitPanel/DrawComponents", fbDrawComponents);
   go4sett->setBool("/FitPanel/UseSamePanelForDraw", fbUseSamePanelForDraw);
   go4sett->setBool("/FitPanel/DrawBackground", fbDrawBackground);
   go4sett->setBool("/FitPanel/DrawInfoOnPad", fbDrawInfoOnPad);
   go4sett->setBool("/FitPanel/RecalculateGaussWidth", fbRecalculateGaussWidth);
   go4sett->setInt("/FitPanel/IntegralMode", fiIntegralMode);
   go4sett->setInt("/FitPanel/BuffersUsage", fiBuffersUsage);
}

void TGo4FitPanel::AboutToShowFitterMenu()
{
   FitterMenu->clear();

   TGo4Fitter *fitter = GetFitter();
   TGo4ViewPanel *panel = LastActivePanel();

   bool samepad =
      !panel ? false : WorkingWithPanel() && (panel == ActivePanel()) && (panel->GetActivePad() == ActivePad());

   QString padname;
   if (panel)
      padname = QString("panel \"") + panel->windowTitle() + "\"";

   if (fiPanelMode == FitGui::pm_Expert)
      AddIdAction(FitterMenu, FitterMap, "&Create for workspace", 1, true);
   AddIdAction(FitterMenu, FitterMap, QString("Create &for ") + padname, 2, (panel != nullptr));
   AddIdAction(FitterMenu, FitterMap, "&Delete", 3, (fitter != nullptr));
   FitterMenu->addSeparator();

   AddIdAction(FitterMenu, FitterMap, "Save to &browser", 21, (fitter != nullptr));
   if (fiPanelMode == FitGui::pm_Expert)
      AddIdAction(FitterMenu, FitterMap, "&Workspace", 23, WorkingWithPanel());
   AddIdAction(FitterMenu, FitterMap, "&Update references", 24, (fitter != nullptr));
   AddIdAction(FitterMenu, FitterMap, "&Print parameters", 25, (fitter != nullptr) && fbParsWidgetShown);
   AddIdAction(FitterMenu, FitterMap, "&Rollback parameters", 26, (fitter != nullptr) && fitter->CanRollbackPars());

   FitterMenu->addSeparator();

   AddIdAction(FitterMenu, FitterMap, "&Close", 99, true);
}

void TGo4FitPanel::FitterMenuItemSelected(int id)
{
   switch (id) {
   case 1: Fitter_New(); break;
   case 2: Fitter_NewForActivePad(true); break;
   case 3: Fitter_Delete(); break;
   case 21: Fitter_SaveToBrowser(); break;
   case 23: Fitter_UseWorkspace(); break;
   case 24: Fitter_UpdateReferences(); break;
   case 25: Fitter_PrintParameters(); break;
   case 26: Fitter_RollbackParameters(); break;
   case 99: parentWidget()->close(); break;
   }
}

void TGo4FitPanel::AboutToShowItemMenu()
{
   QFitItem *item = dynamic_cast<QFitItem *>(FitList->currentItem());
   if (!ItemMenu || !item)
      return;
   ItemMenu->clear();
   FillPopupForItem(item, ItemMenu);
}

void TGo4FitPanel::ItemMenuItemSelected(int id)
{
   QFitItem *item = CurrFitItem;
   if (!item)
      item = dynamic_cast<QFitItem *>(FitList->currentItem());
   if (!item)
      return;

   switch (id) {
   case 1: Cmd_DeleteFitter(); break;
   case 2: Cmd_ClearFitter(); break;
   case 3: Cmd_SaveFitter(false); break;
   case 4: Cmd_SaveFitter(true); break;
   case 5: Cmd_ItemPrint(item); break;
   case 6: Button_FitterDraw(0); break;
   case 8: Cmd_CreateFitter(); break;
   case 9: Cmd_CreateAppropriateFitter(); break;
   case 101: Cmd_DeleteData(item); break;
   case 102: Cmd_DeleteAssosiatedModels(item); break;
   case 105: Cmd_DrawData(item); break;
   case 107: Cmd_DeleteAllData(item); break;
   case 108: Wiz_RebuildDataList(); break;
   case 201: Cmd_DeleteModel(item); break;
   case 202: Cmd_DeleteModels(item); break;
   case 203: Cmd_CloneModel(item); break;
   case 301: Cmd_ClearAssigments(item); break;
   case 302: Cmd_AssignModelToAllData(item); break;
   case 303: Cmd_ClearAssigment(item); break;
   case 401: Cmd_RemoveRangeCondition(item); break;
   case 402: Cmd_RemoveRangeConditions(item); break;
   case 403:
   case 404:
   case 405:
   case 406:
   case 407: Cmd_AddRangeCondition(item, id - 403); break;
   case 501: Cmd_DeleteAction(item); break;
   case 502: Cmd_MoveAction(item, -1); break;
   case 503: Cmd_MoveAction(item, +1); break;
   case 504: Cmd_ExecuteActions(item, false); break;
   case 505: Cmd_DeleteActions(item); break;
   case 506: Cmd_DeleteOutputActions(item); break;
   case 507: Cmd_ExecuteActions(item, true); break;
   case 508: Cmd_ExecuteAction(item); break;
   case 601: Cmd_DeleteDependency(item); break;
   case 602: Cmd_DeleteDependencies(item); break;
   case 603: Cmd_AddDependency(item); break;
   case 701: Cmd_DeletePars(item); break;
   case 702: Cmd_AddNewPar(item); break;
   case 703: Cmd_DeletePar(item); break;
   case 704: Cmd_MemorizePar(item); break;
   case 705: Cmd_RememberPar(item); break;
   case 706: Cmd_MemorizePars(item); break;
   case 707: Cmd_RememberPars(item); break;
   case 801: Cmd_DeleteMinuitResult(item); break;
   case 904: Cmd_UpdateAllSlots(item); break;

   default:
      if ((id >= 110) && (id < 200))
         Cmd_AddNewData(item, id - 110);
      else if ((id >= 210) && (id < 300))
         Cmd_AddNewModel(item, id - 210);
      else if ((id >= 310) && (id < 400))
         Cmd_AssignModelToData(item, id - 310);
      else if ((id >= 510) && (id < 600))
         Cmd_AddNewAction(item, id - 510);
      else if ((id >= 1000) && (id < 2000))
         ExecutePopupForSlot(item, 0, id);
   }
}

void TGo4FitPanel::focusInEvent(QFocusEvent *event)
{
   UpdateStatusBar();
   QWidget::focusInEvent(event);
}

void TGo4FitPanel::changeEvent(QEvent *event)
{
   if (event->type() == QEvent::ActivationChange)
      UpdateStatusBar();
   QWidget::changeEvent(event);
}

// void TGo4FitPanel::windowActivationChange(bool OldActive)
//{
//    UpdateStatusBar();
//    QWidget::windowActivationChange(OldActive);
// }

void TGo4FitPanel::UpdatePFAmplLbl()
{
   TGo4FitPeakFinder *finder = GetPeakFinder(true);
   if (finder)
      PF_AmplLbl->setText(QString("Threshold: ") + QString::number(finder->Get0MaxAmplFactor() * 100) + "%");
   else
      PF_AmplLbl->setText("Threshold:");
   PF_AmplLbl->adjustSize();
}

void TGo4FitPanel::UpdatePFRelNoiseLbl()
{
   TGo4FitPeakFinder *finder = GetPeakFinder(true);

   if (finder)
      PF_RelNoiseLbl->setText(QString("Noise factor: ") + QString::number(finder->Get2NoiseFactor()));
   else
      PF_RelNoiseLbl->setText("Noise factor: ");
   PF_RelNoiseLbl->adjustSize();
}

void TGo4FitPanel::PF_MinWidthEdt_textChanged(const QString &str)
{
   if (fbFillingWidget)
      return;
   bool ok;
   double zn = str.toDouble(&ok);
   if (ok) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder)
         finder->Set0MinWidth(zn);
   }
}

void TGo4FitPanel::PF_MaxWidthEdt_textChanged(const QString &str)
{
   if (fbFillingWidget)
      return;
   bool ok;
   double zn = str.toDouble(&ok);
   if (ok) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder)
         finder->Set0MaxWidth(zn);
   }
}

void TGo4FitPanel::PF_WidthEdit_textChanged(const QString &str)
{
   if (fbFillingWidget)
      return;
   bool ok;
   double zn = str.toDouble(&ok);
   if (ok) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder)
         finder->Set1LineWidth(zn);
   }
}

void TGo4FitPanel::PF_AmplSlider_valueChanged(int zn)
{
   if (!fbFillingWidget) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder)
         finder->Set0MaxAmplFactor(1. - zn / 100.);
      UpdatePFAmplLbl();
   }
}

void TGo4FitPanel::PF_AmplSlider_sliderReleased()
{
   if (!fbFillingWidget)
      Button_PeakFinder();
}

void TGo4FitPanel::PF_RelNoiseSlider_valueChanged(int zn)
{
   if (!fbFillingWidget) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder)
         finder->Set2NoiseFactor((101 - zn) / 10.);
      UpdatePFRelNoiseLbl();
   }
}

void TGo4FitPanel::PF_RelNoiseSlider_sliderReleased()
{
   if (!fbFillingWidget)
      Button_PeakFinder();
}

void TGo4FitPanel::PF_MinNoiseEdit_textChanged(const QString &str)
{
   if (fbFillingWidget)
      return;
   bool ok;
   double zn = str.toDouble(&ok);
   if (ok) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder)
         finder->Set2NoiseMinimum(zn);
   }
}

void TGo4FitPanel::PF_SumUpSpin_valueChanged(int num)
{
   if (!fbFillingWidget) {
      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (finder) {
         finder->Set2ChannelSum(num);
         Button_PeakFinder();
      }
   }
}

void TGo4FitPanel::UpdateWizDataList()
{
   TGo4Fitter *fitter = GetFitter();

   fbFillingWidget = true;

   bool updatestack = false, updatepaint = false;

   Wiz_DataList->clear();
   if (!fitter)
      fxWizDataName = "";
   else {
      if (!Wiz_SelectedData() && (fitter->GetNumData() > 0)) {
         fxWizDataName = fitter->GetDataName(0);
         if (fiWizPageIndex < 1)
            updatestack = true;
         if (fiPaintMode == 0)
            updatepaint = true;
      }

      int selindx = -1;
      for (Int_t n = 0; n < fitter->GetNumData(); n++) {
         TGo4FitData *data = fitter->GetData(n);
         QListWidgetItem *item = new QListWidgetItem(data->GetName());
         if (strcmp(data->GetName(), fxWizDataName.toLatin1().constData()) == 0) {
            selindx = n;
            item->setSelected(true);
         }
         Wiz_DataList->addItem(item);
      }
      if (selindx < 0)
         fxWizDataName = "";
   }
   UpdateWizDataBtns();
   fbFillingWidget = false;

   if (updatestack) {
      fiWizPageIndex = 2;
      UpdateWizStackWidget();
   }
   if (updatepaint)
      UpdateWizPaint(1);
}

void TGo4FitPanel::UpdateWizDataBtns()
{
   TGo4Fitter *fitter = GetFitter();
   TGo4FitData *data = Wiz_SelectedData();

   Wiz_PFSetupBtn->setEnabled(data != nullptr);

   if (!WorkingWithPanel()) {
      Wiz_RebuildDataBtn->setVisible(false);
      Wiz_AddDataBtn->setVisible(true);
      Wiz_DelDataBtn->setVisible(true);
      Wiz_AddDataBtn->setEnabled(fitter != nullptr);
      Wiz_DelDataBtn->setEnabled(data != nullptr);
   } else {
      Wiz_RebuildDataBtn->setVisible(true);
      Wiz_RebuildDataBtn->setEnabled(fitter != nullptr);
      Wiz_AddDataBtn->setVisible(false);
      Wiz_DelDataBtn->setVisible(false);
   }
}

void TGo4FitPanel::UpdateWizModelsList(bool changestack)
{
   fbFillingWidget = true;

   Wiz_ModelList->clear();

   TGo4Fitter *fitter = GetFitter();

   Wiz_ShowAllMod->setChecked(fbWizShowAllModels);
   int selindx = -1;
   if (fitter) {
      TGo4FitData *data = Wiz_SelectedData();
      int indx = 0;
      for (Int_t n = 0; n < fitter->GetNumModel(); n++) {
         TGo4FitModel *model = fitter->GetModel(n);
         if (data && !fbWizShowAllModels)
            if (!model->IsAssignTo(data->GetName()))
               continue;

         bool assign = false;
         if (data && model->IsAssignTo(data->GetName()))
            assign = true;
         QListWidgetItem *item = new QListWidgetItem(model->GetName());
         item->setCheckState(assign ? Qt::Checked : Qt::Unchecked);
         if (strcmp(model->GetName(), fxWizModelName.toLatin1().constData()) == 0) {
            selindx = indx;
            item->setSelected(true);
         }
         Wiz_ModelList->addItem(item);
         indx++;
      }

      if (selindx >= 0)
         Wiz_ModelList->setCurrentRow(selindx);

      if (selindx < 0)
         fxWizModelName = "";
      //                 else Wiz_ModelList->ensureCurrentVisible();

   } else {
      fxWizModelName = "";
   }

   if (changestack) {
      if (selindx >= 0) {
         fiWizPageIndex = 1;
      } else if (fiWizPageIndex == 1) {
         fiWizPageIndex = 0;
      }
   }

   UpdateWizModelsBtns();

   fbFillingWidget = false;

   if (changestack)
      UpdateWizStackWidget();
}

void TGo4FitPanel::UpdateWizModelsBtns()
{
   TGo4Fitter *fitter = GetFitter();
   TGo4FitModel *model = Wiz_SelectedModel();

   Wiz_ShowAllMod->setVisible(!WorkingWithOnlyPad());

   if (fitter) {
      Wiz_AddModelBtn->setEnabled(true);
      Wiz_DelModelBtn->setEnabled(model != nullptr);
      Wiz_CloneModelBtn->setEnabled(model != nullptr);
      Wiz_ShowAllMod->setEnabled(true);
   } else {
      Wiz_AddModelBtn->setEnabled(false);
      Wiz_DelModelBtn->setEnabled(false);
      Wiz_CloneModelBtn->setEnabled(false);
      Wiz_ShowAllMod->setEnabled(false);
   }
}

void TGo4FitPanel::UpdateWizStackWidget()
{
   fbFillingWidget = true;
   TGo4Fitter *fitter = GetFitter();

   int indx = -1;
   if (!fitter)
      fiWizPageIndex = -1;
   else
      indx = fiWizPageIndex;

   QWidget *target = nullptr;

   switch (indx) {
   case -1: target = Wiz_EmptyPage; break;
   case 0: {
      target = Wiz_FitterPage;

      int typ = fitter->GetFitFunctionType();
      if (typ == TGo4Fitter::ff_user)
         typ = TGo4Fitter::ff_least_squares;
      Wiz_FitFuncCmb->setCurrentIndex(typ);

      Wiz_UseAmplEstimChk->setChecked(fbUseAmplEstim);
      Wiz_MigradIterSpin->setValue(fiNumMigradIter);

      break;
   }

   case 1: {
      TGo4FitModel *model = Wiz_SelectedModel();
      if (!model)
         break;

      target = Wiz_ModelPage;

      QString modelinfo;
      Wiz_GetModelInfo(model, &modelinfo);
      Wiz_ModelInfoLbl->setText(modelinfo);

      Wiz_BackgroundChk->setChecked(model->GetGroupIndex() == 0);

      Wiz_ModelBufChk->setChecked(model->GetUseBuffers());
      Wiz_ModelBufChk->setEnabled(fiBuffersUsage == 3);

      FillParsTable(Wiz_ParTable, fitter, model, false, fxWizPars);

      break;
   }

   case 2: {
      TGo4FitData *data = Wiz_SelectedData();
      if (!data)
         break;

      target = Wiz_DataPage;

      QString datainfo("Data: ");
      datainfo += data->GetName();
      datainfo += "  of class: ";
      datainfo += data->ClassName();
      if (fiIntegralMode == 1) {
         datainfo += "\nCounts=";
         double v = fitter->CalculatesIntegral(data->GetName(), 0, kTRUE);
         datainfo += QString::number(v);
      }
      if (fiIntegralMode == 2) {
         datainfo += "\nIntegral=";
         double v = fitter->CalculatesIntegral(data->GetName(), 0, kFALSE);
         datainfo += QString::number(v);
      }
      Wiz_DataInfoLbl->setText(datainfo);

      QString models("Models:");
      for (int n = 0; n < fitter->GetNumModel(); n++) {
         TGo4FitModel *model = fitter->GetModel(n);
         if (model->IsAssignTo(data->GetName())) {
            if (models.length() > 45) {
               models += " ...";
               break;
            }
            models += " ";
            models += model->GetName();
         }
      }
      Wiz_DataModelsLbl->setText(models);

      Wiz_DataBufChk->setChecked((fiBuffersUsage == 1) || (fiBuffersUsage == 2) ||
                                 ((fiBuffersUsage == 3) && data->GetUseBuffers()));
      Wiz_DataBufChk->setEnabled(fiBuffersUsage == 3);

      fxWizSlots->Clear();
      data->FillSlotList(fxWizSlots);

      Wiz_DataSlotsTable->setRowCount(fxWizSlots->GetLast() + 1);

      for (int n = 0; n <= fxWizSlots->GetLast(); n++) {
         TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(fxWizSlots->At(n));
         if (!slot)
            continue;

         Wiz_DataSlotsTable->setVerticalHeaderItem(n, new QTableWidgetItem(slot->GetName()));
         TObject *obj = slot->GetObject();

         if (!obj) {
            Wiz_DataSlotsTable->setItem(n, 0, new QTableWidgetItem(" --- "));
            Wiz_DataSlotsTable->setItem(n, 1, new QTableWidgetItem(slot->GetClass()->GetName()));
            Wiz_DataSlotsTable->setItem(n, 2, new QTableWidgetItem("false"));
         } else {
            Wiz_DataSlotsTable->setItem(n, 0, new QTableWidgetItem(obj->GetName()));
            Wiz_DataSlotsTable->setItem(n, 1, new QTableWidgetItem(obj->ClassName()));
            if (slot->GetOwned())
               Wiz_DataSlotsTable->setItem(n, 2, new QTableWidgetItem("true"));
            else
               Wiz_DataSlotsTable->setItem(n, 2, new QTableWidgetItem("false"));
         }

         Wiz_DataSlotsTable->setItem(n, 3, new QTableWidgetItem(Wiz_GetSlotSourceInfo(slot)));
      }

      Wiz_DataSlotsTable->resizeColumnsToContents();

      Wiz_DataUseRangeBtn->setEnabled(FindPadWhereData(data) != nullptr);
      if (data)
         Wiz_DataClearRangesBtn->setEnabled(data->IsAnyRangeLimits());
      Wiz_DrawDataBtn->setEnabled(data != nullptr);

      break;
   }
   case 3: {
      target = Wiz_PFPage;

      TGo4FitPeakFinder *finder = GetPeakFinder(true);
      if (!finder)
         break;

      UpdateStatusBar();

      FindersTab->setCurrentIndex(finder->GetPeakFinderType());

      Wiz_PFUsePolynChk->setChecked(finder->GetUsePolynom());
      Wiz_PFPolynSpin->setEnabled(finder->GetUsePolynom());
      Wiz_PFPolynSpin->setValue(finder->GetPolynomOrder());

      PF_MinWidthEdt->setText(QString::number(finder->Get0MinWidth()));
      PF_MaxWidthEdt->setText(QString::number(finder->Get0MaxWidth()));
      PF_AmplSlider->setValue(int(100 * (1. - finder->Get0MaxAmplFactor())));
      UpdatePFAmplLbl();

      PF_WidthEdit->setText(QString::number(finder->Get1LineWidth()));

      int zn = int(10 * finder->Get2NoiseFactor()); // from 1..100
      PF_RelNoiseSlider->setValue(101 - zn);
      UpdatePFRelNoiseLbl();
      PF_MinNoiseEdit->setText(QString::number(finder->Get2NoiseMinimum()));
      PF_SumUpSpin->setValue(finder->Get2ChannelSum());

      break;
   }
   }

   if (target)
      Wiz_Stack->setCurrentWidget(target);
   else
      Wiz_Stack->setCurrentWidget(Wiz_EmptyPage);

   fbFillingWidget = false;
}

void TGo4FitPanel::UpdateWizPaint(int mode)
{
   RemovePrimitives();

   if (!fbShowPrimitives)
      return;

   if (mode >= 0)
      fiPaintMode = mode;

   TGo4Fitter *fitter = GetFitter();
   TGo4FitModel *model = Wiz_SelectedModel();
   TGo4FitData *data = Wiz_SelectedData();

   if (fitter && data && (fiPaintMode == 1)) {
      TPad *pad = FindPadWhereData(data);

      if (PaintModelsFor(fitter, data, 0, false))
         for (Int_t n = 0; n < data->GetNumRangeCondition(); n++)
            PaintRange(data, n, pad, 0);
   }

   if (fitter && model && (fiPaintMode == 2)) {
      for (Int_t n = 0; n < model->NumAssigments(); n++) {
         data = fitter->FindData(model->AssignmentName(n));
         if (data && !data->IsAnyDataTransform()) {
            TPad *pad = FindPadWhereData(data);
            PaintModel(model, pad, 0);
         }
      }
   }
}

void TGo4FitPanel::Wiz_DataListSelect(QListWidgetItem *item)
{
   if (fbFillingWidget || !item)
      return;
   QString name = item->text();

   if ((name == fxWizDataName) && (fiWizPageIndex == 2))
      return;
   fxWizDataName = name;
   fiWizPageIndex = 2;
   UpdateWizDataBtns();
   UpdateWizModelsList(false);
   UpdateWizStackWidget();
   UpdateWizPaint(1);
}

void TGo4FitPanel::Wiz_ModelListSelect(QListWidgetItem *item)
{
   if (fbFillingWidget || !item)
      return;

   QString name = item->text();

   bool needupdate = ((name != fxWizModelName) || (fiWizPageIndex != 1));

   fxWizModelName = name;
   fiWizPageIndex = 1;

   TGo4Fitter *fitter = GetFitter();
   TGo4FitModel *model = Wiz_SelectedModel();
   TGo4FitData *data = Wiz_SelectedData();
   if (fitter && data && model) {
      bool wasassigned = model->IsAssignTo(data->GetName());
      bool isassigned = (item->checkState() == Qt::Checked);

      if (wasassigned != isassigned) {
         if (wasassigned)
            fitter->ClearModelAssignmentTo(model->GetName(), data->GetName());
         else
            fitter->AssignModelTo(model->GetName(), data->GetName());
         needupdate = true;
      }
   }

   if (needupdate) {
      UpdateWizModelsBtns();
      UpdateWizStackWidget();
      UpdateWizPaint(2);
   }
}

void TGo4FitPanel::Wiz_AddDataBtn_clicked()
{
   if (fbFillingWidget)
      return;

   QMenu menu;

   FillDataTypesList(&menu, 0, "wiz_data");

   menu.exec(Wiz_AddDataBtn->mapToGlobal(QPoint(5, 5)));
}

void TGo4FitPanel::Wiz_DelDataBtn_clicked()
{
   if (fbFillingWidget)
      return;

   if (Wiz_RemoveData()) {
      UpdateWizDataList();
      UpdateWizModelsList(false);
      fiWizPageIndex = 0;
      UpdateWizStackWidget();
      UpdateWizPaint(0);
      UpdateStatusBar();
   }
}

void TGo4FitPanel::Wiz_AddModelBtn_clicked()
{
   if (fbFillingWidget)
      return;

   QMenu menu;

   FillModelTypesList(&menu, 0, true, "wiz_model");

   menu.exec(Wiz_AddModelBtn->mapToGlobal(QPoint(5, 5)));
}

void TGo4FitPanel::Wiz_DelModelBtn_clicked()
{
   if (fbFillingWidget)
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   if (checkConfirm("Delete models", "Are you sure to delete selected models"))
      return;

   for (unsigned n = 0; n < Wiz_ModelList->count(); n++) {
      if (!Wiz_ModelList->item(n)->isSelected())
         continue;
      QString name = Wiz_ModelList->item(n)->text();
      fitter->RemoveModel(name.toLatin1().constData(), kTRUE);
   }

   fxWizModelName = "";
   UpdateWizModelsList(true);
   UpdateWizPaint(0);
   UpdateStatusBar();
}

void TGo4FitPanel::Wiz_CloneModelBtn_clicked()
{
   if (fbFillingWidget)
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   for (unsigned n = 0; n < Wiz_ModelList->count(); n++) {
      if (!Wiz_ModelList->item(n)->isSelected())
         continue;
      QString name = Wiz_ModelList->item(n)->text();
      fitter->CloneModel(name.toLatin1().constData());
   }

   fxWizModelName = "";
   UpdateWizModelsList(true);
   UpdateWizPaint(0);
   UpdateStatusBar();
}

void TGo4FitPanel::Wiz_ShowAllMod_toggled(bool zn)
{
   if (fbFillingWidget)
      return;
   fbWizShowAllModels = zn;
   UpdateWizModelsList(true);
   UpdateWizPaint(2);
}

void TGo4FitPanel::Wiz_FitFuncCmb_activated(int typ)
{
   if (!fbFillingWidget && GetFitter())
      GetFitter()->SetFitFunctionType(typ);
}

void TGo4FitPanel::Wiz_FitNameEdt_textChanged(const QString &name)
{
   TGo4Fitter *fitter = GetFitter();
   if (fbFillingWidget || !fitter || name.isEmpty())
      return;
   fitter->SetName(name.toLatin1().constData());

   fiWizPageIndex = 0;
   UpdateWizStackWidget();
   UpdateWizPaint(0);

   UpdateStatusBar();
}

void TGo4FitPanel::Wiz_ParTable_valueChanged(int nrow, int ncol)
{
   ParsTableChanged(Wiz_ParTable, nrow, ncol, false, fxWizPars, true);
}

void TGo4FitPanel::Wiz_DataList_doubleClicked(QListWidgetItem *)
{
   TGo4Fitter *fitter = GetFitter();
   TGo4FitData *data = Wiz_SelectedData();
   if (!data || !fitter)
      return;
   bool ok = true;
   QString newname =
      QInputDialog::getText(this, "Change data name", "Input new name", QLineEdit::Normal, data->GetName(), &ok);
   if (ok && (newname.length() > 0) && (newname != data->GetName())) {
      if (fitter->FindData(newname.toLatin1().constData())) {
         QMessageBox::information(this, "Fit panel",
                                  QString("Unable to rename data.\n Name ") + newname + " already exists");
         return;
      }
      fitter->ChangeDataNameInAssignments(data->GetName(), newname.toLatin1().constData());
      data->SetName(newname.toLatin1().constData());
      fxWizDataName = newname;
      UpdateWizDataList();
      UpdateWizPaint(1);
   }
}

void TGo4FitPanel::Wiz_ModelList_doubleClicked(QListWidgetItem *)
{
   TGo4Fitter *fitter = GetFitter();
   TGo4FitModel *model = Wiz_SelectedModel();
   if (!model || !fitter)
      return;
   bool ok = true;
   QString newname =
      QInputDialog::getText(this, "Change model name", "Input new name", QLineEdit::Normal, model->GetName(), &ok);
   if (ok && (newname.length() > 0) && (newname != model->GetName())) {
      if (fitter->FindModel(newname.toLatin1().constData())) {
         QMessageBox::information(this, "Fit panel",
                                  QString("Unable to rename model.\n Name ") + newname + " already exists\n");
         return;
      }
      model->SetName(newname.toLatin1().constData());
      fxWizModelName = newname;
      UpdateWizModelsList(true);
      UpdateWizPaint(2);
   }
}

void TGo4FitPanel::Wiz_ModelList_itemChanged(QListWidgetItem *item)
{
   if (fbFillingWidget)
      return;

   QString name = item->text();

   bool checked = (item->checkState() == Qt::Checked);

   bool needupdate = ((name != fxWizModelName) || (fiWizPageIndex != 1));

   fxWizModelName = name;
   fiWizPageIndex = 1;

   TGo4Fitter *fitter = GetFitter();
   TGo4FitModel *model = Wiz_SelectedModel();
   TGo4FitData *data = Wiz_SelectedData();
   if (fitter && data && model) {
      bool wasassigned = model->IsAssignTo(data->GetName());

      if (wasassigned != checked) {
         if (wasassigned)
            fitter->ClearModelAssignmentTo(model->GetName(), data->GetName());
         else
            fitter->AssignModelTo(model->GetName(), data->GetName());
         needupdate = true;
      }
   }

   if (needupdate) {
      UpdateWizModelsBtns();
      UpdateWizStackWidget();
      UpdateWizPaint(2);
   }
}

void TGo4FitPanel::Wiz_RebuildDataBtn_clicked()
{
   Wiz_RebuildDataList();
}

void TGo4FitPanel::Wiz_DataBufChk_toggled(bool zn)
{
   if (fbFillingWidget)
      return;
   TGo4FitData *data = Wiz_SelectedData();
   if (data)
      data->SetUseBuffers(zn);
}

void TGo4FitPanel::Wiz_UseAmplEstimChk_toggled(bool chk)
{
   if (!fbFillingWidget)
      fbUseAmplEstim = chk;
}

void TGo4FitPanel::Wiz_MigradIterSpin_valueChanged(int num)
{
   if (!fbFillingWidget)
      fiNumMigradIter = num;
}

void TGo4FitPanel::Wiz_DataSlotsTable_contextMenuRequested(const QPoint &pnt)
{
   if (fbFillingWidget || !fxWizSlots)
      return;

   QTableWidgetItem *item = Wiz_DataSlotsTable->itemAt(pnt);

   int nrow = item ? item->row() : -1;

   if ((nrow < 0) || (nrow > fxWizSlots->GetLast()))
      return;

   TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(fxWizSlots->At(nrow));
   if (!slot)
      return;

   QMenu menu;

   if (FillPopupForSlot(slot, &menu))
      menu.exec(Wiz_DataSlotsTable->mapToGlobal(pnt));
}

void TGo4FitPanel::Wiz_DataUseRangeBtn_clicked()
{
   if (!fbFillingWidget)
      Wiz_UseSelectedRange();
}

void TGo4FitPanel::Wiz_DataClearRangesBtn_clicked()
{
   if (fbFillingWidget)
      return;
   TGo4FitData *data = Wiz_SelectedData();
   if (data) {
      data->ClearRanges();
      UpdateActivePage();
   }
}

void TGo4FitPanel::Wiz_DrawDataBtn_clicked()
{
   if (fbFillingWidget)
      return;
   if (Wiz_SelectedData())
      Button_FitterDraw(Wiz_SelectedData());
}

void TGo4FitPanel::MainAttachBtn_clicked()
{
   UpdateStatusBar();

   Button_WorkWithPanel();
}

void TGo4FitPanel::MainFitBtn_clicked()
{
   QApplication::setOverrideCursor(Qt::WaitCursor);

   Button_PerformFit();

   QApplication::restoreOverrideCursor();
}

void TGo4FitPanel::MainDrawBtn_clicked()
{
   Button_FitterDraw(nullptr);
}

void TGo4FitPanel::MainFindBtn_clicked()
{
   if (GetFitter())
      Button_PeakFinder();
}

void TGo4FitPanel::MainParsBtn_clicked()
{
   fbParsWidgetShown = !fbParsWidgetShown;

   UpdateActivePage();
}

void TGo4FitPanel::FillParsWidget()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;
   fbFillingWidget = true;

   Double_t FF = fitter->GetResultFF();
   Int_t NDF = fitter->GetResultNDF();
   if ((FF == 0.) && (NDF == 0))
      Par_FitterResLbl->hide();
   else {
      Par_FitterResLbl->setText("Result: Fit func = " + QString::number(FF) + "  NDF = " + QString::number(NDF));
      Par_FitterResLbl->show();
   }

   FillParsTable(ParsTable, fitter, 0, LineParsChk->isChecked(), fxParsTableList);
   fbFillingWidget = false;
}

void TGo4FitPanel::LineParsChk_toggled(bool)
{
   if (fbFillingWidget)
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;
   fbFillingWidget = true;
   FillParsTable(ParsTable, fitter, 0, LineParsChk->isChecked(), fxParsTableList);
   WidgetStack->setCurrentWidget(PageParameters);
   fbFillingWidget = false;
}

void TGo4FitPanel::ParsTable_valueChanged(int nrow, int ncol)
{
   if (!fbFillingWidget)
      ParsTableChanged(ParsTable, nrow, ncol, LineParsChk->isChecked(), fxParsTableList, false);
}

void TGo4FitPanel::FillParsTable(QTableWidget *table, TGo4Fitter *fitter, TGo4FitModel *model, bool LinesView,
                                 TObjArray *TableList)
{
   if (!table || !fitter || !TableList)
      return;

   if (model && LinesView)
      return;
   TableList->Clear();

   /*   for (int nr = 0; nr < table->rowCount(); nr++)
        for (int nc = 0; nc < table->columnCount(); nc++) {
           QTableWidgetItem* item = table->item(nr,nc);
           if (item) {
              item->setFlags(item->flags() | Qt::ItemIsEnabled);
              if (nc == 0) item->setText("");
           }
        }
   */

   if (LinesView) {
      int MaxAxis = 0;
      for (Int_t n = 0; n < fitter->GetNumModel(); n++) {
         TGo4FitModel *m = fitter->GetModel(n);
         if (!m)
            continue;
         Double_t zn;
         for (int naxis = 0; naxis < 10; naxis++)
            if (m->GetPosition(naxis, zn) || m->GetWidth(naxis, zn))
               MaxAxis = naxis;
      }

      int nfcol = (fiIntegralMode > 0) ? 3 : 1;
      table->setColumnCount(nfcol + (MaxAxis + 1) * 2);
      table->setHorizontalHeaderItem(0, new QTableWidgetItem("Amplitude"));

      QString capt;
      switch (fiIntegralMode) {
      case 1: capt = "Counts"; break;
      case 2: capt = "Integral"; break;
      case 3: capt = "Gauss Int"; break;
      }
      if (!capt.isEmpty()) {
         table->setHorizontalHeaderItem(1, new QTableWidgetItem(capt));
         table->setHorizontalHeaderItem(2, new QTableWidgetItem("Error"));
      }

      for (int naxis = 0; naxis <= MaxAxis; naxis++) {
         capt = QString("Position ") + QString::number(naxis);
         table->setHorizontalHeaderItem(nfcol + naxis * 2, new QTableWidgetItem(capt));
         if (fbRecalculateGaussWidth)
            capt = QString("FWHM ");
         else
            capt = QString("Sigma ");
         capt += QString::number(naxis);
         table->setHorizontalHeaderItem(nfcol + 1 + naxis * 2, new QTableWidgetItem(capt));
      }

      table->setRowCount(fitter->GetNumModel());

      for (Int_t n = 0; n < fitter->GetNumModel(); n++) {
         TGo4FitModel *m = fitter->GetModel(n);
         TableList->Add(m);

         bool recalculatew =
            (m->InheritsFrom(TGo4FitModelGauss1::Class()) || m->InheritsFrom(TGo4FitModelGauss2::Class()) ||
             m->InheritsFrom(TGo4FitModelGaussN::Class())) &&
            fbRecalculateGaussWidth;
         double widthk = recalculatew ? 2.3548 : 1.0;

         table->setVerticalHeaderItem(n, new QTableWidgetItem(m->GetName()));

         QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(m->GetAmplValue()));

         TGo4FitParameter *amplpar = m->GetAmplPar();
         if (!amplpar)
            item0->setFlags(item0->flags() & ~Qt::ItemIsEnabled);
         table->setItem(n, 0, item0);

         if (fiIntegralMode > 0) {
            Double_t v = 0.;
            TGo4FitData *data = Wiz_SelectedData();
            switch (fiIntegralMode) {
            case 1:
               if (data)
                  v = fitter->CalculatesIntegral(data->GetName(), m->GetName(), kTRUE);
               else
                  v = fitter->CalculatesModelIntegral(m->GetName(), kTRUE);
               break;
            case 2:
               if (data)
                  v = fitter->CalculatesIntegral(data->GetName(), m->GetName(), kFALSE);
               else
                  v = fitter->CalculatesModelIntegral(m->GetName(), kFALSE);
               break;
            case 3: v = m->Integral(); break;
            default: v = 0;
            }

            QTableWidgetItem *item1;
            if ((v <= 0.) && (fiIntegralMode == 3))
               item1 = new QTableWidgetItem("---");
            else
               item1 = new QTableWidgetItem(QString::number(v));
            item1->setFlags(item1->flags() & ~Qt::ItemIsEnabled);
            table->setItem(n, 1, item1);

            QTableWidgetItem *item2;
            if (!amplpar || (amplpar->GetValue() == 0) || (amplpar->GetError() == 0) ||
                ((v <= 0.) && (fiIntegralMode == 3)))
               item2 = new QTableWidgetItem("---");
            else
               item2 = new QTableWidgetItem(QString::number(amplpar->GetError() / amplpar->GetValue() * v));
            item2->setFlags(item2->flags() & ~Qt::ItemIsEnabled);
            table->setItem(n, 2, item2);
         }

         for (int naxis = 0; naxis <= MaxAxis; naxis++) {

            Double_t pos, width;

            QTableWidgetItem *item;

            if (m->GetPosition(naxis, pos)) {
               item = new QTableWidgetItem(QString::number(pos));
            } else {
               item = new QTableWidgetItem("---");
               item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            }

            table->setItem(n, nfcol + naxis * 2, item);

            if (m->GetWidth(naxis, width)) {
               item = new QTableWidgetItem(QString::number(width * widthk));
            } else {
               item = new QTableWidgetItem("---");
               item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            }
            table->setItem(n, nfcol + 1 + naxis * 2, item);
         }
      }

   } else {

      TGo4FitParsList *pars = !model ? dynamic_cast<TGo4FitParsList *>(fitter) : dynamic_cast<TGo4FitParsList *>(model);
      if (!pars)
         return;

      for (Int_t n = 0; n < pars->NumPars(); n++)
         TableList->Add(pars->GetPar(n));

      table->setColumnCount(6);
      table->setHorizontalHeaderItem(0, new QTableWidgetItem("Fixed"));
      table->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
      table->setHorizontalHeaderItem(2, new QTableWidgetItem("Error"));
      table->setHorizontalHeaderItem(3, new QTableWidgetItem("Epsilon"));
      table->setHorizontalHeaderItem(4, new QTableWidgetItem("Min"));
      table->setHorizontalHeaderItem(5, new QTableWidgetItem("Max"));

      table->setRowCount(TableList->GetLast() + 1);

      for (Int_t n = 0; n <= TableList->GetLast(); n++) {
         TGo4FitParameter *par = dynamic_cast<TGo4FitParameter *>(TableList->At(n));
         if (!par)
            continue;

         if (model)
            table->setVerticalHeaderItem(n, new QTableWidgetItem(par->GetName()));
         else
            table->setVerticalHeaderItem(n, new QTableWidgetItem(par->GetFullName()));

         QTableWidgetItem *checkitem = new QTableWidgetItem("fix");
         checkitem->setCheckState(par->GetFixed() ? Qt::Checked : Qt::Unchecked);
         table->setItem(n, 0, checkitem);

         QTableWidgetItem *item = new QTableWidgetItem(QString::number(par->GetValue()));
         if (par->GetFixed())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
         table->setItem(n, 1, item);

         table->setItem(n, 2, new QTableWidgetItem(QString::number(par->GetError())));

         Double_t eps, min, max;
         if (par->GetEpsilon(eps))
            table->setItem(n, 3, new QTableWidgetItem(QString::number(eps)));
         else
            table->setItem(n, 3, new QTableWidgetItem(""));

         if (par->GetRange(min, max)) {
            table->setItem(n, 4, new QTableWidgetItem(QString::number(min)));
            table->setItem(n, 5, new QTableWidgetItem(QString::number(max)));
         } else {
            table->setItem(n, 4, new QTableWidgetItem(""));
            table->setItem(n, 5, new QTableWidgetItem(""));
         }
      }
   }
   table->resizeColumnsToContents();
   table->resizeRowsToContents();

   // table->verticalHeader()->adjustHeaderSize();
   // table->horizontalHeader()->adjustHeaderSize();
}

void TGo4FitPanel::ParsTableChanged(QTableWidget *table, int nrow, int ncol, bool LinesView, TObjArray *TableList,
                                    bool updatepaint)
{
   if (fbFillingWidget || !table || !TableList)
      return;

   if ((nrow < 0) || (nrow > TableList->GetLast()))
      return;

   if (LinesView) {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(TableList->At(nrow));
      if (!model)
         return;

      bool recalculatew =
         (model->InheritsFrom(TGo4FitModelGauss1::Class()) || model->InheritsFrom(TGo4FitModelGauss2::Class()) ||
          model->InheritsFrom(TGo4FitModelGaussN::Class())) &&
         fbRecalculateGaussWidth;
      double widthk = recalculatew ? 2.3548 : 1.0;

      bool ok = false;
      double zn = table->item(nrow, ncol)->text().toDouble(&ok);
      if (!ok)
         return;

      if (ncol == 0)
         model->SetAmplValue(zn);
      else {
         int nfcol = (fiIntegralMode > 0) ? 2 : 1;
         int naxis = (ncol - nfcol) / 2;
         if ((ncol - nfcol) % 2 == 0)
            model->SetPosition(naxis, zn);
         else
            model->SetWidth(naxis, zn / widthk);
      }

   } else {

      TGo4FitParameter *par = dynamic_cast<TGo4FitParameter *>(TableList->At(nrow));
      if (!par)
         return;

      switch (ncol) {
      case 0: {
         if (table->item(nrow, 0)->checkState() == Qt::Checked)
            par->SetFixed(kTRUE);
         else if (table->item(nrow, 0)->checkState() == Qt::Unchecked)
            par->SetFixed(kFALSE);
         if (par->GetFixed())
            table->item(nrow, 1)->setFlags(table->item(nrow, 1)->flags() & ~Qt::ItemIsEnabled);
         else
            table->item(nrow, 1)->setFlags(table->item(nrow, 1)->flags() | Qt::ItemIsEnabled);
         break;
      }
      case 1: {
         if (par->GetFixed())
            break;
         bool ok = false;
         double zn = table->item(nrow, 1)->text().toDouble(&ok);
         if (ok) {
            par->SetValue(zn);
            if (updatepaint)
               UpdateWizPaint(2);
         }
         break;
      }
      case 2: {
         if (par->GetFixed())
            break;
         bool ok = false;
         double zn = table->item(nrow, 2)->text().toDouble(&ok);
         if (ok)
            par->SetError(zn);
         break;
      }
      case 3: {
         bool ok = false;
         double zn = table->item(nrow, 3)->text().toDouble(&ok);
         if (ok)
            par->SetEpsilon(zn);
         break;
      }
      case 4:
      case 5: {
         std::cout << "Changing col " << ncol << std::endl;

         bool ok = false;
         bool range_changed = false;
         double zn = table->item(nrow, ncol)->text().toDouble(&ok);
         if (ok) {

            bool changemin = false;
            bool changemax = false;

            Double_t min, max;
            if (!par->GetRange(min, max)) {
               double vvv = par->GetValue();
               if (ncol == 4) {
                  min = zn;
                  if (min < vvv)
                     max = 2 * vvv - min;
                  else
                     max = min;
                  changemax = true;
               } else {
                  max = zn;
                  if (max > vvv)
                     min = 2 * vvv - max;
                  else
                     min = max;
                  changemin = true;
               }
            } else if (ncol == 4) {
               min = zn;
               if (max < min) {
                  max = min;
                  changemax = true;
               }
            } else if (ncol == 5) {
               max = zn;
               if (min > max) {
                  min = max;
                  changemin = true;
               }
            }
            par->SetRange(min, max);
            range_changed = true;
            fbFillingWidget = true;
            if (changemin)
               table->item(nrow, 4)->setText(QString::number(min));
            if (changemax)
               table->item(nrow, 5)->setText(QString::number(max));
            fbFillingWidget = false;
         } else {
            if (table->item(nrow, ncol)->text().isEmpty() && table->item(nrow, 9 - ncol)->text().isEmpty()) {
               par->ClearRange();
               range_changed = true;
            }
         }

         // reset first columns of pars list when we touch range values
         if (range_changed) {
            fbFillingWidget = true;
            table->item(nrow, 0)->setCheckState(par->GetFixed() ? Qt::Checked : Qt::Unchecked);
            if (par->GetFixed())
               table->item(nrow, 1)->setFlags(table->item(nrow, 1)->flags() & ~Qt::ItemIsEnabled);
            else
               table->item(nrow, 1)->setFlags(table->item(nrow, 1)->flags() | Qt::ItemIsEnabled);
            table->item(nrow, 1)->setText(QString::number(par->GetValue()));
            fbFillingWidget = false;
         }

         break;
      }
      }
   }
}

void TGo4FitPanel::Wiz_ModelBufChk_toggled(bool zn)
{
   if (fbFillingWidget)
      return;
   TGo4FitModel *model = Wiz_SelectedModel();
   if (model)
      model->SetUseBuffers(zn);
}

void TGo4FitPanel::Wiz_PFSetupBtn_clicked()
{
   if (fbFillingWidget || !Wiz_SelectedData())
      return;

   fiWizPageIndex = 3;
   UpdateWizStackWidget();
   UpdateWizPaint(1);
}

void TGo4FitPanel::Wiz_PFUsePolynChk_toggled(bool zn)
{
   if (fbFillingWidget)
      return;

   TGo4FitPeakFinder *finder = GetPeakFinder(true);
   if (finder)
      finder->SetUsePolynom(zn);

   Wiz_PFPolynSpin->setEnabled(zn);
}

void TGo4FitPanel::Wiz_PFPolynSpin_valueChanged(int zn)
{
   if (fbFillingWidget)
      return;

   TGo4FitPeakFinder *finder = GetPeakFinder(true);
   if (finder)
      finder->SetPolynomOrder(zn);
}

void TGo4FitPanel::Smp_PolynomBtn_clicked()
{
   Button_SimpleFit(0);
}

void TGo4FitPanel::Smp_GaussianBtn_clicked()
{
   Button_SimpleFit(1);
}

void TGo4FitPanel::Smp_LorenzBtn_clicked()
{
   Button_SimpleFit(2);
}

void TGo4FitPanel::Smp_ExponentBtn_clicked()
{
   Button_SimpleFit(3);
}

void TGo4FitPanel::Smp_ClearBtn_clicked()
{
   Button_SimpleClear();
}

void TGo4FitPanel::Wiz_MinSetupBtn_clicked()
{
   if (fbFillingWidget)
      return;

   fiWizPageIndex = 0;
   UpdateWizStackWidget();
   UpdateWizPaint(0);
}

void TGo4FitPanel::FindersTab_currentChanged(int)
{
   if (fbFillingWidget)
      return;

   TGo4FitPeakFinder *finder = GetPeakFinder(true);
   if (finder)
      finder->SetPeakFinderType(FindersTab->currentIndex());
}

void TGo4FitPanel::Wiz_BackgroundChk_toggled(bool chk)
{
   if (fbFillingWidget)
      return;

   TGo4FitModel *model = Wiz_SelectedModel();
   if (model) {
      if (chk)
         model->SetBackgroundGroupIndex();
      else
         model->SetGroupIndex(-1);
   }
}

void TGo4FitPanel::PF_MinWidthEdt_returnPressed()
{
   if (!fbFillingWidget)
      Button_PeakFinder();
}

void TGo4FitPanel::PF_MaxWidthEdt_returnPressed()
{
   if (!fbFillingWidget)
      Button_PeakFinder();
}

void TGo4FitPanel::PF_WidthEdit_returnPressed()
{
   if (!fbFillingWidget)
      Button_PeakFinder();
}

void TGo4FitPanel::PF_MinNoiseEdit_returnPressed()
{
   if (!fbFillingWidget)
      Button_PeakFinder();
}

TGo4Fitter *TGo4FitPanel::CreateFitterFor(TGo4ViewPanel *panel, TPad *pad, const char *name)
{
   if (!panel || !pad)
      return nullptr;

   TGo4Fitter *fitter = new TGo4Fitter(name, "Fitter object");

   CreateDataFor(panel, pad, fitter);

   fitter->AddStandardActions();

   fitter->SetMemoryUsage(3);

   fitter->SetFitFunctionType(TGo4Fitter::ff_chi_square);

   return fitter;
}

void TGo4FitPanel::CreateDataFor(TGo4ViewPanel *panel, TPad *pad, TGo4Fitter *fitter)
{
   if (!fitter || !panel || !pad)
      return;

   int npads = panel->GetNumberOfPads(pad);

   for (Int_t n = 0; n < npads; n++) {
      TPad *subpad = panel->GetSubPad(pad, n, false);
      if (panel->IsPadHasSubPads(subpad))
         continue;
      TObject *obj = panel->GetPadMainObject(subpad);
      if (!obj)
         continue;

      TGo4FitData *data = nullptr;

      if (obj->InheritsFrom(TH1::Class()))
         data = new TGo4FitDataHistogram(fitter->FindNextName("Data", 0, kFALSE));
      else if (obj->InheritsFrom(TGraph::Class()))
         data = new TGo4FitDataGraph(fitter->FindNextName("Data", 0, kFALSE));

      if (data)
         fitter->AddData(data);
   }
}

void TGo4FitPanel::UpdateActivePage()
{
   int select = fiPanelMode;
   if (!GetFitter())
      select = FitGui::pm_None;

   if (fbParsWidgetShown && (select != FitGui::pm_None))
      select = 100;

   switch (select) {
   case FitGui::pm_None: break;
   case FitGui::pm_Simple: UpdateSimplePage(); break;
   case FitGui::pm_Wizard: UpdateWizardPage(); break;
   case FitGui::pm_Expert: UpdateExtendedPage(); break;
   case 100: {
      FillParsWidget();
      if (fiPanelMode == FitGui::pm_Wizard) {
         RemovePrimitives();
         UpdateWizPaint(-1);
      }
      break;
   }
   }

   ShowPanelPage(select);
   UpdateStatusBar();
   UpdateItemMenu();
}

void TGo4FitPanel::UpdateSimplePage()
{
   RemovePrimitives();

   PaintFitter(GetFitter(), 0, true);

   UpdateStatusBar();
}

void TGo4FitPanel::UpdateWizardPage()
{
   RemovePrimitives();

   TGo4Fitter *fitter = GetFitter();
   fbFillingWidget = true;
   if (fitter)
      Wiz_FitNameEdt->setText(QString(fitter->GetName()));
   else
      Wiz_FitNameEdt->setText(QString(""));
   fbFillingWidget = false;

   UpdateWizDataList();

   UpdateWizModelsList(false);

   UpdateWizStackWidget();

   UpdateWizPaint(-1);

   UpdateStatusBar();
}

void TGo4FitPanel::UpdateExtendedPage()
{
   RemoveItemWidget();

   RemovePrimitives();

   fbFillingWidget = true;
   FitList->clear();
   FitList->setSortingEnabled(false);
   fbFillingWidget = false;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter) {
      new QFitItem(this, FitList->invisibleRootItem(), 0, FitGui::ot_empty, FitGui::wt_none, FitGui::mt_empty);
   } else {
      QFitItem *fitteritem = new QFitItem(this, FitList->invisibleRootItem(), fitter, FitGui::ot_fitter,
                                          FitGui::wt_fitter, FitGui::mt_fitter, FitGui::gt_fitter);
      fitteritem->setExpanded(true);
      FitList->setCurrentItem(fitteritem, QItemSelectionModel::Select);
      ShowItem(fitteritem, false);
   }

   UpdateItemMenu();

   UpdateStatusBar();
}

void TGo4FitPanel::RemovePrimitives()
{
   if (!WorkingWithPanel())
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   for (Int_t n = 0; n < fitter->NumSlots(); n++) {
      TPad *pad = FindPadForSlot(fitter->GetSlot(n));
      if (!pad)
         continue;
      if (ActivePanel()->DeleteDrawObjects(pad, TGo4ViewPanel::kind_FitArrows))
         ActivePanel()->ShootRepaintTimer();
   }
}

void TGo4FitPanel::AddItemAction(QMenu *menu, const QString &name, int id, const QString &handler)
{
   if (handler.isEmpty())
      menu->addAction(name, [this, id]() { ItemMenuItemSelected(id); });
   else if (handler == "wiz_model")
      menu->addAction(name, [this, id]() { Wiz_CreateNewModel(id); });
   else if (handler == "wiz_data")
      menu->addAction(name, [this, id]() { Wiz_CreateNewData(id); });
}

bool TGo4FitPanel::FillPopupForItem(QFitItem *item, QMenu *menu)
{
   if (!item || !menu)
      return false;

   if (item->PopupMenuType() == FitGui::mt_empty) {
      AddItemAction(menu, "Create empty fitter", 8);
      if (WorkingWithPanel())
         AddItemAction(menu, "Create appropriate fitter", 9);
   }

   if (item->PopupMenuType() == FitGui::mt_fitter) {
      AddItemAction(menu, "Delete fitter", 1);
      AddItemAction(menu, "Clear fitter", 2);
      AddItemAction(menu, "Save fitter", 3);
      AddItemAction(menu, "Save fitter as ...", 4);
      AddItemAction(menu, "Print ...", 5);
      AddItemAction(menu, "Draw", 6);
      AddItemAction(menu, "Memorize parameters", 706);
      AddItemAction(menu, "Remember parameters", 707);
   }

   if (item->PopupMenuType() == FitGui::mt_data) {
      if (!WorkingWithPanel())
         AddItemAction(menu, QString("Remove ") + item->Object()->GetName(), 101);
      AddItemAction(menu, "Remove associated models", 102);
      AddItemAction(menu, "Draw", 105);
      AddItemAction(menu, "Print", 5);
   }

   if (item->PopupMenuType() == FitGui::mt_datalist) {
      if (WorkingWithPanel()) {
         AddItemAction(menu, "Rebuild data list", 108);
      } else {
         AddItemAction(menu, "Delete all data", 107);
         FillDataTypesList(menu, 110);
      }
   }

   if (item->PopupMenuType() == FitGui::mt_model) {
      AddItemAction(menu, QString("Remove ") + item->Object()->GetName(), 201);
      AddItemAction(menu, "Clone", 203);
      AddItemAction(menu, "Print", 5);
   }

   if (item->PopupMenuType() == FitGui::mt_modellist) {
      AddItemAction(menu, "Delete all models", 202);
      FillModelTypesList(menu, 210, true);
   }

   if (item->PopupMenuType() == FitGui::mt_asslist) {
      TGo4Fitter *fitter = GetFitter();
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
      if (fitter && model) {
         AddItemAction(menu, "Clear all assignments", 301);
         AddItemAction(menu, "Assign to all data", 302);
         menu->addSeparator();
         for (Int_t n = 0; n < fitter->GetNumData(); n++)
            if (!model->IsAssignTo(fitter->GetDataName(n)))
               AddItemAction(menu, QString("Assign to: ") + fitter->GetDataName(n), 310 + n);
      }
   }

   if (item->PopupMenuType() == FitGui::mt_ass)
      AddItemAction(menu, QString("Clear assignment to ") + item->text(0), 303);

   if (item->PopupMenuType() == FitGui::mt_range)
      AddItemAction(menu, "Remove range condition", 401);

   if (item->PopupMenuType() == FitGui::mt_rangecut)
      AddItemAction(menu, "Remove range condition", 401);

   if (item->PopupMenuType() == FitGui::mt_rangelist) {
      TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(item->Object());
      if (comp) {
         if (comp->IsAnyRangeLimits()) {
            AddItemAction(menu, "Clear all ranges", 402);
            menu->addSeparator();
         }
         AddItemAction(menu, "Include new range", 403);
         AddItemAction(menu, "Exclude new range", 404);
         AddItemAction(menu, "Add left bound", 405);
         AddItemAction(menu, "Add right bound", 406);
         menu->addSeparator();
         AddItemAction(menu, "Add range cut", 407);
      }
   }

   if (item->PopupMenuType() == FitGui::mt_action) {
      AddItemAction(menu, "Delete action", 501);
      AddItemAction(menu, "Move action up", 502);
      AddItemAction(menu, "Move action down", 503);
      AddItemAction(menu, "Print", 5);
      AddItemAction(menu, "Execute", 508);
   }

   if (item->PopupMenuType() == FitGui::mt_actlist) {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (fitter) {
         if (fitter->GetNumActions() > 0) {
            AddItemAction(menu, "Execute norm actions", 504);
            AddItemAction(menu, "Execute all actions", 507);
            AddItemAction(menu, "Delete all actions", 505);
            AddItemAction(menu, "Delete output actions", 506);
            menu->addSeparator();
         }
         AddItemAction(menu, "Add config", 510);
         AddItemAction(menu, "Add peak finder", 514);
         AddItemAction(menu, "Add amplitude estimation", 511);
         AddItemAction(menu, "Add minuit", 512);
         AddItemAction(menu, "Add output", 513);
      }
   }

   if (item->PopupMenuType() == FitGui::mt_depend)
      AddItemAction(menu, "Delete item", 601);

   if (item->PopupMenuType() == FitGui::mt_deplist) {
      AddItemAction(menu, "Delete all items", 602);
      AddItemAction(menu, "Insert new item", 603);
   }

   if (item->PopupMenuType() == FitGui::mt_newpars) {
      AddItemAction(menu, "Delete all items", 701);
      AddItemAction(menu, "Insert new item", 702);
   }

   if (item->PopupMenuType() == FitGui::mt_parcfg)
      AddItemAction(menu, "Delete item", 703);

   if (item->PopupMenuType() == FitGui::mt_par) {
      AddItemAction(menu, "Memorize value", 704);
      AddItemAction(menu, "Restore value", 705);
   }

   if ((item->PopupMenuType() == FitGui::mt_parslist)) {
      AddItemAction(menu, "Memorize parameters", 706);
      AddItemAction(menu, "Remember parameters", 707);
   }

   if (item->PopupMenuType() == FitGui::mt_minuitres) {
      AddItemAction(menu, "Delete result entry", 801);
      AddItemAction(menu, "Print", 5);
   }

   if (item->PopupMenuType() == FitGui::mt_slot) {
      TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(item->Object());
      if (slot)
         FillPopupForSlot(slot, menu);
   }

   if (item->PopupMenuType() == FitGui::mt_allslots) {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (fitter)
         AddItemAction(menu, "Update slots from sources", 904);
   }

   return !menu->isEmpty();
}

bool TGo4FitPanel::FillPopupForSlot(TGo4FitSlot *slot, QMenu *menu)
{
   if (!slot || !menu)
      return false;

   if (!slot->GetOwned() && slot->IsObject())
      AddItemAction(menu, "Clone object in slot", 1000);

   if (slot->IsObject())
      AddItemAction(menu, "Clear object in slot", 1001);

   if (FindPadForSlot(slot))
      AddItemAction(menu, QString("Update from ") + Wiz_GetSlotSourceInfo(slot), 1002);

   if (!menu->isEmpty())
      menu->addSeparator();

   if (slot->GetConnectedSlot())
      AddItemAction(menu, "Brake connection to slot", 1004);

   TGo4Fitter *fitter = GetFitter();
   if (fitter)
      for (Int_t n = 0; n < fitter->NumSlots(); n++) {
         TGo4FitSlot *sl = fitter->GetSlot(n);
         if (slot->CanConnectToSlot(sl) && (slot->GetConnectedSlot() != sl))
            AddItemAction(menu, QString("Connect to ") + sl->GetFullName(), 1400 + n);
      }

   if (!menu->isEmpty())
      menu->addSeparator();

   if (slot->GetClass() == TGo4FitData::Class())
      FillDataTypesList(menu, 1100);
   else if (slot->GetClass() == TGo4FitModel::Class())
      FillModelTypesList(menu, 1200, false);
   else if (slot->GetClass() == TGo4FitAxisTrans::Class()) {
      AddItemAction(menu, TGo4FitLinearTrans::Class()->GetName(), 1300);
      AddItemAction(menu, TGo4FitMatrixTrans::Class()->GetName(), 1301);
   }

   return !menu->isEmpty();
}

void TGo4FitPanel::ExecutePopupForSlot(QFitItem *item, TGo4FitSlot *slot, int id)
{
   if (item)
      slot = dynamic_cast<TGo4FitSlot *>(item->Object());

   TGo4Fitter *fitter = GetFitter();
   if (!slot || !fitter)
      return;

   switch (id) {
   case 1000: {
      TObject *newobj = slot->CloneObject();
      slot->SetObject(newobj, kTRUE);
      break;
   }

   case 1001: {

      if (checkConfirm("Clear slot", QString("Are you sure to clear object from slot ") + slot->GetName()))
         return;

      fitter->ClearSlot(slot, kFALSE);

      if (item)
         for (Int_t i = 0; i < fitter->NumSlots(); i++) {
            QFitItem *sitem = FindItem(fitter->GetSlot(i), FitGui::ot_slot, 0);
            if (sitem)
               UpdateItem(sitem, true);
         }

      break;
   }

   case 1002: {
      UpdateObjectReferenceInSlot(slot, false);
      break;
   }

   case 1004: {
      slot->ClearConnectionToSlot();
      break;
   }

   default: {
      TObject *obj = nullptr;
      if ((id >= 1100) && (id < 1200))
         obj = CreateData(id - 1100, "Data");
      else if ((id >= 1200) && (id < 1300))
         obj = CreateModel(id - 1200, "Model", 0, 0);
      else if (id == 1300)
         obj = new TGo4FitLinearTrans("Trans", "Linear axis transformation");
      else if (id == 1301)
         obj = new TGo4FitMatrixTrans("Trans", "Matrix axis transformation");
      else if (id >= 1400) {
         TGo4FitSlot *sl = fitter->GetSlot(id - 1400);
         slot->ConnectToSlot(sl);
         break;
      }
      if (obj)
         slot->SetObject(obj, kTRUE);
   }
   }

   if (item) {
      ShowItem(item, false);
      UpdateItem(item, true);
      item->setExpanded(true);
   } else
      UpdateActivePage();
}

void TGo4FitPanel::UpdateItem(QFitItem *item, bool trace)
{
   if (!item)
      return;

   if (fxCurrentItem)
      if (fxCurrentItem->FindInParents(item))
         RemoveItemWidget();

   while (item->childCount() > 0)
      delete item->child(0);

   SetItemText(item, false);

   switch (item->ObjectType()) {
   case FitGui::ot_empty: break;
   case FitGui::ot_datalist: {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (fitter)
         for (Int_t n = fitter->GetNumData() - 1; n >= 0; n--)
            new QFitItem(this, item, fitter->GetData(n), FitGui::ot_data, FitGui::wt_data, FitGui::mt_data,
                         FitGui::gt_data, n);
      break;
   }
   case FitGui::ot_data: {
      TGo4FitData *data = dynamic_cast<TGo4FitData *>(item->Object());
      if (!data)
         break;

      FillSlotsList(item, data->GetSlotList(), data);

      new QFitItem(this, item, data, FitGui::ot_parslist, FitGui::wt_none, FitGui::mt_parslist);

      new QFitItem(this, item, data, FitGui::ot_rangelist, FitGui::wt_none, FitGui::mt_rangelist, FitGui::gt_ranges);

      break;
   }
   case FitGui::ot_modellist: {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (fitter)
         for (Int_t n = fitter->GetNumModel() - 1; n >= 0; n--)
            new QFitItem(this, item, fitter->GetModel(n), FitGui::ot_model, DefineModelWidgetType(fitter->GetModel(n)),
                         FitGui::mt_model, FitGui::gt_model, n);
      break;
   }
   case FitGui::ot_model: {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
      if (!model)
         break;

      FillSlotsList(item, model->GetSlotList(), model);

      new QFitItem(this, item, model, FitGui::ot_parslist, FitGui::wt_none, FitGui::mt_parslist);

      new QFitItem(this, item, model, FitGui::ot_rangelist, FitGui::wt_none, FitGui::mt_rangelist, FitGui::gt_ranges);

      new QFitItem(this, item, model, FitGui::ot_asslist, FitGui::wt_none, FitGui::mt_asslist, FitGui::gt_none);

      break;
   }
   case FitGui::ot_parslist: FillParsList(item); break;
   case FitGui::ot_par: break;
   case FitGui::ot_parcfg: break;
   case FitGui::ot_asslist: {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
      if (model)
         for (Int_t n = model->NumAssigments() - 1; n >= 0; n--)
            new QFitItem(this, item, model, FitGui::ot_ass, FitGui::wt_none, FitGui::mt_ass, FitGui::gt_ass, n);
      break;
   }
   case FitGui::ot_ass: break;
   case FitGui::ot_rangelist: {
      TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(item->Object());
      if (comp) {
         for (Int_t n = comp->GetNumRangeCut() - 1; n >= 0; n--)
            new QFitItem(this, item, comp->GetRangeCut(n), FitGui::ot_rangecut, FitGui::wt_rangecut,
                         FitGui::mt_rangecut, FitGui::gt_none, n);
         for (Int_t n = comp->GetNumRangeCondition() - 1; n >= 0; n--)
            new QFitItem(this, item, comp, FitGui::ot_range, FitGui::wt_range, FitGui::mt_range, FitGui::gt_range, n);
      }
      break;
   }
   case FitGui::ot_range: break;
   case FitGui::ot_rangecut: break;
   case FitGui::ot_actlist: {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (!fitter)
         break;
      for (Int_t n = fitter->GetNumActions() - 1; n >= 0; n--) {
         TGo4FitterAction *action = fitter->GetAction(n);
         int ot = FitGui::ot_action, wt = FitGui::wt_named;
         if (action->InheritsFrom(TGo4FitterConfig::Class()))
            ot = FitGui::ot_config;
         else if (action->InheritsFrom(TGo4FitAmplEstimation::Class())) {
            ot = FitGui::ot_amplest;
            wt = FitGui::wt_amplest;
         } else if (action->InheritsFrom(TGo4FitPeakFinder::Class())) {
            ot = FitGui::ot_peakfinder;
            wt = FitGui::wt_peakfinder;
         } else if (action->InheritsFrom(TGo4FitterOutput::Class())) {
            ot = FitGui::ot_output;
            wt = FitGui::wt_output;
         } else if (action->InheritsFrom(TGo4FitMinuit::Class())) {
            ot = FitGui::ot_minuit;
            wt = FitGui::wt_minuit;
         }

         new QFitItem(this, item, action, ot, wt, FitGui::mt_action);
      }
      break;
   }
   case FitGui::ot_action: break;
   case FitGui::ot_amplest: break;
   case FitGui::ot_peakfinder: break;
   case FitGui::ot_output: break;
   case FitGui::ot_config: {
      TGo4FitterConfig *cfg = dynamic_cast<TGo4FitterConfig *>(item->Object());
      if (!cfg)
         break;
      new QFitItem(this, item, &(cfg->GetResults()), FitGui::ot_reslist, FitGui::wt_none, FitGui::mt_deplist);
      new QFitItem(this, item, &(cfg->GetParsDepend()), FitGui::ot_pardeplist, FitGui::wt_none, FitGui::mt_deplist);
      new QFitItem(this, item, &(cfg->GetParsInit()), FitGui::ot_parinitlist, FitGui::wt_none, FitGui::mt_deplist);
      new QFitItem(this, item, &(cfg->GetParsCfg()), FitGui::ot_parscfg, FitGui::wt_none, FitGui::mt_newpars);
      new QFitItem(this, item, &(cfg->GetParsNew()), FitGui::ot_newpars, FitGui::wt_none, FitGui::mt_newpars);
      break;
   }
   case FitGui::ot_minuit: {
      TGo4FitMinuit *minuit = dynamic_cast<TGo4FitMinuit *>(item->Object());
      if (!minuit)
         break;
      for (Int_t n = minuit->GetNumResults() - 1; n >= 0; n--)
         new QFitItem(this, item, minuit->GetResult(n), FitGui::ot_minuitres, FitGui::wt_minuitres,
                      FitGui::mt_minuitres);
      break;
   }
   case FitGui::ot_parinitlist: FillDependencyList(item); break;
   case FitGui::ot_pardeplist: FillDependencyList(item); break;
   case FitGui::ot_reslist: FillDependencyList(item); break;
   case FitGui::ot_depend: break;
   case FitGui::ot_newpars: FillParsList(item); break;
   case FitGui::ot_parscfg: {
      TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Object());
      if (!pars)
         return;
      for (Int_t n = pars->NumPars() - 1; n >= 0; n--)
         new QFitItem(this, item, pars->GetPar(n), FitGui::ot_parcfg, FitGui::wt_parcfg, FitGui::mt_parcfg);
      break;
   }
   case FitGui::ot_minuitres: break;
   case FitGui::ot_slot: {
      TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(item->Object());
      if (!slot)
         break;

      TObject *obj = slot->GetObject();
      if (!obj)
         break;

      if (obj->InheritsFrom(TGo4FitData::Class()))
         new QFitItem(this, item, obj, FitGui::ot_data, FitGui::wt_data, FitGui::mt_none);
      else

         if (obj->InheritsFrom(TGo4FitModel::Class()))
         new QFitItem(this, item, obj, FitGui::ot_model, DefineModelWidgetType(obj), FitGui::mt_none);
      else

         if (obj->InheritsFrom(TGo4FitLinearTrans::Class()))
         new QFitItem(this, item, obj, FitGui::ot_lintrans, FitGui::wt_lintrans, FitGui::mt_none);
      else

         if (obj->InheritsFrom(TGo4FitMatrixTrans::Class()))
         new QFitItem(this, item, obj, FitGui::ot_matrtrans, FitGui::wt_matrtrans, FitGui::mt_none);
      else

         if (obj->InheritsFrom(TGo4FitAxisTrans::Class()))
         new QFitItem(this, item, obj, FitGui::ot_trans, FitGui::wt_named, FitGui::mt_none);
      else

         new QFitItem(this, item, obj, FitGui::ot_none, FitGui::wt_none, FitGui::mt_none);

      break;
   }
   case FitGui::ot_fitter: {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (!fitter)
         break;

      new QFitItem(this, item, fitter, FitGui::ot_results, FitGui::wt_table);
      new QFitItem(this, item, fitter, FitGui::ot_actlist, FitGui::wt_none, FitGui::mt_actlist);
      new QFitItem(this, item, fitter, FitGui::ot_allslots, FitGui::wt_none, FitGui::mt_allslots);
      new QFitItem(this, item, fitter, FitGui::ot_modellist, FitGui::wt_none, FitGui::mt_modellist);
      new QFitItem(this, item, fitter, FitGui::ot_datalist, FitGui::wt_none, FitGui::mt_datalist);
      break;
   }
   case FitGui::ot_trans: FillParsList(item); break;
   case FitGui::ot_lintrans: FillParsList(item); break;
   case FitGui::ot_matrtrans: FillParsList(item); break;
   case FitGui::ot_results: break;
   case FitGui::ot_allslots: {
      TGo4Fitter *fitter = dynamic_cast<TGo4Fitter *>(item->Object());
      if (fitter)
         FillSlotsList(item, fitter->GetSlotList(), nullptr);
      break;
   }
   }

   if (trace && item->Object()) {
      QFitItem *topitem = GetFitterItem();
      if (!topitem)
         return;

      QTreeWidgetItemIterator iter(topitem);

      while (*iter) {
         QFitItem *it = dynamic_cast<QFitItem *>(*iter);
         if (it && (it != item) && (item->ObjectType() == it->ObjectType()) && (item->Object() == it->Object()))
            UpdateItem(it, false);
         ++iter;
      }
   }
}

void TGo4FitPanel::SetItemText(QFitItem *item, bool trace)
{
   if (!item)
      return;
   QString text;
   if (item->Object())
      text = item->Object()->GetName();
   switch (item->ObjectType()) {
   case FitGui::ot_empty: text = "empty"; break;
   case FitGui::ot_datalist: text = "Data"; break;
   case FitGui::ot_data: break;
   case FitGui::ot_modellist: text = "Models"; break;
   case FitGui::ot_model: break;
   case FitGui::ot_parslist: text = "Parameters"; break;
   case FitGui::ot_par:
   case FitGui::ot_parcfg: {
      TGo4FitParameter *par = dynamic_cast<TGo4FitParameter *>(item->Object());
      if (par)
         text += " = " + QString::number(par->GetValue());
      break;
   }
   case FitGui::ot_asslist: text = "Assigments"; break;
   case FitGui::ot_ass: {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(item->Object());
      if (model)
         text = model->AssignmentName(item->Tag());
      break;
   }
   case FitGui::ot_rangelist: text = "Ranges"; break;
   case FitGui::ot_range: {
      TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(item->Object());
      if (!comp || (item->Tag() < 0) || (item->Tag() >= comp->GetNumRangeCondition()))
         break;

      Int_t typ, naxis;
      Double_t left, right;
      comp->GetRangeCondition(item->Tag(), typ, naxis, left, right);
      text = QString::number(naxis) + ": ";

      switch (typ) {
      case 0: text += "[" + QString::number(left) + "..." + QString::number(right) + "]"; break;
      case 1: text += "... " + QString::number(left) + "]   [" + QString::number(right) + " ..."; break;
      case 2: text += "[" + QString::number(left) + " ..."; break;
      case 3: text += "... " + QString::number(right) + "]"; break;
      }
      break;
   }
   case FitGui::ot_rangecut: break;
   case FitGui::ot_actlist: text = "Actions"; break;
   case FitGui::ot_action: break;
   case FitGui::ot_amplest: break;
   case FitGui::ot_peakfinder: break;
   case FitGui::ot_output: break;
   case FitGui::ot_config: break;
   case FitGui::ot_minuit: break;
   case FitGui::ot_parinitlist: text = "Initialization"; break;
   case FitGui::ot_pardeplist: text = "Dependency"; break;
   case FitGui::ot_reslist: text = "Results"; break;
   case FitGui::ot_depend: {
      TGo4FitDependency *depen = dynamic_cast<TGo4FitDependency *>(item->Object());
      if (!depen)
         break;
      if (depen->IsResultDepend())
         text = "Res" + QString::number(item->Tag());
      else
         text = depen->GetParameter().Data();
      text += " = ";
      if (depen->IsInitValue())
         text += QString::number(depen->GetInitValue());
      else
         text += depen->GetExpression().Data();
      break;
   }
   case FitGui::ot_newpars: text = "New pars"; break;
   case FitGui::ot_parscfg: text = "Pars setup"; break;
   case FitGui::ot_minuitres: break;
   case FitGui::ot_slot: break;
   case FitGui::ot_fitter: break;
   case FitGui::ot_trans: break;
   case FitGui::ot_lintrans: break;
   case FitGui::ot_matrtrans: break;
   case FitGui::ot_results: text = "Results"; break;
   case FitGui::ot_allslots: text = "Objects"; break;
   }

   item->setText(0, text);

   if (trace && item->Object()) {
      QFitItem *topitem = GetFitterItem();
      if (!topitem)
         return;

      QTreeWidgetItemIterator iter(topitem);

      while (*iter) {
         QFitItem *it = dynamic_cast<QFitItem *>(*iter);
         if (it && (it != item) && (item->ObjectType() == it->ObjectType()) && (item->Object() == it->Object()) &&
             (item->Tag() == it->Tag()))
            it->setText(0, text);
         ++iter;
      }
   }
}

void TGo4FitPanel::UpdateItemsOfType(int typ, QFitItem *parent)
{
   if (!parent)
      parent = GetFitterItem();

   if (!parent) {
      std::cout << "Did not found " << std::endl;
      return;
   }

   QTreeWidgetItemIterator iter(parent);

   while (*iter) {
      QFitItem *item = dynamic_cast<QFitItem *>(*iter);
      if (item && (item->ObjectType() == typ))
         UpdateItem(item, false);
      ++iter;
   }
}

QFitItem *TGo4FitPanel::GetFitterItem()
{
   QFitItem *item = dynamic_cast<QFitItem *>(FitList->topLevelItem(0));
   if (item && (item->ObjectType() == FitGui::ot_fitter))
      return item;
   return nullptr;
}

QFitItem *TGo4FitPanel::FindItem(TObject *obj, int ObjectType, QFitItem *parent)
{
   if (!parent)
      parent = GetFitterItem();

   QTreeWidgetItemIterator iter(parent);

   while (*iter) {
      QFitItem *item = dynamic_cast<QFitItem *>(*iter);
      if (item)
         if (!obj || (item->Object() == obj))
            if ((ObjectType == FitGui::ot_none) || (ObjectType == item->ObjectType()))
               return item;
      ++iter;
   }
   return nullptr;
}

bool TGo4FitPanel::ShowItem(QFitItem *item, bool force)
{
   if (force)
      RemoveItemWidget();

   bool gr = false;

   RemovePrimitives();
   if (fbShowPrimitives)
      gr = ShowItemAsGraph(item, force);

   bool txt = ShowItemAsText(item, force);

   return (gr || txt);
}

bool TGo4FitPanel::ShowItemAsText(QFitItem *item, bool force)
{

   QFitItem *widgetitem = item->DefineWidgetItem();

   QFitWidget *oldwidget = dynamic_cast<QFitWidget *>(fxCurrentItemWidget);
   if (oldwidget && (oldwidget->GetItem() == widgetitem)) {
      oldwidget->FillWidget();
      return true;
   }

   RemoveItemWidget();

   QFitWidget *widget = nullptr;

   if (!widgetitem)
      return true;

   QWidget *owner = nullptr;

   switch (widgetitem->WidgetType()) {
   case FitGui::wt_par: widget = new QFitParWidget(owner, "Parameter"); break;
   case FitGui::wt_range: widget = new QFitRangeWidget(owner, "Range"); break;
   case FitGui::wt_rangecut: widget = new QFitRangeCutWidget(owner, "Range cut"); break;
   case FitGui::wt_data: widget = new QFitDataWidget(owner, "Data object"); break;
   case FitGui::wt_model: widget = new QFitModelWidget(owner, "Model"); break;
   case FitGui::wt_polynom: widget = new QFitModelPolynomWidget(owner, "Polynom"); break;
   case FitGui::wt_gauss1: widget = new QFitModelGauss1Widget(owner, "Gauss1"); break;
   case FitGui::wt_gauss2: widget = new QFitModelGauss2Widget(owner, "Gauss2"); break;
   case FitGui::wt_gaussn: widget = new QFitModelGaussNWidget(owner, "GaussN"); break;
   case FitGui::wt_formula: widget = new QFitModelFormulaWidget(owner, "Formula"); break;
   case FitGui::wt_function: widget = new QFitModelFunctionWidget(owner, "Function"); break;
   case FitGui::wt_amplest: widget = new QFitAmplEstimWidget(owner, "Amplitude estimation"); break;
   case FitGui::wt_peakfinder: widget = new QFitPeakFinderWidget(owner, "Peak finder"); break;
   case FitGui::wt_output: widget = new QFitOutputActionWidget(owner, "Fitter output"); break;
   case FitGui::wt_minuit: widget = new QFitMinuitWidget(owner, "Minuit"); break;
   case FitGui::wt_minuitres: widget = new QFitMinuitResWidget(owner, "Minuit result"); break;
   case FitGui::wt_depend: widget = new QFitDependencyWidget(owner, "Dependency"); break;
   case FitGui::wt_parcfg: widget = new QFitParCfgWidget(owner, "Parameter configuration"); break;
   case FitGui::wt_slot: widget = new QFitSlotWidget(owner, "Slot for external object"); break;
   case FitGui::wt_fitter: widget = new QFitterWidget(owner, "Fitter"); break;
   case FitGui::wt_named: widget = new QFitNamedWidget(owner, "Named object"); break;
   case FitGui::wt_table: widget = new QFitTableWidget(owner, "Table"); break;
   case FitGui::wt_lintrans: widget = new QFitLinearTransWidget(owner, "Simple transform"); break;
   case FitGui::wt_matrtrans: widget = new QFitMatrixTransWidget(owner, "Matrix transform"); break;
   }

   if (widget) {
      fxCurrentItemWidget = widget;
      widget->SetItem(this, widgetitem);
      ListStack->addWidget(widget);
      ListStack->setCurrentWidget(widget);
   }

   return true;
}

bool TGo4FitPanel::ShowItemAsGraph(QFitItem *item, bool force)
{
   if (!item)
      return false;

   QFitItem *gritem = item->DefineGraphItem();

   if (!gritem)
      return false;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return false;

   if (gritem->GraphType() == FitGui::gt_fitter) {
      PaintFitter(fitter, gritem, false);
      return true;
   }

   if (gritem->GraphType() == FitGui::gt_data) {
      TGo4FitData *data = dynamic_cast<TGo4FitData *>(gritem->Object());

      if (data)
         PaintModelsFor(fitter, data, gritem->Parent()->Parent(), false);

      return data != nullptr;
   }

   if (gritem->GraphType() == FitGui::gt_model) {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(gritem->Object());
      if (!model)
         return false;
      bool draw = false;
      for (Int_t n = 0; n < model->NumAssigments(); n++) {
         TGo4FitData *data = fitter->FindData(model->AssignmentName(n));
         if (PaintModel(model, FindPadWhereData(data), gritem))
            draw = true;
      }
      return draw;
   }

   if (gritem->GraphType() == FitGui::gt_ass) {
      TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(gritem->Parent()->Parent()->Object());
      if (model) {
         TGo4FitData *data = fitter->FindData(gritem->text(0).toLatin1().constData());
         if (PaintModel(model, FindPadWhereData(data), gritem->Parent()->Parent()))
            return true;
      }
   }

   if (gritem->GraphType() == FitGui::gt_range) {
      TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(gritem->Parent()->Object());
      if (!comp)
         return false;
      int nrange = gritem->Tag();
      if (PaintRange(comp, nrange, FindPadWhereComp(comp), gritem))
         return true;
   }

   if (gritem->GraphType() == FitGui::gt_ranges) {
      TGo4FitComponent *comp = dynamic_cast<TGo4FitComponent *>(gritem->Object());
      if (!comp)
         return false;

      TPad *pad = FindPadWhereComp(comp);
      if (pad) {
         for (int nrange = 0; nrange < comp->GetNumRangeCondition(); nrange++) {
            QFitItem *child = dynamic_cast<QFitItem *>(gritem->child(nrange));
            if (!child)
               return false;
            PaintRange(comp, nrange, pad, child);
         }
         return true;
      }
   }

   return false;
}

void TGo4FitPanel::RemoveItemWidget()
{
   fxCurrentItem = nullptr;
   if (fxCurrentItemWidget) {
      ListStack->removeWidget(fxCurrentItemWidget);
      delete fxCurrentItemWidget;
      fxCurrentItemWidget = nullptr;
   }
}

void TGo4FitPanel::FitItemDeleted(QFitItem *item)
{
   if (item && (fxCurrentItem == item))
      RemoveItemWidget();
}

void TGo4FitPanel::FillParsList(QFitItem *item)
{
   if (!item)
      return;
   TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Object());
   if (!pars)
      return;
   for (Int_t n = pars->NumPars() - 1; n >= 0; n--)
      new QFitItem(this, item, pars->GetPar(n), FitGui::ot_par, FitGui::wt_par, FitGui::mt_par);
}

void TGo4FitPanel::FillSlotsList(QFitItem *parent, const TObjArray *lst, TObject *owner)
{
   if (!parent || !lst)
      return;
   for (Int_t n = lst->GetLast(); n >= 0; n--) {
      TGo4FitSlot *slot = dynamic_cast<TGo4FitSlot *>(lst->At(n));
      if (slot && ((slot->GetOwner() == owner) || !owner))
         new QFitItem(this, parent, slot, FitGui::ot_slot, FitGui::wt_slot, FitGui::mt_slot);
   }
}

void TGo4FitPanel::FillDependencyList(QFitItem *parent)
{
   if (!parent)
      return;

   TObjArray *lst = dynamic_cast<TObjArray *>(parent->Object());
   if (lst)
      for (Int_t n = lst->GetLast(); n >= 0; n--)
         new QFitItem(this, parent, lst->At(n), FitGui::ot_depend, FitGui::wt_depend, FitGui::mt_depend,
                      FitGui::gt_none, n);
}

void TGo4FitPanel::FillModelTypesList(QMenu *menu, int id, bool extend, const QString &handler)
{
   if (!menu->isEmpty())
      menu->addSeparator();
   if (extend) {
      AddItemAction(menu, "Add gaussian", id + 20, handler);
      AddItemAction(menu, "Add lorenzian", id + 24, handler);
      AddItemAction(menu, "Add exponent", id + 25, handler);
      AddItemAction(menu, "Add 1-order polynom", id + 21, handler);
      AddItemAction(menu, "Add 3-order polynom", id + 22, handler);
      AddItemAction(menu, "Add 7-order polynom", id + 23, handler);
      menu->addSeparator();
   }

   AddItemAction(menu, QString("Make ") + TGo4FitModelPolynom::Class()->GetName(), id + 0, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitModelGauss1::Class()->GetName(), id + 1, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitModelGauss2::Class()->GetName(), id + 2, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitModelGaussN::Class()->GetName(), id + 3, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitModelFormula::Class()->GetName(), id + 4, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitModelFunction::Class()->GetName(), id + 5, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitModelFromData::Class()->GetName(), id + 6, handler);
}

void TGo4FitPanel::FillDataTypesList(QMenu *menu, int id, const QString &handler)
{
   if (!menu->isEmpty())
      menu->addSeparator();
   AddItemAction(menu, QString("Make ") + TGo4FitDataHistogram::Class()->GetName(), id + 0, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitDataGraph::Class()->GetName(), id + 1, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitDataProfile::Class()->GetName(), id + 2, handler);
   AddItemAction(menu, QString("Make ") + TGo4FitDataRidge::Class()->GetName(), id + 3, handler);
}

void TGo4FitPanel::PaintFitter(TGo4Fitter *fitter, QFitItem *item, bool update)
{
   if (fitter)
      for (Int_t nd = 0; nd < fitter->GetNumData(); nd++)
         PaintModelsFor(fitter, fitter->GetData(nd), item, update);
}

bool TGo4FitPanel::PaintModel(TGo4FitModel *model, TPad *pad, QFitItem *item)
{
   if (!model || !pad || !fbShowPrimitives)
      return false;

   TH1 *histo = ActivePanel()->GetPadHistogram(pad);
   if (!histo || (histo->GetDimension() != 1))
      return false;

   Double_t pos, width, ampl;

   if (model->GetPosition(0, pos)) {

      Int_t nbin = histo->GetXaxis()->FindBin(pos);
      ampl = histo->GetBinContent(nbin);

      Double_t limit = pad->GetUymin() * .7 + pad->GetUymax() * .3;
      if (ampl < limit)
         ampl = limit;

      TGo4FitGuiArrow *arr = new TGo4FitGuiArrow(0.01, ">");
      arr->SetLineColor(2);
      arr->SetLineWidth(1);
      arr->SetItem(item, this);

      TGo4FitGuiArrow *warr = nullptr;
      if (model->GetWidth(0, width)) {
         warr = new TGo4FitGuiArrow(0.01, "<|>");
         warr->SetLineColor(2);
         warr->SetLineWidth(1);
         warr->SetForWidth(model, arr);
         warr->SetItem(item, this);
      }

      arr->SetForPosition(model, warr, ampl);
      arr->Locate();

      ActivePanel()->AddDrawObject(pad, TGo4ViewPanel::kind_FitArrows, "::Model_VArrow", arr, true, nullptr);

      if (warr) {
         warr->Locate();
         ActivePanel()->AddDrawObject(pad, TGo4ViewPanel::kind_FitArrows, "::Model_HArrow", warr, true, nullptr);
      }

      ActivePanel()->ShootRepaintTimer();
   }

   return true;
}

bool TGo4FitPanel::PaintModelsFor(TGo4Fitter *fitter, TGo4FitData *data, QFitItem *item, bool update)
{
   if (!fitter || !data)
      return false;

   if (data->IsAnyDataTransform())
      return false;

   for (Int_t nm = 0; nm < fitter->GetNumModel(); nm++) {
      TGo4FitModel *model = fitter->GetModel(nm);
      if (model->IsAssignTo(data->GetName()))
         PaintModel(model, FindPadWhereData(data), item);
   }

   return true;
}

bool TGo4FitPanel::PaintRange(TGo4FitComponent *comp, int nrange, TPad *pad, QFitItem *item)
{
   if (!comp || !pad || !fbShowPrimitives)
      return false;

   TH1 *histo = ActivePanel()->GetPadHistogram(pad);

   if (!histo || (histo->GetDimension() != 1))
      return false;

   double range = 0.1 * (histo->GetXaxis()->GetXmax() - histo->GetXaxis()->GetXmin());

   double ampl = (0.07 + nrange * 0.02) * (histo->GetMaximum() - histo->GetMinimum()) + histo->GetMinimum();

   double size = 0.015;

   Int_t typ, naxis;
   Double_t left, right;

   comp->GetRangeCondition(nrange, typ, naxis, left, right);

   if (naxis != 0)
      return false;

   TGo4FitGuiArrow *arr = nullptr;

   switch (typ) {
   case 0: arr = new TGo4FitGuiArrow(size, "<|>"); break;
   case 1: arr = new TGo4FitGuiArrow(size, "-><-"); break;
   case 2: arr = new TGo4FitGuiArrow(size, "<"); break;
   case 3: arr = new TGo4FitGuiArrow(size, ">"); break;
   }

   if (arr) {
      arr->SetLineColor(3);
      arr->SetLineWidth(3);
      arr->SetForRange(comp, nrange, ampl, range);
      arr->SetItem(item, this);
      arr->Locate();

      ActivePanel()->AddDrawObject(pad, TGo4ViewPanel::kind_FitArrows, "::Range_Arrow", arr, true, nullptr);

      ActivePanel()->ShootRepaintTimer();
   }
   return arr != nullptr;
}

TGo4FitPeakFinder *TGo4FitPanel::GetPeakFinder(bool autocreate)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return nullptr;
   for (Int_t n = 0; n < fitter->GetNumActions(); n++) {
      TGo4FitPeakFinder *finder = dynamic_cast<TGo4FitPeakFinder *>(fitter->GetAction(n));
      if (finder)
         return finder;
   }

   if (autocreate) {
      TGo4FitPeakFinder *finder = new TGo4FitPeakFinder("Finder", 0, kTRUE, 1);
      finder->SetPeakFinderType(2);
      fitter->AddActionAt(finder, 0);
      return finder;
   }

   return nullptr;
}

TGo4FitData *TGo4FitPanel::Wiz_SelectedData()
{
   TGo4Fitter *fitter = GetFitter();
   return !fitter ? nullptr : fitter->FindData(fxWizDataName.toLatin1().constData());
}

TGo4FitModel *TGo4FitPanel::Wiz_SelectedModel()
{
   TGo4Fitter *fitter = GetFitter();
   return !fitter ? nullptr : fitter->FindModel(fxWizModelName.toLatin1().constData());
}

void TGo4FitPanel::Wiz_CreateNewModel(int id)
{
   TGo4Fitter *fitter = GetFitter();
   TGo4FitModel *model = nullptr;
   if (fitter) {
      auto data = Wiz_SelectedData();
      model = CreateModel(id, 0, fitter, data);
      LocateModel(model, data, true);
   }

   if (model)
      fxWizModelName = model->GetName();
   UpdateWizModelsList(true);
   if (Wiz_SelectedModel())
      UpdateWizPaint(2);
   UpdateStatusBar();
}

void TGo4FitPanel::Wiz_CreateNewData(int id)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   TGo4FitData *data = CreateData(id, 0);
   if (!data)
      return;

   fitter->AddData(data);

   UpdateObjectReferenceInSlots();

   fxWizDataName = data->GetName();
   fiWizPageIndex = 2;
   UpdateWizDataList();
   UpdateWizModelsList(false);
   UpdateWizStackWidget();
   UpdateWizPaint(1);
   UpdateStatusBar();
}

void TGo4FitPanel::Wiz_RebuildDataList()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;
   if (fitter->GetNumData() > 0)
      if (checkConfirm("Rebuild data list", "Remove all current data?"))
         return;

   fitter->DeleteAllData();
   CreateDataFor(ActivePanel(), ActivePad(), fitter);

   UpdateObjectReferenceInSlots();
   UpdateActivePage();
}

bool TGo4FitPanel::Wiz_RemoveData()
{
   TGo4Fitter *fitter = GetFitter();
   TGo4FitData *data = Wiz_SelectedData();
   if (!data || !fitter)
      return false;

   if (checkConfirm("Delete data", QString("Are you sure to delete ") + data->GetName()))
      return false;

   fitter->RemoveData(data->GetName(), kTRUE);

   fxWizDataName = "";

   UpdateObjectReferenceInSlots();

   return true;
}

void TGo4FitPanel::Wiz_UseSelectedRange()
{
   if (!WorkingWithPanel())
      return;

   TGo4FitData *data = Wiz_SelectedData();
   TPad *pad = FindPadWhereData(data);
   if (!pad || !data)
      return;

   double xmin, xmax;
   if (ActivePanel()->GetVisibleRange(pad, 0, xmin, xmax))
      data->SetRange(0, xmin, xmax);

   UpdateActivePage();
}

void TGo4FitPanel::Wiz_TakeCurrentRange()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter || !ActivePanel())
      return;

   for (int n = 0; n < fitter->GetNumData(); n++) {
      TGo4FitData *data = fitter->GetData(n);
      TPad *pad = FindPadWhereData(data);

      if (pad && data) {
         data->ClearRanges();
         double xmin, xmax;
         if (ActivePanel()->GetVisibleRange(pad, 0, xmin, xmax))
            data->SetRange(0, xmin, xmax);
      }
   }
}

void TGo4FitPanel::Wiz_GetModelInfo(TGo4FitModel *model, QString *info)
{
   if (!model || !info)
      return;
   *info = "Model: ";
   *info += model->GetName();
   *info += "  of class: ";
   *info += model->ClassName();

   if (model->InheritsFrom(TGo4FitModelFormula::Class())) {
      TGo4FitModelFormula *formula = dynamic_cast<TGo4FitModelFormula *>(model);
      if (formula) {
         *info += "\nFormula: ";
         *info += formula->GetExpression();
      }
   }

   if (model->InheritsFrom(TGo4FitModelFunction::Class())) {
      TGo4FitModelFunction *func = dynamic_cast<TGo4FitModelFunction *>(model);
      if (func) {
         *info += "\nFunction: ";
         *info += func->GetFunctionName();
         *info += "  from: ";
         *info += func->GetLibraryName();
      }
   }
   if (fiIntegralMode > 0) {
      int mode = fiIntegralMode;

      if ((mode == 1) || (mode == 2)) {
         TGo4Fitter *fitter = GetFitter();
         TGo4FitData *data = Wiz_SelectedData();
         if (data)
            if (!model->IsAssignTo(data->GetName()))
               data = nullptr;
         if (data && fitter) {
            if (mode == 1)
               *info += "\nCounts=";
            else
               *info += "\nIntegral=";
            double v = fitter->CalculatesIntegral(data->GetName(), model->GetName(), (mode == 1));
            *info += QString::number(v);
         }
      }

      if (mode == 3) {
         double v = model->Integral();
         if (v > 0) {
            *info += "\nGauss integral=";
            *info += QString::number(v);
         }
      }
   }
}

QString TGo4FitPanel::Wiz_GetSlotSourceInfo(TGo4FitSlot *slot)
{
   QString res = "";
   if (!slot)
      return res;

   if (!slot->GetObject()) {
      res = "none";
   } else if (slot->GetOwned()) {
      res = "internal object";
   } else if (WorkingWithPanel()) {
      TPad *pad = FindPadForSlot(slot);
      if (!pad)
         res = "reference without pad";
      else {
         res = "Panel: ";
         res += ActivePanel()->objectName();
         if (ActivePanel()->GetCanvas() != pad) {
            res += "  pad: ";
            res += pad->GetName();
         }
      }
   } else {
      int slotindex = GetPadIndexForSlot(slot);
      QString linkname = QString("FitSlotLink_%1").arg(slotindex);
      const char *itemname = GetLinkedName(linkname.toLatin1().constData());
      if (itemname)
         res = itemname;
      else
         res = "reference to external object";
   }

   return res;
}

void TGo4FitPanel::ArrowChanged(TGo4FitGuiArrow *arr)
{
   TGo4Fitter *fitter = GetFitter();

   if (!arr || !fitter)
      return;

   bool ismodified = false;
   for (Int_t n = 0; n < fitter->NumSlots(); n++) {
      TPad *pad = FindPadForSlot(fitter->GetSlot(n));
      if (!pad)
         continue;

      TObjArray lst;
      ActivePanel()->CollectSpecialObjects(pad, &lst, TGo4ViewPanel::kind_FitArrows);

      bool res = false;
      for (Int_t k = 0; k <= lst.GetLast(); k++) {
         TGo4FitGuiArrow *a = dynamic_cast<TGo4FitGuiArrow *>(lst[k]);
         if (!a)
            continue;
         if (a->Locate())
            res = true;
      }
      if (res) {
         ActivePanel()->MarkPadModified(pad);
         ismodified = true;
      }
   }
   if (ismodified)
      ActivePanel()->ShootRepaintTimer(ActivePad());

   if (fiPanelMode == FitGui::pm_Expert) {
      if ((arr->GetType() == TGo4FitGuiArrow::at_pos) || (arr->GetType() == TGo4FitGuiArrow::at_width))
         UpdateItemsOfType(FitGui::ot_par, arr->GetItem());

      if (arr->GetType() == TGo4FitGuiArrow::at_range)
         SetItemText(arr->GetItem(), true);

      QFitWidget *current = dynamic_cast<QFitWidget *>(fxCurrentItemWidget);
      if (current)
         current->FillWidget();
   }

   if (fiPanelMode == FitGui::pm_Wizard) {
      if (fbParsWidgetShown)
         FillParsWidget();
      else
         UpdateWizStackWidget();
   }
}

void TGo4FitPanel::DeleteModelWithPrimit(TGo4FitGuiArrow *arr)
{
   if (!arr)
      return;
   TGo4FitModel *model = arr->GetModel();
   if (!model)
      return;

   if (fiPanelMode == FitGui::pm_Expert) {
      QFitItem *item = FindItem(model, FitGui::ot_model, 0);
      if (item)
         Cmd_DeleteModel(item);
   } else
      Cmd_RemoveModel(model);
}

int TGo4FitPanel::GetPadIndexForSlot(TGo4FitSlot *slot)
{
   TGo4Fitter *fitter = GetFitter();
   if (!slot || !fitter)
      return -1;

   int indx = -1;
   for (int n = 0; n < fitter->NumSlots(); n++) {
      TGo4FitSlot *subslot = fitter->GetSlot(n);
      bool needpad = !subslot->GetOwned() && (subslot->GetClass()->InheritsFrom(TH1::Class()) ||
                                              subslot->GetClass()->InheritsFrom(TGraph::Class()));
      if (needpad)
         indx++;
      if (slot == subslot)
         return needpad ? indx : -1;
   }
   return -1;
}

TPad *TGo4FitPanel::FindPadForSlot(TGo4FitSlot *slot)
{
   if (!WorkingWithPanel())
      return nullptr;
   int indx = GetPadIndexForSlot(slot);
   return (indx < 0) ? nullptr : ActivePanel()->GetSubPad(ActivePad(), indx, true);
}

TPad *TGo4FitPanel::FindPadWhereData(TGo4FitData *data)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return nullptr;
   for (Int_t n = 0; n < fitter->NumSlots(); n++) {
      TGo4FitSlot *slot = fitter->GetSlot(n);
      if (slot->GetOwner() != data)
         continue;
      TPad *pad = FindPadForSlot(slot);
      if (pad)
         return pad;
   }
   return nullptr;
}

TPad *TGo4FitPanel::FindPadWhereModel(TGo4FitModel *model)
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter || !model)
      return nullptr;

   for (Int_t n = 0; n < model->NumAssigments(); n++) {
      TGo4FitData *data = fitter->FindData(model->AssignmentName(n));
      TPad *pad = FindPadWhereData(data);
      if (pad)
         return pad;
   }
   return nullptr;
}

TPad *TGo4FitPanel::FindPadWhereComp(TGo4FitComponent *comp)
{
   if (!comp)
      return nullptr;
   if (comp->InheritsFrom(TGo4FitData::Class()))
      return FindPadWhereData(dynamic_cast<TGo4FitData *>(comp));
   else
      return FindPadWhereModel(dynamic_cast<TGo4FitModel *>(comp));
}

TGo4FitData *TGo4FitPanel::CreateData(int id, const char *name)
{
   TString dataname;

   if (!name) {
      TGo4Fitter *fitter = GetFitter();
      if (!fitter)
         return nullptr;
      dataname = fitter->FindNextName("Data", 0, kFALSE);
   } else
      dataname = name;

   switch (id) {
   case 0: return new TGo4FitDataHistogram(dataname);
   case 1: return new TGo4FitDataGraph(dataname);
   case 2: return new TGo4FitDataProfile(dataname);
   case 3: return new TGo4FitDataRidge(dataname);
   }
   return nullptr;
}

TGo4FitModel *TGo4FitPanel::CreateModel(int id, const char *namebase, TGo4Fitter *fitter, TGo4FitData *data)
{
   TString NameBase;

   if (fitter)
      if (!data && (fitter->GetNumData() > 0))
         data = fitter->GetData(0);
   Int_t NumDim = 0;
   if (data)
      NumDim = data->DefineDimensions();
   if (NumDim <= 0)
      NumDim = 1;

   Int_t order = 0;
   switch (id) {
   case 20: // gaussian
      switch (NumDim) {
      case 1: id = 1; break;
      case 2: id = 2; break;
      default: id = 3; break;
      }
      break;
   case 21:
      order = 1;
      id = 7;
      break; // polynom 1
   case 22:
      order = 3;
      id = 7;
      break; // polynom 3
   case 23:
      order = 7;
      id = 7;
      break; // polynom 7
   }

   if (namebase)
      NameBase = namebase;
   else
      switch (id) {
      case 0: NameBase = "Pol"; break;
      case 1:
      case 2:
      case 3: NameBase = "Gauss"; break;
      case 4: NameBase = "Form"; break;
      case 5: NameBase = "Func"; break;
      case 6: NameBase = "FromData"; break;
      case 7: NameBase = "Pol"; break;
      case 24: NameBase = "Lor"; break;
      case 25: NameBase = "Exp"; break;
      default: NameBase = "Model";
      }

   TString ModelName;

   if (fitter && (id != 7))
      ModelName = fitter->FindNextName(NameBase.Data(), 0);
   else
      ModelName = NameBase;

   TGo4FitModel *model = nullptr;

   switch (id) {
   case 0: model = new TGo4FitModelPolynom(ModelName); break;
   case 1: model = new TGo4FitModelGauss1(ModelName); break;
   case 2: model = new TGo4FitModelGauss2(ModelName); break;
   case 3: model = new TGo4FitModelGaussN(ModelName, NumDim < 3 ? 3 : NumDim); break;
   case 4: model = new TGo4FitModelFormula(ModelName); break;
   case 5: model = new TGo4FitModelFunction(ModelName); break;
   case 6: model = new TGo4FitModelFromData(ModelName); break;
   case 7: fitter->AddPolynoms(data ? data->GetName() : 0, NameBase.Data(), order, NumDim); break;
   case 24: {
      TGo4FitModelFormula *formula = new TGo4FitModelFormula(ModelName, "1./(1.+sq((x-Pos)/Width))", 2, kTRUE);
      formula->SetParsNames("Ampl", "Pos", "Width");
      formula->SetPosParIndex(0, 0);
      formula->SetWidthParIndex(0, 1);
      model = formula;
      break;
   }
   case 25: {
      model = new TGo4FitModelFormula(ModelName, "exp(Koef*x)", 1, kTRUE);
      model->SetParsNames("Ampl", "Koef");
      break;
   }
   }

   if (fitter && model) {
      if (data)
         model->AssignToData(data->GetName());
      fitter->AddModel(model);
   }

   return model;
}

void TGo4FitPanel::LocateModel(TGo4FitModel *model, TGo4FitData *data, bool usepad)
{
   TGo4Fitter *fitter = GetFitter();

   if (!fitter || !model)
      return;

   if (!data)
      for (int n = 0; n <= model->NumAssigments(); n++) {
         data = fitter->FindData(model->AssignmentName(n));
         if (!usepad || FindPadWhereData(data))
            break;
      }
   if (!data)
      return;

   if (!usepad && data->DefineDimensions() != 1)
      usepad = true;

   if (usepad) {
      TPad *pad = FindPadWhereData(data);
      if (pad) {
         model->SetPosition(0, (pad->GetUxmin() + pad->GetUxmax()) / 2.);
         model->SetWidth(0, TMath::Abs(pad->GetUxmax() - pad->GetUxmin()) * .05);
         model->SetPosition(1, (pad->GetUymin() + pad->GetUymax()) / 2.);
         model->SetWidth(1, TMath::Abs(pad->GetUymax() - pad->GetUymin()) * .05);
      }
   } else {
      Double_t mid, width;
      if (fitter->CalculatesMomentums(data->GetName(), kTRUE, kTRUE, mid, width)) {
         model->SetPosition(0, mid);
         model->SetWidth(0, width);
      }
   }
}

int TGo4FitPanel::DefineModelWidgetType(TObject *obj)
{
   if (!obj)
      return FitGui::wt_none;
   if (obj->InheritsFrom(TGo4FitModelPolynom::Class()))
      return FitGui::wt_polynom;
   if (obj->InheritsFrom(TGo4FitModelGauss1::Class()))
      return FitGui::wt_gauss1;
   if (obj->InheritsFrom(TGo4FitModelGauss2::Class()))
      return FitGui::wt_gauss2;
   if (obj->InheritsFrom(TGo4FitModelGaussN::Class()))
      return FitGui::wt_gaussn;
   if (obj->InheritsFrom(TGo4FitModelFormula::Class()))
      return FitGui::wt_formula;
   if (obj->InheritsFrom(TGo4FitModelFunction::Class()))
      return FitGui::wt_function;
   if (obj->InheritsFrom(TGo4FitModel::Class()))
      return FitGui::wt_model;
   return FitGui::wt_none;
}

void TGo4FitPanel::CreateFitSlotLink(TGo4FitSlot *slot, const char *itemname)
{
   if (!slot || !itemname)
      return;

   int slotindex = GetPadIndexForSlot(slot);

   if (slotindex < 0)
      return;

   QString linkname = QString("FitSlotLink_%1").arg(slotindex);

   RemoveLink(linkname.toLatin1().constData());

   AddLink(itemname, linkname.toLatin1().constData());

   GetLinked(linkname.toLatin1().constData(), 1);

   UpdateObjectReferenceInSlot(slot, false);
}

bool TGo4FitPanel::UpdateObjectReferenceInSlot(TGo4FitSlot *slot, bool createlink)
{
   if (!slot)
      return true;

   if (slot->GetOwned())
      return true;

   bool res = true;

   TObject *obj = nullptr;

   if (WorkingWithPanel()) {

      TPad *pad = FindPadForSlot(slot);

      TGo4Slot *objslot = ActivePanel()->GetPadMainObjectSlot(pad);
      obj = !objslot ? nullptr : objslot->GetAssignedObject();

      if (obj && createlink) {
         TString linkname = "data_";
         linkname += slot->GetName();
         AddLink(objslot, linkname);
      }
   } else {
      int slotindex = GetPadIndexForSlot(slot);
      QString linkname = QString("FitSlotLink_%1").arg(slotindex);
      obj = GetLinked(linkname.toLatin1().constData());
   }

   if (!obj)
      res = false;
   if (!obj || !slot->IsSuitable(obj))
      slot->SetObject(nullptr, kFALSE);
   else
      slot->SetObject(obj, kFALSE);

   return res;
}

bool TGo4FitPanel::UpdateObjectReferenceInSlots()
{
   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return false;

   bool res = true;

   // remove all links, responsible to data connection,
   // which are started with "data_"
   //  std::cout << "UpdateObjectReferenceInSlots(). Remove all links " << std::endl;
   if (WorkingWithPanel())
      RemoveLinksMasked("data_");

   // here new links will be create, which are connect widget with datasources
   for (Int_t n = 0; n < fitter->NumSlots(); n++) {
      TGo4FitSlot *slot = fitter->GetSlot(n);
      if (slot->GetClass()->InheritsFrom(TH1::Class()) || slot->GetClass()->InheritsFrom(TGraph::Class()))
         res = res && UpdateObjectReferenceInSlot(slot, true);
   }

   //  std::cout << "UpdateObjectReferenceInSlots() done res = " << res << std::endl;

   return res;
}

void TGo4FitPanel::ClearObjectReferenceInSlots()
{
   if (!WorkingWithPanel())
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   for (Int_t n = 0; n < fitter->NumSlots(); n++) {
      TGo4FitSlot *slot = fitter->GetSlot(n);

      if (slot->GetClass()->InheritsFrom(TH1::Class()) || slot->GetClass()->InheritsFrom(TGraph::Class()))
         if (!slot->GetOwned())
            slot->SetObject(nullptr, kFALSE);
   }
}

void TGo4FitPanel::RemoveDrawObjects()
{
   if (!WorkingWithPanel())
      return;

   TGo4Fitter *fitter = GetFitter();
   if (!fitter)
      return;

   bool res = false;

   for (Int_t n = 0; n < fitter->NumSlots(); n++) {
      TPad *pad = FindPadForSlot(fitter->GetSlot(n));
      if (!pad)
         continue;
      if (ActivePanel()->DeleteDrawObjects(pad, TGo4ViewPanel::kind_FitModels))
         res = true;
      if (ActivePanel()->DeleteDrawObjects(pad, TGo4ViewPanel::kind_FitInfo))
         res = true;
   }
   if (res)
      ActivePanel()->ShootRepaintTimer();
}

void TGo4FitPanel::CloseDrawPanel()
{
   if (fxDrawNewPanel) {
      fxDrawNewPanel->ClosePanel();
      fxDrawNewPanel = nullptr;
   }
}

void TGo4FitPanel::FillNamedWidget(QFitNamedWidget *w)
{
   if (!w)
      return;
   TObject *obj = w->GetObject();
   if (!obj)
      return;

   w->ClassNameLbl->setText(QString("Class name: ") + obj->ClassName());
   w->ClassNameLbl->adjustSize();
   w->NameEdt->setText(obj->GetName());
   w->TitleEdt->setText(obj->GetTitle());
   TNamed *n = dynamic_cast<TNamed *>(obj);
   w->NameEdt->setReadOnly(!n);
   w->TitleEdt->setReadOnly(!n);

   TGo4FitNamed *go4n = dynamic_cast<TGo4FitNamed *>(obj);
   if (go4n && go4n->GetOwner())
      w->FullNameLbl->setText(QString("Full name: ") + go4n->GetFullName());
   else
      w->FullNameLbl->setText("");
   w->FullNameLbl->adjustSize();

   QFitModelWidget *mw = dynamic_cast<QFitModelWidget *>(w);
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(obj);
   if (mw && model) {
      mw->AmplChk->setChecked(model->GetAmplPar() != nullptr);
      mw->AmplChk->setEnabled(model->CanAmplTouch());
      mw->BuffersChk->setChecked(model->GetUseBuffers());
      mw->GroupSpin->setValue(model->GetGroupIndex());
   }
}

void TGo4FitPanel::ChangeObjectName(QFitNamedWidget *w, const char *newname)
{
   if (!w)
      return;
   QFitItem *item = w->GetItem();
   if (!item || !newname || (strlen(newname) == 0))
      return;
   TNamed *obj = dynamic_cast<TNamed *>(item->Object());
   if (!obj)
      return;

   if (item->ObjectType() == FitGui::ot_slot)
      return;

   if (item->ObjectType() == FitGui::ot_par)
      if (item->Parent())
         if (item->Parent()->ObjectType() == FitGui::ot_parslist) {
            TGo4FitParsList *pars = dynamic_cast<TGo4FitParsList *>(item->Parent()->Object());
            if (pars->FindPar(newname))
               return;
         }

   if (item->ObjectType() == FitGui::ot_data)
      if (item->Parent())
         if (item->Parent()->ObjectType() == FitGui::ot_datalist) {
            TGo4Fitter *fitter = GetFitter();
            if (!fitter || (fitter->FindData(newname)))
               return;
            fitter->ChangeDataNameInAssignments(obj->GetName(), newname);
            UpdateItemsOfType(FitGui::ot_ass);
         }

   if (item->ObjectType() == FitGui::ot_model)
      if (item->Parent())
         if (item->Parent()->ObjectType() == FitGui::ot_modellist) {
            TGo4Fitter *fitter = GetFitter();
            if (!fitter || (fitter->FindModel(newname)))
               return;
         }

   obj->SetName(newname);

   w->SetWidgetItemText(true);

   TGo4FitNamed *go4n = dynamic_cast<TGo4FitNamed *>(obj);
   if (go4n && go4n->GetOwner())
      w->FullNameLbl->setText(QString("Full name: ") + go4n->GetFullName());
   else
      w->FullNameLbl->setText("");
   w->FullNameLbl->adjustSize();
}

void TGo4FitPanel::ChangeObjectTitle(QFitNamedWidget *w, const char *newtitle)
{
   if (!w)
      return;
   TNamed *n = dynamic_cast<TNamed *>(w->GetObject());
   if (n)
      n->SetTitle(newtitle);
}

void TGo4FitPanel::ChangeModelPar(QFitModelWidget *w, int npar, int value)
{
   if (!w)
      return;
   TGo4FitModel *model = dynamic_cast<TGo4FitModel *>(w->GetObject());
   if (!model)
      return;
   switch (npar) {
   case 0: {
      if (model->CanAmplTouch()) {
         bool res = false;
         if (value != 0)
            res = model->MakeAmpl();
         else
            res = model->RemoveAmpl();
         if (res)
            w->UpdateItemsOfType(FitGui::ot_parslist, false);
      }
      break;
   }

   case 1: {
      model->SetUseBuffers(value != 0);
      break;
   }

   case 2: {
      model->SetGroupIndex(value);
      break;
   }
   }
}
