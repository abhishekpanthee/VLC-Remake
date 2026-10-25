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
import QtQuick.Layouts
import QtQuick.Templates as T

import VLC.MainInterface
import VLC.Widgets as Widgets
import VLC.Util
import VLC.Style
import VLC.Player

T.Pane {
    id: root

    property bool paintOnly: false

    readonly property color sliderColor: {
        const theme = volControl.colorContext
        return (volControl.position > _fullvolpos) ? theme.fg.negative : theme.fg.primary
    }

    readonly property var _player: paintOnly ? ({ muted: false, volume: .5 }) : Player
    readonly property int _maxvol: MainCtx.maxVolume
    readonly property real _fullvolpos: 100 / _maxvol
    readonly property real _maxvolpos: _maxvol / 100

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    contentWidth: volumeWidget.implicitWidth
    contentHeight: volumeWidget.implicitHeight

    Keys.priority: Keys.AfterItem
    Keys.onPressed: (event) => Navigation.defaultKeyAction(event)

    RowLayout {
        id: volumeWidget

        anchors.fill: parent
        spacing: 0

        Widgets.IconToolButton{
            id: volumeBtn

            focus: true
            paintOnly: root.paintOnly
            text: {
                if (_player.muted)
                    return VLCIcons.volume_muted
                else if (_player.volume === 0)
                    return VLCIcons.volume_zero
                else if (_player.volume < .33)
                    return VLCIcons.volume_low
                else if (_player.volume <= .66)
                    return VLCIcons.volume_medium
                else
                    return VLCIcons.volume_high
            }
            description: qsTr("Mute")

            AccessibleCompat.id: "muteButton"

            onClicked: Player.muted = !Player.muted

            Accessible.onIncreaseAction: {
                Player.muted = false
                Player.setVolumeUp()
            }

            Accessible.onDecreaseAction: {
                Player.muted = false
                Player.setVolumeDown()
            }

            Navigation.parentItem: root
            Navigation.rightItem: volControl
        }

        Widgets.SliderExt {
            id: volControl

            implicitWidth: VLCStyle.volumeMeter_width
            implicitHeight: VLCStyle.volumeMeter_height

            // WARNING: Directly setting the insets break handle and background positioning.
            Binding on topInset {
                when: MainCtx.usingTouch
                value: VLCStyle.touchHandlerMargin
            }
            Binding on bottomInset {
                when: MainCtx.usingTouch
                value: VLCStyle.touchHandlerMargin
            }

            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: VLCStyle.margin_xsmall

            property bool _inhibitPlayerVolumeUpdate: false

            // FIXME: Currently we are not updating the ShiftModifier status while dragging. This
            //        could be fixed with a custom Slider based on a MouseArea.
            property bool _shiftPressed: false

            property real _clamp: 0.01

            property bool _keyPressed: false

            property bool filterEvents: false

            color: root.sliderColor

            // VLC redesign (Figma: "volume"): a wedge shaped meter whose gradient
            // fill (green -> orange -> red over the whole range) is clipped to the
            // current level. Paths are the Figma vectors in a 89x28 box.
            handle: Item {
                implicitWidth: 0
                implicitHeight: 0
            }

            background: Canvas {
                id: volumeMeter

                readonly property string _trackPath: "M3.59756 19.3638C1.55468 19.1572 0 17.4374 0 15.3841V12.6159C0 10.5626 1.55467 8.84279 3.59756 8.6362L85.6982 0.333893C87.4654 0.155189 89 1.54247 89 3.31867V24.6813C89 26.4575 87.4654 27.8448 85.6982 27.6661L3.59756 19.3638Z"
                readonly property string _fillPath: "M3.29183 17.6875C2.27342 17.5809 1.5 16.7223 1.5 15.6984V12.3016C1.5 11.2776 2.27342 10.419 3.29183 10.3125L85.2918 1.73109C86.4723 1.60755 87.5 2.53331 87.5 3.72022V24.2798C87.5 25.4667 86.4723 26.3924 85.2918 26.2689L3.29183 17.6875Z"

                readonly property real level: volControl.visualPosition
                readonly property color trackColor: volControl.colorContext.bg.primary
                readonly property color lowColor: volControl.colorContext.fg.positive
                readonly property color midColor: volControl.colorContext.fg.neutral
                readonly property color highColor: volControl.colorContext.fg.negative

                x: volControl.leftPadding
                y: volControl.topPadding + (volControl.availableHeight - height) / 2
                implicitWidth: VLCStyle.volumeMeter_width
                implicitHeight: VLCStyle.volumeMeter_height
                width: volControl.availableWidth
                height: implicitHeight

                onLevelChanged: requestPaint()
                onTrackColorChanged: requestPaint()
                onLowColorChanged: requestPaint()
                onMidColorChanged: requestPaint()
                onHighColorChanged: requestPaint()
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()

                function _mix(a, b, t) {
                    return Qt.rgba(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
                                   a.b + (b.b - a.b) * t, 1.0)
                }

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    if (width <= 0 || height <= 0)
                        return
                    ctx.scale(width / 89, height / 28)

                    ctx.fillStyle = trackColor
                    ctx.path = _trackPath
                    ctx.fill()

                    if (level <= 0)
                        return

                    ctx.save()
                    ctx.beginPath()
                    ctx.rect(0, -1, 88 * level, 30)
                    ctx.clip()

                    const gradient = ctx.createLinearGradient(-1, 14, 89, 14)
                    gradient.addColorStop(0.0, lowColor)
                    gradient.addColorStop(0.578809, _mix(lowColor, midColor, 0.47))
                    gradient.addColorStop(0.792379, midColor)
                    gradient.addColorStop(1.0, highColor)
                    ctx.fillStyle = gradient
                    ctx.path = _fillPath
                    ctx.fill()

                    ctx.fillStyle = Qt.rgba(0, 0, 0, 0.2)
                    ctx.fill()
                    ctx.restore()
                }
            }

            from: 0
            to: root._maxvolpos
            opacity: _player.muted ? 0.5 : 1

            enabled: !root.paintOnly // disables event handling depending on this

            toolTipTextProvider: function (value) {
                // the real value i.e 'Player.volume' bounds can be different
                // from bounds of this widget and we want to show the current
                // volume here, so directly use Player.volume instead of "value"

                return Math.round(Player.volume * 100) + "%"
            }

            Accessible.name: qsTr("Volume")
            AccessibleCompat.id: "volumeSlider"

            Keys.onPressed: (event) => {
                if (KeyHelper.matchOk(event)) {
                    event.accepted = true

                    _keyPressed = true
                } else {
                    Navigation.defaultKeyAction(event)
                }
            }

            Keys.onReleased: (event) => {
                if (_keyPressed === false)
                    return

                _keyPressed = false

                if (KeyHelper.matchOk(event)) {
                    Player.muted = !Player.muted
                }
            }

            function _syncVolumeWithPlayer() {
                volControl._inhibitPlayerVolumeUpdate = true
                volControl.value = _player.volume
                volControl._inhibitPlayerVolumeUpdate = false
            }

            function _adjustPlayerVolume() {
                Player.muted = false

                let value = volControl.value

                // NOTE: We are clamping the value to make it easier to restore the default volume.
                if (_shiftPressed === false) {
                    if (Math.abs(value - 1.0) < _clamp)
                        value = 1.0
                    else
                        _clamp = 0.01
                }

                Player.volume = value

                volControl.value = value
            }

            Component.onCompleted: {
                root.paintOnlyChanged.connect(_syncVolumeWithPlayer)
                volControl._syncVolumeWithPlayer()
            }

            Connections {
                target: Player
                enabled: !paintOnly

                function onVolumeChanged() { volControl._syncVolumeWithPlayer() }
            }

            Binding on Widgets.PointingToolTipAttached.visible {
                when: sliderMouseArea.pressed
                value: true
            }

            Navigation.leftItem: volumeBtn
            Navigation.parentItem: root

            Keys.onUpPressed: (event) => {
                Player.muted = false
                Player.setVolumeUp()
            }

            Keys.onDownPressed: (event) => {
                Player.muted = false
                Player.setVolumeDown()
            }

            Keys.priority: Keys.BeforeItem

            onValueChanged: {
                if (paintOnly)
                    return

                if (!volControl._inhibitPlayerVolumeUpdate) {
                    if (filterEvents) {
                        Qt.callLater(volControl._adjustPlayerVolume)
                    } else {
                        volControl._adjustPlayerVolume()
                        filterEvents = true
                    }
                }
            }

            Rectangle{
                id: tickmark

                parent: volControl.background

                x : parent.width * root._fullvolpos
                width: VLCStyle.dp(1, VLCStyle.scale)
                height: parent.height
                radius: VLCStyle.dp(2, VLCStyle.scale)

                // NOTE: This shouldn't be visible when the volume stops before a 100.
                // VLC redesign: the meter gradient conveys the level, no tick mark
                visible: false

                // FIXME: tick mark is not visible when it's over slider
                color: root.sliderColor

                // NOTE: This is a helper to select the default volume when clicking on the
                //       tickmark. We apply a higher clamp value to achieve that behavior on
                //       the Slider.
                MouseArea {
                    anchors.fill: parent

                    anchors.margins: -(VLCStyle.dp(4, VLCStyle.scale))

                    onPressed: (mouse) => {
                        mouse.accepted = false

                        if (mouse.modifiers === Qt.ShiftModifier)
                            return

                        volControl._clamp = 0.1
                    }
                }
            }

            MouseArea {
                id: sliderMouseArea

                anchors.fill: parent

                acceptedButtons: (Qt.LeftButton | Qt.RightButton)

                onPressed: (mouse) => {
                    if (root.paintOnly) {
                        mouse.accepted = true
                        return
                    }

                    volControl._shiftPressed = (mouse.modifiers === Qt.ShiftModifier)

                    if (mouse.button === Qt.LeftButton) {
                        mouse.accepted = false

                        return
                    }

                    volControl.filterEvents = false
                    adjustVolume(mouse)
                }

                onPositionChanged: (mouse) => {
                    if (mouse.buttons & Qt.RightButton)
                        adjustVolume(mouse)
                }

                onWheel: (wheel) => {

                    if ((Math.abs(wheel.pixelDelta.x) % 120 > 0) || (Math.abs(wheel.pixelDelta.y) % 120 > 0)) {
                        let delta = 0
                        if (Math.abs(wheel.pixelDelta.x) > Math.abs(wheel.pixelDelta.y))
                            delta = wheel.pixelDelta.x
                        else
                            delta = wheel.pixelDelta.y

                        if (wheel.inverted)
                            delta = -delta
                        volControl.value += 0.001 * delta
                    }
                    else
                        wheelToVLC.qmlWheelEvent(wheel)

                    wheel.accepted = true
                }

                function adjustVolume(mouse) {
                    mouse.accepted = true

                    const pos = mouse.x * root._maxvolpos / width

                    if (pos < 0.25)
                        volControl.value = 0
                    else if (pos < 0.75)
                        volControl.value = 0.5
                    else if (pos < 1.125)
                        volControl.value = 1
                    else
                        volControl.value = 1.25
                }

                WheelToVLCConverter {
                    id: wheelToVLC

                    function handle(steps: int) {
                        if (steps > 0)
                            Player.setVolumeUp(steps)
                        else
                            Player.setVolumeDown(-steps)
                    }

                    Component.onCompleted: {
                        wheelUpDown.connect(wheelToVLC.handle)
                        wheelLeftRight.connect(wheelToVLC.handle)
                    }
                }
            }
        }
    }
}
