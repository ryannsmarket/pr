//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: editinstrument.h 4052 2011-02-23 15:29:16Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//=============================================================================

#ifndef __EDITINSTRUMENT_H__
#define __EDITINSTRUMENT_H__

#include "ui_editinstrument.h"

namespace Ms {

class InstrumentTemplate;

//---------------------------------------------------------
//   EditInstrument
//---------------------------------------------------------

class EditInstrument : public QDialog, private Ui::EditInstrumentBase
      {
      Q_OBJECT

      InstrumentTemplate* instr;
      InstrumentTemplate* lt;

   private slots:
      void on_buttonCancel_pressed();
      void on_buttonOk_pressed();
      void valueChanged();

   public:
      EditInstrument(QWidget* parent = 0);
      ~EditInstrument();
      void setInstrument(InstrumentTemplate* instr);
      };


} // namespace Ms
#endif

