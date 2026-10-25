# VLC Remake

A redesigned interface for **[VLC media player](https://www.videolan.org/vlc/)**, implementing the
community **"VLC Redesign"** Figma design on top of VLC 4.0 (development version).

> **This is an unofficial, independent project.** It is not made, endorsed or supported by
> VideoLAN. VLC media player is developed by [VideoLAN](https://www.videolan.org) and the VLC
> authors; all the media playback, codecs, streaming and the rest of the player are their work.
> "VLC", "VideoLAN" and the VLC cone logo are trademarks of VideoLAN. Get the official VLC at
> <https://www.videolan.org>.

![Player](docs/screenshots/player.png)

## What is redesigned

Only the user interface (`modules/gui/qt`) is changed:

- **Design tokens**: the redesign colors (dark and light themes), the Inter typeface, sizes and radii.
- **Icons**: the redesign icon set (123 glyphs), including new ones.
- **Player**: single row control bar with the inline seek bar, accent play button, wedge volume
  meter, 15 seconds jumps; music player with a rounded cover and a waveform style seek bar;
  floating resume prompt; mini player.
- **Title bar and navigation rail**, library lists and home sections.
- **Track panel**: audio, subtitles and video tracks, video settings (zoom, aspect ratio, crop...).
- **Settings**: sidebar with the 13 categories, toggles, redesigned keyboard shortcuts.
- **Dialogs**: Customize Player, Add Subtitles (new), Open Network Stream, New Playlist.
- **Onboarding**: welcome, theme and accent color, keyboard shortcuts.

| | |
|---|---|
| ![Music](docs/screenshots/music.png) | ![Library](docs/screenshots/library.png) |
| ![Settings](docs/screenshots/settings.png) | ![Track panel](docs/screenshots/track-panel.png) |
| ![Onboarding](docs/screenshots/onboarding.png) | ![Add subtitles](docs/screenshots/add-subtitles.png) |

## Downloads

Windows and macOS builds are produced by GitHub Actions (see `.github/workflows/build.yml`) and
attached to the [releases](../../releases). The builds are **not code signed**:

- **Windows**: SmartScreen may warn about an unknown publisher ("More info" > "Run anyway").
- **macOS**: right click the app > "Open" the first time, or run
  `xattr -dr com.apple.quarantine "/Applications/VLC.app"`.

## Building

The build is the same as VLC's, see [INSTALL](INSTALL) and the
[VLC developer documentation](https://wiki.videolan.org/Developers_Corner).

Linux quick start (Qt >= 6.2, meson >= 1.10):

```sh
meson setup build -Dqt=enabled
ninja -C build
./build/bin/vlc
```

The library views need [medialibrary](https://code.videolan.org/videolan/medialibrary) >= 0.13.

## Credits

- **VLC media player**: © VideoLAN and the VLC authors, see [AUTHORS](AUTHORS) and [THANKS](THANKS).
  This project is based on the VLC source code (upstream commit `d460ecd`), see [README-VLC.md](README-VLC.md).
- **Design**: the "VLC Redesign (Community)" file published on the Figma Community.
- **Typeface**: [Inter](https://rsms.me/inter/) by Rasmus Andersson, SIL Open Font License
  (`modules/gui/qt/pixmaps/fonts/LICENSE`).
- See [CREDITS.md](CREDITS.md) for details.

## License

Same as VLC: the VLC media player is licensed under the **GNU General Public License v2 or later**
([COPYING](COPYING)), libVLC under the **GNU Lesser General Public License v2.1 or later**
([COPYING.LIB](COPYING.LIB)). The changes made in this project are released under the same licenses.
