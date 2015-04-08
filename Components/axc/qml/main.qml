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
        property alias achType: ach_type.checked

        property string user: user_view.model.get(0).name
        property string project: 'Таблица умножения'
        property string req_delimiter: '&***'
        property string par_delimiter: '=***'
        property int lview_index: -1

        property var deletedElements: [];

        function updateProperties() {
            var index = lview.currentIndex
            if (index < 0) {
                top_level.state = ""
                return
            }
            var dict = xml_model.dict(lview.currentIndex)
            top_level.nameText = dict[f_name]
            top_level.descriptionText = dict[f_description]
            top_level.conditionText = dict[f_condition]
            top_level.idLabel = dict[f_id] ? dict[f_id] : ""
            top_level.achType = dict[f_type]
        }

        TopPanel {
            id: top_panel
            width: parent.width
            header.height: 60
            achievements.height: 400
            height: achievements.height + header.height + offset
            y: -achievements.height
            clip: true
            z: 11
            color: "#00BCD4"

            onRemoveRequested: {
                var index = lview.currentIndex
                //change index if last element in list
                var move_up = (index === xml_model.count() - 1)
                var dct = xml_model.remove(index)
                Jsh.registerRemoved(dct)
                if (move_up) lview.currentIndex = index - 1
                top_level.updateProperties()
            }

            onSaveRequested: {
               Jsh.updateModel(top_level.lview_index)
               Jsh.save_achievemets()
            }
            onMoveRightRequested: {
                top_panel.state = top_panel.state === "SHOW_STATISTICS" ? "" : "SHOW_STATISTICS"
            }

            states: [
                State {
                    name: "SHOW_STATISTICS"
                    PropertyChanges { target: top_panel; y: 0 - offset}
                }
            ]
            transitions: [
                Transition {
                    NumberAnimation {targets: [top_panel]; properties: "x, y"; duration: top_level.animation_duration; easing.type: Easing.InOutExpo}
                }
            ]
        }

        Rectangle {
            id: left_panel
            color: "white"
            x: 0
            y: top_panel.header.height
            width: parent.width
            height: parent.height - top_panel.header.height

            ListView {
                id: lview
                anchors.fill: parent
                clip: true
                model: xml_model
                anchors.topMargin: 10

                highlightMoveDuration: 75
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
                        color: Type == Jsh.AE_TYPE_INSTANT ? "#EFFB41" : "#000"
                        Text {
                            anchors.centerIn: parent
                            color: "#303030"
                            text: id ? id.toString() : ""
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
                onCurrentIndexChanged: {
                    Jsh.updateModel(top_level.lview_index)
                    console.log("Current index changed to: " + currentIndex)
                    top_level.lview_index = currentIndex
                }
            }
        }

        Rectangle {
            id: right_panel

            x: parent.width
            y: top_panel.header.height
            width: parent.width - left_panel.width
            height: parent.height - top_panel.header.height
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

            CheckBox {
                id: ach_type
                anchors.left: block_name.right
                anchors.leftMargin: 20
                anchors.top: parent.top
                anchors.topMargin: block_name.g_margin
                text: "Мгновенное"
                checked: true
                onCheckedChanged: {
                    console.log("Checked to " + checked)
                }
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
