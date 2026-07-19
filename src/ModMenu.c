#include "Menus.h"
#include "Widgets.h"
#include "Gui.h"
#include "Drawer2D.h"
#include "Graphics.h"
#include "String_.h"
#include "Funcs.h"
#include "Screens.h"
#include "PluginAPI.h"

#define MODSCREEN_MAX_BTNS 8

static struct ModScreen {
	Screen_Body
	struct FontDesc titleFont, textFont;
	struct TextWidget title;
	struct ButtonWidget btns[MODSCREEN_MAX_BTNS];
	struct ButtonWidget done;
	struct Widget* __widgets[1 + MODSCREEN_MAX_BTNS + 1];
} ModScreen;

static void ModScreen_PluginClick(void* screen, void* widget) {
	struct ButtonWidget* btn = (struct ButtonWidget*)widget;
	int index = btn->meta.val;

	if (index >= 0 && index < Plugin_Count && index < PLUGIN_MAX_CALLBACKS) {
		if (Plugin_SettingsCallbacks[index]) {
			Plugin_SettingsCallbacks[index]();
		}
	}
}

static void ModScreen_GoBack(void* screen, void* widget) {
	Gui_ShowPauseMenu();
}

static void ModScreen_ContextRecreated(void* screen) {
	struct ModScreen* s = (struct ModScreen*)screen;
	struct FontDesc titleFont;
	int i, count = Plugin_Count;

	Gui_MakeTitleFont(&titleFont);
	Gui_MakeBodyFont(&s->textFont);
	Screen_UpdateVb(screen);

	TextWidget_SetConst(&s->title, "Mods", &titleFont);
	ButtonWidget_SetConst(&s->done, "Done", &titleFont);

	if (count == 0) {
		ButtonWidget_SetConst(&s->btns[0], "No mods loaded.", &s->textFont);
		Widget_SetDisabled(&s->btns[0], true);
		for (i = 1; i < MODSCREEN_MAX_BTNS; i++) {
			Widget_SetDisabled(&s->btns[i], true);
		}
	} else {
		if (count > MODSCREEN_MAX_BTNS) count = MODSCREEN_MAX_BTNS;
		for (i = 0; i < count; i++) {
			cc_string plugin = StringsBuffer_UNSAFE_Get(&Plugin_Names, i);
			ButtonWidget_Set(&s->btns[i], &plugin, &s->textFont);
			s->btns[i].meta.val = i;
			if (!Plugin_SettingsCallbacks[i]) {
				Widget_SetDisabled(&s->btns[i], true);
			} else {
				Widget_SetDisabled(&s->btns[i], false);
			}
		}
		for (i = count; i < MODSCREEN_MAX_BTNS; i++) {
			Widget_SetDisabled(&s->btns[i], true);
		}
	}

	Font_Free(&titleFont);
}

static void ModScreen_Layout(void* screen) {
	struct ModScreen* s = (struct ModScreen*)screen;
	int i, y, count = Plugin_Count;

	if (count == 0) count = 1;
	if (count > MODSCREEN_MAX_BTNS) count = MODSCREEN_MAX_BTNS;

	Widget_SetLocation(&s->title, ANCHOR_CENTRE, ANCHOR_CENTRE, 0, -120);

	for (i = 0; i < count; i++) {
		y = -70 + i * 30;
		Widget_SetLocation(&s->btns[i], ANCHOR_CENTRE, ANCHOR_CENTRE, 0, y);
	}

	Menu_LayoutBack(&s->done);
}

static void ModScreen_Init(void* screen) {
	struct ModScreen* s = (struct ModScreen*)screen;
	int i;

	s->widgets    = s->__widgets;
	s->numWidgets = 0;
	s->maxWidgets = Array_Elems(s->__widgets);
	s->selectedI  = -1;

	TextWidget_Add(s, &s->title);
	for (i = 0; i < MODSCREEN_MAX_BTNS; i++) {
		ButtonWidget_Add(s, &s->btns[i], 300, ModScreen_PluginClick);
		s->btns[i].meta.val = i;
		Widget_SetDisabled(&s->btns[i], true);
	}
	ButtonWidget_Add(s, &s->done, 400, ModScreen_GoBack);

	s->maxVertices = Screen_CalcDefaultMaxVertices(s);
}

static void ModScreen_ContextLost(void* screen) {
	struct ModScreen* s = (struct ModScreen*)screen;
	Font_Free(&s->titleFont);
	Font_Free(&s->textFont);
	Screen_ContextLost(screen);
}

static const struct ScreenVTABLE ModScreen_VTABLE = {
	ModScreen_Init,   Screen_NullUpdate, Screen_NullFunc,
	MenuScreen_Render2, Screen_BuildMesh,
	Menu_InputDown,   Screen_InputUp,    Screen_TKeyPress, Screen_TText,
	Menu_PointerDown, Screen_PointerUp,  Menu_PointerMove, Screen_TMouseScroll,
	ModScreen_Layout, ModScreen_ContextLost, ModScreen_ContextRecreated,
	NULL
};

void ModScreen_Show(void) {
	struct ModScreen* s = &ModScreen;
	s->grabsInput = true;
	s->closable   = true;
	s->VTABLE     = &ModScreen_VTABLE;
	Gui_Add((struct Screen*)s, GUI_PRIORITY_MENU);
}
