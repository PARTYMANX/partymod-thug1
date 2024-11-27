#include <net.h>

#include <config.h>
#include <patch.h>
#include <log.h>

char *allocOnlineServiceString(char *fmt, char *url) {
	size_t outsize = strlen(fmt) + strlen(url) + 1;	// most likely oversized but it doesn't matter
	char *result = malloc(outsize);

	sprintf(result, fmt, url);

	log_printf(LL_TRACE, "ALLOC'D STRING %s\n", result);

	return result;
}

void patchOnlineService() {
	// NOTE: these will leak and that's a-okay
	char *url[256];
	getConfigString(CONFIG_MISC_SECTION, "OnlineDomain", "openspy.net", url, 256);

	// gamestats.gamespy.com
	char *gamestats = allocOnlineServiceString("gamestats.%s", url);
	patchDWord(0x004dcbdd + 1, gamestats);
	patchDWord(0x005d2128 + 1, gamestats);
	patchDWord(0x005d2168 + 1, gamestats);
	patchDWord(0x005d21a0 + 2, gamestats);

	// %s.available.gamespy.com
	char *available = allocOnlineServiceString("%%s.available.%s", url);
	patchDWord(0x005c82fd + 1, available);

	// %s.master.gamespy.com
	char *master = allocOnlineServiceString("%%s.master.%s", url);
	patchDWord(0x005cac5b + 1, master);

	// natneg2.gamespy.com
	char *natneg2 = allocOnlineServiceString("natneg2.%s", url);
	patchDWord(0x005cbc6b, natneg2);

	// natneg1.gamespy.com
	char *natneg1 = allocOnlineServiceString("natneg1.%s", url);
	patchDWord(0x005cbc3a, natneg1);

	// http://motd.gamespy.com/motd/motd.asp?userid=%d&productid=%d&versionuniqueid=%s&distid=%d&uniqueid=%s&gamename=%s
	//char *motd = allocOnlineServiceString("http://motd.%s/motd/motd.asp?userid=%%d&productid=%%d&versionuniqueid=%%s&distid=%%d&uniqueid=%%s&gamename=%%s", url);
	//patchDWord(0x005ffd05 + 1, motd);

	// peerchat.gamespy.com
	char *peerchat = allocOnlineServiceString("peerchat.%s", url);
	patchDWord(0x005d560c + 1, peerchat);
	patchDWord(0x005d566c + 1, peerchat);
	patchDWord(0x005d56ce + 1, peerchat);

	// %s.ms%d.gamespy.com
	char *ms = allocOnlineServiceString("%%s.ms%%d.%s", url);
	patchDWord(0x005e1c35 + 1, ms);

	// gpcm.gamespy.com
	char *gpcm = allocOnlineServiceString("gpcm.%s", url);
	patchDWord(0x005e4569 + 1, gpcm);

	// gpsp.gamespy.com
	char *gpsp = allocOnlineServiceString("gpsp.%s", url);
	patchDWord(0x005e54ed + 1, gpsp);
}