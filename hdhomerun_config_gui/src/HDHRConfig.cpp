/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#include "AppInclude.h"

extern unsigned long long DebugExpireTime;
HDHRConfig *HDHRConfigH = NULL;

static inline void lock_widget(GtkWidget *widget)
{
	gtk_widget_set_size_request(widget, widget->allocation.width, widget->allocation.height);
}

void ColorizeProgress(GtkProgressBar *ProgressBar, uint32_t ARGB_Color)
{
	GtkWidget *Widget = GTK_WIDGET(ProgressBar);

	GdkColor Color;
	Color.red = 0;
	Color.green = 0;
	Color.blue = 0;

	gtk_widget_modify_fg(Widget, GTK_STATE_PRELIGHT, &Color);
	gtk_widget_modify_fg(Widget, GTK_STATE_SELECTED, &Color);

	Color.red = ((ARGB_Color >> 16) & 0xFF) *0x101;
	Color.green = ((ARGB_Color >> 8) & 0xFF) *0x101;
	Color.blue = ((ARGB_Color >> 0) & 0xFF) *0x101;

	gtk_widget_modify_bg(Widget, GTK_STATE_PRELIGHT, &Color);
	gtk_widget_modify_bg(Widget, GTK_STATE_SELECTED, &Color);

	gtk_widget_modify_base(Widget, GTK_STATE_PRELIGHT, &Color);
	gtk_widget_modify_base(Widget, GTK_STATE_SELECTED, &Color);
}

HDHRConfig::HDHRConfig()
{
	DebugInit();
	//hdhomerun_debug_printf(dbg, "HDHomeRun Config (GUI) "  HDHOMERUN_RELEASE_VERSION  "\n");

	hdhomerun_config = GTK_WINDOW(create_hdhomerun_config());
	HDHRConfigH = this;

	/* Device */
	scrolledwindow1 = GTK_SCROLLED_WINDOW(lookup_widget("scrolledwindow1"));
	DeviceListTree = GTK_TREE_VIEW(lookup_widget("DeviceListTree"));
	DeviceListStore = gtk_list_store_new(1, G_TYPE_STRING);

	GtkCellRenderer *DeviceListRenderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *DeviceListColumn = gtk_tree_view_column_new_with_attributes("Device ID", DeviceListRenderer, "text", 0, NULL);
	gtk_tree_view_set_model(GTK_TREE_VIEW(DeviceListTree), GTK_TREE_MODEL(DeviceListStore));
	gtk_tree_view_append_column(GTK_TREE_VIEW(DeviceListTree), DeviceListColumn);

	DeviceList = new THDHomeRunDeviceList;

	Tab = GTK_NOTEBOOK(lookup_widget("Tab"));

	/* Tuner Configuration */
	ChannelMapEdit = GTK_COMBO_BOX(lookup_widget("ChannelMapEdit"));
	ChannelNumberSpin = GTK_SPIN_BUTTON(lookup_widget("ChannelNumberSpin"));
	ProgramList = GTK_COMBO_BOX(lookup_widget("ProgramList"));
	ProgramListStore = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	gtk_combo_box_set_model(ProgramList, GTK_TREE_MODEL(ProgramListStore));

	gtk_cell_layout_clear(GTK_CELL_LAYOUT(ProgramList));

	GtkCellRenderer *ProgramListRenderer;

	ProgramListRenderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(ProgramList), ProgramListRenderer, false);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(ProgramList), ProgramListRenderer, "text", 0, NULL);

	ProgramListRenderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(ProgramList), ProgramListRenderer, true);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(ProgramList), ProgramListRenderer, "text", 1, NULL);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(ProgramListStore), 0, GTK_SORT_ASCENDING);

	ScanDownBtn = GTK_TOGGLE_BUTTON(lookup_widget("ScanDownBtn"));
	ScanUpBtn = GTK_TOGGLE_BUTTON(lookup_widget("ScanUpBtn"));
	LaunchVlcBtn = GTK_BUTTON(lookup_widget("LaunchVlcBtn"));
	StopVlcBtn = GTK_BUTTON(lookup_widget("StopVlcBtn"));

#ifndef __WINDOWS__
	signal(SIGCHLD, &::SigChild);
