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
import QtQuick 2.15

Loader {
    id: loader

    signal handleMenuItem(string itemId)

    property alias menu: loader.item
    property var menuAnchorItem: null

    property alias isMenuOpened: loader.active

    property var navigation: null

    QtObject {
        id: prv

        function loadMenu() {
            loader.active = true
        }

        function unloadMenu() {
            loader.active = false
        }
    }

    active: false

    sourceComponent: StyledMenu {
        id: itemMenu

        onHandleMenuItem: {
            itemMenu.close()
            Qt.callLater(loader.handleMenuItem, itemId)
        }

        onClosed: {
            Qt.callLater(prv.unloadMenu)
        }
    }

    function open(model, x = 0, y = 0) {
        prv.loadMenu()

        var menu = loader.menu
        menu.parent = loader.parent
        menu.anchorItem = menuAnchorItem

        if (loader.navigation) {
            menu.navigationParentControl = loader.navigation
            menu.navigation.name = loader.navigation.name + "PopupMenu"
        }

        update(model, x, y)
        menu.open()
    }

    function toggleOpened(model, x = 0, y = 0) {
        prv.loadMenu()

        var menu = loader.menu
        if (menu.isOpened) {
            menu.close()
            return
        }

        open(model, x, y)
    }

    function toggleOpenedWithAlign(model, align) {
        prv.loadMenu()

        loader.menu.preferredAlign = align

        toggleOpened(model)
    }

    function close() {
        if (loader.isMenuOpened) {
            loader.menu.close()
        }
    }

    function update(model, x = 0, y = 0) {
        var menu = loader.menu
        if (!Boolean(menu)) {
            return
        }

        menu.model = model

        if (x !== 0) {
            menu.x = x
        }

        if (y !== 0) {
            menu.y = y
        }
    }

    Timer {
        id: focusOnOpenedMenuTimer

        interval: 50
        running: false
        repeat: false

        onTriggered: {
            if (loader.menu) {
                loader.menu.requestFocus()
            }
        }
    }
}
