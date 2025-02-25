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

#ifndef TGO4PARAEDIT_H
#define TGO4PARAEDIT_H

#include "QGo4Widget.h"
#include "ui_TGo4ParaEdit.h"

class QMenuBar;
class TGo4Fitter;
class TGo4FitPanel;
class QStatusBar;
class TObject;
class TGo4Parameter;
class TGo4ParameterStatus;
class TObjArray;
class TClass;

class TGo4ParaEdit : public QGo4Widget, public Ui::TGo4ParaEdit
{
    Q_OBJECT

public:

   enum ParEditColumns { fiColName = 0, fiColType = 1, fiColValue = 2, fiColComment = 3 };

   TGo4ParaEdit(QWidget *parent = nullptr, const char *name = nullptr);
   virtual ~TGo4ParaEdit();

   bool IsAcceptDrag(const char *itemname, TClass* cl, int kind) override;
   void DropItem(const char *itemname, TClass* cl, int kind) override;
   void linkedObjectUpdated(const char *linkname, TObject *obj) override;
   void linkedObjectRemoved( const char * linkname ) override;
   virtual void WorkWithParameter(const char *itemname, bool isrefresh);
   void ResetWidget() override;
   virtual void RefreshWidget(TGo4Parameter* par);
   virtual void RefreshWidget(TGo4ParameterStatus* parstatus);
   virtual void ShowVisibleItems();

public slots:
   virtual void clearTextFields();
   virtual void ChangedTable( int row, int col );
   virtual void TableDoubleClick( int row, int col );
   virtual void TableContextMenu( const QPoint& );
   virtual void ExpandShrinkArray();
   virtual void IncArrayIndex();
   virtual void DecArrayIndex();
   virtual void EditFitter();
   virtual void GetFitterFromEditor();
   virtual void saveFile();
   virtual void RefreshClicked();
   virtual void ApplyClicked();

 protected:
   int fiCurrentRow{0};
   TObjArray* fItems{nullptr};
   QString fItemName;
   bool fFillingTable{false};
};

#endif
