#ifndef CDIAG_SERVER_DIALOG_H
#define CDIAG_SERVER_DIALOG_H

#include <QtGui>

#include "ui_cdiag_server_dialog.h"


class CDiagServerDialog : public QDialog, public Ui::CDiagServerDialog
{
	Q_OBJECT

public:
	explicit CDiagServerDialog(int _port, QWidget *parent);

	int getPort() { return port; }	

private:	
	int port;

	private slots:
		void setPort(int str);		

};

#endif // CDIAG_SERVER_DIALOG_H