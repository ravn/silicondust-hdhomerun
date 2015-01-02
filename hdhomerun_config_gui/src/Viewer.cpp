/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#include "AppInclude.h"

#define VIEWER_PORT 5000

void *cStreamThreadExecute(TViewer * Viewer)
{
	Viewer->StreamThreadExecute();
	return NULL;
}

TViewer::TViewer()
{
	ExeName = "vlc";
	FindViewer();
	g_mutex_init(&mutex);

	hdhomerun_debug_printf(dbg, "Viewer: viewer location: %s\n", ExeName.c_str());

#ifdef __WINDOWS__
	Process = (PROCESS_INFORMATION *)calloc(1, sizeof(PROCESS_INFORMATION));
	if (!Process) {
		hdhomerun_debug_printf(dbg, "Viewer: memory allocation failed\n");
		throw "memory allocation failed";
	}
#else
	Process = 0;
#endif
	hd = NULL;
	StreamThread = NULL;

	/* Create socket. */
	StreamSock = (int) socket(AF_INET, SOCK_DGRAM, 0);
	if (StreamSock == -1) {
		hdhomerun_debug_printf(dbg, "Viewer: socket create failed (%d)\n", hdhomerun_sock_getlasterror());
		throw "socket create failed";
	}

	/* Expand socket buffer size. */
	int TxSize = 1024 * 1024;
	if (setsockopt(StreamSock, SOL_SOCKET, SO_SNDBUF, (char *) &TxSize, sizeof(TxSize)) < 0) {
		hdhomerun_debug_printf(dbg, "Viewer: socket set send buffer size failed (%d)\n", hdhomerun_sock_getlasterror());
	}
}

TViewer::~TViewer()
{
	Stop();
}

void TViewer::FindViewer()
{
#ifdef __APPLE__
	CFURLRef appURL;
	OSStatus appStatus = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("org.videolan.vlc"), NULL, NULL, &appURL);
	if (appStatus != 0) {
		hdhomerun_debug_printf(dbg, "Viewer: viewer not installed\n");
		return;
	}
	
	CFBundleRef appBundle = CFBundleCreate(kCFAllocatorDefault, appURL);
	
	appURL = CFBundleCopyExecutableURL(appBundle);
	
	char buf[PATH_MAX+1];
	bool ret = CFURLGetFileSystemRepresentation(appURL, TRUE, (UInt8*)buf, PATH_MAX);
	
	if (! ret) {
		hdhomerun_debug_printf(dbg, "Viewer: viewer not found\n");
		return;
	}
	
	ExeName = buf;
#endif

#ifdef __WINDOWS__
	char buf[1024];
	DWORD size = sizeof(buf);

	LSTATUS err = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VideoLAN\\VLC", "", RRF_RT_REG_SZ, NULL, (PVOID) buf, &size);
	if (err != 0) {
		hdhomerun_debug_printf(dbg, "Viewer: viewer not installed\n");
		return;
	}
	
	ExeName = buf;
#endif

}

bool TViewer::Supported()
{
	return (!ExeName.empty());
}

bool TViewer::Active()
{
	return (StreamThread != 0);
}

bool TViewer::Start(THDHomeRunDevice * Dev)
{
	/* Stop any previous instance of viewer. */
	Stop();

	GString *DebugPrefix = g_string_new(NULL);
	g_string_sprintf(DebugPrefix, "%08lX-%u vlc:", (unsigned long)Dev->DeviceID, Dev->TunerIndex);
	hdhomerun_debug_set_prefix(dbg, DebugPrefix->str);
	g_string_free(DebugPrefix, true);

	hdhomerun_debug_printf(dbg, "Viewer::Start\n");

	/* Check viewer is installed. */
	if (ExeName.empty()) {
		hdhomerun_debug_printf(dbg, "Viewer: viewer not installed\n");
		return false;
	}

	/* HDHomeRun device. */
	hd = hdhomerun_device_create(Dev->DeviceID, 0, Dev->TunerIndex, dbg);
	if (!hd) {
		hdhomerun_debug_printf(dbg, "Viewer: failed to create hdhomerun device object\n");
		return false;
	}

	/* Start streaming. */
	if (hdhomerun_device_stream_start(hd) <= 0) {
		hdhomerun_debug_printf(dbg, "Viewer: stream start failed\n");
		printf("stream start failed\n");
		Stop();
		return false;
	}

	/* Start thread. */
	ThreadExecute = true;

	StreamThread = g_thread_new("Viewer", (GThreadFunc) cStreamThreadExecute, (gpointer) this);

	/* Start viewer. */
#ifdef __WINDOWS__
	char cCommandLine[512];
	snprintf(cCommandLine, sizeof(cCommandLine), "%s --deinterlace=-1 udp://@127.0.0.1:%d", ExeName.c_str(), VIEWER_PORT);

	STARTUPINFOA StartupInfo;
	memset(&StartupInfo, 0, sizeof(STARTUPINFOA));
	StartupInfo.cb = sizeof(STARTUPINFOA);

	if (!CreateProcessA(NULL, (LPSTR) cCommandLine, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, Process)) {
		hdhomerun_debug_printf(dbg, "Viewer: Failed to launch viewer\n");
		Stop();
		return false;
	}
#else
	Process = fork();
	if (Process == 0) {
		execlp(ExeName.c_str(), ExeName.c_str(), "--deinterlace=-1", "udp://@127.0.0.1:5000", (char *)NULL);
		_exit(0);
	}
#endif

	/* Success. */
	return true;
}

