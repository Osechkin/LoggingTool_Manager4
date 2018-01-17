#include <QDateTime>
#include <QToolBar>
#include <QDockWidget>
#include <QStyle>
#include <QStyleFactory>
#include <QSizePolicy>

#include "nmrtool_connect_wizard.h"

#include "../Communication/message_processor.h"
#include "Common/app_settings.h"


// **************  NMR Tool Linker ...  ******************
NMRToolLinker::NMRToolLinker(COM_PORT *_com_port, QSettings *settings, QWidget *parent) : QWidget(parent), ui(new Ui::NMRToolLinker)
{
	ui->setupUi(this);	
	setParent(parent);

	app_settings = settings;

	ui->tedReport->setLineWrapMode(QTextEdit::WidgetWidth); //te->setWordWrapMode(QTextOption::WordWrap);
	QPalette p = ui->tedReport->palette();
	p.setColor(QPalette::Base, QColor(Qt::white));
	ui->tedReport->setPalette(p);	

	ui->tbtClearAll->setIcon(QIcon(":/images/Eraser.png"));
	ui->tbtLogOnOff->setIcon(QIcon(":/images/Red Ball.png"));
		
	setLogVizState(true);
	ui->tbtLogOnOff->setCheckable(true);
	ui->tbtLogOnOff->setChecked(true);

	this->com_port = _com_port;
	if (com_port->connect_state == true) connection_state = ConnectionState::State_OK;
	else connection_state = ConnectionState::State_No;
	refreshCOMPortSettings();	
	
	connWidget = new ConnectionWidget(this);
	connWidget->setReport(connection_state);	
	trafficWidget = new TrafficWidget(this);
	trafficWidget->setReport(-1, MsgState::MsgOff);
		
	setConnections();
}

NMRToolLinker::~NMRToolLinker()
{
	qDeleteAll(msg_container);

	delete ui;
}

/*void NMRToolLinker::applyCOMPortSettings()
{
	QString port_name = ui->cboxPort->currentText();
	QString baud_rate = ui->cboxRate->currentText();
	QString bits = ui->cboxBits->currentText();
	QString stop_bits = ui->cboxStopBits->currentText();
	QString parity = ui->cboxParity->currentText();

	setCOMPort(port_name);
	setBaudrate(baud_rate);
	setDataBits(bits);
	setStopBits(stop_bits);
	setParity(parity);
}*/

void NMRToolLinker::refreshCOMPortSettings()
{
	port_names.clear();
	for (int i = 1; i <= 15; i++)
	{
		QString name = QString("COM%1").arg(i);
		QextSerialPort *port = new QextSerialPort(name);
		bool res = port->open(QextSerialPort::ReadWrite);

		if (res)
		{
			port_names.append(name);
			port->close();
		}
		delete port;
	}
	if (port_names.isEmpty()) return;

	int index = port_names.indexOf(com_port->COM_port->portName());
	if (index >= 0)
	{
		port_names.takeAt(index);
		port_names.push_front(com_port->COM_port->portName());
	}	
}

void NMRToolLinker::setConnections()
{
	connect(ui->tbtClearAll, SIGNAL(clicked()), ui->tedReport, SLOT(clear()));
	connect(ui->tbtLogOnOff, SIGNAL(toggled(bool)), this, SLOT(setLogVizState(bool)));
}


void NMRToolLinker::saveCOMPortSettings()
{
	if (app_settings->contains("COMPort/PortName"))
	{
		app_settings->setValue("COMPort/PortName", com_port->COM_port->portName());
		app_settings->setValue("COMPort/BaudRate", com_port->COM_Settings.BaudRate);
		app_settings->setValue("COMPort/DataBits", com_port->COM_Settings.DataBits);
		app_settings->setValue("COMPort/Parity", toString(com_port->COM_Settings.Parity));
		app_settings->setValue("COMPort/StopBits", toString(com_port->COM_Settings.StopBits));
		app_settings->setValue("COMPort/FlowControl", toString(com_port->COM_Settings.FlowControl));
		app_settings->setValue("COMPort/Timeout_ms", com_port->COM_Settings.Timeout_Millisec);
		app_settings->sync();
	}	
}


