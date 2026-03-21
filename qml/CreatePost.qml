import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    signal postSubmitted()
    signal cancelled()

    property bool submitting: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: "New Post"
            font.pixelSize: 20
            font.bold: true
            color: "#212121"
        }

        Label {
            text: "Board: " + (yolo ? yolo.currentBoard : "")
            font.pixelSize: 13
            color: "#757575"
        }

        TextField {
            id: titleField
            Layout.fillWidth: true
            placeholderText: "Post title"
            font.pixelSize: 15
            enabled: !submitting
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: contentField
                placeholderText: "Write your post content here..."
                font.pixelSize: 14
                wrapMode: TextEdit.Wrap
                enabled: !submitting
            }
        }

        Label {
            text: contentField.text.length + " characters"
            font.pixelSize: 11
            color: "#BDBDBD"
            Layout.alignment: Qt.AlignRight
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item { Layout.fillWidth: true }

            Button {
                text: "Cancel"
                flat: true
                enabled: !submitting
                onClicked: root.cancelled()
            }

            Button {
                text: submitting ? "Submitting..." : "Submit"
                enabled: titleField.text.trim().length > 0
                         && contentField.text.trim().length > 0
                         && !submitting
                onClicked: {
                    submitting = true
                    var result = yolo.submitPost(
                        titleField.text.trim(),
                        contentField.text.trim()
                    )
                    submitting = false
                    if (result && result.length > 0) {
                        root.postSubmitted()
                    }
                }
            }
        }
    }
}
