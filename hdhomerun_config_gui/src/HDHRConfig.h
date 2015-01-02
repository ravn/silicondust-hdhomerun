/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#ifndef HDHOMERUN_CONFIG_H
#define HDHOMERUN_CONFIG_H

class HDHRConfig {
public:
	HDHRConfig();
	~HDHRConfig(void);

	void DeviceList_Changed(void);
	void Rescan(void);

	void Tab_Changed(void);

	void ChannelMapEdit_Changed(void);
	void ChannelNumberSpin_Changed(void);
	void ProgramList_Changed(void);
	void ScanDownBtn_Click(void);
	void ScanUpBtn_Click(void);

	void LaunchVlcBtn_Click(void);
	void StopVlcBtn_Click(void);

	void SigChild(int pid);

	void Timer_Tick(void);

	void UpgradeFilename_Changed(void);
	void UpgradeFilenameBtn_Click(void);
	void UpgradeBtn_Click(void);

	void Debug(void);

protected:
	void Error(const gchar * fmt, ...);
	void CommunicationError(void);
	GtkWidget *lookup_widget(const gchar * name);

	GtkWindow *hdhomerun_config;

	/* Device */
	GtkScrolledWindow *scrolledwindow1;
	GtkTreeView *DeviceListTree;
	GtkListStore *DeviceListStore;

	THDHomeRunDeviceList *DeviceList;
	THDHomeRunDevice *SelectedDev;

	GtkNotebook *Tab;

	/* Tuner Configuration */
	GtkComboBox *ChannelMapEdit;
	GtkSpinButton *ChannelNumberSpin;
	GtkComboBox *ProgramList;
	GtkListStore *ProgramListStore;
	GtkToggleButton *ScanDownBtn;
	GtkToggleButton *ScanUpBtn;
	GtkButton *LaunchVlcBtn;
	GtkButton *StopVlcBtn;

	bool SuppressSetChannel;

	void SetChannelMapEdit(gchar * ChannelMap);

	struct hdhomerun_channel_list_t *ChannelList;
	string LastChannelMap;
	int LastChannel;
	int LastProgramNumber;
	int ChannelNumberSpin_ChangedGetChannelNumber();

	void ClearProgramList();
	void UpdateProgramListEntry(uint16_t ProgramNumber, string ProgramStr);
	void UpdateProgramList(char *streaminfo);

	int ScanStep;
	int ScanTestTimeout;
	void ScanNext(void);
	void ScanBtnRefresh(void);

	TViewer *Viewer;
	bool RespawnVLC;
	void SetProgramAndStartVLC(void);
	void StopVLC(void);

	/* Tuner Status */
	GtkEntry *PhysicalChannelStatus;
	GtkProgressBar *SignalStrengthStatus;
	GtkProgressBar *SignalQualityStatus;
	GtkProgressBar *SymbolQualityStatus;
	GtkEntry *RawChannelRateStatus;
	GtkEntry *NetworkRateStatus;

	struct hdhomerun_tuner_status_t LastStatus;

	void UpdateStatus(bool FirstTime);
	void UpdateTunerPageSelectProgram(uint16_t ProgramNumber);
	void UpdateTunerPage(void);

	/* Upgrade Tab */
	GtkEntry *UpgradeFilename;
	GtkButton *UpgradeFilenameBtn;
	GtkButton *UpgradeBtn;
	GtkLabel *FirmwareVersion;

	void UpdateUpgradePage(bool FirstTime);
};

#endif
