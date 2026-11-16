// SPDX-License-Identifier: GPL-3.0-or-later
// Installation slideshow (Calamares slideshow API 2).
import QtQuick
import calamares.slideshow 1.0

Presentation {
    id: presentation
    function nextSlide() { presentation.goToNextSlide() }
    Timer { interval: 7000; running: presentation.activatedInCalamares; repeat: true; onTriggered: presentation.goToNextSlide() }

    component LumenSlide: Slide {
        property string heading
        property string body
        Rectangle {
            anchors.fill: parent
            gradient: Gradient { GradientStop { position: 0; color: "#141a4a" } GradientStop { position: 1; color: "#3a1450" } }
            Column {
                anchors.centerIn: parent; width: parent.width * 0.75; spacing: 14
                Text { width: parent.width; horizontalAlignment: Text.AlignHCenter; text: heading; color: "white"
                       font.family: "Inter Variable"; font.pixelSize: 28; font.weight: Font.Bold; wrapMode: Text.Wrap }
                Text { width: parent.width; horizontalAlignment: Text.AlignHCenter; text: body; color: Qt.rgba(1, 1, 1, 0.8)
                       font.family: "Inter Variable"; font.pixelSize: 16; wrapMode: Text.Wrap }
            }
        }
    }
    LumenSlide { heading: qsTr("Welcome to LUMEN OS"); body: qsTr("A calm, fast desktop built on Fedora Linux, with Liquid Glass and fluid motion everywhere.") }
    LumenSlide { heading: qsTr("Find anything with Spotlight"); body: qsTr("Press Super + Space to launch apps, open files, change settings and calculate.") }
    LumenSlide { heading: qsTr("Thousands of apps"); body: qsTr("The App Store installs sandboxed Flatpak apps from Flathub, kept up to date automatically.") }
    LumenSlide { heading: qsTr("Private by design"); body: qsTr("Full-disk encryption, a firewall and per-app permissions keep your data yours.") }
}
