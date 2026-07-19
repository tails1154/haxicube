# Mods System

ClassiCube has a built-in mod (plugin) system that allows loading external shared libraries at runtime. This document covers the mod menu, the plugin API, and how to create mods with settings.

## Architecture

Mods are `.so`/`.dll`/`.dylib` files placed in the `plugins/` directory. The game loads them at startup via `Plugins_LoadAll()`.

Each mod exports two **required** symbols and one **optional** symbol:

| Symbol | Type | Required | Description |
|---|---|---|---|
| `Plugin_ApiVersion` | `int` | Yes | Must be `1` (`GAME_API_VER`) |
| `Plugin_Component` | `struct IGameComponent` | Yes | Lifecycle callbacks |
| `Plugin_Settings` | `void (*)(void)` | No | Opens mod settings screen |

## The Mod Menu

Press **Escape** → click **Mods...** to open the mod menu. It lists all loaded plugins.

- **Enabled (clickable)** entries — the plugin exports `Plugin_Settings`. Clicking calls the settings function.
- **Disabled (grayed out)** entries — no settings available.

The settings function can print to chat, open an input overlay, or do anything the plugin needs.

## Creating a Mod

### Minimal Plugin

```c
#include "src/PluginAPI.h"
#include "src/Chat.h"
#include "src/String_.h"

static void MyMod_Init(void) {
    cc_string msg = String_FromConst("Hello from my mod!");
    Chat_Add(&msg);
}

PLUGIN_EXPORT int Plugin_ApiVersion = 1;
PLUGIN_EXPORT struct IGameComponent Plugin_Component = { MyMod_Init };
```

### Plugin with Settings

```c
#include "src/PluginAPI.h"
#include "src/Chat.h"

static void MyMod_Init(void) {
    /* called when game starts */
}

static void MyMod_Settings(void) {
    Chat_AddRaw("&e=== My Mod Settings ===");
    Chat_AddRaw("&eOption 1: &fvalue");
    Chat_AddRaw("&eTo change, edit options.txt");
}

PLUGIN_EXPORT int Plugin_ApiVersion = 1;
PLUGIN_EXPORT struct IGameComponent Plugin_Component = { MyMod_Init };
PLUGIN_EXPORT void Plugin_Settings(void) { MyMod_Settings(); }
```

### Full Lifecycle Example

```c
#include "src/PluginAPI.h"
#include "src/Game.h"
#include "src/Chat.h"
#include "src/Event.h"
#include "src/Input.h"
#include "src/Entity.h"
#include "src/EntityComponents.h"
#include "src/Options.h"
#include "src/String_.h"
#include "src/Funcs.h"

static void OnKeyDown(void* obj, int key, cc_bool repeating, struct InputDevice* device) {
    if (repeating) return;
    if (key == CCKEY_F6) {
        struct LocalPlayer* p = Entities.CurPlayer;
        if (p) {
            cc_string msg; char buf[STRING_SIZE];
            String_InitArray(msg, buf);
            String_Format3(&msg, "&ePos: &f%f2, %f2, %f2",
                &p->Base.Position.x, &p->Base.Position.y, &p->Base.Position.z);
            Chat_Add(&msg);
        }
    }
}

static void MyMod_Init(void) {
    Event_Register_(&InputEvents.Down2, NULL, OnKeyDown);
}

static void MyMod_Settings(void) {
    Chat_AddRaw("&e=== My Mod ===");
    Chat_AddRaw("&eF6: &fPrint coordinates");
}

PLUGIN_EXPORT int Plugin_ApiVersion = 1;
PLUGIN_EXPORT struct IGameComponent Plugin_Component = { MyMod_Init };
PLUGIN_EXPORT void Plugin_Settings(void) { MyMod_Settings(); }
```

## IGameComponent Reference

```c
struct IGameComponent {
    void (*Init)(void);              /* Game starting */
    void (*Free)(void);              /* Game closing */
    void (*Reset)(void);             /* Reconnecting to server */
    void (*OnNewMap)(void);          /* New map begins loading */
    void (*OnNewMapLoaded)(void);    /* New map finished loading */
    struct IGameComponent* next;     /* Internal linked list */
};
```

Only `Init` is required. All others can be `NULL`.

## Events

Events use the type-safe registration macros:

```c
Event_Register_(&EventStruct, obj, Callback);
Event_Unregister_(&EventStruct, obj, Callback);
```

Where `obj` is an opaque pointer passed to the callback (can be `NULL`).

### InputEvents

```c
/* Key/button pressed */
void callback(void* obj, int key, cc_bool repeating, struct InputDevice* device);
Event_Register_(&InputEvents.Down2, NULL, callback);

/* Key/button released */
Event_Register_(&InputEvents.Up2, NULL, callback);

/* Mouse wheel */
void callback(void* obj, float delta);
Event_Register_(&InputEvents.Wheel, NULL, callback);
```

Key constants: `CCKEY_F1` through `CCKEY_F12` (290-301), `CCKEY_ESCAPE` (256), etc.

### ChatEvents

```c
/* Intercept received messages (set msg->length = 0 to block) */
void callback(void* obj, const cc_string* msg, int msgType);
Event_Register_(&ChatEvents.ChatReceived, NULL, callback);

/* Intercept messages being sent */
Event_Register_(&ChatEvents.ChatSending, NULL, callback);
```

