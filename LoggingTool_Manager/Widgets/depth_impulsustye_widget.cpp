#include <QMessageBox>
#include <QVariant>

#include "depth_impulsustye_widget.h"


static uint32_t uid = 0;

DepthImpulsUstyeWidget::DepthImpulsUstyeWidget(QSettings *_settings, Clocker *_clocker, TCP_Settings *_socket, QWidget *parent /* = 0 */)  : ui(new Ui::DepthImpulsUstyeWidget)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	ui->lblDepth->setText("");
	ui->lblRate->setText("");
	ui->lblTension->setText("");

	ui->chboxDepth->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxRate->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxTension->setStyleSheet("QCheckBox { color: darkblue }");

	ui->cboxDepth->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxRate->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxTension->setStyleSheet("QComboBox { color: darkblue }");

	ui->pbtConnect->setIconSize(QSize(16,16));
	ui->pbtConnect->setText(tr("Connect to Depth Meter"));
	ui->pbtConnect->setIcon(QIcon(":/images/add.png"));

	QFont pbt_font = ui->pbtConnect->font();
	pbt_font.setBold(false);
	ui->pbtConnect->setFont(pbt_font);

	connectionWidget = new ImpulsConnectionWidget(this);
	connectionWidget->setReport(ConnectionState::State_No);

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
	ui->cboxRate->setEnabled(true);
	ui->cboxTension->setEnabled(true);

	
	is_connected = false;
	device_is_searching = false;

	settings = _settings;
	clocker = _clocker;
	//COM_Port = com_port;
	tcp_settings = _socket;
	depth_communicator = NULL;

	timer.start(1000);	
		
	setConnection();
}

DepthImpulsUstyeWidget::~DepthImpulsUstyeWidget()
{
	delete ui;
	delete connectionWidget;
}


void DepthImpulsUstyeWidget::setConnection()
{
	connect(ui->chboxDepth, SIGNAL(stateChanged(int)), this, SLOT(includeParameter(int)));
	connect(ui->chboxRate, SIGNAL(stateChanged(int)), this, SLOT(includeParameter(int)));
	connect(ui->chboxTension, SIGNAL(stateChanged(int)), this, SLOT(includeParameter(int)));

	connect(ui->cboxDepth, SIGNAL( currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->cboxRate, SIGNAL( currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->cboxTension, SIGNAL( currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));

	connect(ui->pbtConnect, SIGNAL(toggled(bool)), this, SLOT(connectDepthMeter(bool)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(onTime()));
}

void DepthImpulsUstyeWidget::setDepthCommunicatorConnections()
{
	connect(depth_communicator, SIGNAL(measured_data(uint32_t, uint8_t, double)), this, SLOT(getMeasuredData(uint32_t, uint8_t, double)));
	connect(depth_communicator, SIGNAL(data_timed_out(uint32_t, uint8_t)), this, SLOT(measureTimedOut(uint32_t, uint8_t)));
	connect(this, SIGNAL(to_measure(uint32_t, uint8_t)), depth_communicator, SLOT(toMeasure(uint32_t, uint8_t)));
}


void DepthImpulsUstyeWidget::stopDepthMeter()
{
	ui->pbtConnect->setChecked(false);

	timer.stop();

	disconnect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
}

void DepthImpulsUstyeWidget::startDepthMeter()
{
	timer.start(1000);

	connect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
}

void DepthImpulsUstyeWidget::includeParameter(int state)
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

void DepthImpulsUstyeWidget::changeUnits(QString str)
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

void DepthImpulsUstyeWidget::connectDepthMeter(bool flag)
{
	if (!flag)
	{
		/*if (COM_Port->COM_port != NULL) 
		{
			COM_Port->COM_port->close();					
		}*/	
		
		tcp_settings->socket->disconnectFromHost();
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
		//if (COM_Port->COM_port != NULL)
		{
			/*QString COMPort_Name = COM_Port->COM_port->portName();
			if (COMPort_Name.isEmpty()) 
			{
				int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Depth Meter!"), QMessageBox::Ok, QMessageBox::Ok);
				return;
			}*/

			//COM_Port->COM_port->close();
			//COM_Port->COM_port->setPortName(COMPort_Name);
			tcp_settings->socket->disconnectFromHost();
			if (depth_communicator != NULL)
			{
				depth_communicator->exit();
				depth_communicator->wait();
				delete depth_communicator;
				depth_communicator = NULL;
			}	

			//bool res = COM_Port->COM_port->open(QextSerialPort::ReadWrite);
			QString key_value = "DepthMeter/IP_Address";
			QString dmeter_ip_addr = "";
			dmeter_ip_addr = settings->value(key_value).toString();
			bool _ok;
			key_value = "DepthMeter/Port";
			int dmeter_port_id = settings->value(key_value).toInt(&_ok);
			if (!_ok || dmeter_ip_addr.isEmpty())
			{
				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot find Depth Meter settings (IP and port)!"), QMessageBox::Ok, QMessageBox::Ok);
				return;
			}			
			
			tcp_settings->socket->connectToHost(dmeter_ip_addr, dmeter_port_id);
			bool res = tcp_settings->socket->isReadable();
			if (res) 	
			{				
				depth_communicator = new DepthCommunicator(tcp_settings->socket, clocker);
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

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open Depth Meter port (IP: %1, port: %2)!").arg(dmeter_ip_addr).arg(dmeter_port_id), QMessageBox::Ok, QMessageBox::Ok);
			}
		}
		//else
		//{
		//	int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Depth Meter!"), QMessageBox::Ok, QMessageBox::Ok);
		//}			
	}	
}

void DepthImpulsUstyeWidget::getMeasuredData(uint32_t _uid, uint8_t _type, double val)
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

void DepthImpulsUstyeWidget::measureTimedOut(uint32_t _uid, uint8_t _type)
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

void DepthImpulsUstyeWidget::showData(uint8_t type, double val)
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

void DepthImpulsUstyeWidget::onTime()
{	
	if (is_connected)
	{		
		if (depth_active) emit to_measure(++uid, DEPTH_DATA);
		if (rate_active) emit to_measure(++uid, RATE_DATA);
		if (tension_active) emit to_measure(++uid, TENSION_DATA);
	}	
}