#endif

	ScanStep = 0;
	LastChannelMap = "";
	LastChannel = -1;
	LastProgramNumber = -1;
	SuppressSetChannel = false;
	ChannelList = NULL;
	SelectedDev = NULL;

	Viewer = new TViewer;
	RespawnVLC = false;

	/*Tuner Status */
	PhysicalChannelStatus = GTK_ENTRY(lookup_widget("PhysicalChannelStatus"));
	SignalStrengthStatus = GTK_PROGRESS_BAR(lookup_widget("SignalStrengthStatus"));
	SignalQualityStatus = GTK_PROGRESS_BAR(lookup_widget("SignalQualityStatus"));
	SymbolQualityStatus = GTK_PROGRESS_BAR(lookup_widget("SymbolQualityStatus"));
	NetworkRateStatus = GTK_ENTRY(lookup_widget("NetworkRateStatus"));

	g_timeout_add(500, (GSourceFunc) & on_Timer_tick, NULL);

	/* Upgrade Tab */

	UpgradeFilename = GTK_ENTRY(lookup_widget("UpgradeFilename"));
	UpgradeFilenameBtn = GTK_BUTTON(lookup_widget("UpgradeFilenameBtn"));
	UpgradeBtn = GTK_BUTTON(lookup_widget("UpgradeBtn"));
	FirmwareVersion = GTK_LABEL(lookup_widget("FirmwareVersion"));

	/*
	 * Style Fixup
	 */

	/* placeholders to calculate window size */
	GtkTreeIter iter;
	gtk_list_store_append(DeviceListStore, &iter);
	gtk_list_store_set(DeviceListStore, &iter, 0, "XXXXXXXX-XXXX", -1);
	gtk_combo_box_append_text(ChannelMapEdit, "XX-XXXXX");
	gtk_combo_box_set_active(ChannelMapEdit, 0);
	UpdateProgramListEntry(999, "XXXXXXXXXXXX");
	gtk_combo_box_set_active(ProgramList, 0);
	gtk_scrolled_window_set_policy(scrolledwindow1, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	/* The window must me mapped or the sizes will be incorrect */
	gtk_widget_show(GTK_WIDGET(hdhomerun_config));

	lock_widget(GTK_WIDGET(ChannelMapEdit));
	lock_widget(GTK_WIDGET(ProgramList));
	lock_widget(GTK_WIDGET(DeviceListTree));

	gtk_list_store_clear(DeviceListStore);

	/*
	 * Fix colors
	 *
	 * GtkStatusbar relies on a queue system
	 * So .. we're using a GtkEntry instead; it gives a much
	 * better looking widget than a GtkLabel inside a frame.
	 */
	GdkColor *color = gtk_widget_get_style(GTK_WIDGET(PhysicalChannelStatus))->fg;
	gtk_widget_modify_text(GTK_WIDGET(PhysicalChannelStatus), GTK_STATE_INSENSITIVE, color);
	gtk_widget_modify_text(GTK_WIDGET(NetworkRateStatus), GTK_STATE_INSENSITIVE, color);

	ColorizeProgress(SignalStrengthStatus, 0xFFFFFFFF);
	ColorizeProgress(SignalQualityStatus, 0xFFFFFFFF);
	ColorizeProgress(SymbolQualityStatus, 0xFFFFFFFF);

	/* Setup event handler */
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(DeviceListTree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(on_DeviceListTree_selection_changed), NULL);

	Rescan();
}

HDHRConfig::~HDHRConfig(void)
{
	if (ChannelList) {
		hdhomerun_channel_list_destroy(ChannelList);
	}

	delete Viewer;
	delete DeviceList;
}

GtkWidget *HDHRConfig::lookup_widget(const gchar *name)
{
	return::lookup_widget(GTK_WIDGET(hdhomerun_config), name);
}

void HDHRConfig::Error(const gchar *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(hdhomerun_config), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, fmt, args);
	va_end(args);
	gtk_window_set_accept_focus(GTK_WINDOW(hdhomerun_config), false);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_window_set_accept_focus(GTK_WINDOW(hdhomerun_config), true);
	gtk_widget_destroy(dialog);
}

