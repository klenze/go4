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

#ifndef TGO4OPTSTATSSETTINGS_H
#define TGO4OPTSTATSSETTINGS_H

#include <QDialog>
#include "ui_TGo4OptStatsSettings.h"

class TGo4OptStatsSettings : public QDialog, public Ui::TGo4OptStatsSettings
{
   Q_OBJECT

 public:
   TGo4OptStatsSettings(QWidget* parent = nullptr);

public slots:

   void setFlags();

protected:
   double fStatW{0};
   double fStatH{0};
};

#endif
