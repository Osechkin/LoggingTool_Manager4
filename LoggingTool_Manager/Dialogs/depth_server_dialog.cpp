#include "depth_server_dialog.h"


DepthMeterSettingsDialog::DepthMeterSettingsDialog(COM_PORT *com_port, QWidget *parent)
{
	this->setupUi(this);

	COM_Port = com_port;
	COMPort_Name = com_port->COM_port->portName();

	QStringList out;
	for (int i = 1; i <= 15; i++)
	{
		QString port_name = QString("COM%1").arg(i);
		QextSerialPort *port = new QextSerialPort(port_name, COM_Port->COM_Settings);
		bool res = port->open(QextSerialPort::ReadWrite);
		if (res)
		{
			out.append(port_name);
			port->close();
		}
		delete port;
	}
	cboxCOMPorts->clear();
	cboxCOMPorts->addItems(out);
	if (COM_Port->COM_port != NULL) 
	{
		QString port_name = COM_Port->COM_port->portName();
		if (out.contains(port_name)) cboxCOMPorts->setCurrentText(port_name);
	}		

	setConnections();
}

void DepthMeterSettingsDialog::setConnections()
{
	connect(cboxCOMPorts, SIGNAL(currentIndexChanged(QString)), this, SLOT(changePortName(QString)));
}

QString DepthMeterSettingsDialog::getCOMPortName()
{
	//QString port_name = "";
	//if (COM_Port->COM_port != NULL) port_name = COM_Port->COM_port->portName();
	
	return COMPort_Name;
}

void DepthMeterSettingsDialog::changePortName(QString str)
{
	COMPort_Name = str;
}