void HDHRConfig::Rescan(void)
{
	gtk_list_store_clear(DeviceListStore);
	DeviceList_Changed();

	DebugCheckState();

	DeviceList->Detect();

	THDHomeRunDevice *Dev = DeviceList->IterateFirst();
	while (Dev) {
		if (!Dev->Discovered) {
			Dev = DeviceList->IterateNext(Dev);
			continue;
		}

		GtkTreeIter iter;
		gtk_list_store_append(DeviceListStore, &iter);
		gtk_list_store_set(DeviceListStore, &iter, 0, Dev->Name.c_str(), -1);

		Dev = DeviceList->IterateNext(Dev);
	}
	DeviceList->IterateComplete();

	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(DeviceListStore), &iter)) {
		GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(DeviceListTree));
		gtk_tree_selection_select_iter(select, &iter);
	}

	gtk_widget_grab_focus(GTK_WIDGET(DeviceListTree));
}

void HDHRConfig::CommunicationError(void)
{
	Error(_("ERROR: communication error"));
	Rescan();
}

void HDHRConfig::DeviceList_Changed(void)
{
	/* Clear previous selection. */
	RespawnVLC = false;
	StopVLC();
	SelectedDev = NULL;
	gtk_combo_box_clear(ChannelMapEdit);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(DeviceListTree));
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
		/* No device selected */
		Tab_Changed();
		return;
	}

	gchar *cDeviceName;
	gtk_tree_model_get(model, &iter, 0, &cDeviceName, -1);
	string DeviceName = cDeviceName;
	g_free(cDeviceName);

	/* Find new device */
	SelectedDev = DeviceList->Find(DeviceName);
	if (!SelectedDev) {
		Error(_("ERROR: unable to find device"));
		Rescan();
		return;
	}

	/* Channelmaps. */
	char *cChannelmaps;
	if (hdhomerun_device_get_supported(SelectedDev->hd, (char *)"channelmap: ", &cChannelmaps) <= 0) {
		Tab_Changed();
		return;
	}

	char *ChannelMap = cChannelmaps;
	while (*ChannelMap) {
		char *Ptr = strchr(ChannelMap, ' ');
		if (!Ptr) {
			gtk_combo_box_append_text(ChannelMapEdit, ChannelMap);
			break;
		}

		*Ptr++ = 0;
		gtk_combo_box_append_text(ChannelMapEdit, ChannelMap);
		ChannelMap = Ptr;
	}

	/* Complete. */
	Tab_Changed();
}

void HDHRConfig::Tab_Changed()
{
	/*
	 * Stop any operations in progress.
	 */
	ScanStep = 0;
	RespawnVLC = false;
	StopVLC();

	/*
	 * Clear GUI components.
	 */
	LastChannelMap = "";
	LastChannel = -1;
	LastProgramNumber = -1;
	ClearProgramList();

	gtk_entry_set_text(PhysicalChannelStatus, "");
	gtk_progress_bar_set_fraction(SignalStrengthStatus, 0.0);
	gtk_progress_bar_set_text(SignalStrengthStatus, "");
	gtk_progress_bar_set_fraction(SignalQualityStatus, 0.0);
	gtk_progress_bar_set_text(SignalQualityStatus, "");
	gtk_progress_bar_set_fraction(SymbolQualityStatus, 0.0);
	gtk_progress_bar_set_text(SymbolQualityStatus, "");
	gtk_entry_set_text(NetworkRateStatus, "");

	gtk_label_set_text(FirmwareVersion, _("Firmware Version: Unknown"));

	/*
	 * New page.
	 */
	UpdateStatus(true);
}

void HDHRConfig::UpdateProgramListEntry(uint16_t ProgramNumber, string ProgramStr)
{
	GtkTreeIter iter;
	gboolean iter_valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ProgramListStore), &iter);

	gint32 iterProgramNumber;
	string iterProgramStr;

	while (iter_valid) {
		gchar *itercProgramStr;
		gtk_tree_model_get(GTK_TREE_MODEL(ProgramListStore), &iter, 0, &iterProgramNumber, 1, &itercProgramStr, -1);
		iterProgramStr = itercProgramStr;
		g_free((void *) itercProgramStr);

		if (ProgramNumber == iterProgramNumber) {
			break;
		}

		iter_valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(ProgramListStore), &iter);
	}

	if (!iter_valid) {
		gtk_list_store_append(ProgramListStore, &iter);
	} else if (iterProgramStr == ProgramStr) {
		return;
	}

	gtk_list_store_set(ProgramListStore, &iter, 0, ProgramNumber, 1, ProgramStr.c_str(), -1);
}

