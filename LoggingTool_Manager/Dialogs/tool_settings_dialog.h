#ifndef TOOL_SETTINGS_DIALOG_H
#define TOOL_SETTINGS_DIALOG_H

#include <QDialog>

#include "tools_general.h"

#include "ui_tool_settings_dialog.h"


class ToolSettingsSettingsDialog : public QDialog, public Ui::ToolSettingsDialog
{
	Q_OBJECT

public:
	ToolSettingsSettingsDialog(TCP_Settings &_nmrtool_settings, TCP_Settings &_dmeter_settings, TCP_Settings &_stmotor_settings, QWidget *parent = 0);

	TCP_Settings getNMRToolSettings() { return nmrtool_settings; }
	TCP_Settings getDMeterSettings() { return dmeter_settings; }
	TCP_Settings getStMotorSettings() { return stmotor_settings; }

private:
	TCP_Settings nmrtool_settings;
	TCP_Settings dmeter_settings;
	TCP_Settings stmotor_settings;

private:
	void setConnections();

private slots:
	void inputIP();
	void inputPort(int port);

};

#endif // TOOL_SETTINGS_DIALOG_H