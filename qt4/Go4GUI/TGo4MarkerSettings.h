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

#ifndef TGO4MARKERSETTINGS_H
#define TGO4MARKERSETTINGS_H

#include <QDialog>
#include "ui_TGo4MarkerSettings.h"

class TGo4MarkerSettings : public QDialog, public Ui::TGo4MarkerSettings
{
   Q_OBJECT

public:
   TGo4MarkerSettings(QWidget* parent = nullptr);

public slots:
   virtual void setFlags();

};

#endif
