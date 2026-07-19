/* CoordMod - Press F6 to print your coordinates to chat */
#include "../src/PluginAPI.h"
#include "../src/Game.h"
#include "../src/Chat.h"
#include "../src/Event.h"
#include "../src/Input.h"
#include "../src/Entity.h"
#include "../src/String_.h"
#include "../src/Funcs.h"

static void OnKeyDown(void* obj, int key, cc_bool repeating, struct InputDevice* device) {
	struct LocalPlayer* p;
	cc_string msg; char msgBuffer[STRING_SIZE];
	int heading;
	static const char* headingNames[8] = {
		"South", "SW", "West", "NW", "North", "NE", "East", "SE"
	};

	if (repeating) return;
	if (key != CCKEY_F6) return;

	p = Entities.CurPlayer;
	if (!p) return;

	String_InitArray(msg, msgBuffer);
	String_AppendConst(&msg, "&ePosition: &f");
		String_AppendFloat(&msg, p->Base.Position.x, 2);
		String_AppendConst(&msg, " &f");
		String_AppendFloat(&msg, p->Base.Position.y, 2);
		String_AppendConst(&msg, " &f");
		String_AppendFloat(&msg, p->Base.Position.z, 2);
	Chat_Add(&msg);

	heading = ((int)(p->Base.Yaw / 45.0f + 0.5f) & 7);
	String_InitArray(msg, msgBuffer);
	String_AppendConst(&msg, "&eFacing: &f");
	String_AppendFloat(&msg, p->Base.Yaw, 1);
	String_AppendConst(&msg, " (Yaw) / ");
	String_AppendFloat(&msg, p->Base.Pitch, 1);
	String_AppendConst(&msg, " (Pitch) &7[");
	String_AppendConst(&msg, headingNames[heading]);
	String_AppendConst(&msg, "]");
	Chat_Add(&msg);
}

static void CoordMod_Init(void) {
	Event_Register_(&InputEvents.Down2, NULL, OnKeyDown);
}

PLUGIN_EXPORT int Plugin_ApiVersion = 1;
PLUGIN_EXPORT struct IGameComponent Plugin_Component = { CoordMod_Init };
