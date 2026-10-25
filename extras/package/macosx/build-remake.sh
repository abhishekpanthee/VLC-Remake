#!/bin/sh
# VLC Remake: build the macOS packages on a Mac.
#
#   extras/package/macosx/build-remake.sh          # Apple Silicon (arm64)
#   extras/package/macosx/build-remake.sh intel    # Intel (x86_64)
#
# Needs Xcode (with the Metal toolchain: xcodebuild -downloadComponent MetalToolchain)
# and Python >= 3.10 (brew install python@3.12). Builds the contribs (Qt
# included) from source the first time, reuses them afterwards.
#
# Result, in dist/ at the root of the repository:
#   vlc-remake-macos-<arch>.dmg      disk image
#   vlc-remake-macos-<arch>.app.zip  the application, zipped
set -e

case "${1:-arm64}" in
    arm64|aarch64) ARCH=aarch64; NAME=arm64;  BUILD_DIR=build-mac ;;
    intel|x86_64)  ARCH=x86_64;  NAME=x86_64; BUILD_DIR=build-mac-intel ;;
    *) echo "usage: $0 [arm64|intel]"; exit 1 ;;
esac

ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
BUILD="$ROOT/$BUILD_DIR"
JOBS="$(sysctl -n hw.ncpu)"

# Qt 6.8 and recent Xcode need macOS 12 or later
export VLC_MINIMAL_OSX_VERSION="${VLC_MINIMAL_OSX_VERSION:-12.0}"

# build.sh resets PATH: give it a python >= 3.10 through VLC_PATH
if [ -z "$VLC_PATH" ]; then
    for p in /opt/homebrew/opt/python@3.12/libexec/bin /usr/local/opt/python@3.12/libexec/bin \
             /opt/homebrew/opt/python@3.13/libexec/bin /usr/local/opt/python@3.13/libexec/bin; do
        if [ -x "$p/python3" ]; then VLC_PATH="$p"; break; fi
    done
fi
if [ -z "$VLC_PATH" ]; then
    echo "Python >= 3.10 not found: brew install python@3.12"; exit 1
fi
export VLC_PATH

echo "== VLC Remake for macOS $NAME, in $BUILD"
mkdir -p "$BUILD"
cd "$BUILD"

# contribs (Qt included), configure and compile; with recent Xcode the
# compilation can stop on the Objective-C files, finished below
"$ROOT/extras/package/macosx/build.sh" -c -a "$ARCH" -j "$JOBS" \
    || echo "== build.sh stopped, finishing the compilation"

test -f Makefile || { echo "VLC was not configured, see the errors above"; exit 1; }
grep -q "^enable_qt='yes'" config.log || { echo "Qt was not found: the redesigned interface would be missing"; exit 1; }

# configure loses the SDK of the Objective-C compilers with recent Xcode
SDK="$(xcrun --sdk macosx --show-sdk-path)"
FIX="OBJC=$(xcrun --find clang) -isysroot $SDK -mmacosx-version-min=$VLC_MINIMAL_OSX_VERSION"
FIXXX="OBJCXX=$(xcrun --find clang++) -isysroot $SDK -mmacosx-version-min=$VLC_MINIMAL_OSX_VERSION"

make -j "$JOBS" "$FIX" "$FIXXX"
make "$FIX" "$FIXXX" VLC.app
test -f VLC.app/Contents/Frameworks/plugins/libqt_plugin.dylib \
    || { echo "libqt_plugin.dylib is missing from VLC.app"; exit 1; }

rm -f vlc-*.dmg
make "$FIX" "$FIXXX" package-macosx

mkdir -p "$ROOT/dist"
cp vlc-*.dmg "$ROOT/dist/vlc-remake-macos-$NAME.dmg"
rm -f "$ROOT/dist/vlc-remake-macos-$NAME.app.zip"
ditto -c -k --keepParent VLC.app "$ROOT/dist/vlc-remake-macos-$NAME.app.zip"

echo
echo "== Done:"
ls -lh "$ROOT/dist/vlc-remake-macos-$NAME.dmg" "$ROOT/dist/vlc-remake-macos-$NAME.app.zip"