### NetEvents

```c
/* Connection established */
void callback(void* obj);
Event_Register_(&NetEvents.Connected, NULL, callback);

/* Connection lost */
Event_Register_(&NetEvents.Disconnected, NULL, callback);
```

### WorldEvents

```c
/* New map started loading */
void callback(void* obj);
Event_Register_(&WorldEvents.NewMap, NULL, callback);

/* Map fully loaded */
void callback(void* obj);
Event_Register_(&WorldEvents.MapLoaded, NULL, callback);
```

### PointerEvents

```c
/* Cursor moved */
void callback(void* obj, float xDelta, float yDelta);
Event_Register_(&PointerEvents.RawMoved, NULL, callback);

/* Left click */
void callback(void* obj, int pointerIndex);
Event_Register_(&PointerEvents.Down, NULL, callback);
```

### UserEvents

```c
void callback(void* obj);
Event_Register_(&UserEvents.HackPermsChanged, NULL, callback);
Event_Register_(&UserEvents.HeldBlockChanged, NULL, callback);
Event_Register_(&UserEvents.HacksStateChanged, NULL, callback);
```

## Chat API

```c
/* Send to server */
void    Chat_Send(const cc_string* text, cc_bool logUsage);

/* Add local message */
void    Chat_Add(const cc_string* text);
void    Chat_AddOf(const cc_string* text, int msgType);
CC_API void Chat_AddRaw(const char* raw);  /* const char* convenience */

/* Formatted messages (%s = cc_string*, %i = int*, %f2 = float*) */
void    Chat_Add1(const char* format, const void* a1);
void    Chat_Add2(const char* format, const void* a1, const void* a2);
```

## Options API

```c
cc_bool Options_GetBool(const char* key, cc_bool def);
int     Options_GetInt(const char* key, int min, int max, int def);
void    Options_Get(const char* key, cc_string* value, const char* def);
void    Options_SetBool(const char* key, cc_bool value);
void    Options_SetInt(const char* key, int value);
```

## Hacks API

```c
void HacksComp_SetFlying(struct HacksComp* hacks, cc_bool flying);
void HacksComp_SetNoclip(struct HacksComp* hacks, cc_bool noclip);
void HacksComp_Update(struct HacksComp* hacks);  /* Re-evaluate permissions */
```

Access via `Entities.CurPlayer->Hacks`:

```c
struct LocalPlayer* p = Entities.CurPlayer;
p->Hacks.Enabled = 1;                          /* Enable hacks */
p->Hacks.Speeding = !p->Hacks.Speeding;       /* Toggle speed */
HacksComp_SetFlying(&p->Hacks, !p->Hacks.Flying);
HacksComp_SetNoclip(&p->Hacks, !p->Hacks.Noclip);
```

## Delayed Actions (ScheduledTask2)

For one-shot delayed execution:

```c
#include "src/Game.h"

static struct ScheduledTask2 delayTask;

static cc_bool OnDelayed(struct ScheduledTask2* task) {
    Chat_AddRaw("&a1 second later!");
    task->interval = 1e10f;  /* prevent repeat */
    return false;
}

static void ScheduleOneShot(float seconds) {
    delayTask.accumulator = 0;
    delayTask.interval    = seconds;
    delayTask.callback    = OnDelayed;
    delayTask.next        = NULL;
    ScheduledTask2_Add(&delayTask);
}
```

Important: `ScheduledTask2` is a **repeating** timer. The return value is ignored. Set `task->interval` to a large value to stop it.

## Entity/Player Access

```c
struct LocalPlayer* p = Entities.CurPlayer;

/* Position */
float x = p->Base.Position.x;
float y = p->Base.Position.y;
float z = p->Base.Position.z;

/* Rotation */
float yaw   = p->Base.Yaw;
float pitch = p->Base.Pitch;

/* Velocity */
float vx = p->Base.Velocity.x;
```

## Build Instructions

### Linux

```sh
cc mymod.c -o mymod.so -shared -fPIC -I. -I./src
cp mymod.so plugins/
```

### macOS

```sh
cc mymod.c -o mymod.dylib -undefined dynamic_lookup -I. -I./src
cp mymod.dylib plugins/
```

### Windows (MinGW)

```sh
gcc mymod.c -o mymod.dll -s -shared -I. -I./src
cp mymod.dll plugins/
```

## Example Mods

The `mods/` directory in the source tree contains ready-to-build examples:

| File | Keybinds | Description |
|---|---|---|
| `mods/coord_mod.c` | F6 | Prints coordinates, yaw, pitch, heading to chat |
| `mods/toggle_mod.c` | F7/F8/F9 | Toggles Fly/Noclip/Speed with permission checks |
| `mods/auto_msg.c` | — | Sends a configurable message 1s after joining a server |

Build all with:

```sh
make -C mods
```

This produces `.so` files in `plugins/`.

## Portability Notes

- All mods must be **C89 compatible** (no `//` comments, declarations before statements).
- Use `PLUGIN_EXPORT` for exported symbols.
- `PluginAPI.h` must be the first include.
- Functions/structs marked `CC_API`/`CC_VAR` are available to plugins (with `-rdynamic` on Linux/macOS; with `.lib` on Windows).
- Static functions in the game source are NOT accessible — use only `CC_API` functions.
