#ifndef INNER_FUNC_DECL_H
#define INNER_FUNC_DECL_H


namespace Ms {

class MidiChord;
class ReducedFraction;

namespace MidiTuplet {

std::pair<std::multimap<ReducedFraction, MidiChord>::iterator, ReducedFraction>
findBestChordForTupletNote(const ReducedFraction &tupletNotePos,
                           const ReducedFraction &quantValue,
                           const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                           const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt);

bool isTupletAllowed(int tupletNumber,
                     const ReducedFraction &tupletLen,
                     const ReducedFraction &tupletOnTimeSumError,
                     const ReducedFraction &regularSumError,
                     const ReducedFraction &quantValue,
                     const std::map<int, std::multimap<ReducedFraction, MidiChord>::iterator> &tupletChords);

std::vector<int> findTupletNumbers(const ReducedFraction &divLen, const ReducedFraction &barFraction);

ReducedFraction findQuantizationError(const ReducedFraction &onTime, const ReducedFraction &quantValue);

struct TupletInfo;

TupletInfo findTupletApproximation(const ReducedFraction &tupletLen,
                                   int tupletNumber,
                                   const ReducedFraction &quantValue,
                                   const ReducedFraction &startTupletTime,
                                   const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                                   const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt);

int separateTupletVoices(std::vector<TupletInfo> &tuplets,
                         const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                         const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
                         std::multimap<ReducedFraction, MidiChord> &chords,
                         const ReducedFraction &endBarTick);

std::list<int> findTupletsWithCommonChords(std::list<int> &restTuplets,
                                           const std::vector<TupletInfo> &tuplets);

std::vector<int> findTupletsWithNoCommonChords(std::list<int> &commonTuplets,
                                               const std::vector<TupletInfo> &tuplets);

} // namespace MidiTuplet

namespace Meter {

struct MaxLevel;
struct DivisionInfo;

Meter::MaxLevel maxLevelBetween(const ReducedFraction &startTickInBar,
                                const ReducedFraction &endTickInBar,
                                const DivisionInfo &divInfo);

Meter::MaxLevel findMaxLevelBetween(const ReducedFraction &startTickInBar,
                                    const ReducedFraction &endTickInBar,
                                    const std::vector<DivisionInfo> &divsInfo);

} // namespace Meter

} // namespace Ms

#endif // INNER_FUNC_DECL_H
