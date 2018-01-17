#include <QIcon>
#include <QTimer>
#include <QMessageBox>

#include "tools_general.h"

#include "depth_template_wizard.h"


static int port_num = 0;
static uint32_t uid = 0;


DepthTemplateWizard::DepthTemplateWizard(QSettings *_settings, COM_PORT *com_port, COM_PORT *com_port_stepmotor, QStringList depth_meter_list, Clocker *clocker, QWidget *parent) : QWidget(parent), ui(new Ui::DepthTemplateWizard)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
			
	app_settings = _settings;

	connectionWidget = new ImpulsConnectionWidget(this);
	connectionWidget->setReport(ConnectionState::State_No);
	
	this->clocker = clocker;
	COM_Port = com_port;
	COM_Port_stepmotor = com_port_stepmotor;
	
	is_connected = false;
	//device_is_searching = false;
		
	QStringList depth_meters_str; 
	if (depth_meter_list.contains("DepthEmulator"))
	{
		DepthEmulatorWidget *depth_emulator = new DepthEmulatorWidget(clocker);
		depth_meters_str << depth_emulator->getTitle();
		depth_meters << depth_emulator;
	}
	if (depth_meter_list.contains("Impulse-Ustye"))
	{
		DepthImpulsUstyeWidget *depth_impulsustye = new DepthImpulsUstyeWidget(clocker, COM_Port);
		depth_meters_str << depth_impulsustye->getTitle();
		depth_meters << depth_impulsustye;
	}
	if (depth_meter_list.contains("InternalDepthMeter"))
	{
		DepthInternalWidget *depth_internal = new DepthInternalWidget(clocker, COM_Port);
		depth_meters_str << depth_internal->getTitle();
		depth_meters << depth_internal;
	}
	if (depth_meter_list.contains("LeuzePositionMeter"))
	{
		LeuzeDistanceMeterWidget *distance_meter = new LeuzeDistanceMeterWidget(app_settings, clocker, COM_Port, COM_Port_stepmotor);
		connect(distance_meter, SIGNAL(new_core_diameter(double)), this, SIGNAL(new_core_diameter(double)));
		//distance_meter->setVisible(false);
		depth_meters_str << distance_meter->getTitle();
		depth_meters << distance_meter;
	}

	if (depth_meters.isEmpty())
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Available Depth/Position Meters hasn't been found!"), QMessageBox::Ok);	
		exit(0);
	}

	ui->gridLayoutFrame->addWidget(depth_meters.first());
	ui->cboxDepthMeter->addItems(depth_meters_str);
	current_depth_meter = depth_meters.first();	
	
	/*
	DepthEmulatorWidget *depth_emulator = new DepthEmulatorWidget(clocker);
	DepthImpulsUstyeWidget *depth_impulsustye = new DepthImpulsUstyeWidget(clocker, COM_Port);
	DepthInternalWidget *depth_internal = new DepthInternalWidget(clocker, COM_Port);
	LeuzeDistanceMeterWidget *distance_meter = new LeuzeDistanceMeterWidget(clocker, COM_Port, COM_Port_stepmotor);
	connect(depth_impulsustye, SIGNAL(connected(bool)), this, SIGNAL(connected(bool)));

	ui->gridLayoutFrame->addWidget(depth_emulator);

	QStringList depth_meters_str; 
	depth_meters_str << depth_emulator->getTitle() << depth_impulsustye->getTitle() << depth_internal->getTitle() << distance_meter->getTitle();
	ui->cboxDepthMeter->addItems(depth_meters_str);
	
	depth_meters << depth_emulator << depth_impulsustye << depth_internal << distance_meter;
	current_depth_meter = depth_emulator;
	*/
	
	setConnection();	
}

DepthTemplateWizard::~DepthTemplateWizard()
{
	delete ui;	
	qDeleteAll(depth_meters.begin(), depth_meters.end());
	
	/*if (depth_communicator != NULL)
	{
		depth_communicator->exit();
		depth_communicator->wait();
		delete depth_communicator;	
	}*/
		
	//delete COM_Port->COM_port;
	//delete COM_Port;
}

void DepthTemplateWizard::setConnection()
{
	connect(ui->cboxDepthMeter, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeDepthMeter(QString)));
	connect(&timer, SIGNAL(timeout()), this, SLOT(onTime()));	
}

/*
void DepthTemplateWizard::setDepthCommunicatorConnections()
{
	connect(depth_communicator, SIGNAL(measured_data(uint32_t, uint8_t, double)), this, SLOT(getMeasuredData(uint32_t, uint8_t, double)));
	connect(depth_communicator, SIGNAL(data_timed_out(uint32_t, uint8_t)), this, SLOT(measureTimedOut(uint32_t, uint8_t)));
	connect(this, SIGNAL(to_measure(uint32_t, uint8_t)), depth_communicator, SLOT(toMeasure(uint32_t, uint8_t)));
}
*/

/*
void DepthTemplateWizard::includeParameter(int state)
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
*/

