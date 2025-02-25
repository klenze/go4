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

#include "QFitDependencyWidget.h"

#include "QFitItem.h"
#include "TObjArray.h"
#include "TGo4FitDependency.h"

QFitDependencyWidget::QFitDependencyWidget(QWidget *parent, const char *name)
         : QFitWidget(parent, name)
{
   setupUi(this);

   QObject::connect(ParamEdit, &QGo4LineEdit::textChanged, this, &QFitDependencyWidget::ParamEdit_textChanged);
   QObject::connect(ExpressionEdt, &QGo4LineEdit::textChanged, this, &QFitDependencyWidget::ExpressionEdt_textChanged);
}

TGo4FitDependency * QFitDependencyWidget::GetDepen()
{
  return dynamic_cast<TGo4FitDependency*> (GetObject());
}

void QFitDependencyWidget::FillSpecificData()
{
   if(GetDepen()) {
       if (GetDepen()->IsResultDepend()) {
          TObjArray* lst = dynamic_cast<TObjArray*> (GetItem()->Parent()->Object());
          ParamEdit->setText(QString("Result ")+QString::number(lst->IndexOf(GetObject())));
          ParamEdit->setReadOnly(true);
          ParamEdit->setEnabled(false);
       } else {
          ParamEdit->setText(GetDepen()->GetParameter().Data());
          ParamEdit->setReadOnly(false);
          ParamEdit->setEnabled(true);
       }
       if (GetDepen()->IsInitValue()) ExpressionEdt->setText(QString::number(GetDepen()->GetInitValue()));
                                 else ExpressionEdt->setText(GetDepen()->GetExpression().Data());
   }
}


void QFitDependencyWidget::ParamEdit_textChanged( const QString & param)
{
  if(!fbFillWidget && GetDepen() && (param.length()>0)) {
     GetDepen()->SetParameter(param.toLatin1().constData());
     SetWidgetItemText(true);
  }
}


void QFitDependencyWidget::ExpressionEdt_textChanged( const QString & value)
{
  if(!fbFillWidget && GetDepen() && (value.length()>0)) {
     bool ok = false;
     double zn = value.toDouble(&ok);
     if (ok) GetDepen()->SetInitValue(zn);
        else GetDepen()->SetExpression(value.toLatin1().constData());
     SetWidgetItemText(true);
  }
}
