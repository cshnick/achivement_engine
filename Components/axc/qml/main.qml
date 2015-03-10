import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../js/helper.js" as Jsh

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: qsTr("Achivements creator")

    Item {
        id: top_level
        anchors.fill: parent
        property int animation_duration: 350

        property alias nameText: block_name.text
        property alias descriptionText: block_description.text
        property alias conditionText: text_condition.text
        property alias idLabel: label_id.text

        property string user: user_view.model.get(0).name
        property string project: 'Таблица умножения'
        property string req_delimiter: '&***'
        property string par_delimiter: '=***'

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

        ShadowRect {
            property int offset: shadowRadius

            id: top_panel
            width: parent.width
            height: 60 + offset
            clip: true
            z: 11

            color: "#00BCD4"

            BlinkAnimation {
                id: ok_animation
            }
            BlinkAnimation {
                id: error_animation
                color: "#F44336"
            }

            Column {
                id: column_user_project
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 30
                visible: !ok_animation.running && !error_animation.running
                width: parent.width / 2
                spacing: 3
                clip: true

                property int font_sz: 16
                property string font_color: "white"


                ListView {
                    id: user_view
                    width: parent.width / 2
                    height: 18
                    clip: true

                    orientation: ListView.Horizontal
                    snapMode: ListView.SnapOneItem; flickDeceleration: 2000
                    preferredHighlightBegin: 0; preferredHighlightEnd: 0
                    highlightRangeMode: ListView.StrictlyEnforceRange

                    delegate: Text {
                        width: user_view.width
                        text: name
                        color: column_user_project.font_color
                        font.pixelSize: column_user_project.font_sz + 2
                        font.bold: true
                    }

                    model: ListModel {
                        ListElement {
                            name: "Игорек"
                        }
                        ListElement {
                            name: "Илья"
                        }
                    }

                    onCurrentIndexChanged: {
                        console.log("user view index changed to " + currentIndex)
                        var user = model.get(currentIndex).name
                        top_level.user = user
                        Jsh.loadAchievements(user, top_level.project)
                        top_level.state = ""
                    }

                    Component.onCompleted: {
                    }
                }

                Text {
                    text: top_level.project
                    color: column_user_project.font_color
                    font.pixelSize: column_user_project.font_sz
                }
            }

            Text {
                id: message_text

                anchors.left: parent.left
                anchors.leftMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 20
                font.bold: false
                text: "Test"
                color: "white"
                opacity: 0
                onStateChanged: {
                    console.log("Message text state change")
                }

                states: [
                    State {
                        name: "OkVisible"
                        when: ok_animation.running
                        PropertyChanges {
                            target: message_text
                            opacity: 1
                        }
                    },
                    State {
                        name: "ErrorVisible"
                        when: error_animation.running
                        PropertyChanges {
                            target: message_text
                            opacity: 1
                        }
                    }
                ]
                transitions: [
                    Transition {
                        reversible: true
                        NumberAnimation {
                            target: message_text
                            property: "opacity"
                            duration: 200
                            easing.type: Easing.InOutQuad
                        }
                    }
                ]
            }

            function reportHttp200(text) {
                ok_animation.start()
                message_text.text = text
            }
            function reportHttpErrort(text) {
                error_animation.start()
                message_text.text = text
            }

            Row {
                anchors.fill: parent
                anchors.rightMargin: 10
                spacing: 10
                layoutDirection: Qt.RightToLeft

                CircleButton {
                    id: move_right_panel
                    explicitWidth: 40
                    explicitHeight: 40

                    text: top_level.state == "SHOW_RIGHT" ? "\u2192" : "\u2190"
                    font.pointSize: 24
                    color: "white"
                    textColor: "#03A9F4"

                    onClicked: {
//                        top_level.state = ""
                        console.log("top level user" + top_level.user)
                    }
                }
                CircleButton {
                    id: add_new
                    explicitWidth: 40
                    explicitHeight: 40

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
                    explicitWidth: 40
                    explicitHeight:  40
                    opacity: top_level.state === "SHOW_RIGHT" ? 1 : 0
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

                    onStateChanged: {
                        console.log("state changed " + state)
                        console.log("visible: " + visible)
                        console.log("opacity: " + opacity)
                    }
                }
                CircleButton {
                    id: save_entry
                    explicitWidth: 40
                    explicitHeight: 40
                    opacity: lview.currentIndex != -1 ? 1 : 0

                    text: 'S'
                    font.pointSize: 18
                    color: "white"
                    textColor: "#03A9F4"

                    onClicked: {
                        var dict = {}
//                        dict["id"] = parseInt(top_level.idLabel)
                        dict["Name"] = block_name.text
                        dict["Description"] = block_description.text
                        dict["Condition"] = text_condition.text

                        var index = lview.currentIndex
                        xml_model.update(index, dict)
                        var str = xml_model.toXml()
                        console.log("Res string: " + str)
                        console.log("updating dict" + dict["Name"])

                        var request = new XMLHttpRequest()
                        request.open('POST', 'http://127.0.0.1:5555/AchievementListSend')
                        request.setRequestHeader('Content-Type', 'text/xml;charset=utf-8')

                        request.onreadystatechange = function () {
                            if (request.readyState === XMLHttpRequest.DONE) {
                                if (request.status === 200) {
                                    console.log("Reply from server: " + request.responseText)
                                    top_panel.reportHttp200("Сохранено...")
                                } else {
                                    console.log("HTTP request failed", request.status)
                                    top_panel.reportHttpError(request.responseText)
                                }
                            }
                        }
                        request.send("content" + top_level.par_delimiter + str
                                     + top_level.req_delimiter + "user" + top_level.par_delimiter + top_level.user
                                     + top_level.req_delimiter + "project" + top_level.par_delimiter + top_level.project)
                    }
                }
            }
        }

        Rectangle {
            id: left_panel
            color: "white"
            x: 0
            y: top_panel.height - top_panel.offset
            width: parent.width
            height: parent.height - top_panel.height

            ListView {
                id: lview
                anchors.fill: parent
                clip: true
                model: xml_model
                anchors.topMargin: 10

                highlightMoveDuration: 75
//                highlight: Rectangle {
//                    color: "#eee"
//                    opacity: 1
//                }

                delegate: Item {
                    id: delegate
                    width: parent.width
                    height: 50

                    Circle {
                        id: delegate_circle
                        anchors.verticalCenter: parent.verticalCenter
                        x: 10
                        width: 40
                        height: 40
                        color: lview.currentIndex === index ? "#EFFB41" : "#EFFB41"
                        Text {
                            anchors.centerIn: parent
                            color: "#303030"
                            text: id.toString()
                        }
                    }

                    Column {
                        id: delegate_column
                        anchors.left: delegate_circle.right
                        anchors.leftMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            text: Name
                            font.pointSize: 12
                            color: lview.currentIndex === index ? "blue" : "black"
                            font.underline: lview.currentIndex === index ? true : false
                            height: 20
                        }
                        Text {
                            text: Description
                            font.pointSize: 12
                            color: lview.currentIndex === index ? "blue" : "black"
                            font.underline: lview.currentIndex === index ? true : false
                            height: top_level.state !== "SHOW_RIGHT" ? 20 : 0
                            visible: top_level.state !== "SHOW_RIGHT" ? 1 : 0
                        }
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
//                    jsh.loadAchievements(top_level.user, top_level.project)
                    currentIndex = -1
                }
            }
        }

        Rectangle {
            id: right_panel

            x: parent.width
            y: top_panel.height - top_panel.offset
            width: parent.width - left_panel.width
            height: parent.height - top_panel.height
            clip: true

            color: "#fff"

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

            LabeledTextField {
                id: block_name
                width: parent.width / 2
                anchors.top: parent.top
                anchors.topMargin: g_margin
                x: g_margin
                height: 80
                t_label.text: "Название"
            }

            LabeledTextArea {
                id: block_description
                anchors.top: block_name.bottom
                anchors.topMargin: g_margin
                anchors.right: parent.right
                anchors.rightMargin: g_margin
                height: 200
                width: parent.width - 20
                t_label.text: "Описание"
            }

            TAreaWithLabel {
                id: text_condition

                height: 200
                width: parent.width - 20
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: block_description.bottom
                anchors.topMargin: 10
                anchors.bottom: parent.bottom
                anchors.bottomMargin: g_margin
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
                reversible: true
                ParallelAnimation {
                    NumberAnimation {properties: "visible, opacity,y,x, width, contentX, contentY, height"; duration: top_level.animation_duration; easing.type: Easing.InOutExpo }
                    ColorAnimation {target: top_panel; from: "#00BCD4"; to: "#03A9F4"; duration: top_level.animation_duration}
                }
            }
        ]
    }
}
