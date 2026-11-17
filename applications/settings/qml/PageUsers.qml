import QtQuick
import Lumen.Design
import Lumen.Controls
import Lumen.Settings

SettingsPage {
    id: page
    property string message
    Connections { target: Accounts; function onFailed(m) { page.message = m } }
    Column {
        width: parent.width; spacing: 8
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 84; height: 84; radius: 42; clip: true; color: Theme.controlFillPressed
            Image { anchors.fill: parent; source: Accounts.iconFile ? "file://" + Accounts.iconFile : ""; fillMode: Image.PreserveAspectCrop; visible: status === Image.Ready }
            LText { anchors.centerIn: parent; visible: !Accounts.iconFile; text: (Accounts.realName || Accounts.userName).charAt(0).toUpperCase(); font.pixelSize: 36; font.weight: Font.DemiBold }
        }
        LText { anchors.horizontalCenter: parent.horizontalCenter; text: Accounts.realName || Accounts.userName; style: LText.Title2 }
        LText { anchors.horizontalCenter: parent.horizontalCenter; text: Accounts.administrator ? qsTr("Administrator") : qsTr("Standard user"); secondary: true }
    }
    FormSection {
        FormRow { label: qsTr("Full name"); TextField { width: 220; text: Accounts.realName; onEditingFinished: if (text !== Accounts.realName) Accounts.setRealName(text) } }
        FormRow { label: qsTr("Account name"); LText { text: Accounts.userName; secondary: true } }
        FormRow { label: qsTr("Password"); separator: false; Button { text: qsTr("Change…"); onClicked: pwBox.visible = !pwBox.visible } }
    }
    FormSection {
        id: pwBox
        visible: false
        title: qsTr("Change password")
        footer: qsTr("You'll be asked to authenticate. The password is hashed (SHA-512) before it leaves this app.")
        FormRow { label: qsTr("New password"); TextField { id: p1; width: 220; echoMode: TextInput.Password } }
        FormRow { label: qsTr("Verify"); TextField { id: p2; width: 220; echoMode: TextInput.Password } }
        FormRow { label: qsTr("Hint (optional)"); separator: false
            Row { spacing: 6
                TextField { id: hint; width: 140 }
                Button { kind: Button.Prominent; text: qsTr("Change"); enabled: p1.text.length >= 1 && p1.text === p2.text
                         onClicked: { Accounts.setPassword(p1.text, hint.text); p1.text = ""; p2.text = ""; pwBox.visible = false } } } }
    }
    FormSection {
        title: qsTr("Other users")
        visible: Accounts.users.length > 1
        Repeater { model: Accounts.users
            FormRow { required property var modelData; label: modelData.realName || modelData.userName; detail: modelData.admin ? qsTr("Admin") : qsTr("Standard") } }
    }
    LText { visible: page.message.length > 0; text: page.message; color: Theme.red; width: parent.width; wrapMode: Text.Wrap }
}
