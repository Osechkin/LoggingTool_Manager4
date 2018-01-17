#include "tcp_server_dialog.h"

TcpServerDialog::TcpServerDialog(int _port, QWidget *parent) : QDialog(parent)
{
	this->setupUi(this);
	this->setParent(parent);

	port = _port;
	sbxPort->setValue(port);

	connect(sbxPort, SIGNAL(valueChanged(int)), this, SLOT(setPort(int)));
}

void TcpServerDialog::setPort(int str)
{
	port = sbxPort->value();
}