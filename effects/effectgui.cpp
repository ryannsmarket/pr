//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "effectgui.h"
#include "effect.h"
#include <QQmlContext>

namespace Ms {

//---------------------------------------------------------
//   EffectGui
//---------------------------------------------------------

EffectGui::EffectGui(Effect* e, QWidget* parent)
   : QWidget(parent)
      {
      _effect = e;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void EffectGui::init(QUrl& url)
      {
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EffectGui::valueChanged(const QString& msg, qreal val)
      {
      if (_effect->value(msg) != val) {
            _effect->setValue(msg, val);
            emit valueChanged();
            }
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void EffectGui::updateValues()
      {
      }
}

