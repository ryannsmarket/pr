#include "importmidi_meter.h"
#include "libmscore/fraction.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "importmidi_tupletdata.h"

#include <memory>


namespace Ms {
namespace Meter {

            // max level for tuplets: duration cannot go over the tuplet boundary
            // this level should be greater than any other level
const int TUPLET_BOUNDARY_LEVEL = 10;

struct MaxLevel
      {
      int level = 0;         // 0 - the biggest, whole bar level; other: -1, -2, ...
      int levelCount = 0;    // number of ticks with 'level' value
      int lastPos = -1;      // position of last tick with value 'level'; -1 - undefined pos
      };

bool isSimple(const Fraction &barFraction)       // 2/2, 3/4, 4/4, ...
      {
      return barFraction.numerator() < 5;
      }

bool isCompound(const Fraction &barFraction)     // 6/8, 12/4, ...
      {
      return barFraction.numerator() % 3 == 0 && barFraction.numerator() > 3;
      }

bool isComplex(const Fraction &barFraction)      // 5/4, 7/8, ...
      {
      return barFraction.numerator() == 5 || barFraction.numerator() == 7;
      }

bool isDuple(const Fraction &barFraction)        // 2/2, 6/8, ...
      {
      return barFraction.numerator() == 2 || barFraction.numerator() == 6;
      }

bool isTriple(const Fraction &barFraction)       // 3/4, 9/4, ...
      {
      return barFraction.numerator() == 3 || barFraction.numerator() == 9;
      }

bool isQuadruple(const Fraction &barFraction)    // 4/4, 12/8, ...
      {
      return barFraction.numerator() % 4 == 0;
      }

int minAllowedDuration()
      {
      return MScore::division / 32;       // smallest allowed duration is 1/128
      }

struct DivLengthInfo
      {
      int len;
      int level;
      };

struct DivisionInfo
      {
      int onTime = 0;         // division start tick (tick is counted from the beginning of bar)
      int len = 0;            // length of this whole division
      bool isTuplet = false;
      std::vector<DivLengthInfo> divLengths;    // lengths of 'len' subdivisions
      };


// list of bar division lengths in ticks (whole bar len, half bar len, ...)
// and its corresponding levels

DivisionInfo metricDivisionsOfBar(const Fraction &barFraction)
      {
      int barLen = barFraction.ticks();
      DivisionInfo barDivInfo;
      barDivInfo.onTime = 0;
      barDivInfo.len = barLen;
                  // first value of each element in list is a length (in ticks) of every part of bar
                  // on which bar is subdivided on each level
                  // the level value is a second value of each element
      auto &divLengths = barDivInfo.divLengths;
      int level = 0;
      divLengths.push_back({barLen, level});
                  // pulse-level division
      if (Meter::isDuple(barFraction))
            divLengths.push_back({barLen / 2, --level});
      else if (Meter::isTriple(barFraction))
            divLengths.push_back({barLen / 3, --level});
      else if (Meter::isQuadruple(barFraction)) {
            divLengths.push_back({barLen / 2, --level});    // additional central accent
            divLengths.push_back({barLen / 4, --level});
            }
      else {
                        // if complex meter - not a complete solution: pos of central accent is unknown
            divLengths.push_back({barLen / barFraction.numerator(), --level});
            }

      if (Meter::isCompound(barFraction)) {
            --level;    // additional min level for pulse divisions
                        // subdivide pulse of compound meter into 3 parts
            divLengths.push_back({divLengths.back().len / 3, --level});
            }

      while (divLengths.back().len >= 2 * minAllowedDuration())
            divLengths.push_back({divLengths.back().len / 2, --level});

      return barDivInfo;
      }

DivisionInfo metricDivisionsOfTuplet(const TupletData &tuplet,
                                     int startLevel)
      {
      DivisionInfo tupletDivInfo;
      tupletDivInfo.onTime = tuplet.onTime;
      tupletDivInfo.len = tuplet.len;
      tupletDivInfo.isTuplet = true;
      int divLen = tuplet.len / tuplet.tupletNumber;
      tupletDivInfo.divLengths.push_back({divLen, TUPLET_BOUNDARY_LEVEL});
      while (tupletDivInfo.divLengths.back().len >= 2 * minAllowedDuration())
            tupletDivInfo.divLengths.push_back({
                  tupletDivInfo.divLengths.back().len / 2, --startLevel
            });
      return tupletDivInfo;
      }

int beatLength(const Fraction &barFraction)
      {
      int barLen = barFraction.ticks();
      int beatLen = barLen / 4;
      if (Meter::isDuple(barFraction))
            beatLen = barLen / 2;
      else if (Meter::isTriple(barFraction))
            beatLen = barLen / 3;
      else if (Meter::isQuadruple(barFraction))
            beatLen = barLen / 4;
      else if (Meter::isComplex(barFraction))
            beatLen = barLen / barFraction.numerator();
      return beatLen;
      }

std::vector<int> divisionsOfBarForTuplets(const Fraction &barFraction)
      {
      DivisionInfo info = metricDivisionsOfBar(barFraction);
      std::vector<int> divLengths;
      int beatLen = beatLength(barFraction);
      for (const auto &i: info.divLengths) {
                        // in compound meter tuplet starts from beat level, not the whole bar
            if (Meter::isCompound(barFraction) && i.len > beatLen)
                  continue;
            divLengths.push_back(i.len);
            }
      return divLengths;
      }

// result in vector: first - all tuplets info, at the end - one bar division info

std::vector<DivisionInfo> divisionInfo(const Fraction &barFraction,
                                       const std::vector<TupletData> &tupletsInBar)
      {
      std::vector<DivisionInfo> divsInfo;

      auto barDivisionInfo = metricDivisionsOfBar(barFraction);
      for (const auto &tuplet: tupletsInBar) {
            int tupletStartLevel = 0;
            for (const auto &divLenInfo: barDivisionInfo.divLengths) {
                  if (divLenInfo.len == tuplet.len) {
                        tupletStartLevel = divLenInfo.level;
                        break;
                        }
                  }
            divsInfo.push_back(metricDivisionsOfTuplet(tuplet, tupletStartLevel));
            }
      divsInfo.push_back(barDivisionInfo);

      return divsInfo;
      }

// tick is counted from the beginning of bar

int levelOfTick(int tick, const std::vector<DivisionInfo> &divsInfo)
      {
      for (const auto &divInfo: divsInfo) {
            if (tick < divInfo.onTime || tick > divInfo.onTime + divInfo.len)
                  continue;
            for (const auto &divLenInfo: divInfo.divLengths) {
                  if ((tick - divInfo.onTime) % divLenInfo.len == 0)
                        return divLenInfo.level;
                  }
            }
      return 0;
      }

Meter::MaxLevel maxLevelBetween(int startTickInDivision,
                                int endTickInDivision,
                                const DivisionInfo &divInfo)
      {
      Meter::MaxLevel level;
      for (const auto &divLengthInfo: divInfo.divLengths) {
            int divLen = divLengthInfo.len;
            int maxEndRaster = (endTickInDivision / divLen) * divLen;
            if (maxEndRaster == endTickInDivision)
                  maxEndRaster -= divLen;
            if (startTickInDivision < maxEndRaster) {
                              // max level is found
                  level.lastPos = maxEndRaster;
                  int maxStartRaster = (startTickInDivision / divLen) * divLen;
                  level.levelCount = (maxEndRaster - maxStartRaster) / divLen;
                  level.level = divLengthInfo.level;
                  break;
                  }
            }
      return level;
      }

Meter::MaxLevel maxLevelBetween(int startTickInBar,
                                int endTickInBar,
                                const std::vector<DivisionInfo> &divsInfo)
      {
      Meter::MaxLevel level;

      for (const auto &divInfo: divsInfo) {
            if (divInfo.isTuplet) {
                  if (startTickInBar < divInfo.onTime
                              && endTickInBar >= divInfo.onTime + divInfo.len) {
                        level.level = TUPLET_BOUNDARY_LEVEL;
                        level.levelCount = 1;
                        level.lastPos = divInfo.onTime;
                        break;
                        }
                  if (startTickInBar == divInfo.onTime
                              && endTickInBar > divInfo.onTime + divInfo.len) {
                        level.level = TUPLET_BOUNDARY_LEVEL;
                        level.levelCount = 1;
                        level.lastPos = divInfo.onTime + divInfo.len;
                        break;
                        }
                  if (startTickInBar == divInfo.onTime
                              && endTickInBar == divInfo.onTime + divInfo.len) {
                        level = maxLevelBetween(startTickInBar - divInfo.onTime,
                                                endTickInBar - divInfo.onTime,
                                                divInfo);
                        break;
                        }
                  }
            else
                  level = maxLevelBetween(startTickInBar, endTickInBar, divInfo);
            }
      return level;
      }

bool isPowerOfTwo(unsigned int x)
      {
      return x && !(x & (x - 1));
      }

bool isSingleNoteDuration(int ticks)
      {
      int div = (ticks > MScore::division)
                  ? ticks / MScore::division
                  : MScore::division / ticks;
      if (div > 0)
            return isPowerOfTwo((unsigned int)div);
      return false;
      }

bool isQuarterDuration(int ticks)
      {
      Fraction f = Fraction::fromTicks(ticks);
      return (f.numerator() == 1 && f.denominator() == 4);
      }

// If last 2/3 of beat in compound meter is rest,
// it should be splitted into 2 rests

bool is23EndOfBeatInCompoundMeter(int startTickInBar,
                                  int endTickInBar,
                                  const Fraction &barFraction)
      {
      if (endTickInBar - startTickInBar <= 0)
            return false;
      if (!isCompound(barFraction))
            return false;

      int beatLen = beatLength(barFraction);
      int divLen = beatLen / 3;

      if ((startTickInBar - (startTickInBar / beatLen) * beatLen == divLen)
                  && (endTickInBar % beatLen == 0))
            return true;
      return false;
      }

bool isHalfDuration(int ticks)
      {
      Fraction f = Fraction::fromTicks(ticks);
      return (f.numerator() == 1 && f.denominator() == 2);
      }

// 3/4: if half rest starts from beg of bar or ends on bar end
// then it is a bad practice - need to split rest into 2 quarter rests

bool isHalfRestOn34(int startTickInBar,
                    int endTickInBar,
                    const Fraction &barFraction)
      {
      if (endTickInBar - startTickInBar <= 0)
            return false;
      if (barFraction.numerator() == 3 && barFraction.denominator() == 4
                  && (startTickInBar == 0 || endTickInBar == barFraction.ticks())
                  && isHalfDuration(endTickInBar - startTickInBar))
            return true;
      return false;
      }


// Node for binary tree of durations

struct Node
      {
      Node(int startTick, int endTick, int startLevel, int endLevel)
            : startTick(startTick), endTick(endTick)
            , startLevel(startLevel), endLevel(endLevel)
            , parent(nullptr)
            {}

