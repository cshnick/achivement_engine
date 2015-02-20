import QtQuick 2.0

Circle {
    id: button

    property alias text: inner_text.text
    property alias font: inner_text.font
    property alias textColor: inner_text.color
    signal clicked();

    anchors.verticalCenter: parent.verticalCenter
    height: 0
    width: 0
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

    Behavior on width {
        NumberAnimation {
            id: size_animation
            duration: top_level.animation_duration
            easing.type: "OutQuart"

        }
    }
    Behavior on height {
        animation: size_animation
    }

    states: [
        State {
            name: "Pushed"
            when: mouse_area.pressed
            PropertyChanges {target: button; explicit: true; width: width - 5 }
            PropertyChanges {target: button; explicit: true; height: height - 5 }
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
        },
        Transition {
            animations: [size_animation,]
            reversible: true
        }
    ]

}
