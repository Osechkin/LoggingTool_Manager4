#ifndef COMMUNICATION_DIALOG_H
#define COMMUNICATION_DIALOG_H


#include "tools_general.h"

#include "ui_communication_dialog.h"


class CommunicationDialog : public QDialog, Ui::CommunicationDialog
{
	Q_OBJECT

public:
	explicit CommunicationDialog(Communication_Settings *_comm_settings, QObject *parent);
		
	Communication_Settings &getCommSettings() { return comm_settings; }

private:
	void setConnections();

	Communication_Settings comm_settings;
	
private slots:
	void setAntinoiseCoding(bool flag);
 	void setInterleaving(bool flag);
 	void setAdaptiveErrorControl(bool flag);
 	void setManualErrorControl(bool flag);
	void setAutoPackLen(bool flag);
	void setFixedPackLen(bool flag);
	void changeCorrectedErrors(int val);
	void changeBlockLen(int val);
	void changePackLen(int val);
	void setPacketDelay(int val);

};

#endif // COMMUNICATION_DIALOG_H