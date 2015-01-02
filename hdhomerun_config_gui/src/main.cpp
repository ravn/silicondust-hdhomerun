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

#ifdef __APPLE__
	GtkosxApplication *osx = (GtkosxApplication *)g_object_new (GTKOSX_TYPE_APPLICATION, NULL);
	GtkWidget *menubar = gtk_menu_bar_new ();
	gtkosx_application_set_menu_bar (osx, GTK_MENU_SHELL (menubar));
	gtkosx_application_set_use_quartz_accelerators (osx, TRUE);
	gtkosx_application_ready(osx);
#endif

	HDHRConfig HDHomeRunConfig;

	gtk_main();

	return 0;
}
