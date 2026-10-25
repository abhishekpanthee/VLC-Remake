/*****************************************************************************
 * Copyright (C) 2020 VLC authors and VideoLAN
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
import QtQuick.Layouts
import QtQuick.Templates as T


import VLC.MainInterface
import VLC.Player
import VLC.Style
import VLC.Widgets as Widgets
import VLC.Util

FocusScope {
    id: resumePanel

    property int maxWidth



    visible: false
    enabled: visible

    Accessible.role: Accessible.Dialog
    Accessible.name: qsTr("Resume dialog")

    signal hidden()

    function showResumePanel ()
    {
        resumePanel.visible = true
        continueBtn.forceActiveFocus()
        resumeTimeout.start()
    }

    function hideResumePanel() {
        resumeTimeout.stop()
        resumePanel.visible = false
        Player.acknowledgeRestoreCallback()
        hidden()
    }

    Timer {
        id: resumeTimeout
        interval: 10000
        onTriggered: {
            resumePanel.visible = false
            hidden()
        }
    }

    Connections {
        target: Player
        function onCanRestorePlaybackChanged() {
            if (Player.canRestorePlayback) {
                showResumePanel()
            } else {
                hideResumePanel()
            }
        }
    }

    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.Window
    }


    Component.onCompleted: {
        if (Player.canRestorePlayback) {
            showResumePanel()
        }
    }

    Navigation.cancelAction: function() {
        hideResumePanel()
    }

    // VLC redesign (Figma: Player - hide UI): a floating pill at the bottom
    // center of the window, above the control bar. This item takes no room
    // in the top bar anymore.
    implicitHeight: 0
    implicitWidth: 0
    baselineOffset: 0

    readonly property string _timeText: (Player.restorePlaybackTime && Player.restorePlaybackTime.valid())
                                        ? Player.restorePlaybackTime.formatHMS() : ""

    Rectangle {
        id: pill

        parent: resumePanel.Window.contentItem ?? resumePanel
        visible: resumePanel.visible
        z: 100

        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
        anchors.bottom: parent ? parent.bottom : undefined
        anchors.bottomMargin: VLCStyle.controlLayoutHeight + VLCStyle.margin_large

        implicitWidth: row.implicitWidth + VLCStyle.margin_large * 2
        implicitHeight: VLCStyle.dp(52, VLCStyle.scale)

        radius: VLCStyle.dp(16, VLCStyle.scale)
        color: theme.bg.secondary.alpha(0.95)

        RowLayout {
            id: row

            anchors.centerIn: parent
            spacing: VLCStyle.margin_large

            T.Label {
                id: label

                text: resumePanel._timeText.length > 0
                      ? qsTr("You\u2019ve watched till %1 already.").arg(resumePanel._timeText)
                      : qsTr("Do you want to restart the playback where you left off?")
                color: pillTheme.fg.primary
                font.pixelSize: VLCStyle.fontSize_xlarge
            }

            Widgets.ButtonExt {
                id: continueBtn

                text: qsTr("Continue")
                AccessibleCompat.id: "resumeContinueButton"
                focus: true

                colorContext.colorSet: ColorContext.ButtonAccent
                implicitWidth: Math.max(VLCStyle.dp(146, VLCStyle.scale), implicitContentWidth + leftPadding + rightPadding)
                implicitHeight: VLCStyle.control_height

                onClicked: {
                    Player.restorePlaybackPos()
                    hideResumePanel()
                }

                Navigation.parentItem: resumePanel
                Navigation.rightItem: closeBtn
                Keys.priority: Keys.AfterItem
                Keys.onPressed: (event) =>  continueBtn.Navigation.defaultKeyAction(event)
            }

            Widgets.IconToolButton {
                id: closeBtn

                text: VLCIcons.close
                description: qsTr("Dismiss")
                AccessibleCompat.id: "resumeDismissButton"

                onClicked: hideResumePanel()

                Navigation.parentItem: resumePanel
                Navigation.leftItem: continueBtn
                Keys.priority: Keys.AfterItem
                Keys.onPressed: (event) => closeBtn.Navigation.defaultKeyAction(event)
            }
        }

        ColorContext {
            id: pillTheme
            colorSet: ColorContext.View
        }
    }
}
