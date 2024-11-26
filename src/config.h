#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>

#define CONFIG_FILE_NAME "partymod.ini"

#define CONFIG_GRAPHICS_SECTION "Graphics"
#define CONFIG_MISC_SECTION "Miscellaneous"
#define CONFIG_KEYBIND_SECTION "Keybinds"
#define CONFIG_CONTROLLER_SECTION "Gamepad"

void initConfig();
int getConfigBool(char *section, char *key, int def);
int getConfigInt(char *section, char *key, int def);
int getConfigString(char *section, char *key, char *def, char *dst, size_t sz);

#endif