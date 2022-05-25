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
import MuseScore.Project 1.0
import MuseScore.Cloud 1.0

StyledDialogView {
    id: root

    property bool canSaveToComputer: true
    property string name
    property int visibility: CloudVisibility.Private

    contentWidth: contentItem.implicitWidth
    contentHeight: contentItem.implicitHeight

    margins: 20

    function done(response, data = {}) {
        let value = Object.assign(({ response: response }), data)

        root.ret = {
            errcode: 0,
            value: value
        }

        root.hide()
    }

    Item {
        id: contentItem

        implicitWidth: Math.max(420, contentColumn.implicitWidth)
        implicitHeight: contentColumn.implicitHeight

        AccountAvatar {
            anchors.top: parent.top
            anchors.right: parent.right

            side: 38
            url: accountModel.accountInfo.avatarUrl

            AccountModel {
                id: accountModel

                Component.onCompleted: {
                    load()
                }
            }
        }

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            spacing: 20

            StyledTextLabel {
                id: titleLabel
                text: root.visibility === CloudVisibility.Public
                      ? qsTrc("project/save", "Publish to MuseScore.com")
                      : qsTrc("project/save", "Save to cloud")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            ColumnLayout {
                id: optionsColumn
                spacing: 16

                NavigationPanel {
                    id: optionsNavPanel
                    name: "SaveToCloudOptions"
                    enabled: optionsColumn.enabled && optionsColumn.visible
                    direction: NavigationPanel.Vertical
                    section: root.navigationSection
                    order: 1
                    accessible.name: qsTrc("project/save", "Options")
                }

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: qsTrc("project/save", "Name")
                        horizontalAlignment: Text.AlignLeft
                    }

                    TextInputField {
                        Layout.fillWidth: true
                        currentText: root.name

                        navigation.panel: optionsNavPanel
                        navigation.row: 1
                        accessible.name: titleLabel.text + ". " + qsTrc("project/save", "Name") + ": " + currentText

                        onCurrentTextEdited: function(newTextValue) {
                            root.name = newTextValue
                        }
                    }
                }

                ColumnLayout {
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        //: visibility of a score on MuseScore.com: private or public
                        text: qsTrc("project/save", "Visibility")
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledDropdown {
                        Layout.fillWidth: true

                        model: [
                            { value: CloudVisibility.Private, text: qsTrc("project/save", "Private") },
                            { value: CloudVisibility.Public, text: qsTrc("project/save", "Public") }
                        ]

                        currentIndex: indexOfValue(root.visibility)

                        navigation.panel: optionsNavPanel
                        navigation.row: 2
                        navigation.accessible.name: qsTrc("project/save", "Visibility") + ": " + currentText

                        onActivated: function(index, value) {
                            root.visibility = value
                        }
                    }
                }
            }

            ButtonBox {
                id: buttonBox

                separationGap: false

                buttons: ButtonBoxModel.Cancel

                ButtonBoxItem {
                    text: qsTrc("project/save", "Save to computer")
                    buttonRole: ButtonBoxModel.CustomRole
                    visible: root.canSaveToComputer

                    navigationName: "SaveToComputer"

                    onClicked: {
                        root.done(SaveToCloudResponse.SaveLocallyInstead)
                    }
                }

                ButtonBoxItem {
                    text: qsTrc("project/save", "Save")
                    buttonRole: ButtonBoxModel.ApplyRole
                    enabled: Boolean(root.name)
                    isAccent: true

                    navigationName: "Save"

                    onClicked: {
                        root.done(SaveToCloudResponse.Ok, {
                                      name: root.name,
                                      visibility: root.visibility
                                  })
                    }
                }

                navigationPanel: NavigationPanel {
                    name: "SaveToCloudButtons"
                    enabled: buttonBox.enabled && buttonBox.visible
                    section: root.navigationSection
                    direction: NavigationPanel.Horizontal
                    order: 2
                }

                onStandardButtonClicked: function(type) {
                    if (type === ButtonBoxModel.Cancel) {
                        root.done(SaveToCloudResponse.Cancel)
                    }
                }
            }
        }
    }
}

