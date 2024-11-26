#include <windows.h>

#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include <global.h>
#include <patch.h>
#include <config.h>
#include <event.h>
#include <log.h>
#include <cfunc.h>

typedef struct {
	uint32_t vtablePtr;
	uint32_t node;
	uint32_t type;
	uint32_t port;
	// 16
	uint32_t slot;
	uint32_t isValid;
	uint32_t unk_24;
	uint8_t controlData[32];	// PS2 format control data
	uint8_t vibrationData_align[32];
	uint8_t vibrationData_direct[32];
	uint8_t vibrationData_max[32];
	uint8_t vibrationData_oldDirect[32];    // there may be something before this
	// 160
	// 176
	uint32_t unk5;
	// 192
	uint32_t actuatorsDisabled;
	uint32_t capabilities;
	uint32_t unk6;
	uint32_t num_actuators;
	// 208
	uint32_t unk7;
	uint32_t state;
	uint32_t unk8;
	uint32_t index;
	// 224
	uint32_t isPluggedIn;
	uint32_t unplugged_counter;
	uint32_t unplugged_retry;
	uint32_t pressed;
	// 238
	uint32_t start_or_a_pressed;
} device;

struct keybinds {
	SDL_Scancode menu;
	SDL_Scancode cameraToggle;
	SDL_Scancode cameraSwivelLock;
	SDL_Scancode focus;
	SDL_Scancode caveman;

	SDL_Scancode grind;
	SDL_Scancode grab;
	SDL_Scancode ollie;
	SDL_Scancode kick;

	SDL_Scancode leftSpin;
	SDL_Scancode rightSpin;
	SDL_Scancode nollie;
	SDL_Scancode switchRevert;

	SDL_Scancode right;
	SDL_Scancode left;
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode feeble;

	SDL_Scancode cameraRight;
	SDL_Scancode cameraLeft;
	SDL_Scancode cameraUp;
	SDL_Scancode cameraDown;

	SDL_Scancode itemRight;
	SDL_Scancode itemLeft;
	SDL_Scancode itemUp;
	SDL_Scancode itemDown;
};

// a recreation of the SDL_GameControllerButton enum, but with the addition of right/left trigger
typedef enum {
	CONTROLLER_UNBOUND = -1,
	CONTROLLER_BUTTON_A = SDL_CONTROLLER_BUTTON_A,
	CONTROLLER_BUTTON_B = SDL_CONTROLLER_BUTTON_B,
	CONTROLLER_BUTTON_X = SDL_CONTROLLER_BUTTON_X,
	CONTROLLER_BUTTON_Y = SDL_CONTROLLER_BUTTON_Y,
	CONTROLLER_BUTTON_BACK = SDL_CONTROLLER_BUTTON_BACK,
	CONTROLLER_BUTTON_GUIDE = SDL_CONTROLLER_BUTTON_GUIDE,
	CONTROLLER_BUTTON_START = SDL_CONTROLLER_BUTTON_START,
	CONTROLLER_BUTTON_LEFTSTICK = SDL_CONTROLLER_BUTTON_LEFTSTICK,
	CONTROLLER_BUTTON_RIGHTSTICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
	CONTROLLER_BUTTON_LEFTSHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
	CONTROLLER_BUTTON_RIGHTSHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
	CONTROLLER_BUTTON_DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
	CONTROLLER_BUTTON_DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
	CONTROLLER_BUTTON_DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
	CONTROLLER_BUTTON_DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
	CONTROLLER_BUTTON_MISC1 = SDL_CONTROLLER_BUTTON_MISC1,
	CONTROLLER_BUTTON_PADDLE1 = SDL_CONTROLLER_BUTTON_PADDLE1,
	CONTROLLER_BUTTON_PADDLE2 = SDL_CONTROLLER_BUTTON_PADDLE2,
	CONTROLLER_BUTTON_PADDLE3 = SDL_CONTROLLER_BUTTON_PADDLE3,
	CONTROLLER_BUTTON_PADDLE4 = SDL_CONTROLLER_BUTTON_PADDLE4,
	CONTROLLER_BUTTON_TOUCHPAD = SDL_CONTROLLER_BUTTON_TOUCHPAD,
	CONTROLLER_BUTTON_RIGHTTRIGGER = 21,
	CONTROLLER_BUTTON_LEFTTRIGGER = 22,
} controllerButton;

typedef enum {
	CONTROLLER_STICK_UNBOUND = -1,
	CONTROLLER_STICK_LEFT = 0,
	CONTROLLER_STICK_RIGHT = 1
} controllerStick;

struct controllerbinds {
	controllerButton menu;
	controllerButton cameraToggle;
	controllerButton cameraSwivelLock;
	controllerButton focus;
	controllerButton caveman;
	// TODO: add spine button

	controllerButton grind;
	controllerButton grab;
	controllerButton ollie;
	controllerButton kick;

	controllerButton leftSpin;
	controllerButton rightSpin;
	controllerButton nollie;
	controllerButton switchRevert;

	controllerButton right;
	controllerButton left;
	controllerButton up;
	controllerButton down;

	controllerStick movement;
	controllerStick camera;
};

struct inputsettings {
	uint8_t isPs2Controls;
	uint8_t uicontrols;
};

void patchPs2Buttons();

uint8_t *addr_platform = (void *)(0x00452226);	// 83 f8 07 56 c7 44 24 04 00 00 00 00
uint8_t *addr_r2l2_air = (void *)(0x0056c00d);	// 8a 93 80 00 00 00 84 d2 74 0a
uint8_t *addr_r2l2_break_vert1 = (void *)(0x00565488);	// 8a 87 80 00 00 00 84 c0 74 0a
uint8_t *addr_r2l2_break_vert2 = (void *)(0x005654a2);	// 8a 87 80 00 00 00 84 c0 74 0e
uint8_t *addr_r2l2_lip = (void *)(0x00567816);	// 8a 87 80 00 00 00 84 c0 0f 84 92 00 00 00
uint8_t *addr_r2l2_air_recover = (void *)(0x0056bb6f);	// 8a 83 80 00 00 00 84 c0 74 11
uint8_t *addr_r2l2_groundair = (void *)(0x00569359);	// 8a 85 80 00 00 00 84 c0 74 0a
uint8_t *addr_r2l2_groundair_acid1 = (void *)(0x005692ed);	// 8a 85 80 00 00 00 84 c0 74 3a
uint8_t *addr_r2l2_acid_drop = (void *)(0x0056caa8);	// 8a 88 80 00 00 00 84 c9 74 0a
uint8_t *addr_r2l2_walk_acid1 = (void *)(0x00463773);	// 8a 88 a0 00 00 00 83 c0 20 84 c9 0f 84 cb 00 00 00
uint8_t *addr_r2l2_walk_acid2 = (void *)(0x00463843);	// 8a 88 a0 00 00 00 83 c0 20 84 c9 0f 84 cb 00 00 00

