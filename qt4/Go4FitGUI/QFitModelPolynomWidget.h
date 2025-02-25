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

#ifndef QFITMODELPOLYNOMWIDGET_H
#define QFITMODELPOLYNOMWIDGET_H

#include "QFitModelWidget.h"
#include "ui_QFitModelPolynomWidget.h"

class TGo4FitModelPolynom;

class QFitModelPolynomWidget : public QFitModelWidget, public Ui::QFitModelPolynomWidget
 {

    Q_OBJECT

public:
    QFitModelPolynomWidget(QWidget *parent = nullptr, const char *name = nullptr);
    void FillSpecificData() override;

public slots:
    virtual TGo4FitModelPolynom * GetPolynom();
    virtual void AxisSpin_valueChanged( int );
};

#endif // QFITMODELPOLYNOMWIDGET_H

