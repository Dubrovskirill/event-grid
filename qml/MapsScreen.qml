import QtQuick 2.15

Item {
    id: root

    readonly property color backgroundColor: "#101218"
    readonly property color surfaceColor: "#1B1F2A"
    readonly property color primaryColor: "#89B4FA"
    readonly property color primaryTextColor: "#F2F4F8"
    readonly property color secondaryTextColor: "#A9B1C3"

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    Column {
        id: header

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: Math.max(24, Math.min(parent.width, parent.height) * 0.06)
            leftMargin: Math.max(24, Math.min(parent.width, parent.height) * 0.06)
            rightMargin: Math.max(24, Math.min(parent.width, parent.height) * 0.06)
        }
        spacing: 12

        Text {
            text: qsTr("My Maps")
            color: root.primaryTextColor
            font {
                pixelSize: 32
                weight: Font.DemiBold
            }
        }

        Text {
            width: parent.width
            text: qsTr("Visualize habits, events, and progress as interactive maps.")
            color: root.secondaryTextColor
            font.pixelSize: 16
            wrapMode: Text.WordWrapплане
        }

    }

    Column {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        anchors.verticalCenterOffset: 24
        width: Math.min(360, root.width - 48)
        spacing: 14

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 72
            height: 72
            radius: width / 2
            color: root.surfaceColor

            Text {
                anchors.centerIn: parent
                text: "+"
                color: root.primaryColor
                font.pixelSize: 40
                font.weight: Font.Light
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("No maps yet")
            color: root.primaryTextColor
            font {
                pixelSize: 22
                weight: Font.DemiBold
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: qsTr("Create your first map to begin tracking what matters to you.")
            color: root.secondaryTextColor
            font.pixelSize: 15
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 176
            height: 46
            radius: 12
            color: createMouseArea.pressed ? "#6E9FEA" : root.primaryColor

            Text {
                anchors.centerIn: parent
                text: qsTr("Create a map")
                color: "#152033"
                font {
                    pixelSize: 16
                    weight: Font.DemiBold
                }
            }

            MouseArea {
                id: createMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: hint.visible = true
            }
        }

        Text {
            id: hint

            anchors.horizontalCenter: parent.horizontalCenter
            visible: false
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: qsTr("Map creation will be available in the next development step.")
            color: root.secondaryTextColor
            font.pixelSize: 13
        }
    }
}
