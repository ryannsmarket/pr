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
#ifndef MU_NOTATION_NOTATIONINPUTSTATE_H
#define MU_NOTATION_NOTATIONINPUTSTATE_H

#include "../inotationinputstate.h"
#include "async/asyncable.h"
#include "igetscore.h"
#include "inotationinteraction.h"
#include "inotationundostack.h"

namespace Ms {
class Score;
}

namespace mu::notation {
class ScoreCallbacks;
class NotationInputState : public INotationInputState, public async::Asyncable
{
public:
    NotationInputState(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack);
    ~NotationInputState() override;

    bool isNoteInputMode() const override;
    bool isPadActive(Pad pad) const override;

    Duration duration() const override;

    void startNoteInput() override;
    void endNoteInput() override;
    void setNoteInputMethod(NoteInputMethod method) override;
    void addNote(NoteName noteName, NoteAddingMode addingMode) override;
    void padNote(const Pad& pad) override;
    void putNote(const QPointF& pos, bool replace, bool insert) override;

    async::Notification noteAdded() const override;
    async::Notification stateChanged() const override;

private:
    Ms::Score* score() const;

    bool isDurationActive(DurationType durationType) const;

    void updateInputState();

    const IGetScore* m_getScore = nullptr;
    INotationInteraction* m_interaction = nullptr;
    INotationUndoStackPtr m_undoStack;

    async::Notification m_stateChanged;
    async::Notification m_noteAdded;

    ScoreCallbacks* m_scoreCallbacks = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONINPUTSTATE_H
