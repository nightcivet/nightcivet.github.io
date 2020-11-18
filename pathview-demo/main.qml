import QtQuick 2.11

Rectangle {
    width:600;
    height:600;
    id:root;
    color: "black"

    ListModel {
        id:datamodel;
        ListElement {name:"blala小魔仙";}
        ListElement {name:"晴明大人";}
        ListElement {name:"我妻有乃";}
        ListElement {name:"米豆子";}
    }

    PathView {
        anchors.fill:parent;
        delegate:mydelegate;
        model:datamodel;
        pathItemCount:3
        path:pathview_path

    }

    Component {
        id:mydelegate;
        Rectangle {
            width:100;
            height:100;
            color:"white";
            opacity: 0.9
            Text {
                text:name;
            }
        }
    }

    Path{
        id: pathview_path
        startX: 100;
        startY: root.height / 2;

        PathLine {x:root.width / 2;y:root.height / 2;}
        PathPercent {value:0.5}
        PathLine {x:root.width - 50;y:root.height / 2;}
        PathPercent {value:1}
    }
}
