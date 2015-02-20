import QtQuick 2.0

SequentialAnimation {
    id: color_animation
    property string color: "#4CAF50"
    property int animation_duration: 200
    property int  pause_duration: 1000
    ColorAnimation {
        properties: "color"
        duration: 200
        target: top_panel
        from: top_panel.color
        to: color_animation.color
    }
    PauseAnimation {
        duration: 1000
    }
    ColorAnimation {
        properties: "color"
        duration: 200
        target: top_panel
        from: color_animation.color
        to: top_panel.color
    }
}

