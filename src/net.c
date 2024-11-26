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
	patchDWord(0x0054dd13 + 1, gamestats);
	patchDWord(0x006025d8 + 1, gamestats);
	patchDWord(0x00602618 + 1, gamestats);
	patchDWord(0x00602650 + 2, gamestats);

	// %s.available.gamespy.com
	char *available = allocOnlineServiceString("%%s.available.%s", url);
	patchDWord(0x005f86dd + 1, available);

	// %s.master.gamespy.com
	char *master = allocOnlineServiceString("%%s.master.%s", url);
	patchDWord(0x005fb03b + 1, master);

	// natneg2.gamespy.com
	char *natneg2 = allocOnlineServiceString("natneg2.%s", url);
	patchDWord(0x0068e380, natneg2);

	// natneg1.gamespy.com
	char *natneg1 = allocOnlineServiceString("natneg1.%s", url);
	patchDWord(0x0068e37c, natneg1);

	// http://motd.gamespy.com/motd/motd.asp?userid=%d&productid=%d&versionuniqueid=%s&distid=%d&uniqueid=%s&gamename=%s
	char *motd = allocOnlineServiceString("http://motd.%s/motd/motd.asp?userid=%%d&productid=%%d&versionuniqueid=%%s&distid=%%d&uniqueid=%%s&gamename=%%s", url);
	patchDWord(0x005ffd05 + 1, motd);

	// peerchat.gamespy.com
	char *peerchat = allocOnlineServiceString("peerchat.%s", url);
	patchDWord(0x00605a4c + 1, peerchat);
	patchDWord(0x00605aac + 1, peerchat);
	patchDWord(0x00605b0e + 1, peerchat);

	// %s.ms%d.gamespy.com
	char *ms = allocOnlineServiceString("%%s.ms%%d.%s", url);
	patchDWord(0x00612085 + 1, ms);

	// gpcm.gamespy.com
	char *gpcm = allocOnlineServiceString("gpcm.%s", url);
	patchDWord(0x00617339 + 1, gpcm);

	// gpsp.gamespy.com
	char *gpsp = allocOnlineServiceString("gpsp.%s", url);
	patchDWord(0x006182bd + 1, gpsp);
}