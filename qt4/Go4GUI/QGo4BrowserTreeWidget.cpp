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

#include "QGo4BrowserTreeWidget.h"

#include <QDropEvent>
#include <QDrag>

QGo4BrowserTreeWidget::QGo4BrowserTreeWidget(QWidget* parent) :
   QTreeWidget(parent)
{
}

QGo4BrowserTreeWidget::~QGo4BrowserTreeWidget()
{
}

QStringList QGo4BrowserTreeWidget::mimeTypes () const
{
   QStringList qstrList;
   qstrList.append("text");
   return qstrList;
}

Qt::DropActions QGo4BrowserTreeWidget::supportedDropActions () const
{
   // returns what actions are supported when dropping
   return Qt::CopyAction | Qt::MoveAction;
}

bool QGo4BrowserTreeWidget::dropMimeData(QTreeWidgetItem *item, int, const QMimeData *data, Qt::DropAction)
{
   emit ItemDropProcess((void*)item, (void*)data);

   return true;
}

void QGo4BrowserTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
   // if not left button - return
   if (event->buttons() & Qt::LeftButton) {
      QDrag *drag = nullptr;
      // request from browser drag
       emit RequestDragObject(&drag);

      if (drag)
         drag->exec(Qt::CopyAction | Qt::MoveAction);
   }
}
