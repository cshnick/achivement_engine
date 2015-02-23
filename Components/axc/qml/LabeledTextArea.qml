import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.XmlListModel 2.0

Rectangle {
    id: tarea_with_label

    property alias text: ta_text_area.text
    property alias t_area: ta_text_area
    property alias t_label: label_area
    property int g_margin: 10

    clip: true
    color: "#eee"
    radius: 5

    Label {
        id: label_area

        height: 40
        width: parent.width
        verticalAlignment: Qt.AlignVCenter
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: g_margin
        font.pointSize: 16
        font.bold: true
        color: "#E91E63"

        text: "Условие"
    }

    TextArea {
        id: ta_text_area

        //For custom highlighting. Will be used from cpp
        clip: true
        anchors.right: parent.right
        anchors.rightMargin: g_margin
        anchors.left: parent.left
        anchors.leftMargin: g_margin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: g_margin
        anchors.top: label_area.bottom

        style: TextAreaStyle {
            frame: Rectangle {
                radius: 0
                implicitWidth: 100
                implicitHeight: 24
                border.color: "#ddd"
                border.width: 1
            }
        }
        font.pointSize: 14
        font.family: "Courier"

        text: ""
    }
}

