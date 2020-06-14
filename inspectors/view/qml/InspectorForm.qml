import QtQuick 2.9
import QtQml.Models 2.3
import MuseScore.Inspectors 3.3

import "common"
import "general"
import "notation"
import "text"
import "score"

FocusableItem {
    id: root

    property alias inspectorListModel: inspectorRepeater.model

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: globalStyle.window
    }

    Flickable {
        id: flickableArea

        anchors.top: parent.top
        //anchors.top: tabTitleColumn.bottom
        //anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24

        width: parent.width

        contentWidth: contentItem.childrenRect.width

        function updateContentHeight() {
            var resultContentHeight = 0

            for (var i = 0; i < inspectorRepeater.count; ++i) {
                resultContentHeight += inspectorRepeater.itemAt(i).contentHeight
            }

            flickableArea.contentHeight = resultContentHeight
        }

        function ensureContentVisible(delegateY, delegateContentHeight) {

            var contentBottomY = delegateY + delegateContentHeight

            if (contentBottomY > flickableArea.height) {
                flickableArea.contentY = contentBottomY - flickableArea.height
            } else {
                flickableArea.contentY = 0
            }
        }

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        Column {
            width: root.width

            spacing: 6

            Repeater {
                id: inspectorRepeater

                delegate: ExpandableBlank {
                    id: expandableDelegate

                    property var contentHeight: implicitHeight

                    function viewBySectionType() {

                        switch (inspectorData.sectionType) {
                        case Inspector.SECTION_GENERAL: return generalInspector
                        case Inspector.SECTION_TEXT: return textInspector
                        case Inspector.SECTION_NOTATION: return notationInspector
                        case Inspector.SECTION_SCORE_DISPLAY: return scoreInspector
                        case Inspector.SECTION_SCORE_APPEARANCE: return scoreAppearanceInspector
                        }
                    }

                    contentItemComponent: viewBySectionType()

                    menuItemComponent: InspectorMenu {
                        onResetToDefaultsRequested: {
                            inspectorData.requestResetToDefaults()
                        }
                    }

                    Component.onCompleted: {
                        title = inspectorData.title
                    }

                    function updateContentHeight(newContentHeight) {
                        expandableDelegate.contentHeight = newContentHeight
                        flickableArea.updateContentHeight()
                        flickableArea.ensureContentVisible(y, newContentHeight)
                    }

                    Component {
                        id: generalInspector
                        GeneralInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: textInspector
                        TextInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: notationInspector
                        NotationInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: scoreInspector

                        ScoreDisplayInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: scoreAppearanceInspector

                        ScoreAppearanceInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                }
            }
        }
    }

    /// For now the styled text for inspector does not fit with the palettes text.
    /// When a general tab style will be implemented, uncomment the following to get a styled inspector text.
    /// You will also need to change the anchors of the flickableArea to take into account this text.
    /// This stack overflow question might be of some help: https://stackoverflow.com/questions/47518075/how-to-set-different-text-for-a-qdockwidgets-tab-and-window-title
/*
    Rectangle {
        id: tabTitleBackgroundRect

        height: tabTitleColumn.height + 12
        width: parent.width

        color: globalStyle.window
    }

    Column {
        id: tabTitleColumn

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 24

        spacing: 4

        StyledTextLabel {
            id: inspectorTitle

            text: qsTr("Inspector")
            font.bold: true
            font.pixelSize: globalStyle.font.pixelSize * 1.2
        }

        Rectangle {
            id: titleHighlighting

            height: 3
            width: inspectorTitle.width

            color: globalStyle.highlight

            radius: 2
        }

    } */

    FocusableItem {
        id: focusChainBreak

        onActiveFocusChanged: {
            parent.focus = false
        }
    }
}
