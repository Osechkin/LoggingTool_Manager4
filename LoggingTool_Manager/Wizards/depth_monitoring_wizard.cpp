#include <QTimer>
#include <QMessageBox>

#include "tools_general.h"

#include "depth_monitoring_wizard.h"


static int port_num = 0;
static uint32_t uid = 0;


DepthMonitoringWizard::DepthMonitoringWizard(Clocker *clocker, QWidget *parent) : QWidget(parent), ui(new Ui::DepthMonitoringWizard)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	
	ui->gridLayout_2->removeItem(ui->verticalSpacer);

	ui->lblDepth->setText("");
	ui->lblRate->setText("");
	ui->lblTension->setText("");

	ui->chboxDepth->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxRate->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxTension->setStyleSheet("QCheckBox { color: darkblue }");

	ui->cboxDepth->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxRate->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxTension->setStyleSheet("QComboBox { color: darkblue }");

	QFont pbt_font = ui->pbtConnect->font();
	pbt_font.setBold(false);
	ui->pbtConnect->setFont(pbt_font);

	ui->cboxPort->setVisible(false);
	ui->label->setVisible(false);
	ui->line->setVisible(false);

	depth = 0;
	rate = 0;
	tension = 0;

	depth_flag = true;
	rate_flag = false;
	tension_flag = false;

	depth_active = true;
	rate_active = false;
	tension_active = false;

	depth_units_list << "m" << "ft";
	rate_units_list << "m/hr" << "m/s"<< "ft/s" << "ft/hr";
	tension_units_list << "kgf";	
	k_depth = 1;
	k_rate = 3600;
	k_tension = 1;
	
	ui->cboxDepth->addItems(depth_units_list);
	ui->cboxRate->addItems(rate_units_list);
	ui->cboxTension->addItems(tension_units_list);
	
	ui->chboxDepth->setChecked(true);
	ui->chboxRate->setChecked(false);
	ui->chboxTension->setChecked(false);

	ui->cboxDepth->setEnabled(true);
	ui->cboxRate->setEnabled(false);
	ui->cboxTension->setEnabled(false);

	ui->pbtConnect->setIconSize(QSize(16,16));
		
	connectionWidget = new ImpulsConnectionWidget(this);
	connectionWidget->setReport(ConnectionState::State_No);
	
	this->clocker = clocker;
	
	is_connected = false;
	device_is_searching = false;

	COM_Port = new COM_PORT;
	initCOMSettings(COM_Port);	

	QStringList ports = availableCOMPorts();
	ui->cboxPort->addItems(ports);
			
	ui->pbtConnect->setText(tr("Connect to Depth Meter"));
	ui->pbtConnect->setIcon(QIcon(":/images/add.png"));

	COMPort_Name = "";
	if (ports.isEmpty()) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("No COM-Port was detected!"), QMessageBox::Ok, QMessageBox::Ok);
		//return;
	}
	else
	{
		for (int i = ports.count()-1; i >= 0; --i)
		{
			QString port_name = ports[i];
			QextSerialPort *port = new QextSerialPort(port_name, COM_Port->COM_Settings);

			depth_communicator = NULL;
			bool res = port->open(QextSerialPort::ReadWrite);
			if (res) 	
			{
				COM_Port->COM_port = port;
				depth_communicator = new DepthCommunicator(COM_Port->COM_port, clocker);
				setDepthCommunicatorConnections();
				COMPort_Name = port_name;

				ui->cboxPort->setCurrentText(port_name);
				ui->pbtConnect->setChecked(true);

				depth_communicator->start(QThread::NormalPriority);
				is_connected = true;
				timer.start(1000);	
							
				ui->pbtConnect->setText(tr("Disconnect from Depth Meter"));
				ui->pbtConnect->setIcon(QIcon(":/images/remove.png"));
				setConnection();		

				return;
			}
			else
			{
				delete port;
				COM_Port->COM_port = NULL;
				COMPort_Name = "";				
			}
		}	
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Depth Meter!"), QMessageBox::Ok, QMessageBox::Ok);
	}		
	
	setConnection();	
}

