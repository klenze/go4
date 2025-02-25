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

#ifndef QFITAMPLESTIMWIDGET_H
#define QFITAMPLESTIMWIDGET_H

#include "QFitNamedWidget.h"
#include "ui_QFitAmplEstimWidget.h"

class TGo4FitAmplEstimation;

class QFitAmplEstimWidget : public QFitNamedWidget, public Ui::QFitAmplEstimWidget
{
     Q_OBJECT

 public:
    QFitAmplEstimWidget( QWidget* parent = nullptr, const char *name = nullptr);

    void FillSpecificData() override;

    TGo4FitAmplEstimation * GetAmplEstim();

 public slots:
    virtual void Iterations_valueChanged(int);
};

#endif