uint8_t *addr_isKeyboardOnScreen = 0x0076a7a6;	// use key_input (+ 1)
uint8_t *addr_isInMenu = 0x0076a7a7;	// use key_input (+ 1)
void (*key_input)(int32_t key, uint32_t param) = (void *)0x00581bf0;	// a0 ?? ?? ?? ?? 84 c0 75 1a a0 ?? ?? ?? ?? 84 c0 0f 84 a9 00 00 00
uint8_t *unk2 = 0x00769130;	// use key_input (+10)
uint8_t *unk3 = 0x0076a7a7;	// use key_input (+23)
uint8_t *addr_device_processs = 0x00581a20; // 8b 81 d8 00 00 00 48 c6 81 e4 00 00 00 00
uint8_t *addr_call_device_read = 0x00581732; // e8 ?? ?? ?? ?? 8b 8e d8 00 00 00 b8 02 00 00 00
uint8_t *addr_set_actuators = 0x005c4200; // 83 ec 44 55 56 8b 74 24 50
uint8_t *addr_init_dinput = 0x005c56ab; // 6a 00 68 ?? ?? ?? ?? 68 ?? ?? ?? ?? 68 00 08 00 00 50 e8
uint8_t *addr_deinit_dinput = 0x005c4190; // e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? e8 ?? ?? ?? ?? 6a 01 6a 00 6a 00
//uint8_t *addr_unfocuspoll = 0x005f56e0;

int controllerCount;
int controllerListSize;
SDL_GameController **controllerList;
struct inputsettings inputsettings;
struct keybinds keybinds;
struct controllerbinds padbinds;

uint8_t isUsingKeyboard = 1;

struct playerslot {
	SDL_GameController *controller;
	uint8_t lockedOut;	// after sign-in, a controller is "locked out" until all of its buttons are released
};

#define MAX_PLAYERS 2
uint8_t numplayers = 0;
struct playerslot players[MAX_PLAYERS] = { { NULL, 0 }, { NULL, 0 } };

void setUsingKeyboard(uint8_t usingKeyboard) {
	isUsingKeyboard = usingKeyboard;
}

uint8_t getUsingKeyboard() {
	return isUsingKeyboard;
}

void addController(int idx) {
	log_printf(LL_INFO, "Detected controller \"%s\"\n", SDL_GameControllerNameForIndex(idx));

	SDL_GameController *controller = SDL_GameControllerOpen(idx);

	SDL_GameControllerSetPlayerIndex(controller, -1);

	if (controller) {
		if (controllerCount == controllerListSize) {
			int tmpSize = controllerListSize + 1;
			SDL_GameController **tmp = realloc(controllerList, sizeof(SDL_GameController *) * tmpSize);
			if (!tmp) {
				return; // TODO: log something here or otherwise do something
			}

			controllerListSize = tmpSize;
			controllerList = tmp;
		}

		controllerList[controllerCount] = controller;
		controllerCount++;
	}
}

void addplayer(SDL_GameController *controller) {
	if (numplayers < 2) {
		// find open slot
		uint8_t found = 0;
		int i = 0;
		for (; i < MAX_PLAYERS; i++) {
			if (!players[i].controller) {
				found = 1;
				break;
			}
		}
		if (found) {
			SDL_GameControllerSetPlayerIndex(controller, i);
			players[i].controller = controller;

			if (numplayers > 0) {
				players[i].lockedOut = 1;
				SDL_JoystickRumble(SDL_GameControllerGetJoystick(controller), 0x7fff, 0x7fff, 250);
			}
			
			numplayers++;

			log_printf(LL_INFO, "Added player %d: %s\n", i + 1, SDL_GameControllerName(controller));
		}
	} else {
		log_printf(LL_INFO, "Already two players, not adding\n");
	}
}

void pruneplayers() {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (players[i].controller && !SDL_GameControllerGetAttached(players[i].controller)) {
			log_printf(LL_INFO, "Pruned player %d\n", i + 1);

			players[i].controller = NULL;
			numplayers--;
			log_printf(LL_INFO, "Remaining players: %d\n", numplayers);
		}
	}
}

void removeController(SDL_GameController *controller) {
	log_printf(LL_INFO, "Controller \"%s\" disconnected\n", SDL_GameControllerName(controller));

	int i = 0;

	while (i < controllerCount && controllerList[i] != controller) {
		i++;
	}

	if (controllerList[i] == controller) {
		SDL_GameControllerClose(controller);

		int playerIdx = SDL_GameControllerGetPlayerIndex(controller);
		if (playerIdx != -1) {
			log_printf(LL_INFO, "Removed player %d\n", playerIdx + 1);
			players[playerIdx].controller = NULL;
			numplayers--;
		}

		pruneplayers();

		for (; i < controllerCount - 1; i++) {
			controllerList[i] = controllerList[i + 1];
		}
		controllerCount--;
	} else {
		//setActiveController(NULL);
		log_printf(LL_WARN, "Did not find disconnected controller in list\n");
	}
}

void initSDLControllers() {
	log_printf(LL_INFO, "Initializing Controller Input\n");

	controllerCount = 0;
	controllerListSize = 1;
	controllerList = malloc(sizeof(SDL_GameController *) * controllerListSize);

	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			addController(i);
		}
	}

	// add event filter for newly connected controllers
	//SDL_SetEventFilter(controllerEventFilter, NULL);
}

uint8_t axisAbs(uint8_t val) {
	if (val > 127) {
		// positive, just remove top bit
		return val & 0x7F;
	} else {
		// negative
		return ~val & 0x7f;
	}
}

uint8_t getButton(SDL_GameController *controller, controllerButton button) {
	if (button == CONTROLLER_BUTTON_LEFTTRIGGER) {
		uint8_t pressure = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >> 7;
		return pressure > 0x80;
	} else if (button == CONTROLLER_BUTTON_RIGHTTRIGGER) {
		uint8_t pressure = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >> 7;
		return pressure > 0x80;
	} else {
		return SDL_GameControllerGetButton(controller, button);
	}
}

