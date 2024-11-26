#include <stdint.h>

void *addr_cfunclist = 0x00686ac0;
void *addr_cfunccount = 0x0052c880;

/*uint8_t __cdecl isPs2Wrapper(uint8_t *idk, uint8_t *script) {
	uint32_t scriptcrc = 0;

	if (script) {
		scriptcrc = *(uint32_t *)(script + 0xd0);
		printf("ISPS2 FROM 0x%08x!\n", scriptcrc);
	}

	return 0;
}

uint8_t __cdecl isXboxWrapper(uint8_t *idk, uint8_t *script) {
	uint32_t scriptcrc = 0;

	if (script) {
		scriptcrc = *(uint32_t *)(script + 0xc8);
		printf("ISWIN32 FROM 0x%08x!\n", scriptcrc);
	}

	return 1;
}

uint8_t __cdecl getPlatformWrapper(uint8_t *params, uint8_t *script) {
	uint32_t scriptcrc = 0;

	if (script) {
		scriptcrc = *(uint32_t *)(script + 0xc8);
		printf("GETPLATFORM FROM 0x%08x!\n", scriptcrc);
	}

	return addr_getPlatform(params, script);
}*/

struct cfunclistentry {
	char *name;
	void *func;
};

uint8_t wrap_cfunc(char *name, void *wrapper, void **addr_out) {
	int (*getCfuncCount)() = addr_cfunccount;

	int count = getCfuncCount();
	struct cfunclistentry *cfunclist = addr_cfunclist;

	for (int i = 0; i < count; i++) {
		if (name && strcmp(cfunclist[i].name, name) == 0) {
			if (addr_out) {
				*addr_out = cfunclist[i].func;
			}

			if (wrapper) {
				patchDWord(&(cfunclist[i].func), wrapper);
			}

			return 1;
		}
	}

	//printf("Couldn't find cfunc %s!\n", name);

	return 0;
}
