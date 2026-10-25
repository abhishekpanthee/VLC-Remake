/*****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
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
import QtQuick.Window


import VLC.MainInterface
import VLC.Style
import VLC.Playlist
import VLC.Widgets as Widgets
import VLC.Menus as Menus
import VLC.Util

T.ToolBar {
    id: root

    Accessible.name: qsTr("Local top bar")
    AccessibleCompat.id: "localTopbar"

    property bool useAcrylic: true

    // For now, used for d&d functionality
    // Not strictly necessary to set
    property Item plListView

    property bool _showCSD: MainCtx.clientSideDecoration
        && !(MainCtx.intfMainWindow.visibility === Window.FullScreen)
        && !MainCtx.hasToolbarMenu

    //whether the navigation pannel is actually visible or not
    property alias navigationVisible: navigationVisibilty.checked
    property alias playqueueVisible: playlistBtn.checked

    property alias navigationButton: navigationVisibilty

    signal toggleNavigationVisibility()
    signal togglePlayqueueVisibility()

    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                             implicitContentWidth + leftPadding + rightPadding + (_showCSD ? csdButtons.implicitWidth : 0))

    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.Window
    }

    ColorContext {
        id: titleBarTheme
        colorSet: ColorContext.MenuBar
    }

    ColorContext {
        id: viewTheme
        colorSet: ColorContext.View
    }

    // VLC redesign (Figma: topbar): Dark/BG/A3 background
    background:  Widgets.AcrylicBackground {
        enabled: root.useAcrylic
        tintColor: titleBarTheme.bg.secondary
    }

    contentItem: Item {

        implicitHeight: VLCStyle.globalToolbar_height
        implicitWidth: navigationButtons.implicitWidth
                     + playqueueButtonsTop.implicitWidth
                     + VLCStyle.margin_xsmall * 2

        Item {
            id: navigationButtons

            implicitWidth: navigationButtonsRow.implicitWidth

            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                leftMargin: VLCStyle.titleBar_padding
            }

            Navigation.parentItem: root

            // VLC redesign (Figma: topbar): [logo] [menu] [navigation panel] | [back]
            Row {
                id: navigationButtonsRow

                anchors.verticalCenter: parent.verticalCenter
                spacing: VLCStyle.margin_xsmall

                Item {
                    width: VLCStyle.titleBar_logoWidth
                    height: VLCStyle.bannerButton_height

                    Widgets.BannerCone {
                        id: logo

                        anchors.centerIn: parent
                        sourceSize.width: VLCStyle.icon_banner
                        sourceSize.height: VLCStyle.icon_banner
                        color: theme.accent
                    }
                }

                Widgets.IconToolButton {
                    id: menuBtn

                    visible: !MainCtx.hasToolbarMenu
                    font.pixelSize: VLCStyle.icon_banner
                    text: VLCIcons.ellipsis
                    description: qsTr("Menu")
                    width: VLCStyle.titleBar_buttonWidth
                    height: VLCStyle.bannerButton_height
                    checked: contextMenu.shown

                    Navigation.parentItem: navigationButtons
                    Navigation.rightItem: navigationVisibilty

                    onClicked: contextMenu.popup(this.mapToGlobal(0, height))

                    Menus.QmlGlobalMenu {
                        id: contextMenu
                        ctx: MainCtx
                    }
                }

                Navigation.parentItem: root
                Navigation.rightItem: playqueueButtonsTop

                Widgets.IconToolButton {
                    id: navigationVisibilty

                    focus: true

                    text: VLCIcons.panel_left
                    description: qsTr("Show navigation menu")

                    AccessibleCompat.id: "sidebarToggleButton"

                    font.pixelSize: VLCStyle.icon_banner
                    height: VLCStyle.bannerButton_height
                    width: VLCStyle.titleBar_buttonWidth

                    onClicked: root.toggleNavigationVisibility()

                    Navigation.parentItem: navigationButtons
                    Navigation.leftItem: menuBtn
                    Navigation.rightItem: historyBack
                }

                // divider (Figma: Icon/divider, Dark/Divider/Bright)
                Item {
                    width: VLCStyle.icon_banner
                    height: VLCStyle.bannerButton_height

                    Rectangle {
                        anchors.centerIn: parent
                        width: VLCStyle.border
                        height: VLCStyle.dp(16, VLCStyle.scale)
                        color: viewTheme.border
                    }
                }

                Widgets.IconToolButton {
                    id: historyBack

                    font.pixelSize: VLCStyle.icon_banner
                    text: VLCIcons.back
                    description: qsTr("Previous")

                    AccessibleCompat.id: "historyBackButton"

                    height: VLCStyle.bannerButton_height
                    width: VLCStyle.titleBar_buttonWidth

                    onClicked: History.previous()
                    enabled: !History.previousEmpty

                    onEnabledChanged: {
                        if (historyBack.activeFocus)
                            navigationVisibilty.focus = true
                    }

                    Navigation.parentItem: navigationButtons
                    Navigation.leftItem: navigationVisibilty
                    Navigation.rightItem: playlistBtn
                }
            }
        }

        // window title, centered (Figma: topbar, 14px Dark/FG/A2)
        T.Label {
            anchors.centerIn: parent
            width: Math.min(implicitWidth,
                            parent.width - 2 * Math.max(navigationButtons.width, playqueueButtonsTop.width + playqueueButtonsTop.anchors.rightMargin) - 2 * VLCStyle.margin_normal)
            visible: width > 0
            elide: Text.ElideRight

            // media name while playing (qt-name-in-title), application name otherwise
            text: (MainCtx.intfMainWindow && MainCtx.intfMainWindow.title.length > 0) ? MainCtx.intfMainWindow.title
                                                                                     : qsTr("VLC Media Player")
            color: viewTheme.fg.secondary
            font.pixelSize: VLCStyle.fontSize_normal

            Accessible.ignored: true
        }

        Item {
            id: playqueueButtonsTop

            implicitWidth: playqueueButtonsTopRow.implicitWidth

            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                //put the buttons below the CSD if we have enough space
                rightMargin: VLCStyle.margin_xsmall
                    + ((root._showCSD && root.topPadding < csdButtons.height)
                        ? Math.max(csdButtons.width - root.rightPadding, 0)
                        : 0)
            }

            Navigation.parentItem: root

            Row {
                id: playqueueButtonsTopRow

                anchors.verticalCenter: parent.verticalCenter
                spacing: VLCStyle.margin_xsmall

                Navigation.parentItem: root
                Navigation.leftItem: navigationButtons

                PlaylistPane.PlaylistToggleButton {
                    id: playlistBtn

                    plListView: root.plListView

                    Navigation.parentItem: playqueueButtonsTop
                    Navigation.leftItem: historyBack
                    Navigation.rightItem: fullscreenBtn

                    onClicked:  root.togglePlayqueueVisibility()
                }

                // VLC redesign (Figma: topbar): interface full screen toggle,
                // next to the window buttons
                Widgets.IconToolButton {
                    id: fullscreenBtn

                    font.pixelSize: VLCStyle.icon_banner
                    text: (MainCtx.intfMainWindow.visibility === Window.FullScreen) ? VLCIcons.defullscreen
                                                                                    : VLCIcons.fullscreen
                    description: qsTr("Full screen")
                    width: VLCStyle.titleBar_windowButtonWidth
                    height: VLCStyle.bannerButton_height

                    Navigation.parentItem: playqueueButtonsTop
                    Navigation.leftItem: playlistBtn

                    onClicked: MainCtx.toggleInterfaceFullScreen()
                }
            }
        }

        Loader {
            z: -1

            anchors.fill: parent
            active: root._showCSD
            sourceComponent: Widgets.CSDTitlebarTapNDrapHandler {
            }
        }
    }

    Loader {
        id: csdButtons

        //csd button are
        parent:  root
        anchors {
            right: parent.right
            top: parent.top
        }

        height: VLCStyle.globalToolbar_height
        active: root._showCSD
        source: VLCStyle.palette.hasCSDImage
                  ? "qrc:///qt/qml/VLC/Widgets/CSDThemeButtonSet.qml"
                  : "qrc:///qt/qml/VLC/Widgets/CSDWindowButtonSet.qml"
    }
}
