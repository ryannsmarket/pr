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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    property string type: "INFO" // "QUESTION", "INFO", "WARNING", "ERROR"

    property alias title: titleLabel.text
    property alias text: textLabel.text
    property alias textFormat: textLabel.textFormat

    property bool withIcon: false
    property alias iconCode: icon.iconCode

    property bool withShowAgain: false

    property var buttons: []
    property int defaultButtonId: 0

    property var contentWidth: Math.max(textContent.contentWidth, buttons.contentWidth)
    property var contentHeight: content.contentHeight

    property alias navigation: navPanel

    signal clicked(int buttonId, bool showAgain)

    function onOpened() {
        var btn = null
        for (var i = buttons.count; i > 0; --i) {
            btn = buttons.itemAtIndex(i-1)
            if (btn.accentButton) {
                break;
            }

        }

        console.log("onOpened btn: " + btn.text)
       // btn.navigation.requestActive()

        accessibleText.focused = true
        next.start()
    }

    function standardIcon(type) {
        switch (type) {
        case "QUESTION": return IconCode.QUESTION
        case "INFO": return IconCode.INFO
        case "WARNING": return IconCode.WARNING
        case "ERROR": return IconCode.ERROR
        }

        return IconCode.NONE
    }

    function standardName(type) {
        switch (type) {
        case "QUESTION": return qsTrc("global", "Question")
        case "INFO": return qsTrc("global", "Information")
        case "WARNING": return qsTrc("global", "Warning")
        case "ERROR": return qsTrc("global", "Error")
        }

        return qsTrc("global", "Information")
    }

    NavigationPanel {
        id: navPanel
        name: "StandardDialog"
        order: 1
        direction: NavigationPanel.Horizontal
        accessible.role: Accessible.Dialog
        accessible.name: root.standardName(root.type)
    }

    AccessibleItem {
        id: accessibleText
        accessibleParent: navPanel.accessible
        role: Accessible.StaticText
        name: root.title
    }

    Timer {
        id: next
        interval: 1000
        onTriggered: {
            var btn = null
            for (var i = buttons.count; i > 0; --i) {
                btn = buttons.itemAtIndex(i-1)
                if (btn.accentButton) {
                    break;
                }

            }

            console.log("onOpened btn: " + btn.text)
            btn.navigation.requestActive()
        }
    }

    ColumnLayout {
        id: content

        anchors.fill: parent

        property int contentHeight: childrenRect.height

        spacing: 16

        RowLayout {
            id: textContent

            property int contentWidth: childrenRect.width

            Layout.preferredWidth: childrenRect.width
            Layout.preferredHeight: childrenRect.height

            spacing: 28

            StyledIconLabel {
                id: icon

                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48

                font.pixelSize: 48
                iconCode: root.standardIcon(root.type)

                visible: root.withIcon && !isEmpty
            }

            Column {
                property int _preferredWidth: Math.max(Math.min(titleLabel.implicitWidth, 420),
                                                       Math.min(textLabel.implicitWidth, 420))

                Layout.preferredWidth: _preferredWidth + /*todo*/ 76
                Layout.preferredHeight: childrenRect.height

                spacing: 16

                StyledTextLabel {
                    id: titleLabel

                    width: parent._preferredWidth + /*todo*/ 4

                    font: ui.theme.largeBodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    id: textLabel

                    width: parent._preferredWidth + /*todo*/ 4

                    horizontalAlignment: Text.AlignLeft
                    maximumLineCount: 3
                    wrapMode: Text.WordWrap

                    visible: !isEmpty
                }

                CheckBox {
                    id: withShowAgainCheckBox

                    Layout.fillWidth: true

                    text: qsTrc("ui", "Show this message again")

                    checked: true

                    visible: Boolean(root.withShowAgain)

                    onClicked: {
                        checked = !checked
                    }
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: 30
            Layout.fillWidth: true

            color: "#ff0000"

            ListView {
                id: buttons
                anchors.right: parent.right
                spacing: 12

                width: contentWidth
                height: contentHeight

                model: root.buttons
                orientation: Qt.Horizontal

                delegate: FlatButton {

                    navigation.panel: navPanel
                    navigation.column: model.index

                    text: modelData.title
                    accentButton: Boolean(modelData.accent)

                    onClicked: {
                        root.clicked(modelData.buttonId, withShowAgainCheckBox.checked)
                    }
                }
            }
        }
    }
}