void NMRToolLinker::showCmdResult(uint32_t _uid, QString obj_name, QVariantList data)
{
	if (this->objectName() != obj_name) return;

	for (int i = msg_container.count()-1; i >= 0; --i)
	{
		DeviceData *dev_data = msg_container.at(i);
		if (dev_data->uid == _uid) 
		{				
			uint8_t comm_id = dev_data->comm_id;
			switch (comm_id)
			{
			case NMRTOOL_CONNECT:
			case NMRTOOL_CONNECT_DEF:
				{
					if (data.count() > 0)
					{
						bool res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();
						if (res) 
						{
							com_port->connect_state = true;
							report = QString("<font color=darkGreen>") + ctime_str + tr(" Logging Tool is connected !") +  "</font>";
							addText(report);
							connection_state = ConnectionState::State_OK;
							connWidget->stopBlinking();
							connWidget->setReport(connection_state);

							if (comm_id == NMRTOOL_CONNECT) default_comm_settings_on = false;
							else if (comm_id == NMRTOOL_CONNECT_DEF) default_comm_settings_on = true;

							emit place_to_statusbar(tr(" Logging Tool is connected !"));
							emit tool_settings_applied(true);
							emit default_comm_settings(default_comm_settings_on);

							saveCOMPortSettings();
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot find Logging Tool !") + "</font>";
							addText(report);
							//stopConnection();
							//connection_state = ConnectionState::State_Connecting;
							com_port->COM_port->close();
							com_port->connect_state = false;
							addText(QString("<font color=red>") + ctime_str + QString(tr("COM-Port is closed ")) + QString("[%1] !</font>").arg(com_port->COM_port->portName()));
							
							emit place_to_statusbar(tr(" Cannot find Logging Tool !"));								
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();
					if (connection_state != ConnectionState::State_OK && com_port->auto_search) searchForNMRTool();	
					else emit cmd_resulted(NMRTOOL_CONNECT, connection_state);
					break;
				}
			case NMRTOOL_START:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();						
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" NMR Tool is started !") +  "</font>";
							addText(report);
							connWidget->stopBlinking();
							connection_state = ConnectionState::State_OK;
							emit place_to_statusbar(tr(" NMR Tool is started !"));
							emit control_nmrtool(true);
							////emit start_experiment(true);
							//emit cmd_resulted(true, _uid);
							emit fpga_seq_started(true);
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot start NMR Tool !") + "</font>";
							addText(report);	
							connection_state = ConnectionState::State_No;
							emit place_to_statusbar(tr(" Cannot start NMR Tool !"));
							//emit cmd_resulted(false, _uid);
							emit fpga_seq_started(false);
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(NMRTOOL_START, connection_state);
					break;
				}
			case SET_WIN_PARAMS:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();						
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" Window Function Parameters have been applied !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();									
							emit place_to_statusbar(tr(" Window Function Parameters have been applied !"));							
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot apply Window Function Parameters !") + "</font>";
							addText(report);								
							emit place_to_statusbar(tr(" Cannot apply Window Function Parameters !"));					
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(SET_WIN_PARAMS, connection_state);
					break;
				}
			case SET_COMM_PARAMS:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();						
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" Communication Parameters have been applied !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();									
							emit place_to_statusbar(tr(" Communication Parameters have been applied !"));							
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot apply Communication Parameters !") + "</font>";
							addText(report);								
							emit place_to_statusbar(tr(" Cannot apply Communication Parameters !"));					
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(SET_COMM_PARAMS, connection_state);
					break;
				}
			case NMRTOOL_STOP:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" NMR Tool has been stopped !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();	
							connection_state = ConnectionState::State_OK;
							emit place_to_statusbar(tr(" NMR Tool has been stopped !"));
							emit control_nmrtool(false);
							emit start_experiment(false);		
							//emit cmd_resulted(true, 0);
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot stop NMR Tool !") + "</font>";
							addText(report);	
							connection_state = ConnectionState::State_No;
							emit place_to_statusbar(tr(" Cannot stop NMR Tool !"));					
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(NMRTOOL_STOP, connection_state);
					break;
				}
			case DATA_PROC:
				{
					bool res = false;
					ConnectionState conn_state = State_Connecting;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" The program for FPGA has been started !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();		
							conn_state = State_OK;
							emit control_nmrtool(true);
							emit place_to_statusbar(tr(" The program for FPGA has been started !"));
							////emit start_experiment(true);							
							emit fpga_seq_started(true);
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot send the Program for FPGA to NMR Tool !") + "</font>";
							addText(report);	
							conn_state = State_No;
							emit place_to_statusbar(tr(" Cannot send the Program for FPGA to NMR Tool !"));
							emit fpga_seq_started(false);
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(DATA_PROC, conn_state);
					break;
				}
			case SDSP_DATA:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();						
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" Parameters for SDSP tool have been applied !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();									
							emit place_to_statusbar(tr(" Parameters for SDSP tool have been applied !"));							
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot send SDSP Parameters !") + "</font>";
							addText(report);								
							emit place_to_statusbar(tr(" Cannot send SDSP Parameters !"));					
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(SDSP_DATA, connection_state);
					break;
				}
			case LOG_TOOL_SETTINGS:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();						
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" Logging tool settings have been applied !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();									
							emit place_to_statusbar(tr(" Logging tool settings have been applied !"));	
							emit tool_settings_applied(true);
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot send Logging tool settings !") + "</font>";
							addText(report);								
							emit place_to_statusbar(tr(" Cannot send Logging tool settings !"));					
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(LOG_TOOL_SETTINGS, connection_state);
					break;
				}
			case DIEL_DATA_READY:
				{
					bool res = false;
					if (data.count() > 0)
					{
						res = data[0].toBool();
						QString report = "";				
						QString ctime_str = getCurrentTime();						
						if (res) 
						{
							report = QString("<font color=darkGreen>") + ctime_str + tr(" Data from SDSP tool have been received !") +  "</font>";
							addText(report);							
							connWidget->stopBlinking();									
							emit place_to_statusbar(tr(" Data from SDSP tool have been received !"));							
						}
						else 
						{
							report = QString("<font color=red>") + ctime_str + tr(" Cannot get SDSP data !") + "</font>";
							addText(report);								
							emit place_to_statusbar(tr(" Cannot get SDSP data !"));					
						}					
					}
					dev_data = msg_container.takeAt(i);
					delete dev_data;

					QApplication::restoreOverrideCursor();					
					emit cmd_resulted(DT_DIEL_ADJUST, connection_state);
					break;
				}
			case NMRTOOL_DATA:
				{
					//emit fpga_seq_started(true);
					break;
				}
			default: break;
			}			
		}		
	}		
}


