# Credits

## VLC media player

This project is a modified version of **VLC media player**.

- Copyright © 1996-2026 VideoLAN and the VLC authors.
- Website: <https://www.videolan.org>, source: <https://code.videolan.org/videolan/vlc>.
- Contributors: see [AUTHORS](AUTHORS) and [THANKS](THANKS).
- License: GNU GPL v2 or later ([COPYING](COPYING)); libVLC: GNU LGPL v2.1 or later ([COPYING.LIB](COPYING.LIB)).

The source code imported in the first commit of this repository is VLC's upstream commit
`d460ecd` ("demux: image: hand off animated GIFs to libavformat"). Everything except the
user interface changes listed below is VideoLAN's and the VLC authors' work.

"VLC", "VideoLAN" and the VLC cone logo are trademarks of VideoLAN. This project is not
affiliated with or endorsed by VideoLAN.

## Interface redesign

- **Design**: "VLC Redesign (Community)", a community design published on the Figma Community.
- **Implementation**: Abhishek Panthee, in `modules/gui/qt` (see the git history).
- **Typeface**: Inter by Rasmus Andersson, SIL Open Font License 1.1.
- **Icons**: from the redesign Figma file, converted to the VLC icon font.

## Changes compared to VLC

The changes are limited to the Qt interface (`modules/gui/qt`), plus:

- the integration test suite (`test/`) and VideoLAN's GitLab CI configuration were removed,
- a GitHub Actions workflow builds Windows and macOS packages.

As required by the GNU GPL, modified files carry their original copyright notices, and the
complete corresponding source code is this repository.
