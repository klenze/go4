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

#include "QFitParWidget.h"

#include "TGo4FitParameter.h"

QFitParWidget::QFitParWidget(QWidget *parent, const char *name)
         : QFitNamedWidget(parent, name)
{
   setupUi(this);
   QObject::connect(ValueEdt, &QGo4LineEdit::textChanged, this, &QFitParWidget::ValueEdt_textChanged);
   QObject::connect(ErrorEdt, &QGo4LineEdit::textChanged, this, &QFitParWidget::ErrorEdt_textChanged);
   QObject::connect(RangeMinEdt, &QGo4LineEdit::textChanged, this, &QFitParWidget::RangeMinEdt_textChanged);
   QObject::connect(RangeMaxEdt, &QGo4LineEdit::textChanged, this, &QFitParWidget::RangeMaxEdt_textChanged);
   QObject::connect(EpsilonEdt, &QGo4LineEdit::textChanged, this, &QFitParWidget::EpsilonEdt_textChanged);
   QObject::connect(FixedChk, &QCheckBox::toggled, this, &QFitParWidget::FixedChk_toggled);
   QObject::connect(RangeChk, &QCheckBox::toggled, this, &QFitParWidget::RangeChk_toggled);
   QObject::connect(EpsilonChk, &QCheckBox::toggled, this, &QFitParWidget::EpsilonChk_toggled);
}

TGo4FitParameter* QFitParWidget::GetPar()
{
   return dynamic_cast<TGo4FitParameter*> (GetObject());
}

void QFitParWidget::FillSpecificData()
{
   QFitNamedWidget::FillSpecificData();
   TGo4FitParameter* fxParameter = GetPar();
   if (!fxParameter) return;

   ValueEdt->setText(QString::number(fxParameter->GetValue()));
   ErrorEdt->setText(QString::number(fxParameter->GetError()));
   FixedChk->setChecked(fxParameter->GetFixed());

   Double_t min, max;

   bool b = fxParameter->GetRange(min,max);

   RangeChk->setChecked(b);
   RangeMinEdt->setEnabled(b);
   RangeMaxEdt->setEnabled(b);

   RangeMinEdt->setText(QString::number(min));
   RangeMaxEdt->setText(QString::number(max));

   Double_t eps;
   b = fxParameter->GetEpsilon(eps);
   EpsilonChk->setChecked(b);
   EpsilonEdt->setEnabled(b);
   EpsilonEdt->setText(QString::number(eps));
}

void QFitParWidget::ValueEdt_textChanged( const QString & value)
{
  if(!fbFillWidget && GetPar()) {
     bool ok;
     double zn = value.toDouble(&ok);
     if(ok) {
       GetPar()->SetValue(zn);
       SetWidgetItemText(true);
     }
  }
}

void QFitParWidget::ErrorEdt_textChanged( const QString & value)
{
  if(!fbFillWidget && GetPar()) {
     bool ok;
     double zn = value.toDouble(&ok);
     if(ok) GetPar()->SetError(zn);
  }
}

void QFitParWidget::RangeMinEdt_textChanged( const QString & value)
{
  if(!fbFillWidget && GetPar()) {
     bool ok;
     double zn = value.toDouble(&ok);
     if(ok) GetPar()->ChangeRangeMin(zn);
  }
}


void QFitParWidget::RangeMaxEdt_textChanged( const QString & value)
{
  if(!fbFillWidget && GetPar()) {
     bool ok;
     double zn = value.toDouble(&ok);
     if(ok) GetPar()->ChangeRangeMax(zn);
  }
}


void QFitParWidget::EpsilonEdt_textChanged( const QString & value)
{
  if(!fbFillWidget && GetPar()) {
     bool ok;
     double zn = value.toDouble(&ok);
     if(ok) GetPar()->ChangeEpsilon(zn);
  }
}

void QFitParWidget::FixedChk_toggled( bool zn)
{
  if(!fbFillWidget && GetPar())
    GetPar()->SetFixed(zn);
}

void QFitParWidget::RangeChk_toggled( bool zn)
{
   if(!fbFillWidget && GetPar()) {
      RangeMinEdt->setEnabled(zn);
      RangeMaxEdt->setEnabled(zn);
      GetPar()->SetRangeUse(zn);
   }
}

void QFitParWidget::EpsilonChk_toggled( bool zn)
{
   if(!fbFillWidget && GetPar()) {
      EpsilonEdt->setEnabled(zn);
      GetPar()->SetEpsilonUse(zn);
   }
}
