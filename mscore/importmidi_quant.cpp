#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "libmscore/utils.h"
#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_tupletdata.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace Quantize {

void applyAdaptiveQuant(std::multimap<int, MidiChord> &/*chords*/,
                        const TimeSigMap */*sigmap*/,
                        int /*allTicks*/)
      {
      }


int shortestNoteInBar(const std::multimap<int, MidiChord>::const_iterator &startBarChordIt,
                      const std::multimap<int, MidiChord>::const_iterator &endChordIt,
                      int endBarTick)
      {
      int division = MScore::division;
      int minDuration = division;
                  // find shortest note in measure
      for (auto it = startBarChordIt; it != endChordIt; ++it) {
            if (it->first >= endBarTick)
                  break;
            for (const auto &note: it->second.notes)
                  minDuration = qMin(minDuration, note.len);
            }
                  // determine suitable quantization value based
                  // on shortest note in measure
      int div = division;
      if (minDuration <= division / 16)        // minimum duration is 1/64
            div = division / 16;
      else if (minDuration <= division / 8)
            div = division / 8;
      else if (minDuration <= division / 4)
            div = division / 4;
      else if (minDuration <= division / 2)
            div = division / 2;
      else if (minDuration <= division)
            div = division;
      else if (minDuration <= division * 2)
            div = division * 2;
      else if (minDuration <= division * 4)
            div = division * 4;
      else if (minDuration <= division * 8)
            div = division * 8;
      if (div == (division / 16))
            minDuration = div;
      else
            minDuration = quantizeLen(minDuration, div >> 1);    //closest

      return minDuration;
      }

int userQuantNoteToTicks(MidiOperation::QuantValue quantNote)
      {
      int division = MScore::division;
      int userQuantValue = preferences.shortestNote;
                  // specified quantization value
      switch (quantNote) {
            case MidiOperation::QuantValue::N_4:
                  userQuantValue = division;
                  break;
            case MidiOperation::QuantValue::N_8:
                  userQuantValue = division / 2;
                  break;
            case MidiOperation::QuantValue::N_16:
                  userQuantValue = division / 4;
                  break;
            case MidiOperation::QuantValue::N_32:
                  userQuantValue = division / 8;
                  break;
            case MidiOperation::QuantValue::N_64:
                  userQuantValue = division / 16;
                  break;
            case MidiOperation::QuantValue::FROM_PREFERENCES:
            default:
                  userQuantValue = preferences.shortestNote;
                  break;
            }

      return userQuantValue;
      }

int findQuantRaster(const std::multimap<int, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<int, MidiChord>::iterator &endChordIt,
                    int endBarTick)
      {
      int raster;
      auto operations = preferences.midiImportOperations.currentTrackOperations();
                  // find raster value for quantization
      if (operations.quantize.value == MidiOperation::QuantValue::SHORTEST_IN_BAR)
            raster = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
      else {
            int userQuantValue = userQuantNoteToTicks(operations.quantize.value);
                        // if user value larger than the smallest note in bar
                        // then use the smallest note to keep faster events
            if (operations.quantize.reduceToShorterNotesInBar) {
                  raster = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
                  raster = qMin(userQuantValue, raster);
                  }
            else
                  raster = userQuantValue;
            }
      return raster;
      }

// chords onTime values don't repeat despite multimap

void doGridQuantizationOfBar(std::multimap<int, MidiChord> &quantizedChords,
                             const std::multimap<int, MidiChord>::iterator &startChordIt,
                             const std::multimap<int, MidiChord>::iterator &endChordIt,
                             int raster,
                             int endBarTick)
      {
      int raster2 = raster >> 1;
      for (auto it = startChordIt; it != endChordIt; ++it) {
            if (it->first >= endBarTick)
                  break;
            if (quantizedChords.find(it->first) != quantizedChords.end())
                  continue;
            auto chord = it->second;
            chord.onTime = ((chord.onTime + raster2) / raster) * raster;
            for (auto &note: chord.notes) {
                  note.onTime = chord.onTime;
                  note.len = quantizeLen(note.len, raster);
                  }
            quantizedChords.insert({chord.onTime, chord});
            }
      }

std::multimap<int, MidiChord>::iterator
findFirstChordInRange(int startRangeTick,
                      int endRangeTick,
                      const std::multimap<int, MidiChord>::iterator &startChordIt,
                      const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
      auto it = startChordIt;
      for (; it != endChordIt; ++it) {
            if (it->first >= startRangeTick) {
                  if (it->first >= endRangeTick)
                        it = endChordIt;
                  break;
                  }
            }
      return it;
      }

