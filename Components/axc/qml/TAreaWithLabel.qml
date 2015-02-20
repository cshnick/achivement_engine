import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.XmlListModel 2.0

Item {
    id: tarea_with_label

    property alias text: ta_text_condition.text
    signal serverReply(string reply)

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

    CircleButton {
        height: 30
        width: 30
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.verticalCenter: undefined

        text: "::"
        font.bold: true
        textColor: "#fff"
        color: "#E91E63"

        onClicked: {
            if (ta_text_condition.state === "HelperVisible") {
                ta_text_condition.state = ""
            } else {
                ta_text_condition.state = "HelperVisible"
            }
        }
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
            id: helper_list
            x: parent.width
            y: 0
            width: parent.width / 3
            height: parent.height

            model: conventions_xml_model
            delegate: conventions_delegate

            Rectangle {
                id: vline
                y: 10
                height: parent.height - 20
                width: 1
                color: "#ddd"
                opacity: 0.7
            }
        }



        XmlListModel {
            id: conventions_xml_model
            source: "http://127.0.0.1:5555/TablesPath"
            query: "/root/achivement"

            XmlRole { name: "name"; query: "name/string()" }
            XmlRole { name: "value"; query: "value/string()" }
            XmlRole { name: "type_str"; query: "type_str/string()"}
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
//                    Text { text: '$' + value; font.pixelSize: 14 }
//                }
                Rectangle {
                    anchors.fill: parent
                    opacity: 0.9
                    color: "#fff"
                }
                Rectangle {
                    color: "#ddd"
                    x: 20
                    y: parent.height -1;
                    height: 1
                    width: parent.width - 40

                }

                Circle {
                    id: type_sircle
                    function color_for_type(tp) {
                        if (tp === "sql") {
                            return "#009688"
                        } else if (tp === "Numeric") {
                            return "#E91E63"
                        } else if (tp === "Achievements") {
                            return "#9C27B0"
                        } else if (tp === "Statistics") {
                            return "#03A9F4"
                        }

                        return "#03A9F4"
                    }
                    function short_text_for_type(tp) {
                        if (tp === "sql") {
                            return "Sql"
                        } else if (tp === "Numeric") {
                            return "Num"
                        } else if (tp === "Achievements") {
                            return "Ach"
                        } else if (tp === "Statistics") {
                            return "Stt"
                        }

                        return tp
                    }

                    anchors.verticalCenter: parent.verticalCenter
                    x: 5
                    width: 30
                    height: 30
                    color: color_for_type(type_str)
                    Text {
                        anchors.centerIn: parent
                        color: "#fff"
                        text: parent.short_text_for_type(type_str)
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: type_sircle.right
                    anchors.leftMargin: 10
                    text: name
                    font.pixelSize: 16
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("Index clicked" + index)
                        ta_text_condition.insert(ta_text_condition.length, value)
                    }
                }
            }
        }

        states: [
            State {
                name: "HelperVisible"
                PropertyChanges {
                    target: helper_list
                    x: ta_text_condition.width - helper_list.width
                }
            }
        ]
        transitions: [
            Transition {
                ParallelAnimation {
                    NumberAnimation {target: helper_list; properties: "y,x"; duration: top_level.animation_duration; easing.type: Easing.InOutExpo }
                }
            }
        ]
    }
}

