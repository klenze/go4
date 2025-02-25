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

#include "TGo4DividePad.h"

#include "TGo4MdiArea.h"
#include "TGo4ViewPanel.h"

TGo4DividePad::TGo4DividePad( QWidget* parent, const char *name, Qt::WindowFlags fl ) :
   QWidget( parent, fl )
{
   setObjectName(name ? name : "Go4DividePad");
   setupUi(this);
   QObject::connect(DivideCanvas_b, &QPushButton::clicked, this, &TGo4DividePad::DivideCanvas);
}

void TGo4DividePad::DivideCanvas()
{
  TGo4ViewPanel* panel = TGo4MdiArea::Instance()->GetActivePanel();

  if (panel)
     panel->Divide(SpinBoxX->value(), SpinBoxY->value());
}
