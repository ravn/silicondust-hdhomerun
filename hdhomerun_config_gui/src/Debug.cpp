/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#include "AppInclude.h"

hdhomerun_debug_t *dbg = NULL;
unsigned long long DebugExpireTime = 0;
string DebugFilename = "";

void DebugInit(void)
{
	dbg = hdhomerun_debug_create();
	if (!dbg) {
		return;
	}

	DebugCheckState();
}

void DebugCheckState(void)
{

#ifdef __WINDOWS__
	char buf[1024];
	DWORD size = sizeof(buf);

	LSTATUS err = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Silicondust\\HDHomeRun", "DebugFilename", RRF_RT_REG_SZ, NULL, (PVOID) buf, &size);
	if (err != 0) {
		buf[0] = '\0';
	}

	DebugFilename = buf;

	size = sizeof(DebugExpireTime);
	err = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Silicondust\\HDHomeRun", "DebugEnabled", RRF_RT_REG_QWORD, NULL, (PVOID) & DebugExpireTime, &size);
	if (err != 0) {
		DebugExpireTime = 0;
	}
#endif

	if (DebugFilename.empty()) {
		hdhomerun_debug_set_filename(dbg, NULL);
	} else {
		hdhomerun_debug_set_filename(dbg, DebugFilename.c_str());
	}

	if (DebugExpireTime > (unsigned long long)time(NULL)) {
		hdhomerun_debug_enable(dbg);
	} else {
		hdhomerun_debug_disable(dbg);
	}
}
