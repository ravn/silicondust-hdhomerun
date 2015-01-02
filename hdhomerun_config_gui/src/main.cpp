/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#include "AppInclude.h"

int main(int argc, char *argv[])
{
#ifdef __WINDOWS__
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
#endif

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	gtk_set_locale();
	gtk_init(&argc, &argv);

	g_thread_init(NULL);
	gdk_threads_init();

	gdk_threads_enter();
	HDHRConfig HDHomeRunConfig;
	gdk_threads_leave();

	gtk_main();

	return 0;
}
