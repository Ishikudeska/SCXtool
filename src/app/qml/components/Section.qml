import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import ScApp

Rectangle {
    id: section

    property string title: ""
    property string description: ""
    property bool collapsible: false
    property bool collapsed: false
    property string collapsedNote: ""
    default property alias content: body.data
    signal toggleRequested()

    implicitHeight: column.implicitHeight + 28
    color: Theme.panel
    border.color: Theme.border
    radius: 6

    ColumnLayout {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            visible: section.title !== ""
            spacing: 8
            Label {
                visible: section.collapsible
                text: section.collapsed ? "▸" : "▾"
                color: Theme.dim
                font.pixelSize: 15
            }
            Label {
                text: section.title
                color: Theme.title
                font.pixelSize: 15
                font.bold: true
            }
            Label {
                visible: section.collapsible && section.collapsed && section.collapsedNote !== ""
                text: section.collapsedNote
                color: Theme.placeholder
                font.pixelSize: Theme.smallFont + 1
            }
            Item { Layout.fillWidth: true }
            TapHandler {
                enabled: section.collapsible
                onTapped: section.toggleRequested()
            }
            HoverHandler {
                enabled: section.collapsible
                cursorShape: Qt.PointingHandCursor
            }
        }
        Label {
            text: section.description
            color: Theme.dim
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            visible: text !== "" && !section.collapsed
            font.pixelSize: Theme.smallFont + 1
        }
        ColumnLayout {
            id: body
            Layout.fillWidth: true
            visible: !section.collapsed
            spacing: 8
        }
    }
}
