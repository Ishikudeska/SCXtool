pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import ScApp

Item {
    id: page

    property bool active: false
    readonly property bool ready: gs.status === "ready"
    readonly property bool settingsTab: gs.tab !== "keybinds"

    GameSettingsController {
        id: gs
        active: page.active
    }

    component SmallButton: AppButton {
        implicitHeight: 26
        font.pixelSize: Theme.smallFont + 1
    }

    component ResetButton: SmallButton {
        property bool shown: false
        implicitWidth: 34
        text: "↺"
        tip: qsTr("game_settings.reset_tip")
        opacity: shown ? 1 : 0
        enabled: shown
    }

    component NumberField: TextField {
        id: numberField
        property string committed: ""
        property bool numeric: true
        signal commit(string value)
        implicitWidth: 110
        text: committed
        selectByMouse: true
        validator: numeric ? numberRegex : null
        onEditingFinished: if (text !== committed) numberField.commit(text)
        RegularExpressionValidator { id: numberRegex; regularExpression: /^-?[0-9]*[.]?[0-9]*$/ }
    }

    component SettingRow: RowLayout {
        id: settingRow
        required property var base
        readonly property var row: gs.settingState(base.name, gs.revision)
        Layout.fillWidth: true
        spacing: 10

        Rectangle {
            implicitWidth: 6
            implicitHeight: 6
            radius: 3
            color: settingRow.row.pending ? Theme.action("needsApply") : settingRow.row.modified ? Theme.em4 : "transparent"
            ToolTip.visible: dotArea.containsMouse && (settingRow.row.pending || settingRow.row.modified)
            ToolTip.text: settingRow.row.pending ? qsTr("game_settings.pending_tip") : qsTr("game_settings.modified_tip")
            MouseArea { id: dotArea; anchors.fill: parent; anchors.margins: -4; hoverEnabled: true }
        }
        Label {
            id: settingLabel
            Layout.preferredWidth: Math.max(220, settingRow.width * 0.42)
            Layout.maximumWidth: Math.max(220, settingRow.width * 0.42)
            text: settingRow.row.label + (settingRow.row.restart ? "  ⟳" : "")
            color: Theme.text
            wrapMode: Text.WordWrap
            ToolTip.visible: labelArea.containsMouse
            ToolTip.delay: 500
            ToolTip.text: [settingRow.row.description,
                           App.fmt(qsTr("game_settings.setting_tip"), { name: settingRow.row.name, cvar: settingRow.row.cvar || "—" }),
                           settingRow.row.restart ? qsTr("game_settings.restart_tip") : ""].filter(s => s !== "").join("\n\n")
            MouseArea { id: labelArea; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
        }
        Loader {
            Layout.fillWidth: true
            sourceComponent: settingRow.row.kind === "toggle" ? toggleEditor
                           : settingRow.row.kind === "slider" ? sliderEditor
                           : settingRow.row.kind === "choice" ? choiceEditor : fieldEditor
            Component {
                id: toggleEditor
                AppCheckBox {
                    text: checked ? qsTr("game_settings.on") : qsTr("game_settings.off")
                    checked: settingRow.row.checked
                    enabled: page.ready
                    onToggled: gs.setToggle(settingRow.row.name, checked)
                }
            }
            Component {
                id: sliderEditor
                RowLayout {
                    spacing: 8
                    Slider {
                        id: slider
                        Layout.fillWidth: true
                        Layout.maximumWidth: 320
                        from: settingRow.row.min
                        to: settingRow.row.max
                        stepSize: settingRow.row.step
                        value: settingRow.row.number
                        enabled: page.ready
                        onPressedChanged: if (!pressed) gs.setValue(settingRow.row.name, String(value))
                    }
                    Label {
                        text: slider.value.toFixed(settingRow.row.decimals) + (settingRow.row.unit !== "" ? " " + settingRow.row.unit : "")
                        color: Theme.text
                        Layout.preferredWidth: 70
                    }
                    Item { Layout.fillWidth: true }
                }
            }
            Component {
                id: choiceEditor
                ComboBox {
                    implicitWidth: 260
                    model: settingRow.row.choices
                    textRole: "label"
                    currentIndex: settingRow.row.choiceIndex
                    enabled: page.ready
                    onActivated: index => gs.setValue(settingRow.row.name, settingRow.row.choices[index].value)
                }
            }
            Component {
                id: fieldEditor
                RowLayout {
                    spacing: 6
                    NumberField {
                        committed: settingRow.row.value
                        numeric: settingRow.row.kind !== "text"
                        implicitWidth: settingRow.row.kind === "text" ? 220 : 110
                        enabled: page.ready
                        onCommit: value => gs.setValue(settingRow.row.name, value)
                    }
                    Label { text: settingRow.row.unit; color: Theme.dim; visible: text !== "" }
                    Item { Layout.fillWidth: true }
                }
            }
        }
        Label {
            text: settingRow.row.defaultText !== "" ? App.fmt(qsTr("game_settings.default_value"), { value: settingRow.row.defaultText }) : ""
            color: Theme.placeholder
            font.pixelSize: Theme.smallFont
            Layout.preferredWidth: 150
            elide: Text.ElideRight
        }
        ResetButton {
            shown: page.ready && (settingRow.row.modified || settingRow.row.pending)
            onClicked: gs.resetValue(settingRow.row.name)
        }
    }

    component SettingSections: Repeater {
        model: gs.sections
        delegate: Section {
            id: sectionBox
            required property var modelData
            Layout.fillWidth: true
            title: modelData.title
            Repeater {
                model: sectionBox.modelData.rows
                delegate: SettingRow {
                    required property var modelData
                    base: modelData
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacing
        spacing: 8

        Label { text: qsTr("game_settings.title"); color: Theme.title; font.pixelSize: 20; font.bold: true }
        Label { text: qsTr("game_settings.desc"); color: Theme.dim; wrapMode: Text.WordWrap; Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: gs.status === "noinstall" || gs.status === "nodata" || gs.status === "error"
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.action("restore")
                text: gs.status === "noinstall" ? qsTr("game_settings.no_install")
                    : gs.status === "nodata" ? qsTr("game_settings.no_data")
                    : App.fmt(qsTr("game_settings.load_error"), { message: gs.error })
            }
            AppButton {
                visible: gs.status !== "noinstall"
                role: "load"
                text: gs.status === "nodata" ? qsTr("game_settings.extract_btn") : qsTr("game_settings.reload_btn")
                enabled: !App.busy
                onClicked: gs.status === "nodata" ? gs.extractGameData() : gs.reload()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            visible: gs.gameRunning
            implicitHeight: runningLabel.implicitHeight + 14
            radius: Theme.radius
            color: "transparent"
            border.color: Theme.action("needsApply")
            Label {
                id: runningLabel
                anchors.fill: parent
                anchors.margins: 7
                text: qsTr("game_settings.running_banner")
                color: Theme.text
                wrapMode: Text.WordWrap
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: page.ready
            AppButton {
                objectName: "gameSettingsSave"
                role: "apply"
                text: gs.pendingCount > 0 ? App.fmt(qsTr("game_settings.save_count_btn"), { count: gs.pendingCount })
                                          : qsTr("game_settings.save_btn")
                tip: qsTr("game_settings.save_tip")
                enabled: gs.pendingCount > 0 && !gs.gameRunning && !App.busy
                onClicked: gs.save()
            }
            AppButton {
                text: qsTr("game_settings.discard_btn")
                enabled: gs.pendingCount > 0
                onClicked: gs.discard()
            }
            AppButton {
                text: gs.tab === "keybinds" ? qsTr("game_settings.reset_device_btn") : qsTr("game_settings.reset_tab_btn")
                tip: gs.tab === "keybinds" ? qsTr("game_settings.reset_device_tip") : qsTr("game_settings.reset_tab_tip")
                onClicked: resetConfirm.open()
            }
            AppButton {
                role: "restore"
                text: qsTr("game_settings.backups_btn")
                onClicked: { backupList.model = gs.backups(); backupsPopup.open() }
            }
            AppButton {
                role: "open"
                text: qsTr("game_settings.open_folder_btn")
                tip: gs.profileDir
                onClicked: gs.openProfileFolder()
            }
            AppButton {
                text: qsTr("game_settings.reload_btn")
                enabled: !App.busy
                onClicked: gs.reload()
            }
            Item { Layout.fillWidth: true }
        }

        NavBar {
            Layout.fillWidth: true
            visible: page.ready
            pages: gs.tabs
            current: gs.tab
            onSelected: id => gs.tab = id
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: page.ready
            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: qsTr("game_settings.search_placeholder")
                text: gs.search
                onTextEdited: searchDelay.restart()
                Timer { id: searchDelay; interval: 200; onTriggered: gs.search = searchField.text }
            }
            Label { visible: gs.tab === "keybinds"; text: qsTr("game_settings.device_label"); color: Theme.text }
            ComboBox {
                visible: gs.tab === "keybinds"
                implicitWidth: 240
                model: gs.bindDevices
                textRole: "title"
                currentIndex: Math.max(0, gs.bindDevices.findIndex(d => d.id === gs.bindDevice))
                onActivated: index => gs.bindDevice = gs.bindDevices[index].id
            }
            Label { visible: gs.tab === "keybinds"; text: qsTr("game_settings.category_label"); color: Theme.text }
            ComboBox {
                visible: gs.tab === "keybinds"
                implicitWidth: 260
                model: [qsTr("game_settings.all_categories")].concat(gs.bindCategories)
                currentIndex: gs.bindCategory === "" ? 0 : gs.bindCategories.indexOf(gs.bindCategory) + 1
                onActivated: index => gs.bindCategory = index === 0 ? "" : gs.bindCategories[index - 1]
            }
            AppCheckBox {
                text: qsTr("game_settings.modified_only")
                checked: gs.modifiedOnly
                onToggled: gs.modifiedOnly = checked
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.ready
            color: Theme.base
            border.color: Theme.border
            radius: Theme.radius

            ScrollView {
                id: settingsScroll
                anchors.fill: parent
                anchors.margins: 8
                visible: page.settingsTab
                clip: true
                contentWidth: availableWidth

                ColumnLayout {
                    width: settingsScroll.availableWidth
                    spacing: 10

                    Section {
                        Layout.fillWidth: true
                        visible: gs.tab === "controls"
                        title: qsTr("game_settings.sections.input_devices")
                        description: qsTr("game_settings.input_devices_desc")
                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("game_settings.device_label"); color: Theme.text }
                            ComboBox {
                                implicitWidth: 360
                                model: gs.optionDevices
                                textRole: "title"
                                currentIndex: gs.optionDevice
                                onActivated: index => gs.optionDevice = index
                            }
                        }
                        Repeater {
                            model: gs.optionRows
                            delegate: RowLayout {
                                id: optionRow
                                required property var modelData
                                readonly property var row: gs.optionState(modelData.group, gs.revision)
                                readonly property bool heading: !modelData.showInvert && !modelData.showCurve
                                Layout.fillWidth: true
                                spacing: 10
                                Label {
                                    Layout.preferredWidth: 380
                                    leftPadding: optionRow.row.depth * 16
                                    text: optionRow.row.label
                                    color: optionRow.heading ? Theme.title : Theme.text
                                    font.bold: optionRow.heading
                                    elide: Text.ElideRight
                                    ToolTip.visible: optArea.containsMouse
                                    ToolTip.delay: 500
                                    ToolTip.text: optionRow.row.group
                                                  + (optionRow.row.invertCvar !== "" ? "\n" + App.fmt(qsTr("game_settings.invert_cvar_tip"), { cvar: optionRow.row.invertCvar }) : "")
                                    MouseArea { id: optArea; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
                                }
                                AppCheckBox {
                                    visible: !optionRow.heading
                                    opacity: optionRow.row.showInvert ? 1 : 0
                                    enabled: optionRow.row.showInvert
                                    text: qsTr("game_settings.invert")
                                    checked: optionRow.row.inverted
                                    onToggled: gs.setInvert(optionRow.row.group, checked)
                                }
                                RowLayout {
                                    visible: !optionRow.heading
                                    opacity: optionRow.row.showCurve ? 1 : 0
                                    enabled: optionRow.row.showCurve
                                    spacing: 4
                                    Label { text: qsTr("game_settings.exponent"); color: Theme.dim }
                                    NumberField {
                                        implicitWidth: 80
                                        committed: optionRow.row.exponent
                                        placeholderText: optionRow.row.defaultExponent
                                        onCommit: value => gs.setExponent(optionRow.row.group, value)
                                    }
                                    Label {
                                        visible: optionRow.row.hasCurve
                                        text: qsTr("game_settings.has_curve")
                                        color: Theme.placeholder
                                        font.pixelSize: Theme.smallFont
                                    }
                                }
                                Item { Layout.fillWidth: true }
                                ResetButton {
                                    shown: optionRow.row.modified
                                    onClicked: gs.resetOption(optionRow.row.group)
                                }
                            }
                        }
                        Label {
                            visible: gs.axisRows.length > 0
                            text: qsTr("game_settings.axes_heading")
                            color: Theme.title
                            font.bold: true
                            topPadding: 8
                        }
                        Repeater {
                            model: gs.axisRows
                            delegate: RowLayout {
                                id: axisRow
                                required property var modelData
                                readonly property var row: gs.axisState(modelData.input, gs.revision)
                                spacing: 10
                                Label { text: axisRow.row.label; color: Theme.text; Layout.preferredWidth: 200 }
                                Label { text: qsTr("game_settings.deadzone"); color: Theme.dim }
                                NumberField {
                                    implicitWidth: 80
                                    committed: axisRow.row.deadzone
                                    placeholderText: "0"
                                    onCommit: value => gs.setAxisOption(axisRow.row.input, "deadzone", value)
                                }
                                Label { text: qsTr("game_settings.saturation"); color: Theme.dim }
                                NumberField {
                                    implicitWidth: 80
                                    committed: axisRow.row.saturation
                                    placeholderText: "1"
                                    onCommit: value => gs.setAxisOption(axisRow.row.input, "saturation", value)
                                }
                            }
                        }
                    }

                    SettingSections {}

                    Label {
                        visible: gs.sections.length === 0 && gs.tab !== "controls"
                        text: qsTr("game_settings.no_matches")
                        color: Theme.placeholder
                    }
                }
            }

            ListView {
                id: bindList
                anchors.fill: parent
                anchors.margins: 4
                visible: !page.settingsTab
                clip: true
                model: gs.bindRows
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                delegate: Rectangle {
                    id: bindRow
                    required property var modelData
                    required property int index
                    readonly property var row: gs.bindState(modelData.map, modelData.action, gs.revision)
                    width: bindList.width
                    height: bindLine.implicitHeight + 10
                    color: index % 2 ? Theme.alternateBase : "transparent"
                    RowLayout {
                        id: bindLine
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 8
                        spacing: 10
                        Rectangle {
                            implicitWidth: 6
                            implicitHeight: 6
                            radius: 3
                            color: bindRow.row.modified ? Theme.em4 : "transparent"
                        }
                        ColumnLayout {
                            Layout.preferredWidth: 360
                            Layout.maximumWidth: 360
                            Layout.fillWidth: false
                            spacing: 1
                            Label {
                                Layout.fillWidth: true
                                text: bindRow.row.label
                                color: Theme.text
                                elide: Text.ElideRight
                                ToolTip.visible: bindArea.containsMouse
                                ToolTip.delay: 500
                                ToolTip.text: [bindRow.row.description,
                                               bindRow.row.map + " / " + bindRow.row.action].filter(s => s !== "").join("\n\n")
                                MouseArea { id: bindArea; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
                            }
                            Label {
                                Layout.fillWidth: true
                                text: bindRow.row.mapLabel
                                color: Theme.dim
                                font.pixelSize: Theme.smallFont
                                elide: Text.ElideRight
                            }
                        }
                        Label {
                            Layout.preferredWidth: 260
                            text: bindRow.row.binding !== "" ? bindRow.row.binding : qsTr("game_settings.unbound")
                            color: bindRow.row.binding !== "" ? Theme.text : Theme.placeholder
                            font.bold: bindRow.row.binding !== ""
                            elide: Text.ElideRight
                        }
                        Label {
                            Layout.fillWidth: true
                            text: bindRow.row.conflicts !== ""
                                  ? "⚠ " + App.fmt(qsTr("game_settings.conflict"), { actions: bindRow.row.conflicts }) : ""
                            color: Theme.action("restore")
                            font.pixelSize: Theme.smallFont
                            elide: Text.ElideRight
                        }
                        Label {
                            Layout.preferredWidth: 180
                            text: bindRow.row.modified
                                  ? App.fmt(qsTr("game_settings.default_value"),
                                            { value: bindRow.row.defaultText !== "" ? bindRow.row.defaultText : qsTr("game_settings.unbound") })
                                  : ""
                            color: Theme.placeholder
                            font.pixelSize: Theme.smallFont
                            elide: Text.ElideRight
                        }
                        SmallButton {
                            text: qsTr("game_settings.bind_btn")
                            onClicked: capture.openFor(bindRow.modelData)
                        }
                        SmallButton {
                            text: qsTr("game_settings.clear_btn")
                            enabled: bindRow.row.binding !== ""
                            onClicked: gs.unbind(bindRow.row.map, bindRow.row.action)
                        }
                        ResetButton {
                            shown: bindRow.row.modified
                            onClicked: gs.resetBinding(bindRow.row.map, bindRow.row.action)
                        }
                    }
                }
                Label {
                    anchors.centerIn: parent
                    visible: bindList.count === 0
                    text: qsTr("game_settings.no_matches")
                    color: Theme.placeholder
                }
            }
        }
    }

    Popup {
        id: resetConfirm
        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 20
        width: Math.min(520, page.width - 40)
        background: Rectangle { color: Theme.panel; border.color: Theme.action("restore"); radius: 6 }
        contentItem: ColumnLayout {
            spacing: 12
            Label {
                Layout.fillWidth: true
                text: gs.tab === "keybinds" ? qsTr("game_settings.reset_device_confirm") : qsTr("game_settings.reset_tab_confirm")
                color: Theme.text
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                AppButton { text: qsTr("game_settings.cancel_btn"); onClicked: resetConfirm.close() }
                AppButton {
                    role: "clear"
                    text: qsTr("game_settings.reset_confirm_btn")
                    onClicked: { gs.resetTab(); resetConfirm.close() }
                }
            }
        }
    }

    Popup {
        id: backupsPopup
        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 20
        width: Math.min(620, page.width - 40)
        background: Rectangle { color: Theme.panel; border.color: Theme.border; radius: 6 }
        contentItem: ColumnLayout {
            spacing: 10
            Label { text: qsTr("game_settings.backups_title"); color: Theme.title; font.bold: true; font.pixelSize: 16 }
            Label {
                Layout.fillWidth: true
                text: qsTr("game_settings.backups_desc")
                color: Theme.dim
                wrapMode: Text.WordWrap
            }
            Label {
                visible: backupList.count === 0
                text: qsTr("game_settings.no_backups")
                color: Theme.placeholder
            }
            ListView {
                id: backupList
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentHeight, 320)
                clip: true
                delegate: RowLayout {
                    id: backupRow
                    required property var modelData
                    width: backupList.width
                    spacing: 10
                    Label { text: backupRow.modelData.file; color: Theme.text; Layout.preferredWidth: 140 }
                    Label { text: backupRow.modelData.date; color: Theme.dim; Layout.fillWidth: true }
                    SmallButton {
                        role: "restore"
                        text: qsTr("game_settings.restore_btn")
                        enabled: !gs.gameRunning
                        onClicked: { backupsPopup.close(); gs.restoreBackup(backupRow.modelData.path) }
                    }
                }
            }
            AppButton {
                Layout.alignment: Qt.AlignRight
                text: qsTr("game_settings.close_btn")
                onClicked: backupsPopup.close()
            }
        }
    }

    Popup {
        id: capture

        property var target: ({})
        property string captured: ""
        property var held: []
        property bool sawKey: false
        readonly property bool keyboardDevice: gs.bindDevice === "keyboard"
        readonly property bool mouseDevice: gs.bindDevice === "mouse"
        readonly property bool listDevice: !keyboardDevice && !mouseDevice

        function openFor(row) {
            target = row
            captured = row.input
            held = []
            sawKey = false
            modeBox.currentIndex = 0
            tapBox.value = 1
            inputField.editText = row.input
            open()
            keyCatcher.forceActiveFocus()
        }
        function finish() {
            const input = listDevice ? inputField.editText.trim() : captured
            const mode = modeBox.currentIndex > 0 ? gs.activationModes[modeBox.currentIndex - 1] : ""
            gs.setBinding(target.map, target.action, input, mode, tapBox.value)
            close()
        }

        anchors.centerIn: Overlay.overlay
        modal: true
        closePolicy: Popup.NoAutoClose
        padding: 20
        width: Math.min(560, page.width - 40)
        background: Rectangle { color: Theme.panel; border.color: Theme.border; radius: 6 }

        contentItem: ColumnLayout {
            spacing: 12
            Label {
                text: capture.target.label || ""
                color: Theme.title
                font.bold: true
                font.pixelSize: 16
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                text: capture.keyboardDevice ? qsTr("game_settings.capture_keyboard")
                    : capture.mouseDevice ? qsTr("game_settings.capture_mouse") : qsTr("game_settings.capture_list")
                color: Theme.dim
                wrapMode: Text.WordWrap
            }
            Rectangle {
                id: keyCatcher
                visible: !capture.listDevice
                Layout.fillWidth: true
                implicitHeight: 70
                radius: Theme.radius
                color: Theme.base
                border.color: activeFocus ? Theme.highlight : Theme.border
                focus: true
                Label {
                    anchors.centerIn: parent
                    text: capture.captured !== "" ? gs.describeInput(capture.captured) : qsTr("game_settings.capture_waiting")
                    color: capture.captured !== "" ? Theme.title : Theme.placeholder
                    font.pixelSize: 16
                    font.bold: capture.captured !== ""
                }
                Keys.onPressed: event => {
                    event.accepted = true
                    if (event.isAutoRepeat)
                        return
                    const name = gs.keyName(event.key, event.nativeScanCode, event.modifiers)
                    if (name === "")
                        return
                    if (["lshift", "rshift", "lctrl", "rctrl", "lalt", "ralt"].indexOf(name) >= 0) {
                        if (capture.held.indexOf(name) < 0)
                            capture.held = capture.held.concat([name])
                        capture.sawKey = false
                        return
                    }
                    if (capture.keyboardDevice) {
                        capture.captured = capture.held.concat([name]).join("+")
                        capture.sawKey = true
                    }
                }
                Keys.onReleased: event => {
                    event.accepted = true
                    if (event.isAutoRepeat)
                        return
                    const name = gs.keyName(event.key, event.nativeScanCode, event.modifiers)
                    const i = capture.held.indexOf(name)
                    if (i < 0)
                        return
                    if (capture.keyboardDevice && !capture.sawKey && capture.held.length === 1)
                        capture.captured = name
                    capture.held = capture.held.filter(n => n !== name)
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.AllButtons
                    onPressed: mouse => {
                        keyCatcher.forceActiveFocus()
                        if (!capture.mouseDevice)
                            return
                        const name = gs.mouseName(mouse.button)
                        if (name !== "")
                            capture.captured = capture.held.concat([name]).join("+")
                    }
                    onWheel: wheel => {
                        if (capture.mouseDevice && wheel.angleDelta.y !== 0)
                            capture.captured = capture.held.concat([wheel.angleDelta.y > 0 ? "mwheel_up" : "mwheel_down"]).join("+")
                    }
                }
            }
            ComboBox {
                id: inputField
                visible: capture.listDevice
                Layout.fillWidth: true
                editable: true
                model: gs.inputChoices
                onActivated: index => editText = gs.inputChoices[index]
            }
            RowLayout {
                spacing: 8
                Label { text: qsTr("game_settings.activation_mode"); color: Theme.text }
                ComboBox {
                    id: modeBox
                    implicitWidth: 220
                    model: [qsTr("game_settings.activation_default")].concat(gs.activationModes)
                }
                Label { text: qsTr("game_settings.taps"); color: Theme.text }
                SpinBox { id: tapBox; from: 1; to: 3; value: 1 }
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8
                AppButton { text: qsTr("game_settings.cancel_btn"); onClicked: capture.close() }
                AppButton {
                    text: qsTr("game_settings.unbind_btn")
                    onClicked: { gs.unbind(capture.target.map, capture.target.action); capture.close() }
                }
                AppButton {
                    role: "apply"
                    text: qsTr("game_settings.apply_bind_btn")
                    enabled: capture.listDevice ? inputField.editText.trim() !== "" : capture.captured !== ""
                    onClicked: capture.finish()
                }
            }
        }
    }
}
