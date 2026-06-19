# KylinOS V11 Desktop Computer Use

This project provides a lightweight computer-use CLI for KylinOS V11 desktop UI verification.

## Scope

- Real mouse movement and clicks.
- Real keyboard input.
- Screen and pointer inspection.
- Scriptable UI verification support for other local desktop projects.

## Development Rules

- Keep this project independent from system-repair skills and application repositories.
- Prefer stable system libraries over custom platform code.
- Use X11/XTest for real mouse and keyboard simulation in XCB/X11 verification sessions.
- Keep the tool small and CLI-first; do not add GUI, browser engines, or heavyweight runtimes.
- Commands should be deterministic and scriptable so other projects can use the tool in verification workflows.
- The tool is for development verification only. Do not use it to bypass user consent or operate unrelated user applications.
- If screenshot support is added later, prefer lightweight X11/system image APIs or existing system screenshot tools before adding heavy dependencies.

## Expected Use

```bash
kylinos-v11-desktop-computer-use screen
kylinos-v11-desktop-computer-use position
kylinos-v11-desktop-computer-use click <x> <y>
kylinos-v11-desktop-computer-use key Tab
```

## Git

- Use author `Swordup-Z <swordup.zeng@gmail.com>`.
- Commit project changes after verification.
- This repository currently has no remote; do not assume push is available until the user provides a GitHub repository URL.