void NMRToolLinker::setCOMPort(QString str)
{
	com_port->COM_port->setPortName(str);
}

void NMRToolLinker::setBaudrate(QString str)
{
	bool ok;
	int val = str.toInt(&ok);
	if (!ok) return;

	com_port->COM_Settings.BaudRate = (BaudRateType)val;
}

void NMRToolLinker::setDataBits(QString str)
{
	bool ok;
	int val = str.toInt(&ok);
	if (!ok) return;

	com_port->COM_Settings.DataBits = (DataBitsType)val;
}

void NMRToolLinker::setStopBits(QString str)
{
	if (str == "1") com_port->COM_Settings.StopBits = STOP_1;
	else if (str == "1.5") com_port->COM_Settings.StopBits = STOP_1_5;
	else if (str == "2") com_port->COM_Settings.StopBits = STOP_2;
}

void NMRToolLinker::setParity(QString str)
{
	if (str == "No Parity") com_port->COM_Settings.Parity = PAR_NONE;
	else if (str == "Odd Parity") com_port->COM_Settings.Parity = PAR_ODD;
	else if (str == "Even Parity") com_port->COM_Settings.Parity = PAR_EVEN;
	else if (str == "Space Parity") com_port->COM_Settings.Parity = PAR_SPACE;
	else if (str == "Mark Parity") com_port->COM_Settings.Parity = PAR_MARK;
}

void NMRToolLinker::addText(QString text)
{
	if (log_viz_state) ui->tedReport->append(text);	
}

