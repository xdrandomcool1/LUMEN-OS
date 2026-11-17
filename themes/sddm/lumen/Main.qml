// SPDX-License-Identifier: GPL-3.0-or-later
// LUMEN login screen (SDDM greeter theme).
import QtQuick
import SddmComponents 2.0
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls

Rectangle {
    id: root
    width: 1440; height: 900
    color: "black"

    property int userIndex: userModel.lastIndex >= 0 ? userModel.lastIndex : 0
    property int sessionIndex: {
        for (let i = 0; i < sessionModel.rowCount(); ++i)
            if (String(sessionModel.data(sessionModel.index(i, 0), Qt.UserRole + 4)).indexOf("lumen") >= 0) return i
        return sessionModel.lastIndex
    }
    readonly property string userName: userModel.data(userModel.index(userIndex, 0), Qt.UserRole + 1) || ""
    readonly property string realName: userModel.data(userModel.index(userIndex, 0), Qt.UserRole + 2) || userName
    readonly property string avatar: userModel.data(userModel.index(userIndex, 0), Qt.UserRole + 4) || ""
    property bool failed: false
    property bool busy: false
    property date now: new Date()
    Timer { interval: 1000; running: true; repeat: true; onTriggered: root.now = new Date() }

    Connections {
        target: sddm
        function onLoginFailed() { root.failed = true; root.busy = false; password.text = ""; shake.restart(); password.forceActiveFocus() }
        function onLoginSucceeded() { exitFade.start() }
    }

    Image {
        id: wall
        anchors.fill: parent
        source: config.background
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
    }
    BackdropBlur { id: blur; source: wall; strength: 1.6 }
    Rectangle { anchors.fill: parent; color: Qt.rgba(0, 0, 0, 0.18) }

    // Entrance: the clock and user fade up as the greeter appears.
    Spring { id: intro; value: 0; to: 1; preset: Spring.Gentle }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.09 - (1 - intro.value) * 20
        opacity: intro.value
        spacing: 0
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDate(root.now, "dddd d MMMM")
            color: Qt.rgba(1, 1, 1, 0.9); font.family: "Inter Variable"; font.pixelSize: 24; font.weight: Font.DemiBold
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatTime(root.now, "HH:mm")
            color: Qt.rgba(1, 1, 1, 0.92); font.family: "Inter Display"; font.pixelSize: 112; font.weight: Font.Bold
            font.letterSpacing: -2
        }
    }

    Column {
        id: login
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.1 - (1 - intro.value) * 30
        opacity: intro.value
        spacing: 14

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 72; height: 72; radius: 36; clip: true
            color: Qt.rgba(1, 1, 1, 0.25)
            border.width: 1; border.color: Qt.rgba(1, 1, 1, 0.4)
            Image { anchors.fill: parent; source: root.avatar; fillMode: Image.PreserveAspectCrop; visible: status === Image.Ready }
            Text { anchors.centerIn: parent; visible: root.avatar.length === 0; text: root.realName.charAt(0).toUpperCase()
                   color: "white"; font.family: "Inter Variable"; font.pixelSize: 32; font.weight: Font.DemiBold }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.realName
            color: "white"; font.family: "Inter Variable"; font.pixelSize: 17; font.weight: Font.DemiBold
        }
        GlassSurface {
            id: field
            anchors.horizontalCenter: parent.horizontalCenter
            width: 220; height: 34; radius: 17
            material: Glass.Light
            backdrop: blur
            tintAlpha: 0.22
            shadowScale: 0.5
            property real shakeX: 0
            transform: Translate { x: field.shakeX }
            TextInput {
                id: password
                anchors.left: parent.left; anchors.leftMargin: 14
                anchors.right: go.left; anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                echoMode: TextInput.Password
                passwordCharacter: "●"
                color: "white"; font.family: "Inter Variable"; font.pixelSize: 14
                focus: true
                clip: true
                Text { anchors.fill: parent; visible: !password.text.length; text: root.failed ? qsTr("Incorrect password") : qsTr("Enter Password")
                       color: Qt.rgba(1, 1, 1, 0.6); font: password.font; verticalAlignment: Text.AlignVCenter }
                onAccepted: root.submit()
                onTextChanged: root.failed = false
            }
            Rectangle {
                id: go
                anchors.right: parent.right; anchors.rightMargin: 4; anchors.verticalCenter: parent.verticalCenter
                width: 26; height: 26; radius: 13
                color: Qt.rgba(1, 1, 1, password.text.length ? 0.35 : 0.15)
                Symbol { anchors.centerIn: parent; width: 14; height: 14; name: root.busy ? "circle-notch" : "arrow-right"; weight: Symbol.Bold; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: root.submit() }
            }
        }
        SequentialAnimation {
            id: shake
            NumberAnimation { target: field; property: "shakeX"; to: -14; duration: 50 }
            NumberAnimation { target: field; property: "shakeX"; to: 12; duration: 70 }
            NumberAnimation { target: field; property: "shakeX"; to: -8; duration: 60 }
            NumberAnimation { target: field; property: "shakeX"; to: 0; duration: 50 }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: keyboard.layouts.length > 1
            text: keyboard.layouts.length ? keyboard.layouts[keyboard.currentLayout].shortName.toUpperCase() : ""
            color: Qt.rgba(1, 1, 1, 0.7); font.family: "Inter Variable"; font.pixelSize: 12
            MouseArea { anchors.fill: parent; onClicked: keyboard.currentLayout = (keyboard.currentLayout + 1) % keyboard.layouts.length }
        }
    }

    function submit() {
        if (busy) return
        busy = true
        sddm.login(root.userName, password.text, root.sessionIndex)
    }

    // Power actions
    Row {
        anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 28
        spacing: 28
        opacity: intro.value
        Repeater {
            model: [["moon", qsTr("Sleep"), () => sddm.suspend(), sddm.canSuspend],
                    ["arrow-clockwise", qsTr("Restart"), () => sddm.reboot(), sddm.canReboot],
                    ["power", qsTr("Shut Down"), () => sddm.powerOff(), sddm.canPowerOff]]
            Column {
                required property var modelData
                visible: modelData[3]
                spacing: 6
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 40; height: 40; radius: 20; color: Qt.rgba(1, 1, 1, ma.containsMouse ? 0.3 : 0.18)
                    Symbol { anchors.centerIn: parent; width: 18; height: 18; name: modelData[0]; weight: Symbol.Bold; color: "white" }
                    MouseArea { id: ma; anchors.fill: parent; hoverEnabled: true; onClicked: modelData[2]() }
                }
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: modelData[1]; color: Qt.rgba(1, 1, 1, 0.8); font.family: "Inter Variable"; font.pixelSize: 11 }
            }
        }
    }

    // Login transition: fade to the wallpaper the desktop starts with.
    Rectangle { id: veil; anchors.fill: parent; color: "black"; opacity: 0 }
    NumberAnimation { id: exitFade; target: veil; property: "opacity"; to: 1; duration: 450; easing.type: Easing.InOutQuad }
    Component.onCompleted: password.forceActiveFocus()
}