std::multimap<int, MidiChord>::iterator
findEndChordInRange(int endRangeTick,
                    const std::multimap<int, MidiChord>::iterator &startChordIt,
                    const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
      auto it = startChordIt;
      for (; it != endChordIt; ++it) {
            if (it->first > endRangeTick)
                  break;
            }
      return it;
      }

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    std::multimap<int, MidiChord> &quantizedChords,
                    int lastTick,
                    const TimeSigMap* sigmap)
      {
      int startBarTick = 0;
      auto startBarChordIt = chords.begin();
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            startBarChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                                    startBarChordIt, chords.end());
            if (startBarChordIt != chords.end()) {     // if chords are found in this bar
                  int raster = findQuantRaster(startBarChordIt, chords.end(), endBarTick);
                  doGridQuantizationOfBar(quantizedChords, startBarChordIt, chords.end(),
                                          raster, endBarTick);
                  }
            else
                  startBarChordIt = chords.begin();
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      }

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    const TimeSigMap* sigmap,
                    int lastTick)
      {
      std::multimap<int, MidiChord> quantizedChords;
      applyGridQuant(chords, quantizedChords, lastTick, sigmap);
      std::swap(chords, quantizedChords);
      }

bool isTupletAllowed(int tupletNumber,
                     int tupletLen,
                     int tupletOnTimeSumError,
                     int regularSumError,
                     int quantValue,
                     const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
      {
                  // special check for duplets and triplets
      std::vector<int> nums = {2, 3};
                  // for duplet: if note first and single - only 1/2*tupletLen duration is allowed
                  // for triplet: if note first and single - only 1/3*tupletLen duration is allowed
      for (auto num: nums) {
            if (tupletNumber == num && tupletChords.size() == 1
                        && tupletChords.begin()->first == 0) {
                  auto &chordEventIt = tupletChords.begin()->second;
                  for (const auto &note: chordEventIt->second.notes) {
                        if (std::abs(note.len - tupletLen / num) > quantValue)
                              return false;
                        }
                  }
            }
                  // for all tuplets
      int minAllowedNoteCount = tupletNumber / 2 + tupletNumber / 4;
      if ((int)tupletChords.size() < minAllowedNoteCount
                  || tupletOnTimeSumError >= regularSumError) {
            return false;
            }

      int tupletNoteLen = tupletLen / tupletNumber;
      for (const auto &tupletChord: tupletChords) {
            for (const auto &note: tupletChord.second->second.notes) {
                  if (note.len >= tupletNoteLen / 2)
                        return true;
                  }
            }

      return false;
      }

std::vector<int> findTupletNumbers(int divLen, const Fraction &barFraction)
      {
      std::vector<int> tupletNumbers;
      if (Meter::isCompound(barFraction) && divLen == Meter::beatLength(barFraction))
            tupletNumbers = {2, 4};       // duplets and quadruplets
      else
            tupletNumbers = {3, 5, 7};
      return tupletNumbers;
      }

int findOnTimeRegularError(int onTime, int quantValue)
      {
      int regularPos = ((onTime + quantValue / 2) / quantValue) * quantValue;
      return std::abs(onTime - regularPos);
      }

// return: <chordIter, minChordError>

std::pair<std::multimap<int, MidiChord>::iterator, int>
findBestChordForTupletNote(int tupletNotePos,
                           int quantValue,
                           const std::multimap<int, MidiChord>::iterator &startChordIt,
                           const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
                  // choose the best chord, if any, for this tuplet note
      std::pair<std::multimap<int, MidiChord>::iterator, int> bestChord;
      bestChord.first = endChordIt;
      bestChord.second = std::numeric_limits<int>::max();
                  // check chords - whether they can be in tuplet without large error
      for (auto chordIt = startChordIt; chordIt != endChordIt; ++chordIt) {
            int tupletError = std::abs(chordIt->first - tupletNotePos);
            if (tupletError > quantValue)
                  continue;
            if (tupletError < bestChord.second) {
                  bestChord.first = chordIt;
                  bestChord.second = tupletError;
                  }
            }
      return bestChord;
      }

TupletInfo findTupletApproximation(int tupletNumber,
                                   int tupletNoteLen,
                                   int quantValue,
                                   int startTupletTime,
                                   const std::multimap<int, MidiChord>::iterator &startChordIt,
                                   const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;
      tupletInfo.onTime = startTupletTime;
      tupletInfo.len = tupletNoteLen * tupletNumber;
      tupletInfo.tupletQuantValue = tupletNoteLen;
      while (tupletInfo.tupletQuantValue / 2 >= quantValue)
            tupletInfo.tupletQuantValue /= 2;
      tupletInfo.regularQuantValue = quantValue;

      auto startTupletChordIt = startChordIt;
      for (int k = 0; k != tupletNumber; ++k) {
            int tupletNotePos = startTupletTime + k * tupletNoteLen;
                        // choose the best chord, if any, for this tuplet note
            auto bestChord = findBestChordForTupletNote(tupletNotePos, quantValue,
                                                        startTupletChordIt, endChordIt);
            if (bestChord.first == endChordIt)
                  continue;   // no chord fits to this tuplet note position
                        // chord can be in tuplet
            tupletInfo.chords.insert({k, bestChord.first});
            tupletInfo.tupletOnTimeSumError += bestChord.second;
                        // for next tuplet note we start from the next chord
                        // because chord for the next tuplet note cannot be earlier
            startTupletChordIt = bestChord.first;
            ++startTupletChordIt;
                        // find chord quant error for a regular grid
            int regularError = findOnTimeRegularError(bestChord.first->first, quantValue);
            tupletInfo.regularSumError += regularError;
            }

      return tupletInfo;
      }

std::multimap<double, TupletInfo>
findTupletCandidatesOfBar(int startBarTick,
                          int endBarTick,
                          const Fraction &barFraction,
                          std::multimap<int, MidiChord> &chords)
      {
      std::multimap<double, TupletInfo> tupletCandidates;   // average error, TupletInfo
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return tupletCandidates;

      int barLen = barFraction.ticks();
                  // barLen / 4 - additional tolerance
      auto startBarChordIt = findFirstChordInRange(startBarTick - barLen / 4,
                                                   endBarTick,
                                                   chords.begin(),
                                                   chords.end());
      if (startBarChordIt == chords.end()) // no chords in this bar
            return tupletCandidates;
                  // end iterator, as usual, will point to the next - invalid chord
      auto endBarChordIt = findEndChordInRange(endBarTick + barLen / 4, startBarChordIt, chords.end());

      int quantValue = findQuantRaster(startBarChordIt, endBarChordIt, endBarTick);
      auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

      for (const auto &divLen: divLengths) {
            auto tupletNumbers = findTupletNumbers(divLen, barFraction);
            int divCount = barLen / divLen;

            for (int i = 0; i != divCount; ++i) {
                  int startDivTime = startBarTick + i * divLen;
                  int endDivTime = startDivTime + divLen;
                              // check which chords can be inside tuplet period
                              // [startDivTime - quantValue, endDivTime + quantValue]
                  auto startDivChordIt = findFirstChordInRange(startDivTime - quantValue,
                                                               endDivTime + quantValue,
                                                               startBarChordIt,
                                                               endBarChordIt);
                  if (startDivChordIt == endBarChordIt) // no chords in this division
                        continue;
                              // end iterator, as usual, will point to the next - invalid chord
                  auto endDivChordIt = findEndChordInRange(endDivTime + quantValue, startDivChordIt,
                                                           endBarChordIt);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        int tupletNoteLen = divLen / tupletNumber;
                        if (tupletNoteLen < quantValue)
                              continue;
                        TupletInfo tupletInfo = findTupletApproximation(tupletNumber, tupletNoteLen,
                                    quantValue, startDivTime - quantValue, startDivChordIt, endDivChordIt);
                                    // check - is it a valid tuplet approximation?
                        if (!isTupletAllowed(tupletNumber, divLen,
                                             tupletInfo.tupletOnTimeSumError,
                                             tupletInfo.regularSumError,
                                             quantValue, tupletInfo.chords))
                              continue;
                                    // tuplet found
                        double averageError = tupletInfo.tupletOnTimeSumError * 1.0 / tupletInfo.chords.size();
                        tupletCandidates.insert({averageError, tupletInfo});
                        }     // next tuplet type
                  }
            }
      return tupletCandidates;
      }

