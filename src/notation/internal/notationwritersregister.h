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

#ifndef MU_NOTATION_NOTATIONWRITERSREGISTER_H
#define MU_NOTATION_NOTATIONWRITERSREGISTER_H

#include "../inotationwritersregister.h"

namespace mu::notation {
class NotationWritersRegister : public INotationWritersRegister
{
public:
    void reg(const std::vector<std::string>& suffixes, INotationWriterPtr writer) override;
    INotationWriterPtr writer(const std::string& suffix) const override;

    std::vector<std::string> registeredSuffixes() const override;

private:
    std::map<std::string, INotationWriterPtr> m_writers;
};
}

#endif // MU_NOTATION_NOTATIONWRITERSREGISTER_H