void NMRToolLinker::applyProcPrg(QVector<uint8_t> &proc_prg, QVector<uint8_t> &proc_instr)
{	
	QString ctime_srt = getCurrentTime();
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Send new program for FPGA to NMR Tool... ")));

	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(DATA_PROC, "Send new program for FPGA to NMR Tool...", id);
	
	QVector<double> *prg_data = new QVector<double>(proc_prg.count());
	for (int i = 0; i < prg_data->size(); i++)
	{
		prg_data->data()[i] = (double)proc_prg[i];
	}
	device_data->fields->at(0)->code = DATA_PROC;
	device_data->fields->at(0)->value = prg_data;
	//msg_container.append(device_data);

	QVector<double> *instr_data = new QVector<double>(proc_instr.count());
	for (int i = 0; i < instr_data->size(); i++)
	{
		instr_data->data()[i] = (double)proc_instr[i];
	}
	device_data->fields->at(1)->code = FPGA_PRG;
	device_data->fields->at(1)->value = instr_data;
	msg_container.append(device_data);

	emit send_msg(device_data, this->objectName()); // Send new program for FPGA to NMR Tool (FPGA program)	
	emit place_to_statusbar(tr("Send new program for FPGA to NMR Tool..."));
}

void NMRToolLinker::applyMsgCommParams(QVector<uint8_t> params)
{
	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_srt = QString("&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ");
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Send Communication Settings to NMR Tool... ")));
		
	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(SET_COMM_PARAMS, "Send Communication Settings to NMR Tool...", id);
	
	QVector<double> *params_data = new QVector<double>(params.count());
	for (int i = 0; i < params_data->size(); i++)
	{
		params_data->data()[i] = (double)params[i];
	}
	device_data->fields->at(0)->code = SET_COMM_PARAMS;
	device_data->fields->at(0)->value = params_data;	
	msg_container.append(device_data);
	
	emit send_msg(device_data, this->objectName()); 
	emit place_to_statusbar(tr("Send Communication Settings to NMR Tool..."));
}

void NMRToolLinker::applyWinFuncParams(QVector<int> params)
{
	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_srt = QString("&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ");
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Applying Window Function Parameters to NMR Tool... ")));

	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(SET_WIN_PARAMS, "Applying Window Function Parameters to NMR Tool...", id);

	QVector<double> *params_data = new QVector<double>(params.count());
	for (int i = 0; i < params_data->size(); i++)
	{
		params_data->data()[i] = (double)params[i];
	}
	device_data->fields->at(0)->code = SET_WIN_PARAMS;
	device_data->fields->at(0)->value = params_data;	
	msg_container.append(device_data);

	emit send_msg(device_data, this->objectName()); 
	emit place_to_statusbar(tr("Applying Window Function Parameters to NMR Tool..."));
}

void NMRToolLinker::sendDataToSDSP(QVector<int> &params)
{
	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_srt = QString("&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ");
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Send SDSP data to Logging Tool... ")));
		
	uint32_t id = COM_Message::generateMsgId();
	
	DeviceData *device_data = new DeviceData(SDSP_DATA, "Send SDSP parameters to Logging Tool...", id);

	int size = params.size();
	QVector<double> *params_data = new QVector<double>(size);
	for (int i = 0; i < params_data->size(); i++)
	{
		params_data->data()[i] = (double)(uint8_t(params[i]));
	}
	device_data->fields->at(0)->code = SDSP_DATA;
	device_data->fields->at(0)->value = params_data;	
	msg_container.append(device_data);

	emit send_msg(device_data, this->objectName()); 
	emit place_to_statusbar(tr("Send SDSP settings to Logging Tool..."));		
}

