import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property var posts: []
    property string currentPrefix: 
    property string statusMsg: 

    Rectangle { anchors.fill: parent; color: "#171717" }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 12

        Text { text: "YOLO – Your Own Local Opinion"; color: "#FF8800"; font.pixelSize: 20; font.bold: true; font.family: "monospace" }

        Rectangle {
            Layout.fillWidth: true; height: boardRow.implicitHeight + 24; radius: 8
            color: "#1C1C1C"; border.color: "#2B303B"; border.width: 1

            RowLayout {
                id: boardRow
                anchors { fill: parent; margins: 12 }; spacing: 8
                TextField {
                    id: prefixField; Layout.fillWidth: true
                    placeholderText: "Board prefix"; color: "#FFF"; font.family: "monospace"
                    background: Rectangle { color: "#262626"; border.color: "#2B303B"; border.width: 1; radius: 4 }
                }
                TextField {
                    id: pubkeyField; Layout.fillWidth: true
                    placeholderText: "Your pubkey"; color: "#FFF"; font.family: "monospace"
                    background: Rectangle { color: "#262626"; border.color: "#2B303B"; border.width: 1; radius: 4 }
                }
                Button {
                    text: "Create"
                    onClicked: {
                        var r = JSON.parse(yoloModule.createBoard("YOLO Board", pubkeyField.text))
                        if (r.success) { currentPrefix = r.prefix; prefixField.text = r.prefix; refreshPosts(); statusMsg = "Board: " + r.prefix }
                    }
                    contentItem: Text { text: parent.text; color: "#171717"; font.bold: true; font.family: "monospace"; horizontalAlignment: Text.AlignHCenter }
                    background: Rectangle { color: parent.hovered ? "#E07A00" : "#FF8800"; radius: 4 }
                }
                Button {
                    text: "Load"
                    onClicked: { currentPrefix = prefixField.text; yoloModule.followBoard(currentPrefix); refreshPosts() }
                    contentItem: Text { text: parent.text; color: "#FF8800"; font.family: "monospace"; horizontalAlignment: Text.AlignHCenter }
                    background: Rectangle { color: parent.hovered ? "#262626" : "#1C1C1C"; border.color: "#FF8800"; border.width: 1; radius: 4 }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true; height: postCol.implicitHeight + 24; radius: 8
            color: "#1C1C1C"; border.color: "#2B303B"; border.width: 1
            visible: currentPrefix.length > 0
            ColumnLayout {
                id: postCol; anchors { fill: parent; margins: 12 }; spacing: 8
                TextField { id: titleField; Layout.fillWidth: true; placeholderText: "Title"; color: "#FFF"; font.family: "monospace"
                    background: Rectangle { color: "#262626"; border.color: "#2B303B"; border.width: 1; radius: 4 } }
                TextArea { id: contentField; Layout.fillWidth: true; Layout.minimumHeight: 60; placeholderText: "Speak your mind..."; color: "#FFF"; font.family: "monospace"; wrapMode: TextEdit.Wrap
                    background: Rectangle { color: "#262626"; border.color: "#2B303B"; border.width: 1; radius: 4 } }
                Button {
                    text: "Post"; Layout.alignment: Qt.AlignRight
                    enabled: titleField.text.length > 0 && contentField.text.length > 0
                    onClicked: {
                        var r = JSON.parse(yoloModule.postOpinion(currentPrefix, pubkeyField.text, titleField.text, contentField.text))
                        if (r.success) { titleField.text = ""; contentField.text = ""; refreshPosts(); statusMsg = "Posted!" }
                    }
                    contentItem: Text { text: parent.text; color: "#171717"; font.bold: true; font.family: "monospace"; horizontalAlignment: Text.AlignHCenter }
                    background: Rectangle { color: parent.enabled ? (parent.hovered ? "#E07A00" : "#FF8800") : "#333"; radius: 4 }
                }
            }
        }

        Text { visible: statusMsg.length > 0; text: statusMsg; color: "#4CAF50"; font.pixelSize: 11; font.family: "monospace" }

        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 8; clip: true; model: posts
            ScrollBar.vertical: ScrollBar {}
            delegate: Rectangle {
                width: ListView.view.width; height: dc.implicitHeight + 20; radius: 8
                color: "#1C1C1C"; border.color: "#2B303B"; border.width: 1
                Column { id: dc; anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }; spacing: 4
                    Text { text: modelData.title || ""; color: "#FFF"; font.bold: true; font.pixelSize: 14; font.family: "monospace" }
                    Text { text: modelData.content || ""; color: "#CCC"; font.pixelSize: 13; font.family: "monospace"; wrapMode: Text.Wrap; width: parent.width }
                    Text { text: (modelData.author || "").substring(0,8) + "..."; color: "#FF8800"; font.pixelSize: 11; font.family: "monospace" }
                }
            }
            Text { anchors.centerIn: parent; visible: posts.length === 0 && currentPrefix.length > 0; text: "No opinions yet."; color: "#666"; font.family: "monospace" }
        }
    }

    function refreshPosts() {
        if (!currentPrefix) return
        var r = JSON.parse(yoloModule.getPosts(currentPrefix))
        if (r.success) posts = r.posts
    }
    Timer { interval: 5000; running: currentPrefix.length > 0; repeat: true; onTriggered: refreshPosts() }
}
