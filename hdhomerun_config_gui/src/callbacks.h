/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

void on_DeviceListTree_selection_changed(GtkTreeSelection *selection, gpointer user_data);
void on_RescanBtn_clicked(GtkButton *button, gpointer user_data);

void on_Tab_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data);

void on_ChannelMapEdit_changed(GtkComboBox *combobox, gpointer user_data);
void on_ChannelNumberSpin_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_ChannelNumberSpin_activate(GtkEntry *entry, gpointer user_data);
void on_ProgramList_changed(GtkComboBox *combobox, gpointer user_data);
void on_ScanDownBtn_clicked(GtkButton *button, gpointer user_data);
void on_ScanUpBtn_clicked(GtkButton *button, gpointer user_data);
void on_LaunchVlcBtn_clicked(GtkButton *button, gpointer user_data);
void on_StopVlcBtn_clicked(GtkButton *button, gpointer user_data);
gboolean on_Timer_tick(gpointer user_data);

void on_UpgradeFilename_changed(GtkEditable *editable, gpointer user_data);
void on_UpgradeFilenameBtn_clicked(GtkButton *button, gpointer user_data);
void on_UpgradeBtn_clicked(GtkButton *button, gpointer user_data);

void SigChild(int pid);

gboolean on_hdhomerun_config_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
