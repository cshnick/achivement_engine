import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
    id: top_item
    property alias color: rect.color
    property alias shadowRadius: rectShadow.radius
    width: 320
    height: 240

    Item {
        id: container;
        anchors.centerIn: parent;
        width:  top_item.width
        height: top_item.height

        Rectangle {
            id: rect
            //width: container.width - (2 * rectShadow.radius)
            //height: container.height - (2 * rectShadow.radius)
            width: container.width
            height: container.height - (2 * rectShadow.radius)
            color: "orange";
            antialiasing: true;
            anchors.centerIn: parent;
        }
    }
    DropShadow {
        id: rectShadow;
        anchors.fill: source
        cached: true;
//        horizontalOffset: 3;
        verticalOffset: 3;
        radius: 8.0;
        samples: 16;
        color: "#80000000";
        smooth: true;
        source: container;
    }
}
