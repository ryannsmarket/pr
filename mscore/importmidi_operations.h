#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

#include "importmidi_operation.h"
#include "midi/midifile.h"


namespace Ms {

class ReducedFraction;

namespace MidiCharset {
      QString defaultCharset();
}

namespace MidiOperations {

// operation types are in importmidi_operation.h

// to add an operation one need to add code also to:
//   - importmidi_operation.h,
//   - importmidi_opmodel.cpp (2 places),
//   - importmidi_trmodel.cpp (2 places),
// and - other importmidi files where algorithm requires it

template<typename T>
class TrackOp
      {
   public:
      TrackOp(T defaultValue)
            : _operation{{-1, defaultValue}}
            {}

      T value(int trackIndex) const
            {
            const auto it = _operation.find(trackIndex);
            if (it == _operation.end())
                  return _operation.find(-1)->second;
            return it->second;
            }

      void setValue(int trackIndex, T value)
            {
            Q_ASSERT_X(trackIndex >= 0, "TrackOperation", "Invalid track index");

            if (value != this->value(trackIndex))
                  _operation[trackIndex] = value;
            }

      T defaultValue() const
            {
            return _operation.find(-1)->second;
            }

      void setDefaultValue(T value)
            {
            _operation[-1] = value;
            }
   private:
                  // <track index, operation value>
                  // if track index == -1 then it's default value (for all tracks)
      std::map<int, T> _operation;
      };

// values that can be changed

struct Opers
      {
                  // data that cannot be changed by the user
      TrackOp<std::string> staffName = std::string();       // will be converted to unicode later
      TrackOp<QString> instrumentName = QString();
      TrackOp<bool> isDrumTrack = false;

                  // operations for all tracks
      bool isHumanPerformance = false;
      bool searchPickupMeasure = true;
      TimeSigNumerator timeSigNumerator = TimeSigNumerator::_4;
      TimeSigDenominator timeSigDenominator = TimeSigDenominator::_4;

                  // operations for individual tracks
      TrackOp<int> trackIndexAfterShuffle = 0;
      TrackOp<bool> doImport = true;
      TrackOp<QuantValue> quantValue = QuantValue::FROM_PREFERENCES;
      TrackOp<bool> searchTuplets = true;
      TrackOp<bool> search2plets = false;
      TrackOp<bool> search3plets = true;
      TrackOp<bool> search4plets = true;
      TrackOp<bool> search5plets = true;
      TrackOp<bool> search7plets = true;
      TrackOp<bool> search9plets = true;
      TrackOp<bool> useDots = true;
      TrackOp<bool> simplifyDurations = true;   // for drum tracks - remove rests and ties
      TrackOp<bool> showStaccato = true;
      TrackOp<bool> doStaffSplit = false;       // for drum tracks - split by voices
      TrackOp<StaffSplitMethod> staffSplitMethod = StaffSplitMethod::HAND_WIDTH;
      TrackOp<StaffSplitOctave> staffSplitOctave = StaffSplitOctave::C4;
      TrackOp<StaffSplitNote> staffSplitNote = StaffSplitNote::E;
      TrackOp<VoiceCount> maxVoiceCount = VoiceCount::V_4;
      TrackOp<bool> changeClef = true;
      TrackOp<Swing> swing = Swing::NONE;
      TrackOp<bool> removeDrumRests = true;
      TrackOp<int> lyricTrackIndex = -1;        // empty lyric
      };

struct HumanBeatData
      {
      std::set<ReducedFraction> beatSet;
                // to adapt human beats to a different time sig, if necessary
      int addedFirstBeats;
      int addedLastBeats;
      ReducedFraction firstChordTick;
      ReducedFraction lastChordTick;
      ReducedFraction timeSig;
      };

struct FileData
      {
      MidiFile midiFile;
      int processingsOfOpenedFile = 0;
      bool canRedefineDefaultsLater = true;
      QByteArray HHeaderData;
      QByteArray VHeaderData;
      int trackCount = 0;
      MidiOperations::Opers trackOpers;
      QString charset = MidiCharset::defaultCharset();
                  // after the user apply MIDI import operations
                  // this value should be set to false
                  // tracks of <tick, lyric fragment> from karaoke files
                  // QList of lyric tracks - there can be multiple lyric tracks,
                  // lyric track count != MIDI track count in general
      QList<std::multimap<ReducedFraction, std::string>> lyricTracks;
      HumanBeatData humanBeatData;
      };

class Data
      {
   public:
      FileData* data();
      const FileData* data() const;

      void addNewFile(const QString &fileName);
      int currentTrack() const;
      void setMidiFileData(const QString &fileName, const MidiFile &midiFile);
      void excludeFile(const QString &fileName);
      bool hasFile(const QString &fileName);
      const MidiFile* midiFile(const QString &fileName);

   private:
      friend class CurrentTrackSetter;
      friend class CurrentMidiFileSetter;

      QString _currentMidiFile;
      int _currentTrack = -1;

      std::map<QString, FileData> _data;    // <file name, tracks data>
      };

// scoped setter of current track
class CurrentTrackSetter
      {
   public:
      CurrentTrackSetter(Data &opers, int track)
            : _opers(opers)
            {
            _oldValue = _opers._currentTrack;
            _opers._currentTrack = track;
            }

      ~CurrentTrackSetter()
            {
            _opers._currentTrack = _oldValue;
            }
   private:
      Data &_opers;
      int _oldValue;
                  // disallow heap allocation - for stack-only usage
      void* operator new(size_t);               // standard new
      void* operator new(size_t, void*);        // placement new
      void* operator new[](size_t);             // array new
      void* operator new[](size_t, void*);      // placement array new
      };

// scoped setter of current MIDI file
class CurrentMidiFileSetter
      {
public:
      CurrentMidiFileSetter(Data &opers, const QString &fileName)
            : _opers(opers)
            {
            _oldValue = _opers._currentMidiFile;
            _opers._currentMidiFile = fileName;
            }

      ~CurrentMidiFileSetter()
            {
            _opers._currentMidiFile = _oldValue;
            }
private:
      Data &_opers;
      QString _oldValue;
                  // disallow heap allocation - for stack-only usage
      void* operator new(size_t);               // standard new
      void* operator new(size_t, void*);        // placement new
      void* operator new[](size_t);             // array new
      void* operator new[](size_t, void*);      // placement array new
      };

} // namespace MidiOperations
} // namespace Ms


#endif // IMPORTMIDI_OPERATIONS_H
