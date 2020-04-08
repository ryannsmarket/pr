import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

Rectangle {
    id: root

    property bool opened: false
    property bool hovered: false
    signal resetToDefaultsRequested

    height: 24
    width: 24

    radius: 4

    color: "transparent"

    opacity: menuButtonMouseArea.containsMouse || root.opened
             || isMenuButtonVisible

    StyledIcon {
        anchors.centerIn: parent

        icon: "qrc:/resources/icons/menu_dots.svg"
    }

    MouseArea {
        id: menuButtonMouseArea

        anchors.fill: parent

        onClicked: {
            if (!opened) {
                contextMenu.open()
            } else {
                contextMenu.close()
            }
        }

        hoverEnabled: true

        onEntered: {
            root.hovered = true
        }

        onExited: {
            root.hovered = false
        }
    }

    Menu {
        id: contextMenu

        y: root.y + root.height

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutsideParent

        MenuItem {
            text: qsTr("Reset to defaults")
            onTriggered: {
                root.resetToDefaultsRequested()
            }
        }

        onOpened: {
            root.opened = true
        }

        onClosed: {
            root.opened = false
        }
    }

    states: [

        State {
            name: "HOVERED"
            when: root.hovered && !root.opened
            PropertyChanges {
                target: root
                color: globalStyle.button
            }
        },
        State {
            name: "PRESSED"
            when: root.opened
            PropertyChanges {
                target: root
                color: globalStyle.voice1Color
            }
        }
    ]
}
