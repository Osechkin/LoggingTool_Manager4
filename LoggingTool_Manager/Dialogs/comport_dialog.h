#ifndef COMPORT_DIALOG_H
#define COMPORT_DIALOG_H

#include "qextserialport.h"

#include "ui_comport_dialog.h"

class COMPortDialog : public QDialog, Ui::COMPortDialog
{
    Q_OBJECT

public:
    explicit COMPortDialog(QString _port, PortSettings _settings, bool _auto_search, QObject *parent);

    QString getCOMPort() { return port_name; }
    PortSettings getPortSettings() { return COM_Settings; }
	bool getAutoSearchState() { return auto_search; }
	void enableAutoSearch(bool flag);
	void setTitle(QString str) { setWindowTitle(str); }

private:
    void setConnections();

    PortSettings COM_Settings;
    QString port_name;
	bool auto_search;

private slots:
    void setCOMPort(QString str);
    void setBaudrate(QString str);
    void setDataBits(QString str);
    void setParity(QString str);
    void setStopBits(QString str);
	void setAutoSearch(bool flag);
};

#endif // COMPORT_DIALOG_H
