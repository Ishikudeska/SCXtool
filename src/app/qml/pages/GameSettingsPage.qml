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
    readonly property bool sidePanel: gs.infoMode === "sidebar"
    readonly property color inspectedColor: Qt.rgba(Theme.highlight.r, Theme.highlight.g, Theme.highlight.b, 0.18)
    property var inspected: null
    property var foldedSections: ({})
    readonly property bool filtering: gs.search !== "" || gs.modifiedOnly
    readonly property var inspectedInfo: page.describe(page.inspected, gs.revision)

    function inspect(target) {
        if (page.sidePanel)
            page.inspected = target
    }
    function sectionFolded(id) {
        if (page.filtering)
            return false
        return id in page.foldedSections ? page.foldedSections[id] : id.endsWith(".other")
    }
    function toggleSection(id) {
        const next = Object.assign({}, page.foldedSections)
        next[id] = !page.sectionFolded(id)
        page.foldedSections = next
    }
    function isInspected(kind, key) {
        return page.sidePanel && page.inspected !== null && page.inspected.kind === kind && page.inspected.key === key
    }
    function showInfo(item, target) {
        infoPopup.target = target
        infoPopup.anchorItem = item
        infoPopup.open()
    }
    function hideInfo(item) {
        if (infoPopup.anchorItem === item)
            infoPopup.close()
    }
    function settingValueText(row) {
        const unit = row.unit !== "" ? " " + row.unit : ""
        if (row.kind === "toggle")
            return row.checked ? qsTr("game_settings.on") : qsTr("game_settings.off")
        if (row.kind === "choice")
            return row.choiceIndex >= 0 && row.choiceIndex < row.choices.length ? row.choices[row.choiceIndex].label : row.value
        if (row.kind === "slider")
            return Number(row.number).toFixed(row.decimals) + unit
        return row.value !== "" ? row.value + unit : "—"
    }
    function describe(target, revision) {
        if (!target)
            return null
        const on = qsTr("game_settings.on")
        const off = qsTr("game_settings.off")
        const changed = { text: qsTr("game_settings.modified_tip"), color: Theme.em4 }
        if (target.kind === "setting") {
            const row = gs.settingState(target.key, revision)
            if (!row.name)
                return null
            const details = [{ label: qsTr("game_settings.info.current"), value: page.settingValueText(row) }]
            if (row.defaultText !== "")
                details.push({ label: qsTr("game_settings.info.default"), value: row.defaultText })
            if (row.kind === "slider") {
                const unit = row.unit !== "" ? " " + row.unit : ""
                details.push({ label: qsTr("game_settings.info.range"),
                               value: App.fmt(qsTr("game_settings.info.range_value"),
                                              { min: Number(row.min).toFixed(row.decimals) + unit,
                                                max: Number(row.max).toFixed(row.decimals) + unit }) })
            }
            if (row.kind === "choice")
                details.push({ label: qsTr("game_settings.info.options"), value: row.choices.map(c => c.label).join(", ") })
            details.push({ label: qsTr("game_settings.info.attribute"), value: row.name })
            if (row.cvar)
                details.push({ label: qsTr("game_settings.info.cvar"), value: row.cvar })
            const notes = []
            if (row.restart)
                notes.push({ text: qsTr("game_settings.restart_tip"), color: Theme.dim })
            if (row.pending)
                notes.push({ text: qsTr("game_settings.pending_tip"), color: Theme.action("needsApply") })
            else if (row.modified)
                notes.push(changed)
            return { title: row.label, description: row.description, details: details, notes: notes }
        }
        if (target.kind === "option") {
            const row = gs.optionState(target.key, revision)
            if (!row.group)
                return null
            const details = []
            if (row.showInvert)
                details.push({ label: qsTr("game_settings.invert"), value: row.inverted ? on : off })
            if (row.showCurve)
                details.push({ label: qsTr("game_settings.exponent"), value: row.exponent !== "" ? row.exponent : row.defaultExponent })
            details.push({ label: qsTr("game_settings.info.group"), value: row.group })
            if (row.invertCvar !== "")
                details.push({ label: qsTr("game_settings.info.invert_cvar"), value: row.invertCvar })
            const notes = []
            if (row.hasCurve)
                notes.push({ text: qsTr("game_settings.info.has_curve"), color: Theme.dim })
            if (row.modified)
                notes.push(changed)
            return { title: row.label,
                     description: row.showInvert || row.showCurve ? qsTr("game_settings.info.option_desc") : qsTr("game_settings.input_devices_desc"),
                     details: details, notes: notes }
        }
        if (target.kind === "axis") {
            const row = gs.axisState(target.key, revision)
            if (!row.input)
                return null
            return { title: row.label, description: qsTr("game_settings.info.axis_desc"),
                     details: [{ label: qsTr("game_settings.deadzone"), value: row.deadzone !== "" ? row.deadzone : "0" },
                               { label: qsTr("game_settings.saturation"), value: row.saturation !== "" ? row.saturation : "1" },
                               { label: qsTr("game_settings.info.input"), value: row.input }],
                     notes: [] }
        }
        return null
    }

    GameSettingsController {
        id: gs
        active: page.active
    }

    Connections {
        target: gs
        function onTabChanged() {
            page.inspected = null
            infoPopup.close()
        }
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

    component InfoIcon: Rectangle {
        id: infoIcon
        property var target: null
        visible: !page.sidePanel
        implicitWidth: 16
        implicitHeight: 16
        radius: 8
        color: "transparent"
        border.color: iconHover.hovered ? Theme.highlight : Theme.dim
        Label {
            anchors.centerIn: parent
            text: "i"
            font.pixelSize: 11
            font.bold: true
            color: iconHover.hovered ? Theme.highlight : Theme.dim
        }
        HoverHandler {
            id: iconHover
            onHoveredChanged: hovered ? page.showInfo(infoIcon, infoIcon.target) : page.hideInfo(infoIcon)
        }
    }

    component InfoView: ColumnLayout {
        id: infoView
        property var info: null
        property bool selectable: false
        readonly property bool described: infoView.info !== null && infoView.info.description !== ""
        spacing: 8
        Label {
            Layout.fillWidth: true
            text: infoView.info !== null ? infoView.info.title : ""
            color: Theme.title
            font.bold: true
            font.pixelSize: 15
            wrapMode: Text.WordWrap
        }
        Label {
            Layout.fillWidth: true
            text: infoView.described ? infoView.info.description : qsTr("game_settings.info.no_description")
            color: infoView.described ? Theme.text : Theme.placeholder
            font.italic: !infoView.described
            wrapMode: Text.WordWrap
        }
        Repeater {
            model: infoView.info !== null ? infoView.info.details : []
            delegate: RowLayout {
                id: detail
                required property var modelData
                Layout.fillWidth: true
                spacing: 10
                Label {
                    Layout.preferredWidth: 110
                    Layout.alignment: Qt.AlignTop
                    text: detail.modelData.label
                    color: Theme.dim
                    wrapMode: Text.WordWrap
                }
                TextEdit {
                    Layout.fillWidth: true
                    text: detail.modelData.value
                    color: Theme.text
                    font.pixelSize: Theme.fontSize
                    readOnly: true
                    selectByMouse: infoView.selectable
                    selectionColor: Theme.highlight
                    selectedTextColor: Theme.highlightedText
                    wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere
                }
            }
        }
        Repeater {
            model: infoView.info !== null ? infoView.info.notes : []
            delegate: Label {
                required property var modelData
                Layout.fillWidth: true
                text: "• " + modelData.text
                color: modelData.color
                wrapMode: Text.WordWrap
            }
        }
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

    component SettingRow: Rectangle {
        id: settingRow
        required property var base
        readonly property var row: gs.settingState(base.name, gs.revision)
        Layout.fillWidth: true
        implicitHeight: settingLine.implicitHeight + 6
        radius: Theme.radius
        color: page.isInspected("setting", base.name) ? page.inspectedColor : "transparent"

        HoverHandler {
            onHoveredChanged: if (hovered) page.inspect({ kind: "setting", key: settingRow.base.name })
        }

        RowLayout {
            id: settingLine
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 4
            anchors.rightMargin: 4
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
            RowLayout {
                Layout.preferredWidth: Math.max(220, settingRow.width * 0.42)
                Layout.maximumWidth: Math.max(220, settingRow.width * 0.42)
                spacing: 6
                Label {
                    Layout.fillWidth: true
                    text: settingRow.row.label + (settingRow.row.restart ? "  ⟳" : "")
                    color: Theme.text
                    wrapMode: Text.WordWrap
                }
                InfoIcon {
                    Layout.alignment: Qt.AlignTop
                    target: ({ kind: "setting", key: settingRow.base.name })
                }
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
    }

    component SettingSections: Repeater {
        model: gs.sections
        delegate: Section {
            id: sectionBox
            required property var modelData
            Layout.fillWidth: true
            title: modelData.title
            collapsible: !page.filtering
            collapsed: page.sectionFolded(modelData.id)
            collapsedNote: modelData.rows.length === 1 ? qsTr("game_settings.section_count_one")
                         : App.fmt(qsTr("game_settings.section_count"), { count: modelData.rows.length })
            onToggleRequested: page.toggleSection(modelData.id)
            Repeater {
                model: sectionBox.collapsed ? [] : sectionBox.modelData.rows
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
            Label { text: qsTr("game_settings.info.mode_label"); color: Theme.text }
            ComboBox {
                objectName: "gameSettingsInfoMode"
                implicitWidth: 150
                model: [qsTr("game_settings.info.mode_popup"), qsTr("game_settings.info.mode_sidebar")]
                currentIndex: page.sidePanel ? 1 : 0
                onActivated: index => gs.infoMode = index === 1 ? "sidebar" : "popup"
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("game_settings.info.mode_tip")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.ready
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
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
                            collapsible: !page.filtering
                            collapsed: page.sectionFolded("controls.input_devices")
                            onToggleRequested: page.toggleSection("controls.input_devices")
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
                                    HoverHandler {
                                        onHoveredChanged: if (hovered) page.inspect({ kind: "option", key: optionRow.modelData.group })
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: 380
                                        Layout.maximumWidth: 380
                                        spacing: 6
                                        Label {
                                            Layout.fillWidth: true
                                            leftPadding: optionRow.row.depth * 16
                                            text: optionRow.row.label
                                            color: optionRow.heading ? Theme.title : Theme.text
                                            font.bold: optionRow.heading
                                            elide: Text.ElideRight
                                        }
                                        InfoIcon { target: ({ kind: "option", key: optionRow.modelData.group }) }
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
                                    HoverHandler {
                                        onHoveredChanged: if (hovered) page.inspect({ kind: "axis", key: axisRow.modelData.input })
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: 200
                                        Layout.maximumWidth: 200
                                        spacing: 6
                                        Label { Layout.fillWidth: true; text: axisRow.row.label; color: Theme.text; elide: Text.ElideRight }
                                        InfoIcon { target: ({ kind: "axis", key: axisRow.modelData.input }) }
                                    }
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

            Rectangle {
                objectName: "gameSettingsInfoPanel"
                visible: page.sidePanel && page.settingsTab
                Layout.preferredWidth: Math.max(260, Math.min(360, page.width * 0.28))
                Layout.fillHeight: true
                color: Theme.panel
                border.color: Theme.border
                radius: Theme.radius

                ScrollView {
                    id: infoScroll
                    anchors.fill: parent
                    anchors.margins: 12
                    visible: page.inspectedInfo !== null
                    clip: true
                    contentWidth: availableWidth

                    InfoView {
                        width: infoScroll.availableWidth
                        info: page.inspectedInfo
                        selectable: true
                    }
                }
                Label {
                    anchors.fill: parent
                    anchors.margins: 12
                    visible: page.inspectedInfo === null
                    text: qsTr("game_settings.info.empty")
                    color: Theme.placeholder
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    Popup {
        id: infoPopup
        property var target: null
        property Item anchorItem: null
        parent: Overlay.overlay
        x: {
            if (infoPopup.anchorItem === null)
                return 0
            const right = infoPopup.anchorItem.mapToItem(infoPopup.parent, infoPopup.anchorItem.width + 8, 0).x
            return right + infoPopup.width <= infoPopup.parent.width - 8
                   ? right : Math.max(8, infoPopup.anchorItem.mapToItem(infoPopup.parent, 0, 0).x - infoPopup.width - 8)
        }
        y: infoPopup.anchorItem === null ? 0
           : Math.max(8, Math.min(infoPopup.anchorItem.mapToItem(infoPopup.parent, 0, -8).y,
                                  infoPopup.parent.height - infoPopup.height - 8))
        width: Math.min(380, page.width - 40)
        padding: 12
        modal: false
        focus: false
        closePolicy: Popup.NoAutoClose
        background: Rectangle { color: Theme.panel; border.color: Theme.border; radius: 6 }
        contentItem: InfoView { info: page.describe(infoPopup.target, gs.revision) }
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
