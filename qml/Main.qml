import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 960
    height: 640
    minimumWidth: 420
    minimumHeight: 560
    visible: true
    title: qsTr("Event Grid")

    MapsScreen {
        anchors.fill: parent
    }
}
