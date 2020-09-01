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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_NOTATIONPARTS_H
#define MU_NOTATION_NOTATIONPARTS_H

#include "inotationparts.h"
#include "igetscore.h"
#include "async/asyncable.h"

namespace mu {
namespace notation {
class NotationParts : public INotationParts, public async::Asyncable
{
public:
    NotationParts(IGetScore* getScore, mu::async::Notification selectionChangedNotification);

    PartList partList() const override;
    instruments::InstrumentList instrumentList(const QString& partId) const override;
    StaffList staffList(const QString& partId, const QString& instrumentId) const override;

    bool canChangeInstrumentVisibility(const QString& partId, const QString& instrumentId) const override;

    void setInstruments(const instruments::InstrumentList& instruments) override;
    void setPartVisible(const QString& partId, bool visible) override;
    void setInstrumentVisible(const QString& partId, const QString& instrumentId, bool visible) override;
    void setStaffVisible(int staffIndex, bool visible) override;
    void setVoiceVisible(int staffIndex, int voiceIndex, bool visible) override;
    void setPartName(const QString& partId, const QString& name) override;
    void setInstrumentName(const QString& partId, const QString& instrumentId, const QString& name) override;
    void setInstrumentAbbreviature(const QString& partId, const QString& instrumentId, const QString& abbreviature) override;
    void setStaffType(int staffIndex, StaffType type) override;
    void setCutaway(int staffIndex, bool value) override;
    void setSmallStaff(int staffIndex, bool value) override;

    void removeParts(const std::vector<QString>& partsIds) override;
    void removeInstruments(const QString& partId, const std::vector<QString>& instrumentIds) override;
    void removeStaves(const std::vector<int>& stavesIndexes) override;

    void moveParts(const std::vector<QString>& partIds, const QString& toPartId, InsertMode mode = Before) override;
    void moveInstruments(const std::vector<QString>& instrumentIds, const QString& fromPartId, const QString& toPartId,
                         const QString& toInstrumentId,InsertMode mode = Before) override;
    void moveStaves(const std::vector<int>& stavesIndexes, int toStaffIndex, InsertMode mode = Before) override;

    void appendInstrument(const QString& partId, const instruments::Instrument& instrument) override;
    void appendStaff(const QString& partId, const QString& instrumentId) override;
    void appendLinkedStaff(int originStaffIndex) override;

    void replaceInstrument(const QString& partId, const QString& instrumentId, const instruments::Instrument& newInstrument) override;

    mu::async::Channel<PartChangeData> partChanged() const override;
    async::Channel<InstrumentChangeData> instrumentChanged() const override;
    async::Channel<StaffChangeData> staffChanged() const override;
    async::Notification partsChanged() const override;

    async::Channel<StaffChangeData> staffAppended() const override;
    async::Channel<InstrumentChangeData> instrumentAppended() const override;

    async::Notification canChangeInstrumentsVisibilityChanged() const override;

private:
    struct InstrumentInfo
    {
        int tick = 0;
        Ms::Instrument* instrument = nullptr;

        InstrumentInfo() = default;

        InstrumentInfo(int tick, Ms::Instrument* instrument)
            : tick(tick), instrument(instrument) {}

        bool isValid() const { return instrument != nullptr; }
    };

    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;

    void startEdit();
    void apply();

    Ms::ChordRest* selectedChord() const;

    bool isDoublingInstrument(int ticks) const;
    bool isInstrumentAssignedToChord(const QString& partId, const QString& instrumentId) const;
    void updateCanChangeInstrumentsVisibility();
    void assignIstrumentToSelectedChord(Ms::Instrument* instrument);

    void doMovePart(const QString& partId, const QString& toPartId, InsertMode mode = Before);
    void doSetStaffVisible(Staff* staff, bool visible);
    void doRemoveParts(const std::vector<QString>& partsIds);
    void doRemoveInstruments(Part* part, const std::vector<QString>& instrumentIds);
    void doRemoveStaves(const std::vector<int>& stavesIndexes);
    void doSetPartName(Part* part, const QString& name);

    Part* part(const QString& partId, const Ms::Score* score = nullptr) const;
    InstrumentInfo instrumentInfo(const Part* part, const QString& instrumentId) const;
    InstrumentInfo instrumentInfo(const Staff* staff) const;
    Staff* staff(int staffIndex) const;

    StaffList staves(const Part* part, const QString& instrumentId) const;

    QList<Part*> scoreParts(const Ms::Score* score) const;
    QList<Part*> excerptParts(const Ms::Score* score) const;

    void appendPart(Part* part);
    void addStaves(Part* part, const instruments::Instrument& instrument, int& globalStaffIndex);

    void insertInstrument(Part* part, Ms::Instrument* instrumentInfo, const StaffList& staves, const QString& toInstrumentId,
                          InsertMode mode);

    void removeUnselectedInstruments(const std::vector<QString>& selectedInstrumentIds);
    std::vector<QString> missingInstrumentIds(const std::vector<QString>& selectedInstrumentIds) const;

    void removeEmptyExcerpts();

    Ms::Instrument convertedInstrument(const instruments::Instrument& instrument) const;
    instruments::Instrument convertedInstrument(const Ms::Instrument* museScoreInstrument, const Part* part) const;

    bool isInstrumentVisible(const Part* part, const QString& instrumentId) const;

    void initStaff(Staff* staff, const instruments::Instrument& instrument, const Ms::StaffType* staffType, int cidx);

    QList<Ms::NamedEventList> convertedMidiActions(const instruments::MidiActionList& midiActions) const;
    instruments::MidiActionList convertedMidiActions(const QList<Ms::NamedEventList>& midiActions) const;

    void sortParts(const std::vector<QString>& instrumentIds);

    IGetScore* m_getScore = nullptr;

    async::Channel<PartChangeData> m_partChanged;
    async::Channel<InstrumentChangeData> m_instrumentChanged;
    async::Channel<StaffChangeData> m_staffChanged;
    async::Notification m_partsChanged;

    async::Channel<StaffChangeData> m_staffAppended;
    async::Channel<InstrumentChangeData> m_instrumentAppended;

    async::Notification m_canChangeInstrumentsVisibilityChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONPARTS_H
