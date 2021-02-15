//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_ZERBERUS_CONTROLLERS_H
#define MU_ZERBERUS_CONTROLLERS_H

namespace mu::zerberus {
enum {
    CTRL_VOLUME             = 0x07,
    CTRL_PANPOT             = 0x0a,
    CTRL_EXPRESSION         = 0x0b,
    CTRL_SUSTAIN            = 0x40,

    CTRL_ALL_NOTES_OFF      = 0x7b,

    // special midi events are mapped to internal
    // controller
    //
    CTRL_PROGRAM   = 0x81,
};
}

#endif // MU_ZERBERUS_CONTROLLERS_H
