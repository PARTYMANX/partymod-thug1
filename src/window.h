#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <stdint.h>

void loadInputSettings(struct inputsettings *settingsOut);
void loadControllerBinds(struct controllerbinds *bindsOut);
void loadKeyBinds(struct keybinds *bindsOut);
int getIniBool(char *section, char *key, int def, char *file);

void patchWindow();
void getWindowDimensions(uint32_t *width, uint32_t *height);

#endif