#ifndef TCP_SERVER_DIALOG_H
#define TCP_SERVER_DIALOG_H

#include <QtGui>

#include "ui_tcp_server_dialog.h"


class TcpServerDialog : public QDialog, public Ui::TcpServerDialog
{
	Q_OBJECT

public:
	explicit TcpServerDialog(int _port, QWidget *parent);

	int getPort() { return port; }	

private:	
	int port;

private slots:
	void setPort(int str);		

};

#endif // TCP_SERVER_DIALOG_H