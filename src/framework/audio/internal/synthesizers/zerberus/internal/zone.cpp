/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "zone.h"

#include <stdio.h>
#include <cmath>

#include "channel.h"
#include "sample.h"

using namespace mu::zerberus;

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

Zone::Zone()
{
    for (int i = 0; i < 128; ++i) {
        onLocc[i] = -1;
        onHicc[i] = -1;
        locc[i]    = 0;
        hicc[i]    = 127;
    }
}

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

Zone::~Zone()
{
    delete sample;
}

//---------------------------------------------------------
//   match
//---------------------------------------------------------

bool Zone::match(Channel* c, int k, int v, Trigger et, double rand, int cc, int ccVal)
{
    if ((k >= keyLo || et == Trigger::CC)
        && (k <= keyHi || et == Trigger::CC)
        && (v >= veloLo || et == Trigger::CC)
        && (v <= veloHi || et == Trigger::CC)
        && (loRand <= rand && hiRand > rand)
        && (et == trigger)
        ) {
//printf("   Zone match %d %d %d -- %d %d  %d %d  center %d trigger %d\n",
//         k, v, et, keyLo, keyHi, veloLo, veloHi, keyBase, trigger);
        if (useCC) {
            for (int i = 0; i < 128; i++) {
                if (locc[i] == 0 && hicc[i] == 127) {
                    continue;
                }
                if (locc[i] > c->getCtrl(i) || hicc[i] < c->getCtrl(i)) {
                    return false;
                }
            }
        }

        int oldSeq = seq;

        if (trigger == Trigger::CC) {
            if (onLocc[cc] <= ccVal && onHicc[cc] >= ccVal) {
                seq++;
                if (seq > seqLen) {
                    seq = 0;
                }
                return oldSeq == seqPos;
            } else {
                return false;
            }
        }

        ++seq;
        if (seq > seqLen) {
            seq = 0;
        }
        return oldSeq == seqPos;
    }
    return false;
}

//---------------------------------------------------------
//   updateCCGain
//---------------------------------------------------------

void Zone::updateCCGain(Channel* c)
{
    ccGain = 1.0;
    for (auto oncc : gainOnCC) {
        ccGain *= pow(10, (((float)c->getCtrl(oncc.first) / (float)127.0) * oncc.second) / 20);
    }
}
