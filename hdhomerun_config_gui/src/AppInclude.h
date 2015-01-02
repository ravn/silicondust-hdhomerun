/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#ifndef APPINCLUDE_H
#define APPINCLUDE_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef __APPLE__
#  include <ApplicationServices/ApplicationServices.h>
#  include <CoreServices/CoreServices.h>
#  include <CoreFoundation/CoreFoundation.h>
#  include <gtkmacintegration/gtkosxapplication.h>
#endif

#include "hdhomerun.h"

#include <gtk/gtk.h>
#ifdef __WINDOWS__
#  include <gdk/gdkwin32.h>
#endif

#include "interface.h"
#include "support.h"
#include "callbacks.h"

#include <signal.h>
#include <string>
using namespace std;

#include "Debug.h"
#include "Devices.h"
#include "Viewer.h"

#include "HDHRConfig.h"

#define gtk_combo_box_clear(combo) gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(combo)))

#endif
