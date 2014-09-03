#include "importmidi_key.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "libmscore/key.h"
#include "libmscore/staff.h"
#include "preferences.h"


// This simple key detection algorithm is from thesis
// "Inferring Score Level Musical Information From Low-Level Musical Data", 2004
// by Jürgen Kilian

namespace Ms {
namespace MidiKey {

class KeyData {
   public:
      KeyData(Key key, int count) : key_(key), count_(count) {}

      Key key() const { return key_; }

      bool operator<(const KeyData &second) const
            {
                        // choose key with max sum count of transitions
            if (count_ > second.count_)
                  return true;
            else if (count_ < second.count_)
                  return false;
                        // if equal - prefer key with less accitential count
            return qAbs((int)key_) < qAbs((int)second.key_);
            }

   private:
      Key key_;
      int count_;
      };

Key findKey(const QList<MTrack> &tracks)
      {
      const int octave = 12;
      std::vector<int> counts(octave);

      for (const auto &track: tracks) {
            for (auto it = track.chords.begin(); it != track.chords.end(); ++it) {
                  const auto next = std::next(it);
                  if (next == track.chords.end())
                        continue;

                  for (const auto &note1: it->second.notes) {
                        for (const auto &note2: next->second.notes) {
                              if (qAbs(note1.pitch - note2.pitch) == 1)
                                    ++counts[qMin(note1.pitch, note2.pitch) % octave];
                              }
                        }
                  }
            }

      std::vector<KeyData> keys = {
              {Key::C_B, counts[3] + counts[10]}
            , {Key::G_B, counts[10] + counts[5]}
            , {Key::D_B, counts[5] + counts[0]}
            , {Key::A_B, counts[0] + counts[7]}
            , {Key::E_B, counts[7] + counts[2]}
            , {Key::B_B, counts[2] + counts[9]}
            , {Key::F, counts[9] + counts[4]}
            , {Key::C, counts[4] + counts[11]}
            , {Key::G, counts[11] + counts[6]}
            , {Key::D, counts[6] + counts[1]}
            , {Key::A, counts[1] + counts[8]}
            , {Key::E, counts[8] + counts[3]}
            , {Key::B, counts[3] + counts[10]}
            , {Key::F_S, counts[10] + counts[5]}
            , {Key::C_S, counts[5] + counts[0]}
            };

      std::sort(keys.begin(), keys.end());

      return keys[0].key();
      }

void setMainKeySig(QList<MTrack> &tracks)
      {
      const Key key = findKey(tracks);
      for (MTrack &track: tracks) {
            auto &opers = preferences.midiImportOperations;
            MidiOperations::CurrentTrackSetter setCurrentTrack{opers, track.indexOfOperation};
            if (!track.hasKey || opers.data()->trackOpers.isHumanPerformance.value())
                  track.staff->setKey(0, key);
            }
      }

} // namespace MidiKey
} // namespace Ms
