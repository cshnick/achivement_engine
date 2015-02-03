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

}