      int startTick;
      int endTick;
      int startLevel;
      int endLevel;

      Node *parent;
      std::unique_ptr<Node> left;
      std::unique_ptr<Node> right;
      };

void treeToDurationList(Node *node,
                        QList<TDuration> &dl,
                        bool useDots)
      {
      if (node->left != nullptr && node->right != nullptr) {
            treeToDurationList(node->left.get(), dl, useDots);
            treeToDurationList(node->right.get(), dl, useDots);
            }
      else {
            const int MAX_DOTS = 1;
            dl.append(toDurationList(
                  Fraction::fromTicks(node->endTick - node->startTick), useDots, MAX_DOTS));
            }
      }

// duration start/end should be quantisized at least to 1/128 note

QList<TDuration> toDurationList(int startTickInBar,
                                int endTickInBar,
                                const Fraction &barFraction,
                                const std::vector<TupletData> &tupletsInBar,
                                DurationType durationType,
                                bool useDots)
      {
      QList<TDuration> durations;
      if (startTickInBar < 0 || endTickInBar <= startTickInBar
                  || endTickInBar > barFraction.ticks())
            return durations;
                  // analyse mectric structure of bar
      auto divInfo = divisionInfo(barFraction, tupletsInBar);
                  // create a root for binary tree of durationsstd::multimap<int, MidiChord>
      std::unique_ptr<Node> root(new Node(startTickInBar, endTickInBar,
                                          levelOfTick(startTickInBar, divInfo),
                                          levelOfTick(endTickInBar, divInfo)));
      QQueue<Node *> nodesToProcess;
      nodesToProcess.enqueue(root.get());
                  // max allowed difference between start/end level of duration and split point level
      int tol = 0;
      if (durationType == DurationType::NOTE)
            tol = 1;
      else if (durationType == DurationType::REST)
            tol = 0;
                  // each child node duration after division is not less than minDuration()
      const int minDuration = minAllowedDuration() * 2;
                  // build duration tree such that durations don't go across strong beat divisions
      while (!nodesToProcess.isEmpty()) {
            Node *node = nodesToProcess.dequeue();
                        // don't split node if its duration is less than minDuration
            if (node->endTick - node->startTick < minDuration)
                  continue;
            auto splitPoint = maxLevelBetween(node->startTick, node->endTick, divInfo);
                        // sum levels if there are several positions (beats) with max level value
                        // for example, 8th + half duration + 8th in 3/4, and half is over two beats
            int effectiveLevel = splitPoint.level + splitPoint.levelCount - 1;
            if (effectiveLevel - node->startLevel > tol
                        || effectiveLevel - node->endLevel > tol
                        || isHalfRestOn34(node->startTick, node->endTick, barFraction)
                        || (durationType == DurationType::REST
                            && is23EndOfBeatInCompoundMeter(node->startTick, node->endTick, barFraction))
                        )
                  {
                              // split duration in splitPoint position
                  node->left.reset(new Node(node->startTick, splitPoint.lastPos,
                                            node->startLevel, splitPoint.level));
                  node->left->parent = node;
                  nodesToProcess.enqueue(node->left.get());

                  node->right.reset(new Node(splitPoint.lastPos, node->endTick,
                                             splitPoint.level, node->endLevel));
                  node->right->parent = node;
                  nodesToProcess.enqueue(node->right.get());
                  }
            }
                  // collect the resulting durations
      treeToDurationList(root.get(), durations, useDots);
      return durations;
      }

} // namespace Meter
} // namespace Ms