void TViewer::Stop()
{
#ifdef __WINDOWS__
	if (Process->hProcess) {
		hdhomerun_debug_printf(dbg, "Viewer::Stop\n");
		TerminateProcess(Process->hProcess, 0);
		CloseHandle(Process->hProcess);
		CloseHandle(Process->hThread);
		memset(Process, 0, sizeof(PROCESS_INFORMATION));
	}
#else
	if (Process) {
		hdhomerun_debug_printf(dbg, "Viewer::Stop\n");
		kill(Process, SIGKILL);
		Process = 0;
	}
#endif

	if (g_mutex_trylock(&mutex) == false) {
		return;
	}

	if (StreamThread) {
		ThreadExecute = false;
		g_thread_join(StreamThread);
		StreamThread = NULL;
	}

	if (hd) {
		hdhomerun_device_stream_stop(hd);
		hdhomerun_device_destroy(hd);
		hd = NULL;
	}

	hdhomerun_debug_set_prefix(dbg, NULL);

	g_mutex_unlock(&mutex);
}


void TViewer::StreamThreadExecute()
{
	struct sockaddr_in SendAddr;
	memset(&SendAddr, 0, sizeof(SendAddr));
	SendAddr.sin_family = AF_INET;
	SendAddr.sin_addr.s_addr = htonl(0x7F000001);
	SendAddr.sin_port = htons(VIEWER_PORT);

	uint32_t MaxSize = VIDEO_DATA_PACKET_SIZE * 32;

	g_usleep(250000);
	if (!hd) {
		return;
	}
	hdhomerun_device_stream_flush(hd);

	uint64_t StartTime = getcurrenttime();
	uint64_t NextDebugTime = StartTime + 1000;

	while (ThreadExecute) {
		/* Debug. */
		uint64_t CurrentTime = getcurrenttime();
		if (CurrentTime >= NextDebugTime) {
			/* Print stats. */
			hdhomerun_device_debug_print_video_stats(hd);

			/* Schedule next debug print. */
			if (CurrentTime < StartTime + 60 * 1000) {
				NextDebugTime = CurrentTime + 10 * 1000;
			} else {
				NextDebugTime = CurrentTime + 120 * 1000;
			}
		}

		/* Receive from HDHR. */
		size_t ActualSize;
		uint8_t *VideoBuffer = hdhomerun_device_stream_recv(hd, MaxSize, &ActualSize);
		if (!VideoBuffer) {
			g_usleep(64000);
			continue;
		}

		/* Send to viewer. */
		uint8_t *End = VideoBuffer + ActualSize;
		while (VideoBuffer + VIDEO_DATA_PACKET_SIZE <= End) {
			if (sendto(StreamSock, (char *) VideoBuffer, VIDEO_DATA_PACKET_SIZE, 0, (struct sockaddr *) &SendAddr, sizeof(SendAddr)) != VIDEO_DATA_PACKET_SIZE) {
				hdhomerun_debug_printf(dbg, "Viewer: stream send failed (%d)\n", hdhomerun_sock_getlasterror());
				hdhomerun_device_stream_flush(hd);
				break;
			}
			VideoBuffer += VIDEO_DATA_PACKET_SIZE;
		}
	}
}