void markChordsAsUsed(std::map<int, int> &usedFirstTupletNotes,
                      std::set<int> &usedChords,
                      const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
      {
      if (tupletChords.empty())
            return;

      auto i = tupletChords.begin();
      int tupletNoteIndex = i->first;
      if (tupletNoteIndex == 0) {
                        // check is the note of the first tuplet chord in use
            int chordOnTime = i->second->first;
            auto ii = usedFirstTupletNotes.find(chordOnTime);
            if (ii == usedFirstTupletNotes.end())
                  ii = usedFirstTupletNotes.insert({chordOnTime, 1}).first;
            else
                  ++(ii->second);         // increase chord note counter
            ++i;              // start from the second chord
            }
      for ( ; i != tupletChords.end(); ++i) {
                        // mark the chord as used
            int chordOnTime = i->second->first;
            usedChords.insert(chordOnTime);
            }
      }

bool areTupletChordsInUse(const std::map<int, int> &usedFirstTupletNotes,
                          const std::set<int> &usedChords,
                          const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
      {
      if (tupletChords.empty())
            return false;

      auto i = tupletChords.begin();
      int tupletNoteIndex = i->first;
      if (tupletNoteIndex == 0) {
                        // check are first tuplet notes all in use (1 note - 1 voice)
            int chordOnTime = i->second->first;
            auto ii = usedFirstTupletNotes.find(chordOnTime);
            if (ii != usedFirstTupletNotes.end()) {
                  if (ii->second >= i->second->second.notes.size()) {
                              // need to choose next tuplet candidate - no more available voices
                        return true;
                        }
                  }
            ++i;
      }
      for ( ; i != tupletChords.end(); ++i) {
            int chordOnTime = i->second->first;
            if (usedChords.find(chordOnTime) != usedChords.end()) {
                              // the chord note is in use - cannot use this chord again
                  return true;
                  }
            }
      return false;
      }

// use case for this: first chord in tuplet can belong
// to any other tuplet at the same time
// if there are enough notes in this first chord
// to be splitted to different voices

void filterTuplets(std::multimap<double, TupletInfo> &tuplets)
      {
                  // structure of map: <tick, count of use of first tuplet chord with this tick>
      std::map<int, int> usedFirstTupletNotes;
                  // onTime values - tick - of already used chords
      std::set<int> usedChords;
                  // select tuplets with min average error
      for (auto tc = tuplets.begin(); tc != tuplets.end(); ) {  // tc - tuplet candidate
            auto &tupletChords = tc->second.chords;
                        // check for chords notes already used in another tuplets
            if (tupletChords.empty()
                        || areTupletChordsInUse(usedFirstTupletNotes, usedChords, tupletChords)) {
                  tc = tuplets.erase(tc);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(usedFirstTupletNotes, usedChords, tupletChords);
            ++tc;
            }
      }

int averagePitch(const std::map<int, std::multimap<int, MidiChord>::iterator> &chords)
      {
      int sumPitch = 0;
      int noteCounter = 0;
      for (const auto &chord: chords) {
            const auto &midiNotes = chord.second->second.notes;
            for (const auto &midiNote: midiNotes) {
                  sumPitch += midiNote.pitch;
                  ++noteCounter;
                  }
            }
      return sumPitch / noteCounter;
      }

void sortNotesByPitch(std::multimap<int, MidiChord> &chords)
      {
      struct {
            bool operator()(const MidiNote &n1, const MidiNote &n2)
                  {
                  return (n1.pitch > n2.pitch);
                  }
            } pitchComparator;

      for (auto &chordEvent: chords) {
            auto &midiNotes = chordEvent.second.notes;
            std::sort(midiNotes.begin(), midiNotes.end(), pitchComparator);
            }
      }

void sortTupletsByAveragePitch(std::vector<TupletInfo> &tuplets)
      {
      struct {
            bool operator()(const TupletInfo &t1, const TupletInfo &t2)
                  {
                  return (averagePitch(t1.chords) > averagePitch(t2.chords));
                  }
            } averagePitchComparator;
      std::sort(tuplets.begin(), tuplets.end(), averagePitchComparator);
      }

// tuplets should be filtered (for mutual validity)

void separateTupletVoices(std::vector<TupletInfo> &tuplets,
                          std::multimap<int, MidiChord> &chords)
      {
                  // it's better before to sort tuplets by their average pitch
                  // and notes of each chord as well (desc. order)
      sortNotesByPitch(chords);
      sortTupletsByAveragePitch(tuplets);

      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            int counter = 0;
            auto lastMatch = tuplets.end();
            auto firstNowChordIt = now->chords.begin();
            for (auto prev = tuplets.begin(); prev != now; ++prev) {
                              // check is now tuplet go over previous tuplets
                  if (now->onTime + now->len > prev->onTime
                              && now->onTime < prev->onTime + prev->len)
                        ++counter;
                              // if first notes in tuplets match - split this chord
                              // into 2 voices
                  auto firstPrevChordIt = prev->chords.begin();
                  if (firstNowChordIt->first == 0 && firstPrevChordIt->first == 0
                              && firstNowChordIt->second->second.onTime
                              == firstPrevChordIt->second->second.onTime) {
                        lastMatch = prev;
                        }
                  }
            if (lastMatch != tuplets.end()) {
                              // split first tuplet chord, that belong to 2 tuplets, into 2 voices
                  MidiChord &prevMidiChord = lastMatch->chords.begin()->second->second;
                  MidiChord newChord = prevMidiChord;
                              // erase all notes except the first one
                  auto beg = prevMidiChord.notes.begin();
                  prevMidiChord.notes.erase(++beg, prevMidiChord.notes.end());
                              // erase the first note
                  newChord.notes.erase(newChord.notes.begin());
                  auto newChordIt = chords.insert({newChord.onTime, newChord});
                              // update 'now' first tuplet chord
                  now->chords.begin()->second = newChordIt;
                  if (newChord.notes.isEmpty()) {
                                    // normally this should not happen at all
                        qDebug("Tuplets were not filtered correctly: same notes in different tuplets");
                        return;
                        }
                  }

            for (auto &tupletChord: now->chords) {
                  MidiChord &midiChord = tupletChord.second->second;
                  midiChord.voice = counter;
                  }
            }
      }


// TODO: separate different overlapping tuplet voices

void quantizeTupletChord(MidiChord &midiChord, int onTime, const TupletInfo &tupletInfo)
      {
      midiChord.onTime = onTime;
      midiChord.voice = tupletInfo.voice;
      for (auto &note: midiChord.notes) {
            int raster;
            if (note.onTime + note.len <= tupletInfo.onTime + tupletInfo.len) {
                              // if offTime is inside the tuplet - quant by tuplet grid
                  raster = tupletInfo.tupletQuantValue;
                  }
            else {            // if offTime is outside the tuplet - quant by regular grid
                  raster = tupletInfo.regularQuantValue;
                  }
            int offTime = ((note.onTime + note.len + raster / 2) / raster) * raster;
            note.onTime = onTime;
            note.len = offTime - onTime;
            }
      }

// input chords - sorted by onTime value,
// onTime values don't repeat even in multimap below

void quantizeChordsAndFindTuplets(std::multimap<int, TupletData> &tupletEvents,
                                  std::multimap<int, MidiChord> &chords,
                                  const TimeSigMap* sigmap,
                                  int lastTick)
      {
      std::multimap<int, MidiChord> quantizedChords;  // chords with already quantized onTime values
                  // quantize chords in tuplets
      int startBarTick = 0;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            Fraction barFraction = sigmap->timesig(startBarTick).timesig();
            auto tuplets = findTupletCandidatesOfBar(startBarTick, endBarTick, barFraction, chords);
            filterTuplets(tuplets);

            std::vector<TupletInfo> preparedTuplets;
            for (const auto &t: tuplets)
                  preparedTuplets.push_back(t.second);
            separateTupletVoices(preparedTuplets, chords);

            for (auto &tuplet: tuplets) {
                  TupletInfo &tupletInfo = tuplet.second;
                  auto &infoChords = tupletInfo.chords;
                  for (auto &tupletChord: infoChords) {
                        int tupletNoteNum = tupletChord.first;
                        int onTime = tupletInfo.onTime + tupletNoteNum
                                    * (tupletInfo.len / tupletInfo.tupletNumber);
                        std::multimap<int, MidiChord>::iterator &midiChordEventIt = tupletChord.second;
                                    // quantize chord to onTime value
                        MidiChord midiChord = midiChordEventIt->second;
                        quantizeTupletChord(midiChord, onTime, tupletInfo);
                        quantizedChords.insert({onTime, midiChord});
                        }
                  TupletData tupletData = {tupletInfo.voice, tupletInfo.onTime,
                                           tupletInfo.len, tupletInfo.tupletNumber};
                  tupletEvents.insert({tupletInfo.onTime, tupletData});
                  }
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
                  // quantize non-tuplet (remaining) chords with ordinary grid
      applyGridQuant(chords, quantizedChords, lastTick, sigmap);

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
