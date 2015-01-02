/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#include "AppInclude.h"
#include <gdk/gdkkeysyms.h>

/* This file does nothing more than hand the call to the following C++ class */
extern HDHRConfig *HDHRConfigH;

void on_DeviceListTree_selection_changed(GtkTreeSelection *selection, gpointer user_data)
{
	HDHRConfigH->DeviceList_Changed();
}

void on_RescanBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->Rescan();
}

void on_Tab_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data)
{
	HDHRConfigH->Tab_Changed();
}

void on_ChannelMapEdit_changed(GtkComboBox *combobox, gpointer user_data)
{
	HDHRConfigH->ChannelMapEdit_Changed();
}

void on_ChannelNumberSpin_value_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
	HDHRConfigH->ChannelNumberSpin_Changed();
}

void on_ChannelNumberSpin_activate(GtkEntry *entry, gpointer user_data)
{
	HDHRConfigH->ChannelNumberSpin_Changed();
}

void on_ProgramList_changed(GtkComboBox *combobox, gpointer user_data)
{
	HDHRConfigH->ProgramList_Changed();
}

void on_ScanDownBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->ScanDownBtn_Click();
}

void on_ScanUpBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->ScanUpBtn_Click();
}

void on_LaunchVlcBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->LaunchVlcBtn_Click();
}

void on_StopVlcBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->StopVlcBtn_Click();
}

gboolean on_Timer_tick(gpointer user_data)
{
	gdk_threads_enter();
	HDHRConfigH->Timer_Tick();
	gdk_threads_leave();
	return true;
}

void on_UpgradeFilename_changed(GtkEditable *editable, gpointer user_data)
{
	HDHRConfigH->UpgradeFilename_Changed();
}

void on_UpgradeFilenameBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->UpgradeFilenameBtn_Click();
}

void on_UpgradeBtn_clicked(GtkButton *button, gpointer user_data)
{
	HDHRConfigH->UpgradeBtn_Click();
}

void SigChild(int pid)
{
	HDHRConfigH->SigChild(pid);
}


gboolean on_hdhomerun_config_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (event->state & GDK_CONTROL_MASK) {
		switch (event->keyval) {
		case 'D':
		case 'd':
			HDHRConfigH->Debug();
			return TRUE;
		default:
			break;
		}
	}
	return FALSE;
}
