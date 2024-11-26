#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <incbin/incbin.h>
#include <Windows.h>

#include <global.h>
#include <patch.h>
#include <input.h>
#include <log.h>

#include <util/hash.h>

INCBIN(keyboard, "patches/keyboard.bps");

char scriptCacheFile[1024];

uint8_t usingPS2Controls = 0;
uint8_t menuControls = 0;

map_t *cachedScriptMap;	// persistent cache, written to disk when modified
map_t *patchMap;
map_t *scriptMap;	// runtime cache, potentially modified scripts go in here.  think of it as a cache of buffers rather than scripts themselves

#define SCRIPT_CACHE_FILE_NAME "scriptCache.qbc"
#define CACHE_MAGIC_NUMBER 0x49676681 // QBC1

typedef struct {
	size_t filenameLen;
	char *filename;
	uint32_t checksum;	// this is the checksum of the patch this script was made against
	size_t scriptLen;
	uint8_t *script;
} scriptCacheEntry;

void saveScriptCache(char *filename) {
	FILE *file = fopen(filename, "wb");

	if (file) {
		uint32_t magicNum = CACHE_MAGIC_NUMBER;
		fwrite(&magicNum, sizeof(uint32_t), 1, file);
		fwrite(&cachedScriptMap->entries, sizeof(size_t), 1, file);
		log_printf(LL_INFO, "Saving %d entries\n", cachedScriptMap->entries);

		for (int i = 0; i < cachedScriptMap->len; i++) {
			struct bucketNode *node = cachedScriptMap->buckets[i].head;
			while (node) {
				scriptCacheEntry *entry = node->val;

				fwrite(&entry->filenameLen, sizeof(size_t), 1, file);
				fwrite(entry->filename, entry->filenameLen, 1, file);
				fwrite(&entry->checksum, sizeof(uint32_t), 1, file);
				fwrite(&entry->scriptLen, sizeof(size_t), 1, file);
				fwrite(entry->script, entry->scriptLen, 1, file);

				node = node->next;
			}
		}

		fclose(file);
	} else {
		log_printf(LL_ERROR, "FAILED TO OPEN FILE!!\n");
	}
}

int loadScriptCache(char *filename) {
	FILE *file = fopen(filename, "rb");

	if (file) {
		uint32_t magicNum = 0;
		fread(&magicNum, sizeof(uint32_t), 1, file);

		if (magicNum != CACHE_MAGIC_NUMBER) {
			goto failure;
		}

		size_t entryCount = 0;
		if (!fread(&entryCount, sizeof(size_t), 1, file)) {
			goto failure;
		}
		log_printf(LL_INFO, "%d entries\n", entryCount);

		for (int i = 0; i < entryCount; i++) {
			scriptCacheEntry entry;

			if (!fread(&entry.filenameLen, sizeof(size_t), 1, file)) {
				goto failure;
			}

			entry.filename = calloc(entry.filenameLen + 1, 1);
			if (!entry.filename || !fread(entry.filename, entry.filenameLen, 1, file)) {
				goto failure;	
			}
			entry.filename[entry.filenameLen] = '\0';

			if (!fread(&entry.checksum, sizeof(uint32_t), 1, file)) {
				goto failure;
			}

			if (!fread(&entry.scriptLen, sizeof(size_t), 1, file)) {
				goto failure;
			}

			entry.script = malloc(entry.scriptLen);
			if (!entry.script || !fread(entry.script, entry.scriptLen, 1, file)) {
				goto failure;	
			}

			map_put(cachedScriptMap, entry.filename, entry.filenameLen, &entry, sizeof(entry));

			log_printf(LL_INFO, "Loaded cache entry for %s, patch checksum %08x\n", entry.filename, entry.checksum);
		}

		fclose(file);
		return 0;

	failure:
		log_printf(LL_ERROR, "Failed to read script cache file!!  Cache may be corrupt and will be rewritten...\n");
		fclose(file);
		return -1;
	} else {
		log_printf(LL_WARN, "Script cache does not exist\n");
		return -1;
	}
}

int applyPatch(uint8_t *patch, size_t patchLen, uint8_t *input, size_t inputLen, uint8_t **output, size_t *outputLen);
uint32_t getPatchChecksum(uint8_t *patch, size_t sz);

void registerPatch(char *name, unsigned int sz, char *data) {
	map_put(patchMap, name, strlen(name), data, sz);
	log_printf(LL_INFO, "Registered patch for %s\n", name);
}

void initScriptPatches() {
	//sprintf(scriptCacheFile, "%s%s", executableDirectory, SCRIPT_CACHE_FILE_NAME);

	patchMap = map_alloc(16, NULL, NULL);
	cachedScriptMap = map_alloc(16, NULL, NULL);
	scriptMap = map_alloc(16, NULL, NULL);

	registerPatch("scripts\\engine\\menu\\keyboard.qb", gkeyboardSize, gkeyboardData);

	//printf("Loading script cache from %s...\n", scriptCacheFile);
	//loadScriptCache(scriptCacheFile);
	//printf("Done!\n");
}

