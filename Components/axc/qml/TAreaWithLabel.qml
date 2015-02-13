import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.XmlListModel 2.0

Item {
    property alias text: ta_text_condition.text
    clip: true

    Label {
        id: label_condition

        height: 40
        width: parent.width
        verticalAlignment: Qt.AlignVCenter
        anchors.top: parent.top
        font.pointSize: 16
        font.bold: true
        color: "#E91E63"

        text: "Условие"
    }
    TextArea {
        id: ta_text_condition

        //For custom highlighting. Will be used from cpp
        textDocument.objectName: "TDHighlighted"
        clip: true
        anchors.right: parent.right
        anchors.rightMargin: 0
        y: label_condition.height
        width: parent.width
        height: parent.height - label_condition.height

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

        text: "Условие достижения"

        ListView {
            anchors.right: parent.right
            width: parent.width / 3
            height: parent.height

            model: conventions_xml_model
            delegate: conventions_delegate
        }

        XmlListModel {
            id: conventions_xml_model
            source: "http://127.0.0.1:5555/TablesPath"
            query: "/root/achivement"

            XmlRole { name: "name"; query: "name/string()" }
            XmlRole { name: "cost"; query: "value/string()" }
        }

        Component {
            id: conventions_delegate

            Rectangle {
                height: 50
                width: parent.width
                color: "Transparent"
//                border.color: "#ddd"

//                Column {
//                    anchors.centerIn: parent
//                    spacing: 10
//                    Text { text: name; font.pixelSize: 14 }
//                    Text { text: '$' + cost; font.pixelSize: 14 }
//                }

                Circle {
                    anchors.verticalCenter: parent.verticalCenter
                    x: 10
                    width: 30
                    height: 30
                    color: "#E91E63"
                    Text {
                        anchors.centerIn: parent
                        color: "#fff"
                        text: "Sql"
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: name
                    font.pixelSize: 18
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("Index clicked" + index)
                        ta_text_condition.insert(ta_text_condition.length, cost)
                    }
                }
            }
        }
    }
}

