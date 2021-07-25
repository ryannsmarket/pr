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

#ifndef MU_PALETTE_PALETTEWIDGET_H
#define MU_PALETTE_PALETTEWIDGET_H

#include <QScrollArea>

#include "palette/palettepanel.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "ui/iuiactionsregister.h"
#include "context/iglobalcontext.h"
#include "iinteractive.h"

namespace Ms {
class Element;
class XmlWriter;
class XmlReader;
}

namespace mu::palette {
class PaletteWidget : public QWidget
{
    Q_OBJECT

    INJECT_STATIC(palette, IPaletteConfiguration, configuration)
    INJECT_STATIC(palette, ui::IUiActionsRegister, actionsRegister)
    INJECT_STATIC(palette, context::IGlobalContext, globalContext)
    INJECT(palette, framework::IInteractive, interactive)

public:
    PaletteWidget(QWidget* parent = nullptr);
    PaletteWidget(PalettePanelPtr palette, QWidget* parent = nullptr);

    QString name() const;
    void setName(const QString& name);

    // Elements & Cells
    int actualCellCount() const;
    PaletteCellPtr cellAt(size_t index) const;
    Ms::ElementPtr elementForCellAt(int idx) const;

    PaletteCellPtr insertElement(int idx, Ms::ElementPtr element, const QString& name, qreal mag = 1.0);
    PaletteCellPtr appendElement(Ms::ElementPtr element, const QString& name, qreal mag = 1.0);

    void clear();

    // Drawing
    static void paintPaletteElement(void* data, Ms::Element* element);

    qreal mag() const;
    void setMag(qreal val);

    qreal yOffset() const;
    void setYOffset(qreal val);

    // Grid
    bool drawGrid() const;
    void setDrawGrid(bool val);

    int gridWidth() const;
    int gridHeight() const;
    void setGridSize(int width, int height);

    int gridWidthScaled() const;
    int gridHeightScaled() const;

    // Filter
    bool isFilterActive();
    bool setFilterText(const QString& text);

    // Selection
    bool isSelectable() const;
    void setSelectable(bool val);

    int selectedIdx() const;
    void setSelected(int idx);

    int currentIdx() const;
    void setCurrentIdx(int idx);

    void nextPaletteElement();
    void prevPaletteElement();

    // Applying elements
    bool isApplyingElementsDisabled() const;
    void setApplyingElementsDisabled(bool val);

    void applyPaletteElement();
    static bool applyPaletteElement(Ms::ElementPtr element, Qt::KeyboardModifiers modifiers = {});

    // Settings
    bool readOnly() const;
    void setReadOnly(bool val);
    void setCellReadOnly(int cellIndex, bool readonly);

    void setShowContextMenu(bool val);

    // Sizing
    int heightForWidth(int) const override;
    QSize sizeHint() const override;

    // Read/write
    void read(Ms::XmlReader&);
    void write(Ms::XmlWriter&) const;
    bool readFromFile(const QString& path);
    void writeToFile(const QString& path) const;

signals:
    void changed();
    void boxClicked(int index);

private:
    bool event(QEvent*) override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;

    void dragEnterEvent(QDragEnterEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dropEvent(QDropEvent*) override;

    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;

    qreal paletteScaling() const;
    const std::vector<PaletteCellPtr>& cells() const;

    int rows() const;
    int columns() const;

    int cellIndexForPoint(const QPoint&) const; // Only indices of actual cells
    int theoreticalCellIndexForPoint(const QPoint&) const; // Also indices greater than cells.size() - 1
    QRect rectForCellAt(int idx) const;
    QPixmap pixmapForCellAt(int cellIdx) const;

    const std::vector<PaletteCellPtr>& actualCellsList() const;

    bool notationHasSelection() const;
    void applyElementAtPosition(QPoint pos, Qt::KeyboardModifiers modifiers);

    PalettePanelPtr m_palette;

    std::vector<PaletteCellPtr> m_filteredCells; // used for filter & backup

    bool m_isFilterActive = false;
    bool m_selectable = false;
    bool m_isReadOnly = false;
    bool m_isApplyingElementsDisabled = false;
    bool m_showContextMenu = true;

    int m_currentIdx = -1;
    int m_pressedIndex = -1;
    int m_dragIdx = -1;
    int m_selectedIdx = -1;
    QPoint m_dragStartPosition;
};

class PaletteScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit PaletteScrollArea(PaletteWidget* w, QWidget* parent = nullptr);

    bool restrictHeight() const;
    void setRestrictHeight(bool val);

private:
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent*) override;

    bool m_restrictHeight = false;
};
}

#endif // MU_PALETTE_PALETTEWIDGET_H
