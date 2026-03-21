import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 800
    height: 600

    // The C++ backend is available as "yolo" (set via setContextProperty).

    Rectangle {
        anchors.fill: parent
        color: "#FAFAFA"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Header bar ──────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "#5C6BC0"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                ToolButton {
                    visible: stackView.depth > 1
                    contentItem: Label {
                        text: "\u2190"
                        font.pixelSize: 20
                        color: "#FFFFFF"
                    }
                    background: Item {}
                    onClicked: stackView.pop()
                }

                Label {
                    text: {
                        if (stackView.depth <= 1) return "YOLO Board"
                        if (stackView.currentItem && stackView.currentItem.pageTitle)
                            return stackView.currentItem.pageTitle
                        if (yolo && yolo.currentBoard) return yolo.currentBoard
                        return "YOLO Board"
                    }
                    font.pixelSize: 18
                    font.bold: true
                    color: "#FFFFFF"
                    Layout.fillWidth: true
                }

                ToolButton {
                    contentItem: Label {
                        text: eventPanel.visible ? "Hide Log" : "Log"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                    }
                    background: Item {}
                    onClicked: eventPanel.visible = !eventPanel.visible
                }
            }
        }

        // ── Error banner ────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 36 : 0
            color: "#FFEBEE"
            visible: yolo && yolo.errorMessage !== ""
            border.color: "#E53935"
            border.width: 1

            Behavior on Layout.preferredHeight { NumberAnimation { duration: 150 } }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                Label {
                    text: yolo ? yolo.errorMessage : ""
                    color: "#E53935"
                    font.pixelSize: 13
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                ToolButton {
                    contentItem: Label {
                        text: "\u2715"
                        color: "#E53935"
                        font.pixelSize: 14
                    }
                    background: Item {}
                    onClicked: yolo.clearError()
                }
            }
        }

        // ── Main content (StackView) ────────────────────────────────────
        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: boardListPage
        }

        // ── Separator ───────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#E0E0E0"
            visible: eventPanel.visible
        }

        // ── Event log panel (bottom) ────────────────────────────────────
        EventLog {
            id: eventPanel
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            visible: false
        }
    }

    // ── Page components ─────────────────────────────────────────────────

    Component {
        id: boardListPage
        BoardList {
            onBoardSelected: function(name) {
                yolo.selectBoard(name)
                stackView.push(boardViewPage)
            }
        }
    }

    Component {
        id: boardViewPage
        BoardView {
            property string pageTitle: yolo ? yolo.currentBoard : ""
            onCreatePostRequested: stackView.push(createPostPage)
        }
    }

    Component {
        id: createPostPage
        CreatePost {
            property string pageTitle: "New Post"
            onPostSubmitted: stackView.pop()
            onCancelled: stackView.pop()
        }
    }
}