void HDHRConfig::UpdateProgramList(char *streaminfo)
{
	if (!streaminfo) {
		/* Firmware does not support stream info. */
		return;
	}

	/*
	 * Update program list.
	 */
	while (1) {
		char *end = strchr(streaminfo, '\n');
		if (!end) {
			break;
		}

		*end++ = 0;
		string ProgramStr = streaminfo;

		int ColonIndex = ProgramStr.find_first_of(":");
		if (ColonIndex <= 0) {
			streaminfo = end;
			continue;
		}

		uint16_t ProgramNumber = atoi(ProgramStr.substr(0, ColonIndex).c_str());
		ProgramStr = ProgramStr.substr(ColonIndex + 1);

		UpdateProgramListEntry(ProgramNumber, ProgramStr);

		streaminfo = end;
	}

	/*
	 * Update VLC button.
	 */
	if (ProgramListStore->length == 0) {
		gtk_widget_set_sensitive(GTK_WIDGET(LaunchVlcBtn), false);
		return;
	}

	/*
	 * Automatic first selection.
	 */
	if (gtk_combo_box_get_active(ProgramList) < 0) {
		gtk_combo_box_set_active(ProgramList, 0);
	}

	if (Viewer->Active()) {
		gtk_widget_set_sensitive(GTK_WIDGET(LaunchVlcBtn), false);
		return;
	}
	gtk_widget_set_sensitive(GTK_WIDGET(LaunchVlcBtn), true);
}

void HDHRConfig::ClearProgramList()
{
	gtk_list_store_clear(ProgramListStore);
	gtk_widget_set_sensitive(GTK_WIDGET(LaunchVlcBtn), false);
}

void HDHRConfig::SetChannelMapEdit(gchar *ChannelMap)
{
	GtkTreeModel *model = gtk_combo_box_get_model(ChannelMapEdit);
	GtkTreeIter iter;
	gboolean iter_valid = gtk_tree_model_get_iter_first(model, &iter);
	while (iter_valid) {
		gchar *string;
		gtk_tree_model_get(model, &iter, 0, &string, -1);
		if (strcmp(string, ChannelMap) == 0) {
			gtk_combo_box_set_active_iter(ChannelMapEdit, &iter);
			ChannelMapEdit_Changed();
			g_free((void *) string);
			return;
		}
		g_free((void *) string);
		iter_valid = gtk_tree_model_iter_next(model, &iter);
	}
}

void HDHRConfig::ScanBtnRefresh(void)
{
	int SavedScanStep = ScanStep;

	if (ScanStep < 0) {
		gtk_toggle_button_set_active(ScanDownBtn, true);
		gtk_toggle_button_set_active(ScanUpBtn, false);
	} else if (ScanStep == 0) {
		gtk_toggle_button_set_active(ScanDownBtn, false);
		gtk_toggle_button_set_active(ScanUpBtn, false);
	} else if (ScanStep > 0) {
		gtk_toggle_button_set_active(ScanDownBtn, false);
		gtk_toggle_button_set_active(ScanUpBtn, true);
	}

	ScanStep = SavedScanStep;
}

void HDHRConfig::UpdateTunerPageSelectProgram(uint16_t ProgramNumber)
{
	LastProgramNumber = ProgramNumber;
	if (ProgramNumber == 0) {
		return;
	}

	GtkTreeIter iter;
	gboolean iter_valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ProgramListStore), &iter);

	gint32 iterProgramNumber;
	string iterProgramStr;

	while (iter_valid) {
		gchar *itercProgramStr;
		gtk_tree_model_get(GTK_TREE_MODEL(ProgramListStore), &iter, 0, &iterProgramNumber, 1, &itercProgramStr, -1);
		iterProgramStr = itercProgramStr;
		g_free((void *) itercProgramStr);

		if (ProgramNumber == iterProgramNumber) {
			gtk_combo_box_set_active_iter(ProgramList, &iter);
			return;
		}

		iter_valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(ProgramListStore), &iter);
	}
}

