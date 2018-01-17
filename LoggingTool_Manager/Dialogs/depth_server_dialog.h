#ifndef DEPTH_SERVER_DIALOG_H
#define DEPTH_SERVER_DIALOG_H

#include <QDialog>

#include "tools_general.h"

#include "ui_depth_server_dialog.h"


class DepthMeterSettingsDialog : public QDialog, public Ui::DepthServerDialog
{
	Q_OBJECT

public:
	DepthMeterSettingsDialog(COM_PORT *com_port, QWidget *parent = 0);

	QString getCOMPortName();

private:
	COM_PORT *COM_Port;
	QString COMPort_Name;

private:
	void setConnections();

private slots:
	void changePortName(QString str);

};

#endif // DEPTH_SERVER_DIALOG_H