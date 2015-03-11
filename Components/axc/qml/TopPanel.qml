import QtQuick 2.0
import "../js/helper.js" as Jsh

ShadowRect {
    property int offset: shadowRadius
    property alias header: header
    property alias achievements: achievements
    signal saveRequested()
    signal moveRightRequested()
    color: "#00BCD4"
    id: top_panel

    Item {
        id: achievements
        x: 0
        y: 0
        width: parent.width
        height: 400

        Rectangle {
            property int indent: 40
            id: bottom_line
            height: 1
            width: parent.width - indent * 2
            color: "#fff"
            y: achievements.height - 1
            x: indent
        }
    }

    Item {
        id: header
        y: parent.height - 60 - offset
        x: 0
        width: parent.width
        height: 60
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
                     moveRightRequested()
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
                    //                        dict["id"] = xml_model.getId()
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
                    saveRequested()
                }
            }
        }
    }
}