void HDHRConfig::UpdateTunerPage()
{
	char *cStatusStr;
	int ret = hdhomerun_device_get_tuner_status(SelectedDev->hd, &cStatusStr, &LastStatus);
	if (ret < 0) {
		CommunicationError();
		return;
	}
	if (ret == 0) {
		cStatusStr = (char *)"";
	}

	GString *text = g_string_new(NULL);

	gtk_entry_set_text(PhysicalChannelStatus, LastStatus.channel);
	if (LastStatus.lock_supported) {
		char *Ptr = strchr(LastStatus.channel, ':');
		if (Ptr) {
			g_string_sprintf(text, "%s:%s", LastStatus.lock_str, ++Ptr);
			gtk_entry_set_text(PhysicalChannelStatus, text->str);
		}
	}

	gtk_progress_bar_set_fraction(SignalStrengthStatus, ((double) LastStatus.signal_strength) / 100);
	g_string_sprintf(text, "%d%%", LastStatus.signal_strength);
	gtk_progress_bar_set_text(SignalStrengthStatus, text->str);
	ColorizeProgress(SignalStrengthStatus, hdhomerun_device_get_tuner_status_ss_color(&LastStatus));
	char *Ptr = strstr(cStatusStr, "ss=");
	if (Ptr) {
		signed int SignalStrengthDBMV;
		if (sscanf(Ptr, "ss=%*u(%d)", &SignalStrengthDBMV) == 1) {
			g_string_sprintf(text, "%d dBmV", SignalStrengthDBMV);
			gtk_progress_bar_set_text(SignalStrengthStatus, text->str);
		}
	}

	gtk_progress_bar_set_fraction(SignalQualityStatus, ((double)LastStatus.signal_to_noise_quality) / 100);
	g_string_sprintf(text, "%d%%", LastStatus.signal_to_noise_quality);
	gtk_progress_bar_set_text(SignalQualityStatus, text->str);
	ColorizeProgress(SignalQualityStatus, hdhomerun_device_get_tuner_status_snq_color(&LastStatus));
	Ptr = strstr(cStatusStr, "snq=");
	if (Ptr) {
		signed int SignalQualityDB;
		if (sscanf(Ptr, "snq=%*u(%d)", &SignalQualityDB) == 1) {
			g_string_sprintf(text, "%d dB", SignalQualityDB);
			gtk_progress_bar_set_text(SignalQualityStatus, text->str);
		}
	}

	gtk_progress_bar_set_fraction(SymbolQualityStatus, ((double)LastStatus.symbol_error_quality) / 100);
	g_string_sprintf(text, "%d%%", LastStatus.symbol_error_quality);
	gtk_progress_bar_set_text(SymbolQualityStatus, text->str);
	ColorizeProgress(SymbolQualityStatus, hdhomerun_device_get_tuner_status_seq_color(&LastStatus));

	g_string_sprintf(text, "%.3f Mbps", (double)(LastStatus.packets_per_second * 1316 * 8) / 1000000.0);
	gtk_entry_set_text(NetworkRateStatus, text->str);

	g_string_free(text, true);

	char *streaminfo = NULL;
	if (hdhomerun_device_get_tuner_streaminfo(SelectedDev->hd, &streaminfo) < 0) {
		CommunicationError();
		return;
	}
	UpdateProgramList(streaminfo);

	/* Read channel map. */
	char *cChannelMap;
	ret = hdhomerun_device_get_tuner_channelmap(SelectedDev->hd, &cChannelMap);
	if (ret < 0) {
		CommunicationError();
		return;
	}
	if (ret == 0) {
		cChannelMap = (char *)"";
	}

	string ChannelMap = cChannelMap;
	if (ChannelMap != LastChannelMap) {
		LastChannelMap = ChannelMap;
		SetChannelMapEdit((char *)ChannelMap.c_str());
	}

	/* Read channel number. */
	uint32_t Channel = 0;

	char *ChannelStr = strchr(LastStatus.channel, ':');
	if (ChannelStr) {
		Channel = atol(ChannelStr + 1);
		if ((Channel >= 1000000) && ChannelList) {
			Channel = hdhomerun_channel_frequency_to_number(ChannelList, Channel);
		}
	}

	double Minimum, Maximum;
	gtk_spin_button_get_range(ChannelNumberSpin, &Minimum, &Maximum);

	if (Channel < (uint32_t)Minimum) {
		Channel = (uint32_t)Minimum;
	}
	if (Channel > (uint32_t)Maximum) {
		Channel = (uint32_t)Maximum;
	}

	if (Channel != LastChannel) {
		LastChannel = Channel;
		gtk_spin_button_set_value(ChannelNumberSpin, (gdouble)Channel);
	}

	/* Read program number. */
	char *cProgramStr;
	ret = hdhomerun_device_get_tuner_program(SelectedDev->hd, &cProgramStr);
	if (ret < 0) {
		CommunicationError();
		return;
	}
	if (ret == 0) {
		cProgramStr = (char *)"0";
	}

	uint32_t ProgramNumber = atol(cProgramStr);
	if (ProgramNumber != LastProgramNumber) {
		UpdateTunerPageSelectProgram(ProgramNumber);
	}
}

