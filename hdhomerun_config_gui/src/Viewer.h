/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#ifndef VIEWER_H
#define VIEWER_H

class TViewer {
public:
	TViewer();
	~TViewer();

	void FindViewer();
	bool Supported();
	bool Active();
	bool Start(THDHomeRunDevice * Dev);
	void Stop();
	void StreamThreadExecute();

protected:
	int StreamSock;
	GThread *StreamThread;
	string ExeName;

#ifdef __WINDOWS__
	PROCESS_INFORMATION *Process;
#else
	pid_t Process;
#endif

	struct hdhomerun_device_t *hd;

	bool ThreadExecute;
	GMutex mutex;
};

#endif
