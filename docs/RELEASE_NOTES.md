# VLC Remake

**VLC media player with a completely redesigned interface**, based on the community "VLC Redesign" Figma design and VLC 4.0.
Interface redesigned by **[Abhishek Panthee](https://abhishekpanthee.com.np)** ([GitHub](https://github.com/abhishekpanthee)).

> Unofficial project, not made or endorsed by VideoLAN. VLC media player is © VideoLAN and the VLC authors, GNU GPL v2 or later.

## Download

| Your system | Download |
|---|---|
| **Windows** 10 / 11 (64-bit) | [**vlc-remake-windows-x64-setup.exe**](https://github.com/abhishekpanthee/VLC-Remake/releases/latest/download/vlc-remake-windows-x64-setup.exe) |
| **macOS** 12+, Apple Silicon (M1, M2, M3, M4, M5 and later) | [**vlc-remake-macos-arm64.dmg**](https://github.com/abhishekpanthee/VLC-Remake/releases/latest/download/vlc-remake-macos-arm64.dmg) |
| **macOS** 12+, Intel | [**vlc-remake-macos-x86_64.dmg**](https://github.com/abhishekpanthee/VLC-Remake/releases/latest/download/vlc-remake-macos-x86_64.dmg) |
| **Ubuntu** 24.04, Linux Mint 22, Pop!_OS 24.04 | [**vlc-remake-linux-amd64.deb**](https://github.com/abhishekpanthee/VLC-Remake/releases/latest/download/vlc-remake-linux-amd64.deb) |
| **Any other Linux** (64-bit) | [**vlc-remake-linux-x86_64.AppImage**](https://github.com/abhishekpanthee/VLC-Remake/releases/latest/download/vlc-remake-linux-x86_64.AppImage) |

Which Mac do I have? Apple menu > **About This Mac**: "Chip: Apple M..." is Apple Silicon, "Processor: Intel..." is Intel.

Also available: `vlc-remake-macos-*.app.zip` (the app without a disk image), `vlc-remake-linux-x86_64.tar.gz` (folder for Ubuntu 24.04) and `SHA256SUMS.txt` (checksums).

## Install

**Windows:** run the installer. If you see "Windows protected your PC", click **More info > Run anyway**.

**macOS:** open the `.dmg` and **drag VLC onto Applications**. The first time, **right-click VLC > Open > Open**.

**Ubuntu, Linux Mint, Pop!_OS:**

```
sudo apt install ./vlc-remake-linux-amd64.deb
```

**Other Linux:**

```
chmod +x vlc-remake-linux-x86_64.AppImage
./vlc-remake-linux-x86_64.AppImage
```

The apps are not code signed, which is why your system asks for confirmation the first time.

## What's new

- **macOS**: the redesigned interface on Mac, for Apple Silicon and Intel
- **Linux**: video plays inside the player window on X11 and Wayland, new AppImage for any distribution, `.deb` for Ubuntu
- **Windows**: a single installer
- **No automatic updates**: VLC's own updater would replace VLC Remake with the official VLC
- **About** now credits the redesign, **Help > Check for Updates** explains where new versions are published
- Fixed missing icons on Linux

## The redesign

New colors and Inter typeface (dark and light themes), new icon set, single-row player bar with inline seek bar, 15-second jumps and a wedge volume meter, music player with a rounded cover and waveform seek bar, title bar and icon rail, track panel with video settings, settings with 13 categories, new Add Subtitles dialog, onboarding with theme and accent color.

## Stay updated

VLC Remake does not update itself. Click **Watch > Custom > Releases** on the [repository](https://github.com/abhishekpanthee/VLC-Remake) to be notified of new versions.

## Credits

**VLC media player** by [VideoLAN](https://www.videolan.org) and the VLC authors. **Design**: "VLC Redesign (Community)" on the Figma Community. **Typeface**: [Inter](https://rsms.me/inter/) by Rasmus Andersson. **Redesign**: [Abhishek Panthee](https://abhishekpanthee.com.np).

Licensed under the GNU GPL v2 or later, like VLC. Full details in the [README](https://github.com/abhishekpanthee/VLC-Remake#readme).