void HDHRConfig::UpdateUpgradePage(bool FirstTime)
{
	if (!FirstTime) {
		return;
	}

	char *Version;
	int ret = hdhomerun_device_get_version(SelectedDev->hd, &Version, NULL);
	if (ret < 0) {
		CommunicationError();
		return;
	}

	if (ret == 0) {
		/* Unknown firmware */
		return;
	}

	GString *cFirmware = g_string_new(NULL);
	g_string_sprintf(cFirmware, _("Firmware Version: %s"), Version);
	gtk_label_set_text(FirmwareVersion, cFirmware->str);
	g_string_free(cFirmware, true);
}

void HDHRConfig::UpdateStatus(bool FirstTime)
{
	ScanBtnRefresh();

	if (!SelectedDev) {
		return;
	}

	int page = gtk_notebook_get_current_page(Tab);

	if (page == 0) {
		SuppressSetChannel = true;
		UpdateTunerPage();
		SuppressSetChannel = false;
	}

	if (page == 1) {
		UpdateUpgradePage(FirstTime);
	}
}

void HDHRConfig::Timer_Tick(void)
{
	UpdateStatus(false);
	ScanNext();
}

void HDHRConfig::ChannelMapEdit_Changed(void)
{
	/*
	 * New channel map.
	 */
	if (ChannelList) {
		hdhomerun_channel_list_destroy(ChannelList);
		ChannelList = NULL;
	}

	const gchar *cChannelMap = gtk_combo_box_get_active_text(ChannelMapEdit);
	if (!cChannelMap) {
		gtk_spin_button_set_range(ChannelNumberSpin, 0, 255);
		return;
	}
	ChannelList = hdhomerun_channel_list_create(cChannelMap);
	if (!ChannelList) {
		gtk_spin_button_set_range(ChannelNumberSpin, 0, 255);
		return;
	}

	/*
	 * Set channel maximum.
	 */
	uint16_t Maximum = 0;
	struct hdhomerun_channel_entry_t *Channel = hdhomerun_channel_list_first(ChannelList);
	while (Channel) {
		uint16_t ChannelNumber = hdhomerun_channel_entry_channel_number(Channel);
		if (ChannelNumber > Maximum) {
			Maximum = ChannelNumber;
		}

		Channel = hdhomerun_channel_list_next(ChannelList, Channel);
	}

	gtk_spin_button_set_range(ChannelNumberSpin, 0, (gdouble)Maximum);
	ChannelNumberSpin_Changed();
}

int HDHRConfig::ChannelNumberSpin_ChangedGetChannelNumber()
{
	int NewChannel = gtk_spin_button_get_value_as_int(ChannelNumberSpin);
	if (NewChannel == 0) {
		return NewChannel;
	}

	if (!ChannelList) {
		return NewChannel;
	}
	if (hdhomerun_channel_number_to_frequency(ChannelList, (uint16_t)NewChannel) != 0) {
		return NewChannel;
	}

	int Direction = (NewChannel - LastChannel) >= 0 ? 1 : -1;

	double Minimum, Maximum;
	gtk_spin_button_get_range(ChannelNumberSpin, &Minimum, &Maximum);

	while (1) {
		NewChannel += Direction;

		if (NewChannel <= 0) {
			return 0;
		}
		if (NewChannel >= (int)Maximum) {
			return (int)Maximum;
		}
		if (hdhomerun_channel_number_to_frequency(ChannelList, (uint16_t)NewChannel) != 0) {
			return NewChannel;
		}
	}
}