/*
void DepthTemplateWizard::changeDepthMeter(QString str)
{
	current_depth_meter->stopDepthMeter();

	for (int i = 0; i < depth_meters.count(); i++)
	{
		if (depth_meters[i]->getTitle() == str)
		{		
			AbstractDepthMeter *selected_depth_meter = depth_meters[i];
						
			bool removed = false;	
			AbstractDepthMeter::DepthMeterType removed_type = AbstractDepthMeter::DepthMeterType::NoType;
			for (int j = 0; j < depth_meters.count(); j++)
			{				
				if (current_depth_meter == depth_meters[j])
				{
					depth_meters.removeAt(j);
					removed_type = current_depth_meter->getType();
					removed = true;
				}
			}
			if (!removed) return;
												
			ui->gridLayoutFrame->removeWidget(current_depth_meter);
			delete current_depth_meter;

			switch (removed_type)
			{
			case AbstractDepthMeter::DepthEmulator:			depth_meters.append(new DepthEmulatorWidget(clocker)); break;
			case AbstractDepthMeter::ImpulsUstye:			depth_meters.append(new DepthImpulsUstyeWidget(clocker, COM_Port)); break;
			case AbstractDepthMeter::InternalDepthMeter:	depth_meters.append(new DepthInternalWidget(clocker, COM_Port)); break;
			default: break;
			}
			
			current_depth_meter = selected_depth_meter;
			current_depth_meter->startDepthMeter();
			ui->gridLayoutFrame->addWidget(current_depth_meter);			
		}
	}
}
*/

void DepthTemplateWizard::changeDepthMeter(QString str)
{
	current_depth_meter->stopDepthMeter();

	AbstractDepthMeter *selected_depth_meter = NULL;
	for (int i = 0; i < depth_meters.count(); i++)
	{
		if (depth_meters[i]->getTitle() == str)
		{		
			selected_depth_meter = depth_meters[i];					
		}
	}
	if (selected_depth_meter == NULL) return;

	bool removed = false;	
	AbstractDepthMeter::DepthMeterType removed_type = AbstractDepthMeter::DepthMeterType::NoType;
	for (int j = depth_meters.count()-1; j >= 0; --j)
	{				
		if (current_depth_meter == depth_meters[j])
		{
			depth_meters.removeAt(j);
			removed_type = current_depth_meter->getType();
			removed = true;
			break;
		}
	}
	if (!removed) return;

	ui->gridLayoutFrame->removeWidget(current_depth_meter);
	delete current_depth_meter;

	switch (removed_type)
	{
	case AbstractDepthMeter::DepthEmulator:			depth_meters.append(new DepthEmulatorWidget(clocker)); break;
	case AbstractDepthMeter::ImpulsUstye:			depth_meters.append(new DepthImpulsUstyeWidget(clocker, COM_Port)); break;
	case AbstractDepthMeter::InternalDepthMeter:	depth_meters.append(new DepthInternalWidget(clocker, COM_Port)); break;
	case AbstractDepthMeter::LeuzeDistanceMeter:	depth_meters.append(new LeuzeDistanceMeterWidget(app_settings, clocker, COM_Port, COM_Port_stepmotor)); break;
	default: break;
	}

	current_depth_meter = selected_depth_meter;
	current_depth_meter->startDepthMeter();
	ui->gridLayoutFrame->addWidget(current_depth_meter);	
}


bool DepthTemplateWizard::isAvailableCOMPort(COM_PORT *_com_port, int port_num)
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


QStringList DepthTemplateWizard::availableCOMPorts()
{
	//ui->cboxPort->clear();
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

void DepthTemplateWizard::initCOMSettings(COM_PORT *com_port)
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

/*
void DepthTemplateWizard::getMeasuredData(uint32_t _uid, uint8_t _type, double val)
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
*/

/*
void DepthTemplateWizard::measureTimedOut(uint32_t _uid, uint8_t _type)
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
*/

void DepthTemplateWizard::onTime()
{	
	if (is_connected)
	{		
		if (depth_active) emit to_measure(++uid, DEPTH_DATA);
		if (rate_active) emit to_measure(++uid, RATE_DATA);
		if (tension_active) emit to_measure(++uid, TENSION_DATA);
	}	
}


void DepthTemplateWizard::showData(uint8_t type, double val)
{	
	QString str = "<font color=darkGreen><b>%1</font>";
	
	/*
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
	*/
	
	connectionWidget->setReport(ConnectionState::State_OK);	// Проверить ! Похоже, что именно это рисует "Conn..." на внешнем виде виджета дальномера
}


/*
void DepthTemplateWizard::changeUnits(QString str)
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
*/

/*
void DepthTemplateWizard::changeCOMPort(QString str)
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
*/

/*
void DepthTemplateWizard::changeLocation(Qt::DockWidgetArea area)
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
*/

/*
void DepthTemplateWizard::connectDepthMeter(bool flag)
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

		
		//ui->pbtConnect->setText(tr("Connect to Depth Meter"));
		//ui->pbtConnect->setIcon(QIcon(":/images/add.png"));
		//ui->lblDepth->setText("");
		//ui->lblRate->setText("");
		//ui->lblTension->setText("");
		
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
								
				//ui->pbtConnect->setText(tr("Disconnect from Depth Meter"));
				//ui->pbtConnect->setIcon(QIcon(":/images/remove.png"));				
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
*/