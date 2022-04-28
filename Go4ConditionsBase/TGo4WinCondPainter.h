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

#ifndef TGO4WINCONDPAINTER_H
#define TGO4WINCONDPAINTER_H

#include "TGo4ConditionPainter.h"

class TGo4WinCondView;

class TGo4WinCondPainter : public TGo4ConditionPainter {
  public:
     TGo4WinCondPainter();
     TGo4WinCondPainter(const char* name, const char* title = "Go4 window condition");
     virtual ~TGo4WinCondPainter();

     /** Display condition with plain root canvas and in go4 viewpanel */
     void PaintCondition(Option_t* opt="") override;

     /** Erase condition view  */
     void UnPaintCondition(Option_t* opt="") override;

     /** pop all components to the front of the pad*/
     void DisplayToFront(Option_t* opt="") override;

  private:

    /** Window marker box. */
    TGo4WinCondView* fxBox{nullptr};

    ClassDefOverride(TGo4WinCondPainter,6)
};

#endif //TGO4WINCONDPAINTER_H
