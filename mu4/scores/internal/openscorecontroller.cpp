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
#include "openscorecontroller.h"

#include <QObject>
#include "log.h"

using namespace mu::scores;

void OpenScoreController::init()
{
    dispatcher()->reg(this, "file-open", this, &OpenScoreController::openScore);
    dispatcher()->reg(this, "file-import", this, &OpenScoreController::importScore);
}

void OpenScoreController::openScore()
{
    QStringList filter;
    filter << QObject::tr("MuseScore Files") + " (*.mscz *.mscx)";

    doOpenScore(filter);
}

void OpenScoreController::importScore()
{
    QString allExt = "*.mscz *.mscx *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx"
                     "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mscz, *.mscx,";

    QStringList filter;
    filter << QObject::tr("All Supported Files") + " (" + allExt + ")"
           << QObject::tr("MuseScore Files") + " (*.mscz *.mscx)"
           << QObject::tr("MusicXML Files") + " (*.mxl *.musicxml *.xml)"
           << QObject::tr("MIDI Files") + " (*.mid *.midi *.kar)"
           << QObject::tr("MuseData Files") + " (*.md)"
           << QObject::tr("Capella Files") + " (*.cap *.capx)"
           << QObject::tr("BB Files (experimental)") + " (*.mgu *.sgu)"
           << QObject::tr("Overture / Score Writer Files (experimental)") + " (*.ove *.scw)"
           << QObject::tr("Bagpipe Music Writer Files (experimental)") + " (*.bmw *.bww)"
           << QObject::tr("Guitar Pro Files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)"
           << QObject::tr("Power Tab Editor Files (experimental)") + " (*.ptb)"
           << QObject::tr("MuseScore Backup Files") + " (*.mscz, *.mscx,)";

    doOpenScore(filter);
}

void OpenScoreController::doOpenScore(const QStringList& filter)
{
    std::string filterStr = filter.join(";;").toStdString();
    io::path filePath = interactive()->selectOpeningFile("Score", "", filterStr);
    if (filePath.empty()) {
        return;
    }

    if (globalContext()->isContainsNotation(filePath)) {
        LOGI() << "already loaded score: " << filePath;
        return;
    }

    auto notation = notationCreator()->newNotation();
    IF_ASSERT_FAILED(notation) {
        return;
    }

    Ret ret = notation->load(filePath);
    if (!ret) {
        LOGE() << "failed load: " << filePath << ", ret: " << ret.toString();
        //! TODO Show dialog about error
        return;
    }

    globalContext()->addNotation(notation);
    globalContext()->setCurrentNotation(notation);
}
