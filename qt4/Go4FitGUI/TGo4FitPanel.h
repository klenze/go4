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

#ifndef TGO4FITPANEL_H
#define TGO4FITPANEL_H

#include "QGo4Widget.h"
#include "ui_TGo4FitPanel.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QWidgetStack;
class QWidget;
class QSplitter;
class QPushButton;
class QSpinBox;
class QLabel;
class QTableWidget;
class QCheckBox;
class QComboBox;
class QTabWidget;
class QGo4LineEdit;
class QSlider;
class QFrame;
class QLineEdit;
class TGo4ViewPanel;
class TPad;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QMenuBar;
class QFitItem;
class QFitNamedWidget;
class QFitModelWidget;
class TObject;
class TObjArray;
class TGo4Slot;
class TGo4FitSlot;
class TGo4FitComponent;
class TGo4FitModel;
class TGo4FitData;
class TGo4Fitter;
class TGo4FitPeakFinder;
class TGo4FitGuiArrow;

// =======================================================================================

class TGo4FitPanel : public QGo4Widget, public Ui::TGo4FitPanel
 {
    Q_OBJECT

public:
    TGo4FitPanel(QWidget *parent = nullptr, const char *name = nullptr);
    virtual ~TGo4FitPanel();

    bool IsAcceptDrag(const char *itemname, TClass *cl, int kind) override;
    void linkedObjectUpdated( const char * linkname, TObject * obj) override;
    void linkedObjectRemoved( const char * linkname ) override;
    void linkedRemoved(TGo4Slot* slot, TObject *obj) override;
    virtual void DropOnPanel( QDropEvent* event, const char * itemname, TClass * cl, int kind );
    virtual void CreateFitSlotLink(TGo4FitSlot* slot, const char * itemname);
    virtual void WorkWithFitter(const char *itemname, TGo4ViewPanel* panel, TPad* pad);
    virtual void WorkWithFitterInSlot(TGo4Slot* slot);
    virtual bool WorkingWithPanel();
    virtual bool WorkingWithOnlyPad();
    virtual TGo4ViewPanel* ActivePanel();
    virtual TPad* ActivePad();
    virtual TGo4Fitter* GetFitter();
    virtual TGo4Fitter* CloneFitter();
    virtual void RemoveFitterLink();
    virtual void SetFitter(TGo4Fitter*);
    virtual void Fitter_New();
    virtual void Fitter_NewForActivePad(bool overwrite);
    virtual void Fitter_Delete();
    virtual void Fitter_UseWorkspace();
    virtual void Fitter_UpdateReferences();
    virtual void Fitter_SaveToBrowser();
    virtual void Fitter_PrintParameters();
    virtual void Fitter_RollbackParameters();
    virtual void Button_WorkWithPanel();
    virtual void Button_SimpleFit(int nmodel);
    virtual void Button_SimpleClear();
    virtual void Button_PeakFinder();
    virtual void Button_PerformFit();
    virtual void Button_FitterDraw(TGo4FitData* selecteddata);
    virtual void Cmd_CreateFitter();
    virtual void Cmd_CreateAppropriateFitter();
    virtual void Cmd_DeleteFitter();
    virtual void Cmd_ClearFitter();
    virtual void Cmd_SaveFitter(bool ask);
    virtual void Cmd_ItemPrint(QFitItem* item);
    virtual void Cmd_AddNewData(QFitItem * item, int id);
    virtual void Cmd_DrawData(QFitItem * item);
    virtual void Cmd_DeleteData(QFitItem * item);
    virtual void Cmd_DeleteAssosiatedModels(QFitItem * item);
    virtual void Cmd_DeleteAllData(QFitItem * item);
    virtual void Cmd_AddNewModel(QFitItem * item, int id);
    virtual void Cmd_DeleteModel(QFitItem * item);
    virtual void Cmd_RemoveModel(TGo4FitModel* model);
    virtual void Cmd_DeleteModels(QFitItem * item);
    virtual void Cmd_CloneModel(QFitItem * item);
    virtual void Cmd_ClearAssigment(QFitItem* item);
    virtual void Cmd_ClearAssigments(QFitItem* item);
    virtual void Cmd_AssignModelToAllData(QFitItem* item);
    virtual void Cmd_AssignModelToData(QFitItem* item, int id);
    virtual void Cmd_RemoveRangeCondition(QFitItem* item);
    virtual void Cmd_RemoveRangeConditions(QFitItem* item);
    virtual void Cmd_AddRangeCondition(QFitItem* item, int id);
    virtual void Cmd_DeleteAction(QFitItem* item);
    virtual void Cmd_DeleteActions(QFitItem* item);
    virtual void Cmd_MoveAction(QFitItem* item, int dir);
    virtual void Cmd_ExecuteAction(QFitItem* item);
    virtual void Cmd_ExecuteActions(QFitItem* item, bool expert);
    virtual void Cmd_DeleteOutputActions(QFitItem* item);
    virtual void Cmd_AddNewAction(QFitItem* item, int id);
    virtual void Cmd_DeleteDependency(QFitItem* item);
    virtual void Cmd_DeleteDependencies(QFitItem* item);
    virtual void Cmd_AddDependency(QFitItem* item);
    virtual void Cmd_MemorizePars(QFitItem* item);
    virtual void Cmd_RememberPars(QFitItem* item);
    virtual void Cmd_DeletePars(QFitItem* item);
    virtual void Cmd_AddNewPar(QFitItem* item);
    virtual void Cmd_MemorizePar(QFitItem* item);
    virtual void Cmd_RememberPar(QFitItem* item);
    virtual void Cmd_DeletePar(QFitItem* item);
    virtual void Cmd_DeleteMinuitResult(QFitItem* item);
    virtual void Cmd_UpdateAllSlots(QFitItem* item);
    virtual void UpdateActivePage();
    virtual void UpdateSimplePage();
    virtual void UpdateWizardPage();
    virtual void UpdateExtendedPage();
    virtual void RemovePrimitives();
    virtual TGo4Fitter* CreateFitterFor(TGo4ViewPanel* panel, TPad* pad, const char *name);
    virtual void CreateDataFor(TGo4ViewPanel* panel, TPad* pad, TGo4Fitter* fitter);
    virtual bool FillPopupForItem(QFitItem* item, QMenu* menu);
    virtual bool FillPopupForSlot(TGo4FitSlot* slot, QMenu* menu);
    virtual void ExecutePopupForSlot(QFitItem* item, TGo4FitSlot* slot, int id);
    virtual QFitItem* GetFitterItem();
    virtual QFitItem* FindItem(TObject *obj, int ObjectType, QFitItem* parent);
    virtual bool ShowItem(QFitItem* item, bool force);
    virtual bool ShowItemAsText(QFitItem* item, bool force);
    virtual bool ShowItemAsGraph(QFitItem* item, bool force);
    virtual void RemoveItemWidget();
    virtual void UpdateItem(QFitItem* item, bool trace);
    virtual void SetItemText(QFitItem* item, bool trace);
    virtual void UpdateItemsOfType(int typ, QFitItem* parent = nullptr);
    virtual void FitItemDeleted(QFitItem* item);
    virtual void FillModelTypesList(QMenu* menu, int id, bool extend, const QString &handler = "");
    virtual void FillDataTypesList(QMenu* menu, int id, const QString &handler = "");
    virtual void FillParsList(QFitItem* item);
    virtual void FillSlotsList(QFitItem* parent, const TObjArray* lst, TObject* owner);
    virtual void FillDependencyList(QFitItem* parent);
    virtual void PaintFitter(TGo4Fitter* fitter, QFitItem* item, bool update);
    virtual bool PaintModel(TGo4FitModel* model, TPad* pad, QFitItem* item);
    virtual bool PaintModelsFor(TGo4Fitter* fitter, TGo4FitData* data, QFitItem* item, bool update);
    virtual bool PaintRange(TGo4FitComponent* comp, int nrange, TPad* pad, QFitItem* item);
    virtual TGo4FitPeakFinder* GetPeakFinder(bool autocreate);
    virtual TGo4FitData* Wiz_SelectedData();
    virtual TGo4FitModel* Wiz_SelectedModel();
    virtual void Wiz_CreateNewModel(int id);
    virtual void Wiz_CreateNewData(int id);
    virtual void Wiz_RebuildDataList();
    virtual bool Wiz_RemoveData();
    virtual void Wiz_UseSelectedRange();
    virtual void Wiz_TakeCurrentRange();
    virtual void Wiz_GetModelInfo(TGo4FitModel* model, QString* info);
    virtual QString Wiz_GetSlotSourceInfo(TGo4FitSlot* slot);
    virtual void ArrowChanged(TGo4FitGuiArrow* arr);
    virtual void DeleteModelWithPrimit(TGo4FitGuiArrow* arr);
    virtual int GetPadIndexForSlot(TGo4FitSlot* slot);
    virtual TPad* FindPadForSlot(TGo4FitSlot* slot);
    virtual TPad* FindPadWhereData(TGo4FitData* data);
    virtual TPad* FindPadWhereModel(TGo4FitModel* model);
    virtual TPad* FindPadWhereComp(TGo4FitComponent* comp);
    virtual TGo4FitData* CreateData(int id, const char *name);
    virtual TGo4FitModel* CreateModel(int id, const char *namebase, TGo4Fitter* fitter, TGo4FitData* data);
    virtual void LocateModel(TGo4FitModel* model, TGo4FitData* data, bool usepad);
    virtual int DefineModelWidgetType(TObject *obj);
    virtual bool UpdateObjectReferenceInSlot(TGo4FitSlot* slot, bool createlink);
    virtual bool UpdateObjectReferenceInSlots();
    virtual void ClearObjectReferenceInSlots();
    virtual void RemoveDrawObjects();    //virtual void windowActivationChange( bool OldActive );

    virtual void CloseDrawPanel();
    virtual void UpdateStatusBar(const char *info = nullptr);
    virtual void UpdateItemMenu();
    virtual void UpdatePFAmplLbl();
    virtual void UpdatePFRelNoiseLbl();
    virtual void UpdateWizDataList();
    virtual void UpdateWizDataBtns();
    virtual void UpdateWizModelsList(bool changestack);
    virtual void UpdateWizModelsBtns();
    virtual void UpdateWizStackWidget();
    virtual void UpdateWizPaint(int mode);
    virtual void FillParsTable(QTableWidget *table, TGo4Fitter *fitter, TGo4FitModel *model, bool LinesView, TObjArray *TableList);
    virtual void FillNamedWidget(QFitNamedWidget* w);
    virtual void ChangeObjectName(QFitNamedWidget* w, const char *newname);
    virtual void ChangeObjectTitle(QFitNamedWidget* w, const char *newtitle);
    virtual void ChangeModelPar(QFitModelWidget* w, int npar, int value);

public slots:
    virtual void panelSlot(TGo4ViewPanel*, TPad*, int);
    virtual void FitList_customContextMenuRequested(const QPoint &);
    virtual void FitList_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
    virtual void AboutToShowViewMenu();
    virtual void ChangeViewType(int id);
    virtual void ShowPanelPage(int id);
    virtual void AboutToShowSettMenu();
    virtual void ChangeSettings( int id );
    virtual void AboutToShowFitterMenu();
    virtual void FitterMenuItemSelected( int id );
    virtual void AboutToShowItemMenu();
    virtual void ItemMenuItemSelected( int id );
    virtual void PF_MinWidthEdt_textChanged( const QString & str );
    virtual void PF_MaxWidthEdt_textChanged( const QString & str );
    virtual void PF_WidthEdit_textChanged( const QString & str );
    virtual void PF_AmplSlider_valueChanged( int zn );
    virtual void PF_AmplSlider_sliderReleased();
    virtual void PF_RelNoiseSlider_valueChanged( int zn );
    virtual void PF_RelNoiseSlider_sliderReleased();
    virtual void PF_MinNoiseEdit_textChanged( const QString & str );
    virtual void PF_SumUpSpin_valueChanged( int num );
    virtual void Wiz_DataListSelect(QListWidgetItem*);
    virtual void Wiz_ModelListSelect(QListWidgetItem*);
    virtual void Wiz_AddDataBtn_clicked();
    virtual void Wiz_DelDataBtn_clicked();
    virtual void Wiz_AddModelBtn_clicked();
    virtual void Wiz_DelModelBtn_clicked();
    virtual void Wiz_CloneModelBtn_clicked();
    virtual void Wiz_ShowAllMod_toggled( bool zn );
    virtual void Wiz_FitFuncCmb_activated( int typ );
    virtual void Wiz_FitNameEdt_textChanged( const QString & name );
    virtual void Wiz_ParTable_valueChanged( int nrow, int ncol );
    virtual void Wiz_DataList_doubleClicked(QListWidgetItem*);
    virtual void Wiz_ModelList_doubleClicked(QListWidgetItem*);
    virtual void Wiz_ModelList_itemChanged(QListWidgetItem*);
    virtual void Wiz_RebuildDataBtn_clicked();
    virtual void Wiz_DataBufChk_toggled( bool zn );
    virtual void Wiz_UseAmplEstimChk_toggled( bool chk );
    virtual void Wiz_MigradIterSpin_valueChanged( int num );
    virtual void Wiz_DataSlotsTable_contextMenuRequested(const QPoint & pnt );
    virtual void Wiz_DataUseRangeBtn_clicked();
    virtual void Wiz_DataClearRangesBtn_clicked();
    virtual void Wiz_DrawDataBtn_clicked();
    virtual void Wiz_ModelBufChk_toggled( bool zn );
    virtual void Wiz_PFSetupBtn_clicked();
    virtual void Wiz_PFUsePolynChk_toggled( bool zn );
    virtual void Wiz_PFPolynSpin_valueChanged( int zn );
    virtual void Wiz_MinSetupBtn_clicked();
    virtual void MainAttachBtn_clicked();
    virtual void MainFitBtn_clicked();
    virtual void MainDrawBtn_clicked();
    virtual void MainFindBtn_clicked();
    virtual void MainParsBtn_clicked();
    virtual void FillParsWidget();
    virtual void LineParsChk_toggled( bool );
    virtual void ParsTable_valueChanged( int nrow, int ncol );
    virtual void ParsTableChanged( QTableWidget * table, int nrow, int ncol,
                                    bool LinesView, TObjArray * TableList,
                                    bool updatepaint );
    virtual void Smp_PolynomBtn_clicked();
    virtual void Smp_GaussianBtn_clicked();
    virtual void Smp_LorenzBtn_clicked();
    virtual void Smp_ExponentBtn_clicked();
    virtual void Smp_ClearBtn_clicked();
    virtual void FindersTab_currentChanged( int);
    virtual void Wiz_BackgroundChk_toggled( bool chk );
    virtual void PF_MinWidthEdt_returnPressed();
    virtual void PF_MaxWidthEdt_returnPressed();
    virtual void PF_WidthEdit_returnPressed();
    virtual void PF_MinNoiseEdit_returnPressed();

public:
   QFitItem* CurrFitItem{nullptr};

protected:
    QMenuBar* MenuBar{nullptr};
    QMenu* FitterMenu{nullptr};
    QSignalMapper* FitterMap{nullptr};
    QMenu* ViewMenu{nullptr};
    QSignalMapper* ViewMap{nullptr};
    QMenu* SettMenu{nullptr};
    QSignalMapper* SettMap{nullptr};
    QMenu* ItemMenu{nullptr};

    bool fbFillingWidget{false};
    int fiPaintMode{0};
    TObjArray* fxWizPars{nullptr};
    TObjArray* fxWizSlots{nullptr};
    TObjArray* fxParsTableList{nullptr};
    TGo4ViewPanel* fxActivePanel{nullptr};
    TPad* fxActivePad{nullptr};
    TGo4ViewPanel* fxDrawNewPanel{nullptr};
    bool fbDrawPanelCreation{false};
    QFitItem* fxCurrentItem{nullptr};
    QWidget* fxCurrentItemWidget{nullptr};
    bool fbNeedConfirmation{false};
    bool fbShowPrimitives{false};
    bool fbFreezeMode{false};
    int fiPanelMode{0};
    bool fbParsWidgetShown{false};
    bool fbDrawModels{false};
    bool fbDrawComponents{false};
    bool fbDrawBackground{false};
    bool fbUseSamePanelForDraw{false};
    bool fbDrawInfoOnPad{false};
    int fiIntegralMode{0};
    bool fbRecalculateGaussWidth{false};
    int fiBuffersUsage{0};
    bool fbSaveWithReferences{false};
    bool fbUseCurrentRange{false};
    QString fxWizDataName;
    QString fxWizModelName;
    bool fbWizShowAllModels{false};
    int fiWizPageIndex{0};
    bool fbUseAmplEstim{false};
    int fiNumMigradIter{0};

    void focusInEvent(QFocusEvent * event) override;
    void changeEvent(QEvent *event) override;
    bool checkConfirm(const QString &title, const QString &msg);
    void AddItemAction(QMenu* menu, const QString &name, int id, const QString &handler = "");
};

#endif