DepthMonitoringWizard::~DepthMonitoringWizard()
{
	delete ui;		
	
	if (depth_communicator != NULL)
	{
		depth_communicator->exit();
		depth_communicator->wait();
		delete depth_communicator;	
	}
		
	delete COM_Port->COM_port;
	delete COM_Port;
}

void DepthMonitoringWizard::setConnection()
{
	connect(ui->chboxDepth, SIGNAL(stateChanged(int)), this, SLOT(includeParameter(int)));
	connect(ui->chboxRate, SIGNAL(stateChanged(int)), this, SLOT(includeParameter(int)));
	connect(ui->chboxTension, SIGNAL(stateChanged(int)), this, SLOT(includeParameter(int)));

	connect(ui->cboxDepth, SIGNAL( currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->cboxRate, SIGNAL( currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->cboxTension, SIGNAL( currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(onTime()));
		
	connect(ui->cboxPort, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeCOMPort(QString)));
	connect(ui->pbtConnect, SIGNAL(toggled(bool)), this, SLOT(connectDepthMeter(bool)));
}

void DepthMonitoringWizard::setDepthCommunicatorConnections()
{
	connect(depth_communicator, SIGNAL(measured_data(uint32_t, uint8_t, double)), this, SLOT(getMeasuredData(uint32_t, uint8_t, double)));
	connect(depth_communicator, SIGNAL(data_timed_out(uint32_t, uint8_t)), this, SLOT(measureTimedOut(uint32_t, uint8_t)));
	connect(this, SIGNAL(to_measure(uint32_t, uint8_t)), depth_communicator, SLOT(toMeasure(uint32_t, uint8_t)));
}

void DepthMonitoringWizard::includeParameter(int state)
{
	QCheckBox *chbox = (QCheckBox*)sender();
	if (!chbox) return;

	bool flag;
	if (state == Qt::Checked) flag = true;
	else if (state == Qt::Unchecked) flag = false;

	if (chbox == ui->chboxDepth) 
	{
		ui->lblDepth->setText("");
		ui->cboxDepth->setEnabled(flag);
		depth_active = flag;
	}
	else if (chbox == ui->chboxRate)
	{
		ui->lblRate->setText("");
		ui->cboxRate->setEnabled(flag);
		rate_active = flag;
	}
	else if (chbox == ui->chboxTension)
	{
		ui->lblTension->setText("");
		ui->cboxTension->setEnabled(flag);
		tension_active = flag;
	}
}

/*void DepthMonitoringWizard::searchDevice()
{
	is_connected = false;
	device_is_searching = true;
	
	searchingDevice(COM_Port);
}

void DepthMonitoringWizard::searchingDevice(COM_PORT *_com_port)
{
	if (is_connected) return;

	while (!isAvailableCOMPort(COM_Port, ++port_num) && port_num < 16);
	
	if (port_num < 16)
	{
		depth_communicator->freeze();
		bool res = COM_Port->COM_port->open(QextSerialPort::ReadWrite);
		depth_communicator->setPort(COM_Port->COM_port);
		depth_communicator->wake();

		emit to_measure(++uid, DEVICE_SEARCH);
	}
	else 
	{		
		is_connected = false;
		device_is_searching = false;
		connectionWidget->setReport(ConnectionState::State_No);
		
		port_num = 0;

		timer.start(1000);
	}	
}*/

bool DepthMonitoringWizard::isAvailableCOMPort(COM_PORT *_com_port, int port_num)
{
	QString port_name = QString("COM%1").arg(port_num);
	QextSerialPort *port = new QextSerialPort(port_name, _com_port->COM_Settings);
	bool res = port->open(QextSerialPort::ReadWrite);
	if (res)
	{
		_com_port->COM_port = port;
		_com_port->COM_port->setPortName(port_name);
		port->close();
		return true;
	}
	delete port;

	return false;
}

QStringList DepthMonitoringWizard::availableCOMPorts()
{
	ui->cboxPort->clear();
	if (COM_Port->COM_port != NULL) COM_Port->COM_port->close();

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
	
	return out;
}

void DepthMonitoringWizard::initCOMSettings(COM_PORT *com_port)
{
	com_port->COM_port = NULL;

	com_port->COM_Settings.BaudRate = BAUD9600;
	com_port->COM_Settings.DataBits = DATA_8;
	com_port->COM_Settings.Parity = PAR_NONE;
	com_port->COM_Settings.StopBits = STOP_1;
	com_port->COM_Settings.FlowControl = FLOW_OFF;
	com_port->COM_Settings.Timeout_Millisec = 0;

	com_port->connect_state = false;
}


void DepthMonitoringWizard::getMeasuredData(uint32_t _uid, uint8_t _type, double val)
{
	if (!is_connected) is_connected = true;
	if (device_is_searching) 
	{
		timer.start();
		device_is_searching = false;
	}

	switch (_type)
	{
	case DEPTH_DATA:	depth = val;	depth_flag = true;		break;
	case RATE_DATA:		rate = val;		rate_flag = true;		break;
	case TENSION_DATA:	tension = val;	tension_flag = true;	break;
	case DEVICE_SEARCH:	device_is_searching = false;			break;
	default: break;
	}

	showData(_type, val);
}

void DepthMonitoringWizard::measureTimedOut(uint32_t _uid, uint8_t _type)
{	
	switch (_type)
	{
	case DEPTH_DATA:	ui->lblDepth->setText("");		depth_flag = false;		break;
	case RATE_DATA:		ui->lblRate->setText("");		rate_flag = false;		break;
	case TENSION_DATA:	ui->lblTension->setText("");	tension_flag = false;	break;
	case DEVICE_SEARCH:	
		{
			ui->lblDepth->setText("");
			ui->lblRate->setText("");
			ui->lblTension->setText("");

			connectionWidget->setReport(ConnectionState::State_No); 
			break;
		}
	default: break;
	}

	connectionWidget->setReport(ConnectionState::State_No);
}


void DepthMonitoringWizard::onTime()
{	
	if (is_connected)
	{		
		if (depth_active) emit to_measure(++uid, DEPTH_DATA);
		if (rate_active) emit to_measure(++uid, RATE_DATA);
		if (tension_active) emit to_measure(++uid, TENSION_DATA);
	}	
}

void DepthMonitoringWizard::showData(uint8_t type, double val)
{	
	QString str = "<font color=darkGreen><b>%1</font>";
	
	switch (type)
	{
	case DEPTH_DATA:	
		{
			double value = val * k_depth;  
			QString str_value = QString::number(value);
			str = QString(str).arg(str_value);
			ui->lblDepth->setText(str); 
			break;
		}
	case RATE_DATA:		
		{
			double value = val * k_rate;  
			QString str_rate = QString::number(value);
			str = QString(str).arg(str_rate);
			ui->lblRate->setText(str); 
			break;
		}
	case TENSION_DATA:	
		{
			double value = val * k_tension;  
			QString str_value = QString::number(value);
			str = QString(str).arg(str_value);
			ui->lblTension->setText(str); 
			break;
		}
	case DEVICE_SEARCH:	
		{
			ui->lblDepth->setText(""); 
			ui->lblRate->setText("");
			ui->lblTension->setText("");
			break;
		}
	default: break;
	}

	connectionWidget->setReport(ConnectionState::State_OK);
}

void DepthMonitoringWizard::changeUnits(QString str)
{
	QComboBox *cbox = (QComboBox*)sender();
	if (!cbox) return;

	if (cbox == ui->cboxDepth)
	{		
		if (str == depth_units_list[0]) k_depth = 1;				// [m]
		else if (str == depth_units_list[1]) k_depth = 1/0.3048;	// [ft]
		else k_depth = 0;
	}
	else if (cbox == ui->cboxRate)
	{		
		if (str == rate_units_list[0]) k_rate = 3600;				// [m/hrs]
		else if (str == rate_units_list[1]) k_rate = 1;				// [m/s]
		else if (str == rate_units_list[2]) k_rate = 3600/0.3048;	// [ft/hrs] 
		else if (str == rate_units_list[3]) k_rate = 1/0.3048;		// [ft/s]
		else k_rate = 0;
	}
	else if (cbox == ui->cboxTension)
	{		
		if (str == rate_units_list[0]) k_tension = 1;				// [kgf]
		else k_tension = 0;
	}
}

void DepthMonitoringWizard::changeCOMPort(QString str)
{
	timer.stop();

	if (COM_Port->COM_port != NULL) 
	{
		delete COM_Port->COM_port;
		COM_Port->COM_port = NULL;		
	}

	if (depth_communicator != NULL)
	{
		depth_communicator->exit();
		depth_communicator->wait();
		delete depth_communicator;	
		depth_communicator = NULL;
	}	
		
	QString port_name = str;
	QextSerialPort *port = new QextSerialPort(port_name, COM_Port->COM_Settings);

	depth_communicator = NULL;
	bool res = port->open(QextSerialPort::ReadWrite);
	if (res) 	
	{
		COM_Port->COM_port = port;
		depth_communicator = new DepthCommunicator(COM_Port->COM_port, clocker);
		setDepthCommunicatorConnections();
		depth_communicator->start(QThread::NormalPriority);

		is_connected = true;
		timer.start(1000);
	}
	else
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open COM-Port!"), QMessageBox::Ok, QMessageBox::Ok);
	}
}

void DepthMonitoringWizard::changeLocation(Qt::DockWidgetArea area)
{
	if (area == Qt::RightDockWidgetArea || area == Qt::LeftDockWidgetArea)
	{
		ui->gridLayout_2->addItem(ui->verticalSpacer, 3, 0, 1, 1);
		this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	}
	else if (area == Qt::BottomDockWidgetArea || area == Qt::NoDockWidgetArea)
	{
		ui->gridLayout_2->removeItem(ui->verticalSpacer);
		this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	}
}

void DepthMonitoringWizard::connectDepthMeter(bool flag)
{
	if (!flag)
	{
		if (COM_Port->COM_port != NULL) 
		{
			COM_Port->COM_port->close();			

			//delete COM_Port->COM_port;
			//COM_Port->COM_port = NULL;		
		}	

		if (depth_communicator != NULL)
		{
			depth_communicator->exit();
			depth_communicator->wait();
			delete depth_communicator;	
			depth_communicator = NULL;
		}

		timer.stop();
		is_connected = false;
		emit connected(is_connected);

		ui->pbtConnect->setText(tr("Connect to Depth Meter"));
		ui->pbtConnect->setIcon(QIcon(":/images/add.png"));
		ui->lblDepth->setText("");
		ui->lblRate->setText("");
		ui->lblTension->setText("");
	}	
	else
	{
		if (COMPort_Name.isEmpty()) 
		{
			int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Depth Meter!"), QMessageBox::Ok, QMessageBox::Ok);
		}

		if (COM_Port->COM_port != NULL)
		{
			COM_Port->COM_port->close();
			COM_Port->COM_port->setPortName(COMPort_Name);

			if (depth_communicator != NULL)
			{
				depth_communicator->exit();
				depth_communicator->wait();
				delete depth_communicator;
				depth_communicator = NULL;
			}			
			bool res = COM_Port->COM_port->open(QextSerialPort::ReadWrite);
			if (res) 	
			{				
				depth_communicator = new DepthCommunicator(COM_Port->COM_port, clocker);
				setDepthCommunicatorConnections();
				depth_communicator->start(QThread::NormalPriority);

				is_connected = true;
				emit connected(true);

				timer.start(1000);

				ui->pbtConnect->setText(tr("Disconnect from Depth Meter"));
				ui->pbtConnect->setIcon(QIcon(":/images/remove.png"));
			}
			else
			{
				is_connected = false;
				emit connected(false);

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open COM-Port (%1)!").arg(COMPort_Name), QMessageBox::Ok, QMessageBox::Ok);
			}

		}
		else
		{			
			QextSerialPort *port = new QextSerialPort(COMPort_Name, COM_Port->COM_Settings);

			depth_communicator = NULL;
			bool res = port->open(QextSerialPort::ReadWrite);
			if (res) 	
			{
				COM_Port->COM_port = port;
				depth_communicator = new DepthCommunicator(COM_Port->COM_port, clocker);
				setDepthCommunicatorConnections();
				depth_communicator->start(QThread::NormalPriority);

				is_connected = true;
				emit connected(true);

				timer.start(1000);
			}
			else
			{
				is_connected = false;
				emit connected(false);

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open COM-Port (%1)!").arg(COMPort_Name), QMessageBox::Ok, QMessageBox::Ok);
			}
		}		
	}	
}