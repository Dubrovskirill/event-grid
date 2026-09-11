import QtQuick 2.15

Item {
    id: root

    signal cancelRequested()

    readonly property color backgroundColor: "#101218"
    readonly property color surfaceColor: "#1B1F2A"
    readonly property color primaryColor: "#89B4FA"
    readonly property color primaryTextColor: "#F2F4F8"
    readonly property color secondaryTextColor: "#A9B1C3"

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    Rectangle {
        id: backButton

        anchors {
            top: parent.top
            left: parent.left
            topMargin: Math.max(24, Math.min(parent.width, parent.height) * 0.06)
            leftMargin: Math.max(24, Math.min(parent.width, parent.height) * 0.06)
        }
        width: 48
        height: 40
        radius: 10
        color: backMouseArea.pressed ? "#2B3242" : "transparent"

        Text {
            anchors.centerIn: parent
            text: "‹"
            color: root.primaryTextColor
            font.pixelSize: 34
        }

        MouseArea {
            id: backMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.cancelRequested()
        }
    }

    Column {
        anchors {
            top: parent.top
            left: backButton.right
            right: parent.right
            topMargin: Math.max(28, Math.min(parent.width, parent.height) * 0.065)
            leftMargin: 10
            rightMargin: Math.max(24, Math.min(parent.width, parent.height) * 0.06)
        }
        spacing: 8

        Text {
            text: qsTr("Create a map")
            color: root.primaryTextColor
            font {
                pixelSize: 32
                weight: Font.DemiBold
            }
        }

        Text {
            width: parent.width
            text: qsTr("Choose how you want to organize your map.")
            color: root.secondaryTextColor
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(460, parent.width - 48)
        height: 220
        radius: 18
        color: root.surfaceColor

        Column {
            anchors {
                fill: parent
                margins: 28
            }
            spacing: 12

            Text {
                text: qsTr("Map setup is next")
                color: root.primaryTextColor
                font {
                    pixelSize: 22
                    weight: Font.DemiBold
                }
            }

            Text {
                width: parent.width
                text: qsTr("This screen will collect a map name, grid type, dimensions, unit, and mode. These values will be connected once the domain model and Create Map use case are ready.")
                color: root.secondaryTextColor
                font.pixelSize: 15
                wrapMode: Text.WordWrap
            }

            Item {
                width: 1
                height: 1
            }

            Text {
                text: qsTr("For now, return to My Maps to continue.")
                color: root.primaryColor
                font.pixelSize: 14
            }
        }
    }
}
