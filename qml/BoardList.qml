import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    signal boardSelected(string name)

    property bool showNewBoardForm: false

    ListModel { id: boardModel }

    function refreshBoards() {
        boardModel.clear()
        if (!yolo) return
        var boards = yolo.discoverBoards()
        for (var i = 0; i < boards.length; i++) {
            boardModel.append(boards[i])
        }
    }

    Component.onCompleted: refreshBoards()

    Connections {
        target: yolo
        function onBoardsChanged() { refreshBoards() }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Title row
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Boards"
                font.pixelSize: 20
                font.bold: true
                color: "#212121"
                Layout.fillWidth: true
            }

            Button {
                text: showNewBoardForm ? "Cancel" : "+ New Board"
                flat: true
                font.pixelSize: 13
                onClicked: showNewBoardForm = !showNewBoardForm
            }

            Button {
                text: "Refresh"
                flat: true
                font.pixelSize: 13
                onClicked: refreshBoards()
            }
        }

        // New board form (inline)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? newBoardLayout.implicitHeight + 24 : 0
            visible: showNewBoardForm
            color: "#FFFFFF"
            border.color: "#E0E0E0"
            border.width: 1
            radius: 4

            ColumnLayout {
                id: newBoardLayout
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                TextField {
                    id: newBoardName
                    Layout.fillWidth: true
                    placeholderText: "Board name"
                    font.pixelSize: 14
                }

                TextField {
                    id: newBoardDesc
                    Layout.fillWidth: true
                    placeholderText: "Description (optional)"
                    font.pixelSize: 14
                }

                Button {
                    text: "Create Board"
                    enabled: newBoardName.text.trim().length > 0
                    Layout.alignment: Qt.AlignRight
                    onClicked: {
                        var name = newBoardName.text.trim()
                        var desc = newBoardDesc.text.trim()
                        yolo.createNewBoard(name, desc)
                        newBoardName.text = ""
                        newBoardDesc.text = ""
                        showNewBoardForm = false
                        root.boardSelected(name)
                    }
                }
            }
        }

        // Board list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: boardModel
            clip: true
            spacing: 4

            delegate: Rectangle {
                width: ListView.view ? ListView.view.width : 0
                height: 60
                color: delegateMouse.containsMouse ? "#F5F5F5" : "#FFFFFF"
                border.color: "#E0E0E0"
                border.width: 1
                radius: 4

                MouseArea {
                    id: delegateMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.boardSelected(model.name)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: model.name || model.channelId
                            font.pixelSize: 15
                            font.bold: true
                            color: "#212121"
                        }

                        Label {
                            text: (model.inscriptionCount || 0) + " posts"
                            font.pixelSize: 12
                            color: "#757575"
                        }
                    }

                    Label {
                        text: "\u203A"
                        font.pixelSize: 24
                        color: "#BDBDBD"
                    }
                }
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                visible: boardModel.count === 0
                text: "No boards discovered.\nCreate one to get started."
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 14
                color: "#757575"
            }
        }
    }
}
