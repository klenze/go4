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

#ifndef QFITMATRIXTRANSWIDGET_H
#define QFITMATRIXTRANSWIDGET_H

#include "QFitNamedWidget.h"
#include "ui_QFitMatrixTransWidget.h"

class TGo4FitMatrixTrans;

class QFitMatrixTransWidget : public QFitNamedWidget, public Ui::QFitMatrixTransWidget
 {
     Q_OBJECT


public:
    QFitMatrixTransWidget(QWidget *parent = nullptr, const char *name = nullptr);

    void FillSpecificData() override;

public slots:
    virtual TGo4FitMatrixTrans * GetMatrixTrans();
    virtual void NumAxisSpin_valueChanged( int );

 };

#endif
