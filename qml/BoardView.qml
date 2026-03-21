import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    signal createPostRequested()

    ListModel { id: postModel }

    function refreshPosts() {
        postModel.clear()
        if (!yolo) return
        var posts = yolo.getPosts(50)
        for (var i = 0; i < posts.length; i++) {
            postModel.append(posts[i])
        }
    }

    Component.onCompleted: refreshPosts()

    Connections {
        target: yolo
        function onPostsChanged() { refreshPosts() }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 8
            Layout.bottomMargin: 4

            Label {
                text: yolo ? yolo.currentBoard : ""
                font.pixelSize: 18
                font.bold: true
                color: "#212121"
                Layout.fillWidth: true
            }

            Button {
                text: "Refresh"
                flat: true
                font.pixelSize: 13
                onClicked: refreshPosts()
            }

            Button {
                text: "+ New Post"
                font.pixelSize: 13
                onClicked: root.createPostRequested()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#E0E0E0"
        }

        // Post list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: postModel
            clip: true
            spacing: 0

            delegate: Rectangle {
                width: ListView.view ? ListView.view.width : 0
                height: postContent.implicitHeight + 24
                color: "#FFFFFF"

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#EEEEEE"
                }

                ColumnLayout {
                    id: postContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 4

                    Label {
                        text: model.title || "(untitled)"
                        font.pixelSize: 15
                        font.bold: true
                        color: "#212121"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: model.author || "anonymous"
                            font.pixelSize: 12
                            color: "#5C6BC0"
                        }

                        Label {
                            text: {
                                if (!model.timestamp) return ""
                                var d = new Date(Number(model.timestamp))
                                return Qt.formatDateTime(d, "yyyy-MM-dd hh:mm")
                            }
                            font.pixelSize: 12
                            color: "#9E9E9E"
                        }

                        Item { Layout.fillWidth: true }

                        Label {
                            visible: (model.cid || "") !== ""
                            text: "CID"
                            font.pixelSize: 10
                            color: "#BDBDBD"
                            padding: 2
                        }
                    }

                    Label {
                        text: {
                            var c = model.content || ""
                            if (c.indexOf("cid:") === 0) return "(content stored externally)"
                            return c.length > 200 ? c.substring(0, 200) + "..." : c
                        }
                        font.pixelSize: 13
                        color: "#424242"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        maximumLineCount: 3
                        elide: Text.ElideRight
                    }
                }
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                visible: postModel.count === 0
                text: "No posts yet.\nBe the first to post!"
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 14
                color: "#757575"
            }
        }
    }
}
