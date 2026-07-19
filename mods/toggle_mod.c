/* ToggleMod - F7: Fly, F8: Noclip, F9: Speed */
#include "../src/PluginAPI.h"
#include "../src/Game.h"
#include "../src/Chat.h"
#include "../src/Event.h"
#include "../src/Input.h"
#include "../src/Entity.h"
#include "../src/EntityComponents.h"
#include "../src/String_.h"
#include "../src/Funcs.h"

static void PrintToggle(const char* name, cc_bool on) {
	cc_string msg; char msgBuffer[STRING_SIZE];
	String_InitArray(msg, msgBuffer);
	String_AppendConst(&msg, "&e");
	String_AppendConst(&msg, name);
	String_AppendConst(&msg, ": ");
	String_AppendConst(&msg, on ? "&aON" : "&cOFF");
	Chat_Add(&msg);
}

static void OnKeyDown(void* obj, int key, cc_bool repeating, struct InputDevice* device) {
	struct LocalPlayer* p;

	if (repeating) return;
	p = Entities.CurPlayer;
	if (!p) return;

	if (key == CCKEY_F7) {
		if (!p->Hacks.Enabled) { Chat_AddRaw("&cHacks not enabled"); return; }
		if (!p->Hacks.CanFly)  { Chat_AddRaw("&cFlying not allowed"); return; }

		HacksComp_SetFlying(&p->Hacks, !p->Hacks.Flying);
		PrintToggle("Fly", p->Hacks.Flying);
	} else if (key == CCKEY_F8) {
		if (!p->Hacks.Enabled)  { Chat_AddRaw("&cHacks not enabled"); return; }
		if (!p->Hacks.CanNoclip) { Chat_AddRaw("&cNoclip not allowed"); return; }

		HacksComp_SetNoclip(&p->Hacks, !p->Hacks.Noclip);
		PrintToggle("Noclip", p->Hacks.Noclip);
	} else if (key == CCKEY_F9) {
		if (!p->Hacks.Enabled) { Chat_AddRaw("&cHacks not enabled"); return; }
		if (!p->Hacks.CanSpeed) { Chat_AddRaw("&cSpeed not allowed"); return; }

		p->Hacks.Speeding = !p->Hacks.Speeding;
		PrintToggle("Speed", p->Hacks.Speeding);
	}
}

static void ToggleMod_Init(void) {
	Event_Register_(&InputEvents.Down2, NULL, OnKeyDown);
}

static void ToggleMod_ShowSettings(void) {
	Chat_AddRaw("&e=== ToggleMod Keybinds ===");
	Chat_AddRaw("&eF7: &fToggle Fly");
	Chat_AddRaw("&eF8: &fToggle Noclip");
	Chat_AddRaw("&eF9: &fToggle Speed");
}

PLUGIN_EXPORT int Plugin_ApiVersion = 1;
PLUGIN_EXPORT struct IGameComponent Plugin_Component = { ToggleMod_Init };
PLUGIN_EXPORT void Plugin_Settings(void) { ToggleMod_ShowSettings(); }
