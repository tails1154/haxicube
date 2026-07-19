# AGENTS.md

## Project

ClassiCube is a Minecraft Classic client written in C89. Single flat `src/` directory with platform-specific subdirectories. Third-party code in `third_party/` (bearssl, tinyalloc, freetype vendored in `src/freetype/`).

## Build Commands

**Linux** (requires `libx11-dev`, `libxi-dev`, `libgl1-mesa-dev`):
```
make linux              # debug build
make linux RELEASE=1    # optimized build
```

**Windows** (MinGW-w64): `make mingw` or `make mingw RELEASE=1`

**macOS**: `make darwin` or `make darwin RELEASE=1`

**Web** (emscripten): `make web` or `make web RELEASE=1`

**SDL2/SDL3**: `make sdl2` / `make sdl3`

**Terminal** (software renderer): `make terminal`

**Clean**: `make clean` (when it's the only goal)

All platform makefiles live in `misc/makefiles/`. Console ports have their own Makefiles under `misc/<platform>/`.

## Key Compiler Flags

- `-DCC_BUILD_GL11` — force OpenGL 1.1 support (must be using `CC_GFX_BACKEND_GL1`)
- `-DCC_GFX_BACKEND=CC_GFX_BACKEND_GL2` — modern OpenGL shaders
- `-DCC_WIN_BACKEND=CC_WIN_BACKEND_SDL2` — use SDL2 window backend
- `-DCC_BUILD_GLES` — make GL2 shaders GLES-compatible

Backends are auto-detected in `Core.h`. Define `CC_BUILD_MANUAL` to override all detection.

## Code Style

- **C89 compatible**, also compilable as C++
- Each `.c` file = one module
- Public symbols prefixed by module name (e.g. `Game_Reset`, `Chat_Add`)
- Private functions: PascalCase, optional module prefix
- Custom string type `cc_string` (not `char*`). `STRING_REF` annotation = caller must not free the buffer. `_UNSAFE_` in function name usually means it returns a `STRING_REF` reference.
- Warnings are errors: `-Werror` in default CFLAGS (with `-Wno-error=missing-braces -Wno-error=strict-aliasing`)

## Architecture

- `src/Game.c` — main game loop and state
- `src/Launcher.c` — launcher loop (separate from game)
- `src/Platform_Posix.c` / `src/Platform_Windows.c` — OS abstraction
- `src/Graphics_GL1.c`, `Graphics_GL2.c`, `Graphics_D3D9.c`, etc. — rendering backends
- `src/Window_X11.c`, `Window_Win.c`, `Window_SDL2.c` — window backends
- `src/Audio_OpenAL.c`, `Audio_WinMM.c`, etc. — audio backends
- Modules documented in `doc/modules.md`

## Testing

No test suite. Verification = compile and run. CI builds for 30+ platforms in `.github/workflows/`.

## Gotchas

- No package manager, no npm, no cargo — pure C with Makefiles only
- `doc/style.md`, `doc/modules.md`, `doc/portability.md` have useful context for code changes
- Plugin API exported via `CC_API`/`CC_VAR` macros; see `doc/plugin-dev.md`
- Mod system adds built-in mod menu, plugin settings API; see `doc/mods.md`
- Build Windows cross-compile: `make -f misc/makefiles/windows.mk CC=i686-w64-mingw32-gcc`
- Build mods for Windows: need `libClassiCube.a` from `gendef`/`dlltool`, then link with `-lClassiCube -Wl,--allow-multiple-definition`
- Android/iOS builds use separate build systems (`misc/android/`, `misc/ios/`)
- `third_party/bearssl/` is compiled directly into the binary for console SSL support
