import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/*
 * Self-contained demo of the YOLO Board UI with mock data.
 * Used for screenshot capture without the C++ backend.
 */
ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "YOLO Board – Logos Core Module"

    Rectangle {
        anchors.fill: parent
        color: "#FAFAFA"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Header bar ────────────────────────────────────────────────
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
                    contentItem: Label {
                        text: "\u2190"
                        font.pixelSize: 20
                        color: "#FFFFFF"
                    }
                    background: Item {}
                }

                Label {
                    text: "ETHCluj General"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#FFFFFF"
                    Layout.fillWidth: true
                }

                ToolButton {
                    contentItem: Label {
                        text: "Hide Log"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                    }
                    background: Item {}
                }
            }
        }

        // ── Board View: Posts ──────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Toolbar
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 8
                Layout.bottomMargin: 4

                Label {
                    text: "ETHCluj General"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#212121"
                    Layout.fillWidth: true
                }

                Button {
                    text: "Refresh"
                    flat: true
                    font.pixelSize: 13
                }

                Button {
                    text: "+ New Post"
                    font.pixelSize: 13
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#E0E0E0"
            }

            // Post list (hardcoded mock data)
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 0

                model: ListModel {
                    ListElement {
                        title: "Welcome to YOLO Board!"
                        author: "alice"
                        ts: "2026-03-21 09:15"
                        content: "This is the first post on our censorship-resistant message board. YOLO uses Logos pipe federation to sync posts across all nodes in the network. Every inscription is cryptographically signed and content-addressed."
                        hasCid: false
                    }
                    ListElement {
                        title: "Federation test – multi-admin merge"
                        author: "bob"
                        ts: "2026-03-21 09:22"
                        content: "Successfully merged posts from two independent admin channels. The FederatedChannel automatically discovers and indexes inscriptions from all writers. Content larger than 1KB is offloaded to the ContentStore and referenced by CID."
                        hasCid: true
                    }
                    ListElement {
                        title: "Building decentralised social"
                        author: "charlie"
                        ts: "2026-03-21 09:30"
                        content: "Logos core modules enable censorship-resistant applications on top of the Logos network stack. The YOLO board demonstrates the full pipeline: create board → post message → federate → read from any node."
                        hasCid: false
                    }
                    ListElement {
                        title: "Workshop notes: ETHCluj hackathon"
                        author: "alice"
                        ts: "2026-03-21 10:05"
                        content: "Key takeaways from the workshop session: 1) Module architecture separates headless logic from UI, 2) The event pipeline provides real-time feedback, 3) Federation is automatic once channels are joined, 4) ContentStore handles large payload offloading."
                        hasCid: true
                    }
                }

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
                            text: model.title
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
                                text: model.author
                                font.pixelSize: 12
                                color: "#5C6BC0"
                            }

                            Label {
                                text: model.ts
                                font.pixelSize: 12
                                color: "#9E9E9E"
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                visible: model.hasCid
                                text: "CID"
                                font.pixelSize: 10
                                color: "#BDBDBD"
                                padding: 2
                            }
                        }

                        Label {
                            text: model.content.length > 200 ? model.content.substring(0, 200) + "..." : model.content
                            font.pixelSize: 13
                            color: "#424242"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            maximumLineCount: 3
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        // ── Separator ─────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#E0E0E0"
        }

        // ── Event log panel (bottom) ──────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "#263238"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    color: "#1B2327"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8

                        Label {
                            text: "Event Log"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#B0BEC5"
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "6 events"
                            font.pixelSize: 11
                            color: "#78909C"
                        }

                        ToolButton {
                            contentItem: Label {
                                text: "Clear"
                                color: "#78909C"
                                font.pixelSize: 11
                            }
                            background: Item {}
                        }
                    }
                }

                // Event list
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 0

                    model: ListModel {
                        ListElement { time: "09:15:01"; eventType: "info";    message: "Board 'ETHCluj General' created" }
                        ListElement { time: "09:15:02"; eventType: "success"; message: "Channel joined: yolo/ethcluj-general" }
                        ListElement { time: "09:15:03"; eventType: "success"; message: "Post inscribed: 'Welcome to YOLO Board!' (142 bytes)" }
                        ListElement { time: "09:22:10"; eventType: "info";    message: "Federation: discovered writer bob on channel yolo/ethcluj-general" }
                        ListElement { time: "09:22:11"; eventType: "success"; message: "Post inscribed: 'Federation test' → CID bafybeig..." }
                        ListElement { time: "09:30:05"; eventType: "success"; message: "Post inscribed: 'Building decentralised social' (287 bytes)" }
                    }

                    delegate: Rectangle {
                        width: ListView.view ? ListView.view.width : 0
                        height: eventRow.implicitHeight + 4
                        color: index % 2 === 0 ? "transparent" : "#1E2A30"

                        RowLayout {
                            id: eventRow
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            Label {
                                text: model.time
                                font.pixelSize: 11
                                font.family: "monospace"
                                color: "#78909C"
                            }

                            Rectangle {
                                width: 6
                                height: 6
                                radius: 3
                                color: {
                                    if (model.eventType === "success") return "#43A047"
                                    if (model.eventType === "error") return "#E53935"
                                    return "#1565C0"
                                }
                            }

                            Label {
                                text: model.message
                                font.pixelSize: 12
                                font.family: "monospace"
                                color: {
                                    if (model.eventType === "success") return "#81C784"
                                    if (model.eventType === "error") return "#EF9A9A"
                                    return "#B0BEC5"
                                }
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}
