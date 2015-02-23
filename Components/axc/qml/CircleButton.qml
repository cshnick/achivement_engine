import QtQuick 2.0

Circle {
    id: button

    property alias text: inner_text.text
    property alias font: inner_text.font
    property alias textColor: inner_text.color
    property int explicitWidth: button.width
    property int explicitHeight: button.height
    signal clicked();

    anchors.verticalCenter: parent.verticalCenter

    height: explicitHeight
    width: explicitWidth
    color: "white"
    Text {
        id: inner_text
        anchors.centerIn: parent
        font.pointSize: 12
        color: "black"
    }
    MouseArea {
        id: mouse_area

        anchors.fill: parent
        onClicked: {
            console.log("Circle button clicked")
            button.clicked()
        }
    }

    states: [
        State {
            name: "Pushed"
            when: mouse_area.pressed
            PropertyChanges {target: button; explicit: true; width: width - 5 }
            PropertyChanges {target: button; explicit: true; height: height - 5 }
        }
        ,State {
            name: "Visible"
            when: button.opacity === 1
            PropertyChanges {target: button; explicit: true; height: button.explicitHeight; width: button.explicitWidth }
            PropertyChanges {target: inner_text; opacity: 1 }
        }
        ,State {
            name: "InVisible"
            when: button.opacity === 0
            PropertyChanges {target: button; explicit: true; height: 0; width:0}
            PropertyChanges {target: inner_text; opacity: 0 }
        }
    ]

    transitions: [
        Transition {
            reversible: true
            to: "Pushed"

            NumberAnimation {
                target: button
                properties: "width,height"
                duration: 50
                easing.type: Easing.InOutQuad
            }
        }
        ,Transition {
            reversible: true
            to: "Visible"

            NumberAnimation {
                target: button
                properties: "width, height"
                duration: top_level.animation_duration
                easing.type: Easing.OutQuart
            }
        }
        ,Transition {
            reversible: true
            to: "InVisible"

            NumberAnimation {
                target: button
                properties: "width, height"
                duration: top_level.animation_duration
                easing.type: Easing.OutQuart
            }
            NumberAnimation {
                target: inner_text
                properties: "opacity"
                duration: top_level.animation_duration
                easing.type: Easing.OutQuart
            }
        }
    ]
}
