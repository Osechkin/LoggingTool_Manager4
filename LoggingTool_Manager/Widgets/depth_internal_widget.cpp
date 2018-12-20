#include <QMessageBox>

#include "depth_internal_widget.h"


DepthInternalWidget::DepthInternalWidget(Clocker *_clocker, TCP_Settings *_socket, QWidget *parent /* = 0 */) : ui(new Ui::InternalDepthMeter)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	QFont tab_font_control = ui->tabWidget->tabBar()->font();
	tab_font_control.setBold(false);
	ui->tabWidget->tabBar()->setFont(tab_font_control);

	ui->ledDepth->setStyleSheet("QLineEdit { color: darkGreen }");

	ui->chboxDepth->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxRate->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxTension->setStyleSheet("QCheckBox { color: darkblue }");
	ui->chboxMLabels->setStyleSheet("QCheckBox { color: darkblue }");
	
	ui->cboxDepth->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxRate->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxTension->setStyleSheet("QComboBox { color: darkblue }");

	ui->label->setStyleSheet("QLabel { color: darkblue }");
	ui->label_2->setStyleSheet("QLabel { color: darkblue }");
	ui->label_3->setStyleSheet("QLabel { color: darkblue }");
	ui->label_4->setStyleSheet("QLabel { color: darkblue }");

	ui->gboxAlarm->setStyleSheet("QGroupBox { color: darkblue }");

	ui->chboxInversion->setStyleSheet("QCheckBox { color: darkblue }");

	ui->dsboxClockMult->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->sboxClocksAver->setStyleSheet("QSpinBox { color: darkGreen }");
	
	ui->ledMinDepth->setStyleSheet("QLineEdit { color: darkGreen }");
	ui->ledMaxDepth->setStyleSheet("QLineEdit { color: darkGreen }");

	ui->cboxMinDepth->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxMaxDepth->setStyleSheet("QComboBox { color: darkblue }");
	

	ui->pbtConnect->setIconSize(QSize(16,16));
	ui->pbtConnect->setText(tr("Connect to Depth Meter"));
	ui->pbtConnect->setIcon(QIcon(":/images/add.png"));

	QFont pbt_font = ui->pbtConnect->font();
	pbt_font.setBold(false);
	ui->pbtConnect->setFont(pbt_font);

	QFont font2 = ui->ledMinDepth->font();
	font2.setBold(true);
	ui->ledMinDepth->setFont(font2);
	ui->ledMaxDepth->setFont(font2);

	QFont font3 = ui->dsboxClockMult->font();
	font3.setBold(true);
	ui->dsboxClockMult->setFont(font3);
	ui->sboxClocksAver->setFont(font3);

	QFont font4 = ui->ledDepth->font();
	font4.setBold(true);
	ui->ledDepth->setFont(font4);

	depth_units_list << "m" << "ft";
	rate_units_list << "m/hr" << "m/s"<< "ft/s" << "ft/hr";
	tension_units_list << "kgf";	
	k_depth = 1;
	k_rate = 3600;
	k_tension = 1;

	ui->cboxDepth->addItems(depth_units_list);
	ui->cboxRate->addItems(rate_units_list);
	ui->cboxTension->addItems(tension_units_list);
	ui->cboxMinDepth->addItems(depth_units_list);
	ui->cboxMaxDepth->addItems(depth_units_list);

	ui->chboxDepth->setChecked(true);
	ui->chboxRate->setChecked(false);
	ui->chboxTension->setChecked(false);

	ui->cboxDepth->setEnabled(true);
	ui->cboxRate->setEnabled(true);
	ui->cboxTension->setEnabled(true);


	is_connected = false;
	device_is_searching = false;

	clocker = _clocker;
	//COM_Port = com_port;
	dmeter_socket = _socket;
	depth_communicator = NULL;

	timer.start(1000);	

	setConnection();
}

DepthInternalWidget::~DepthInternalWidget()
{
	delete ui;
}


void DepthInternalWidget::setConnection()
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

void DepthInternalWidget::includeParameter(int state)
{
	QCheckBox *chbox = (QCheckBox*)sender();
	if (!chbox) return;

	bool flag;
	if (state == Qt::Checked) flag = true;
	else if (state == Qt::Unchecked) flag = false;

	if (chbox == ui->chboxDepth) 
	{
		ui->ledDepth->setText("");
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
	else if (chbox == ui->chboxMLabels)
	{
		ui->lblMagnMarks->setText("");
		magnmarks_active = flag;
	}
}

void DepthInternalWidget::connectDepthMeter(bool flag)
{
	if (!flag)
	{
		if (dmeter_socket != NULL) 
		{
			dmeter_socket->socket->disconnectFromHost();	
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
		ui->ledDepth->setText("");
		ui->lblRate->setText("");
		ui->lblTension->setText("");
		ui->lblMagnMarks->setText("");
	}	
	else
	{		
		if (dmeter_socket != NULL)
		{
			/*QString COMPort_Name = COM_Port->COM_port->portName();
			if (COMPort_Name.isEmpty()) 
			{
				int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Depth Meter!"), QMessageBox::Ok, QMessageBox::Ok);
				return;
			}*/

			//COM_Port->COM_port->close();
			//COM_Port->COM_port->setPortName(COMPort_Name);
			dmeter_socket->socket->disconnectFromHost();

			if (depth_communicator != NULL)
			{
				depth_communicator->exit();
				depth_communicator->wait();
				delete depth_communicator;
				depth_communicator = NULL;
			}			
			//bool res = COM_Port->COM_port->open(QextSerialPort::ReadWrite);
			//if (res)
			dmeter_socket->socket->connectToHost(dmeter_socket->ip_address, dmeter_socket->port);
			if (dmeter_socket->socket->open(QIODevice::ReadWrite))			
			{				
				depth_communicator = new DepthCommunicator(dmeter_socket->socket, clocker);
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

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open Depth Meter socket (IP: %1, Port: %2)!").arg(dmeter_socket->ip_address).arg(dmeter_socket->port), QMessageBox::Ok, QMessageBox::Ok);
			}
		}
		else
		{
			int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available socket was found to connect to Depth Meter!"), QMessageBox::Ok, QMessageBox::Ok);
		}
		/*else
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
		}*/		
	}	
}

void DepthInternalWidget::setDepthCommunicatorConnections()
{
	connect(depth_communicator, SIGNAL(measured_data(uint32_t, uint8_t, double)), this, SLOT(getMeasuredData(uint32_t, uint8_t, double)));
	connect(depth_communicator, SIGNAL(data_timed_out(uint32_t, uint8_t)), this, SLOT(measureTimedOut(uint32_t, uint8_t)));
	connect(this, SIGNAL(to_measure(uint32_t, uint8_t)), depth_communicator, SLOT(toMeasure(uint32_t, uint8_t)));
}

void DepthInternalWidget::getMeasuredData(uint32_t _uid, uint8_t _type, double val)
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

void DepthInternalWidget::showData(uint8_t type, double val)
{	
	QString str = "<font color=darkGreen><b>%1</font>";

	switch (type)
	{
	case DEPTH_DATA:	
		{
			double value = val * k_depth;  
			QString str_value = QString::number(value);
			str = QString(str).arg(str_value);
			ui->ledDepth->setText(str); 
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
	case MAGN_LABELS:
		{
			break; // !!!!!!!!!
		}
	case DEVICE_SEARCH:	
		{
			ui->ledDepth->setText(""); 
			ui->lblRate->setText("");
			ui->lblTension->setText("");
			break;
		}
	default: break;
	}

	connectionWidget->setReport(ConnectionState::State_OK);
}
