import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    id: window

    width: 960
    height: 640
    minimumWidth: 420
    minimumHeight: 560
    visible: true
    title: qsTr("Event Grid")

    property bool creatingMap: false

    MapsScreen {
        anchors.fill: parent
        visible: !window.creatingMap

        onCreateMapRequested: window.creatingMap = true
    }

    CreateMapScreen {
        anchors.fill: parent
        visible: window.creatingMap

        onCancelRequested: window.creatingMap = false
    }
}
