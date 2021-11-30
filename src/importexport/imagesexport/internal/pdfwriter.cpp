/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pdfwriter.h"

#include <QPdfWriter>

#include "libmscore/masterscore.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;
using namespace mu::draw;
using namespace Ms;

std::vector<INotationWriter::UnitType> PdfWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART, UnitType::MULTI_PART };
}

mu::Ret PdfWriter::write(INotationPtr notation, io::Device& destinationDevice, const Options& options)
{
    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::PER_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    preparePdfWriter(pdfWriter, notation->completedTitle());

    PagedPaintDevice dev(&pdfWriter);

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    INotationPainting::Options opt;

    notation->painting()->paintPdf(&dev, &painter, opt);

    painter.endDraw();

    return true;
}

mu::Ret PdfWriter::writeList(const INotationPtrList& notations, io::Device& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    INotationPtr firstNotation = notations.front();
    IF_ASSERT_FAILED(firstNotation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    preparePdfWriter(pdfWriter, firstNotation->scoreTitle());

    PagedPaintDevice dev(&pdfWriter);

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    INotationPainting::Options opt;

    for (auto notation : notations) {
        IF_ASSERT_FAILED(notation) {
            return make_ret(Ret::Code::UnknownError);
        }

        if (notation != firstNotation) {
            pdfWriter.newPage();
        }

        notation->painting()->paintPdf(&dev, &painter, opt);
    }

    painter.endDraw();

    return true;
}

void PdfWriter::preparePdfWriter(QPdfWriter& pdfWriter, const QString& title) const
{
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    pdfWriter.setCreator("MuseScore Version: " VERSION);
    pdfWriter.setTitle(title);
    pdfWriter.setPageMargins(QMarginsF());
}
