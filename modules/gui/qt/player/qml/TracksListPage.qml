
/*****************************************************************************
 * Copyright (C) 2023 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QtQml.Models


import VLC.MainInterface
import VLC.Style
import VLC.Player
import VLC.Widgets as Widgets
import VLC.Util
import VLC.Dialogs

// VLC redesign (Figma: Controls / subtitles, audio, video): tabs for the
// track types above a single track list and a "Show Preferences" footer
ColumnLayout {
    id: root

    readonly property ColorContext colorContext: ColorContext {
        id: theme

        colorSet: ColorContext.View
    }

    //FIXME make TrackMenuController a proper type (see TrackMenu)
    required property QtObject trackMenuController

    // the tab shown when the panel opens: 0 video, 1 audio, 2 subtitles
    property int currentTab: 2

    spacing: 0

    focus: true

    onActiveFocusChanged: if (activeFocus) tabRow.forceActiveFocus()

    // tab header (Figma: 50px, Dark/BG/B2)
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: VLCStyle.dp(50, VLCStyle.scale)

        color: theme.bg.secondary
        // rounded top corners only (per-corner radii require Qt 6.7)
        radius: VLCStyle.radius_large

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.radius
            color: parent.color
        }

        Widgets.NavigableRow {
            id: tabRow

            anchors.fill: parent
            anchors.leftMargin: VLCStyle.dp(6, VLCStyle.scale)
            anchors.rightMargin: VLCStyle.dp(6, VLCStyle.scale)

            focus: true
            Navigation.parentItem: root
            Navigation.downItem: tracksStack

            TabHeaderButton {
                text: qsTr("Playback")
                iconText: VLCIcons.play_filled
                checked: false
                onClicked: trackMenuController.requestPlaybackSpeedPage()
            }
            TabHeaderButton {
                text: qsTr("Video")
                iconText: VLCIcons.topbar_video
                checked: root.currentTab === 0
                onClicked: root.currentTab = 0
            }
            TabHeaderButton {
                text: qsTr("Audio")
                iconText: VLCIcons.audio
                checked: root.currentTab === 1
                onClicked: root.currentTab = 1
            }
            TabHeaderButton {
                text: qsTr("Subtitles")
                iconText: VLCIcons.subtitles
                checked: root.currentTab === 2
                onClicked: root.currentTab = 2
            }
        }
    }

    StackLayout {
        id: tracksStack

        Layout.fillWidth: true
        Layout.fillHeight: true

        currentIndex: root.currentTab

        TrackColumn {
            title: qsTr("Embedded Tracks")
            tracksModel: Player.videoTracks
            menuIcon: VLCIcons.add
            menuText: qsTr("Add")
            onMenuAction: (menuPos) => {
                DialogsProvider.loadVideoFile()
            }
        }
        TrackColumn {
            title: qsTr("Embedded Tracks")
            tracksModel: Player.audioTracks
            menuIcon: VLCIcons.ellipsis
            menuText: qsTr("Menu")
            onMenuAction: (menuPos)  => {
                menuAudio.popup(menuPos)
            }
        }
        TrackColumn {
            title: qsTr("Embedded Tracks")
            tracksModel: Player.subtitleTracks
            menuIcon: VLCIcons.ellipsis
            menuText: qsTr("Menu")
            onMenuAction: (menuPos)  => {
                menuSubtitle.popup(menuPos)
            }
        }
    }

    // footer (Figma: "Show Preferences" 360x32 button)
    Widgets.ButtonExt {
        Layout.fillWidth: true
        Layout.margins: VLCStyle.margin_normal + VLCStyle.margin_xxsmall
        Layout.preferredHeight: VLCStyle.control_height

        text: qsTr("Show Preferences")
        font.pixelSize: VLCStyle.fontSize_normal

        onClicked: DialogsProvider.prefsDialog()
    }

    component TabHeaderButton: T.AbstractButton {
        id: tab

        property string iconText

        Layout.fillWidth: true
        implicitHeight: VLCStyle.dp(50, VLCStyle.scale)
        implicitWidth: tabContent.implicitWidth + VLCStyle.margin_normal
        height: parent ? parent.height : implicitHeight
        width: parent ? (parent.width / 4) : implicitWidth

        focusPolicy: Qt.StrongFocus

        Navigation.parentItem: tabRow

        Row {
            id: tabContent

            anchors.centerIn: parent
            spacing: VLCStyle.margin_xsmall

            Widgets.IconLabel {
                anchors.verticalCenter: parent.verticalCenter
                text: tab.iconText
                font.pixelSize: VLCStyle.icon_normal
                color: (tab.checked || tab.hovered) ? theme.fg.primary : theme.fg.secondary
            }

            T.Label {
                anchors.verticalCenter: parent.verticalCenter
                text: tab.text
                font.pixelSize: VLCStyle.fontSize_small
                color: (tab.checked || tab.hovered) ? theme.fg.primary : theme.fg.secondary
            }
        }

        // active tab underline (Figma: 2px accent)
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: VLCStyle.margin_xsmall
            anchors.bottomMargin: 0
            height: VLCStyle.dp(2, VLCStyle.scale)
            color: theme.accent
            visible: tab.checked
        }
    }

    component TrackColumn: T.Container {
        // wrap the contentItem i.e Column into Container
        // so that we can get focusReason, also Container
        // is a FocusScope
        id: tracksListContainer

        implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
        implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 implicitContentHeight + topPadding + bottomPadding)

        required property string title
        required property var tracksModel
        required property string menuIcon
        required property string menuText

        signal menuAction(point position)

        focus: true

        // sized by the StackLayout

        onActiveFocusChanged: if (activeFocus) tracksList.forceActiveFocus(focusReason)

        // this is required to initialize attached Navigation property
        Navigation.parentItem: tracksStack

        contentItem: Column {
            anchors.fill: parent

            focus: true

            Accessible.role: Accessible.Pane
            Accessible.name: tracksListContainer.title

            Item {
                // keep it inside so "Column" doesn't mess with it
               Rectangle {
                   id: separator

                   x: 0
                   y: 0
                   width: VLCStyle.margin_xxxsmall

                   height: tracksListContainer.height
                   color: theme.border
               }
            }

            Row {
                id: titleHeader

                width: tracksListContainer.width
                height: implicitHeight

                padding: VLCStyle.margin_xsmall

                topPadding: VLCStyle.margin_large
                leftPadding: VLCStyle.margin_xxlarge + separator.width

                focus: true

                clip: true

                Widgets.SubtitleLabel {
                    id: titleText

                    width: parent.width - button.width - parent.leftPadding
                           - parent.rightPadding

                    text: tracksListContainer.title
                    color: theme.fg.secondary
                    font.pixelSize: VLCStyle.fontSize_normal
                    anchors.verticalCenter: parent.verticalCenter
                }

                Widgets.IconTrackButton {
                    id: button

                    font.pixelSize: VLCStyle.icon_track

                    focus: true

                    description: tracksListContainer.menuText
                    text: tracksListContainer.menuIcon

                    Navigation.parentItem: tracksListContainer
                    Navigation.downItem: tracksList

                    onClicked: {
                        tracksListContainer.menuAction(mapToGlobal(0, height))
                    }
                }
            }

            Widgets.ListViewExt {
                id: tracksList

                model: tracksListContainer.tracksModel
                width: tracksListContainer.width
                height: tracksListContainer.height - titleHeader.height
                leftMargin: separator.width
                focus: true
                clip: !fadingEdge.implicitClipping && (height < contentHeight)

                fadingEdge.backgroundColor: theme.bg.primary

                Accessible.role: Accessible.List
                Accessible.name: qsTr("Track list")

                Navigation.parentItem: tracksListContainer
                Navigation.upItem: button
                Keys.priority: Keys.AfterItem
                Keys.onPressed: (event) => Navigation.defaultKeyAction(event)

                delegate: Widgets.CheckedDelegate {
                    readonly property bool isModelChecked: model.checked
                    clip: true

                    focus: true
                    text: model.display
                    width: tracksListContainer.width - VLCStyle.margin_xxxsmall
                    // Figma: 32px rows with 12px spacing, 14px Dark/FG/A1
                    height: VLCStyle.dp(44, VLCStyle.scale)
                    leftPadding: VLCStyle.margin_large
                    font.pixelSize: VLCStyle.fontSize_normal
                    font.weight: Font.Normal

                    onIsModelCheckedChanged: {
                        if (model.checked !== checked)
                            checked = model.checked
                    }

                    onCheckedChanged: {
                        if (model.checked !== checked)
                            model.checked = checked
                    }

                    onClicked: {
                        tracksList.currentIndex = index
                        tracksList.setCurrentItemFocus(Qt.MouseFocusReason)
                    }

                    Navigation.parentItem: tracksList
                }
            }
        }
    }

    QmlSubtitleMenu {
        id: menuSubtitle

        player: Player
        ctx: MainCtx

        onTriggered: {
            if (action === QmlSubtitleMenu.Open) {
                DialogsProvider.loadSubtitlesFile()
            }
            else if (action === QmlSubtitleMenu.Synchronize) {
                trackMenuController.requestSubtitlePage()
            }
            else if (action === QmlSubtitleMenu.Download) {
                Player.openVLsub()
            }
        }
    }

    QmlAudioMenu {
        id: menuAudio

        ctx: MainCtx

        onTriggered: {
            if (action === QmlSubtitleMenu.Open) {
                DialogsProvider.loadAudioFile()
            }
            else if (action === QmlSubtitleMenu.Synchronize) {
                trackMenuController.requestAudioPage()
            }
        }
    }
}