uint32_t (__cdecl *their_crc32)(char *) = (void *)0x00401b90;
uint32_t (__cdecl *addr_parseqbsecondfunc)(uint32_t, char *) = (void *)0x0040a8f0;
void (__cdecl *addr_parseqbthirdfunc)(uint8_t *, uint8_t *, void *, uint32_t, int) = (void *)0x0046ca50;
void *addr_parseqb = 0x0046fba0;

struct component {
	uint8_t unk;
	uint8_t type;
	uint16_t size;
	uint32_t name;
	void *data;
	struct component *next;
};

struct scrStruct {
	void *unk;
	struct component *head;
};

struct scrobjArray {
	void *unk;
	uint32_t size;
	void *data;
};

struct scrobjArray *scr_get_array(struct scrStruct *str, uint32_t checksum) {
	struct scrobjArray *result = NULL;

	struct component *node = str->head;
	while (node && node->name != checksum) {
		node = node->next;
	}
	if (node) {
		// get array
		result = node->data;
	}

	return result;
}

void __cdecl pipLoadWrapper(char *filename, int unk1, int unk2, int unk3, int unk4) {
	uint8_t *(__cdecl *orig_pipLoad)(char *) = 0x0057e840;
	uint32_t (__cdecl *getFileSize)(uint32_t) = 0x0057def0;
	uint32_t (__cdecl *their_crc32)(char *) = (void *)0x00401920;

	//printf("LOADING SCRIPT %s\n", filename);

	uint8_t *filedata = orig_pipLoad(filename);
	uint32_t filesize = getFileSize(their_crc32(filename));

	/*if (their_crc32("scripts\\game\\skater\\grindscripts.qb") == their_crc32(filename)) {
		printf("WRITING GRINDSCRIPTS TO FILE\n");
		char *fileoutbuffer[1024];
		sprintf(fileoutbuffer, "scriptsout\\%s", filename);
		printf("????\n");
		FILE *file = fopen(fileoutbuffer, "wb");
		printf("TEST\n");
		if (file) {
			fwrite(filedata, 1, filesize, file);
			printf("TEST\n");
			fclose(file);
		}
	}*/

	//printf("FILE SIZE IS %d\n", filesize);

	uint8_t *scriptOut;


	// TODO: cache script to avoid continuous leak
	size_t filenameLen = strlen(filename);
	uint8_t *patch = map_get(patchMap, filename, filenameLen);
	if (patch) {
		//uint32_t filesize = ((uint32_t *)script)[1];
		scriptCacheEntry *cachedScript = map_get(cachedScriptMap, filename, filenameLen);

		size_t patchLen = map_getsz(patchMap, filename, filenameLen);
		uint32_t patchcrc = getPatchChecksum(patch, patchLen);

		if (cachedScript && cachedScript->checksum == patchcrc) {
			log_printf(LL_INFO, "Using cached patched script for %s\n", filename);

			uint8_t *s = map_get(scriptMap, filename, filenameLen);
			if (s) {
				memcpy(s, cachedScript->script, cachedScript->scriptLen);	// rewrite script to memory
			} else {
				s = malloc(cachedScript->scriptLen);
				memcpy(s, cachedScript->script, cachedScript->scriptLen);

				map_put(scriptMap, filename, filenameLen, s, cachedScript->scriptLen);
			}

			scriptOut = s;
		} else {
			if (cachedScript) {
				log_printf(LL_INFO, "Cached script checksum %08x didn't match patch %08x\n", cachedScript->checksum, patchcrc);
			}

			log_printf(LL_INFO, "Applying patch for %s\n", filename);
			uint8_t *patchedBuffer = NULL;
			size_t patchedBufferLen = 0;

			int result = applyPatch(patch, patchLen, filedata, filesize, &patchedBuffer, &patchedBufferLen);
				
			if (!result) {
				log_printf(LL_INFO, "Patch succeeded! Writing to cache...\n");

				scriptCacheEntry entry;
				entry.filenameLen = filenameLen;
				entry.filename = malloc(filenameLen + 1);
				memcpy(entry.filename, filename, filenameLen);
				entry.filename[filenameLen] = '\0';
				entry.checksum = getPatchChecksum(patch, patchLen);
				entry.scriptLen = patchedBufferLen;
				entry.script = malloc(patchedBufferLen);
				memcpy(entry.script, patchedBuffer, patchedBufferLen);

				map_put(cachedScriptMap, filename, filenameLen, &entry, sizeof(scriptCacheEntry));

				//saveScriptCache(scriptCacheFile);

				log_printf(LL_INFO, "Done!\n");

				map_put(scriptMap, filename, filenameLen, patchedBuffer, patchedBufferLen);

				scriptOut = patchedBuffer;
			} else {
				scriptOut = filedata;

				/*printf("WRITING TO FILE\n");
				char *fileoutbuffer[1024];
				sprintf(fileoutbuffer, "scriptsout\\%s", filename);
				printf("????\n");
				FILE *file = fopen(fileoutbuffer, "wb");
				printf("TEST\n");
				if (file) {
					fwrite(filedata, 1, filesize, file);
					printf("TEST\n");
					fclose(file);
				}*/

				log_printf(LL_WARN, "Patch failed! Continuing with original script\n");
			}
		}
	} else {
		scriptOut = filedata;
	}

	return scriptOut;
}

void patchScriptHook() {
	patchCall(0x0040a049, pipLoadWrapper);
}