void getStick(SDL_GameController *controller, controllerStick stick, uint8_t *xOut, uint8_t *yOut) {
	uint8_t result_x, result_y;

	if (stick == CONTROLLER_STICK_LEFT) {
		result_x = (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) >> 8) + 128);
		result_y = (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) >> 8) + 128);
	} else if (stick == CONTROLLER_STICK_RIGHT) {
		result_x = (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) >> 8) + 128);
		result_y = (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) >> 8) + 128);
	} else {
		result_x = 0x80;
		result_y = 0x80;
	}

	if (axisAbs(result_x) > axisAbs(*xOut)) {
		*xOut = result_x;
	}

	if (axisAbs(result_y) > axisAbs(*yOut)) {
		*yOut = result_y;
	}
}

void pollController(device *dev, SDL_GameController *controller) {
	if (SDL_GameControllerGetAttached(controller)) {
		dev->isValid = 1;
		dev->isPluggedIn = 1;

		// buttons
		if (getButton(controller, padbinds.menu)) {
			dev->controlData[2] |= 0x01 << 3;
		}
		if (getButton(controller, padbinds.cameraToggle)) {
			dev->controlData[2] |= 0x01 << 0;
		}
		if (getButton(controller, padbinds.cameraSwivelLock)) {
			dev->controlData[2] |= 0x01 << 2;
		}
		if (getButton(controller, padbinds.focus)) {
			dev->controlData[2] |= 0x01 << 1;
		}

		if (getButton(controller, padbinds.grind)) {
			dev->controlData[3] |= 0x01 << 4;
			dev->controlData[12] = 0xff;
		}
		if (getButton(controller, padbinds.grab)) {
			dev->controlData[3] |= 0x01 << 5;
			dev->controlData[13] = 0xff;
		}
		if (getButton(controller, padbinds.ollie)) {
			dev->controlData[3] |= 0x01 << 6;
			dev->controlData[14] = 0xff;
		}
		if (getButton(controller, padbinds.kick)) {
			dev->controlData[3] |= 0x01 << 7;
			dev->controlData[15] = 0xff;
		}

		// shoulders
		if (inputsettings.isPs2Controls) {
			if (getButton(controller, padbinds.leftSpin)) {
				dev->controlData[3] |= 0x01 << 2;
				dev->controlData[16] = 0xff;
			}

			if (getButton(controller, padbinds.rightSpin)) {
				dev->controlData[3] |= 0x01 << 3;
				dev->controlData[17] = 0xff;
			}

			if (getButton(controller, padbinds.nollie)) {
				dev->controlData[3] |= 0x01 << 0;
				dev->controlData[18] = 0xff;
			}

			if (getButton(controller, padbinds.switchRevert)) {
				dev->controlData[3] |= 0x01 << 1;
				dev->controlData[19] = 0xff;
			}
		} else {
			if (getButton(controller, padbinds.nollie)) {
				dev->controlData[3] |= 0x01 << 2;
				dev->controlData[16] = 0xff;
				dev->controlData[3] |= 0x01 << 0;
				dev->controlData[18] = 0xff;
			}

			if (getButton(controller, padbinds.switchRevert)) {
				dev->controlData[3] |= 0x01 << 3;
				dev->controlData[17] = 0xff;
				dev->controlData[3] |= 0x01 << 1;
				dev->controlData[19] = 0xff;
			}

			if (getButton(controller, padbinds.leftSpin)) {
				dev->controlData[20] |= 0x01 << 1;
			}

			if (getButton(controller, padbinds.rightSpin)) {
				dev->controlData[20] |= 0x01 << 0;
			}
		}
		
		// d-pad
		if (SDL_GameControllerGetButton(controller, padbinds.up)) {
			dev->controlData[2] |= 0x01 << 4;
			dev->controlData[10] = 0xFF;
		}
		if (SDL_GameControllerGetButton(controller, padbinds.right)) {
			dev->controlData[2] |= 0x01 << 5;
			dev->controlData[8] = 0xFF;
		}
		if (SDL_GameControllerGetButton(controller, padbinds.down)) {
			dev->controlData[2] |= 0x01 << 6;
			dev->controlData[11] = 0xFF;
		}
		if (SDL_GameControllerGetButton(controller, padbinds.left)) {
			dev->controlData[2] |= 0x01 << 7;
			dev->controlData[9] = 0xFF;
		}
		
		// sticks
		getStick(controller, padbinds.camera, &(dev->controlData[4]), &(dev->controlData[5]));
		getStick(controller, padbinds.movement, &(dev->controlData[6]), &(dev->controlData[7]));
	}
}

uint32_t escState = 0;