void NMRToolLinker::sendDataToSDSP(QByteArray& arr)
{
	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_srt = QString("&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ");
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Send SDSP data to Logging Tool... ")));
		
	uint32_t id = COM_Message::generateMsgId();
	/*if (arr.size() == 1) 
	{
		if (uint8_t(arr.at(0)) == 0xC8)
		{
			DeviceData *device_data = new DeviceData(SDSP_REQUEST_C8, "Send Request 0xC8 to SDSP...", id);
						
			device_data->comm_id = SDSP_REQUEST_C8;				
			msg_container.append(device_data);

			emit send_msg(device_data, this->objectName()); 
			emit place_to_statusbar(tr("Send Request 0xC8 to SDSP..."));	
		}
		else if (uint8_t(arr.at(0)) == 0x88)
		{
			DeviceData *device_data = new DeviceData(SDSP_REQUEST_88, "Send Request 0x88 to SDSP...", id);

			device_data->comm_id = SDSP_REQUEST_88;				
			msg_container.append(device_data);

			emit send_msg(device_data, this->objectName()); 
			emit place_to_statusbar(tr("Send Request 0x88 to SDSP..."));	
		}
	}
	else */
	{
		DeviceData *device_data = new DeviceData(SDSP_DATA, "Send SDSP parameters to Logging Tool...", id);

		int size = arr.size();
		QVector<double> *params_data = new QVector<double>(size);
		for (int i = 0; i < params_data->size(); i++)
		{
			params_data->data()[i] = (double)(uint8_t(arr[i]));
		}
		device_data->fields->at(0)->code = SDSP_DATA;
		device_data->fields->at(0)->value = params_data;	
		msg_container.append(device_data);

		emit send_msg(device_data, this->objectName()); 
		emit place_to_statusbar(tr("Send SDSP parameters to Logging Tool..."));	
	}		
}

void NMRToolLinker::sendToolSettings(QVector<int> params)
{
	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_srt = QString("&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ");
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Send tool settings to Logging Tool... ")));

	uint32_t id = COM_Message::generateMsgId();

	DeviceData *device_data = new DeviceData(LOG_TOOL_SETTINGS, "Send tool settings to Logging Tool...", id);

	int size = params.size();
	QVector<double> *params_data = new QVector<double>(size);
	for (int i = 0; i < params_data->size(); i++)
	{
		params_data->data()[i] = (double)params[i];
	}
	device_data->fields->at(0)->code = LOG_TOOL_SETTINGS;
	device_data->fields->at(0)->value = params_data;	
	msg_container.append(device_data);

	emit send_msg(device_data, this->objectName()); 
	emit place_to_statusbar(tr("Send tool settings to Logging Tool..."));	
}


void NMRToolLinker::startNMRTool()
{
	QString ctime_srt = getCurrentTime();
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Starting Logging Tool... ")));

	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(NMRTOOL_START, "Starting Logging Tool...", id);
	msg_container.append(device_data);

	emit send_msg(device_data, this->objectName()); // Start NMR Tool (FPGA program)	
	emit place_to_statusbar(tr("Starting Logging Tool..."));
}

void NMRToolLinker::stopNMRTool()
{
	QString ctime_srt = getCurrentTime();
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Stopping Logging Tool... ")));

	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(NMRTOOL_STOP, "Stopping Logging Tool...", id);
	msg_container.append(device_data);

	emit send_msg(device_data, this->objectName()); // Stop NMR Tool (FPGA program)	
	emit place_to_statusbar(tr("Stopping Logging Tool..."));
}

void NMRToolLinker::startConnection(bool flag)
{	
	if (flag) 
	{
		QString port_name = com_port->COM_port->portName();
		int index = port_names.indexOf(port_name);
		if (index >= 0) port_names.takeAt(index);
		port_names.push_front(port_name);

		searchForNMRTool();		
	}
	else stopConnection();
}

void NMRToolLinker::searchForNMRTool()
{
	static int index = 0;		// COM Port device counter	
	if (com_port->auto_search) 
	{
		if (index >= port_names.count()) 
		{		
			stopConnection();
			index = 0;
			return; 
		}
	}
	else
	{
		if (index > 0)
		{
			stopConnection();
			index = 0;
			return; 
		}
	}

	switch (connection_state)
	{
	case ConnectionState::State_OK: { index = 0; return; }	
	case ConnectionState::State_No: { index = 0; break; }
	default: break;
	}

	QString port_name = port_names[index++];
	com_port->COM_port->setPortName(port_name);

	QApplication::setOverrideCursor(Qt::BusyCursor);

	connection_state = ConnectionState::State_Connecting;	
	uint8_t cmd_code = NMRTOOL_CONNECT;
	if (default_comm_settings_on) cmd_code = NMRTOOL_CONNECT_DEF;
	emit cmd_resulted(cmd_code, connection_state);

	connWidget->startBlinking();
	bool res = openCOMPort();
	if (!res)
	{				
		index = 0;	
		stopConnection();		
	}
	else findNMRTool();	

	if (!com_port->auto_search) index = 0;
}

