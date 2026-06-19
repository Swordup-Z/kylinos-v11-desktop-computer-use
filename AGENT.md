# Project Agent Notes

This project provides a lightweight computer-use CLI for KylinOS V11 desktop UI verification.

## Development Rules

- Keep this project independent from system-repair skills and application repositories.
- Prefer stable system libraries over custom platform code.
- Use X11/XTest for real mouse and keyboard simulation in XCB/X11 verification sessions.
- Keep the tool small and CLI-first; do not add GUI, browser engines, or heavyweight runtimes.
- Commands should be deterministic and scriptable so other projects can use the tool in verification workflows.
- The tool is for development verification only. Do not use it to bypass user consent or operate unrelated user applications.

## Expected Use

```bash
kylinos-v11-desktop-computer-use screen
kylinos-v11-desktop-computer-use position
kylinos-v11-desktop-computer-use click <x> <y>
kylinos-v11-desktop-computer-use key Tab
```