void HDHRConfig::ChannelNumberSpin_Changed(void)
{
	/*
	 * Stop current channel.
	 */
	RespawnVLC = false;
	StopVLC();
	ClearProgramList();

	if (SuppressSetChannel) {
		return;
	}
	if (!SelectedDev) {
		return;
	}

	/*
	 * Validate channel number.
	 */
	int NewChannel = ChannelNumberSpin_ChangedGetChannelNumber();
	if (NewChannel != gtk_spin_button_get_value_as_int(ChannelNumberSpin)) {
		gtk_spin_button_set_value(ChannelNumberSpin, (gdouble)NewChannel);
		return;
	}

	LastChannel = NewChannel;

	/*
	 * Tune new channel.
	 */
	if (NewChannel == 0) {
		if (hdhomerun_device_set_tuner_channel(SelectedDev->hd, "none") < 0) {
			CommunicationError();
			return;
		}
		return;
	}

	const gchar *ChannelMap = gtk_combo_box_get_active_text(ChannelMapEdit);
	if (!ChannelMap) {
		ScanStep = 0;
		return;
	}
	if (hdhomerun_device_set_tuner_channelmap(SelectedDev->hd, ChannelMap) < 0) {
		CommunicationError();
		return;
	}

	char cChannel[64];
	snprintf(cChannel, sizeof(cChannel), "auto:%u", (unsigned int)NewChannel);
	int ret = hdhomerun_device_set_tuner_channel(SelectedDev->hd, cChannel);
	if (ret < 0) {
		CommunicationError();
		return;
	}
	if (ret == 0) {
		ScanStep = 0;
		return;
	}
}

void HDHRConfig::ScanNext(void)
{
	if (ScanStep == 0) {
		return;
	}

	if (LastStatus.lock_supported) {
		ScanStep = 0;
		return;
	}

	if (!LastStatus.signal_present || LastStatus.lock_unsupported) {
		/* Force timeout. */
		ScanTestTimeout = 0;
	}

	if (ScanTestTimeout > 0) {
		ScanTestTimeout--;
		return;
	}

	double chMin, chMax;
	gtk_spin_button_get_range(ChannelNumberSpin, &chMin, &chMax);

	int NextChannel = gtk_spin_button_get_value_as_int(ChannelNumberSpin) + ScanStep;
	if (NextChannel < chMin || NextChannel > chMax) {
		ScanStep = 0;
		return;
	}

	ScanTestTimeout = 2;
	gtk_spin_button_set_value(ChannelNumberSpin, (gdouble) NextChannel);
}

void HDHRConfig::ScanDownBtn_Click(void)
{
	if (gtk_toggle_button_get_active(ScanDownBtn) == false) {
		ScanStep = 0;
		return;
	}

	memset(&LastStatus, 0, sizeof(LastStatus));
	ScanStep = -1;
	ScanTestTimeout = 0;
	ScanNext();
}

void HDHRConfig::ScanUpBtn_Click(void)
{
	if (gtk_toggle_button_get_active(ScanUpBtn) == false) {
		ScanStep = 0;
		return;
	}

	memset(&LastStatus, 0, sizeof(LastStatus));
	ScanStep = 1;
	ScanTestTimeout = 0;
	ScanNext();
}