bool NMRToolLinker::openCOMPort()
{		
	QString ctime_str = getCurrentTime();

	com_port->COM_port->close();

	com_port->COM_port->setPortName(com_port->COM_port->portName());
	com_port->COM_port->setBaudRate(com_port->COM_Settings.BaudRate);
	com_port->COM_port->setDataBits(com_port->COM_Settings.DataBits);
	com_port->COM_port->setParity(com_port->COM_Settings.Parity);
	com_port->COM_port->setStopBits(com_port->COM_Settings.StopBits);
	com_port->COM_port->setFlowControl(com_port->COM_Settings.FlowControl);
	com_port->COM_port->setTimeout(com_port->COM_Settings.Timeout_Millisec);

	bool res = false;
	res = com_port->COM_port->open(QextSerialPort::ReadWrite);
	if (!res)
	{
		com_port->connect_state = false;			
		addText(QString("<font color=red>") + ctime_str + QString(tr("Cannot open COM-Port ")) + QString("[%1] !</font>").arg(com_port->COM_port->portName()));				
	}
	else
	{
		com_port->connect_state = true;		
		addText(QString("<font color=darkGreen>") + ctime_str + QString(tr("COM-Port is opened ")) + QString("[%1] !</font>").arg(com_port->COM_port->portName()));		
	}

	return res;
}

void NMRToolLinker::stopConnection()
{
	QString ctime_str = getCurrentTime();

	com_port->COM_port->close();
	com_port->connect_state = false;
	//if (connection_state != ConnectionState::State_Connecting) 
	{
		addText(QString("<font color=red>") + ctime_str + QString(tr("COM-Port is closed ")) + QString("[%1] !</font>").arg(com_port->COM_port->portName()));
	}

	connection_state = ConnectionState::State_No;
	connWidget->stopBlinking();
	connWidget->setReport(connection_state);	
	
	QApplication::restoreOverrideCursor();

	emit cmd_resulted(NMRTOOL_CONNECT, connection_state);
}

void NMRToolLinker::findNMRTool()
{
	QString ctime_srt = getCurrentTime();
	addText(QString("<font color=darkBlue>%1").arg(ctime_srt) + QString(tr("Connect to Logging Tool ")) + QString("[%1] ...</font>").arg(com_port->COM_port->portName()));

	uint32_t id = COM_Message::generateMsgId();
	if (default_comm_settings_on)
	{
		DeviceData *device_data = new DeviceData(NMRTOOL_CONNECT_DEF, "Connect to Logging Tool with default Communication Settings", id);
		msg_container.append(device_data);

		emit send_msg(device_data, this->objectName()); // Find NMR Tool and Connect
	}
	else
	{
		DeviceData *device_data = new DeviceData(NMRTOOL_CONNECT, "Connect to Logging Tool", id);
		msg_container.append(device_data);

		emit send_msg(device_data, this->objectName()); // Find NMR Tool and Connect
	}
		
	emit place_to_statusbar(tr("Connecting to Logging Tool..."));
}

QString NMRToolLinker::getCurrentTime()
{
	QDateTime ctime = QDateTime::currentDateTime();
	
	return QString("&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ");
}

void NMRToolLinker::showMsgTrafficReport(int num, int total)
{
	if (total <= 0) return;

	int err_rate = (int)(100.0*num/total);
	
	if (num == 0) trafficWidget->setReport(err_rate, MsgState::MsgOK);
	else if (num == total) trafficWidget->setReport(err_rate, MsgState::MsgBad);
	else if (num < total) trafficWidget->setReport(err_rate, MsgState::MsgPoor);
}

void NMRToolLinker::setLogVizState(bool flag)
{
	log_viz_state = flag;	
	if (!log_viz_state) ui->tbtLogOnOff->setIcon(QIcon(":/images/Red Ball.png"));
	else ui->tbtLogOnOff->setIcon(QIcon(":/images/green_ball.png"));
}