uint8_t isInParkEditor() {
	void **ScreenElementManager = 0x007078a4;
	uint32_t *(__fastcall *ScreenElementManager_GetElement)(void *, void *, void *, uint32_t, uint32_t) = 0x00490260;
	if (ScreenElementManager) {
		int unk[4] = {0, 0, 0, 0};	// honestly, a complete mystery to me.  this seems to be the same thing returned?  is this some compiler magic for struct returns?

		uint32_t *result = ScreenElementManager_GetElement(*ScreenElementManager, NULL, unk, 0x5999551E, 0);

		if (result && *result) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

void pollKeyboard(device *dev) {
	dev->isValid = 1;
	dev->isPluggedIn = 1;

	uint8_t *keyboardState = SDL_GetKeyboardState(NULL);
	uint8_t isInMenu = *addr_isInMenu && !isInParkEditor();

	// to prevent esc double-pressing on menu transitions, process its state here.  
	// escState = 1 means esc was pressed in menu, 2 means pressed out of menu, 0 is unpressed
	if (keyboardState[SDL_SCANCODE_ESCAPE] && !escState) {
		escState = (isInMenu) ? 1 : 2;
	} else if (!keyboardState[SDL_SCANCODE_ESCAPE]) {
		escState = 0;
	}

	// buttons
	if ((keybinds.menu != SDL_SCANCODE_ESCAPE && keyboardState[keybinds.menu]) || escState == 2) {	// NOTE: menu is also always hardcoded to esc
		dev->controlData[2] |= 0x01 << 3;
	}
	if (keyboardState[keybinds.cameraToggle]) {
		dev->controlData[2] |= 0x01 << 0;
	}
	if (keyboardState[keybinds.focus]) {	// no control for left stick on keyboard
		dev->controlData[2] |= 0x01 << 1;
	}
	if (keyboardState[keybinds.cameraSwivelLock]) {
		dev->controlData[2] |= 0x01 << 2;
	}

	if (keyboardState[keybinds.grind] || (isInMenu && keyboardState[SDL_SCANCODE_R])) {
		dev->controlData[3] |= 0x01 << 4;
		dev->controlData[12] = 0xff;
	}
	if (keyboardState[keybinds.grab] || (isInMenu && escState == 1)) {
		dev->controlData[3] |= 0x01 << 5;
		dev->controlData[13] = 0xff;
	}
	if (keyboardState[keybinds.ollie] || (isInMenu && keyboardState[SDL_SCANCODE_RETURN])) {
		dev->controlData[3] |= 0x01 << 6;
		dev->controlData[14] = 0xff;
	}
	if (keyboardState[keybinds.kick] || (isInMenu && keyboardState[SDL_SCANCODE_E])) {
		dev->controlData[3] |= 0x01 << 7;
		dev->controlData[15] = 0xff;
	}

	// shoulders
	if (inputsettings.isPs2Controls) {
		if (keyboardState[keybinds.leftSpin] || (isInMenu && keyboardState[SDL_SCANCODE_1])) {
			dev->controlData[3] |= 0x01 << 2;
			dev->controlData[16] = 0xff;
		}
		if (keyboardState[keybinds.rightSpin] || (isInMenu && keyboardState[SDL_SCANCODE_2])) {
			dev->controlData[3] |= 0x01 << 3;
			dev->controlData[17] = 0xff;
		}
		if (keyboardState[keybinds.nollie]) {
			dev->controlData[3] |= 0x01 << 0;
			dev->controlData[18] = 0xff;
		}
		if (keyboardState[keybinds.switchRevert]) {
			dev->controlData[3] |= 0x01 << 1;
			dev->controlData[19] = 0xff;
		}
	} else {
		if (keyboardState[keybinds.nollie]) {
			dev->controlData[3] |= 0x01 << 2;
			dev->controlData[16] = 0xff;
			dev->controlData[3] |= 0x01 << 0;
			dev->controlData[18] = 0xff;
		}
		if (keyboardState[keybinds.switchRevert]) {
			dev->controlData[3] |= 0x01 << 3;
			dev->controlData[17] = 0xff;
			dev->controlData[3] |= 0x01 << 1;
			dev->controlData[19] = 0xff;
		}
		if (keyboardState[keybinds.leftSpin]) {
			dev->controlData[20] |= 0x01 << 1;
		}
		if (keyboardState[keybinds.rightSpin]) {
			dev->controlData[20] |= 0x01 << 0;
		}
	}
		
	// d-pad
	if (keyboardState[keybinds.itemUp] || (isInMenu && keyboardState[SDL_SCANCODE_UP])) {
		dev->controlData[2] |= 0x01 << 4;
		dev->controlData[10] = 0xFF;
	}
	if (keyboardState[keybinds.itemRight] || (isInMenu && keyboardState[SDL_SCANCODE_RIGHT])) {
		dev->controlData[2] |= 0x01 << 5;
		dev->controlData[8] = 0xFF;
	}
	if (keyboardState[keybinds.itemDown] || (isInMenu && keyboardState[SDL_SCANCODE_DOWN])) {
		dev->controlData[2] |= 0x01 << 6;
		dev->controlData[11] = 0xFF;
	}
	if (keyboardState[keybinds.itemLeft] || (isInMenu && keyboardState[SDL_SCANCODE_LEFT])) {
		dev->controlData[2] |= 0x01 << 7;
		dev->controlData[9] = 0xFF;
	}

	// sticks - NOTE: because these keys are very rarely used/important, SOCD handling is just to cancel
	// right
	// x
	if (keyboardState[keybinds.cameraLeft] && !keyboardState[keybinds.cameraRight]) {
		dev->controlData[4] = 0;
	}
	if (keyboardState[keybinds.cameraRight] && !keyboardState[keybinds.cameraLeft]) {
		dev->controlData[4] = 255;
	}

	// y
	if (keyboardState[keybinds.cameraUp] && !keyboardState[keybinds.cameraDown]) {
		dev->controlData[5] = 0;
	}
	if (keyboardState[keybinds.cameraDown] && !keyboardState[keybinds.cameraUp]) {
		dev->controlData[5] = 255;
	}

	// left
	// x
	if (keyboardState[keybinds.left] && !keyboardState[keybinds.right]) {
		dev->controlData[6] = 0;
	}
	if (keyboardState[keybinds.right] && !keyboardState[keybinds.left]) {
		dev->controlData[6] = 255;
	}

	// y
	if (keyboardState[keybinds.up] && !keyboardState[keybinds.down]) {
		dev->controlData[7] = 0;
	}
	if (keyboardState[keybinds.down] && !keyboardState[keybinds.up]) {
		dev->controlData[7] = 255;
	}
}

// returns 1 if a text entry prompt is on-screen so that keybinds don't interfere with text entry confirmation/cancellation
uint8_t isKeyboardTyping() {
	return *addr_isKeyboardOnScreen;
}

void __fastcall processController(device *dev, void *pad, device *also_dev) {
	// cheating:
	// replace type with index
	//dev->type = 60;

	//printf("Processing Controller %d %d %d!\n", dev->index, dev->slot, dev->port);
	//printf("TYPE: %d\n", dev->type);
	//printf("ISPLUGGEDIN: %d\n", dev->isPluggedIn);
	dev->capabilities = 0x0003;
	dev->num_actuators = 2;
	dev->vibrationData_max[0] = 255;
	dev->vibrationData_max[1] = 255;
	dev->state = 2;
	dev->actuatorsDisabled = 0;

	handleEvents();

	dev->isValid = 0;
	dev->isPluggedIn = 0;

	dev->controlData[0] = 0;
	dev->controlData[1] = 0x70;

	// buttons bitmap
	// this bitmap may not work how you would expect. each bit is 1 if the button is *up*, not down
	// the original code sets this initial value at 0xff and XOR's each button bit with the map
	// this scheme cannot be continuously composited, so instead we OR each button with the bitmap and bitwise negate it after all controllers are processed
	dev->controlData[2] = 0x00;
	dev->controlData[3] = 0x00;

	// buttons
	dev->controlData[12] = 0;
	dev->controlData[13] = 0;
	dev->controlData[14] = 0;
	dev->controlData[15] = 0;

	// shoulders
	dev->controlData[16] = 0;
	dev->controlData[17] = 0;
	dev->controlData[18] = 0;
	dev->controlData[19] = 0;

	// d-pad
	dev->controlData[8] = 0;
	dev->controlData[9] = 0;
	dev->controlData[10] = 0;
	dev->controlData[11] = 0;

	// sticks
	dev->controlData[4] = 127;
	dev->controlData[5] = 127;
	dev->controlData[6] = 127;
	dev->controlData[7] = 127;

	dev->controlData[20] = 0;

	if (dev->port == 0) {
		dev->isValid = 1;
		dev->isPluggedIn = 1;

		if (!isKeyboardTyping()) {
			pollKeyboard(dev);
		}
	}
	
	if (dev->port < MAX_PLAYERS) {
		if (players[dev->port].controller && !players[dev->port].lockedOut) {
			pollController(dev, players[dev->port].controller);
		}
	}

	dev->controlData[2] = ~dev->controlData[2];
	dev->controlData[3] = ~dev->controlData[3];
	//dev->controlData[20] = ~dev->controlData[20];

	if (0xFFFF ^ ((dev->controlData[2] << 8 ) | dev->controlData[3])) {
		dev->pressed = 1;
	} else {
		dev->pressed = 0;
	}

	if (~dev->controlData[2] & 0x01 << 3 || ~dev->controlData[3] & 0x01 << 6) {
		dev->start_or_a_pressed = 1;
	} else {
		dev->start_or_a_pressed = 0;
	}

	// keyboard text entry doesn't work unless these values are set correctly

	*unk2 = 1;
	*unk3 = 0;

	//printf("UNKNOWN VALUES: 0x0074fb42: %d, 0x00751dc0: %d, 0x0074fb43: %d\n", *unk1, *unk2, *unk3);
}

void __cdecl set_actuators(int port, uint16_t high, uint16_t low) {
	//printf("SETTING ACTUATORS: %d %d %d\n", port, high, low);
	for (int i = 0; i < controllerCount; i++) {
		if (SDL_GameControllerGetAttached(controllerList[i]) && SDL_GameControllerGetPlayerIndex(controllerList[i]) == port) {
			SDL_JoystickRumble(SDL_GameControllerGetJoystick(controllerList[i]), low, high, 0);
		}
	}
}

void do_key_input(SDL_KeyCode key) {
	if (!*addr_isKeyboardOnScreen) {
		if (key == SDLK_RETURN) {
			key_input(0x19, 0);
		} else if (key == SDLK_F1) {
			key_input(0x1f, 0);
		} else if (key == SDLK_F2) {
			key_input(0x1c, 0);
		} else if (key == SDLK_F3) {
			key_input(0x1d, 0);
		} else if (key == SDLK_F4) {
			key_input(0x1e, 0);
		}

		return;
	}

	int32_t key_out = 0;
	uint8_t modstate = SDL_GetModState();
	uint8_t shift = SDL_GetModState() & KMOD_SHIFT;
	uint8_t caps = SDL_GetModState() & KMOD_CAPS;

	if (key == SDLK_RETURN) {
		key_out = 0x0d;	// CR
	} else if (key == SDLK_BACKSPACE) {
		key_out = 0x08;	// BS
	} else if (key == SDLK_ESCAPE) {
		key_out = 0x1b;	// ESC
	} else if (key == SDLK_SPACE) {
		key_out = ' ';
	} else if (key >= SDLK_0 && key <= SDLK_9 && !(modstate & KMOD_SHIFT)) {
		key_out = key;
	} else if (key >= SDLK_a && key <= SDLK_z) {
		key_out = key;
		if (modstate & (KMOD_SHIFT | KMOD_CAPS)) {
			key_out -= 0x20;
		}
	} else if (key == SDLK_PERIOD) {
		if (modstate & KMOD_SHIFT) {
			key_out = '>';
		} else {
			key_out = '.';
		}
	} else if (key == SDLK_COMMA) {
		if (modstate & KMOD_SHIFT) {
			key_out = '<';
		} else {
			key_out = ',';
		}
	} else if (key == SDLK_SLASH) {
		if (modstate & KMOD_SHIFT) {
			key_out = '?';
		} else {
			key_out = '/';
		}
	} else if (key == SDLK_SEMICOLON) {
		if (modstate & KMOD_SHIFT) {
			key_out = ':';
		} else {
			key_out = ';';
		}
	} else if (key == SDLK_QUOTE) {
		if (modstate & KMOD_SHIFT) {
			key_out = '\"';
		} else {
			key_out = '\'';
		}
	} else if (key == SDLK_LEFTBRACKET) {
		if (modstate & KMOD_SHIFT) {
			key_out = '{';
		} else {
			key_out = '[';
		}
	} else if (key == SDLK_RIGHTBRACKET) {
		if (modstate & KMOD_SHIFT) {
			key_out = '}';
		} else {
			key_out = ']';
		}
	} else if (key == SDLK_BACKSLASH) {
		if (modstate & KMOD_SHIFT) {
			key_out = '|';
		} else {
			key_out = '\\';
		}
	} else if (key == SDLK_MINUS) {
		if (modstate & KMOD_SHIFT) {
			key_out = '_';
		} else {
			key_out = '-';
		}
	} else if (key == SDLK_EQUALS) {
		if (modstate & KMOD_SHIFT) {
			key_out = '+';
		} else {
			key_out = '=';
		}
	} else if (key == SDLK_BACKQUOTE) {
		if (modstate & KMOD_SHIFT) {
			key_out = '~';
		} else {
			key_out = '`';
		}
	} else if (key == SDLK_1 && modstate & KMOD_SHIFT) {
		key_out = '!';
	} else if (key == SDLK_2 && modstate & KMOD_SHIFT) {
		key_out = '@';
	} else if (key == SDLK_3 && modstate & KMOD_SHIFT) {
		key_out = '#';
	} else if (key == SDLK_4 && modstate & KMOD_SHIFT) {
		key_out = '$';
	} else if (key == SDLK_5 && modstate & KMOD_SHIFT) {
		key_out = '%';
	} else if (key == SDLK_6 && modstate & KMOD_SHIFT) {
		key_out = '^';
	} else if (key == SDLK_7 && modstate & KMOD_SHIFT) {
		key_out = '&';
	} else if (key == SDLK_8 && modstate & KMOD_SHIFT) {
		key_out = '*';
	} else if (key == SDLK_9 && modstate & KMOD_SHIFT) {
		key_out = '(';
	} else if (key == SDLK_0 && modstate & KMOD_SHIFT) {
		key_out = ')';
	} else if (key == SDLK_KP_0) {
		key_out = '0';
	} else if (key == SDLK_KP_1) {
		key_out = '1';
	} else if (key == SDLK_KP_2) {
		key_out = '2';
	} else if (key == SDLK_KP_3) {
		key_out = '3';
	} else if (key == SDLK_KP_4) {
		key_out = '4';
	} else if (key == SDLK_KP_5) {
		key_out = '5';
	} else if (key == SDLK_KP_6) {
		key_out = '6';
	} else if (key == SDLK_KP_7) {
		key_out = '7';
	} else if (key == SDLK_KP_8) {
		key_out = '8';
	} else if (key == SDLK_KP_9) {
		key_out = '9';
	} else if (key == SDLK_KP_MINUS) {
		key_out = '-';
	} else if (key == SDLK_KP_EQUALS) {
		key_out = '=';
	} else if (key == SDLK_KP_PLUS) {
		key_out = '+';
	} else if (key == SDLK_KP_DIVIDE) {
		key_out = '/';
	} else if (key == SDLK_KP_MULTIPLY) {
		key_out = '*';
	} else if (key == SDLK_KP_DECIMAL) {
		key_out = '.';
	} else if (key == SDLK_KP_ENTER) {
		key_out = 0x0d;
	} else {
		key_out = -1;
	}

	key_input(key_out, 0);
}

void handleInputEvent(SDL_Event *e) {
	switch (e->type) {
		case SDL_CONTROLLERDEVICEADDED:
			if (SDL_IsGameController(e->cdevice.which)) {
				addController(e->cdevice.which);
			} else {
				log_printf(LL_INFO, "Not a game controller: %s\n", SDL_JoystickNameForIndex(e->cdevice.which));
			}
			return;
		case SDL_CONTROLLERDEVICEREMOVED: {
			SDL_GameController *controller = SDL_GameControllerFromInstanceID(e->cdevice.which);
			if (controller) {
				removeController(controller);
			}
			return;
		}
		case SDL_JOYDEVICEADDED:
			log_printf(LL_INFO, "Joystick added: %s\n", SDL_JoystickNameForIndex(e->jdevice.which));
			return;
		case SDL_KEYDOWN: 
			setUsingKeyboard(1);
			do_key_input(e->key.keysym.sym);
			return;
		case SDL_CONTROLLERBUTTONDOWN: {
			SDL_GameController *controller = SDL_GameControllerFromInstanceID(e->cdevice.which);

			int idx = SDL_GameControllerGetPlayerIndex(controller);
			if (idx == -1) {
				addplayer(controller);
			} else if (players[idx].lockedOut) {
				players[idx].lockedOut++;
			}

			setUsingKeyboard(0);
			return;
		}
		case SDL_CONTROLLERBUTTONUP: {
			SDL_GameController *controller = SDL_GameControllerFromInstanceID(e->cdevice.which);

			int idx = SDL_GameControllerGetPlayerIndex(controller);
			if (idx != -1 && players[idx].lockedOut) {
				players[idx].lockedOut--;
			}

			return;
		}
		case SDL_CONTROLLERAXISMOTION:
			setUsingKeyboard(0);
			return;
		default:
			return;
	}
}

// LUT translating from SDL scancodes to directinput keycodes
uint32_t DIK_LUT[256] = {
	-1, -1, -1, -1, 0x1E, 0x30, 0x2E, 0x20,	// 0..7
	0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26,	// 8..15
	0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14,	// 16..23
	0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x02, 0x03,	// 24..31
	0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,	// 32..39
	0x1C, 0x01, 0x0E, 0x0F, 0x39, 0x0C, 0x0D, 0x1A,	// 40..47
	0x1B, 0x2B, 0x2B, 0x27, 0x28, 0x29, 0x33, 0x34,	// 48..55
	0x35, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,	// 56..63
	0x41, 0x42, 0x43, 0x44, 0x57, 0x58, -1, 0x46,	// 64..71
	0xC5, 0xD2, 0xC7, 0xC9, 0xD3, 0xCF, 0xD1, 0xCD,	// 72..79
	0xCB, 0xD0, 0xC8, 0x45, 0xB5, 0x37, 0x4A, 0x4E,	// 80..87
	0x9C, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47,	// 88..95
	0x48, 0x49, 0x52, 0x53, 0x2B, 0xDD, 0xDE, 0x8D,	// 96..103
	0x64, 0x65, 0x66, -1, -1, -1, -1, -1,	// 104..111
	-1, -1, -1, -1, -1, -1, -1, -1,	// 112..119
	-1, -1, -1, -1, -1, -1, -1, -1,	// 120..127
	0xB0, 0xAE, -1, -1, -1, 0xB3, -1, -1,	// 128..135
	-1, -1, -1, -1, -1, -1, -1, -1,	// 136..143
	-1, -1, -1, -1, -1, -1, -1, -1,	// 144..151
	-1, -1, -1, -1, -1, -1, -1, -1,	// 152..159
	-1, -1, -1, -1, -1, -1, -1, -1,	// 160..167
	-1, -1, -1, -1, -1, -1, -1, -1,	// 168..175
	-1, -1, -1, -1, -1, -1, -1, -1,	// 176..183
	-1, -1, -1, -1, -1, -1, -1, -1,	// 184..191
	-1, -1, -1, -1, -1, -1, -1, -1,	// 192..199
	-1, -1, -1, -1, -1, -1, -1, -1,	// 200..207
	-1, -1, -1, -1, -1, -1, -1, -1,	// 208..215
	-1, -1, -1, -1, -1, -1, -1, -1,	// 216..223
	0x1D, 0x2A, 0x38, 0xDB, 0x9D, 0x36, 0xB8, 0xDC,	// 224..231
	-1, -1, -1, -1, -1, -1, -1, -1,	// 232..239
	-1, -1, -1, -1, -1, -1, -1, -1,	// 240..247
	-1, -1, -1, -1, -1, -1, -1, -1,	// 248..255
};

int32_t translateSDLScancodeToDIK(SDL_Scancode scancode) {
	if (scancode < 256) {
		return DIK_LUT[scancode];
	} else {
		return -1;
	}
}

void writeNativeBinds() {
	// configure binds for the purposes of control prompts

	int32_t *binds = 0x007d1140;

	binds[0] = translateSDLScancodeToDIK(keybinds.up);
	binds[1] = translateSDLScancodeToDIK(keybinds.down);
	binds[2] = translateSDLScancodeToDIK(keybinds.left);
	binds[3] = translateSDLScancodeToDIK(keybinds.right);

	binds[4] = translateSDLScancodeToDIK(keybinds.cameraUp);
	binds[5] = translateSDLScancodeToDIK(keybinds.cameraDown);
	binds[6] = translateSDLScancodeToDIK(keybinds.cameraLeft);
	binds[7] = translateSDLScancodeToDIK(keybinds.cameraRight);

	binds[8] = translateSDLScancodeToDIK(keybinds.ollie);
	binds[9] = translateSDLScancodeToDIK(keybinds.grab);
	binds[10] = translateSDLScancodeToDIK(keybinds.kick);
	binds[11] = translateSDLScancodeToDIK(keybinds.grind);

	binds[13] = translateSDLScancodeToDIK(keybinds.cameraToggle);
	binds[14] = translateSDLScancodeToDIK(keybinds.leftSpin);
	binds[15] = translateSDLScancodeToDIK(keybinds.rightSpin);
	binds[16] = translateSDLScancodeToDIK(keybinds.nollie);
	binds[17] = translateSDLScancodeToDIK(keybinds.switchRevert);
	binds[18] = translateSDLScancodeToDIK(keybinds.focus);

	binds[20] = translateSDLScancodeToDIK(keybinds.itemUp);
	binds[21] = translateSDLScancodeToDIK(keybinds.itemDown);
	binds[22] = translateSDLScancodeToDIK(keybinds.itemLeft);
	binds[23] = translateSDLScancodeToDIK(keybinds.itemRight);
}

void loadInputSettings(struct inputsettings *settingsOut) {
	if (settingsOut) {
		settingsOut->isPs2Controls = getConfigBool(CONFIG_MISC_SECTION, "UsePS2Controls", 1);
		settingsOut->uicontrols = (uint8_t)getConfigInt("Miscellaneous", "UIControls", 1);
	}
}

void loadKeyBinds(struct keybinds *bindsOut) {
	if (bindsOut) {
		bindsOut->menu = getConfigInt(CONFIG_KEYBIND_SECTION, "Pause", SDL_SCANCODE_RETURN);
		bindsOut->cameraToggle = getConfigInt(CONFIG_KEYBIND_SECTION, "ViewToggle", SDL_SCANCODE_F);
		bindsOut->cameraSwivelLock = getConfigInt(CONFIG_KEYBIND_SECTION, "SwivelLock", SDL_SCANCODE_GRAVE);
		bindsOut->focus = getConfigInt(CONFIG_KEYBIND_SECTION, "Focus", SDL_SCANCODE_KP_ENTER);
		bindsOut->caveman = getConfigInt(CONFIG_KEYBIND_SECTION, "Caveman", SDL_SCANCODE_E);

		bindsOut->grind = getConfigInt(CONFIG_KEYBIND_SECTION, "Grind", SDL_SCANCODE_KP_8);
		bindsOut->grab = getConfigInt(CONFIG_KEYBIND_SECTION, "Grab", SDL_SCANCODE_KP_6);
		bindsOut->ollie = getConfigInt(CONFIG_KEYBIND_SECTION, "Ollie", SDL_SCANCODE_KP_2);
		bindsOut->kick = getConfigInt(CONFIG_KEYBIND_SECTION, "Flip", SDL_SCANCODE_KP_4);

		bindsOut->leftSpin = getConfigInt(CONFIG_KEYBIND_SECTION, "SpinLeft", SDL_SCANCODE_KP_1);
		bindsOut->rightSpin = getConfigInt(CONFIG_KEYBIND_SECTION, "SpinRight", SDL_SCANCODE_KP_3);
		bindsOut->nollie = getConfigInt(CONFIG_KEYBIND_SECTION, "Nollie", SDL_SCANCODE_KP_7);
		bindsOut->switchRevert = getConfigInt(CONFIG_KEYBIND_SECTION, "Switch", SDL_SCANCODE_KP_9);

		bindsOut->right = getConfigInt(CONFIG_KEYBIND_SECTION, "Right", SDL_SCANCODE_D);
		bindsOut->left = getConfigInt(CONFIG_KEYBIND_SECTION, "Left", SDL_SCANCODE_A);
		bindsOut->up = getConfigInt(CONFIG_KEYBIND_SECTION, "Forward", SDL_SCANCODE_W);
		bindsOut->down = getConfigInt(CONFIG_KEYBIND_SECTION, "Backward", SDL_SCANCODE_S);
		bindsOut->feeble = getConfigInt(CONFIG_KEYBIND_SECTION, "Feeble", SDL_SCANCODE_LSHIFT);

		bindsOut->cameraRight = getConfigInt(CONFIG_KEYBIND_SECTION, "CameraRight", SDL_SCANCODE_KP_MULTIPLY);
		bindsOut->cameraLeft = getConfigInt(CONFIG_KEYBIND_SECTION, "CameraLeft", SDL_SCANCODE_KP_DIVIDE);
		bindsOut->cameraUp = getConfigInt(CONFIG_KEYBIND_SECTION, "CameraUp", SDL_SCANCODE_KP_PLUS);
		bindsOut->cameraDown = getConfigInt(CONFIG_KEYBIND_SECTION, "CameraDown", SDL_SCANCODE_KP_MINUS);

		bindsOut->itemRight = getConfigInt(CONFIG_KEYBIND_SECTION, "ItemRight", SDL_SCANCODE_L);
		bindsOut->itemLeft = getConfigInt(CONFIG_KEYBIND_SECTION, "ItemLeft", SDL_SCANCODE_J);
		bindsOut->itemUp = getConfigInt(CONFIG_KEYBIND_SECTION, "ItemUp", SDL_SCANCODE_I);
		bindsOut->itemDown = getConfigInt(CONFIG_KEYBIND_SECTION, "ItemDown", SDL_SCANCODE_K);
	}
}

void loadControllerBinds(struct controllerbinds *bindsOut) {
	if (bindsOut) {
		bindsOut->menu = getConfigInt(CONFIG_CONTROLLER_SECTION, "Pause", CONTROLLER_BUTTON_START);
		bindsOut->cameraToggle = getConfigInt(CONFIG_CONTROLLER_SECTION, "ViewToggle", CONTROLLER_BUTTON_BACK);
		bindsOut->cameraSwivelLock = getConfigInt(CONFIG_CONTROLLER_SECTION, "SwivelLock", CONTROLLER_BUTTON_RIGHTSTICK);
		bindsOut->focus = getConfigInt(CONFIG_CONTROLLER_SECTION, "Focus", CONTROLLER_BUTTON_LEFTSTICK);
		bindsOut->caveman = getConfigInt(CONFIG_CONTROLLER_SECTION, "Caveman", 0);

		bindsOut->grind = getConfigInt(CONFIG_CONTROLLER_SECTION, "Grind", CONTROLLER_BUTTON_Y);
		bindsOut->grab = getConfigInt(CONFIG_CONTROLLER_SECTION, "Grab", CONTROLLER_BUTTON_B);
		bindsOut->ollie = getConfigInt(CONFIG_CONTROLLER_SECTION, "Ollie", CONTROLLER_BUTTON_A);
		bindsOut->kick = getConfigInt(CONFIG_CONTROLLER_SECTION, "Flip", CONTROLLER_BUTTON_X);

		bindsOut->leftSpin = getConfigInt(CONFIG_CONTROLLER_SECTION, "SpinLeft", CONTROLLER_BUTTON_LEFTSHOULDER);
		bindsOut->rightSpin = getConfigInt(CONFIG_CONTROLLER_SECTION, "SpinRight", CONTROLLER_BUTTON_RIGHTSHOULDER);
		bindsOut->nollie = getConfigInt(CONFIG_CONTROLLER_SECTION, "Nollie", CONTROLLER_BUTTON_LEFTTRIGGER);
		bindsOut->switchRevert = getConfigInt(CONFIG_CONTROLLER_SECTION, "Switch", CONTROLLER_BUTTON_RIGHTTRIGGER);

		bindsOut->right = getConfigInt(CONFIG_CONTROLLER_SECTION, "Right", CONTROLLER_BUTTON_DPAD_RIGHT);
		bindsOut->left = getConfigInt(CONFIG_CONTROLLER_SECTION, "Left", CONTROLLER_BUTTON_DPAD_LEFT);
		bindsOut->up = getConfigInt(CONFIG_CONTROLLER_SECTION, "Forward", CONTROLLER_BUTTON_DPAD_UP);
		bindsOut->down = getConfigInt(CONFIG_CONTROLLER_SECTION, "Backward", CONTROLLER_BUTTON_DPAD_DOWN);

		bindsOut->movement = getConfigInt(CONFIG_CONTROLLER_SECTION, "MovementStick", CONTROLLER_STICK_LEFT);
		bindsOut->camera = getConfigInt(CONFIG_CONTROLLER_SECTION, "CameraStick", CONTROLLER_STICK_RIGHT);
	}
}

void __stdcall initManager() {
	log_printf(LL_INFO, "Initializing Input Manager!\n");

	// init sdl here
	SDL_Init(SDL_INIT_GAMECONTROLLER);

	//SDL_SetHint(SDL_HINT_HIDAPI_IGNORE_DEVICES, "0x1ccf/0x0000");
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "false");

	char *controllerDbPath[filePathBufLen];
	int result = sprintf_s(controllerDbPath, filePathBufLen,"%s%s", executableDirectory, "gamecontrollerdb.txt");

	if (result) {
		result = SDL_GameControllerAddMappingsFromFile(controllerDbPath);
		if (result) {
			log_printf(LL_INFO, "Loaded mappings\n");
		} else {
			log_printf(LL_WARN, "Failed to load %s\n", controllerDbPath);
		}
		
	}

	loadInputSettings(&inputsettings);
	loadKeyBinds(&keybinds);
	loadControllerBinds(&padbinds);

	initSDLControllers();

	if (inputsettings.isPs2Controls) {
		patchPs2Buttons();
	}

	writeNativeBinds();

	registerEventHandler(handleInputEvent);

	//setMenuControls(inputsettings.uicontrols);
}

uint8_t __cdecl isPs2Wrapper_DD(uint8_t *idk, uint8_t *script) {
	uint32_t scriptcrc = 0;

	if (script) {
		scriptcrc = *(uint32_t *)(script + 0xd8);
		//printf("ISPS2 FROM 0x%08x!\n", scriptcrc);

		//printf("ISPS2 FROM 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x!\n", *(uint32_t *)(script + 0xd0), *(uint32_t *)(script + 0xd4), *(uint32_t *)(script + 0xd8), *(uint32_t *)(script + 0xc4), *(uint32_t *)(script + 0xc0));
		// detect grindscripts
		if (scriptcrc == 0x56181c77) {
			return 1;
		}
	}

	return 0;
}

void patchPs2Buttons() {
	patchByte((void *)(addr_platform + 2), 0x05);	// change xbox platform to something else to just skip it
	wrap_cfunc("IsPS2", isPs2Wrapper_DD, NULL);

	// in air
	patchByte((void *)(addr_r2l2_air), 0x75);
	patchByte((void *)(addr_r2l2_air + 1), 0x09);
	
	// break vert
	patchByte((void *)(addr_r2l2_break_vert1), 0x75);
	patchByte((void *)(addr_r2l2_break_vert1 + 1), 0x11);
	patchNop((void *)(addr_r2l2_break_vert1 + 2), 4);

	patchByte((void *)(addr_r2l2_break_vert2), 0x75);
	patchByte((void *)(addr_r2l2_break_vert2 + 1), 0x11);
	patchNop((void *)(addr_r2l2_break_vert2 + 2), 4);

	// lip jump?
	patchByte((void *)(addr_r2l2_lip), 0x75);
	patchByte((void *)(addr_r2l2_lip + 1), 0x09);
	
	// air recovery
	patchByte((void *)(addr_r2l2_air_recover), 0x75);
	patchByte((void *)(addr_r2l2_air_recover + 1), 0x09);

	// ground-to-air
	patchByte((void *)(addr_r2l2_groundair), 0x75);
	patchByte((void *)(addr_r2l2_groundair + 1), 0x09);

	// ground-to-air acid drop
	patchByte((void *)(addr_r2l2_groundair_acid1), 0x75);
	patchByte((void *)(addr_r2l2_groundair_acid1 + 1), 0x09);

	// also ground-to-air acid drop

	// acid drop
	patchByte((void *)(addr_r2l2_acid_drop), 0x75);
	patchByte((void *)(addr_r2l2_acid_drop + 1), 0x09);

	// walk acid drop
	patchNop((void *)(addr_r2l2_walk_acid1), 6);
	patchByte((void *)(addr_r2l2_walk_acid1), 0x75);
	patchByte((void *)(addr_r2l2_walk_acid1 + 1), 0x11);

	patchNop((void *)(addr_r2l2_walk_acid2), 6);
	patchByte((void *)(addr_r2l2_walk_acid2), 0x75);
	patchByte((void *)(addr_r2l2_walk_acid2 + 1), 0x11);

	// disable spin delay on l1/r1
	//patchNop((void *)(addr_spin_delay1), 2);
	//patchNop((void *)(addr_spin_delay2), 2);
}

void patchInput() {
	// patch SIO::Device
	// process
	patchJmp((void *)addr_device_processs, &processController);	// 0062b240
	//patchByte((void *)(addr_device_processs + 7), 0xC3);

	// set_actuator
	// don't call read_data in activate_actuators
	patchNop(addr_call_device_read - 2, 7);
	patchJmp(addr_set_actuators, set_actuators);
	
	// init input patch - nop direct input setup
	patchNop(addr_init_dinput, 47);
	patchCall(addr_init_dinput + 5, &initManager);

	// some config call relating to the dinput devices
	patchByte(addr_deinit_dinput, 0xC3);	// immediately return

	// fix chat handler to be able to detect f1
	patchByte(0x004c82e5 + 1, 0x1f);
}