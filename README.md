# kylinos-v11-desktop-computer-use

Lightweight `uinput` computer-use helper for KylinOS Desktop V11 UI verification.

The tool creates a temporary Linux virtual input device and sends pointer and
keyboard events through the normal kernel input stack. Commands are intentionally
relative/current-pointer based so they can work beyond X11/Xwayland. Absolute
window coordinates, screenshots, and window inspection are separate capture or
compositor problems and are not part of this input helper.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Commands

```bash
./build/kylinos-v11-desktop-computer-use move 30 -10
./build/kylinos-v11-desktop-computer-use move 300 0 1200
./build/kylinos-v11-desktop-computer-use down
./build/kylinos-v11-desktop-computer-use up
./build/kylinos-v11-desktop-computer-use click
./build/kylinos-v11-desktop-computer-use double-click
./build/kylinos-v11-desktop-computer-use drag 260 100
./build/kylinos-v11-desktop-computer-use drag 600 0 1 1500
./build/kylinos-v11-desktop-computer-use scroll -3
./build/kylinos-v11-desktop-computer-use key Tab
./build/kylinos-v11-desktop-computer-use type "hello"
./build/kylinos-v11-desktop-computer-use script down 1 move 300 0 1200 up 1
./build/kylinos-v11-desktop-computer-use sleep 250
```

Mouse buttons use `1` left, `2` middle, `3` right. Scroll steps use positive
values for down and negative values for up.

The process needs write access to `/dev/uinput`.

Keyboard commands send real global key events to the current focused application.
During verification, prefer an event monitor such as `evtest` or a disposable
test window before running `key` or `type`.

On Fcitx5 desktops, switch to direct input before keyboard verification:

```bash
fcitx5-remote -c
```

## Demos

Draw and fill a visible yellow five-point star in `gpaint` for screen recording:

```bash
tests/draw-star-demo.sh
```

Useful recording overrides:

```bash
STAR_SEGMENT_MS=1500 STAR_RADIUS=210 tests/draw-star-demo.sh
```
