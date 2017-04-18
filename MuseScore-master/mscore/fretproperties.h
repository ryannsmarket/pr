//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __FRETPROPERTIES_H__
#define __FRETPROPERTIES_H__

#include "ui_fretdprops.h"

namespace Ms {

class FretDiagram;

//---------------------------------------------------------
//   FretDiagramProperties
//---------------------------------------------------------

class FretDiagramProperties : public QDialog, public Ui::FretDiagramProperties {
      Q_OBJECT
      FretDiagram* fd;

   private slots:
      void stringsChanged(int);
      void fretsChanged(int);
      void fretOffsetChanged(int);

   public:
      FretDiagramProperties(FretDiagram*, QWidget* parent = 0);
      };


} // namespace Ms
#endif