void HDHRConfig::SetProgramAndStartVLC()
{
	if (!Viewer->Supported()) {
		Error(_("ERROR: VLC not installed"));
		return;
	}

	/*
	 * Program number.
	 */
	uint16_t ProgramNumber = 0;

	GtkTreeIter iter;
	if (gtk_combo_box_get_active_iter(ProgramList, &iter)) {
		uint32_t iterProgramNumber;
		gtk_tree_model_get(GTK_TREE_MODEL(ProgramListStore), &iter, 0, &iterProgramNumber, -1);
		ProgramNumber = iterProgramNumber;
	}

	hdhomerun_debug_printf(dbg, "MainForm::SetProgramAndStartVLC: %08lX-%u %s %u\n", SelectedDev->DeviceID, SelectedDev->TunerIndex, LastStatus.channel, ProgramNumber);

	/*
	 * Set program.
	 */
	char Program[64];
	sprintf(Program, "%u", ProgramNumber);

	if (hdhomerun_device_set_tuner_program(SelectedDev->hd, Program) < 0) {
		CommunicationError();
		return;
	}

	/*
	 * Launch VLC.
	 */
	if (!Viewer->Start(SelectedDev)) {
		Error(_("ERROR: unable to launch VLC"));
		return;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(LaunchVlcBtn), false);
	gtk_widget_set_sensitive(GTK_WIDGET(StopVlcBtn), true);
}

void HDHRConfig::StopVLC()
{
	if (!Viewer) {
		return;
	}
	Viewer->Stop();
	gtk_widget_set_sensitive(GTK_WIDGET(StopVlcBtn), false);
}

void HDHRConfig::SigChild(int pid)
{
#ifndef __WINDOWS__
	wait(NULL);
#endif
	if (!RespawnVLC) {
		StopVLC();
	}
	RespawnVLC = false;
}

void HDHRConfig::ProgramList_Changed()
{
	if (!SelectedDev) {
		return;
	}

	if (Viewer && Viewer->Active()) {
		RespawnVLC = true;
		StopVLC();
		SetProgramAndStartVLC();
	}
}

void HDHRConfig::LaunchVlcBtn_Click()
{
	if (!SelectedDev) {
		return;
	}

	RespawnVLC = false;
	StopVLC();
	SetProgramAndStartVLC();
}

void HDHRConfig::StopVlcBtn_Click()
{
	RespawnVLC = false;
	StopVLC();
}

void HDHRConfig::UpgradeFilename_Changed()
{
	const gchar *filename = gtk_entry_get_text(UpgradeFilename);
	gtk_widget_set_sensitive(GTK_WIDGET(UpgradeBtn), filename[0] != '\0');
}

void HDHRConfig::UpgradeFilenameBtn_Click()
{
	const gchar *filename = gtk_entry_get_text(UpgradeFilename);

#ifdef __WINDOWS__
	OPENFILENAMEA ofn;
	char szFileName[MAX_PATH];
	strncpy(szFileName, filename, MAX_PATH);

	szFileName[MAX_PATH - 1] = '\0';

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = (HWND) gdk_win32_drawable_get_handle(GTK_WIDGET(hdhomerun_config)->window);
	ofn.lpstrFilter = "Firmware (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "bin";

	gtk_window_set_accept_focus(GTK_WINDOW(hdhomerun_config), false);
	if (GetOpenFileNameA(&ofn)) {
		gtk_entry_set_text(GTK_ENTRY(UpgradeFilename), szFileName);
	}
	gtk_window_set_accept_focus(GTK_WINDOW(hdhomerun_config), true);

#else
	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open File"), GTK_WINDOW(hdhomerun_config), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (filename[0] != '\0') {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filename);
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(GTK_ENTRY(UpgradeFilename), filename);
		g_free((void *) filename);
	}

	gtk_widget_destroy(dialog);
#endif

	gtk_widget_grab_focus(GTK_WIDGET(UpgradeFilename));
}

void HDHRConfig::UpgradeBtn_Click()
{
	if (!SelectedDev) {
		return;
	}

	const gchar *filename = gtk_entry_get_text(GTK_ENTRY(UpgradeFilename));
	FILE *fp = fopen(filename, "rb");

	if (!fp) {
		Error(_("ERROR: Unable to open upgrade file"));
		return;
	}

	if (hdhomerun_device_upgrade(SelectedDev->hd, fp) <= 0) {
		Error(_("ERROR: Error attempting to upgrade HDHomeRun"));
		fclose(fp);
		return;
	}

	fclose(fp);
	Rescan();
}

void HDHRConfig::Debug(void)
{
	GtkWindow *win = GTK_WINDOW(lookup_widget("hdhomerun_config"));
	gtk_window_set_title(GTK_WINDOW(hdhomerun_config), _("HDHomeRun Config - Debug"));
	DebugExpireTime = 0xFFFFFFFFFFFFFFFFULL;
	Rescan();
}
