# KylinOS V11 Desktop Computer Use

This project provides a lightweight computer-use CLI for KylinOS V11 desktop UI verification.

## Scope

- Real mouse movement and clicks.
- Real keyboard input.
- Relative/current-pointer input for cross-session verification.
- Scriptable UI verification support for other local desktop projects.

## Development Rules

- Keep this project independent from system-repair skills and application repositories.
- Prefer stable system libraries over custom platform code.
- Use Linux `uinput` for real mouse and keyboard simulation because it goes through the kernel input stack and works beyond X11/Xwayland.
- Keep the tool small and CLI-first; do not add GUI, browser engines, or heavyweight runtimes.
- Commands should be deterministic and scriptable so other projects can use the tool in verification workflows.
- The tool is for development verification only. Do not use it to bypass user consent or operate unrelated user applications.
- Screenshot, window inspection, and absolute coordinate targeting are separate capture/compositor capabilities; do not add X11-specific shortcuts as the default design.
- When another local desktop project needs a missing verification capability, implement and verify the needed command here first instead of weakening that project's verification. Common required commands include relative movement, drag, scrolling, right-click, clicks, keyboard input, and short sleeps.

## Expected Use

```bash
kylinos-v11-desktop-computer-use move <dx> <dy>
kylinos-v11-desktop-computer-use click
kylinos-v11-desktop-computer-use drag <dx> <dy>
kylinos-v11-desktop-computer-use key Tab
```

## Git

- Use author `Swordup-Z <swordup.zeng@gmail.com>`.
- Commit project changes after verification.
- This repository currently has no remote; do not assume push is available until the user provides a GitHub repository URL.
