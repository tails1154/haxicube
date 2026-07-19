#include "Core.h"
#include "String_.h"
#define INTERNAL_PLUGIN_HDR
#include "PluginAPI.h"

struct StringsBuffer Plugin_Names;
int Plugin_Count;
PluginSettingsFunc Plugin_SettingsCallbacks[PLUGIN_MAX_CALLBACKS];

#ifdef CC_BUILD_PLUGINS
#include "Game.h"
#include "Utils.h"
#include "Logger.h"
#include "Chat.h"
#include "Platform.h"

static void LoadPlugin(const cc_string* path, void* obj, int isDirectory) {
	void* lib;
	void* verSym;  /* EXPORT int Plugin_ApiVersion = GAME_API_VER; */
	void* compSym; /* EXPORT struct IGameComponent Plugin_Component = { (whatever) } */
	void* setSym;
	int ver, slash;
	int index;
	cc_string name;
	if (isDirectory) return;

	/* ignore accepted.txt, deskop.ini, .pdb files, etc */
	if (!String_CaselessEnds(path, &DynamicLib_Ext)) return;
	/* don't try to load 32 bit plugins on 64 bit OS or vice versa */
	if (sizeof(void*) == 4 && String_ContainsConst(path, "_64.")) return;
	if (sizeof(void*) == 8 && String_ContainsConst(path, "_32.")) return;

	lib = DynamicLib_Load2(path);
	if (!lib) { Logger_DynamicLibWarn("loading", path); return; }

	verSym  = DynamicLib_Get2(lib, "Plugin_ApiVersion");
	if (!verSym)  { Logger_DynamicLibWarn("getting version of", path); return; }
	compSym = DynamicLib_Get2(lib, "Plugin_Component");
	if (!compSym) { Logger_DynamicLibWarn("initing", path); return; }

	ver = *((int*)verSym);
	if (ver < GAME_API_VER) {
		Chat_Add1("&c%s plugin is outdated! Try getting a more recent version.", path);
		return;
	} else if (ver > GAME_API_VER) {
		Chat_Add1("&cYour game is too outdated to use %s plugin! Try updating it.", path);
		return;
	}

	Game_AddComponent((struct IGameComponent*)compSym);

	/* track plugin name for mod list */
	slash = String_LastIndexOf(path, '/');
	if (slash < 0) slash = String_LastIndexOf(path, '\\');
	if (slash >= 0) {
		name.buffer   = path->buffer + slash + 1;
		name.length   = path->length - slash - 1;
		name.capacity = name.length;
	} else {
		name = *path;
	}
	StringsBuffer_Add(&Plugin_Names, &name);
	Plugin_Count = Plugin_Names.count;

	/* check for optional Plugin_Settings symbol */
	index = Plugin_Count - 1;
	if (index < PLUGIN_MAX_CALLBACKS) {
		setSym = DynamicLib_Get2(lib, "Plugin_Settings");
		Plugin_SettingsCallbacks[index] = (PluginSettingsFunc)setSym;
	}
}

void Plugins_LoadAll(void) {
	static const cc_string dir = String_FromConst("plugins");
	int i;
	cc_result res;

	StringsBuffer_Init(&Plugin_Names);
	Plugin_Count = 0;
	for (i = 0; i < PLUGIN_MAX_CALLBACKS; i++) {
		Plugin_SettingsCallbacks[i] = NULL;
	}

	Utils_EnsureDirectory("plugins");
	res = Directory_Enum(&dir, NULL, LoadPlugin);
	if (res) Logger_SysWarn(res, "enumerating plugins directory");
}
#else
void Plugins_LoadAll(void) {
	int i;
	StringsBuffer_Init(&Plugin_Names);
	Plugin_Count = 0;
	for (i = 0; i < PLUGIN_MAX_CALLBACKS; i++) {
		Plugin_SettingsCallbacks[i] = NULL;
	}
}
#endif

