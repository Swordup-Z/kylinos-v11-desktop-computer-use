# kylinos-v11-desktop-computer-use

Lightweight X11/XTest computer-use helper for KylinOS Desktop V11 UI verification.

It sends real pointer and keyboard events to the active X session. Start GUI apps with
`QT_QPA_PLATFORM=xcb` when they need deterministic window-manager control.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Commands

```bash
./build/kylinos-v11-desktop-computer-use screen
./build/kylinos-v11-desktop-computer-use position
./build/kylinos-v11-desktop-computer-use move 600 320
./build/kylinos-v11-desktop-computer-use click 600 320
./build/kylinos-v11-desktop-computer-use double-click 600 320
./build/kylinos-v11-desktop-computer-use key Tab
./build/kylinos-v11-desktop-computer-use type "hello"
```

Mouse buttons follow X11 numbering: `1` left, `2` middle, `3` right.
