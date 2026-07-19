/* AutoMsg - Sends a configurable message 1s after joining a server */
/* Configure by adding this to options.txt: auto-msg=Your message here */
#include "../src/PluginAPI.h"
#include "../src/Game.h"
#include "../src/Chat.h"
#include "../src/Event.h"
#include "../src/Options.h"
#include "../src/String_.h"

static struct ScheduledTask2 delayTask;
static cc_string msgBuffer;
static char msgData[STRING_SIZE];
static cc_bool taskAdded;

static cc_bool DelayedSend(struct ScheduledTask2* task) {
	Chat_Send(&msgBuffer, false);
	task->interval = 1e10f;
	taskAdded = false;
	return false;
}

static void OnConnected(void* obj) {
	cc_string raw; char rawBuffer[STRING_SIZE];

	if (taskAdded) return;

	String_InitArray(raw, rawBuffer);
	Options_Get("auto-msg", &raw, "Hi everyone!");
	if (!raw.length) return;

	msgBuffer.buffer  = msgData;
	msgBuffer.length  = 0;
	msgBuffer.capacity = sizeof(msgData);
	String_AppendString(&msgBuffer, &raw);

	delayTask.accumulator = 0;
	delayTask.interval    = 1.0f;
	delayTask.callback    = DelayedSend;
	delayTask.next        = NULL;

	ScheduledTask2_Add(&delayTask);
	taskAdded = true;
}

static void AutoMsg_ShowSettings(void) {
	cc_string msg; char msgBuffer[STRING_SIZE];

	String_InitArray(msg, msgBuffer);
	Options_Get("auto-msg", &msg, "(not set)");

	Chat_AddRaw("&e=== AutoMsg Settings ===");
	Chat_Add1("&eCurrent message: &f%s", &msg);
	Chat_AddRaw("&eTo change, edit options.txt and add:");
	Chat_AddRaw("&7auto-msg=Your message here");
}

static void AutoMsg_Init(void) {
	Event_Register_(&NetEvents.Connected, NULL, OnConnected);
}

PLUGIN_EXPORT int Plugin_ApiVersion = 1;
PLUGIN_EXPORT struct IGameComponent Plugin_Component = { AutoMsg_Init };
PLUGIN_EXPORT void Plugin_Settings(void) { AutoMsg_ShowSettings(); }
