// SPDX-License-Identifier: GPL-3.0-or-later
// Media Player: video & audio via Qt Multimedia (FFmpeg back-end).
import QtQuick
import QtQuick.Dialogs
import QtMultimedia
import Lumen.Design
import Lumen.Motion
import Lumen.Glass
import Lumen.Controls

AppWindow {
    id: win
    property string openFile
    title: player.source.toString().length ? decodeURIComponent(player.source.toString().split("/").pop()) : qsTr("Media Player")
    width: 900; height: 560
    visible: true
    contentColor: "black"
    function fmt(ms) { const s = Math.floor(ms / 1000); return Math.floor(s / 60) + ":" + String(s % 60).padStart(2, "0") }

    MediaPlayer {
        id: player
        source: win.openFile
        videoOutput: video
        audioOutput: AudioOutput { id: audio; volume: 0.8 }
        onErrorOccurred: (e, msg) => error.text = msg
        Component.onCompleted: if (source.toString().length) play()
    }
    VideoOutput { id: video; x: 0; y: 0; width: win.width; height: win.height }
    Column {
        anchors.centerIn: parent; spacing: 10
        visible: !player.hasVideo
        Symbol { anchors.horizontalCenter: parent.horizontalCenter; width: 64; height: 64; name: player.hasAudio ? "music-notes" : "film-strip"; color: "#8E8E93" }
        LText { anchors.horizontalCenter: parent.horizontalCenter; text: player.metaData.stringValue(MediaMetaData.Title) || (player.hasAudio ? win.title : qsTr("Drop a video or song here")); color: "#C7C7CC" }
        Button { anchors.horizontalCenter: parent.horizontalCenter; visible: !player.hasAudio; text: qsTr("Open…"); onClicked: dlg.open() }
        LText { id: error; anchors.horizontalCenter: parent.horizontalCenter; color: Theme.red }
    }
    FileDialog { id: dlg; onAccepted: { player.source = selectedFile; player.play() } }
    DropArea { anchors.fill: parent; onDropped: d => { if (d.hasUrls) { player.source = d.urls[0]; player.play() } } }

    // Floating glass transport controls; fade out while playing.
    HoverHandler { id: hover }
    Spring { id: show; to: hover.hovered || player.playbackState !== MediaPlayer.PlayingState ? 1 : 0; preset: Spring.Smooth }
    GlassSurface {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 18
        width: Math.min(parent.width - 40, 560); height: 70; radius: Theme.metrics.windowRadius
        material: Glass.Dark; compositorBlur: false
        opacity: show.value; visible: opacity > 0.01
        Column {
            anchors.fill: parent; anchors.margins: 10; spacing: 4
            Row {
                anchors.horizontalCenter: parent.horizontalCenter; spacing: 18
                Symbol { width: 20; height: 20; name: "rewind"; weight: Symbol.Fill; color: "white"; MouseArea { anchors.fill: parent; onClicked: player.position = Math.max(0, player.position - 10000) } }
                Symbol { width: 24; height: 24; name: player.playbackState === MediaPlayer.PlayingState ? "pause" : "play"; weight: Symbol.Fill; color: "white"
                         MouseArea { anchors.fill: parent; onClicked: player.playbackState === MediaPlayer.PlayingState ? player.pause() : player.play() } }
                Symbol { width: 20; height: 20; name: "fast-forward"; weight: Symbol.Fill; color: "white"; MouseArea { anchors.fill: parent; onClicked: player.position = Math.min(player.duration, player.position + 10000) } }
            }
            Row {
                spacing: 8; width: parent.width
                LText { text: win.fmt(player.position); color: "white"; style: LText.Caption; width: 40 }
                Slider { width: parent.width - 180; from: 0; to: Math.max(1, player.duration); value: player.position; onMoved: player.position = value }
                LText { text: win.fmt(player.duration); color: "white"; style: LText.Caption; width: 40 }
                Slider { width: 70; from: 0; to: 1; value: audio.volume; onMoved: audio.volume = value }
            }
        }
    }
    Shortcut { sequence: "Space"; onActivated: player.playbackState === MediaPlayer.PlayingState ? player.pause() : player.play() }
    Shortcut { sequence: "Right"; onActivated: player.position = Math.min(player.duration, player.position + 5000) }
    Shortcut { sequence: "Left"; onActivated: player.position = Math.max(0, player.position - 5000) }
    Shortcut { sequence: "F"; onActivated: win.visibility === Window.FullScreen ? win.showNormal() : win.showFullScreen() }
}
