import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: qsTr("Achivements creator")

    Item {
        id: top_level
        anchors.fill: parent
        property int animation_duration: 350

        property alias nameText: text_name.text
        property alias descriptionText: text_description.text
        property alias conditionText: text_condition.text
        property alias idLabel: label_id.text

        function updateProperties() {
            var index = lview.currentIndex
            if (index < 0) {
                top_level.state = ""
                return
            }
            var dict = xml_model.dict(lview.currentIndex)
            top_level.nameText = dict["Name"]
            top_level.descriptionText = dict["Description"]
            top_level.conditionText = dict["Condition"]
            top_level.idLabel = dict["id"]
        }
        Rectangle {
            id: top_panel
            width: parent.width
            height: 60
            clip: true

            color: "#00BCD4"

            Row {
                anchors.fill: parent
                anchors.rightMargin: 10
                spacing: 10
                layoutDirection: Qt.RightToLeft

                CircleButton {
                    id: move_right_panel
                    width: 40
                    height: 40

                    text: top_level.state == "SHOW_RIGHT" ? "\u2192" : "\u2190"
                    font.pointSize: 24
                    color: "white"
                    textColor: "#03A9F4"

                    onClicked: {
//                        console.log("right panel button")
//                        if (text == "\u2192") {
//                            top_level.state = "SHOW_RIGTH"
//                        } else {
//                            top_level.state = ""
//                        }
                        top_level.state = ""
                    }
                }
                CircleButton {
                    id: add_new
                    width: 40
                    height: 40

                    text: '+'
                    font.pointSize: 24
                    color: "white"
                    textColor: "#03A9F4"

                    onClicked: {
                        var dict = {}
                        dict["id"] = xml_model.getId()
                        dict["Name"] = "Имя"
                        dict["Description"] = "Описание достижения"
                        dict["Condition"] = "Условие достижения"

                        xml_model.append(dict)
                        lview.currentIndex = xml_model.count() - 1
                        top_level.updateProperties()
                        console.log("dict" + dict["Name"])
                        top_level.state = "SHOW_RIGHT"
                    }
                }
                CircleButton {
                    id: remove_entry
                    width: lview.currentIndex != -1 ? 40 : 0
                    height: lview.currentIndex != -1 ? 40 : 0

                    text: 'R'
                    font.pointSize: 18
                    color: "white"
                    textColor: "#03A9F4"

                    onClicked: {
                        xml_model.remove(lview.currentIndex)
                        if (xml_model.count() >= lview.currentIndex) {
                            lview.currentIndex = xml_model.count() - 1
                        }
                        top_level.updateProperties()
                    }
                }
                CircleButton {
                    id: save_entry
                    width: lview.currentIndex != -1 ? 40 : 0
                    height: lview.currentIndex != -1 ? 40 : 0

                    text: 'S'
                    font.pointSize: 18
                    color: "white"
                    textColor: "#03A9F4"

                    onClicked: {
                        var dict = {}
                        dict["id"] = parseInt(top_level.idLabel)
                        dict["Name"] = text_name.text
                        dict["Description"] = text_description.text
                        dict["Condition"] = text_condition.text

                        var index = lview.currentIndex
                        xml_model.update(index, dict)
                        console.log("updating dict" + dict["Name"])
                    }
                }
            }
        }

        Rectangle {
            id: left_panel
            color: "white"
            x: 0
            y: top_panel.height
            width: parent.width
            height: parent.height - top_panel.height

            ListView {
                id: lview
                anchors.fill: parent
                clip: true
                model: xml_model

                highlightMoveDuration: 75
                highlight: Rectangle {
                    color: "#eee"
                    opacity: 1
                }

                delegate: Item {
                    width: parent.width
                    height: 50

                    Circle {
                        anchors.verticalCenter: parent.verticalCenter
                        x: 10
                        width: 40
                        height: 40
                        color: "#EFFB41"
                        Text {
                            anchors.centerIn: parent
                            color: "#303030"
                            text: id.toString()
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: Name
                        font.pointSize: 12
                    }

                    MouseArea {
                        id: list_elem_area
                        z: 1
                        hoverEnabled: false
                        anchors.fill: parent
                        onClicked:  {
                            lview.currentIndex = index
                            top_level.state = "SHOW_RIGHT"
                            top_level.nameText = Name
                            top_level.descriptionText = Description
                            top_level.conditionText = Condition
                            top_level.idLabel = id
                        }
                    }
                }

                Component.onCompleted: {
//                    if (!xml_model.fromXml()) {
//                        xml_model.addTest()
//                    }
                    xml_model.fromXml()

                    currentIndex = -1
                }
            }
        }

        Rectangle {
            id: right_panel

            x: parent.width
            y: top_panel.height
            width: parent.width - left_panel.width
            height: parent.height - top_panel.height
            clip: true

            color: "#eee"

            Label {
                id: label_id

                height: 40
                width: 40
                anchors.left: parent.left
                anchors.leftMargin: 10
                verticalAlignment: Qt.AlignVCenter
                y: 10
                font.pointSize: 16
                font.bold: true
                color: "#E91E63"

                text: "-1"
                visible: false
            }

            Label {
                id: label_name

                height: 40
                width: parent.width / 2
                anchors.right: parent.right
                anchors.rightMargin: 10
                verticalAlignment: Qt.AlignVCenter
                y: 10
                font.pointSize: 16
                font.bold: true
                color: "#E91E63"

                text: "Имя"
            }
            TextField {
                id: text_name

                height: 40
                style: TextFieldStyle {
                    background: Rectangle {
                        radius: 0
                        implicitWidth: 100
                        implicitHeight: 24
                        border.color: "#ccc"
                        border.width: 1
                    }
                }

                width: parent.width / 2
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: label_name.bottom
                font.pointSize: 16

                text: "Name"
            }
            Label {
                id: label_description

                height: 40
                width: parent.width - 20
                anchors.right: parent.right
                anchors.rightMargin: 10
                verticalAlignment: Qt.AlignVCenter
                anchors.top: text_name.bottom
                font.pointSize: 16
                font.bold: true
                color: "#E91E63"

                text: "Описание"
            }
            TextArea {
                id: text_description

                height: 200
                style: TextAreaStyle {
                    frame: Rectangle {
                        radius: 0
                        implicitWidth: 100
                        implicitHeight: 24
                        border.color: "#ddd"
                        border.width: 1
                    }
                }
                width: parent.width - 20
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: label_description.bottom
                font.pointSize: 14


                text: "Описание достижения"
            }
            Label {
                id: label_condition

                height: 40
                width: parent.width - 20
                anchors.right: parent.right
                anchors.rightMargin: 10
                verticalAlignment: Qt.AlignVCenter
                anchors.top: text_description.bottom
                font.pointSize: 16
                font.bold: true
                color: "#E91E63"

                text: "Условие"
            }
            TextArea {
                id: text_condition

                //For custom highlighting. Will be used from cpp
                textDocument.objectName: "TDHighlighted"
                height: 200
                style: TextAreaStyle {
                    frame: Rectangle {
                        radius: 0
                        implicitWidth: 100
                        implicitHeight: 24
                        border.color: "#ddd"
                        border.width: 1
                    }
                }
                width: parent.width - 20
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: label_condition.bottom
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                font.pointSize: 14
                font.family: "Courier"


                text: "Условие достижения"
            }
        }

        states: [
            State {
                name: "SHOW_RIGHT"
                PropertyChanges {target: left_panel; width: 250}
                PropertyChanges {target: right_panel; width: parent.width - left_panel.width; x: left_panel.width;}
                PropertyChanges {target: top_panel; color: "#03A9F4"}
            }
        ]
        transitions: [
            Transition {
                ParallelAnimation {
                    NumberAnimation {properties: "visible, opacity,y,x, width, contentX, contentY, height"; duration: top_level.animation_duration; easing.type: Easing.InOutExpo }
                    ColorAnimation {target: top_panel; from: "#00BCD4"; to: "#03A9F4"; duration: top_level.animation_duration}
                }
            }
        ]
    }
}
