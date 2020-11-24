import QtQuick 2.7
import QtQuick.Controls 2.2

Rectangle {
    width:800;
    height:600;
    id:root
    color:"black"

    PathView{
        id:pathView
        delegate: mydelegate;
        model: datamodel;

        path: pathview_path
        pathItemCount: datamodel.itemCount
        anchors.fill: parent

        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5

        focus: true;
        Keys.onLeftPressed: decrementCurrentIndex();
        Keys.onRightPressed: incrementCurrentIndex();
        Keys.onTabPressed: incrementCurrentIndex();

    }

    Text
    {
        anchors.left: pathView.left
        anchors.leftMargin: root.width/5

        anchors.bottom: root.bottom
        anchors.bottomMargin: 120

        width: pathView.width*3/5
        height: 80
        opacity: 0.7

        text: pathView.currentItem.infoText
        color: "white"
        font.pixelSize: 22
        wrapMode: Text.WrapAnywhere
    }

    Path{
        id:pathview_path

        startX: root.width*1/7
        startY: root.height/3
        PathAttribute{name:"picZ";value: 0}
        PathAttribute{name:"picAngle";value: 50}
        PathAttribute{name:"picScale";value: 0.6}

        PathLine{x:root.width/2;y:root.height/3}
        PathAttribute{name:"picZ";value: 100}
        PathAttribute{name:"picAngle";value: 0}
        PathAttribute{name:"picScale";value: 1.0}

        PathLine{x:root.width*6/7;y:root.height/3}
        PathAttribute{name:"picZ";value: 0}
        PathAttribute{name:"picAngle";value: -50}
        PathAttribute{name:"picScale";value: 0.6}
    }

    Component {
        id:mydelegate

        Item{
            width: 200
            height: 200
            z:PathView.picZ
            scale:PathView.picScale
            property int angle1: PathView.picAngle
            property string infoText: info

            Image{
                id:image
                antialiasing: true
                source: url
                width: mydelegate.width
                height: mydelegate.height
            }

            ShaderEffect {
                id: effectImage
                anchors.top: image.bottom
                width: image.width
                height: image.height;
                anchors.left: image.left
                property variant source: image;
                property size sourceSize: Qt.size(0.5 / image.width, 0.5 / image.height);
                fragmentShader:
                        "varying highp vec2 qt_TexCoord0;
                        uniform lowp sampler2D source;
                        uniform lowp vec2 sourceSize;
                        uniform lowp float qt_Opacity;
                        void main() {

                            lowp vec2 tc = qt_TexCoord0 * vec2(1, -1) + vec2(0, 1);
                            lowp vec4 col = 0.25 * (texture2D(source, tc + sourceSize) + texture2D(source, tc- sourceSize)
                            + texture2D(source, tc + sourceSize * vec2(1, -1))
                            + texture2D(source, tc + sourceSize * vec2(-1, 1)));
                            gl_FragColor = col * qt_Opacity * (1.0 - qt_TexCoord0.y) * 0.2;
                        }"
            }

            transform: Rotation{
                origin.x:image.width/2.0
                origin.y:image.height/2.0
                axis{x:0;y:1;z:0}
                angle: angle1
            }

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    pathView.currentIndex = index;
                }

            }
        }
    }
}

