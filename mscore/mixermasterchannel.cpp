//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#include "mixermasterchannel.h"

#include "libmscore/instrument.h"
#include "musescore.h"
#include "synthesizer/msynthesizer.h"     // required for MidiPatch

#include "mixer.h"
#include "mixertrackitem.h"
#include "mixeroptions.h"

namespace Ms {

//--------------------------------------------------------------
//  MixerMasterChannel provides an widget that is displayed in a
//  row of a QTreeWidget. The widget includes a slider (by default
//  to control track volume) and Play and Loop buttons. It shares
//  the UI class with MixerMasterChannel as the master control
//  needs to look like the track controls.
//--------------------------------------------------------------
MixerMasterChannel::MixerMasterChannel()
      {
      setupUi(this);
      volumeSlider->setMinimum(-60); // in case .ui file gets stuffed up
      volumeSlider->setMaximum(20);  // in case .ui file gets stuffed up

      playButton->setDefaultAction(getAction("play"));
      loopButton->setDefaultAction(getAction("loop"));

      volumeSlider->setDoubleValue(synti->gain());

      setupAdditionalUi();
      setupSlotsAndSignals();
      }


void MixerMasterChannel::setupSlotsAndSignals()
      {
      connect(volumeSlider, SIGNAL(valueChanged(int)), SLOT(masterVolumeSliderMoved(int)));
      }


void MixerMasterChannel::setupAdditionalUi()
      {
      // the label is retained but made transparent to preserve
      // alignment with the track channel sliders
      QString transparentColorLabelStyle = "QToolButton { background: none;}";
      colorLabel->setStyleSheet(transparentColorLabelStyle);
      }


void MixerMasterChannel::updateUiControls()
      {
      colorLabel->setVisible(false);
      }

      
void MixerMasterChannel::volumeChanged(float synthGain)
      {
      const QSignalBlocker blockSignals(volumeSlider); // block during this method
      volumeSlider->setDoubleValue(synthGain);
      }


void MixerMasterChannel::masterVolumeSliderMoved(int value)
      {
      const QSignalBlocker blockSignals(volumeSlider); // block during this method
      float newGain = volumeSlider->doubleValue();
      if (newGain == synti->gain())
            return;

      synti->setGain(newGain);

      //TODO:- magic numbers (needs to be made unmagical)
      float n = 20.0;         // from playpanel.h
      float mute = 0.0;       // from playpanel.h
      float decibels = (newGain == mute) ? -80.0 : ((n * std::log10(newGain)) - n);
      volumeSlider->setToolTip(tr("Volume: %1 dB").arg(QString::number(decibels, 'f', 1)));
      }

}
