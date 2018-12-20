#include <QDockWidget>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QTimer>
#include <QPen>
#include <QBrush>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QtMultimedia/QSound>

#include "main_window.h"

#include "qwt_symbol.h"
#include "qwt_plot.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"

//#include "Dialogs/comport_dialog.h"
#include "Dialogs/communication_dialog.h"
#include "Dialogs/axis_dialog.h"
//#include "Dialogs/tcp_server_dialog.h"
//#include "Dialogs/cdiag_server_dialog.h"
#include "Dialogs/about_dialog.h"
#include "Dialogs/processing_settings_dialog.h"
#include "Dialogs/export_settings_dialog.h"
#include "Dialogs/experiment_settings_dialog.h"
#include "Dialogs/export_toFile_dialog.h"
//#include "Dialogs/depth_server_dialog.h"
#include "Dialogs/tools_dialog.h"
#include "Dialogs/tool_settings_dialog.h"

#include "Wizards/depth_template_wizard.h"

#include "qextserialport.h"
#include "Galois/gf_data.h"
#include "GraphClasses/graph_classes.h"
#include "Communication/message_class.h"

#include "Common/data_processing.h"
#include "Common/app_settings.h"
#include "DMDeconv/DMDeconv.h"

#include "Common/profiler.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{		
	/*bool ok;
	RING_BUFFER ring_buff(8);
	uint8_t arr[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	ring_buff.put(0, &ok);
	ring_buff.put_bytes(&arr[0], 5, &ok);
	ring_buff.put_bytes(&arr[0], 5, &ok);	
	ring_buff.get_all(&arr[0], &ok);
	ring_buff.put(6, &ok);
	ring_buff.put(7, &ok);
	ring_buff.put(8, &ok);
	memset(&arr[0], 0x0, 8);
	ring_buff.get_bytes(&arr[0], 5, &ok);*/
	
	clocker = new Clocker();
	clocker->setPeriod(CLOCK_PERIOD);
	clocker->start(QThread::IdlePriority);

	loadToolsSettings();
	current_tool_settings = NULL;	
	current_tool_was_applied = false;
	if (tools_settings.isEmpty())
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot find tool settings file in /Tools folder!"), QMessageBox::Ok, QMessageBox::Ok);
		exit(0);
	}
		
	loadAppSettings();	

	gf_data = (GF_Data*)malloc(sizeof(GF_Data));
	gfdata_init(gf_data,127);	// 127 -  максимально возможное число исправляемых ошибок для поля GP(256)
	gf_data->index = 1;			// номер порождающего полинома, который равен максимальному количеству исправляемых ошибок плюс 1  
	gf_data->index_hdr = 1;
	gf_data->index_body = 1;
	gf_data->index_ftr = 0;

	loadCommSettings();	
		
	initNMRToolPortSettings();
	initDepthMeterPortSettings();
	initStepMotorPortSettings();

	//COM_Port = new COM_PORT;
	//initCOMSettings(COM_Port);
	//bool conn_res = setupCOMPort(COM_Port, QString(port_name));
	//if (!conn_res) conn_res = findAvailableCOMPort(COM_Port);
	//if (conn_res)
	{
		com_msg_mutex = new QMutex(QMutex::Recursive);
		com_commander = new COMCommander(this);
		com_commander->setNMRToolState(false);					// NMR Tool has not started yet.
		com_commander->start(QThread::NormalPriority);

		msg_processor = new MsgProcessor(com_msg_mutex);
		//msg_processor->moveToThread(msg_processor);
		//msg_processor->start(QThread::HighPriority);

		msg_data_container = new MsgInfoContainer; 
		dataset_storage = new DataSets;
	}
	//else
	//{
	//	int ret = QMessageBox::warning(this, "Warning!", tr("Available COM-Ports haven't been found!"), QMessageBox::Ok);
	//	exit(0);
	//}
	

	ui->setupUi(this);
	ui->gridLayout_2->setContentsMargins(2,2,2,2);
	ui->toolBarMain->setFloatable(false);  

	ui->toolBarSettings->setFloatable(false);   

	// расположение окна приложения по центру экрана. Не работает под Linux
	QRect frect = frameGeometry();
	frect.moveCenter(QDesktopWidget().availableGeometry().center());
	move(frect.topLeft());

	setWindowTitle("LoggingTool Manager");
	//setWindowState(Qt::WindowMaximized); // распахнуть главное окно сразу при запуске
	setAttribute(Qt::WA_QuitOnClose);
	//setWindowState(Qt::WindowFullScreen);	
	// ********************************************************************
		
	
	dataset_mutex = new QMutex(QMutex::Recursive);
	//tcp_data_manager = new TcpDataManager(dataset_mutex);

    dock_msgConnect = new QDockWidget(tr("Logging Tool Console"), this);
	QFont fontConnect = dock_msgConnect->font();
	fontConnect.setPointSize(9);
	fontConnect.setBold(true);
	dock_msgConnect->setFont(fontConnect);
	dock_msgConnect->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	nmrtoolLinker = new NMRToolLinker(&nmrtool_tcp_settings, app_settings, dock_msgConnect);
	QFont font_nmrTool = nmrtoolLinker->getUI()->tbtClearAll->font();
	font_nmrTool.setBold(false);
	nmrtoolLinker->getUI()->tbtClearAll->setFont(font_nmrTool);
	nmrtoolLinker->getUI()->tbtLogOnOff->setFont(font_nmrTool);
	font_nmrTool.setPointSize(8);
	nmrtoolLinker->getUI()->tedReport->setFont(font_nmrTool);
	dock_msgConnect->setWidget(nmrtoolLinker);    
	addDockWidget(Qt::BottomDockWidgetArea, dock_msgConnect);
	dock_msgConnect->setVisible(true);
	
	dock_msgLog = new QDockWidget(tr("Communication Log Monitor"), this);
	QFont fontLog = dock_msgLog->font();
	fontLog.setPointSize(9);
	fontLog.setBold(true);
	dock_msgLog->setFont(fontLog);
	dock_msgLog->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	msgLog = new MessageLogMonitor(dock_msgLog);
	QFont font_msgLog = msgLog->getUI()->tbtClearAll->font();
	font_msgLog.setBold(false);
	msgLog->getUI()->tbtClearAll->setFont(font_msgLog);	
	msgLog->getUI()->tbtLogOnOff->setFont(font_msgLog);
	font_msgLog.setPointSize(6);
	msgLog->getUI()->tedLog->setFont(font_msgLog);
	dock_msgLog->setWidget(msgLog);    
	addDockWidget(Qt::BottomDockWidgetArea, dock_msgLog);
	dock_msgLog->setVisible(true);

	dock_PressureUnit = new QDockWidget(tr("Pressure Unit"), this);
	dock_PressureUnit->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	QFont fontPressureUnit = dock_PressureUnit->font();
	fontPressureUnit.setPointSize(9);
	fontPressureUnit.setBold(true);
	dock_PressureUnit->setFont(fontPressureUnit);
	pressUnit = new PressureUnit(dock_PressureUnit);
	dock_PressureUnit->setWidget(pressUnit);
	addDockWidget(Qt::BottomDockWidgetArea, dock_PressureUnit);
	dock_PressureUnit->setVisible(true);
	
	dock_FreqAutoadjust = new QDockWidget(tr("Frequency Autotune"), this);
	dock_FreqAutoadjust->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	QFont fontFreqAutoadjust = dock_FreqAutoadjust->font();
	fontFreqAutoadjust.setPointSize(9);
	fontFreqAutoadjust.setBold(true);
	dock_FreqAutoadjust->setFont(fontFreqAutoadjust);
	freqAutoadjust = new FreqAutoadjustWizard(dock_FreqAutoadjust);
	dock_FreqAutoadjust->setWidget(freqAutoadjust);
	addDockWidget(Qt::BottomDockWidgetArea, dock_FreqAutoadjust);
	dock_FreqAutoadjust->setVisible(true);

	dock_RxTxControl = new QDockWidget(tr("Rx/Tx Control"), this);
	dock_RxTxControl->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	QFont fontRxTxControl = dock_RxTxControl->font();
	fontRxTxControl.setPointSize(9);
	fontRxTxControl.setBold(true);
	dock_RxTxControl->setFont(fontRxTxControl);
	rxtxControl = new RxTxControlWizard(dock_RxTxControl);
	dock_RxTxControl->setWidget(rxtxControl);
	addDockWidget(Qt::BottomDockWidgetArea, dock_RxTxControl);
	dock_RxTxControl->setVisible(true);

	dock_RFPulseControl = new QDockWidget(tr("RF-Pulse Control"), this);
	dock_RFPulseControl->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	QFont fontRFPulseControl = dock_RFPulseControl->font();
	fontRFPulseControl.setPointSize(9);
	fontRFPulseControl.setBold(true);
	dock_RFPulseControl->setFont(fontRFPulseControl);
	rfpulseControl = new RFPulseControlWizard(dock_RFPulseControl);
	dock_RFPulseControl->setWidget(rfpulseControl);
	addDockWidget(Qt::BottomDockWidgetArea, dock_RFPulseControl);
	dock_RFPulseControl->setVisible(true);
		
	/*
	QString depth_port_name = "COM9";
	if (app_settings->contains("DepthMeter/PortName")) depth_port_name = app_settings->value("DepthMeter/PortName").toString();
	else app_settings->setValue(("DepthMeter/PortName"), QVariant(depth_port_name));
	COM_Port_depth = new COM_PORT;
	initCOMSettings(COM_Port_depth, "DepthMeter");
	conn_res = setupCOMPort(COM_Port_depth, depth_port_name);
	if (!conn_res) conn_res = findAvailableCOMPort(COM_Port_depth);
	if (!conn_res)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Available COM-Port for Depth Meter hasn't been found!"), QMessageBox::Ok);	
		exit(0);
	}	
	QString stepmotor_port_name = "COM8";
	if (app_settings->contains("StepMotor/PortName")) stepmotor_port_name = app_settings->value("StepMotor/PortName").toString();
	else app_settings->setValue(("StepMotor/PortName"), QVariant(stepmotor_port_name));
	COM_Port_stepmotor = new COM_PORT;
	initCOMSettings(COM_Port_stepmotor, "StepMotor");
	conn_res = setupCOMPort(COM_Port_stepmotor, stepmotor_port_name);
	if (!conn_res) conn_res = findAvailableCOMPort(COM_Port_stepmotor);
	if (!conn_res)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Available COM-Port for Step Motor control system hasn't been found!"), QMessageBox::Ok);	
		exit(0);
	}
	*/

	QStringList depth_meter_list; 
	depth_meter_list << current_tool.depth_monitors;
	
	dock_depthTemplate = new QDockWidget(tr("Depth Monitoring"), this);
	dock_depthTemplate->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	QFont fontDepthTemplate = dock_depthTemplate->font();
	fontDepthTemplate.setPointSize(9);
	fontDepthTemplate.setBold(true);
	dock_depthTemplate->setFont(fontDepthTemplate);
	depthTemplate = new DepthTemplateWizard(app_settings, &dmeter_tcp_settings, &stmotor_tcp_settings, depth_meter_list, clocker, dock_depthTemplate);
	fontDepthTemplate.setBold(false);
	depthTemplate->getUI()->cboxDepthMeter->setFont(fontDepthTemplate);
	depthTemplate->getUI()->lblDepthMeter->setFont(fontDepthTemplate);
	dock_depthTemplate->setWidget(depthTemplate);
	addDockWidget(Qt::BottomDockWidgetArea, dock_depthTemplate);
	dock_depthTemplate->setVisible(true);
			
	dock_sequenceProc = new QDockWidget(tr("Sequence Wizard"), this);	
	QFont fontProc = dock_sequenceProc->font();
	fontProc.setPointSize(9);
	fontProc.setBold(true);
	//dock_sequenceProc->resize(300, dock_sequenceProc->height());
	dock_sequenceProc->setFont(fontProc);
	dock_sequenceProc->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::NoDockWidgetArea);
	//dock_sequenceProc->setMinimumWidth(350);
	sequenceProc = new SequenceWizard(app_settings, dock_sequenceProc);	
	sequenceProc->setMinimumWidth(300);
	sequenceProc->resize(500, sequenceProc->height());
	QFont font_proc = sequenceProc->getUI()->lblSeqFile->font();
	font_proc.setBold(false);
	sequenceProc->getUI()->lblSeqFile->setFont(font_proc);
	sequenceProc->getUI()->lblSeqName->setFont(font_proc);
	sequenceProc->getUI()->ledSeqName->setFont(font_proc);
	sequenceProc->getUI()->cboxSequences->setFont(font_proc);
	sequenceProc->getUI()->lblDescription->setFont(font_proc);
	sequenceProc->getUI()->pbtViewCode->setFont(font_proc);
	sequenceProc->getUI()->pbtRefresh->setFont(font_proc);
	//sequenceProc->getUI()->pbtExportSettings->setFont(font_proc);
	dock_sequenceProc->setWidget(sequenceProc);    
	addDockWidget(Qt::LeftDockWidgetArea, dock_sequenceProc);	
	//dock_sequenceProc->setVisible(true);
	sequence_status = SeqStatus::Seq_Not_Appl;

	dock_sdspProc = new QDockWidget(tr("SDSP Wizard"), this);	
	QFont fontSDSPProc = dock_sdspProc->font();
	fontSDSPProc.setPointSize(9);
	fontSDSPProc.setBold(true);
	//dock_sequenceProc->resize(300, dock_sequenceProc->height());
	dock_sdspProc->setFont(fontSDSPProc);
	dock_sdspProc->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::NoDockWidgetArea);
	//dock_sequenceProc->setMinimumWidth(350);
	sdspProc = new SDSPWizard(app_settings, dock_sdspProc);	
	sdspProc->setMinimumWidth(300);
	sdspProc->resize(500, sdspProc->height());
	QFont font_sdsp_proc = sdspProc->getUI()->lblSeqFile->font();
	font_sdsp_proc.setBold(false);
	sdspProc->getUI()->lblSeqFile->setFont(font_sdsp_proc);
	sdspProc->getUI()->lblSeqName->setFont(font_sdsp_proc);
	sdspProc->getUI()->ledSeqName->setFont(font_sdsp_proc);
	sdspProc->getUI()->cboxSequences->setFont(font_sdsp_proc);
	sdspProc->getUI()->lblDescription->setFont(font_sdsp_proc);
	sdspProc->getUI()->pbtViewCode->setFont(font_sdsp_proc);
	sdspProc->getUI()->pbtRefresh->setFont(font_sdsp_proc);
	dock_sdspProc->setWidget(sdspProc);    
	addDockWidget(Qt::LeftDockWidgetArea, dock_sdspProc);	
	//dock_sdspProc->setVisible(true);
	sdsp_sequence_status = SeqStatus::Seq_Not_Appl;	
		
	dock_expScheduler = new QDockWidget(tr("Experiment Scheduler"), this);
	QFont fontManager = dock_expScheduler->font();
	fontManager.setPointSize(9);
	dock_expScheduler->setFont(fontManager);
	dock_expScheduler->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::NoDockWidgetArea);
	expScheduler = new SchedulerWizard(sequenceProc, depthTemplate, nmrtoolLinker, clocker, dock_expScheduler);	
	expScheduler->setMinimumWidth(300);
	expScheduler->resize(500, sdspProc->height());
	expScheduler->setJSeqList(sequenceProc->getSeqFileList());
	//expScheduler->setJSeqFile(sequenceProc->getJSeqFile());
	dock_expScheduler->setWidget(expScheduler);    
	addDockWidget(Qt::LeftDockWidgetArea, dock_expScheduler);	
	
	dock_nmrtoolStatus = new QDockWidget(tr("Logging Tool Status"), this);
	QFont fontNMRToolStatus = dock_nmrtoolStatus->font();
	fontNMRToolStatus.setPointSize(9);
	fontNMRToolStatus.setBold(true);
	dock_nmrtoolStatus->setFont(fontNMRToolStatus);
	dock_nmrtoolStatus->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	nmrtoolStatus = new NMRToolStatusWizard(dock_nmrtoolStatus);
	QFont toolstatus_font = nmrtoolStatus->getUI()->lblPower->font();
	toolstatus_font.setBold(false);
	nmrtoolStatus->getUI()->lblPower->setFont(toolstatus_font);
	nmrtoolStatus->getUI()->lblTemperature->setFont(toolstatus_font);
	nmrtoolStatus->getUI()->lblDataTransfer->setFont(toolstatus_font);
	dock_nmrtoolStatus->setWidget(nmrtoolStatus);
	addDockWidget(Qt::BottomDockWidgetArea, dock_nmrtoolStatus);	
	dock_nmrtoolStatus->setVisible(true);
			
	this->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);

	this->tabifyDockWidget(dock_sdspProc, dock_expScheduler);
	this->tabifyDockWidget(dock_expScheduler, dock_sequenceProc);
	QList<QTabBar*> tabBars = findChildren<QTabBar*>();
	foreach( QTabBar* bar, tabBars )
	{
		int count = bar->count();
		for (int i = 0; i < count; i++)
		{
			QVariant data = bar->tabData(i);
			QVariant::DataPtr dataPtr = data.data_ptr();
			if (dataPtr.data.ptr == dock_expScheduler)
			{
				bar->setCurrentIndex(i);
			}
		}
	}	

	relax_widget = new RelaxationWidget(ui->tabDataViewer, app_settings);
		
	osc_widget = new OscilloscopeWidget(ui->tabOscilloscope, app_settings, tool_channels);
	//ui->tabOscilloscope->setVisible(false);
	//ui->tabOscilloscope->setContentsMargins(1,1,1,1);

	monitoring_widget = new MonitoringWidget(app_settings, ui->tabMonitoring);
	QGridLayout *grlout_mon = new QGridLayout(ui->tabMonitoring);
	grlout_mon->setContentsMargins(1,1,1,1);
	grlout_mon->addWidget(monitoring_widget, 0, 0, 1, 1);

	sdsp_widget = new SDSPWidget(app_settings, ui->tabSDSP);
	QGridLayout *grlout_sdsp = new QGridLayout(ui->tabSDSP);
	grlout_sdsp->setContentsMargins(1,1,1,1);
	grlout_sdsp->addWidget(sdsp_widget, 0, 0, 1, 1);

	logging_widget = new LoggingWidget(tool_channels, ui->tabLogging); 
	QSizePolicy size_policy = logging_widget->sizePolicy();
	size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
	logging_widget->setSizePolicy(size_policy);

	double core_d = 0.10; 
	double core_porosity = 30;
	double core_diameter = 0.10;
	bool _ok;
	if (app_settings->contains("CoreTransportSystem/CoreDiameter")) core_d = app_settings->value("CoreTransportSystem/CoreDiameter").toDouble(&_ok);
	else app_settings->setValue("CoreTransportSystem/CoreDiameter", 0.10);
	if (_ok) logging_widget->setCoreDiameter(core_d);
	else logging_widget->setCoreDiameter(0.10);	
	if (app_settings->contains("CoreTransportSystem/StandardPorosity")) core_porosity = app_settings->value("CoreTransportSystem/StandardPorosity").toDouble(&_ok);
	else app_settings->setValue("CoreTransportSystem/StandardPorosity", 30);
	if (_ok) logging_widget->setStandardPorosity(core_porosity);
	else logging_widget->setStandardPorosity(30);	
	if (app_settings->contains("CoreTransportSystem/StandardCoreDiameter")) core_diameter = app_settings->value("CoreTransportSystem/StandardCoreDiameter").toDouble(&_ok);
	else app_settings->setValue("CoreTransportSystem/StandardCoreDiameter", 0.10);
	if (_ok) logging_widget->setStandardCoreDiameter(core_diameter);
	else logging_widget->setStandardCoreDiameter(0.10);
	QGridLayout *grlout_logging = new QGridLayout(ui->tabLogging);
	grlout_logging->setContentsMargins(1,1,1,1);
	grlout_logging->addWidget(logging_widget, 0, 0, 1, 1);
	
	/*
	dock_expScheduler->setVisible(false);
	dock_sequenceProc->setVisible(false);
	dock_sdspProc->setVisible(false);

	relax_widget->setVisible(false);
	osc_widget->setVisible(false);
	monitoring_widget->setVisible(false);
	sdsp_widget->setVisible(false);
	logging_widget->setVisible(false);
	*/

	ui->a_CommunicationLogMonitor->setCheckable(true);
	ui->a_LoggingToolConsole->setCheckable(true);
	ui->a_SequenceWizard->setCheckable(true);
	ui->a_MainToolBar->setCheckable(true);
	ui->a_LoggingToolStatus->setCheckable(true);
	ui->a_RxTxControl->setCheckable(true);
	ui->a_DepthMonitor->setCheckable(true);

	ui->a_CommunicationLogMonitor->setChecked(true);
	ui->a_LoggingToolConsole->setChecked(true);
	ui->a_LoggingToolStatus->setChecked(true);
	ui->a_RxTxControl->setChecked(true);
	ui->a_DepthMonitor->setChecked(true);	
	ui->a_SequenceWizard->setChecked(true);
	ui->a_MainToolBar->setChecked(true);
		
	setActions();
	initSettingsBar();
	initExperimentBar();
	initStatusBar();
	initSaveDataAttrs();
	initExperimentSettings();
	initDataTypes();
	initExperimentalInfo();
		
	loadProcessingSettings();
	QwtPlot *spectrum_plot = relax_widget->getRelaxMathPlot()->getPlot();
	spectrum_plot->setAxisScale(QwtPlot::xBottom, processing_relax.T2_from, processing_relax.T2_to);
	
	setConnections();

	ui->tabWidget->setMovable(true);

	//dock_sequenceProc->setVisible(true);
	//dock_sdspProc->setVisible(false);
		
	sdsptab_is_active = false;

	nmrtool_state = false;
	sdsptool_state = false;

	applyToolSettings();

	setWindowState(Qt::WindowMaximized);

	//setDefaultCommSettings(true);
	default_comm_settings_on = false;
	nmrtoolLinker->setDefaultCommSettingsState(default_comm_settings_on);	
}

MainWindow::~MainWindow()
{
	delete ui;   

	delete a_connect;
	delete a_disconnect;
	delete a_start;
	delete a_apply_prg;
	delete a_stop;

	delete relax_widget;
	delete osc_widget;

	/*delete COM_Port->COM_port;
	delete COM_Port;
	delete comm_settings;
	delete COM_Port_depth->COM_port;
	delete COM_Port_depth;
	delete COM_Port_stepmotor->COM_port;
	delete COM_Port_stepmotor;
	*/
	delete nmrtool_tcp_settings.socket;
	delete dmeter_tcp_settings.socket;
	delete stmotor_tcp_settings.socket;

	//delete expScheduler;

	qDeleteAll(tools_settings.begin(), tools_settings.end());

	//delete depthMonitor;
	
	int count = 0;
	/*while (count < msg_data_container->size())
	{
		MsgInfo* msg_info = msg_data_container->at(count++);
		delete msg_info;		
	}
	msg_data_container->clear();
	delete msg_data_container;*/
	
	//delete tcp_data_manager;

	qDeleteAll(msg_data_container->begin(), msg_data_container->end());
	delete msg_data_container;
		
	qDeleteAll(dataset_storage->begin(), dataset_storage->end());
	delete dataset_storage;

	qDeleteAll(tool_channels.begin(), tool_channels.end());
	
	//clocker->stop();
	emit stop_clocker();
	clocker->exit();
	clocker->wait();
	delete clocker;

	emit stop_com_commander();
	com_commander->exit();
	com_commander->wait();
	delete com_commander;
	
	//msg_processor->exit();
	//msg_processor->wait();
	delete msg_processor;
		
	delete com_msg_mutex;	
	delete dataset_mutex;

	gfdata_destroy(gf_data);
	free(gf_data);
}


void MainWindow::setActions()
{
	ui->toolBarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	//a_connect = new QAction(this);
	a_connect = ui->a_connect;
	a_connect->setText("Connect");
	a_connect->setCheckable(true);    
	a_connect->setIcon(QIcon(":/images/add.png"));

	//a_disconnect = new QAction(this);
	a_disconnect = ui->a_disconnect;
	a_disconnect->setText("Disconnect");
	a_disconnect->setCheckable(false);    
	a_disconnect->setIcon(QIcon(":/images/remove.png")); 

	//a_start = new QAction(this);
	a_start = ui->a_start;
	a_start->setText("Start Sequence");
	a_start->setCheckable(true);
	//a_start->setIcon(QIcon(":/images/play.png"));
	a_start->setIcon(QIcon(":/images/play_red.png"));

	a_run = new QAction(this);
	a_run->setText("Run Sequence");
	a_run->setCheckable(true);
	a_run->setIcon(QIcon(":/images/play_all.png"));
	a_run->setVisible(false);

	a_apply_prg = new QAction(this);
	a_apply_prg->setText("Apply Sequence");
	a_apply_prg->setCheckable(true);
	a_apply_prg->setIcon(QIcon(":/images/record_button_red.png"));

	//a_stop = new QAction(this);
	a_stop = ui->a_stop;
	a_stop->setText("Stop Sequence");
	a_stop->setCheckable(true);
	a_stop->setIcon(QIcon(":/images/stop.png"));

	//a_break = new QAction(this);
	a_break = ui->a_break;
	a_break->setText("Break All Actions");
	a_break->setCheckable(false);
	a_break->setIcon(QIcon(":/images/pause.png"));

	//a_start_sdsp = new QAction(this);
	a_start_sdsp = ui->a_start_sdsp;
	a_start_sdsp->setText("Start SDSP");
	a_start_sdsp->setCheckable(true);	
	a_start_sdsp->setIcon(QIcon(":/images/start_green.png"));

	//a_stop_sdsp = new QAction(this);
	a_stop_sdsp = ui->a_stop_sdsp;
	a_stop_sdsp->setText("Stop SDSP");
	a_stop_sdsp->setCheckable(true);
	a_stop_sdsp->setIcon(QIcon(":/images/stop_red.png"));
	
	ui->toolBarMain->addAction(a_connect);
	ui->toolBarMain->addAction(a_disconnect);
	ui->toolBarMain->addAction(a_start);	
	//ui->toolBarMain->addAction(a_run);
	ui->toolBarMain->addAction(a_stop);	
	//ui->toolBarMain->addAction(a_apply_prg);
	ui->toolBarMain->addAction(a_start_sdsp);
	ui->toolBarMain->addAction(a_stop_sdsp);
	ui->toolBarMain->addAction(a_break);

	a_connect->setEnabled(true);
	a_disconnect->setEnabled(true);
	a_start->setEnabled(false);
	a_stop->setEnabled(false);
	//a_apply_prg->setEnabled(false);
	a_start_sdsp->setEnabled(false);
	a_stop_sdsp->setEnabled(false);

	a_start->setVisible(true);
	a_stop->setVisible(true);
	a_start_sdsp->setVisible(false);
	a_stop_sdsp->setVisible(false);

	ui->a_toFile->setVisible(false);
	ui->a_toOil->setVisible(false);


	ui->toolBarSettings->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	ui->a_SaveAllSettings->setIcon(QIcon(":/images/save_settings.png"));		
	ui->a_Processing->setIcon(QIcon(":/images/settings2.png"));
	ui->a_DataFile_Settings->setIcon(QIcon(":/images/export_data.png"));
	ui->a_Experiment_Settings->setIcon(QIcon(":/images/experiment_settings.png"));
}

void MainWindow::setConnections()
{
	qRegisterMetaType<uint32_t>("uint32_t");
	qRegisterMetaType<QVariantList>("QVariantList");

	connect(ui->a_COMPort, SIGNAL(triggered()), this, SLOT(setCOMPortSettings()));   
	connect(ui->a_Communication, SIGNAL(triggered()), this, SLOT(setCommunicationSettings()));
	//connect(ui->a_TCP_Connection, SIGNAL(triggered()), this, SLOT(setTCPConnectionSettings()));
	//connect(ui->a_CDiag_Connection, SIGNAL(triggered()), this, SLOT(setCDiagConnectionSettings()));
	connect(ui->a_Processing, SIGNAL(triggered()), this, SLOT(setProcessingSettings()));
	connect(ui->a_About, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
	//connect(ui->a_toOil, SIGNAL(triggered()), this, SLOT(setExportToOilData()));
	//connect(ui->a_toFile, SIGNAL(triggered()), this, SLOT(setExportToFileData()));
	//connect(ui->a_Depthmeter_Connection, SIGNAL(triggered()), this, SLOT(setDepthMeterSettings()));
	//connect(ui->a_StepMotor, SIGNAL(triggered()), this, SLOT(setStepMotorCOMSettings()));
	connect(ui->a_Tool_Settings, SIGNAL(triggered()), this, SLOT(setToolSettings()));
	connect(ui->a_Change_Tool, SIGNAL(triggered()), this, SLOT(changeLoggingTool()));
	connect(ui->a_DataFile_Settings, SIGNAL(triggered()), this, SLOT(setDataFileSettings()));
	connect(ui->a_Experiment_Settings, SIGNAL(triggered()), this, SLOT(setExperimentSettings()));
	
	connect(dock_RxTxControl, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), rxtxControl, SLOT(changeLocation(Qt::DockWidgetArea)));
	connect(dock_RFPulseControl, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), rfpulseControl, SLOT(changeLocation(Qt::DockWidgetArea)));
	connect(dock_nmrtoolStatus, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), nmrtoolStatus, SLOT(changeLocation(Qt::DockWidgetArea)));
	//connect(dock_depthMonitor, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), depthMonitor, SLOT(changeLocation(Qt::DockWidgetArea)));

	connect(com_commander, SIGNAL(COM_message(COM_Message*, uint32_t)), msg_processor, SLOT(receiveMsgFromCOMComander(COM_Message*, uint32_t)));
	//connect(com_commander, SIGNAL(bad_COM_message(COM_Message*)), msg_processor, SLOT(receiveBadMsgFromCOMCommander(COM_Message*)));
	connect(com_commander, SIGNAL(bad_COM_message(QString)), msg_processor, SLOT(receiveBadMsgFromCOMCommander(QString)));
	connect(com_commander, SIGNAL(device_data_timed_out(uint32_t)), msg_processor, SLOT(reportNoResponse(uint32_t)));
	connect(com_commander, SIGNAL(msg_state(int, int)), nmrtoolLinker, SLOT(showMsgTrafficReport(int, int)));

	connect(logging_widget, SIGNAL(new_calibration_coef_toCfg(double, ToolChannel*)), this, SLOT(saveNewCalibrCoefficientToCfg(double, ToolChannel*)));
	connect(logging_widget, SIGNAL(new_calibration_coef(double, ToolChannel*)), this, SLOT(saveNewCalibrCoefficient(double, ToolChannel*)));

	//connect(tcp_data_manager, SIGNAL(get_data(const QString&, int)), this, SLOT(sendDataToNetClients(const QString&, int)));

	//connect(depthMonitor, SIGNAL(connected(bool)), this, SLOT(depthDepthMeterConnected(bool)));
	connect(depthTemplate, SIGNAL(connected(bool)), this, SLOT(depthDepthMeterConnected(bool)));
	connect(depthTemplate, SIGNAL(new_core_diameter(double)), logging_widget, SLOT(setCoreDiameter(double)));

	connect(nmrtoolLinker, SIGNAL(control_nmrtool(bool)), com_commander, SLOT(setNMRToolState(bool)));
	connect(this, SIGNAL(control_sdsptool(bool)), com_commander, SLOT(setSDSPToolState(bool)));
	connect(this, SIGNAL(stop_com_commander()), com_commander, SLOT(stopThread()));
	connect(this, SIGNAL(sdsp_is_enabled(bool)), sdsp_widget, SLOT(enableSDSP(bool)));
	
	connect(msg_processor, SIGNAL(message_str(QString)), msgLog, SLOT(addText(QString)));
	connect(msg_processor, SIGNAL(message_str_to_Tool_console(QString)), nmrtoolLinker, SLOT(addText(QString)));
	connect(msg_processor, SIGNAL(show_cmd_result(uint32_t, QString, QVariantList)), nmrtoolLinker, SLOT(showCmdResult(uint32_t, QString, QVariantList)));
	connect(msg_processor, SIGNAL(message_info(MsgInfo*)), this, SLOT(storeMsgData(MsgInfo*)));
	connect(msg_processor, SIGNAL(new_data(DeviceData*)), this, SLOT(treatNewData(DeviceData*)));
	connect(msg_processor, SIGNAL(execute_cmd(DeviceData*)), com_commander, SLOT(addCmdToQueue(DeviceData*)));
	connect(msg_processor, SIGNAL(power_status(unsigned char)), this, SLOT(showPowerStatus(unsigned char)));
	connect(msg_processor, SIGNAL(fpga_seq_status(unsigned char)), expScheduler, SLOT(setSeqStatus(unsigned char)));
	connect(msg_processor, SIGNAL(apply_tool_id(unsigned char)), this, SLOT(setToolId(unsigned char)));
	
	connect(nmrtoolLinker, SIGNAL(send_msg(DeviceData*, const QString&)), msg_processor, SLOT(sendMsg(DeviceData*, const QString&)));
	connect(nmrtoolLinker, SIGNAL(place_to_statusbar(QString&)), this, SLOT(placeInfoToStatusBar(QString&)));
	connect(nmrtoolLinker, SIGNAL(start_experiment(bool)), this, SLOT(startExperiment(bool)));
	connect(nmrtoolLinker, SIGNAL(cmd_resulted(uint8_t, ConnectionState)), this, SLOT(setCmdResult(uint8_t, ConnectionState)));
	connect(nmrtoolLinker, SIGNAL(send_data_toSDSP(QByteArray&)), com_commander, SLOT(sendToSDSP(QByteArray*)));
	connect(nmrtoolLinker, SIGNAL(tool_settings_applied(bool)), this, SLOT(setToolSettingsApplied(bool)));
	connect(nmrtoolLinker, SIGNAL(fpga_seq_started(bool)), expScheduler, SLOT(setSeqStarted(bool)));
	connect(nmrtoolLinker, SIGNAL(default_comm_settings(bool)), this, SLOT(setDefaultCommSettings(bool)));

	connect(pressUnit, SIGNAL(send_msg(DeviceData*, const QString&)), msg_processor, SLOT(sendMsg(DeviceData*, const QString&)));
	
	connect(&experiment_timer, SIGNAL(timeout()), this, SLOT(setExperimentalInfo()));

	connect(sequenceProc, SIGNAL(sequence_changed()), this, SLOT(setSequenceChanged()));
	connect(sequenceProc, SIGNAL(save_data_changed(DataSave&)), this, SLOT(setSaveDataAttrs(DataSave&)));

	connect(monitoring_widget, SIGNAL(temperature_status(unsigned char)), this, SLOT(showTemperatureStatus(unsigned char)));

	connect(osc_widget, SIGNAL(apply_win_func(QVector<int>&)), this, SLOT(applyWinFuncParams(QVector<int>&)));
	connect(relax_widget, SIGNAL(apply_moving_averaging(bool, int)), this, SLOT(setMovingAveragingSettings(bool, int)));

	connect(sdsp_widget, SIGNAL(apply_sdsp_params(QVector<int>&)), this, SLOT(applySDSPParams(QVector<int>&)));
	connect(sdsp_widget, SIGNAL(place_to_statusbar(QString&)), this, SLOT(placeInfoToStatusBar(QString&)));

	connect(expScheduler, SIGNAL(finished()), this, SLOT(setExpSchedulerFinished()));
	connect(expScheduler, SIGNAL(started()), this, SLOT(setExpSchedulerStarted()));
	connect(expScheduler, SIGNAL(calibration_started()), logging_widget, SLOT(startCalibration()));
	connect(expScheduler, SIGNAL(calibration_finished()), logging_widget, SLOT(finishCalibration()));
	connect(expScheduler, SIGNAL(new_msg_req_delay(int)), com_commander, SLOT(setMsgReqDelay(int)));		// added 5.07.2018

	connect(clocker, SIGNAL(clock()), com_commander, SLOT(timeClocked()));	
	connect(this, SIGNAL(stop_clocker()), clocker, SLOT(stopThread()));

	connect(a_start, SIGNAL(triggered(bool)), this, SLOT(startNMRTool(bool)));
	connect(a_stop, SIGNAL(triggered(bool)), this, SLOT(stopNMRTool(bool)));
	connect(a_apply_prg, SIGNAL(triggered(bool)), this, SLOT(applyDSPProcPrg(bool)));
	connect(a_connect, SIGNAL(triggered(bool)), this, SLOT(connectToNMRTool(bool)));
	connect(a_disconnect, SIGNAL(triggered()), this, SLOT(disconnectFromNMRTool()));
	connect(a_break, SIGNAL(triggered()), com_commander, SLOT(breakAllActions()));
	connect(a_break, SIGNAL(triggered()), this, SLOT(breakAllActions()));
	connect(a_start_sdsp, SIGNAL(triggered(bool)), this, SLOT(startSDSPTool(bool)));
	connect(a_stop_sdsp, SIGNAL(triggered(bool)), this, SLOT(stopSDSPTool(bool)));
	connect(ui->a_reset_comm_settings, SIGNAL(triggered()), this, SLOT(resetCommSettings()));

	connect(ui->a_CommunicationLogMonitor, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));
	connect(ui->a_LoggingToolConsole, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));
	connect(ui->a_SequenceWizard, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));
	connect(ui->a_LoggingToolStatus, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));
	connect(ui->a_DepthMonitor, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));
	connect(ui->a_RxTxControl, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));
	connect(ui->a_MainToolBar, SIGNAL(triggered(bool)), this, SLOT(viewWizard(bool)));

	connect(ui->a_SaveAllSettings, SIGNAL(triggered()), this, SLOT(saveAllSettings()));

	connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabIsActivated(int)));
	connect(dock_sdspProc, SIGNAL(visibilityChanged(bool)), this, SLOT(sdspIsActivated(bool)));

	connect(&T2processing_pool, SIGNAL(data_ready()), this, SLOT(plotSpectralData()));
}


void MainWindow::initExperimentBar()
{
	lblStartTime = new QLabel(this);
	lblStartTime->setFrameShape(QFrame::WinPanel);
	lblStartTime->setFrameStyle(QFrame::Sunken);
	QFont font1("Arial", 11);
	lblStartTime->setFont(font1);
	lblStartTime->setText("");
	
	lblDataCounter = new QLabel(this);
	lblDataCounter->setFrameShape(QFrame::WinPanel);
	lblDataCounter->setFrameStyle(QFrame::Sunken);
	QFont font2("Arial", 11);
	lblDataCounter->setFont(font2);
	lblDataCounter->setText("");

	lblTool = new QLabel(this);
	lblTool->setFrameShape(QFrame::WinPanel);
	lblTool->setFrameStyle(QFrame::Sunken);
	QFont font3("Arial", 11);
	lblTool->setFont(font3);
	lblTool->setText("");

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);	
	ui->toolBarExperiment->addWidget(lblTool);
	ui->toolBarExperiment->addWidget(spacer);
	ui->toolBarExperiment->addWidget(lblDataCounter);
	ui->toolBarExperiment->addWidget(lblStartTime);

	ui->toolBarExperiment->layout()->setSpacing(20);

	clearStartExperimentData();
}

void MainWindow::initSettingsBar()
{
	QWidget* spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);		
	ui->toolBarSettings->addWidget(spacer);
	ui->toolBarSettings->addAction(ui->a_Processing);	
	ui->toolBarSettings->addAction(ui->a_SaveAllSettings);
}

void MainWindow::clearStartExperimentData()
{
	lblStartTime->setText("");
	starting_time = QTime(0,0,0);
	obtained_data_counter = 0;
}

void MainWindow::initStatusBar()
{
	lblStatusInfo = new QLabel(this);
	lblStatusInfo->setFrameShape(QFrame::StyledPanel);
	lblStatusInfo->setFrameStyle(QFrame::Sunken);
	lblStatusInfo->setText("");	
	this->statusBar()->addPermanentWidget(lblStatusInfo,100);

	TrafficWidget *traffic_widget = nmrtoolLinker->getTrafficWidget();
	ConnectionWidget *conn_widget = nmrtoolLinker->getConnectionWidget();	

	//TcpConnectionWidget *tcp_conn_widget = tcp_data_manager->getTcpConnectionWidget();
	//this->statusBar()->addPermanentWidget(tcp_conn_widget, 10);
	
	ImpulsConnectionWidget *impuls_widget = depthTemplate->getConnectionWidget();
	this->statusBar()->addPermanentWidget(impuls_widget, 15);
	this->statusBar()->addPermanentWidget(conn_widget, 15); 
	this->statusBar()->addPermanentWidget(traffic_widget, 14);	
}


void MainWindow::placeInfoToStatusBar(QString& str)
{
	lblStatusInfo->setText(str);	
	QTimer::singleShot(5000, this, SLOT(clearStatusBar()));
}

void MainWindow::clearStatusBar()
{
	lblStatusInfo->setText("");
}

void MainWindow::placeInfoToExpToolBar(QString& str_count, QString &str_time, QString& tool_info)
{
	lblStartTime->setText(QString("<font color = darkGreen><b>%1</b></font>").arg(str_count));	
	lblDataCounter->setText(QString("<font color = darkGreen><b>%1</b></font>").arg(str_time));	
	lblTool->setText(QString("<font color = darkBlue><b>%1</b></font>").arg(tool_info));	
}

void MainWindow::placeInfoToExpToolBar(QString& str_count, QString &str_time)
{	
	lblStartTime->setText(QString("<font color = darkGreen><b>%1</b></font>").arg(str_count));	
	lblDataCounter->setText(QString("<font color = darkGreen><b>%1</b></font>").arg(str_time));	
}

void MainWindow::placeInfoToExpToolBar(QString& tool_info)
{		
	//QString text = QString("<font color = darkBlue><b>%1</b></font>").arg(tool_info);
	//lblTool->setText(trUtf8(text.toLocal8Bit()));
	lblTool->setText(QString("<font color = darkBlue><b>%1</b></font>").arg(tool_info));		
}

void MainWindow::startExperiment(bool flag)
{
	nmrtool_state = flag;

	if (flag) experiment_timer.start(1000);		
	else experiment_timer.stop();

	starting_time = QTime::currentTime();	
	obtained_data_counter = 0;

	//expScheduler->stop();
}

void MainWindow::initExperimentalInfo()
{
	QString str = "00:00:00";	
	QString info1 = tr("Datasets obtained") + QString(": %1").arg(0);
	QString info2 = tr("Time elapsed") + QString(": %1").arg(str);
	QString info3 = tr("Logging Tool") + QString(": %1").arg("");
	
	placeInfoToExpToolBar(info1, info2, info3);
}

void MainWindow::setExperimentalInfo()
{
	int all_secs = starting_time.secsTo(QTime::currentTime());
	int hrs = all_secs/3600;
	int mins = (all_secs - 3600*hrs)/60;
	int secs = all_secs - 3600*hrs - 60*mins;
	QTime elapsed_time = QTime(hrs, mins, secs);

	QString info1 = tr("Datasets obtained") + QString(": %1").arg(obtained_data_counter);
	QString info2 = tr("Time elapsed") + QString(": %1").arg(elapsed_time.toString("hh:mm:ss"));

	placeInfoToExpToolBar(info1, info2);
}

void MainWindow::setToolSettingsApplied(bool flag)
{
	current_tool_was_applied = true;	
}

void MainWindow::loadProcessingSettings()
{	
	if (app_settings->contains("ProcessingSettings/QualityControl")) processing_relax.is_quality_controlled = app_settings->value("ProcessingSettings/QualityControl").toBool(); else app_settings->setValue("ProcessingSettings/QualityControl", processing_relax.is_quality_controlled);

	bool ok;
	if (app_settings->contains("ProcessingSettings/RelaxAverage")) processing_relax.win_aver_on = app_settings->value("ProcessingSettings/RelaxAverage").toBool(); else app_settings->setValue("ProcessingSettings/RelaxAverage", processing_relax.win_aver_on);
	if (app_settings->contains("ProcessingSettings/RelaxAverageWin")) processing_relax.win_aver_len = app_settings->value("ProcessingSettings/RelaxAverageWin").toInt(&ok); else app_settings->setValue("ProcessingSettings/RelaxAverageWin", processing_relax.win_aver_len);
	
	if (app_settings->contains("ProcessingSettings/T2_from")) processing_relax.T2_from = app_settings->value("ProcessingSettings/T2_from").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_from", processing_relax.T2_from);
	if (app_settings->contains("ProcessingSettings/T2_to")) processing_relax.T2_to = app_settings->value("ProcessingSettings/T2_to").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_to", processing_relax.T2_to);
	if (app_settings->contains("ProcessingSettings/T2_min")) processing_relax.T2_min = app_settings->value("ProcessingSettings/T2_min").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_min", processing_relax.T2_min);
	if (app_settings->contains("ProcessingSettings/T2_max")) processing_relax.T2_max = app_settings->value("ProcessingSettings/T2_max").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_max", processing_relax.T2_max);
	if (app_settings->contains("ProcessingSettings/T2_cutoff")) processing_relax.T2_cutoff = app_settings->value("ProcessingSettings/T2_cutoff").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_cutoff", processing_relax.T2_cutoff);
	if (app_settings->contains("ProcessingSettings/T2_cutoff_clay")) processing_relax.T2_cutoff_clay = app_settings->value("ProcessingSettings/T2_cutoff_clay").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_cutoff_clay", processing_relax.T2_cutoff_clay);
	if (app_settings->contains("ProcessingSettings/T2_points")) processing_relax.T2_points = app_settings->value("ProcessingSettings/T2_points").toDouble(&ok); else app_settings->setValue("ProcessingSettings/T2_points", processing_relax.T2_points);
	if (app_settings->contains("ProcessingSettings/Iterations")) processing_relax.iters = app_settings->value("ProcessingSettings/Iterations").toDouble(&ok); else app_settings->setValue("ProcessingSettings/Iterations", processing_relax.iters);
	
	if (app_settings->contains("ProcessingSettings/PorosityOn")) processing_relax.porosity_on = app_settings->value("ProcessingSettings/PorosityOn") .toBool(); else app_settings->setValue("ProcessingSettings/PorosityOn", processing_relax.porosity_on);

	QVariant v_MCBW_color = app_settings->value("ProcessingSettings/MCBWColor");
	if (v_MCBW_color != QVariant()) processing_relax.MCBWcolor = QVarToQColor(v_MCBW_color);
	else app_settings->setValue("ProcessingSettings/MCBWColor", QColorToQVar(processing_relax.MCBWcolor));
	QVariant v_MBVIcolor = app_settings->value("ProcessingSettings/MBVIColor");
	if (v_MBVIcolor != QVariant()) processing_relax.MBVIcolor = QVarToQColor(v_MBVIcolor);
	else app_settings->setValue("ProcessingSettings/MBVIColor", QColorToQVar(processing_relax.MBVIcolor));
	QVariant v_MFFIcolor = app_settings->value("ProcessingSettings/MFFIColor");
	if (v_MFFIcolor != QVariant()) processing_relax.MFFIcolor = QVarToQColor(v_MFFIcolor);
	else app_settings->setValue("ProcessingSettings/MFFIColor", QColorToQVar(processing_relax.MFFIcolor));		
}

void MainWindow::loadCommSettings()
{
	comm_settings = new Communication_Settings;
	comm_settings->packet_length = 100;
	comm_settings->block_length = 20;
	comm_settings->errs_count = 2;
	comm_settings->packet_delay = 0;
	comm_settings->antinoise_coding = true;
	comm_settings->packlen_autoadjust = true;
	comm_settings->interleaving = false;
	comm_settings->noise_control = false;

	bool ok;
	if (app_settings->contains("CommSettings/PacketLenght")) comm_settings->packet_length = app_settings->value("CommSettings/PacketLenght").toInt(&ok); 
	else app_settings->setValue("CommSettings/PacketLenght", comm_settings->packet_length);
	if (!ok) { comm_settings->packet_length = 100; app_settings->setValue("CommSettings/PacketLenght", comm_settings->packet_length); }
	
	if (app_settings->contains("CommSettings/BlockLenght")) comm_settings->block_length = app_settings->value("CommSettings/BlockLenght").toInt(&ok); 
	else app_settings->setValue("CommSettings/BlockLenght", comm_settings->block_length);
	if (!ok) { comm_settings->block_length = 20; app_settings->setValue("CommSettings/BlockLenght", comm_settings->block_length); }
	
	if (app_settings->contains("CommSettings/ErrCount")) comm_settings->errs_count = app_settings->value("CommSettings/ErrCount").toInt(&ok); 
	else app_settings->setValue("CommSettings/ErrCount", comm_settings->errs_count);
	if (!ok) { comm_settings->errs_count = 2; app_settings->setValue("CommSettings/ErrCount", comm_settings->errs_count); }

	if (app_settings->contains("CommSettings/PacketDelay")) comm_settings->packet_delay = app_settings->value("CommSettings/PacketDelay").toInt(&ok); 
	else app_settings->setValue("CommSettings/PacketDelay", comm_settings->packet_delay);
	if (!ok) { comm_settings->packet_delay = 0; app_settings->setValue("CommSettings/PacketDelay", comm_settings->packet_delay); }
	
	if (app_settings->contains("CommSettings/AntinoiseCoding")) comm_settings->antinoise_coding = app_settings->value("CommSettings/AntinoiseCoding").toBool(); 
	else app_settings->setValue("CommSettings/AntinoiseCoding", comm_settings->antinoise_coding);
	
	if (app_settings->contains("CommSettings/PacketLengthAutoadjust")) comm_settings->packlen_autoadjust = app_settings->value("CommSettings/PacketLengthAutoadjust").toBool(); 
	else app_settings->setValue("CommSettings/PacketLengthAutoadjust", comm_settings->packlen_autoadjust);

	if (app_settings->contains("CommSettings/Interleaving")) comm_settings->interleaving = app_settings->value("CommSettings/Interleaving").toBool(); 
	else app_settings->setValue("CommSettings/Interleaving", comm_settings->interleaving);

	if (app_settings->contains("CommSettings/NoiseControl")) comm_settings->noise_control = app_settings->value("CommSettings/NoiseControl").toBool(); 
	else app_settings->setValue("CommSettings/NoiseControl", comm_settings->noise_control);
}


void MainWindow::changeLoggingTool()
{	
	loadToolsSettings();
	if (tools_settings.isEmpty())
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot find tool settings file in /Tools folder!"), QMessageBox::Ok, QMessageBox::Ok);
		exit(0);
	}
		
	////////////////////////////////////
	QList<ToolInfo> tools;
	int current_tool_index = -1;
	for (int i = 0; i < tools_settings.count(); i++)
	{				
		QSettings *settings = tools_settings[i];
		QString tool_file = settings->fileName();

		int uid = 0; 
		QString tool_type = "";
		bool ok = false;
		if (settings->contains("Tool/uid")) uid = settings->value("Tool/uid").toInt(&ok); 
		else uid = 0;
		if (settings->contains("Tool/type")) tool_type = settings->value("Tool/type").toString(); 

		if (current_tool.id == uid) current_tool_index = i;
		if (current_tool.type == tool_type) current_tool_index = i;

		QString scan_q;
		if (settings->contains("ScannedQuantity/Quantity")) scan_q = settings->value("ScannedQuantity/Quantity").toString(); 
		if (scan_q == "Depth") current_tool.scanned_quantity == ScannedQuantity::Depth;
		else if (scan_q == "Distance") current_tool.scanned_quantity == ScannedQuantity::Distance;
		else if (scan_q == "Time") current_tool.scanned_quantity == ScannedQuantity::Time;
		else if (scan_q == "Temperature") current_tool.scanned_quantity == ScannedQuantity::Temperature;
		else if (scan_q == "Concentration") current_tool.scanned_quantity == ScannedQuantity::Concentration;
		else current_tool.scanned_quantity == ScannedQuantity::Depth;

		QStringList tab_widgets;
		if (settings->contains("VisualSettings/TabWidgets")) tab_widgets = settings->value("VisualSettings/TabWidgets").toStringList();
		QStringList seq_wizards;
		if (settings->contains("VisualSettings/SequenceWizards")) seq_wizards = settings->value("VisualSettings/SequenceWizards").toStringList();
		QStringList depth_monitors;
		if (settings->contains("VisualSettings/DepthMonitors")) depth_monitors = settings->value("VisualSettings/DepthMonitors").toStringList(); 
		QString tool_info_bar;
		if (settings->contains("VisualSettings/InfoBar")) tool_info_bar = settings->value("VisualSettings/InfoBar").toString().simplified(); 

		if (ok && uid > 0 && !tool_type.isEmpty()) 
		{
			bool tool_exist = false;
			for (int j = 0; j < tools.count(); j++)
			{
				ToolInfo tool_info = tools[j];
				if (tool_info.id == uid) tool_exist = true;
				if (tool_info.type == tool_type) tool_exist = true;
			}

			if (!tool_exist)
			{
				ToolInfo tool_info(uid, tool_type, tool_file); 
				tool_info.tab_widgets = tab_widgets;
				tool_info.seq_wizards = seq_wizards;
				tool_info.depth_monitors = depth_monitors;
				tool_info.info_bar = tool_info_bar;
				tools.append(tool_info);
			}
		}
	}
	///////////////////////////////////////
	
	QStringList tab_widgets = current_tool.tab_widgets;
	QStringList seq_widgets = current_tool.seq_wizards;
	QStringList depth_monitors = current_tool.depth_monitors;
	QString info_bar_text = current_tool.info_bar;
	QString cur_tool_file = current_tool.file_name;
	unsigned char tool_id = current_tool.id;
	QString cur_tool_type = current_tool.type;
	ToolsDialog *dlg = new ToolsDialog(tools, current_tool_index);
	if (dlg->exec())
	{
		cur_tool_file = dlg->getSelectedToolFile();
		tool_id = dlg->getSelectedToolId();
		cur_tool_type = dlg->getSelectedTool();
		tab_widgets = dlg->getTabWidgets();
		seq_widgets = dlg->getSeqWizards();
		depth_monitors = dlg->getDepthMonitors();
		info_bar_text = dlg->getInfoBarText();

		app_settings->setValue("Tool/Id", QVariant(tool_id));
		app_settings->setValue("Tool/Type", QVariant(cur_tool_type));
		app_settings->setValue("Tool/CfgFile", QVariant(cur_tool_file));

		if (!current_tool.file_name.isEmpty() /*&& tool_id > 0*/ )
		{			
			current_tool.file_name = cur_tool_file;
			current_tool.id = tool_id;
			current_tool.type = cur_tool_type;
			current_tool.tab_widgets = tab_widgets;
			current_tool.seq_wizards = seq_widgets;
			current_tool.depth_monitors = depth_monitors;
			current_tool.info_bar = info_bar_text;

			if (current_tool_settings) delete current_tool_settings;
			current_tool_settings = new QSettings(cur_tool_file, QSettings::IniFormat, this);
			setToolChannels(current_tool_settings);
		}
		else
		{
			int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot find info about current Tool!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}

		logging_widget->resetLoggingPlots(tool_channels);
		applyToolSettings();
	}			
	delete dlg;	
}

void MainWindow::applyToolSettings()
{
	/*
	switch (current_tool.id)
	{
	case KMRK:
		{
			bool isSDSP = false;
			for (int index = ui->tabWidget->count()-1; index >= 0; --index)
			{
				QWidget *widget = ui->tabWidget->widget(index);
				if (widget->objectName() == "tabSDSP") isSDSP = true;
			}
			if (!isSDSP) ui->tabWidget->addTab(ui->tabSDSP, tr("SDSP"));

			dock_expScheduler->setVisible(false);
			dock_sdspProc->setVisible(false);
			
			placeInfoToExpToolBar(tr("Logging Tool: KMRK"));
			break;
		}
	case NMKT:
		{
			for (int index = ui->tabWidget->count()-1; index >= 0; --index)
			{
				QWidget *widget = ui->tabWidget->widget(index);
				if (widget->objectName() == "tabSDSP")
				{
					ui->tabWidget->removeTab(index);
				}
			}

			dock_expScheduler->setVisible(false);

			placeInfoToExpToolBar(tr("Logging Tool: NMKT"));
			break;
		}
	case NMRKern:
		{
			for (int index = ui->tabWidget->count()-1; index >= 0; --index)
			{
				QWidget *widget = ui->tabWidget->widget(index);
				if (widget->objectName() == "tabSDSP")
				{
					ui->tabWidget->removeTab(index);
				}
			}

			dock_expScheduler->setVisible(true);


			placeInfoToExpToolBar(tr("Logging Tool: NMKT"));
			break;
		}
	default: 
		{
			// Включить/выключить SDSP виджет
			QString tool_name = current_tool.type;
			bool has_SDSP = false;
			for (int i = 0; i < tool_channels.count(); i++)
			{
				ToolChannel *channel = tool_channels[i];
				if (channel->data_type == "SDSP_CHANNEL") has_SDSP = true;
			}

			bool has_SDSP_widget = false;
			int SDSP_index = -1;
			for (int index = ui->tabWidget->count()-1; index >= 0; --index)
			{
				QWidget *widget = ui->tabWidget->widget(index);
				if (widget->objectName() == "tabSDSP") 
				{
					has_SDSP_widget = true;			
					SDSP_index = index;
				}
			}

			if (has_SDSP)
			{
				if (!has_SDSP_widget) ui->tabWidget->addTab(ui->tabSDSP, tr("SDSP"));
			}
			else 
			{
				if (has_SDSP_widget) ui->tabWidget->removeTab(SDSP_index);
			}

			placeInfoToExpToolBar(tr("Logging Tool: %1").arg(tool_name));
			break;
		}
	}
	*/

	dock_expScheduler->setVisible(false);
	dock_sequenceProc->setVisible(false);
	dock_sdspProc->setVisible(false);
	
	for (int index = ui->tabWidget->count()-1; index >= 0; --index)
	{
		QWidget *widget = ui->tabWidget->widget(index);
		QString tab_name = widget->objectName();
		if (tab_name == "tabSDSP" || tab_name == "SDSPWidget") ui->tabWidget->removeTab(index);
		if (tab_name == "tabMonitoring") ui->tabWidget->removeTab(index);	
		//if (tab_name == "tabOscilloscope") ui->tabWidget->removeTab(index);	
		//if (tab_name == "tabDataViewer") ui->tabWidget->removeTab(index);	
	}

	QStringList tab_widgets = current_tool.tab_widgets;
	//if (tab_widgets.contains(tr("Logging"))) ui->tabWidget->addTab(logging_widget, tr("Logging"));
	//if (tab_widgets.contains(tr("Oscilloscope"))) { ui->tabWidget->addTab(osc_widget, tr("Oscilloscope")); }
	//if (tab_widgets.contains(tr("DataPreview"))) ui->tabWidget->addTab(relax_widget, tr("DataPreview"));
	if (tab_widgets.contains(tr("SDSP"))) ui->tabWidget->addTab(sdsp_widget, tr("SDSP"));
	if (tab_widgets.contains(tr("Monitoring"))) ui->tabWidget->addTab(monitoring_widget, tr("Monitoring"));

	QStringList seq_wizards = current_tool.seq_wizards;
	dock_sequenceProc->setVisible(seq_wizards.contains("SequenceWizard"));
	dock_expScheduler->setVisible(seq_wizards.contains("ExperimentScheduler"));
	dock_sdspProc->setVisible(seq_wizards.contains("SDSPWizard"));

	QStringList dock_widgets = current_tool.dock_widgets;
	dock_msgConnect->setVisible(dock_widgets.contains("LoggingToolConsole"));
	dock_msgLog->setVisible(dock_widgets.contains("CommunicationLogMonitor"));
	dock_FreqAutoadjust->setVisible(dock_widgets.contains("FrequencyAutoTune"));
	dock_RxTxControl->setVisible(dock_widgets.contains("RxTxControl"));
	dock_RFPulseControl->setVisible(dock_widgets.contains("RFPulseControl"));

	placeInfoToExpToolBar(tr("Logging Tool: %1").arg(current_tool.info_bar));
}


void MainWindow::setToolId(unsigned char id)
{		
	if (!current_tool_was_applied /*&& nmrtoolLinker->getConnectionState() == ConnectionState::State_OK*/)
	{
		sendToolSettings();
		//return;
	}

	if (id == current_tool.id) return;
	if (id == 0) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Logging Tool with id = %1 was found. Bad Tool id!").arg(id), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	a_connect->setChecked(false);
	//nmrtoolLinker->startConnection(false);	
	//disconnect(COM_Port->COM_port, SIGNAL(readyRead()), com_commander, SLOT(onDataAvailable()));
	//nmrtool_tcp_settings.socket->disconnectFromHost();
	
	QList<ToolInfo> tools;	
	for (int i = 0; i < tools_settings.count(); i++)
	{		
		QSettings *settings = tools_settings[i];
		QString tool_file = settings->fileName();
		
		int uid = 0; 
		QString tool_type = "";
		bool ok = false;
		if (settings->contains("Tool/uid")) uid = settings->value("Tool/uid").toInt(&ok); 
		else uid = 0;
		if (settings->contains("Tool/type")) tool_type = settings->value("Tool/type").toString(); 
		
		if (ok && uid > 0 && !tool_type.isEmpty()) 
		{
			bool tool_exist = false;
			for (int j = 0; j < tools.count(); j++)
			{
				ToolInfo tool_info = tools[j];
				if (tool_info.id == uid) tool_exist = true;
			}

			if (!tool_exist)
			{
				ToolInfo tool_info(uid, tool_type, tool_file); 
				tools.append(tool_info);
			}
		}		
	}

	bool tool_exist = false;
	for (int i = 0; i < tools.count(); i++)
	{		
		ToolInfo tool_info = tools[i];
		int uid = tool_info.id;
		QString tool_type = tool_info.type;
		QString tool_file = tool_info.file_name;

		if (uid > 0 && !tool_type.isEmpty()) 
		{			
			if (uid == id)
			{
				QString msg_text = tr("Tool id = %1 [%2] was received from Logging Tool! Apply new id ?").arg(id).arg(tool_type);
				QMessageBox msgBox;
				msgBox.setText(tr("Warning!"));
				msgBox.setInformativeText(msg_text);
				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msgBox.setDefaultButton(QMessageBox::Yes);
				int ret = msgBox.exec();
				if (ret == QMessageBox::Yes)
				{
					//Нажата кнопка Yes
					current_tool.id = id;
					current_tool.file_name = tool_file;
					current_tool.type = tool_type;
					current_tool_was_applied = false;

					app_settings->setValue("Tool/Id", QVariant(current_tool.id));
					app_settings->setValue("Tool/Type", QVariant(current_tool.type));
					app_settings->setValue("Tool/CfgFile", QVariant(current_tool.file_name));

					if (current_tool_settings != NULL) delete current_tool_settings;
					current_tool_settings = new QSettings(current_tool.file_name, QSettings::IniFormat, this);
					setToolChannels(current_tool_settings);
					logging_widget->resetLoggingPlots(tool_channels);
					applyToolSettings();

					a_connect->setChecked(true);
					//nmrtoolLinker->startConnection(true);	

					tool_exist = true;
				}
				else
				{
					return;
				}

				/*current_tool.id = id;
				current_tool.file_name = tool_file;
				current_tool.type = tool_type;
				current_tool_was_applied = false;

				app_settings->setValue("Tool/Id", QVariant(current_tool.id));
				app_settings->setValue("Tool/Type", QVariant(current_tool.type));
				app_settings->setValue("Tool/CfgFile", QVariant(current_tool.file_name));

				if (current_tool_settings != NULL) delete current_tool_settings;
				current_tool_settings = new QSettings(current_tool.file_name, QSettings::IniFormat, this);
				setToolChannels(current_tool_settings);
				logging_widget->resetLoggingPlots(tool_channels);
				applyToolSettings();
								
				a_connect->setChecked(true);
				nmrtoolLinker->startConnection(true);	
								
				tool_exist = true;

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Tool id = %1 was received from Logging Tool! New id was applied.").arg(id), QMessageBox::Ok, QMessageBox::Ok);
				*/
			}			
		}		
	}

	if (!tool_exist)
	{
		ToolsDialog *dlg = new ToolsDialog(tools);
		if (id <= 0) 
		{
			dlg->setMessage(tr("Connected Logging Tool send incorrect id = %! Please, define a type of the Logging Tool:").arg(id));
		
			if (dlg->exec())
			{
				current_tool.id = dlg->getSelectedToolId();
				current_tool.type = dlg->getSelectedTool();
				current_tool.file_name = dlg->getSelectedToolFile();
				current_tool_was_applied = false;

				app_settings->setValue("Tool/Id", QVariant(current_tool.id));
				app_settings->setValue("Tool/Type", QVariant(current_tool.type));
				app_settings->setValue("Tool/CfgFile", QVariant(current_tool.file_name));

				if (current_tool_settings != NULL) delete current_tool_settings;
				current_tool_settings = new QSettings(current_tool.file_name, QSettings::IniFormat, this);
				setToolChannels(current_tool_settings);
				applyToolSettings();

				tool_exist = true;
			}
		
			delete dlg;		
		}
		else 
		{
			int ret = QMessageBox::warning(this, tr("Warning!"), tr("Logging Tool with id = %1 was found. Cannot find correct settings file for this tool in /Tools folder!").arg(id), QMessageBox::Ok, QMessageBox::Ok);
		}
	}

	
	//connect(COM_Port->COM_port, SIGNAL(readyRead()), com_commander, SLOT(onDataAvailable()));
	//connect(nmrtool_tcp_settings.socket, SIGNAL(readyRead()), com_commander, SLOT(onDataAvailable()));

	//if (tool_exist)
	//{
	//	sendToolSettings();
	//}
}


void MainWindow::loadAppSettings()
{
	app_settings = new QSettings("NMRTool_Communicator.ini", QSettings::IniFormat, this);

	bool ok = false;
	QString cur_tool_file = "";
	QString cur_tool_type = "";
	unsigned char tool_id = 0;
	bool file_settings_ok = false;
	if (app_settings->contains("Tool/Id") && app_settings->contains("Tool/CfgFile") && app_settings->contains("Tool/Type")) 
	{		
		tool_id = app_settings->value("Tool/Id").toInt(&ok);
		cur_tool_type = app_settings->value("Tool/Type").toString();
		cur_tool_file = app_settings->value("Tool/CfgFile").toString();
		
		QFile file(cur_tool_file);
		if(file.open(QIODevice::ReadOnly)) 
		{
			current_tool.file_name = cur_tool_file;
			current_tool.id = tool_id;
			current_tool.type = cur_tool_type;

			file_settings_ok = true;
		}
		file.close();		
	}
	//else
	if (!file_settings_ok)
	{
		QStringList tools_files;
		for (int i = 0; i < tools_settings.count(); i++)
		{
			QString file_name = tools_settings[i]->fileName();
			tools_files.append(file_name);
		}

		/////////////////////////////
		QList<ToolInfo> tools;
		for (int i = 0; i < tools_files.count(); i++)
		{
			QString tool_file = tools_files[i];
			QSettings *settings = tools_settings[i];

			int uid = 0; 
			QString tool_type = "";
			bool ok = false;
			if (settings->contains("Tool/uid")) uid = settings->value("Tool/uid").toInt(&ok); 
			else uid = 0;
			if (settings->contains("Tool/type")) tool_type = settings->value("Tool/type").toString(); 

			QStringList tab_widgets;
			if (settings->contains("VisualSettings/TabWidgets")) tab_widgets = settings->value("VisualSettings/TabWidgets").toStringList();
			QStringList seq_wizards;
			if (settings->contains("VisualSettings/SequenceWizards")) seq_wizards = settings->value("VisualSettings/SequenceWizards").toStringList();
			QStringList depth_monitors;
			if (settings->contains("VisualSettings/DepthMonitors")) depth_monitors = settings->value("VisualSettings/DepthMonitors").toStringList(); 
			QStringList dock_widgets;
			if (settings->contains("VisualSettings/DockWidgets")) dock_widgets = settings->value("VisualSettings/DockWidgets").toStringList(); 
			QString tool_info_bar;
			if (settings->contains("VisualSettings/InfoBar")) tool_info_bar = settings->value("VisualSettings/InfoBar").toString().simplified(); 

			if (ok && uid > 0 && !tool_type.isEmpty()) 
			{
				bool tool_exist = false;
				for (int j = 0; j < tools.count(); j++)
				{
					ToolInfo tool_info = tools[j];
					if (tool_info.id == uid) tool_exist = true;
				}

				if (!tool_exist)
				{
					ToolInfo tool_info(uid, tool_type, tool_file); 
					tool_info.tab_widgets = tab_widgets;
					tool_info.seq_wizards = seq_wizards;
					tool_info.depth_monitors = depth_monitors;
					tool_info.dock_widgets = dock_widgets;
					tool_info.info_bar = tool_info_bar;
					tools.append(tool_info);
				}
			}
		}
		/////////////////////////////
		QStringList tab_widgets;
		QStringList seq_widgets;
		QStringList depth_monitors;
		QStringList dock_widgets;
		QString info_bar_text;
		ToolsDialog *dlg = new ToolsDialog(tools);
		if (dlg->exec())
		{
			cur_tool_file = dlg->getSelectedToolFile();
			tool_id = dlg->getSelectedToolId();
			cur_tool_type = dlg->getSelectedTool();
			tab_widgets = dlg->getTabWidgets();
			seq_widgets = dlg->getSeqWizards();
			depth_monitors = dlg->getDepthMonitors();
			dock_widgets = dlg->getDockWidgets();
			info_bar_text = dlg->getInfoBarText();

			app_settings->setValue("Tool/Id", QVariant(tool_id));
			app_settings->setValue("Tool/Type", QVariant(cur_tool_type));
			app_settings->setValue("Tool/CfgFile", QVariant(cur_tool_file));

			current_tool.file_name = cur_tool_file;
			current_tool.id = tool_id;
			current_tool.type = cur_tool_type;
			current_tool.tab_widgets = tab_widgets;
			current_tool.seq_wizards = seq_widgets;
			current_tool.depth_monitors = depth_monitors;
			current_tool.dock_widgets = dock_widgets;
			current_tool.info_bar = info_bar_text;
		}			
		delete dlg;
	}

	if (!current_tool.file_name.isEmpty() && tool_id > 0)
	{
		current_tool_settings = new QSettings(cur_tool_file, QSettings::IniFormat, this);
		
		QStringList tab_widgets;
		if (current_tool_settings->contains("VisualSettings/TabWidgets")) tab_widgets = current_tool_settings->value("VisualSettings/TabWidgets").toStringList();
		QStringList seq_wizards;
		if (current_tool_settings->contains("VisualSettings/SequenceWizards")) seq_wizards = current_tool_settings->value("VisualSettings/SequenceWizards").toStringList();
		QStringList depth_monitors;
		if (current_tool_settings->contains("VisualSettings/DepthMonitors")) depth_monitors = current_tool_settings->value("VisualSettings/DepthMonitors").toStringList(); 
		QStringList dock_widgets;
		if (current_tool_settings->contains("VisualSettings/DockWidgets")) dock_widgets = current_tool_settings->value("VisualSettings/DockWidgets").toStringList(); 
		QString tool_info_bar;
		if (current_tool_settings->contains("VisualSettings/InfoBar")) tool_info_bar = current_tool_settings->value("VisualSettings/InfoBar").toString().simplified(); 
		
		current_tool.tab_widgets = tab_widgets;
		current_tool.seq_wizards = seq_wizards;
		current_tool.depth_monitors = depth_monitors;
		current_tool.dock_widgets = dock_widgets;
		current_tool.info_bar = tool_info_bar;

		setToolChannels(current_tool_settings);
	}
	else
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot find info about current Tool!"), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
}

void MainWindow::loadToolsSettings()
{
	if (!tools_settings.isEmpty()) 
	{
		qDeleteAll(tools_settings.begin(), tools_settings.end());
		tools_settings.clear();
	}

	QString path = QDir::currentPath() + "/Tools";
	QDir dir(path);	
	QStringList file_list = dir.entryList(QDir::Files | QDir::NoSymLinks);

	QStringList res;
	for (int i = 0; i < file_list.count(); i++)
	{
		if (file_list[i].split(".").last().toLower() == QString("cfg")) 
		{			
			QString file_name = path + "/" + file_list[i];
			QSettings *settings = new QSettings(file_name, QSettings::IniFormat);	

#ifdef Q_OS_WIN
			QString code_page = "Windows-1251";
#elif Q_OS_LINUX
			QString code_page = "UTF-8";
#endif
			QTextCodec *codec = QTextCodec::codecForName(code_page.toUtf8());
			settings->setIniCodec(codec);

			tools_settings.append(settings);
		}
	}
}

void MainWindow::sendToolSettings()
{
	// поле field_gradient не отправляется в DSP, т.к. совсем там не нужно.

	QVector<int> params;
	for (int i = 0; i < tool_channels.count(); i++)
	{
		ToolChannel *tool_channel = tool_channels[i];
		int channel_id = tool_channel->channel_id;
		bool _ok; int data_type = LoggingWidget::getDataTypeId(tool_channel->data_type, &_ok);
		int freq_set_num = tool_channel->frq_set_num;		
		int addr_rx = tool_channel->addr_rx;
		int addr_tx = tool_channel->addr_tx;
		int freq1 = tool_channel->frq1;
		int freq2 = tool_channel->frq2;
		int freq3 = tool_channel->frq3;
		int freq4 = tool_channel->frq4;
		int freq5 = tool_channel->frq5;
		int freq6 = tool_channel->frq6;

		params.append(data_type);
		params.append(channel_id);
		params.append(freq_set_num);
		params.append(addr_rx);
		params.append(addr_tx);
		params.append(freq1);
		params.append(freq2);
		params.append(freq3);
		params.append(freq4);
		params.append(freq5);
		params.append(freq6);
	}

	nmrtoolLinker->sendToolSettings(params);
}

void MainWindow::setToolChannels(QSettings *settings)
{
	if (settings == NULL) return;

	qDeleteAll(tool_channels.begin(), tool_channels.end());
	tool_channels.clear();

	bool finish = false;
	int i = 1;
	while (!finish)
	{
		QString channel_head = QString("channel#%1").arg(i++); 
		//QStringList child_groups = settings->childGroups();
		if (settings->childGroups().contains(channel_head))
		{
			bool ok;
			QString data_type = settings->value(channel_head + "/type").toString();
			uint8_t channel_id = settings->value(channel_head + "/channel_id").toInt(&ok);			if (!ok) channel_id = 0;
			uint8_t frq_set_num = settings->value(channel_head + "/frq_set_num").toInt(&ok);		if (!ok) frq_set_num = 0;
			QString channel_name = settings->value(channel_head + "/descp").toString();
			double depth_displ = settings->value(channel_head + "/depth_displ").toDouble(&ok);		if (!ok) depth_displ = 0;
			double norm_coef1 = settings->value(channel_head + "/normalize_coef1").toDouble(&ok);	if (!ok) norm_coef1 = 1;
			double norm_coef2 = settings->value(channel_head + "/normalize_coef2").toDouble(&ok);	if (!ok) norm_coef2 = 1;
			double meas_frq = settings->value(channel_head + "/meas_frq").toDouble(&ok);			if (!ok) meas_frq = 1;
			double sample_frq = settings->value(channel_head + "/sample_frq").toDouble(&ok);		if (!ok) sample_frq = 1;
			double field_gradient = settings->value(channel_head + "/field_gradient").toDouble(&ok);if (!ok) field_gradient = 0;
			uint32_t addr_rx = settings->value(channel_head + "/addr_rx").toInt(&ok);				if (!ok) addr_rx = 0;
			uint32_t addr_tx = settings->value(channel_head + "/addr_tx").toInt(&ok);				if (!ok) addr_tx = 0;				
			uint32_t frq1 = settings->value(channel_head + "/frq1").toInt(&ok);						if (!ok) frq1 = 0;	
			uint32_t frq2 = settings->value(channel_head + "/frq2").toInt(&ok);						if (!ok) frq2 = 0;
			uint32_t frq3 = settings->value(channel_head + "/frq3").toInt(&ok);						if (!ok) frq3 = 0;
			uint32_t frq4 = settings->value(channel_head + "/frq4").toInt(&ok);						if (!ok) frq4 = 0;
			uint32_t frq5 = settings->value(channel_head + "/frq5").toInt(&ok);						if (!ok) frq5 = 0;
			uint32_t frq6 = settings->value(channel_head + "/frq6").toInt(&ok);						if (!ok) frq6 = 0;
			QString axisx_title = settings->value(channel_head + "/axisX_title").toString();
			
			ToolChannel *channel = new ToolChannel(channel_id, data_type, frq_set_num, channel_name, depth_displ, norm_coef1, norm_coef2, meas_frq, sample_frq, field_gradient, addr_rx, addr_tx, frq1, frq2, frq3, frq4, frq5, frq6);
			tool_channels.append(channel);
		}
		else finish = true;		
	}
}


double MainWindow::getDepthDisplacement(uint8_t _channel_id, QVector<ToolChannel*> &_tool_channels)
{	
	double x = 0;
	for (int i = 0; i < _tool_channels.count(); i++)
	{
		if (_channel_id == _tool_channels[i]->channel_id) x = _tool_channels[i]->depth_displ;
	}	

	return x;
}

bool MainWindow::findSounds(QString &finish_sound)
{
	finish_sound = "";
	if (app_settings->contains("Sounds/FinishExperiment")) finish_sound = app_settings->value("Sounds/FinishExperiment").toString();
	
	QString path = QDir::currentPath() + "/Sounds";
	QDir dir(path);	
	QStringList file_list = dir.entryList(QDir::Files | QDir::NoSymLinks);	
	for (int i = 0; i < file_list.count(); i++)
	{
		file_list[i] = path + "/" + file_list[i];
	}
	for (int i = file_list.count()-1; i >= 0; --i)
	{

		if (file_list[i].split(".").last().toLower() != QString("wav")) 
		{
			file_list.removeAt(i);
		}
	}
	
	if (finish_sound.isEmpty() && !file_list.isEmpty()) finish_sound = file_list.first();
	
	if (!file_list.contains(finish_sound) && !file_list.isEmpty()) finish_sound = file_list.first();
	
	if (!finish_sound.isEmpty()) 
	{		
		app_settings->setValue("Sounds/FinishExperiment", finish_sound);
	}

	return !finish_sound.isEmpty();
}


/*void MainWindow::saveCOMSettings(COM_PORT *com_port, QString objName)
{	
	QString key_value = QString("%1/PortName").arg(objName);
	app_settings->setValue(key_value, com_port->COM_port->portName());

	key_value = QString("%1/BaudRate").arg(objName);
	app_settings->setValue(key_value, com_port->COM_Settings.BaudRate);
	
	key_value = QString("%1/DataBits").arg(objName);
	app_settings->setValue(key_value, com_port->COM_Settings.DataBits);

	key_value = QString("%1/Parity").arg(objName);
	app_settings->setValue(key_value, toString(com_port->COM_Settings.Parity));

	key_value = QString("%1/StopBits").arg(objName);
	app_settings->setValue(key_value, toString(com_port->COM_Settings.StopBits));

	key_value = QString("%1/FlowControl").arg(objName);
	app_settings->setValue(key_value, toString(com_port->COM_Settings.FlowControl));

	key_value = QString("%1/Timeout_ms").arg(objName);
	app_settings->setValue(key_value, com_port->COM_Settings.Timeout_Millisec);
}*/

void MainWindow::saveSocketSettings(TCP_Settings *tcp_settings, QString objName)
{
	QString key_value = QString("%1/IP_Address").arg(objName);
	app_settings->setValue(key_value, tcp_settings->ip_address);

	key_value = QString("%1/Port").arg(objName);
	app_settings->setValue(key_value, tcp_settings->port);

	app_settings->sync();
}

/*void MainWindow::initCOMSettings(COM_PORT *com_port)
{
	com_port->COM_port = 0;
	
	com_port->COM_Settings.BaudRate = BAUD19200;
	com_port->COM_Settings.DataBits = DATA_8;
	com_port->COM_Settings.Parity = PAR_NONE;
	com_port->COM_Settings.StopBits = STOP_1;
	com_port->COM_Settings.FlowControl = FLOW_OFF;
	com_port->COM_Settings.Timeout_Millisec = 0;
	
	com_port->connect_state = false;
	com_port->auto_search = false;
}

void MainWindow::initCOMSettings(COM_PORT *com_port, QString objName)
{
	bool _ok;
	QString key_value = QString("%1/BaudRate").arg(objName);
	if (app_settings->contains(key_value)) com_port->COM_Settings.BaudRate = (BaudRateType)app_settings->value(key_value).toInt(&_ok);
	else app_settings->setValue(key_value, BAUD9600);
	if (!_ok) 
	{
		com_port->COM_Settings.BaudRate = BAUD9600;
		app_settings->setValue(key_value, BAUD9600);
	}

	key_value = QString("%1/DataBits").arg(objName);
	if (app_settings->contains(key_value)) com_port->COM_Settings.DataBits = (DataBitsType)app_settings->value(key_value).toInt(&_ok);
	else app_settings->setValue(key_value, DATA_8);
	if (!_ok)
	{
		com_port->COM_Settings.DataBits = DATA_8;
		app_settings->setValue(key_value, DATA_8);
	}

	key_value = QString("%1/Parity").arg(objName);
	QString par = app_settings->value(key_value).toString();
	if (par == GET_STRING(PAR_NONE)) com_port->COM_Settings.Parity = PAR_NONE;
	else if (par == GET_STRING(PAR_ODD)) com_port->COM_Settings.Parity = PAR_ODD;
	else if (par == GET_STRING(PAR_EVEN)) com_port->COM_Settings.Parity = PAR_EVEN;
	else if (par == GET_STRING(PAR_MARK)) com_port->COM_Settings.Parity = PAR_MARK;
	else if (par == GET_STRING(PAR_SPACE)) com_port->COM_Settings.Parity = PAR_SPACE;
	else
	{
		com_port->COM_Settings.Parity = PAR_NONE;
		app_settings->setValue(key_value, toString(PAR_NONE));
	}

	key_value = QString("%1/StopBits").arg(objName);
	QString stbit = app_settings->value(key_value).toString();
	if (stbit == GET_STRING(STOP_1)) com_port->COM_Settings.StopBits = STOP_1;
	else if (stbit == GET_STRING(STOP_1_5)) com_port->COM_Settings.StopBits = STOP_1_5;
	else if (stbit == GET_STRING(STOP_2)) com_port->COM_Settings.StopBits = STOP_2;	
	else	
	{
		com_port->COM_Settings.StopBits = STOP_1;
		app_settings->setValue(key_value, toString(STOP_1));
	}

	key_value = QString("%1/FlowControl").arg(objName);
	QString flowc = app_settings->value(key_value).toString();
	if (flowc == GET_STRING(FLOW_OFF)) com_port->COM_Settings.FlowControl = FLOW_OFF;
	else if (flowc == GET_STRING(FLOW_HARDWARE)) com_port->COM_Settings.FlowControl = FLOW_HARDWARE;
	else if (flowc == GET_STRING(FLOW_XONXOFF)) com_port->COM_Settings.FlowControl = FLOW_XONXOFF;		
	else
	{
		com_port->COM_Settings.FlowControl = FLOW_OFF;
		app_settings->setValue(key_value, toString(FLOW_OFF));
	}

	key_value = QString("%1/Timeout_ms").arg(objName);
	if (app_settings->contains(key_value)) com_port->COM_Settings.Timeout_Millisec = (FlowType)app_settings->value(key_value).toInt(&_ok);
	else app_settings->setValue(key_value, 0);
	if (!_ok)
	{
		com_port->COM_Settings.Timeout_Millisec = 0;
		app_settings->setValue(key_value, 0);
	}

	com_port->connect_state = false;
	com_port->auto_search = false;
}
*/

void MainWindow::initNMRToolPortSettings()
{
	int nmrtool_port_id = 3004;
	QString nmrtool_ip_addr = "127.0.0.1";

	// Read IP Address	
	QString key_value = "NMRTool/IP_Address";
	if (app_settings->contains(key_value)) nmrtool_ip_addr = app_settings->value(key_value).toString();
	else app_settings->setValue(key_value, nmrtool_ip_addr);
	
	// Read TCP Port number
	bool _ok;
	key_value = "NMRTool/Port";
	if (app_settings->contains(key_value)) nmrtool_port_id = app_settings->value(key_value).toInt(&_ok);
	else app_settings->setValue(key_value, nmrtool_port_id);
	if (!_ok) 
	{		
		app_settings->setValue(key_value, nmrtool_port_id);
	}

	nmrtool_tcp_settings.socket = new QTcpSocket(this);
	nmrtool_tcp_settings.ip_address = nmrtool_ip_addr;
	nmrtool_tcp_settings.port = nmrtool_port_id;
}

void MainWindow::initDepthMeterPortSettings()
{
	int dmeter_port_id = 3006;
	QString dmeter_ip_addr = "127.0.0.1";

	// Read IP Address	
	QString key_value = "DepthMeter/IP_Address";
	if (app_settings->contains(key_value)) dmeter_ip_addr = app_settings->value(key_value).toString();
	else app_settings->setValue(key_value, QVariant(dmeter_ip_addr));

	// Read TCP Port number
	bool _ok;
	key_value = "DepthMeter/Port";
	if (app_settings->contains(key_value)) dmeter_port_id = app_settings->value(key_value).toInt(&_ok);
	else app_settings->setValue(key_value, dmeter_port_id);
	if (!_ok) 
	{		
		app_settings->setValue(key_value, QVariant(dmeter_port_id));
	}

	dmeter_tcp_settings.socket = new QTcpSocket(this);
	dmeter_tcp_settings.ip_address = dmeter_ip_addr;
	dmeter_tcp_settings.port = dmeter_port_id;
}

void MainWindow::initStepMotorPortSettings()
{
	int stmotor_port_id = 3008;
	QString stmotor_ip_addr = "127.0.0.1";

	// Read IP Address	
	QString key_value = "StepMotor/IP_Address";
	if (app_settings->contains(key_value)) stmotor_ip_addr = app_settings->value(key_value).toString();
	else app_settings->setValue(key_value, QVariant(stmotor_ip_addr));

	// Read TCP Port number
	bool _ok;
	key_value = "StepMotor/Port";
	if (app_settings->contains(key_value)) stmotor_port_id = app_settings->value(key_value).toInt(&_ok);
	else app_settings->setValue(key_value, stmotor_port_id);
	if (!_ok) 
	{		
		app_settings->setValue(key_value, QVariant(stmotor_port_id));
	}

	stmotor_tcp_settings.socket = new QTcpSocket(this);
	stmotor_tcp_settings.ip_address = stmotor_ip_addr;
	stmotor_tcp_settings.port = stmotor_port_id;
}

/*void MainWindow::connectToNMRToolSocket(QString ip_addr, int port)
{
	com_commander->connectToHost(ip_addr, port);
}

void MainWindow::disconnectFromNMRToolSocket()
{
	com_commander->disconnectFromHost();
}*/

/*
bool MainWindow::setupCOMPort(COM_PORT *com_port, QString _port_name)
{
	com_port->connect_state = false;

	QextSerialPort *port = new QextSerialPort(_port_name, com_port->COM_Settings);
	bool res = port->open(QextSerialPort::ReadWrite);
	if (res)
	{
		com_port->COM_port = port;
		com_port->COM_port->setPortName(_port_name);
		port->close();
		return true;
	}
	delete port;

	return false;
}
*/

/*
bool MainWindow::findAvailableCOMPort(COM_PORT *com_port)
{
	for (int i = 1; i <= 15; i++)
	{
		QString port_name = QString("COM%1").arg(i);
		QextSerialPort *port = new QextSerialPort(port_name, com_port->COM_Settings);
		bool res = port->open(QextSerialPort::ReadWrite);

		if (res)
		{
			com_port->COM_port = port;
			com_port->COM_port->setPortName(port_name);
			port->close();
			return true;
		}
		delete port;
	}

	return false;
}
*/

/*void MainWindow::setCOMPortSettings()
{
    COMPortDialog dlg(COM_Port->COM_port->portName(), COM_Port->COM_Settings, COM_Port->auto_search, this);
    if (dlg.exec())
    {
        COM_Port->COM_Settings = dlg.getPortSettings();
		COM_Port->COM_port->setPortName(dlg.getCOMPort());
		COM_Port->auto_search = dlg.getAutoSearchState();
    }
}*/

void MainWindow::setCommunicationSettings()
{
	CommunicationDialog dlg(comm_settings, this);
	if (dlg.exec())
	{
		//if (getCOMPort()->isOpen())
		if (nmrtool_tcp_settings.socket->isOpen())
		{
			Communication_Settings _comm_settings = dlg.getCommSettings();
			comm_settings->antinoise_coding = _comm_settings.antinoise_coding;
			comm_settings->block_length = _comm_settings.block_length;
			comm_settings->errs_count = _comm_settings.errs_count;
			comm_settings->interleaving = _comm_settings.interleaving;
			comm_settings->noise_control = _comm_settings.noise_control;
			comm_settings->packet_length = _comm_settings.packet_length;
			comm_settings->packlen_autoadjust = _comm_settings.packlen_autoadjust;
			comm_settings->packet_delay = _comm_settings.packet_delay;

			app_settings->setValue("CommSettings/PacketLenght", comm_settings->packet_length);
			app_settings->setValue("CommSettings/BlockLenght", comm_settings->block_length);
			app_settings->setValue("CommSettings/ErrCount", comm_settings->errs_count);
			app_settings->setValue("CommSettings/PacketDelay", comm_settings->packet_delay);
			app_settings->setValue("CommSettings/AntinoiseCoding", comm_settings->antinoise_coding);
			app_settings->setValue("CommSettings/PacketLengthAutoadjust", comm_settings->packlen_autoadjust);
			app_settings->setValue("CommSettings/Interleaving", comm_settings->interleaving);
			app_settings->setValue("CommSettings/NoiseControl", comm_settings->noise_control);

			QVector<uint8_t> comm_data;
			comm_data.push_back(comm_settings->packet_length);
			comm_data.push_back(comm_settings->block_length);
			comm_data.push_back(comm_settings->errs_count);
			uint8_t bools = 0;
			bools |= (uint8_t)comm_settings->packlen_autoadjust;
			bools |= ((uint8_t)comm_settings->antinoise_coding << 1);
			bools |= ((uint8_t)comm_settings->interleaving << 2);
			comm_data.push_back(bools);
			comm_data.push_back(comm_settings->packet_delay);

			setCmdResult(SET_COMM_PARAMS, ConnectionState::State_Connecting);	
			nmrtoolLinker->applyMsgCommParams(comm_data);
		}
		else QMessageBox::warning(this, tr("Warning!"), tr("COM-Port is closed!"), QMessageBox::Ok, QMessageBox::Ok);
	}
}

void MainWindow::setToolSettings()
{
	ToolSettingsSettingsDialog dlg(nmrtool_tcp_settings, dmeter_tcp_settings, stmotor_tcp_settings);
	if (dlg.exec())
	{
		nmrtool_tcp_settings = dlg.getNMRToolSettings();
		dmeter_tcp_settings = dlg.getDMeterSettings();
		stmotor_tcp_settings = dlg.getStMotorSettings();

		if (nmrtool_tcp_settings.socket->isOpen()) 
		{
			nmrtool_tcp_settings.socket->disconnectFromHost();
			nmrtool_tcp_settings.socket->connectToHost(nmrtool_tcp_settings.ip_address, nmrtool_tcp_settings.port);
		}

		if (dmeter_tcp_settings.socket->isOpen()) 
		{
			dmeter_tcp_settings.socket->disconnectFromHost();
			dmeter_tcp_settings.socket->connectToHost(dmeter_tcp_settings.ip_address, dmeter_tcp_settings.port);
		}

		if (stmotor_tcp_settings.socket->isOpen()) 
		{
			stmotor_tcp_settings.socket->disconnectFromHost();
			stmotor_tcp_settings.socket->connectToHost(stmotor_tcp_settings.ip_address, stmotor_tcp_settings.port);
		}
				
		app_settings->setValue("NMRTool/IP_Address", nmrtool_tcp_settings.ip_address);
		app_settings->setValue("NMRTool/Port", nmrtool_tcp_settings.port);

		app_settings->setValue("DepthMeter/IP_Address", dmeter_tcp_settings.ip_address);
		app_settings->setValue("DepthMeter/Port", dmeter_tcp_settings.port);

		app_settings->setValue("StepMotor/IP_Address", stmotor_tcp_settings.ip_address);
		app_settings->setValue("StepMotor/Port", stmotor_tcp_settings.port);
	}
}

/*
void MainWindow::setTCPConnectionSettings()
{
	TcpServerDialog dlg(tcp_data_manager->getTcpCommunicator()->getPort(), this);
	if (dlg.exec())
	{
		tcp_data_manager->resetTcpCommunicator(dlg.getPort());
	}
}
*/

/*
void MainWindow::setDepthMeterSettings()
{
	COMPortDialog dlg(COM_Port_depth->COM_port->portName(), COM_Port_depth->COM_Settings, false, this);
	dlg.setTitle(tr("Set COM-Port Settings for Depth Meter"));
	dlg.enableAutoSearch(false);
	if (dlg.exec())
	{
		COM_Port_depth->COM_Settings = dlg.getPortSettings();		
		COM_Port_depth->auto_search = dlg.getAutoSearchState();

		QextSerialPort *com_port = COM_Port_depth->COM_port;
		PortSettings port_settings = COM_Port_depth->COM_Settings;
		com_port->setPortName(dlg.getCOMPort());
		com_port->setBaudRate(port_settings.BaudRate);
		com_port->setDataBits(port_settings.DataBits);
		com_port->setParity(port_settings.Parity);
		//com_port->setFlowControl(port_settings.FlowControl);
		com_port->setStopBits(port_settings.StopBits);		

		//saveCOMSettings(COM_Port_depth, "DepthMeter");
	}
}
*/

/*
void MainWindow::setStepMotorCOMSettings()
{
	COMPortDialog dlg(COM_Port_stepmotor->COM_port->portName(), COM_Port_stepmotor->COM_Settings, false, this);
	dlg.setTitle(tr("Set COM-Port Settings for Step Motor control board"));
	dlg.enableAutoSearch(false);
	if (dlg.exec())
	{
		COM_Port_stepmotor->COM_Settings = dlg.getPortSettings();		
		COM_Port_stepmotor->auto_search = dlg.getAutoSearchState();

		QextSerialPort *com_port = COM_Port_stepmotor->COM_port;
		PortSettings port_settings = COM_Port_stepmotor->COM_Settings;
		com_port->setPortName(dlg.getCOMPort());
		com_port->setBaudRate(port_settings.BaudRate);
		com_port->setDataBits(port_settings.DataBits);
		com_port->setParity(port_settings.Parity);		
		com_port->setStopBits(port_settings.StopBits);		

		//saveCOMSettings(COM_Port_stepmotor, "StepMotor");
	}
}
*/

void MainWindow::setProcessingSettings()
{
	ProcessingSettingsDialog dlg(processing_relax);
	if (dlg.exec())
	{		
		processing_relax.smoothing_number = dlg.getSmoothingNumber();
		processing_relax.extrapolation_number = dlg.getExtrapolationNumber();
		processing_relax.alpha = dlg.getAlpha();
		processing_relax.is_smoothed = dlg.isSmoothed();
		processing_relax.is_centered = dlg.isCentered();
		processing_relax.is_extrapolated = dlg.isExtrapolated();
		processing_relax.is_quality_controlled = dlg.isQualityControlled();

		processing_relax.I = dlg.getI();
		processing_relax.K = dlg.getK();

		processing_relax.T2_from = dlg.getT2_from();
		processing_relax.T2_to = dlg.getT2_to();
		processing_relax.T2_min = dlg.getT2_min();
		processing_relax.T2_max = dlg.getT2_max();
		processing_relax.T2_cutoff = dlg.getT2_cutoff();
		processing_relax.T2_cutoff_clay = dlg.getT2_cutoff_clay();
		processing_relax.T2_points = dlg.getT2_points();
		processing_relax.iters = dlg.getIterations();
		processing_relax.regulParam = processing_relax.T2_points;
		processing_relax.MCBWcolor = dlg.getMCBWcolor();
		processing_relax.MBVIcolor = dlg.getMBVIcolor();
		processing_relax.MFFIcolor = dlg.getMFFIcolor();
		processing_relax.porosity_on = dlg.isPorosityOn();

		QwtPlot *spectrum_plot = relax_widget->getRelaxMathPlot()->getPlot();
		spectrum_plot->setAxisScale(QwtPlot::xBottom, processing_relax.T2_from, processing_relax.T2_to);
		spectrum_plot->replot();
	}
}

void MainWindow::setSequenceChanged()
{
	setSequenceStatus(SeqStatus::Seq_Changed);
}

void MainWindow::initSaveDataAttrs()
{	
	if (app_settings->contains("SaveDataSettings/Path")) save_data_attrs.path = app_settings->value("SaveDataSettings/Path").toString();
	else
	{
		QString cur_dir = QCoreApplication::applicationDirPath() + "/UserData";		
		save_data_attrs.path = cur_dir;
		app_settings->setValue("SaveDataSettings/Path", save_data_attrs.path);
	}
			
	if (app_settings->contains("SaveDataSettings/Prefix")) save_data_attrs.prefix = app_settings->value("SaveDataSettings/Prefix").toString();
	else
	{
		save_data_attrs.prefix = "data";
		app_settings->setValue("SaveDataSettings/Prefix", save_data_attrs.prefix);
	}	
		
	if (app_settings->contains("SaveDataSettings/Postfix")) save_data_attrs.postfix = app_settings->value("SaveDataSettings/Postfix").toString();
	else
	{
		save_data_attrs.postfix = "";
		app_settings->setValue("SaveDataSettings/Postfix", save_data_attrs.postfix);
	}
		
	if (app_settings->contains("SaveDataSettings/Extension")) save_data_attrs.extension = app_settings->value("SaveDataSettings/Extension").toString();
	else
	{
		save_data_attrs.extension = "dat";
		app_settings->setValue("SaveDataSettings/Extension", save_data_attrs.extension);
	}
	
	/*if (app_settings->contains("SaveDataSettings/SaveData")) save_data_attrs.to_save = app_settings->value("SaveDataSettings/Path").toBool();
	else
	{		
		save_data_attrs.to_save = false;
		app_settings->setValue("SaveDataSettings/SaveData", save_data_attrs.to_save);
	}*/
	save_data_attrs.to_save = true;
	save_data_attrs.to_save_all = false;

	//save_data_file = NULL;
	//start_data_export = false;		// по умолчанию не экспортировать измеренные данные	
	experiment_id = 0;
}

void MainWindow::initExperimentSettings()
{	
	if (app_settings->contains("ExperimentSettings/WellUID")) exper_attrs.well_UID = app_settings->value("ExperimentSettings/WellUID").toString();
	else 
	{
		exper_attrs.well_UID = "";
		app_settings->setValue("ExperimentSettings/WellUID", exper_attrs.well_UID);
	}

	if (app_settings->contains("ExperimentSettings/WellName")) exper_attrs.well_name = app_settings->value("ExperimentSettings/WellName").toString();
	else 
	{
		exper_attrs.well_name = "";
		app_settings->setValue("ExperimentSettings/WellName", exper_attrs.well_name);
	}
	
	if (app_settings->contains("ExperimentSettings/Field")) exper_attrs.field_name = app_settings->value("ExperimentSettings/Field").toString();
	else 
	{
		exper_attrs.field_name = "";
		app_settings->setValue("ExperimentSettings/Field", exper_attrs.field_name);
	}
	
	if (app_settings->contains("ExperimentSettings/Location")) exper_attrs.location = app_settings->value("ExperimentSettings/Location").toString();
	else 
	{
		exper_attrs.location = "";
		app_settings->setValue("ExperimentSettings/Location", exper_attrs.location);
	}
	
	if (app_settings->contains("ExperimentSettings/Province")) exper_attrs.province = app_settings->value("ExperimentSettings/Province").toString();
	else 
	{
		exper_attrs.province = "";
		app_settings->setValue("ExperimentSettings/Province", exper_attrs.province);
	}

	if (app_settings->contains("ExperimentSettings/Country")) exper_attrs.country = app_settings->value("ExperimentSettings/Country").toString();
	else 
	{
		exper_attrs.country = "Russia";
		app_settings->setValue("ExperimentSettings/Country", exper_attrs.country);
	}
	
	if (app_settings->contains("ExperimentSettings/ServiceCompany")) exper_attrs.service_company = app_settings->value("ExperimentSettings/ServiceCompany").toString();
	else 
	{
		exper_attrs.service_company = "TNG group";
		app_settings->setValue("ExperimentSettings/ServiceCompany", exper_attrs.service_company);
	}

	if (app_settings->contains("ExperimentSettings/Company")) exper_attrs.company = app_settings->value("ExperimentSettings/Company").toString();
	else 
	{
		exper_attrs.company = "";
		app_settings->setValue("ExperimentSettings/Company", exper_attrs.company);
	}

	if (app_settings->contains("ExperimentSettings/Operator")) exper_attrs.oper = app_settings->value("ExperimentSettings/Operator").toString();
	else 
	{
		exper_attrs.oper = "";
		app_settings->setValue("ExperimentSettings/Operator", exper_attrs.oper);
	}
	
	if (app_settings->contains("Tool/Type")) exper_attrs.tool = app_settings->value("Tool/Type").toString();
	else exper_attrs.tool = "";
		
	exper_attrs.date = QDate::currentDate();	

	if (app_settings->contains("ExperimentSettings/DontShowAgain")) exper_attrs.dont_show_again = app_settings->value("ExperimentSettings/DontShowAgain").toBool();
	else 
	{
		exper_attrs.dont_show_again = false;
		app_settings->setValue("ExperimentSettings/DontShowAgain", exper_attrs.dont_show_again);
	}
	
}

void MainWindow::initDataTypes()
{
	data_type_list_Oil.append(DataType(DT_NS_FID_ORG, GET_STRING(DT_NS_FID_ORG), false, "FID noise data from ADC (UINT16)"));
	data_type_list_Oil.append(DataType(DT_NS_SE_ORG, GET_STRING(DT_NS_SE_ORG), false, "Spin-echo noise data from ADC (UINT16)"));
	data_type_list_Oil.append(DataType(DT_SGN_FID_ORG, GET_STRING(DT_SGN_FID_ORG), false, "FID signal data from ADC (UINT16)"));
	data_type_list_Oil.append(DataType(DT_SGN_SE_ORG, GET_STRING(DT_SGN_SE_ORG), false, "Spin-echo signal data from ADC (UINT16)"));
	
	data_type_list_Oil.append(DataType(DT_NS_FID, GET_STRING(DT_NS_FID), false, "FID noise data"));
	data_type_list_Oil.append(DataType(DT_NS_SE, GET_STRING(DT_NS_SE), false, "Spin-echo noise data"));
	data_type_list_Oil.append(DataType(DT_SGN_FID, GET_STRING(DT_SGN_FID), false, "FID signal data"));
	data_type_list_Oil.append(DataType(DT_SGN_SE, GET_STRING(DT_SGN_SE), false, "Spin-echo signal data"));

	data_type_list_Oil.append(DataType(DT_NS_QUAD_FID_RE, GET_STRING(DT_NS_QUAD_FID_RE), false, "FID noise data quad.-detected (Re)"));
	data_type_list_Oil.append(DataType(DT_NS_QUAD_FID_IM, GET_STRING(DT_NS_QUAD_FID_IM), false, "FID noise data quad.-detected (Im)"));
	data_type_list_Oil.append(DataType(DT_NS_QUAD_SE_RE, GET_STRING(DT_NS_QUAD_SE_RE), false, "Spin-echo noise data quad.-detected (Re)"));
	data_type_list_Oil.append(DataType(DT_NS_QUAD_SE_IM, GET_STRING(DT_NS_QUAD_SE_IM), false, "Spin-echo noise data quad.-detected (Im)"));
	data_type_list_Oil.append(DataType(DT_SGN_QUAD_FID_RE, GET_STRING(DT_SGN_QUAD_FID_RE), false, "FID signal data quad.-detected (Re)"));
	data_type_list_Oil.append(DataType(DT_SGN_QUAD_FID_IM, GET_STRING(DT_SGN_QUAD_FID_IM), false, "FID signal data quad.-detected (Im)"));
	data_type_list_Oil.append(DataType(DT_SGN_QUAD_SE_RE, GET_STRING(DT_SGN_QUAD_SE_RE), false, "Spin-echo signal data quad.-detected (Re)"));
	data_type_list_Oil.append(DataType(DT_SGN_QUAD_SE_IM, GET_STRING(DT_SGN_QUAD_SE_IM), false, "Spin-echo signal data quad.-detected (Im)"));
	data_type_list_Oil.append(DataType(DT_NS_QUAD_FID, GET_STRING(DT_NS_QUAD_FID), false, "FID noise data quad.-detected (Amplitude)"));
	data_type_list_Oil.append(DataType(DT_NS_QUAD_SE, GET_STRING(DT_NS_QUAD_SE), false, "Spin-echo noise data quad.-detected (Amplitude)"));
	data_type_list_Oil.append(DataType(DT_SGN_QUAD_FID, GET_STRING(DT_SGN_QUAD_FID), false, "FID signal data quad.-detected (Amplitude)"));
	data_type_list_Oil.append(DataType(DT_SGN_QUAD_SE, GET_STRING(DT_SGN_QUAD_SE), false, "Spin-echo signal data quad.-detected (Amplitude)"));

	data_type_list_Oil.append(DataType(DT_NS_FFT_FID_RE, GET_STRING(DT_NS_FFT_FID_RE), false, "Fourier spectrum of FID noise data (Re)"));
	data_type_list_Oil.append(DataType(DT_NS_FFT_SE_RE, GET_STRING(DT_NS_FFT_SE_RE), false, "Fourier spectrum of spin-echo noise data (Re)"));
	data_type_list_Oil.append(DataType(DT_SGN_FFT_FID_RE, GET_STRING(DT_SGN_FFT_FID_RE), false, "Fourier spectrum of FID signal data (Re)"));
	data_type_list_Oil.append(DataType(DT_SGN_FFT_SE_RE, GET_STRING(DT_SGN_FFT_SE_RE), false, "Fourier spectrum of spin-echo signal data (Re)"));	
	data_type_list_Oil.append(DataType(DT_NS_FFT_FID_IM, GET_STRING(DT_NS_FFT_FID_IM), false, "Fourier spectrum of FID noise data (Im)"));
	data_type_list_Oil.append(DataType(DT_NS_FFT_SE_IM, GET_STRING(DT_NS_FFT_SE_IM), false, "Fourier spectrum of spin-echo noise data (Im)"));
	data_type_list_Oil.append(DataType(DT_SGN_FFT_FID_IM, GET_STRING(DT_SGN_FFT_FID_IM), false, "Fourier spectrum of FID signal data (Im)"));
	data_type_list_Oil.append(DataType(DT_SGN_FFT_SE_IM, GET_STRING(DT_SGN_FFT_SE_IM), false, "Fourier spectrum of spin-echo signal data (Im)"));	
	data_type_list_Oil.append(DataType(DT_NS_FFT_FID_AM, GET_STRING(DT_NS_FFT_FID_AM), false, "Fourier spectrum of FID noise data (Amplitude)"));
	data_type_list_Oil.append(DataType(DT_NS_FFT_SE_AM, GET_STRING(DT_NS_FFT_SE_AM), false, "Fourier spectrum of spin-echo noise data (Amplitude)"));
	data_type_list_Oil.append(DataType(DT_SGN_FFT_FID_AM, GET_STRING(DT_SGN_FFT_FID_AM), false, "Fourier spectrum of FID signal data (Amplitude)"));
	data_type_list_Oil.append(DataType(DT_SGN_FFT_SE_AM, GET_STRING(DT_SGN_FFT_SE_AM), false, "Fourier spectrum of spin-echo signal data (Amplitude)"));	
	
	data_type_list_Oil.append(DataType(DT_SGN_POWER_SE, GET_STRING(DT_SGN_POWER_SE), false, "Power spectrum of spin-echo signal data"));
	data_type_list_Oil.append(DataType(DT_SGN_POWER_FID, GET_STRING(DT_SGN_POWER_FID), false, "Power spectrum of FID signal data"));
	data_type_list_Oil.append(DataType(DT_NS_POWER_SE, GET_STRING(DT_NS_POWER_SE), false, "Power spectrum of spin-echo noise data"));
	data_type_list_Oil.append(DataType(DT_NS_POWER_FID, GET_STRING(DT_NS_POWER_FID), false, "Power spectrum of FID signal data"));

	data_type_list_Oil.append(DataType(DT_SGN_RELAX, GET_STRING(DT_SGN_RELAX), false, "Relaxation data #1"));
	data_type_list_Oil.append(DataType(DT_SGN_RELAX2, GET_STRING(DT_SGN_RELAX2), false, "Relaxation data #2"));
	data_type_list_Oil.append(DataType(DT_SGN_RELAX3, GET_STRING(DT_SGN_RELAX3), false, "Relaxation data #3"));
	data_type_list_Oil.append(DataType(DT_AVER_RELAX, GET_STRING(DT_AVER_RELAX), false, "Averaged Relaxation data (Moving averaging) #1"));
	data_type_list_Oil.append(DataType(DT_AVER_RELAX2, GET_STRING(DT_AVER_RELAX2), false, "Averaged Relaxation data (Moving averaging) #2"));
	data_type_list_Oil.append(DataType(DT_AVER_RELAX3, GET_STRING(DT_AVER_RELAX3), false, "Averaged Relaxation data (Moving averaging) #3"));
	
	data_type_list_Oil.append(DataType(DT_GAMMA, GET_STRING(DT_GAMMA), false, "Data of gamma-logging"));
	
	data_type_list_Oil.append(DataType(DT_DIEL, GET_STRING(DT_DIEL), false, "Data of SDSP-logging"));

	data_type_list_Oil.append(DataType(DT_AFR1_RX, GET_STRING(DT_AFR1_RX), false, "Amplitude Frequency Response Rx #1"));
	data_type_list_Oil.append(DataType(DT_AFR2_RX, GET_STRING(DT_AFR2_RX), false, "Amplitude Frequency Response Rx #2"));
	data_type_list_Oil.append(DataType(DT_AFR3_RX, GET_STRING(DT_AFR3_RX), false, "Amplitude Frequency Response Rx #3"));
	data_type_list_Oil.append(DataType(DT_AFR1_TX, GET_STRING(DT_AFR1_TX), false, "Amplitude Frequency Response Tx #1"));
	data_type_list_Oil.append(DataType(DT_AFR2_TX, GET_STRING(DT_AFR2_TX), false, "Amplitude Frequency Response Tx #2"));
	data_type_list_Oil.append(DataType(DT_AFR3_TX, GET_STRING(DT_AFR3_TX), false, "Amplitude Frequency Response Tx #3"));
	
	data_type_list_Oil.append(DataType(DT_RFP, GET_STRING(DT_RFP), false, "RF-Pulse Response"));
	data_type_list_Oil.append(DataType(DT_RFP2, GET_STRING(DT_RFP2), false, "RF-Pulse Response"));

	data_type_list_Oil.append(DataType(DT_DU_T, GET_STRING(DT_DU_T), false, "Temperature data from DU-board"));
	data_type_list_Oil.append(DataType(DT_TU_T, GET_STRING(DT_TU_T), false, "Temperature data from TU-board"));
	data_type_list_Oil.append(DataType(DT_PU_T, GET_STRING(DT_PU_T), false, "Temperature data from PU-board"));

	for (int i = 0; i < data_type_list_Oil.count(); i++)
	{
		DataType dt = data_type_list_Oil[i];
		bool state = getExportingStateToOil(dt);
		data_type_list_Oil[i].flag = state;
	}


	data_type_list_File.append(DataType(DT_NS_FID_ORG, GET_STRING(DT_NS_FID_ORG), false, "FID noise data from ADC (UINT16)"));
	data_type_list_File.append(DataType(DT_NS_SE_ORG, GET_STRING(DT_NS_SE_ORG), false, "Spin-echo noise data from ADC (UINT16)"));
	data_type_list_File.append(DataType(DT_SGN_FID_ORG, GET_STRING(DT_SGN_FID_ORG), false, "FID signal data from ADC (UINT16)"));
	data_type_list_File.append(DataType(DT_SGN_SE_ORG, GET_STRING(DT_SGN_SE_ORG), false, "Spin-echo signal data from ADC (UINT16)"));

	data_type_list_File.append(DataType(DT_NS_FID, GET_STRING(DT_NS_FID), false, "FID noise data"));
	data_type_list_File.append(DataType(DT_NS_SE, GET_STRING(DT_NS_SE), false, "Spin-echo noise data"));
	data_type_list_File.append(DataType(DT_SGN_FID, GET_STRING(DT_SGN_FID), false, "FID signal data"));
	data_type_list_File.append(DataType(DT_SGN_SE, GET_STRING(DT_SGN_SE), false, "Spin-echo signal data"));

	data_type_list_File.append(DataType(DT_NS_QUAD_FID_RE, GET_STRING(DT_NS_QUAD_FID_RE), false, "FID noise data quad.-detected (Re)"));
	data_type_list_File.append(DataType(DT_NS_QUAD_FID_IM, GET_STRING(DT_NS_QUAD_FID_IM), false, "FID noise data quad.-detected (Im)"));
	data_type_list_File.append(DataType(DT_NS_QUAD_SE_RE, GET_STRING(DT_NS_QUAD_SE_RE), false, "Spin-echo noise data quad.-detected (Re)"));
	data_type_list_File.append(DataType(DT_NS_QUAD_SE_IM, GET_STRING(DT_NS_QUAD_SE_IM), false, "Spin-echo noise data quad.-detected (Im)"));
	data_type_list_File.append(DataType(DT_SGN_QUAD_FID_RE, GET_STRING(DT_SGN_QUAD_FID_RE), false, "FID signal data quad.-detected (Re)"));
	data_type_list_File.append(DataType(DT_SGN_QUAD_FID_IM, GET_STRING(DT_SGN_QUAD_FID_IM), false, "FID signal data quad.-detected (Im)"));
	data_type_list_File.append(DataType(DT_SGN_QUAD_SE_RE, GET_STRING(DT_SGN_QUAD_SE_RE), false, "Spin-echo signal data quad.-detected (Re)"));
	data_type_list_File.append(DataType(DT_SGN_QUAD_SE_IM, GET_STRING(DT_SGN_QUAD_SE_IM), false, "Spin-echo signal data quad.-detected (Im)"));
	data_type_list_File.append(DataType(DT_NS_QUAD_FID, GET_STRING(DT_NS_QUAD_FID), false, "FID noise data quad.-detected (Amplitude)"));
	data_type_list_File.append(DataType(DT_NS_QUAD_SE, GET_STRING(DT_NS_QUAD_SE), false, "Spin-echo noise data quad.-detected (Amplitude)"));
	data_type_list_File.append(DataType(DT_SGN_QUAD_FID, GET_STRING(DT_SGN_QUAD_FID), false, "FID signal data quad.-detected (Amplitude)"));
	data_type_list_File.append(DataType(DT_SGN_QUAD_SE, GET_STRING(DT_SGN_QUAD_SE), false, "Spin-echo signal data quad.-detected (Amplitude)"));

	data_type_list_File.append(DataType(DT_NS_FFT_FID_RE, GET_STRING(DT_NS_FFT_FID_RE), false, "Fourier spectrum of FID noise data (Re)"));
	data_type_list_File.append(DataType(DT_NS_FFT_SE_RE, GET_STRING(DT_NS_FFT_SE_RE), false, "Fourier spectrum of spin-echo noise data (Re)"));
	data_type_list_File.append(DataType(DT_SGN_FFT_FID_RE, GET_STRING(DT_SGN_FFT_FID_RE), false, "Fourier spectrum of FID signal data (Re)"));
	data_type_list_File.append(DataType(DT_SGN_FFT_SE_RE, GET_STRING(DT_SGN_FFT_SE_RE), false, "Fourier spectrum of spin-echo signal data (Re)"));	
	data_type_list_File.append(DataType(DT_NS_FFT_FID_IM, GET_STRING(DT_NS_FFT_FID_IM), false, "Fourier spectrum of FID noise data (Im)"));
	data_type_list_File.append(DataType(DT_NS_FFT_SE_IM, GET_STRING(DT_NS_FFT_SE_IM), false, "Fourier spectrum of spin-echo noise data (Im)"));
	data_type_list_File.append(DataType(DT_SGN_FFT_FID_IM, GET_STRING(DT_SGN_FFT_FID_IM), false, "Fourier spectrum of FID signal data (Im)"));
	data_type_list_File.append(DataType(DT_SGN_FFT_SE_IM, GET_STRING(DT_SGN_FFT_SE_IM), false, "Fourier spectrum of spin-echo signal data (Im)"));	
	data_type_list_File.append(DataType(DT_NS_FFT_FID_AM, GET_STRING(DT_NS_FFT_FID_AM), false, "Fourier spectrum of FID noise data (Amplitude)"));
	data_type_list_File.append(DataType(DT_NS_FFT_SE_AM, GET_STRING(DT_NS_FFT_SE_AM), false, "Fourier spectrum of spin-echo noise data (Amplitude)"));
	data_type_list_File.append(DataType(DT_SGN_FFT_FID_AM, GET_STRING(DT_SGN_FFT_FID_AM), false, "Fourier spectrum of FID signal data (Amplitude)"));
	data_type_list_File.append(DataType(DT_SGN_FFT_SE_AM, GET_STRING(DT_SGN_FFT_SE_AM), false, "Fourier spectrum of spin-echo signal data (Amplitude)"));	

	data_type_list_File.append(DataType(DT_SGN_POWER_SE, GET_STRING(DT_SGN_POWER_SE), false, "Power spectrum of spin-echo signal data"));
	data_type_list_File.append(DataType(DT_SGN_POWER_FID, GET_STRING(DT_SGN_POWER_FID), false, "Power spectrum of FID signal data"));
	data_type_list_File.append(DataType(DT_NS_POWER_SE, GET_STRING(DT_NS_POWER_SE), false, "Power spectrum of spin-echo noise data"));
	data_type_list_File.append(DataType(DT_NS_POWER_FID, GET_STRING(DT_NS_POWER_FID), false, "Power spectrum of FID signal data"));

	data_type_list_File.append(DataType(DT_SGN_RELAX, GET_STRING(DT_SGN_RELAX), false, "Relaxation data #1"));
	data_type_list_File.append(DataType(DT_SGN_RELAX2, GET_STRING(DT_SGN_RELAX2), false, "Relaxation data #2"));
	data_type_list_File.append(DataType(DT_SGN_RELAX3, GET_STRING(DT_SGN_RELAX3), false, "Relaxation data #3"));
	data_type_list_File.append(DataType(DT_AVER_RELAX, GET_STRING(DT_AVER_RELAX), false, "Averaged Relaxation data (Moving averaging) #1"));
	data_type_list_File.append(DataType(DT_AVER_RELAX2, GET_STRING(DT_AVER_RELAX2), false, "Averaged Relaxation data (Moving averaging) #2"));
	data_type_list_File.append(DataType(DT_AVER_RELAX3, GET_STRING(DT_AVER_RELAX3), false, "Averaged Relaxation data (Moving averaging) #3"));
	data_type_list_File.append(DataType(DT_SOLID_ECHO, GET_STRING(DT_SOLID_ECHO), false, "Solid Echo data #1"));	
	data_type_list_File.append(DataType(DT_T1T2_NMR, GET_STRING(DT_T1T2_NMR), false, "T1T2 2D-NMR data #2"));
	data_type_list_File.append(DataType(DT_DsT2_NMR, GET_STRING(DT_DsT2_NMR), false, "DsT2 2D-NMR data #3"));

	data_type_list_File.append(DataType(DT_GAMMA, GET_STRING(DT_GAMMA), false, "Data of gamma-logging"));

	data_type_list_File.append(DataType(DT_DIEL, GET_STRING(DT_DIEL), false, "Data of SDSP-logging"));

	data_type_list_File.append(DataType(DT_AFR1_RX, GET_STRING(DT_AFR1_RX), false, "Amplitude Frequency Response Rx #1"));
	data_type_list_File.append(DataType(DT_AFR2_RX, GET_STRING(DT_AFR2_RX), false, "Amplitude Frequency Response Rx #2"));
	data_type_list_File.append(DataType(DT_AFR3_RX, GET_STRING(DT_AFR3_RX), false, "Amplitude Frequency Response Rx #3"));
	data_type_list_File.append(DataType(DT_AFR1_TX, GET_STRING(DT_AFR1_TX), false, "Amplitude Frequency Response Tx #1"));
	data_type_list_File.append(DataType(DT_AFR2_TX, GET_STRING(DT_AFR2_TX), false, "Amplitude Frequency Response Tx #2"));
	data_type_list_File.append(DataType(DT_AFR3_TX, GET_STRING(DT_AFR3_TX), false, "Amplitude Frequency Response Tx #3"));
	data_type_list_File.append(DataType(DT_FREQ_TUNE, GET_STRING(DT_FREQ_TUNE), false, "Frequency Autotune"));

	data_type_list_File.append(DataType(DT_RFP, GET_STRING(DT_RFP), false, "RF-Pulse Response #1"));
	data_type_list_File.append(DataType(DT_RFP2, GET_STRING(DT_RFP2), false, "RF-Pulse Response #2"));

	data_type_list_File.append(DataType(DT_DU_T, GET_STRING(DT_DU_T), false, "Temperature data from DU-board"));
	data_type_list_File.append(DataType(DT_TU_T, GET_STRING(DT_TU_T), false, "Temperature data from TU-board"));
	data_type_list_File.append(DataType(DT_PU_T, GET_STRING(DT_PU_T), false, "Temperature data from PU-board"));

	for (int i = 0; i < data_type_list_File.count(); i++)
	{
		DataType dt = data_type_list_File[i];
		bool state = getExportingStateToFile(dt);
		data_type_list_File[i].flag = state;
	}
}

void MainWindow::showAboutDialog()
{
	AboutDialog dlg;
	dlg.exec();
}

/*
void MainWindow::setExportToOilData()
{
	ToOilExportDialog dlg(data_type_list_Oil);
	if (dlg.exec())
	{
		DataTypeList dt_list = dlg.getDataTypeList();
		bool saving = dlg.isSaving();
		for (int i = 0; i < data_type_list_Oil.count(); i++)
		{
			DataType dt = dt_list[i];
			data_type_list_Oil[i].flag = dt.flag;
			if (saving) 
			{				
				QString s_path = QString("ExportDataToOil/%1").arg(dt.mnemonics);
				app_settings->setValue(s_path, dt.flag);	
			}
		}
		if (saving) app_settings->sync();
	}
}
*/

void MainWindow::setExportToFileData()
{
	ToOilExportDialog dlg(data_type_list_File);
	dlg.setWinTitle(tr("Export to file"));
	dlg.setLblTitle(tr("Select Data Types to export to a File"));
	if (dlg.exec())
	{
		DataTypeList dt_list = dlg.getDataTypeList();
		bool saving = dlg.isSaving();
		for (int i = 0; i < data_type_list_File.count(); i++)
		{
			DataType dt = dt_list[i];
			data_type_list_File[i].flag = dt.flag;
			if (saving) 
			{				
				QString s_path = QString("ExportDataToFile/%1").arg(dt.mnemonics);
				app_settings->setValue(s_path, dt.flag);	
			}
		}
		if (saving) app_settings->sync();
	}
}

void MainWindow::setDataFileSettings()
{
	ExportSettingsDialog dlg(app_settings, save_data_attrs.to_save);
	if (dlg.exec())
	{
		save_data_attrs.file_name = dlg.getFileName();
		save_data_attrs.path = dlg.getPath();
		save_data_attrs.prefix = dlg.getPrefix();
		save_data_attrs.postfix = dlg.getPostfix();
		save_data_attrs.extension = dlg.getExtension();
		save_data_attrs.to_save = dlg.isDataExported();
		save_data_attrs.to_save_all = dlg.isAllDataExported();

		app_settings->setValue("SaveDataSettings/Path", save_data_attrs.path);
		app_settings->setValue("SaveDataSettings/Prefix", save_data_attrs.prefix);			
		app_settings->setValue("SaveDataSettings/Postfix", save_data_attrs.postfix);
		app_settings->setValue("SaveDataSettings/Extension", save_data_attrs.extension);

		//app_settings->setValue("ExperimentSettings/WellUID", well_UID);
	}
}

bool MainWindow::setExperimentSettings()
{
	ExperimentSettingsDialog dlg(app_settings, exper_attrs.dont_show_again);
	if (dlg.exec())
	{
		exper_attrs.well_UID = dlg.getWellUID();
		exper_attrs.well_name = dlg.getWellName();
		exper_attrs.field_name = dlg.getFieldName();
		exper_attrs.location = dlg.getLocation();
		exper_attrs.province = dlg.getProvince();
		exper_attrs.country = dlg.getCountry();
		exper_attrs.service_company = dlg.getServiceCompany();
		exper_attrs.company = dlg.getCompany();
		exper_attrs.oper = dlg.getOperator();
		exper_attrs.tool = dlg.getTool();
		exper_attrs.date = dlg.getDate();
		exper_attrs.dont_show_again = dlg.getDontShowAgain();

		app_settings->setValue("ExperimentSettings/WellUID", exper_attrs.well_UID);
		app_settings->setValue("ExperimentSettings/WellName", exper_attrs.well_name);
		app_settings->setValue("ExperimentSettings/Field", exper_attrs.field_name);
		app_settings->setValue("ExperimentSettings/Location", exper_attrs.location);
		app_settings->setValue("ExperimentSettings/Province", exper_attrs.province);
		app_settings->setValue("ExperimentSettings/Country", exper_attrs.country);
		app_settings->setValue("ExperimentSettings/ServiceCompany", exper_attrs.service_company);
		app_settings->setValue("ExperimentSettings/Company", exper_attrs.company);
		app_settings->setValue("ExperimentSettings/Operator", exper_attrs.oper);
		app_settings->setValue("ExperimentSettings/DontShowAgain", exper_attrs.dont_show_again);		
	}
	else return false;

	return true;
}

void MainWindow::resetCommSettings()
{
	//default_comm_settings_on = true;
	setDefaultCommSettings(true);
	nmrtoolLinker->setDefaultCommSettingsState(true);
	nmrtoolLinker->startConnection(true);
}

void MainWindow::setDefaultCommSettings(bool state)
{
	default_comm_settings_on = state;
	if (default_comm_settings_on)
	{
		comm_settings->packet_length = 100;
		comm_settings->block_length = 20;
		comm_settings->errs_count = 2;
		comm_settings->packet_delay = 0;
		comm_settings->antinoise_coding = true;
		comm_settings->packlen_autoadjust = true;
		comm_settings->interleaving = false;
		comm_settings->noise_control = false;
	}
}

void MainWindow::setSaveDataAttrs(DataSave &attrs)
{
	save_data_attrs.file_name = attrs.file_name;
	save_data_attrs.path = attrs.path;
	save_data_attrs.prefix = attrs.prefix;
	save_data_attrs.postfix = attrs.postfix;
	save_data_attrs.extension = attrs.extension;
	save_data_attrs.to_save = attrs.to_save;
	save_data_attrs.to_save_all = attrs.to_save_all;
}

void MainWindow::setSequenceStatus(SeqStatus status)
{
	sequence_status = status;

	switch (status)
	{
	case SeqStatus::Seq_Appl_OK:		a_start->setIcon(QIcon(":/images/play.png")); break;
	case SeqStatus::Seq_Appl_Failed:	a_start->setIcon(QIcon(":/images/play_red.png")); break;
	case SeqStatus::Seq_Changed:		a_start->setIcon(QIcon(":/images/play_red.png")); break;
	case SeqStatus::Seq_Not_Appl:		a_start->setIcon(QIcon(":/images/play_red.png")); break;
	default: break;
	}
}


void MainWindow::setExpSchedulerFinished()
{
	setCmdResult(NMRTOOL_STOP, ConnectionState::State_OK);
	QString finish_sound = "";
	if (findSounds(finish_sound))
	{
		QSound::play(finish_sound);
	}
}

void MainWindow::setExpSchedulerStarted()
{
	setCmdResult(NMRTOOL_START, ConnectionState::State_OK);
}

void MainWindow::connectToNMRTool(bool flag)
{		
	/*QVector<double> x,y;
	x << 1 << 2 << 3;
	y << 1 << 4 << 9;
	QVector<uint8_t> bad_v;
	bad_v << 1 << 1 << 1;
	DataSet *ds = new DataSet(1, 64, &x, &y, &bad_v);
	exportData(DataSets() << ds, QList<QVector<uint8_t> >() << QVector<uint8_t>(), QList<QVector<double> >() << QVector<double>());*/

	if (!flag) 
	{
		a_connect->setChecked(true);		
		return;
	}
	//else if (!flag) disconnectFromNMRTool();

	//QStringList e;
	//expScheduler->scheduling(e);		// temporary ! For tests only !

	nmrtoolLinker->startConnection(true);
}

void MainWindow::disconnectFromNMRTool()
{
	a_connect->setEnabled(true);
	a_connect->setChecked(false);
	a_disconnect->setEnabled(true);
	a_start->setEnabled(false);
	a_start->setChecked(false);
	a_stop->setEnabled(false);
	a_start_sdsp->setEnabled(false);
	a_start_sdsp->setChecked(false);
	a_stop_sdsp->setEnabled(false);
	a_apply_prg->setEnabled(false);

	nmrtoolLinker->startConnection(false);
	sdsp_widget->adjustmentWasStarted(false);

	experiment_timer.stop();
	lblStartTime->setVisible(false);
	lblDataCounter->setVisible(false);

	expScheduler->stop();
}

/*
void MainWindow::startNMRTool(bool flag)
{	
	if (!flag) 
	{
		a_start->setChecked(true);
		return;
	}

	data_set_windows.clear();
	data_set_windows.append(DataSetWindow(processing_relax.win_aver_len));

	experiment_id++;
	start_data_export = true;

	//applyFPGAandDSPPrg();
	
	//if (sequence_status != SeqStatus::Seq_Appl_OK)
	{
		QVector<uint8_t> proc_prg;
		QVector<uint8_t> proc_instr;
		bool res = sequenceProc->getDSPPrg(proc_prg, proc_instr);
		if (proc_prg.isEmpty() || !res) 
		{
			int ret = QMessageBox::warning(this, "Warning!", tr("The Program for FPGA not found, or contains errors!"), QMessageBox::Ok);	
			a_apply_prg->setChecked(false);
			a_start->setChecked(false);
			return;
		}
		//sequenceProc->refreshArgFormula();

		emit sdsp_is_enabled(false);

		setCmdResult(DATA_PROC, ConnectionState::State_Connecting);	
		nmrtoolLinker->applyProcPrg(proc_prg, proc_instr);
	}
	//else
	//{
	//	setCmdResult(NMRTOOL_START, ConnectionState::State_Connecting);
	//	nmrtoolLinker->startNMRTool();
	//}	
		
	delete save_data_file;
	save_data_file = NULL;	
}
*/

void MainWindow::startNMRTool(bool flag)
{		
	if (!flag) 
	{
		a_start->setChecked(true);
		return;
	}

	// Temporary commented !
	QStringList e;
	if (expScheduler->isEmpty()) 
	{
		/*AbstractDepthMeter *abs_depthmeter = depthTemplate->getCurrentDepthMeter();
		if (abs_depthmeter)		
		{
			if (abs_depthmeter->getType() == AbstractDepthMeter::LeuzeDistanceMeter)	// if KERN is in use
			{		
				//if (!expScheduler->generateDistanceScanPrg(e))
				if (!expScheduler->generateExecPrg(e))
				{
					int ret = QMessageBox::warning(this, tr("Warning!"), e.join("\n"), QMessageBox::Ok, QMessageBox::Ok);
					return;
				}
			}
			else // else if other Logging Tools are in use (KMRK, NMKT)
			{
				if (!expScheduler->generateExecPrg(e))
				{
					int ret = QMessageBox::warning(this, tr("Warning!"), e.join("\n"), QMessageBox::Ok, QMessageBox::Ok);
					return;
				}
			}
		}*/

		int ret = QMessageBox::warning(this, "Warning!", tr("Please, enter the experiment program in the Experiment Scheduler!"), QMessageBox::Ok);	
		a_apply_prg->setChecked(false);
		a_start->setChecked(false);
		return;
	}	
	
	if (!expScheduler->scheduling(e))
	{
		int ret = QMessageBox::warning(this, "Warning!", e.join("\n"), QMessageBox::Ok);	
		a_apply_prg->setChecked(false);
		a_start->setChecked(false);
		return;
	}

	if (!exper_attrs.dont_show_again)
	{
		if (!setExperimentSettings()) 
		{
			int ret = QMessageBox::warning(this, "Warning!", tr("Experiment was stopped!"), QMessageBox::Ok);	
			a_apply_prg->setChecked(false);
			a_start->setChecked(false);
			return;
		}
	}

	data_set_windows.clear();
	data_set_windows.append(DataSetWindow(processing_relax.win_aver_len));
		
	experiment_id++;
	//start_data_export = true;
		
	emit sdsp_is_enabled(false);	// удалить, т.к. имеется в строке 2224 !
		
	//delete save_data_file;
	//save_data_file = NULL;	

	logging_widget->clearAllData();
	expScheduler->start();
	startExperiment(true);
}

void MainWindow::stopNMRTool(bool flag)
{
	if (!flag) 
	{
		a_stop->setChecked(true);
		return;
	}

	setCmdResult(NMRTOOL_STOP, ConnectionState::State_Connecting);
	nmrtoolLinker->stopNMRTool();
	expScheduler->stop();
}


void MainWindow::applyDSPProcPrg(bool flag)
{
	if (!flag) 
	{
		a_apply_prg->setChecked(true);
		return;
	}

	QVector<uint8_t> proc_prg;
	QVector<uint8_t> proc_instr;
	bool res = sequenceProc->getDSPPrg(proc_prg, proc_instr);
	if (proc_prg.isEmpty() || !res) 
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("The Program for FPGA not found, or contains errors!"), QMessageBox::Ok);	
		a_apply_prg->setChecked(false);
		a_start->setChecked(false);
		return;
	}
	//sequenceProc->refreshArgFormula();

	setCmdResult(DATA_PROC, ConnectionState::State_Connecting);	
	nmrtoolLinker->applyProcPrg(proc_prg, proc_instr);
}

void MainWindow::applyWinFuncParams(QVector<int>& params)
{
	if (nmrtoolLinker->getConnectionState() == ConnectionState::State_No)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("NMR Tool is not connected!"), QMessageBox::Ok);
		return;
	}
	/*else if (nmrtoolLinker->getConnectionState() == ConnectionState::State_Connecting)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("NMR Tool is busy!"), QMessageBox::Ok);
		return;
	}*/

	OscilloscopeWidget *widget = (OscilloscopeWidget*)sender();
	if (widget)
	{
		setCmdResult(SET_WIN_PARAMS, ConnectionState::State_Connecting);
		nmrtoolLinker->applyWinFuncParams(params);
	}
}

void MainWindow::applySDSPParams(QVector<int>& params)
{
	if (nmrtoolLinker->getConnectionState() == ConnectionState::State_No)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Logging Tool is not connected!"), QMessageBox::Ok);
		return;
	}
	else if (nmrtoolLinker->getConnectionState() == ConnectionState::State_Connecting)
	{
		//int ret = QMessageBox::warning(this, "Warning!", tr("Logging Tool is busy!"), QMessageBox::Ok);
		return;
	}

	setCmdResult(SDSP_DATA, ConnectionState::State_Connecting);	
	nmrtoolLinker->sendDataToSDSP(params);
}

void MainWindow::startSDSPTool(bool flag)
{
	if (!flag) 
	{
		a_start_sdsp->setChecked(true);
		return;
	}

	data_set_windows.clear();
	data_set_windows.append(DataSetWindow(processing_relax.win_aver_len));

	experiment_id++;
	//start_data_export = true;

	QVector<uint8_t> proc_prg;
	QVector<uint8_t> proc_instr;
	bool res = sdspProc->getDSPPrg(proc_prg, proc_instr);
	if (proc_prg.isEmpty() || !res) 
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("The Program for FPGA not found, or contains errors!"), QMessageBox::Ok);			
		a_start_sdsp->setChecked(false);
		return;
	}
	sdspProc->refreshArgFormula();

	emit sdsp_is_enabled(true);

	setCmdResult(DATA_PROC, ConnectionState::State_Connecting);	
	nmrtoolLinker->applyProcPrg(proc_prg, proc_instr);
		
	//delete save_data_file;
	//save_data_file = NULL;	
}

void MainWindow::stopSDSPTool(bool flag)
{
	if (!flag) 
	{
		a_stop_sdsp->setChecked(true);
		return;
	}

	setCmdResult(NMRTOOL_STOP, ConnectionState::State_Connecting);
	nmrtoolLinker->stopNMRTool();
}

/*
void MainWindow::applyFPGAandDSPPrg()
{
	Sequence *curSeq = NULL;
	if (!sdsptab_is_active) curSeq = sequenceProc->getCurrentSequence();
	else curSeq = sdsp_widget->getSequence();
	if (curSeq == NULL) return;

	for (int i = 0; i < curSeq->arg_list.count(); i++)
	{
		Argument *seq_arg = curSeq->arg_list[i];
		seq_arg->flag = true;
		if (seq_arg->formula != "")
		{
			QString formula = seq_arg->formula;
			QString _formula = SequenceWizard::simplifyFormula(formula, curSeq);
			seq_arg->cur_formula = _formula;

			QString idx = seq_arg->index_name;
			int index = 1;
			bool ok = false;
			double res = SequenceWizard::calcArgument(index, seq_arg, &ok);		// проверка формулы
			if (!ok)
			{
				QString par_name = seq_arg->name;
				curSeq->seq_errors.append(SeqErrors(E_BadFormula, par_name));
				seq_arg->flag = false;
			}
			seq_arg->formula = formula;
		}

		if (seq_arg->pts_formula != "")
		{
			QString formula = seq_arg->pts_formula;
			QString _formula = SequenceWizard::simplifyFormula(formula, curSeq);

			bool ok = false;
			double res = SequenceWizard::calcActualDataSize(_formula, &ok);
			if (!ok)
			{
				QString par_name = seq_arg->name;
				curSeq->seq_errors.append(SeqErrors(E_BadFormula, par_name));
				seq_arg->flag = false;
			}			
			else seq_arg->actual_points = res;
		}
		bool ok;
		seq_arg->xdata.resize(seq_arg->actual_points);
		for (int j = 0; j < seq_arg->actual_points; j++)
		{			
			double x = SequenceWizard::calcArgument(j, seq_arg, &ok);
			seq_arg->xdata.data()[j] = x;
		}
	}

	for (int i = 0; i < curSeq->cmd_list.count(); i++)
	{
		Sequence_Cmd *cmd = curSeq->cmd_list[i];
		for (int j = 0; j < curSeq->param_list.count(); j++)
		{
			Sequence_Param *cur_param = curSeq->param_list[j];
			if (cmd->str_bytes234 == cur_param->name)
			{
				cmd->byte2 = (uint8_t)((cur_param->app_value & 0x00FF0000) >> 16);
				cmd->byte3 = (uint8_t)((cur_param->app_value & 0x0000FF00) >> 8);
				cmd->byte4 = (uint8_t)((cur_param->app_value & 0x000000FF));
			}
		}
	}

	for (int i = 0; i < curSeq->instr_pack_list.count(); i++)
	{
		Sequence_InstrPack *pack = curSeq->instr_pack_list[i];
		for (int k = 0; k < pack->instr_list.count(); k++)
		{
			Sequence_Instr *instr = pack->instr_list[k];
			for (int j = 0; j < curSeq->param_list.count(); j++)
			{
				Sequence_Param *cur_param = curSeq->param_list[j];
				if (!instr->str_params.isEmpty())
				{
					for (int m = 0; m < instr->str_params.count(); m++)
					{
						if (instr->str_params[m] == cur_param->name)
						{
							instr->param_bytes[4*m+3] = (uint8_t)((cur_param->app_value & 0xFF000000) >> 24);
							instr->param_bytes[4*m+2] = (uint8_t)((cur_param->app_value & 0x00FF0000) >> 16);
							instr->param_bytes[4*m+1] = (uint8_t)((cur_param->app_value & 0x0000FF00) >> 8);
							instr->param_bytes[4*m] = (uint8_t)((cur_param->app_value & 0x000000FF));
						}
					}
				}
			}
		}
	}	
}
*/

void MainWindow::breakAllActions()
{
	//setCmdResult(NMRTOOL_STOP, ConnectionState::State_OK);
	//nmrtoolLinker->stopNMRTool();

	a_connect->setEnabled(true);

	a_disconnect->setEnabled(true);

	a_start->setEnabled(true);
	a_start->setChecked(false);

	a_stop->setEnabled(false);
	a_stop->setChecked(false);

	a_start_sdsp->setEnabled(true);
	a_start_sdsp->setChecked(false);

	a_stop_sdsp->setEnabled(true);
	a_stop_sdsp->setChecked(false);

	emit sdsp_is_enabled(true);			
	sdsp_widget->adjustmentWasStarted(false);

	expScheduler->stop();
	
	QApplication::restoreOverrideCursor();	
}

void MainWindow::setMovingAveragingSettings(bool is_on, int count)
{
	DataSetWindow::is_on = is_on;
	DataSetWindow::max_size = count;

	processing_relax.win_aver_on = is_on;
	processing_relax.win_aver_len = count;
}

void MainWindow::setCmdResult(uint8_t cmd, ConnectionState state)
{
	switch (cmd)
	{
	case NMRTOOL_CONNECT:
	case NMRTOOL_CONNECT_DEF:
		if (state == ConnectionState::State_Connecting)
		{
			a_disconnect->setEnabled(false);
			a_start->setEnabled(false);
			a_stop->setEnabled(false);
			a_apply_prg->setEnabled(false);
			a_start_sdsp->setEnabled(false);
			a_stop_sdsp->setEnabled(false);

			ui->a_COMPort->setEnabled(false);
		}
		else if (state == ConnectionState::State_OK)
		{			
			a_connect->setEnabled(true);
			a_disconnect->setEnabled(true);
			a_start->setEnabled(true);
			a_stop->setEnabled(false);
			a_apply_prg->setEnabled(true);
			a_start_sdsp->setEnabled(true);
			a_stop_sdsp->setEnabled(false);

			ui->a_COMPort->setEnabled(false);

			emit sdsp_is_enabled(true);
		}
		else  
		{
			a_connect->setChecked(false);
			a_disconnect->setEnabled(true);
			a_start->setEnabled(false);		// must be set to false !
			a_start->setChecked(false);
			a_stop->setEnabled(false);
			a_stop->setChecked(false);
			a_apply_prg->setEnabled(false);
			a_apply_prg->setChecked(false);
			a_start_sdsp->setEnabled(false);
			a_start_sdsp->setChecked(false);
			a_stop_sdsp->setEnabled(false);
			a_stop_sdsp->setChecked(false);

			ui->a_COMPort->setEnabled(true);

			emit sdsp_is_enabled(false);
		}
		break;
	case NMRTOOL_START:
		if (state == ConnectionState::State_Connecting)
		{
			a_connect->setEnabled(false);
			a_disconnect->setEnabled(false);
			a_start->setEnabled(true);
			a_stop->setEnabled(false);
			//a_apply_prg->setEnabled(false);
			a_start_sdsp->setEnabled(true);
			a_stop_sdsp->setEnabled(false);

			setSequenceStatus(Seq_Not_Appl);
		}
		else if (state == ConnectionState::State_OK)
		{			
			a_connect->setEnabled(false);
			a_disconnect->setEnabled(false);
			a_start->setEnabled(true);
			a_stop->setEnabled(true);
			//a_apply_prg->setEnabled(false);
			a_start_sdsp->setEnabled(true);
			a_start_sdsp->setChecked(false);
			a_stop_sdsp->setEnabled(true);
			a_stop_sdsp->setChecked(false);

			setSequenceStatus(Seq_Appl_OK);

			emit sdsp_is_enabled(false);
		}
		else
		{
			a_disconnect->setEnabled(true);
			a_start->setEnabled(true);
			a_start->setChecked(false);
			a_stop->setEnabled(false);
			a_stop->setChecked(false);
			//a_apply_prg->setEnabled(true);
			//a_apply_prg->setChecked(false);
			a_start_sdsp->setEnabled(true);
			a_start_sdsp->setChecked(false);
			a_stop_sdsp->setEnabled(false);
			a_stop_sdsp->setChecked(false);

			setSequenceStatus(Seq_Appl_Failed);

			emit sdsp_is_enabled(true);
		}
		break;
	case NMRTOOL_STOP:
		if (state == ConnectionState::State_Connecting)
		{
			a_connect->setEnabled(false);
			a_disconnect->setEnabled(false);
			a_start->setEnabled(false);
			a_stop->setEnabled(true);
			a_apply_prg->setEnabled(false);
			a_start_sdsp->setEnabled(false);
			a_stop_sdsp->setEnabled(true);
		}
		else if (state == ConnectionState::State_OK)
		{			
			a_connect->setEnabled(true);
			a_disconnect->setEnabled(true);
			a_start->setEnabled(true);
			a_start->setChecked(false);
			a_stop->setEnabled(false);
			a_stop->setChecked(false);
			a_apply_prg->setEnabled(true);
			a_start_sdsp->setEnabled(true);
			a_start_sdsp->setChecked(false);
			a_stop_sdsp->setEnabled(true);
			a_stop_sdsp->setChecked(false);

			emit sdsp_is_enabled(true);			
			sdsp_widget->adjustmentWasStarted(false);
		}
		else
		{
			a_connect->setEnabled(true);
			a_disconnect->setEnabled(true);
			a_start->setEnabled(true);
			a_start->setChecked(true);
			a_stop->setEnabled(true);
			a_stop->setChecked(false);
			a_apply_prg->setEnabled(false);
			a_start_sdsp->setEnabled(true);
			a_start_sdsp->setChecked(true);
			a_stop_sdsp->setEnabled(true);
			a_stop_sdsp->setChecked(false);

			emit sdsp_is_enabled(false);
		}
		break;
	case DATA_PROC:	
		if (state == ConnectionState::State_Connecting)
		{
			a_connect->setEnabled(false);
			a_disconnect->setEnabled(false);
			a_start->setEnabled(false);
			a_stop->setEnabled(false);
			//a_apply_prg->setEnabled(true);
			a_start_sdsp->setEnabled(false);
			a_stop_sdsp->setEnabled(false);

			setSequenceStatus(Seq_Not_Appl);
		}
		else if (state == ConnectionState::State_OK)
		{			
			a_connect->setEnabled(false);
			a_disconnect->setEnabled(true);
			a_start->setEnabled(true);
			a_stop->setEnabled(true);
			//a_apply_prg->setEnabled(true);
			//a_apply_prg->setChecked(false);
			a_start_sdsp->setEnabled(true);
			a_stop_sdsp->setEnabled(true);

			setSequenceStatus(Seq_Appl_OK);
		}
		else
		{
			a_connect->setEnabled(true);
			a_disconnect->setEnabled(true);
			a_start->setEnabled(true);
			a_start->setChecked(false);
			a_stop->setEnabled(false);			
			//a_apply_prg->setEnabled(true);
			//a_apply_prg->setChecked(false);
			a_start_sdsp->setEnabled(true);
			a_start->setChecked(false);
			a_stop_sdsp->setEnabled(false);

			setSequenceStatus(Seq_Appl_Failed);
		}
		break;
	case SET_WIN_PARAMS:
	case SET_COMM_PARAMS:	
	case DIEL_DATA_READY:
	case SDSP_REQUEST_C8:
	case SDSP_REQUEST_88:
	case LOG_TOOL_SETTINGS:
		if (state == ConnectionState::State_Connecting)
		{
			QApplication::setOverrideCursor(Qt::BusyCursor);					
		}
		else if (state == ConnectionState::State_OK)
		{			
			QApplication::restoreOverrideCursor();					
		}
		else
		{
			QApplication::restoreOverrideCursor();			
		}
		break;
	case SDSP_DATA:
		if (state == ConnectionState::State_Connecting)
		{
			a_start_sdsp->setEnabled(false);
			a_stop_sdsp->setEnabled(false);

			QApplication::setOverrideCursor(Qt::BusyCursor);		
			sdsp_widget->waitCmdResult(true);
		}
		else if (state == ConnectionState::State_OK)
		{			
			a_start_sdsp->setEnabled(true);
			//a_start_sdsp->setChecked(false);
			if (a_start_sdsp->isChecked()) a_stop_sdsp->setEnabled(true);
			else a_stop_sdsp->setChecked(false);
			//a_stop_sdsp->setChecked(false);

			QApplication::restoreOverrideCursor();		
			sdsp_widget->resetSDSPParameters();
			sdsp_widget->waitCmdResult(false);			
		}
		else
		{
			a_start_sdsp->setEnabled(false);
			if (a_start_sdsp->isChecked()) 
			{
				a_start_sdsp->setEnabled(true);
				a_stop_sdsp->setEnabled(true);
			}
			else 
			{
				a_start_sdsp->setEnabled(false);
				a_stop_sdsp->setChecked(false);
			}
			//a_start_sdsp->setChecked(false);
			//a_stop_sdsp->setEnabled(false);
			//a_stop_sdsp->setChecked(false);

			QApplication::restoreOverrideCursor();		
			sdsp_widget->waitCmdResult(false);			
		}

		break;
	}
}


void MainWindow::storeMsgData(MsgInfo* msg_info)
{
	msg_data_container->append(msg_info);	
}


void MainWindow::treatNewData(DeviceData *device_data)
{	
	QString data_title = device_data->name;
	QList<Field_Comm*> *fields = device_data->fields;

	if (fields->count() == 0) return;	
	if (device_data->comm_id != NMRTOOL_DATA) return;

	obtained_data_counter++;

	DataSets dss;
	DataSets forT2spec_dss;		// данные для последующего расчета спектров времен Т2 
	forT2spec_dss.clear();
	QList<QVector<uint8_t> > gap_list;
	QList<QVector<double> > full_xdata_list;
	QMap<QPair<int,int>, int> data_num_generator;
	for (int i = 0; i < fields->size(); i++)
	{				
		QVector<double> *y_data = new QVector<double>(fields->at(i)->value->size());		
		QVector<double> *x_data = new QVector<double>(y_data->size());	
		QVector<double> x_data_full(y_data->size());
		QVector<uint8_t> *bad_map = new QVector<uint8_t>(fields->at(i)->bad_data->size());
		QVector<uint8_t> gap_map(y_data->size());
		
		memset(y_data->data(), 0x00, y_data->size()*sizeof(double));
		memset(x_data->data(), 0x00, x_data->size()*sizeof(double));
		memset(x_data_full.data(), 0x00, x_data->size()*sizeof(double));
		memcpy(bad_map->data(), fields->at(i)->bad_data->data(), fields->at(i)->bad_data->size()*sizeof(uint8_t));
		memset(gap_map.data(), 0x00, gap_map.count()*sizeof(uint8_t));

		uint8_t comm_id = fields->at(i)->code;
		uint8_t channel_data_id = fields->at(i)->channel;
		uint32_t msg_uid = device_data->uid;

		QString ds_name_base = "";
		switch (comm_id)
		{
		case DT_SGN_SE_ORG:			ds_name_base = "nmr_echo_sgn%1#%2"; break;
		case DT_NS_SE_ORG:			ds_name_base = "nmr_echo_noise%1#%2"; break;
		case DT_SGN_FID_ORG:		ds_name_base = "nmr_fid_sgn%1#%2"; break;
		case DT_NS_FID_ORG:			ds_name_base = "nmr_fid_noise%1#%2"; break;
		case DT_SGN_SE:				ds_name_base = "nmr_echo_sgn%1#%2"; break;
		case DT_NS_SE:				ds_name_base = "nmr_echo_noise%1#%2"; break;
		case DT_NS_QUAD_FID_RE:		ds_name_base = "nmr_fid_noise_re%1#%2"; break;
		case DT_NS_QUAD_FID_IM:		ds_name_base = "nmr_fid_noise_im%1#%2"; break;
		case DT_NS_QUAD_SE_RE:		ds_name_base = "nmr_se_noise_re%1#%2"; break;
		case DT_NS_QUAD_SE_IM:		ds_name_base = "nmr_se_noise_im%1#%2"; break;
		case DT_SGN_QUAD_FID_RE:	ds_name_base = "nmr_fid_sgn_re%1#%2"; break;
		case DT_SGN_QUAD_FID_IM:	ds_name_base = "nmr_fid_sgn_im%1#%2"; break;
		case DT_SGN_QUAD_SE_RE:		ds_name_base = "nmr_se_sgn_re%1#%2"; break;
		case DT_SGN_QUAD_SE_IM:		ds_name_base = "nmr_se_sgn_im%1#%2"; break;
		case DT_NS_FFT_FID_RE:		ds_name_base = "nmr_fid_noise_re%1#%2"; break;
		case DT_NS_FFT_SE_RE:		ds_name_base = "nmr_se_noise_re%1#%2"; break;
		case DT_SGN_FFT_FID_RE:		ds_name_base = "nmr_fid_sgn_re%1#%2"; break;
		case DT_SGN_FFT_SE_RE:		ds_name_base = "nmr_se_sgn_re%1#%2"; break;
		case DT_NS_FFT_FID_IM:		ds_name_base = "nmr_fid_noise_im%1#%2"; break;
		case DT_NS_FFT_SE_IM:		ds_name_base = "nmr_se_noise_im%1#%2"; break;
		case DT_SGN_FFT_FID_IM:		ds_name_base = "nmr_fid_sgn_im%1#%2"; break;
		case DT_SGN_FFT_SE_IM:		ds_name_base = "nmr_se_sgn_im%1#%2"; break;
		case DT_SGN_RELAX:			ds_name_base = "nmr_relax_sgn%1#%2"; break;
		case DT_SGN_RELAX2:			ds_name_base = "nmr_relax_sgn%1#%2"; break;
		case DT_SGN_RELAX3:			ds_name_base = "nmr_relax_sgn%1#%2"; break;
		case DT_T1T2_NMR:			ds_name_base = "T1T2_2D-NMR%1#%2"; break;
		case DT_DsT2_NMR:			ds_name_base = "DsT2_2D-NMR%1#%2"; break;
		case DT_SOLID_ECHO:			ds_name_base = "nmr_solid_echo_sgn%1#%2"; break;		
		case DT_SGN_POWER_SE:		ds_name_base = "nmr_se_power_sgn%1#%2"; break;
		case DT_SGN_POWER_FID:		ds_name_base = "nmr_fft_power_sgn%1#%2"; break;
		case DT_NS_POWER_SE:		ds_name_base = "nmr_se_power_ns%1#%2"; break;
		case DT_NS_POWER_FID:		ds_name_base = "nmr_fft_power_ns%1#%2"; break;
		case DT_SGN_FFT_FID_AM:		ds_name_base = "nmr_fid_sgn_ampl%1#%2"; break;
		case DT_NS_FFT_FID_AM:		ds_name_base = "nmr_fid_ns_ampl%1#%2"; break;
		case DT_SGN_FFT_SE_AM:		ds_name_base = "nmr_se_sgn_ampl%1#%2"; break;
		case DT_NS_FFT_SE_AM:		ds_name_base = "nmr_se_ns_ampl%1#%2"; break;
		case DT_GAMMA:				ds_name_base = "gamma%1#%2"; break;
		case DT_DIEL:				ds_name_base = "dielectric%1#%2"; break;
		case DT_DIEL_ADJUST:		ds_name_base = "dielectric_adjust%1#%2"; break;
		case DT_AFR1_RX:			ds_name_base = "AFR_RX%1#%2"; break;
		case DT_AFR2_RX:			ds_name_base = "AFR_RX%1#%2"; break;
		case DT_AFR3_RX:			ds_name_base = "AFR_RX%1#%2"; break;
		case DT_AFR1_TX:			ds_name_base = "AFR_TX%1#%2"; break;
		case DT_AFR2_TX:			ds_name_base = "AFR_TX%1#%2"; break;
		case DT_AFR3_TX:			ds_name_base = "AFR_TX%1#%2"; break;
		case DT_FREQ_TUNE:			ds_name_base = "Freq_Autotune%1#%2"; break;
		case DT_RFP:				ds_name_base = "RFP%1#%2"; break;
		case DT_RFP2:				ds_name_base = "RFP%1#%2"; break;
		case DT_DU:					ds_name_base = "telemetry_DU#%1"; break;
		case DT_DU_T:				ds_name_base = "DU_T#%1"; break;
		case DT_TU:					ds_name_base = "telemetry_PU#%1"; break;
		case DT_TU_T:				ds_name_base = "TU_T#%1"; break;
		case DT_PU:					ds_name_base = "telemetry_PU#%1"; break;
		case DT_PU_T:				ds_name_base = "PU_T#%1"; break;

		default: ds_name_base = "unknown_data#%1"; break;
		}
				
		switch (comm_id)
		{
		case DT_SGN_RELAX:
		case DT_SGN_RELAX2:
		case DT_SGN_RELAX3:
		case DT_T1T2_NMR:
		case DT_DsT2_NMR:
			{
				int group_index = fields->at(i)->tag;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				/*if (channel_data_id <= 0 || channel_data_id > tool_channels.count()) 
				{
					delete x_data;
					delete y_data;
					delete bad_map;
					continue;
				}*/
				int full_size = 0;
				double te = 0;
				double tw = 0;
				double td = 0;
				if (group_index > 0)
				{
					int data_index = 0; 
					//Argument *arg = sequenceProc->getCurrentSequence()->arg_list[group_index-1];
					//full_size = arg->actual_points;
					////LUSI::Argument *arg = sequenceProc->getCurrentSequence()->arg_list[group_index-1];
					//LUSI::Argument *arg = sequenceProc->getCurrentSequence()->getArgument(group_index);
					JSeqObject *executingJSeq = expScheduler->getExecutingJSeqObject();
					if (!executingJSeq) 
					{
						delete x_data;
						delete y_data;
						delete bad_map;
						continue;
					}
					LUSI::Argument *arg = executingJSeq->lusi_Seq->getArgument(group_index);
					te = arg->getTE();
					tw = arg->getTW();
					td = arg->getTD();
					full_size = arg->getSize();
								
					for (int j = 0; j < x_data->size(); j++)
					{
						//full_size++;
						float val = fields->at(i)->value->at(j);
						uint32_t *b = (uint32_t*)(&val);
						uint32_t bb = *b;
						if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
						else gap_map.data()[j] = DATA_OK;

						bool ok = false;						
						//x_data_full.data()[j] = arg->xdata.data()[j];
						x_data_full.data()[j] = arg->getPoints().at(j).toDouble(&ok);
						
						if (bad_map->at(j) == DATA_OK)
						{
							bool ok = false;
							*(x_data->data()+data_index) = x_data_full.data()[j]; //sequenceProc->calcArgument(j, arg, &ok);
							*(y_data->data()+data_index) = fields->at(i)->value->at(j); ///512;		// 512 - количество точек после преобразования фурье
							data_index++;
						}	
						else gap_map.data()[j] = BAD_DATA;
					} 
					x_data->resize(data_index);
					y_data->resize(data_index);					
				}
				else
				{
					int data_index = 0;
					for (int j = 0; j < x_data->size(); j++) 
					{		
						full_size++;
						float val = fields->at(i)->value->at(j);
						uint32_t *b = (uint32_t*)(&val);
						uint32_t bb = *b;
						if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
						else gap_map.data()[j] = DATA_OK;

						bool ok = false;
						x_data_full.data()[j] = (double)(j+1);
						
						if (bad_map->at(j) == DATA_OK)
						{
							*(x_data->data()+data_index) = (double)(j+1);
							*(y_data->data()+data_index) = fields->at(i)->value->at(j); ///512;		// 512 - количество точек после преобразования фурье
							data_index++;
						}
						else gap_map.data()[j] = BAD_DATA;
					}
					x_data->resize(data_index);
					y_data->resize(data_index);
				}	
				
				bool data_ok = true;
				if (processing_relax.is_quality_controlled) data_ok = doQualityControl(y_data);
				if (!data_ok)
				{
					delete x_data;
					delete y_data;
					delete bad_map;

					continue;
				}

				if (processing_relax.is_smoothed)
				{
					doSmoothing(y_data);
				}
				if (processing_relax.is_centered)
				{
					doZeroAdjustment(y_data);
				}
				
				static int num_data = 0;
				DataSetWindow *data_set_window = 0;
				if (group_index > data_set_windows.count()) 
				{
					for (int k = 0; k < group_index; k++)
					{
						data_set_windows.append(DataSetWindow(processing_relax.win_aver_len));
					}
				}
				if (group_index > 0) data_set_window = &data_set_windows[group_index-1];
				else data_set_window = &data_set_windows.first();
				
				if (data_set_window->is_on)
				{
					QVector<double> x_vec(x_data->size());
					QVector<double> y_vec(y_data->size());
					memcpy(x_vec.data(), x_data->data(), x_data->size()*sizeof(double));
					memcpy(y_vec.data(), y_data->data(), y_data->size()*sizeof(double));

					int k = data_set_window->y_vectors.count() - data_set_window->max_size;
					if (k >= 0)
					{
						while (k-- >= 0)
						{
							data_set_window->x_vectors.pop_front();
							data_set_window->y_vectors.pop_front();
						}
						
						data_set_window->x_vectors.push_back(x_vec);
						data_set_window->y_vectors.push_back(y_vec);
					}
					else
					{
						data_set_window->x_vectors.push_back(x_vec);
						data_set_window->y_vectors.push_back(y_vec);						
					}

					int max_index = -1;
					int max_data_len = data_set_window->max_data_len(max_index);					
					if (max_index >= 0)
					{
						QVector<double> *aver_y_vec = new QVector<double>(max_data_len);		
						QVector<double> *aver_x_vec = new QVector<double>(max_data_len);
						QVector<uint8_t> *aver_bad_vec = new QVector<uint8_t>(max_data_len);
						memcpy(aver_x_vec->data(), data_set_window->x_vectors[max_index].data(), max_data_len*sizeof(double));
						memset(aver_bad_vec->data(), DATA_OK, max_data_len*sizeof(uint8_t));
						for (int k = 0; k < max_data_len; k++)
						{						
							double S = 0;
							int cnt = 0;
							for (int m = 0; m < data_set_window->count(); m++)
							{
								if (k < data_set_window->y_vectors[m].size())
								{
									double val = data_set_window->y_vectors[m].data()[k];
									if (val != NAN && !isNAN(val))
									{
										S += val;
										cnt++;
									}
									else 
									{
										aver_bad_vec->data()[k] = BAD_DATA;
									}
								}
								
							}
							if (cnt != 0) aver_y_vec->data()[k] = S/cnt;
						}
							
						// раскомментировать это, чтобы вернуть одновременный вывод усредненных и измеренных данных
						//
						//QString _ds_name_base = "nmr_relax_aver#%1";
						//uint8_t aver_comm_id = DT_AVER_RELAX;
						//if (comm_id == DT_SGN_RELAX2) aver_comm_id = DT_AVER_RELAX2;
						//else if (comm_id == DT_SGN_RELAX3) aver_comm_id = DT_AVER_RELAX3;
						//DataSet *ds = new DataSet(_ds_name_base.arg(++num_data), msg_uid, aver_comm_id, aver_x_vec, aver_y_vec, aver_bad_vec);
						//ds->setInitialDataSize(full_size);	
						//ds->setGroupIndex(group_index);
						//QPair<bool,double> dpt = depthMonitor->getDepthData();
						//ds->setDepth(dpt);
						//ds->setExpId(experiment_id);
						//dss.append(ds);
						//gap_list.append(gap_map);
						//full_xdata_list.append(x_data_full);
						//

						// убрать это, чтобы вернуть одновременный вывод усредненных и измеренных данных
						DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, aver_y_vec, aver_bad_vec);
						ds->setInitialDataSize(full_size);	
						ds->setGroupIndex(group_index);
						ds->setChannelId(channel_data_id);						
						//double x_displ = getDepthDisplacement(comm_id, tool_channels);
						double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
						QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
						dpt.second = dpt.second + x_displ;
						ds->setDepth(dpt);
						ds->setExpId(experiment_id);
						ds->setTE(te);
						ds->setTW(tw);
						ds->setTD(td);
						QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
						if (data_num_generator.contains(key))
						{							
							int last_data_num = data_num_generator[key];
							ds->setDataNum(last_data_num+1);
							data_num_generator[key] = last_data_num+1;
						}
						else data_num_generator[key] = 1;
						ds->setDataNum(data_num_generator[key]);

						dss.append(ds);
						gap_list.append(gap_map);
						full_xdata_list.append(x_data_full);

						if (comm_id != DT_T1T2_NMR && comm_id != DT_DsT2_NMR)
						{
							forT2spec_dss.append(ds);
						}						
					}					
				}
								
				// раскомментировать это, чтобы вернуть одновременный вывод усредненных и измеренных данных
				//DataSet *ds = new DataSet(ds_name_base.arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				//ds->setInitialDataSize(full_size);	
				//ds->setGroupIndex(group_index);
				//QPair<bool,double> dpt = depthMonitor->getDepthData();
				//ds->setDepth(dpt);
				//ds->setExpId(experiment_id);
				//dss.append(ds);
				//gap_list.append(gap_map);
				//full_xdata_list.append(x_data_full);
				
				// for spectrum of T2 times
				//forT2spec_dss.append(ds);	
	
				break;
			} 		
		case DT_SOLID_ECHO:
			{
				int group_index = fields->at(i)->tag;
				
				int full_size = 0;
				double te = 0;
				double tw = 0;
				double td = 0;
				if (group_index > 0)
				{
					int data_index = 0; 					
					JSeqObject *executingJSeq = expScheduler->getExecutingJSeqObject();
					if (!executingJSeq) 
					{
						delete x_data;
						delete y_data;
						delete bad_map;
						continue;
					}
					LUSI::Argument *arg = executingJSeq->lusi_Seq->getArgument(group_index);
					te = arg->getTE();
					tw = arg->getTW();
					td = arg->getTD();
					full_size = arg->getSize();
								
					for (int j = 0; j < x_data->size(); j++)
					{						
						float val = fields->at(i)->value->at(j);
						uint32_t *b = (uint32_t*)(&val);
						uint32_t bb = *b;
						if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
						else gap_map.data()[j] = DATA_OK;

						bool ok = false;										
						x_data_full.data()[j] = arg->getPoints().at(j).toDouble(&ok);
						
						if (bad_map->at(j) == DATA_OK)
						{
							bool ok = false;
							*(x_data->data()+data_index) = x_data_full.data()[j]; 
							*(y_data->data()+data_index) = fields->at(i)->value->at(j); ///512;		// 512 - количество точек после преобразования фурье
							data_index++;
						}	
						else gap_map.data()[j] = BAD_DATA;
					} 
					x_data->resize(data_index);
					y_data->resize(data_index);					
				}
				else
				{
					int data_index = 0;
					for (int j = 0; j < x_data->size(); j++) 
					{		
						full_size++;
						float val = fields->at(i)->value->at(j);
						uint32_t *b = (uint32_t*)(&val);
						uint32_t bb = *b;
						if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
						else gap_map.data()[j] = DATA_OK;

						bool ok = false;
						x_data_full.data()[j] = (double)(j+1);
						
						if (bad_map->at(j) == DATA_OK)
						{
							*(x_data->data()+data_index) = (double)(j+1);
							*(y_data->data()+data_index) = fields->at(i)->value->at(j); ///512;		// 512 - количество точек после преобразования фурье
							data_index++;
						}
						else gap_map.data()[j] = BAD_DATA;
					}
					x_data->resize(data_index);
					y_data->resize(data_index);
				}	
								
				static int num_data = 0;
				DataSetWindow *data_set_window = 0;
				if (group_index > data_set_windows.count()) 
				{
					for (int k = 0; k < group_index; k++)
					{
						data_set_windows.append(DataSetWindow(processing_relax.win_aver_len));
					}
				}
				if (group_index > 0) data_set_window = &data_set_windows[group_index-1];
				else data_set_window = &data_set_windows.first();
				
				if (data_set_window->is_on)
				{
					QVector<double> x_vec(x_data->size());
					QVector<double> y_vec(y_data->size());
					memcpy(x_vec.data(), x_data->data(), x_data->size()*sizeof(double));
					memcpy(y_vec.data(), y_data->data(), y_data->size()*sizeof(double));

					int k = data_set_window->y_vectors.count() - data_set_window->max_size;
					if (k >= 0)
					{
						while (k-- >= 0)
						{
							data_set_window->x_vectors.pop_front();
							data_set_window->y_vectors.pop_front();
						}
						
						data_set_window->x_vectors.push_back(x_vec);
						data_set_window->y_vectors.push_back(y_vec);
					}
					else
					{
						data_set_window->x_vectors.push_back(x_vec);
						data_set_window->y_vectors.push_back(y_vec);						
					}

					int max_index = -1;
					int max_data_len = data_set_window->max_data_len(max_index);					
					if (max_index >= 0)
					{
						QVector<double> *aver_y_vec = new QVector<double>(max_data_len);		
						QVector<double> *aver_x_vec = new QVector<double>(max_data_len);
						QVector<uint8_t> *aver_bad_vec = new QVector<uint8_t>(max_data_len);
						memcpy(aver_x_vec->data(), data_set_window->x_vectors[max_index].data(), max_data_len*sizeof(double));
						memset(aver_bad_vec->data(), DATA_OK, max_data_len*sizeof(uint8_t));
						for (int k = 0; k < max_data_len; k++)
						{						
							double S = 0;
							int cnt = 0;
							for (int m = 0; m < data_set_window->count(); m++)
							{
								if (k < data_set_window->y_vectors[m].size())
								{
									double val = data_set_window->y_vectors[m].data()[k];
									if (val != NAN && !isNAN(val))
									{
										S += val;
										cnt++;
									}
									else 
									{
										aver_bad_vec->data()[k] = BAD_DATA;
									}
								}
								
							}
							if (cnt != 0) aver_y_vec->data()[k] = S/cnt;
						}
						
						// убрать это, чтобы вернуть одновременный вывод усредненных и измеренных данных
						DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, aver_y_vec, aver_bad_vec);
						ds->setInitialDataSize(full_size);	
						ds->setGroupIndex(group_index);
						ds->setChannelId(channel_data_id);						
						double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
						QPair<bool,double> dpt = depthTemplate->getDepthData(); 
						dpt.second = dpt.second + x_displ;
						ds->setDepth(dpt);
						ds->setTE(te);
						ds->setTW(tw);
						ds->setTD(td);
						ds->setExpId(experiment_id);
						QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
						if (data_num_generator.contains(key))
						{							
							int last_data_num = data_num_generator[key];							
							ds->setDataNum(last_data_num+1);
							data_num_generator[key] = last_data_num+1;
						}
						else data_num_generator[key] = 1;
						ds->setDataNum(data_num_generator[key]);

						dss.append(ds);
						gap_list.append(gap_map);
						full_xdata_list.append(x_data_full);						
					}
				}
				break;
			} 
		case DT_SGN_SE_ORG:
		case DT_SGN_FID_ORG:
		case DT_NS_SE_ORG:
		case DT_NS_FID_ORG:
		case DT_SGN_SE:
		case DT_NS_SE:
			{
				int full_size = 0;
				int data_index = 0;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				double NMR_SAMPLE_FREQ = 4*250 * 1000;		// Hz
				//if (channel_data_id > 0 || channel_data_id <= tool_channels.count())
				{
					for (int j = 0; j < tool_channels.count(); j++)
					{
						ToolChannel *tool_ch = tool_channels[j];
						if (tool_ch->channel_id == channel_data_id)
						{
							if (tool_ch->sample_freq > 1) 
							{
								NMR_SAMPLE_FREQ = tool_ch->sample_freq*1000;		// originally sample freq in [kHz]
								break;
							}
						}
					}
				}
				
				for (int j = 0; j < x_data->size(); j++) 
				{				
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;
										
					x_data_full.data()[j] = (double)(j)/NMR_SAMPLE_FREQ*1000000;
					
					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)(j)/NMR_SAMPLE_FREQ*1000000;
						*(y_data->data()+data_index) = fields->at(i)->value->at(j);
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);
				
				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);	
				ds->setChannelId(channel_data_id);
				//double x_displ = getDepthDisplacement(comm_id, tool_channels);
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);
				
				break;
			}			
		case DT_RFP:
		case DT_RFP2:
			{
				int full_size = 0;
				int data_index = 0;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				double NMR_SAMPLE_FREQ = 4*250*1000;
				//if (channel_data_id > 0 || channel_data_id <= tool_channels.count())
				{
					for (int j = 0; j < tool_channels.count(); j++)
					{
						ToolChannel *tool_ch = tool_channels[j];
						if (tool_ch->channel_id == channel_data_id)
						{
							if (tool_ch->sample_freq > 1) 
							{
								NMR_SAMPLE_FREQ = tool_ch->sample_freq*1000;		// originally sample freq in [kHz]
							}
						}						
					}					
				}
				
				for (int j = 0; j < x_data->size(); j++) 
				{				
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					x_data_full.data()[j] = (double)(j)/NMR_SAMPLE_FREQ*1000000;

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)(j)/NMR_SAMPLE_FREQ*1000000;
						*(y_data->data()+data_index) = fields->at(i)->value->at(j);
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);	
				ds->setChannelId(channel_data_id);
				//double x_displ = getDepthDisplacement(comm_id, tool_channels);
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}			
		case DT_NS_QUAD_FID_RE:
		case DT_NS_QUAD_SE_RE:
		case DT_SGN_QUAD_FID_RE:
		case DT_SGN_QUAD_SE_RE:
		case DT_NS_QUAD_FID_IM:		
		case DT_NS_QUAD_SE_IM:
		case DT_SGN_QUAD_FID_IM:		
		case DT_SGN_QUAD_SE_IM:		
			{		
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				double NMR_SAMPLE_FREQ = 4*250*1000;
				//if (channel_data_id > 0 || channel_data_id <= tool_channels.count())
				{
					for (int j = 0; j < tool_channels.count(); j++)
					{
						ToolChannel *tool_ch = tool_channels[j];
						if (tool_ch->channel_id == channel_data_id)
						{
							if (tool_ch->sample_freq > 1) 
							{
								NMR_SAMPLE_FREQ = tool_ch->sample_freq*1000;		// originally sample freq in [kHz]
							}
						}						
					}					
				}
				
				int full_size = 0;
				int data_index = 0;
				for (int j = 0; j < x_data->size(); j++) 
				{		
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					x_data_full.data()[j] = (double)(j+1)*2.0/NMR_SAMPLE_FREQ*1000000;

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)(j+1)*2.0/NMR_SAMPLE_FREQ*1000000;
						*(y_data->data()+data_index) = fields->at(i)->value->at(j);
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);		
				ds->setChannelId(channel_data_id);
				//QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);	
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);
				
				break;
			}				
		case DT_NS_FFT_FID_RE:
		case DT_NS_FFT_SE_RE:
		case DT_SGN_FFT_FID_RE:
		case DT_SGN_FFT_SE_RE:	
		case DT_NS_FFT_FID_IM:
		case DT_NS_FFT_SE_IM:
		case DT_SGN_FFT_FID_IM:
		case DT_SGN_FFT_SE_IM:
			{			
				int full_size = 0;
				int data_index = 0;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				double NMR_SAMPLE_FREQ = 4*250*1000;
				//if (channel_data_id > 0 || channel_data_id <= tool_channels.count())
				{
					for (int j = 0; j < tool_channels.count(); j++)
					{
						ToolChannel *tool_ch = tool_channels[j];
						if (tool_ch->channel_id == channel_data_id)
						{
							if (tool_ch->sample_freq > 1) 
							{
								NMR_SAMPLE_FREQ = tool_ch->sample_freq*1000;		// originally sample freq in [kHz]
							}
						}						
					}					
				}
				
				for (int j = 0; j < x_data->size(); j++) 
				{
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					x_data_full.data()[j] = (double)(j)*NMR_SAMPLE_FREQ/2.0/1000/x_data->size();

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)(j)*NMR_SAMPLE_FREQ/2.0/1000/x_data->size();	
						*(y_data->data()+data_index) = fields->at(i)->value->at(j)/y_data->size();
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);		
				ds->setChannelId(channel_data_id);
				//QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}			
		case DT_SGN_FFT_FID_AM:						
		case DT_SGN_FFT_SE_AM:	
		case DT_NS_FFT_FID_AM:
		case DT_NS_FFT_SE_AM:
			{
				// построение данных на графике Data Preview...		
				int full_size = 0;
				int data_index = 0;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				double NMR_SAMPLE_FREQ_HALF = 2*250*1000;
				//if (channel_data_id > 0 || channel_data_id <= tool_channels.count())
				{
					for (int j = 0; j < tool_channels.count(); j++)
					{
						ToolChannel *tool_ch = tool_channels[j];
						if (tool_ch->channel_id == channel_data_id)
						{
							if (tool_ch->sample_freq > 1) 
							{
								NMR_SAMPLE_FREQ_HALF = tool_ch->sample_freq/2*1000;		// originally sample freq in [kHz]
							}
						}						
					}					
				}
				
				for (int j = 0; j < x_data->size(); j++) 
				{		
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					x_data_full.data()[j] = (double)(j)*NMR_SAMPLE_FREQ_HALF/1000/x_data->size();		// 1000 - перевод из Гц в кГц

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)(j)*NMR_SAMPLE_FREQ_HALF/1000/x_data->size();		// 1000 - перевод из Гц в кГц
						*(y_data->data()+data_index) = fields->at(i)->value->at(j)/y_data->size();
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				ds->setChannelId(channel_data_id);
				//QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}		
		case DT_SGN_POWER_SE:
		case DT_SGN_POWER_FID:	
		case DT_NS_POWER_SE:
		case DT_NS_POWER_FID:
			{
				// построение данных на графике Data Preview...		
				int full_size = 0;
				int data_index = 0;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				double NMR_SAMPLE_FREQ_HALF = 2*250*1000;
				//if (channel_data_id > 0 || channel_data_id <= tool_channels.count())
				{
					for (int j = 0; j < tool_channels.count(); j++)
					{
						ToolChannel *tool_ch = tool_channels[j];
						if (tool_ch->channel_id == channel_data_id)
						{
							if (tool_ch->sample_freq > 1) 
							{
								NMR_SAMPLE_FREQ_HALF = tool_ch->sample_freq/2*1000;		// originally sample freq in [kHz]
							}
						}						
					}					
				}
				
				for (int j = 0; j < x_data->size(); j++) 
				{
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					x_data_full.data()[j] = (double)(j)*NMR_SAMPLE_FREQ_HALF/1000/x_data->size();		// 1000 - перевод из Гц в кГц

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)(j)*NMR_SAMPLE_FREQ_HALF/1000/x_data->size();		// 1000 - перевод из Гц в кГц
						*(y_data->data()+data_index) = fields->at(i)->value->at(j)/y_data->size()/y_data->size();
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				ds->setChannelId(channel_data_id);
				//QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}		
		case DT_AFR1_RX:
		case DT_AFR2_RX:
		case DT_AFR3_RX:
		case DT_FREQ_TUNE:
			{
				double te = 0;
				double tw = 0;
				double td = 0;
				int full_size = 0;
				int group_index = fields->at(i)->tag;				
				if (group_index > 0)
				{
					int data_index = 0;					
					JSeqObject *executingJSeq = expScheduler->getExecutingJSeqObject();
					if (!executingJSeq) 
					{
						delete x_data;
						delete y_data;
						delete bad_map;
						continue;
					}
					LUSI::Argument *arg = executingJSeq->lusi_Seq->getArgument(group_index);
					te = arg->getTE();
					tw = arg->getTW();
					td = arg->getTD();
					for (int j = 0; j < x_data->size(); j++)
					{
						full_size++;
						float val = fields->at(i)->value->at(j);
						uint32_t *b = (uint32_t*)(&val);
						uint32_t bb = *b;
						//if (bb == 0xffffffff) continue;
						if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
						else gap_map.data()[j] = DATA_OK;

						bool ok = false;						
						//x_data_full.data()[j] = arg->xdata.data()[j];
						x_data_full.data()[j] = arg->getPoints().at(j).toDouble(&ok);

						if (bad_map->at(j) == DATA_OK)
						{
							bool ok = false;							
							//*(x_data->data()+data_index) = arg->xdata.data()[j];
							*(x_data->data()+data_index) = arg->getPoints().at(j).toDouble(&ok);
							*(y_data->data()+data_index) = fields->at(i)->value->at(j)/512;		// 512 - количество точек после преобразования фурье
							data_index++;
						}	
						else gap_map.data()[j] = BAD_DATA;
					}
					x_data->resize(data_index);
					y_data->resize(data_index);
				}
				else
				{
					int data_index = 0;
					for (int j = 0; j < x_data->size(); j++) 
					{		
						full_size++;
						float val = fields->at(i)->value->at(j);
						uint32_t *b = (uint32_t*)(&val);
						uint32_t bb = *b;
						//if (bb == 0xffffffff) continue;
						if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
						else gap_map.data()[j] = DATA_OK;
												
						x_data_full.data()[j] = (double)(j+1);

						if (bad_map->at(j) == DATA_OK)
						{
							*(x_data->data()+data_index) = (double)(j+1);
							*(y_data->data()+data_index) = fields->at(i)->value->at(j)/512;		// 512 - количество точек после преобразования фурье
							data_index++;
						}
						else gap_map.data()[j] = BAD_DATA;
					}
					x_data->resize(data_index);
					y_data->resize(data_index);
				}				

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				ds->setChannelId(channel_data_id);
				//double x_displ = getDepthDisplacement(comm_id, tool_channels);
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); 
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setTE(te);
				ds->setTW(tw);
				ds->setTD(td);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);			
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			} 			
		case DT_GAMMA:
			{
				int full_size = 1;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				/*if (channel_data_id <= 0 || channel_data_id > tool_channels.count()) 
				{
					delete x_data;
					delete y_data;
					delete bad_map;
					continue;
				}*/

				float val = fields->at(i)->value->at(0);
				uint32_t *b = (uint32_t*)(&val);
				uint32_t bb = *b;				
				if (bb == 0xffffffff) gap_map.data()[0] = BAD_DATA;
				else gap_map.data()[0] = DATA_OK;

				x_data_full.data()[0] = 0;

				int data_index = 0;
				if (bad_map->at(0) == DATA_OK)
				{
					*(x_data->data()+data_index) = 0;		
					*(y_data->data()+data_index) = fields->at(i)->value->at(0);
					data_index++;
				}
				else gap_map.data()[0] = BAD_DATA;
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				ds->setChannelId(channel_data_id);
				//double x_displ = getDepthDisplacement(comm_id, tool_channels);
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}
		case DT_DIEL:
			{
				int full_size = 0;
				int data_index = 0;
				//uint8_t channel_data_id = fields->at(i)->channel-1;
				/*if (channel_data_id <= 0 || channel_data_id > tool_channels.count()) 
				{
					delete x_data;
					delete y_data;
					delete bad_map;
					continue;
				}*/
				for (int j = 0; j < y_data->size(); j++) 
				{				
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;
										
					x_data_full.data()[j] = j+1;

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = j+1;
						*(y_data->data()+data_index) = fields->at(i)->value->at(j);
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				ds->setChannelId(channel_data_id);
				//double x_displ = getDepthDisplacement(comm_id, tool_channels);
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}
		case DT_DIEL_ADJUST:
			{
				int full_size = 0;
				int data_index = 0;
				int group_index = fields->at(i)->tag;
				//uint8_t channel_data_id = fields->at(i)->channel;
				/*if (channel_data_id <= 0 || channel_data_id > tool_channels.count()) 
				{
					delete x_data;
					delete y_data;
					delete bad_map;
					continue;
				}*/
				Argument *arg = sdspProc->getCurrentSequence()->arg_list[group_index-1];
				for (int j = 0; j < y_data->size(); j++) 
				{				
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = arg->xdata.data()[j/4];
						*(y_data->data()+data_index) = fields->at(i)->value->at(j);
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				ds->setChannelId(channel_data_id);
				//QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels);
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];							
					ds->setDataNum(last_data_num+1);
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				
				emit control_sdsptool(false);		// больше не запрашивать даные из диэлектрического прибора
				break;
			}
		case DT_DU_T:
		case DT_PU_T:
		case DT_TU_T:
			{
				// построение данных мониторинга...
				double A = 405.5837;
				double B = -69;
				double C = 3.8222;
				double D = -0.0885;

				int full_size = 0;
				int data_index = 0;
				QDateTime cur_dtime = QDateTime::currentDateTime(); 
				for (int j = 0; j < x_data->size(); j++) 
				{			
					full_size++;
					float val = fields->at(i)->value->at(j);
					uint32_t *b = (uint32_t*)(&val);
					uint32_t bb = *b;
					if (bb == 0xffffffff) gap_map.data()[j] = BAD_DATA;
					else gap_map.data()[j] = DATA_OK;

					bool ok = false;
					x_data_full.data()[j] = (double)cur_dtime.toTime_t();

					if (bad_map->at(j) == DATA_OK)
					{
						*(x_data->data()+data_index) = (double)cur_dtime.toTime_t();
						
						double y = fields->at(i)->value->at(j);
						if (y > 0) 
						{
							double lnR = log(y);
							y = D*lnR*lnR*lnR + C*lnR*lnR + B*lnR + A;
						}
						else y = NAN;	
						
						*(y_data->data()+data_index) = y;
						data_index++;
					}
					else gap_map.data()[j] = BAD_DATA;
				}
				x_data->resize(data_index);
				y_data->resize(data_index);

				static int num_data = 0;
				DataSet *ds = new DataSet(ds_name_base.arg(++num_data), msg_uid, comm_id, x_data, y_data, bad_map);
				ds->setInitialDataSize(full_size);
				//QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				double x_displ = getDepthDisplacement(channel_data_id, tool_channels); 
				QPair<bool,double> dpt = depthTemplate->getDepthData(); //depthMonitor->getDepthData();
				dpt.second = dpt.second + x_displ;
				ds->setDepth(dpt);
				ds->setExpId(experiment_id);
				QPair<int,int> key = QPair<int,int>(comm_id,channel_data_id);
				if (data_num_generator.contains(key))
				{							
					int last_data_num = data_num_generator[key];						
					data_num_generator[key] = last_data_num+1;
				}
				else data_num_generator[key] = 1;
				ds->setDataNum(data_num_generator[key]);

				dss.append(ds);
				gap_list.append(gap_map);
				full_xdata_list.append(x_data_full);

				break;
			}			
		default: break;
		}		
	}		

	calcT2Spectra(forT2spec_dss);

	plotData(dss);
	plotLoggingData(dss);
	exportData(dss, gap_list, full_xdata_list);

	lockDataSet();
	dataset_storage->append(dss);
	unlockDataSet();
}


/*
void MainWindow::exportData(DataSets &dss, QList<QVector<uint8_t> > &gap, QList<QVector<double> > &full_xdata)
{
	if (dss.isEmpty()) return;
	if (!nmrtool_state) return;
	if (!save_data_attrs.to_save)
	{
		placeInfoToStatusBar(QString("<font color=red><b>Measured data is not saved to the file!</b></font>"));			// не записывать данные !
		return;
	}
		
	//profiler.tic(4, "MainWindow::exportData()");
		
	//LUSI::Sequence *cur_seq = sequenceProc->getCurrentSequence();
	JSeqObject *executingJSeq = expScheduler->getExecutingJSeqObject();
	if (!executingJSeq) return;
	LUSI::Sequence *cur_seq = executingJSeq->lusi_Seq;

	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_str = ctime.toString("d-M-yyyy_hh:mm:ss");

	uint32_t uid = dss[0]->getUId();
	QPair<bool, double> depth = dss[0]->getDepth();

	static QVector<int> x_full_size; 

	bool xdata_is_ok = true;
	QString memo = "";

	QString file_name = expScheduler->getDataFile();	
	if (file_name.isEmpty()) return;

	QFileInfo data_file_info(file_name);
	int data_file_size = data_file_info.size();
	if (data_file_size == 0)
	//if (start_data_export)
	{		
		memo = "[Description]\n";
		memo += QString("Device = %1\n").arg(current_tool.type);
		memo += QString("DeviceUId = %1\n").arg(current_tool.id);
		memo += QString("Object = %1\n").arg("Скважина X");
		memo += QString("Company = %1\n").arg("КФУ");
		memo += QString("Experiment = %1\n").arg("");
		memo += QString("Operator = %1\n").arg("");			
		memo += QString("DateTime = %1\n").arg(ctime_str);
		memo += QString("\n\n");

		memo += "[Format]\n";
		memo += QString("Version = %1\n").arg("1.0");
		memo += QString("DecimalSymbol = %1\n").arg(".");
		memo += QString("DataSeparator = %1\n").arg(" ");
		memo += QString("DataSetSeparator = %1\n").arg(";");
		memo += QString("\n\n");

		memo += "[Intervals]\n";
		for (int i = 0; i < cur_seq->param_list.count(); i++)
		{
			//Sequence_Param *param = cur_seq->param_list[i];
			LUSI::Parameter *param = cur_seq->param_list[i];
			//if (param->type == SeqParamType::Interval && !param->read_only)
			if (!param->getReadOnly())
			{
				//QString int_name = param->name;				
				//QString int_caption = param->caption;				
				//double int_value = param->app_value;
				QString int_name = param->getObjName();
				QString int_caption = param->getTitle();
				int int_value = param->getAppValue();
				memo += QString("%1 = %2\t; %3\n").arg(int_name).arg(int_value).arg(int_caption);
			}			
		}	
		memo += QString("\n\n");

		 // Channels & settings
		for (int i = 0; i < tool_channels.count(); i++)
		{
			ToolChannel *channel = tool_channels[i];
			int channel_id = channel->channel_id;
			QString channel_name = channel->name;
			QString channel_type = channel->data_type;
			int channel_frq_set_num = channel->frq_set_num;
			int frq1 = channel->frq1;
			int frq2 = channel->frq2;
			int frq3 = channel->frq3;
			int frq4 = channel->frq4;
			int frq5 = channel->frq5;
			int frq6 = channel->frq6;
			int addr_rx = channel->addr_rx;
			int addr_tx = channel->addr_tx;
			double displ = channel->depth_displ;
			double norm_coef1 = channel->normalize_coef1;
			double norm_coef2 = channel->normalize_coef2;
			double meas_frq = channel->meas_frq;
			double field_gradient = channel->field_gradient;
			
			memo += QString("[channel#%1]\n").arg(channel_id);
			memo += QString("type=%1\n").arg(channel_type);
			memo += QString("channel_id=%1\n").arg(channel_id);
			memo += QString("descp=%1\n").arg(channel_name);
			memo += QString("frq_set_num=%1\n").arg(channel_frq_set_num);
			memo += QString("frq1=%1\n").arg(frq1);
			memo += QString("frq2=%1\n").arg(frq2);
			memo += QString("frq3=%1\n").arg(frq3);
			memo += QString("frq4=%1\n").arg(frq4);
			memo += QString("frq5=%1\n").arg(frq5);
			memo += QString("addr_rx=%1\n").arg(addr_rx);
			memo += QString("addr_tx=%1\n").arg(addr_tx);
			memo += QString("depth_displ=%1\n").arg(displ);
			memo += QString("normalize_coef1=%1\n").arg(norm_coef1);
			memo += QString("normalize_coef2=%1\n").arg(norm_coef2);
			memo += QString("meas_frq=%1\n").arg(meas_frq);
			memo += QString("field_gradient=%1\n").arg(field_gradient);
			memo += QString("\n\n");
		}
		
		memo += QString("[Sequence]\n");
		memo += QString("Name = %1\n").arg(cur_seq->name);
		memo += QString("Author = %1\n").arg(cur_seq->author);
		memo += QString("\n\n");
		
		memo += QString("[ScannedQuantity]\n");
		if (current_tool.scanned_quantity == ScannedQuantity::Depth) memo += QString("Quantity = %1\n").arg("Depth");
		else if (current_tool.scanned_quantity == ScannedQuantity::Distance) memo += QString("Quantity = %1\n").arg("Distance");
		else if (current_tool.scanned_quantity == ScannedQuantity::Time) memo += QString("Quantity = %1\n").arg("Time");
		else if (current_tool.scanned_quantity == ScannedQuantity::Temperature) memo += QString("Quantity = %1\n").arg("Temperature");
		else if (current_tool.scanned_quantity == ScannedQuantity::Concentration) memo += QString("Quantity = %1\n").arg("Concentration");
		memo += QString("\n\n");

		memo += "[DataX]\n";
		QString d_str = " NAN             ";
		if (depth.first) 
		{
			d_str = QString::number(depth.second, 'E', 6);
			if (depth.second >= 0) d_str = QString(" %1%2").arg(d_str).arg("    ");
			else d_str = QString("%1%2").arg(d_str).arg("    ");
		}
		memo += d_str;

		memo += QString("%1;    ").arg(ctime_str);

		for (int i = 0; i < dss.count(); i++)
		{
			DataSet *ds = dss[i];
			uint8_t comm_id = ds->getDataCode();
			//if (!dataIsExportingToFile(comm_id)) continue;

			switch (comm_id)
			{
			case DT_SGN_SE_ORG:
			case DT_NS_SE_ORG:
			case DT_SGN_FID_ORG:
			case DT_NS_FID_ORG:
			case DT_SGN_SE:
			case DT_NS_SE:
			case DT_NS_QUAD_FID_RE:
			case DT_NS_QUAD_FID_IM:
			case DT_NS_QUAD_SE_RE:
			case DT_NS_QUAD_SE_IM:
			case DT_SGN_QUAD_FID_RE:
			case DT_SGN_QUAD_FID_IM:
			case DT_SGN_QUAD_SE_RE:
			case DT_SGN_QUAD_SE_IM:
			case DT_NS_FFT_FID_RE:
			case DT_NS_FFT_SE_RE:
			case DT_SGN_FFT_FID_RE:
			case DT_SGN_FFT_SE_RE:
			case DT_NS_FFT_FID_IM:
			case DT_NS_FFT_SE_IM:
			case DT_SGN_FFT_FID_IM:
			case DT_SGN_FFT_SE_IM:
			case DT_SGN_RELAX:
			case DT_SGN_RELAX2:
			case DT_SGN_RELAX3:			
			case DT_SOLID_ECHO:
			case DT_T1T2_NMR:
			case DT_DsT2_NMR:
			case DT_SGN_POWER_SE:		
			case DT_SGN_POWER_FID:		
			case DT_NS_POWER_SE:		
			case DT_NS_POWER_FID:		
			case DT_SGN_FFT_FID_AM:		
			case DT_NS_FFT_FID_AM:		
			case DT_SGN_FFT_SE_AM:		
			case DT_NS_FFT_SE_AM:		
			case DT_GAMMA:		
			case DT_DIEL:
			case DT_AFR1_RX:
			case DT_AFR2_RX:
			case DT_AFR3_RX:
			case DT_RFP:
			case DT_RFP2:
			case DT_DU_T:
			case DT_PU_T:
			case DT_TU_T:
				{
					QString ds_name = ds->getDataName();
					QStringList dn_list = ds_name.split("#");
					ds_name = dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt());
					memo += "'" + ds_name + "'    ";
										
					int x_size = full_xdata[i].count();
					QVector<double> *x_vec = &full_xdata[i];

					x_full_size.push_back(x_size);

					for (int j = 0; j < x_size; j++)
					{
						double val = x_vec->at(j);
						QString x_str = "";
						if (j < x_size-1)
						{					
							x_str = QString::number(val, 'E', 6);
							if (val >= 0) x_str = QString(" %1%2").arg(x_str).arg("    ");
							else x_str = QString("%1%2").arg(x_str).arg("    ");
						}
						else if (j < x_size)
						{
							x_str = QString::number(val, 'E', 6);
							if (val >= 0) x_str = QString(" %1%2").arg(x_str).arg(";    ");
							else x_str = QString("%1%2").arg(x_str).arg(";    ");
						}

						memo += x_str;
					}
					break;
				}			
			default: break;
			}
		}
		memo += QString("\n\n");
		memo += "[DataY]\n";
	}

	QString d_str = " NAN             ";
	if (depth.first) 
	{
		d_str = QString::number(depth.second, 'E', 6);
		if (depth.second >= 0) d_str = QString(" %1%2").arg(d_str).arg("    ");
		else d_str = QString("%1%2").arg(d_str).arg("    ");
	}
	QString str_data = "";
	str_data += d_str;
	str_data += QString("%1;    ").arg(ctime_str);


	for (int i = 0; i < dss.count(); i++)
	{
		DataSet *ds = dss[i];
		uint8_t comm_id = ds->getDataCode();
		//if (!dataIsExportingToFile(comm_id)) continue;

		QVector<uint8_t> gap_vec = gap[i];
		switch (comm_id)
		{
		case DT_SGN_SE_ORG:			
		case DT_NS_SE_ORG:			
		case DT_SGN_FID_ORG:		
		case DT_NS_FID_ORG:			
		case DT_SGN_SE:				
		case DT_NS_SE:				
		case DT_NS_QUAD_FID_RE:		
		case DT_NS_QUAD_FID_IM:		
		case DT_NS_QUAD_SE_RE:		
		case DT_NS_QUAD_SE_IM:		
		case DT_SGN_QUAD_FID_RE:	
		case DT_SGN_QUAD_FID_IM:	
		case DT_SGN_QUAD_SE_RE:		
		case DT_SGN_QUAD_SE_IM:		
		case DT_NS_FFT_FID_RE:		
		case DT_NS_FFT_SE_RE:		
		case DT_SGN_FFT_FID_RE:		
		case DT_SGN_FFT_SE_RE:		
		case DT_NS_FFT_FID_IM:		
		case DT_NS_FFT_SE_IM:		
		case DT_SGN_FFT_FID_IM:		
		case DT_SGN_FFT_SE_IM:		
		case DT_SGN_RELAX:	
		case DT_SGN_RELAX2:
		case DT_SGN_RELAX3:		
		case DT_SOLID_ECHO:
		case DT_T1T2_NMR:
		case DT_DsT2_NMR:
		case DT_SGN_POWER_SE:		
		case DT_SGN_POWER_FID:		
		case DT_NS_POWER_SE:		
		case DT_NS_POWER_FID:		
		case DT_SGN_FFT_FID_AM:		
		case DT_NS_FFT_FID_AM:		
		case DT_SGN_FFT_SE_AM:		
		case DT_NS_FFT_SE_AM:		
		case DT_GAMMA:	
		case DT_DIEL:
		case DT_AFR1_RX:
		case DT_AFR2_RX:
		case DT_AFR3_RX:
		case DT_RFP:
		case DT_RFP2:
		case DT_DU_T:
		case DT_PU_T:
		case DT_TU_T:
			{
				//QVector<double> *x_vec = ds->getXData();
				QVector<double> *y_vec = ds->getYData();

				QString ds_name = ds->getDataName();
				QStringList dn_list = ds_name.split("#");
				ds_name = dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt());
				str_data += "'" + ds_name + "'    ";

				int cnt = 0;
				//for (int j = 0; j < y_vec->size(); j++)
				for (int j = 0; j < gap_vec.count(); j++)
				{			
					if (cnt < y_vec->size()) 
					{
						if (gap_vec[j] == DATA_OK)
						{
							QString str_value = "";
							double val = y_vec->at(cnt);
							if (cnt < y_vec->size()-1)
							{
								QString str_v = QString::number(val, 'E', 6);
								if (val >= 0) str_value = QString(" %1%2").arg(str_v).arg("    ");
								else str_value = QString("%1%2").arg(str_v).arg("    ");
								str_data += str_value;
							}
							else if (cnt < y_vec->size())
							{
								QString str_v = QString::number(val, 'E', 6);
								if (val >= 0) str_value = QString(" %1%2").arg(str_v).arg(";    ");
								else str_value = QString("%1%2").arg(str_v).arg(";    ");				
								str_data += str_value;
							}	
							cnt++;						
						}
						else
						{						
							QString str_value = " NAN             ";										
							str_data += str_value;
						}		
					}			
				}
			}		
		default: break;
		}		
	}
	str_data += QString("\n");
		
	QFile data_to_save(file_name);
	if (data_file_size == 0)	
	{		
		if (!data_to_save.open(QIODevice::WriteOnly))
		{
			placeInfoToStatusBar(QString("<font color=red><b>Warning! Cannot open file '%1' to save data!</b></font>").arg(file_name));
			return;
		}
	}
	else
	{
		if (!data_to_save.open(QIODevice::Append))
		{
			placeInfoToStatusBar(QString("<font color=red><b>Warning! Cannot open file '%1' to save data!</b></font>").arg(file_name));
			return;
		}
	}

	QTextStream stream(&data_to_save);
	stream << (memo + str_data);
	data_to_save.close();		
}*/


void MainWindow::exportData(DataSets &dss, QList<QVector<uint8_t> > &gap, QList<QVector<double> > &full_xdata)
{
	if (dss.isEmpty()) return;
	if (!nmrtool_state) return;
	if (!save_data_attrs.to_save)
	{
		placeInfoToStatusBar(QString("<font color=red><b>Measured data is not saved to the file!</b></font>"));			// не записывать данные !
		return;
	}
		
	//profiler.tic(4, "MainWindow::exportData()");
	
	JSeqObject *executingJSeq = expScheduler->getExecutingJSeqObject();
	if (!executingJSeq) return;
	LUSI::Sequence *cur_seq = executingJSeq->lusi_Seq;

	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_str = ctime.toString("d-M-yyyy_hh:mm:ss");

	uint32_t uid = dss[0]->getUId();
	QPair<bool, double> depth = dss[0]->getDepth();

	static QVector<int> x_full_size; 

	bool xdata_is_ok = true;
	QString memo = "";

	static QString temp_memo = "";

	QString file_name = expScheduler->getDataFile();	
	if (file_name.isEmpty()) return;

	QFileInfo data_file_info(file_name);
	int data_file_size = data_file_info.size();
	if (data_file_size == 0)
	//if (start_data_export)
	{		
		memo = "[Description]\n";
		memo += QString("Device = %1\n").arg(current_tool.type);
		memo += QString("DeviceUId = %1\n").arg(current_tool.id);
		/*memo += QString("Object = %1\n").arg("Скважина X");
		memo += QString("Company = %1\n").arg("КФУ");
		memo += QString("Experiment = %1\n").arg("");
		memo += QString("Operator = %1\n").arg("");			
		memo += QString("DateTime = %1\n").arg(ctime_str);*/
		memo += QString("Object = %1, %2, %3, %4, %5, %6\n").arg(exper_attrs.well_name).arg(exper_attrs.well_UID).arg(exper_attrs.field_name).arg(exper_attrs.location).arg(exper_attrs.province).arg(exper_attrs.country);
		/*memo += QString("UNIQUE WELL ID = %1\n").arg(exper_attrs.well_UID);
		memo += QString("WELL = %1\n").arg(exper_attrs.well_name);
		memo += QString("FIELD = %1\n").arg(exper_attrs.field_name);
		memo += QString("LOCATION = %1\n").arg(exper_attrs.location);	
		memo += QString("PROVINCE = %1\n").arg(exper_attrs.province);
		memo += QString("COUNTRY = %1\n").arg(exper_attrs.country);*/
		memo += QString("Company = %1, %2\n").arg(exper_attrs.company).arg(exper_attrs.service_company);
		/*memo += QString("SERVICE COMPANY = %1\n").arg(exper_attrs.service_company);
		memo += QString("COMPANY = %1\n").arg(exper_attrs.company);*/
		memo += QString("Operator = %1\n").arg(exper_attrs.oper);
		//memo += QString("DATE = %1\n").arg(exper_attrs.date.toString());
		memo += QString("DateTime = %1\n").arg(ctime_str);
		memo += QString("\n\n");

		memo += "[Format]\n";
		memo += QString("Version = %1\n").arg("2.0");
		memo += QString("DecimalSymbol = %1\n").arg(".");
		memo += QString("DataSeparator = %1\n").arg(" ");
		memo += QString("DataSetSeparator = %1\n").arg(";");
		memo += QString("\n\n");

		memo += "[Intervals]\n";
		for (int i = 0; i < cur_seq->param_list.count(); i++)
		{			
			LUSI::Parameter *param = cur_seq->param_list[i];			
			if (!param->getReadOnly())
			{
				//QString int_name = param->name;				
				//QString int_caption = param->caption;				
				//double int_value = param->app_value;
				QString int_name = param->getObjName();
				QString int_caption = param->getTitle();
				int int_value = param->getAppValue();
				memo += QString("%1 = %2\t; %3\n").arg(int_name).arg(int_value).arg(int_caption);
			}			
		}	
		memo += QString("\n\n");

		memo += QString("[Sequence]\n");
		memo += QString("Name = %1\n").arg(cur_seq->name);
		memo += QString("Author = %1\n").arg(cur_seq->author);
		memo += QString("\n\n");

		
		// if autocalibration is on, then Channel data and experimental data are saved after the autocalibration period is expired.
		// For this goal DataX, DataY and DataColumns are collected to temp. QString buffer.
		
		autocalibration_state = logging_widget->isCalibrationON();
		temp_memo.clear();

		if (!autocalibration_state)
		{
			// Channels & settings
			for (int i = 0; i < tool_channels.count(); i++)
			{
				ToolChannel *channel = tool_channels[i];
				int channel_id = channel->channel_id;
				QString channel_name = channel->name;
				QString channel_type = channel->data_type;
				int channel_frq_set_num = channel->frq_set_num;
				int frq1 = channel->frq1;
				int frq2 = channel->frq2;
				int frq3 = channel->frq3;
				int frq4 = channel->frq4;
				int frq5 = channel->frq5;
				int frq6 = channel->frq6;
				int addr_rx = channel->addr_rx;
				int addr_tx = channel->addr_tx;
				double displ = channel->depth_displ;
				double norm_coef1 = channel->normalize_coef1;
				double norm_coef2 = channel->normalize_coef2;
				double meas_frq = channel->meas_frq;
				double field_gradient = channel->field_gradient;

				memo += QString("[channel#%1]\n").arg(channel_id);
				memo += QString("type=%1\n").arg(channel_type);
				memo += QString("channel_id=%1\n").arg(channel_id);
				memo += QString("descp=%1\n").arg(channel_name);
				memo += QString("frq_set_num=%1\n").arg(channel_frq_set_num);
				memo += QString("frq1=%1\n").arg(frq1);
				memo += QString("frq2=%1\n").arg(frq2);
				memo += QString("frq3=%1\n").arg(frq3);
				memo += QString("frq4=%1\n").arg(frq4);
				memo += QString("frq5=%1\n").arg(frq5);
				memo += QString("addr_rx=%1\n").arg(addr_rx);
				memo += QString("addr_tx=%1\n").arg(addr_tx);
				memo += QString("depth_displ=%1\n").arg(displ);
				memo += QString("normalize_coef1=%1\n").arg(norm_coef1);
				memo += QString("normalize_coef2=%1\n").arg(norm_coef2);
				memo += QString("meas_frq=%1\n").arg(meas_frq);
				memo += QString("field_gradient=%1\n").arg(field_gradient);
				memo += QString("\n\n");
			}

			QString str_quantity = "                ";	// 16 spaces
			memo += QString("[ScannedQuantity]\n");
			if (current_tool.scanned_quantity == ScannedQuantity::Depth)				{ memo += QString("Quantity = %1\n").arg("Depth"); str_quantity.replace(0,6, "#DEPTH"); }
			else if (current_tool.scanned_quantity == ScannedQuantity::Distance)		{ memo += QString("Quantity = %1\n").arg("Distance"); str_quantity.replace(0,9, "#DISTANCE"); }
			else if (current_tool.scanned_quantity == ScannedQuantity::Time)			{ memo += QString("Quantity = %1\n").arg("Time"); str_quantity.replace(0,5, "#TIME"); }
			else if (current_tool.scanned_quantity == ScannedQuantity::Temperature)		{ memo += QString("Quantity = %1\n").arg("Temperature"); str_quantity.replace(0,5, "#TEMP"); }
			else if (current_tool.scanned_quantity == ScannedQuantity::Concentration)	{ memo += QString("Quantity = %1\n").arg("Concentration"); str_quantity.replace(0,5, "#CONC"); }
			memo += QString("\n\n");


			memo += "[DataColumns]\n";
			memo += str_quantity;
			memo += "#DATE_TIME             ";
			for (int i = 0; i < dss.count(); i++)
			{
				DataSet *ds = dss[i];
				uint8_t comm_id = ds->getDataCode();

				switch (comm_id)
				{
				case DT_SGN_SE_ORG:
				case DT_NS_SE_ORG:
				case DT_SGN_FID_ORG:
				case DT_NS_FID_ORG:
				case DT_SGN_SE:
				case DT_NS_SE:
				case DT_NS_QUAD_FID_RE:
				case DT_NS_QUAD_FID_IM:
				case DT_NS_QUAD_SE_RE:
				case DT_NS_QUAD_SE_IM:
				case DT_SGN_QUAD_FID_RE:
				case DT_SGN_QUAD_FID_IM:
				case DT_SGN_QUAD_SE_RE:
				case DT_SGN_QUAD_SE_IM:
				case DT_NS_FFT_FID_RE:
				case DT_NS_FFT_SE_RE:
				case DT_SGN_FFT_FID_RE:
				case DT_SGN_FFT_SE_RE:
				case DT_NS_FFT_FID_IM:
				case DT_NS_FFT_SE_IM:
				case DT_SGN_FFT_FID_IM:
				case DT_SGN_FFT_SE_IM:
				case DT_SGN_RELAX:
				case DT_SGN_RELAX2:
				case DT_SGN_RELAX3:			
				case DT_SOLID_ECHO:
				case DT_T1T2_NMR:
				case DT_DsT2_NMR:
				case DT_SGN_POWER_SE:		
				case DT_SGN_POWER_FID:		
				case DT_NS_POWER_SE:		
				case DT_NS_POWER_FID:		
				case DT_SGN_FFT_FID_AM:		
				case DT_NS_FFT_FID_AM:		
				case DT_SGN_FFT_SE_AM:		
				case DT_NS_FFT_SE_AM:		
				case DT_GAMMA:		
				case DT_DIEL:
				case DT_AFR1_RX:
				case DT_AFR2_RX:
				case DT_AFR3_RX:
				case DT_RFP:
				case DT_RFP2:
				case DT_DU_T:
				case DT_PU_T:
				case DT_TU_T:
					{
						QString ds_name = ds->getDataName();
						QStringList dn_list = ds_name.split("#");
						ds_name = "'" + dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt()) + "'   ";					

						int ds_name_len = ds_name.length();
						QString str_ds_name = QString(" ").repeated(ds_name_len-1);
						str_ds_name.replace(0,13, "#DATASET_NAME");
						memo += str_ds_name;	

						memo += "#DATA_TYPE   #CHANNEL   #GROUP_INDEX   #DATA_NUM   ";					
						if (ds->TE() > 0 || ds->TD() > 0 || ds->TW() > 0) memo += "#TE            #TW            #TD             "; 

						int x_size = full_xdata[i].count();
						for (int j = 0; j < x_size; j++) 
						{
							QString str_num = "#" + QString("%1").arg(j+1);
							int str_num_len = str_num.length();
							str_num = QString(" ").repeated(16).replace(0,str_num_len,str_num);
							memo += str_num;
						}					
						break;
					}
				default: break;
				}
			}
			memo += "\n\n";

			memo += "[DataX]\n";
			QString d_str = " NAN            ";
			if (depth.first) 
			{
				d_str = QString::number(depth.second, 'E', 6);
				if (depth.second >= 0) d_str = QString(" %1%2").arg(d_str).arg("   ");
				else d_str = QString("%1%2").arg(d_str).arg("   ");
			}
			memo += d_str;

			memo += QString("%1;   ").arg(ctime_str);

			for (int i = 0; i < dss.count(); i++)
			{
				DataSet *ds = dss[i];
				uint8_t comm_id = ds->getDataCode();

				switch (comm_id)
				{
				case DT_SGN_SE_ORG:
				case DT_NS_SE_ORG:
				case DT_SGN_FID_ORG:
				case DT_NS_FID_ORG:
				case DT_SGN_SE:
				case DT_NS_SE:
				case DT_NS_QUAD_FID_RE:
				case DT_NS_QUAD_FID_IM:
				case DT_NS_QUAD_SE_RE:
				case DT_NS_QUAD_SE_IM:
				case DT_SGN_QUAD_FID_RE:
				case DT_SGN_QUAD_FID_IM:
				case DT_SGN_QUAD_SE_RE:
				case DT_SGN_QUAD_SE_IM:
				case DT_NS_FFT_FID_RE:
				case DT_NS_FFT_SE_RE:
				case DT_SGN_FFT_FID_RE:
				case DT_SGN_FFT_SE_RE:
				case DT_NS_FFT_FID_IM:
				case DT_NS_FFT_SE_IM:
				case DT_SGN_FFT_FID_IM:
				case DT_SGN_FFT_SE_IM:
				case DT_SGN_RELAX:
				case DT_SGN_RELAX2:
				case DT_SGN_RELAX3:			
				case DT_SOLID_ECHO:
				case DT_T1T2_NMR:
				case DT_DsT2_NMR:
				case DT_SGN_POWER_SE:		
				case DT_SGN_POWER_FID:		
				case DT_NS_POWER_SE:		
				case DT_NS_POWER_FID:		
				case DT_SGN_FFT_FID_AM:		
				case DT_NS_FFT_FID_AM:		
				case DT_SGN_FFT_SE_AM:		
				case DT_NS_FFT_SE_AM:		
				case DT_GAMMA:		
				case DT_DIEL:
				case DT_AFR1_RX:
				case DT_AFR2_RX:
				case DT_AFR3_RX:
				case DT_RFP:
				case DT_RFP2:
				case DT_DU_T:
				case DT_PU_T:
				case DT_TU_T:
					{
						QString ds_name = ds->getDataName();
						QStringList dn_list = ds_name.split("#");
						ds_name = dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt());
						memo += "'" + ds_name + "'   ";

						memo += toAlignedSpacedString(comm_id, 13, " ");				// for #DATA_TYPE
						memo += toAlignedSpacedString(ds->getChannelId(), 11, " ");		// for #CHANNEL
						memo += toAlignedSpacedString(ds->getGroupIndex(), 15, " ");	// for #GROUP_INDEX
						memo += toAlignedSpacedString(ds->getDataNum(), 12, " ");		// for #DATA_NUM

						if (ds->TE() > 0 || ds->TW() > 0 || ds->TD() > 0) 
						{
							memo += QString::number(ds->TE(), 'E', 6) + "   ";
							memo += QString::number(ds->TW(), 'E', 6) + "   ";
							memo += QString::number(ds->TD(), 'E', 6) + "   ";
						}

						int x_size = full_xdata[i].count();
						QVector<double> *x_vec = &full_xdata[i];
						x_full_size.push_back(x_size);
						for (int j = 0; j < x_size; j++)
						{
							double val = x_vec->at(j);
							QString x_str = "";
							if (j < x_size-1)
							{					
								x_str = QString::number(val, 'E', 6);
								if (val >= 0) x_str = QString(" %1%2").arg(x_str).arg("   ");
								else x_str = QString("%1%2").arg(x_str).arg("   ");
							}
							else if (j < x_size)
							{
								x_str = QString::number(val, 'E', 6);
								if (val >= 0) x_str = QString(" %1%2").arg(x_str).arg(";   ");
								else x_str = QString("%1%2").arg(x_str).arg(";   ");
							}

							memo += x_str;
						}
						break;
					}			
				default: break;
				}
			}
			memo += QString("\n\n");
			memo += "[DataY]\n";		
		}		
	}
	
	QString d_str = " NAN            ";
	if (depth.first) 
	{
		d_str = QString::number(depth.second, 'E', 6);
		if (depth.second >= 0) d_str = QString(" %1%2").arg(d_str).arg("   ");
		else d_str = QString("%1%2").arg(d_str).arg("   ");
	}
	QString str_data = "";
	str_data += d_str;
	str_data += QString("%1;   ").arg(ctime_str);


	for (int i = 0; i < dss.count(); i++)
	{
		DataSet *ds = dss[i];
		uint8_t comm_id = ds->getDataCode();
		//if (!dataIsExportingToFile(comm_id)) continue;

		QVector<uint8_t> gap_vec = gap[i];
		switch (comm_id)
		{
		case DT_SGN_SE_ORG:			
		case DT_NS_SE_ORG:			
		case DT_SGN_FID_ORG:		
		case DT_NS_FID_ORG:			
		case DT_SGN_SE:				
		case DT_NS_SE:				
		case DT_NS_QUAD_FID_RE:		
		case DT_NS_QUAD_FID_IM:		
		case DT_NS_QUAD_SE_RE:		
		case DT_NS_QUAD_SE_IM:		
		case DT_SGN_QUAD_FID_RE:	
		case DT_SGN_QUAD_FID_IM:	
		case DT_SGN_QUAD_SE_RE:		
		case DT_SGN_QUAD_SE_IM:		
		case DT_NS_FFT_FID_RE:		
		case DT_NS_FFT_SE_RE:		
		case DT_SGN_FFT_FID_RE:		
		case DT_SGN_FFT_SE_RE:		
		case DT_NS_FFT_FID_IM:		
		case DT_NS_FFT_SE_IM:		
		case DT_SGN_FFT_FID_IM:		
		case DT_SGN_FFT_SE_IM:		
		case DT_SGN_RELAX:	
		case DT_SGN_RELAX2:
		case DT_SGN_RELAX3:		
		case DT_SOLID_ECHO:
		case DT_T1T2_NMR:
		case DT_DsT2_NMR:
		case DT_SGN_POWER_SE:		
		case DT_SGN_POWER_FID:		
		case DT_NS_POWER_SE:		
		case DT_NS_POWER_FID:		
		case DT_SGN_FFT_FID_AM:		
		case DT_NS_FFT_FID_AM:		
		case DT_SGN_FFT_SE_AM:		
		case DT_NS_FFT_SE_AM:		
		case DT_GAMMA:	
		case DT_DIEL:
		case DT_AFR1_RX:
		case DT_AFR2_RX:
		case DT_AFR3_RX:
		case DT_RFP:
		case DT_RFP2:
		case DT_DU_T:
		case DT_PU_T:
		case DT_TU_T:
			{
				//QVector<double> *x_vec = ds->getXData();
				QVector<double> *y_vec = ds->getYData();

				QString ds_name = ds->getDataName();
				QStringList dn_list = ds_name.split("#");
				ds_name = dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt());
				str_data += "'" + ds_name + "'   ";

				str_data += toAlignedSpacedString(comm_id, 13, " ");				// for #DATA_TYPE
				str_data += toAlignedSpacedString(ds->getChannelId(), 11, " ");		// for #CHANNEL
				str_data += toAlignedSpacedString(ds->getGroupIndex(), 15, " ");	// for #GROUP_INDEX
				str_data += toAlignedSpacedString(ds->getDataNum(), 12, " ");		// for #DATA_NUM

				if (ds->TE() > 0 || ds->TW() > 0 || ds->TD() > 0) 
				{
					str_data += QString::number(ds->TE(), 'E', 6) + "   ";
					str_data += QString::number(ds->TW(), 'E', 6) + "   ";
					str_data += QString::number(ds->TD(), 'E', 6) + "   ";
				}

				int cnt = 0;
				//for (int j = 0; j < y_vec->size(); j++)
				for (int j = 0; j < gap_vec.count(); j++)
				{			
					if (cnt < y_vec->size()) 
					{
						if (gap_vec[j] == DATA_OK)
						{
							QString str_value = "";
							double val = y_vec->at(cnt);
							if (cnt < y_vec->size()-1)
							{
								QString str_v = QString::number(val, 'E', 6);
								if (val >= 0) str_value = QString(" %1%2").arg(str_v).arg("   ");
								else str_value = QString("%1%2").arg(str_v).arg("   ");
								str_data += str_value;
							}
							else if (cnt < y_vec->size())
							{
								QString str_v = QString::number(val, 'E', 6);
								if (val >= 0) str_value = QString(" %1%2").arg(str_v).arg(";   ");
								else str_value = QString("%1%2").arg(str_v).arg(";   ");				
								str_data += str_value;
							}	
							cnt++;						
						}
						else
						{						
							QString str_value = " NAN             ";										
							str_data += str_value;
						}		
					}			
				}
			}		
		default: break;
		}		
	}
	str_data += QString("\n");

	if (autocalibration_state) temp_memo += str_data;
	else if (temp_memo.isEmpty())
	{
		memo += str_data;
	}
	else
	{
		QString memo2 = "";
		// Channels & settings
		for (int i = 0; i < tool_channels.count(); i++)
		{
			ToolChannel *channel = tool_channels[i];
			int channel_id = channel->channel_id;
			QString channel_name = channel->name;
			QString channel_type = channel->data_type;
			int channel_frq_set_num = channel->frq_set_num;
			int frq1 = channel->frq1;
			int frq2 = channel->frq2;
			int frq3 = channel->frq3;
			int frq4 = channel->frq4;
			int frq5 = channel->frq5;
			int frq6 = channel->frq6;
			int addr_rx = channel->addr_rx;
			int addr_tx = channel->addr_tx;
			double displ = channel->depth_displ;
			double norm_coef1 = channel->normalize_coef1;
			double norm_coef2 = channel->normalize_coef2;
			double meas_frq = channel->meas_frq;
			double field_gradient = channel->field_gradient;

			memo2 += QString("[channel#%1]\n").arg(channel_id);
			memo2 += QString("type=%1\n").arg(channel_type);
			memo2 += QString("channel_id=%1\n").arg(channel_id);
			memo2 += QString("descp=%1\n").arg(channel_name);
			memo2 += QString("frq_set_num=%1\n").arg(channel_frq_set_num);
			memo2 += QString("frq1=%1\n").arg(frq1);
			memo2 += QString("frq2=%1\n").arg(frq2);
			memo2 += QString("frq3=%1\n").arg(frq3);
			memo2 += QString("frq4=%1\n").arg(frq4);
			memo2 += QString("frq5=%1\n").arg(frq5);
			memo2 += QString("addr_rx=%1\n").arg(addr_rx);
			memo2 += QString("addr_tx=%1\n").arg(addr_tx);
			memo2 += QString("depth_displ=%1\n").arg(displ);
			memo2 += QString("normalize_coef1=%1\n").arg(norm_coef1);
			memo2 += QString("normalize_coef2=%1\n").arg(norm_coef2);
			memo2 += QString("meas_frq=%1\n").arg(meas_frq);
			memo2 += QString("field_gradient=%1\n").arg(field_gradient);
			memo2 += QString("\n\n");
		}

		QString str_quantity = "                ";	// 16 spaces
		memo2 += QString("[ScannedQuantity]\n");
		if (current_tool.scanned_quantity == ScannedQuantity::Depth)				{ memo2 += QString("Quantity = %1\n").arg("Depth"); str_quantity.replace(0,6, "#DEPTH"); }
		else if (current_tool.scanned_quantity == ScannedQuantity::Distance)		{ memo2 += QString("Quantity = %1\n").arg("Distance"); str_quantity.replace(0,9, "#DISTANCE"); }
		else if (current_tool.scanned_quantity == ScannedQuantity::Time)			{ memo2 += QString("Quantity = %1\n").arg("Time"); str_quantity.replace(0,5, "#TIME"); }
		else if (current_tool.scanned_quantity == ScannedQuantity::Temperature)		{ memo2 += QString("Quantity = %1\n").arg("Temperature"); str_quantity.replace(0,5, "#TEMP"); }
		else if (current_tool.scanned_quantity == ScannedQuantity::Concentration)	{ memo2 += QString("Quantity = %1\n").arg("Concentration"); str_quantity.replace(0,5, "#CONC"); }
		memo2 += QString("\n\n");


		memo2 += "[DataColumns]\n";
		memo2 += str_quantity;
		memo2 += "#DATE_TIME             ";
		for (int i = 0; i < dss.count(); i++)
		{
			DataSet *ds = dss[i];
			uint8_t comm_id = ds->getDataCode();

			switch (comm_id)
			{
			case DT_SGN_SE_ORG:
			case DT_NS_SE_ORG:
			case DT_SGN_FID_ORG:
			case DT_NS_FID_ORG:
			case DT_SGN_SE:
			case DT_NS_SE:
			case DT_NS_QUAD_FID_RE:
			case DT_NS_QUAD_FID_IM:
			case DT_NS_QUAD_SE_RE:
			case DT_NS_QUAD_SE_IM:
			case DT_SGN_QUAD_FID_RE:
			case DT_SGN_QUAD_FID_IM:
			case DT_SGN_QUAD_SE_RE:
			case DT_SGN_QUAD_SE_IM:
			case DT_NS_FFT_FID_RE:
			case DT_NS_FFT_SE_RE:
			case DT_SGN_FFT_FID_RE:
			case DT_SGN_FFT_SE_RE:
			case DT_NS_FFT_FID_IM:
			case DT_NS_FFT_SE_IM:
			case DT_SGN_FFT_FID_IM:
			case DT_SGN_FFT_SE_IM:
			case DT_SGN_RELAX:
			case DT_SGN_RELAX2:
			case DT_SGN_RELAX3:			
			case DT_SOLID_ECHO:
			case DT_T1T2_NMR:
			case DT_DsT2_NMR:
			case DT_SGN_POWER_SE:		
			case DT_SGN_POWER_FID:		
			case DT_NS_POWER_SE:		
			case DT_NS_POWER_FID:		
			case DT_SGN_FFT_FID_AM:		
			case DT_NS_FFT_FID_AM:		
			case DT_SGN_FFT_SE_AM:		
			case DT_NS_FFT_SE_AM:		
			case DT_GAMMA:		
			case DT_DIEL:
			case DT_AFR1_RX:
			case DT_AFR2_RX:
			case DT_AFR3_RX:
			case DT_RFP:
			case DT_RFP2:
			case DT_DU_T:
			case DT_PU_T:
			case DT_TU_T:
				{
					QString ds_name = ds->getDataName();
					QStringList dn_list = ds_name.split("#");
					ds_name = "'" + dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt()) + "'   ";					

					int ds_name_len = ds_name.length();
					QString str_ds_name = QString(" ").repeated(ds_name_len-1);
					str_ds_name.replace(0,13, "#DATASET_NAME");
					memo2 += str_ds_name;	

					memo2 += "#DATA_TYPE   #CHANNEL   #GROUP_INDEX   #DATA_NUM   ";					
					if (ds->TE() > 0 || ds->TD() > 0 || ds->TW() > 0) memo2 += "#TE            #TW            #TD             "; 

					int x_size = full_xdata[i].count();
					for (int j = 0; j < x_size; j++) 
					{
						QString str_num = "#" + QString("%1").arg(j+1);
						int str_num_len = str_num.length();
						str_num = QString(" ").repeated(16).replace(0,str_num_len,str_num);
						memo2 += str_num;
					}					
					break;
				}
			default: break;
			}
		}
		memo2 += "\n\n";

		memo2 += "[DataX]\n";
		QString d_str = " NAN            ";
		if (depth.first) 
		{
			d_str = QString::number(depth.second, 'E', 6);
			if (depth.second >= 0) d_str = QString(" %1%2").arg(d_str).arg("   ");
			else d_str = QString("%1%2").arg(d_str).arg("   ");
		}
		memo2 += d_str;

		memo2 += QString("%1;   ").arg(ctime_str);

		for (int i = 0; i < dss.count(); i++)
		{
			DataSet *ds = dss[i];
			uint8_t comm_id = ds->getDataCode();

			switch (comm_id)
			{
			case DT_SGN_SE_ORG:
			case DT_NS_SE_ORG:
			case DT_SGN_FID_ORG:
			case DT_NS_FID_ORG:
			case DT_SGN_SE:
			case DT_NS_SE:
			case DT_NS_QUAD_FID_RE:
			case DT_NS_QUAD_FID_IM:
			case DT_NS_QUAD_SE_RE:
			case DT_NS_QUAD_SE_IM:
			case DT_SGN_QUAD_FID_RE:
			case DT_SGN_QUAD_FID_IM:
			case DT_SGN_QUAD_SE_RE:
			case DT_SGN_QUAD_SE_IM:
			case DT_NS_FFT_FID_RE:
			case DT_NS_FFT_SE_RE:
			case DT_SGN_FFT_FID_RE:
			case DT_SGN_FFT_SE_RE:
			case DT_NS_FFT_FID_IM:
			case DT_NS_FFT_SE_IM:
			case DT_SGN_FFT_FID_IM:
			case DT_SGN_FFT_SE_IM:
			case DT_SGN_RELAX:
			case DT_SGN_RELAX2:
			case DT_SGN_RELAX3:			
			case DT_SOLID_ECHO:
			case DT_T1T2_NMR:
			case DT_DsT2_NMR:
			case DT_SGN_POWER_SE:		
			case DT_SGN_POWER_FID:		
			case DT_NS_POWER_SE:		
			case DT_NS_POWER_FID:		
			case DT_SGN_FFT_FID_AM:		
			case DT_NS_FFT_FID_AM:		
			case DT_SGN_FFT_SE_AM:		
			case DT_NS_FFT_SE_AM:		
			case DT_GAMMA:		
			case DT_DIEL:
			case DT_AFR1_RX:
			case DT_AFR2_RX:
			case DT_AFR3_RX:
			case DT_RFP:
			case DT_RFP2:
			case DT_DU_T:
			case DT_PU_T:
			case DT_TU_T:
				{
					QString ds_name = ds->getDataName();
					QStringList dn_list = ds_name.split("#");
					ds_name = dn_list.first() + "#" + toAlignedString(5, dn_list.last().toInt());
					memo2 += "'" + ds_name + "'   ";

					memo2 += toAlignedSpacedString(comm_id, 13, " ");				// for #DATA_TYPE
					memo2 += toAlignedSpacedString(ds->getChannelId(), 11, " ");		// for #CHANNEL
					memo2 += toAlignedSpacedString(ds->getGroupIndex(), 15, " ");	// for #GROUP_INDEX
					memo2 += toAlignedSpacedString(ds->getDataNum(), 12, " ");		// for #DATA_NUM

					if (ds->TE() > 0 || ds->TW() > 0 || ds->TD() > 0) 
					{
						memo2 += QString::number(ds->TE(), 'E', 6) + "   ";
						memo2 += QString::number(ds->TW(), 'E', 6) + "   ";
						memo2 += QString::number(ds->TD(), 'E', 6) + "   ";
					}

					int x_size = full_xdata[i].count();
					QVector<double> *x_vec = &full_xdata[i];
					x_full_size.push_back(x_size);
					for (int j = 0; j < x_size; j++)
					{
						double val = x_vec->at(j);
						QString x_str = "";
						if (j < x_size-1)
						{					
							x_str = QString::number(val, 'E', 6);
							if (val >= 0) x_str = QString(" %1%2").arg(x_str).arg("   ");
							else x_str = QString("%1%2").arg(x_str).arg("   ");
						}
						else if (j < x_size)
						{
							x_str = QString::number(val, 'E', 6);
							if (val >= 0) x_str = QString(" %1%2").arg(x_str).arg(";   ");
							else x_str = QString("%1%2").arg(x_str).arg(";   ");
						}

						memo2 += x_str;
					}
					break;
				}			
			default: break;
			}
		}
		memo2 += QString("\n\n");
		memo2 += "[DataY]\n";		

		temp_memo = memo2 + temp_memo;
		memo += temp_memo;
		temp_memo.clear();
	}
	
		
	QFile data_to_save(file_name);
	if (data_file_size == 0)	
	{		
		if (!data_to_save.open(QIODevice::WriteOnly))
		{
			placeInfoToStatusBar(QString("<font color=red><b>Warning! Cannot open file '%1' to save data!</b></font>").arg(file_name));
			return;
		}
	}
	else
	{
		if (!data_to_save.open(QIODevice::Append))
		{
			placeInfoToStatusBar(QString("<font color=red><b>Warning! Cannot open file '%1' to save data!</b></font>").arg(file_name));
			return;
		}
	}

	QTextStream stream(&data_to_save);
	stream << memo;	
	data_to_save.close();		
}


void MainWindow::depthDepthMeterConnected(bool flag)
{
	ui->a_Depthmeter_Connection->setEnabled(!flag);
}

/*
void MainWindow::sendDataToNetClients(const QString &sock_id, int index)
{
	if (dataset_storage->isEmpty()) return;

	DataSets *dss = new DataSets;
	for (int i = 0; i < dataset_storage->size(); i++)
	{
		DataSet *ds = dataset_storage->at(i);
		if (ds->getExpId() == experiment_id)
		{
			if (ds->getUId() > index)
			{
				if (dataIsExportingToOil(ds->getDataCode()))
				{
					DataSet *new_ds = new DataSet;
					ds->copyTo(new_ds);
					dss->append(new_ds);
				}
			}
		}
	}	
	
	if (dss->isEmpty())
	{
		delete dss;
		return;
	}

	tcp_data_manager->dataToSend(sock_id, dss);
}
*/

void MainWindow::sendToSDSP(QByteArray& arr)
{
	if (nmrtool_state) return;

	if (nmrtoolLinker->getConnectionState() == ConnectionState::State_No)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Logging Tool is not connected!"), QMessageBox::Ok);
		return;
	}
	else if (nmrtoolLinker->getConnectionState() == ConnectionState::State_Connecting)
	{
		//int ret = QMessageBox::warning(this, "Warning!", tr("Logging Tool is busy!"), QMessageBox::Ok);
		return;
	}

	setCmdResult(SDSP_DATA, ConnectionState::State_Connecting);
		
	nmrtoolLinker->sendDataToSDSP(arr);
}


void MainWindow::plotData(DataSets &_dss)
{
	if (!nmrtool_state) return;
	if (_dss.count() == 0) return;		

	relax_widget->refreshDataSets();
	osc_widget->refreshDataSets();

	QVector<double> *addy_data = NULL;

	int osc_color_number1 = 0;
	int osc_color_number2 = 0;
	int relax_color_number = 0;
	for (int i = 0; i < _dss.count(); i++)
	{
		DataSet *ds = _dss[i];
		QVector<double> *y_data = ds->getYData();
		QVector<double> *x_data = ds->getXData();
		QVector<uint8_t> *bad_map = ds->getBadData();	

		uint8_t comm_id = ds->getDataCode();
		switch (comm_id)
		{
		case DT_SGN_RELAX:
		case DT_SGN_RELAX2:
		case DT_SGN_RELAX3:	
		case DT_T1T2_NMR:
		case DT_DsT2_NMR:
		case DT_SOLID_ECHO:
			{
				relax_widget->addDataSet(ds->getDataName(), relax_widget->getRelaxDataPlot(), x_data, y_data, bad_map, false, relax_color_number++);
				PlottedDataSet *pds = relax_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				break;
			}		
		case DT_AVER_RELAX:
			{
				QwtCurveSettings *qwt_curve_settings = new QwtCurveSettings;
				qwt_curve_settings->title = tr("Moving averaging curve");
				QPen pen(Qt::black);
				pen.setWidthF(2);
				qwt_curve_settings->pen = pen;
				qwt_curve_settings->dataType = QwtCurveSettings::Lines;
				qwt_curve_settings->z = 100;
				relax_widget->addDataSet(ds->getDataName(), relax_widget->getRelaxDataPlot(), x_data, y_data, bad_map, qwt_curve_settings, false);
				PlottedDataSet *pds = relax_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_SGN_SE_ORG:
		case DT_SGN_FID_ORG:
			{				
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscDataPlot(), x_data, y_data, bad_map, false, osc_color_number1++);
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_NS_SE_ORG:
		case DT_NS_FID_ORG:
			{
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscDataPlot(), x_data, y_data, bad_map, false, osc_color_number1++);
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_SGN_SE:
			{
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscDataPlot(), x_data, y_data, bad_map, false, osc_color_number1++);
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_NS_SE:
			{
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscDataPlot(), x_data, y_data, bad_map, false, osc_color_number1++);
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_NS_QUAD_FID_RE:
		case DT_NS_QUAD_SE_RE:
		case DT_SGN_QUAD_FID_RE:
		case DT_SGN_QUAD_SE_RE:
			{																	
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscDataPlot(), x_data, y_data, bad_map, false, osc_color_number1++);		
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);		

				QVector<double> *_addy_data = new QVector<double>(x_data->size());
				for (int j = 0; j < y_data->size(); j++)
				{
					if (j < x_data->size())
					{
						*(_addy_data->data()+j) = y_data->at(j);
					}
				}
				addy_data = _addy_data;

				break;
			}			
		case DT_NS_QUAD_FID_IM:		
		case DT_NS_QUAD_SE_IM:
		case DT_SGN_QUAD_FID_IM:		
		case DT_SGN_QUAD_SE_IM:		
			{														
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscDataPlot(), x_data, y_data, bad_map, false, osc_color_number1++);		
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);	

				if (addy_data != NULL)
				{
					for (int j = 0; j < y_data->size(); j++)
					{
						if (j < addy_data->size())
						{
							double val = addy_data->at(j)*addy_data->at(j) + y_data->at(j) * y_data->at(j);
							*(addy_data->data()+j) = sqrt(val);
						}					
					}

					QwtCurveSettings *curve_settings = new QwtCurveSettings;
					QColor color = QColor(Qt::black);
					QBrush symbol_brush(color);
					symbol_brush.setStyle(Qt::NoBrush);
					curve_settings->symbol->setStyle(QwtSymbol::NoSymbol);
					curve_settings->symbol->setBrush(symbol_brush);
					curve_settings->symbol->setSize(QSize(8,8));
					QPen symbol_pen(Qt::black);
					symbol_pen.setWidth(1);
					curve_settings->symbol->setPen(symbol_pen);	
					curve_settings->pen.setColor(color);
					curve_settings->pen.setStyle(Qt::SolidLine);
					curve_settings->pen.setWidth(2);
					curve_settings->dataType = QwtCurveSettings::Lines;

					QString str_name = ds->getDataName().split("_im").first() + QString("#") + ds->getDataName().split("_im#").last();
					osc_widget->addDataSet(str_name, osc_widget->getOscDataPlot(), x_data, addy_data, bad_map, curve_settings, false);		
					PlottedDataSet *add_ds = osc_widget->getDataSetLast();
					
					uint8_t add_comm_id = 0;
					if (comm_id == DT_NS_QUAD_FID_IM) add_comm_id = DT_NS_QUAD_FID;
					else if (comm_id == DT_NS_QUAD_SE_IM) add_comm_id = DT_NS_QUAD_SE;
					else if (comm_id == DT_SGN_QUAD_FID_IM) add_comm_id = DT_SGN_QUAD_FID;
					else if (comm_id == DT_SGN_QUAD_SE_IM) add_comm_id = DT_SGN_QUAD_SE;
					add_ds->setTypeId(comm_id);
				}
				break;
			}	
		case DT_NS_FFT_FID_RE:
		case DT_NS_FFT_SE_RE:
		case DT_SGN_FFT_FID_RE:
		case DT_SGN_FFT_SE_RE:		
			{														
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscMathPlot(), x_data, y_data, bad_map, false, osc_color_number2++);		
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);	

				QVector<double> *_addy_data = new QVector<double>(x_data->size());
				for (int j = 0; j < y_data->size(); j++)
				{
					if (j < x_data->size())
					{
						*(_addy_data->data()+j) = y_data->at(j);
					}
				}
				addy_data = _addy_data;

				break;
			}		
		case DT_NS_FFT_FID_IM:
		case DT_NS_FFT_SE_IM:
		case DT_SGN_FFT_FID_IM:
		case DT_SGN_FFT_SE_IM:
			{
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscMathPlot(), x_data, y_data, bad_map, false, osc_color_number2++);		
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);	

				if (addy_data != NULL)
				{
					for (int j = 0; j < y_data->size(); j++)
					{
						if (j < addy_data->size())
						{
							double val = addy_data->at(j)*addy_data->at(j) + y_data->at(j) * y_data->at(j);
							*(addy_data->data()+j) = sqrt(val);
						}					
					}

					QwtCurveSettings *curve_settings = new QwtCurveSettings;
					QColor color = QColor(Qt::black);
					QBrush symbol_brush(color);
					symbol_brush.setStyle(Qt::NoBrush);
					curve_settings->symbol->setStyle(QwtSymbol::NoSymbol);
					curve_settings->symbol->setBrush(symbol_brush);
					curve_settings->symbol->setSize(QSize(8,8));
					QPen symbol_pen(Qt::black);
					symbol_pen.setWidth(1);
					curve_settings->symbol->setPen(symbol_pen);	
					curve_settings->pen.setColor(color);
					curve_settings->pen.setStyle(Qt::SolidLine);
					curve_settings->pen.setWidth(2);
					curve_settings->dataType = QwtCurveSettings::Lines;

					QString str_name = ds->getDataName().split("_im").first() + QString("#") + ds->getDataName().split("_im#").last();
					osc_widget->addDataSet(str_name, osc_widget->getOscMathPlot(), x_data, addy_data, bad_map, curve_settings, false);		
					PlottedDataSet *add_ds = osc_widget->getDataSetLast();
					
					uint8_t add_comm_id = 0;
					if (comm_id == DT_NS_FFT_FID_IM) add_comm_id = DT_NS_FFT_FID;
					else if (comm_id == DT_NS_FFT_SE_IM) add_comm_id = DT_NS_FFT_SE;
					else if (comm_id == DT_SGN_FFT_FID_IM) add_comm_id = DT_SGN_FFT_FID;
					else if (comm_id == DT_SGN_FFT_SE_IM) add_comm_id = DT_SGN_FFT_SE;
					add_ds->setTypeId(comm_id);
				}
				break;
			}
		case DT_SGN_FFT_FID_AM:						
		case DT_SGN_FFT_SE_AM:			
			{
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscMathPlot(), x_data, y_data, bad_map, false, osc_color_number2++);	
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);	

				break;
			}
		case DT_NS_FFT_FID_AM:
		case DT_NS_FFT_SE_AM:
			{														
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscMathPlot(), x_data, y_data, bad_map, false, osc_color_number2++);	
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);	

				break;
			}
		case DT_SGN_POWER_SE:
		case DT_SGN_POWER_FID:		
			{														
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscMathPlot(), x_data, y_data, bad_map, false, osc_color_number2++);	
				PlottedDataSet *pds = osc_widget->getDataSetLast();
				pds->setTypeId(comm_id);	

				break;
			}
		case DT_NS_POWER_SE:
		case DT_NS_POWER_FID:
			{					
				osc_widget->addDataSet(ds->getDataName(), osc_widget->getOscMathPlot(), x_data, y_data, bad_map, false, osc_color_number2++);	
				PlottedDataSet *ds = osc_widget->getDataSetLast();
				ds->setTypeId(comm_id);					
				break;
			}
		case DT_AFR1_RX:
		case DT_AFR2_RX:
		case DT_AFR3_RX:
			{
				rxtxControl->setDataSet(comm_id, ds->getDataName(), x_data, y_data, bad_map);
				PlottedDataSet *pds = rxtxControl->getDataSet(comm_id);
				pds->setTypeId(comm_id);
				break;
			}
		case DT_FREQ_TUNE:
			{
				freqAutoadjust->setDataSet(ds->getDataName(), x_data, y_data, bad_map);
				PlottedDataSet *pds = freqAutoadjust->getDataSet();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_RFP:
		case DT_RFP2:
			{
				rfpulseControl->setDataSet(ds->getDataName(), x_data, y_data, bad_map);
				PlottedDataSet *pds = rfpulseControl->getDataSet();
				pds->setTypeId(comm_id);
				break;
			}
		case DT_DU_T:	
		case DT_TU_T:
		case DT_PU_T:
			{
				monitoring_widget->addData(comm_id, x_data, y_data);
				break;
			}
		case DT_DIEL_ADJUST:
			{
				sdsp_widget->addData(ds->getXData(), ds->getYData());
				break;
			}
		default: break;
		}		
	}	
}

void MainWindow::plotLoggingData(DataSets &_dss)
{
	logging_widget->addDataSets(_dss);
}


bool MainWindow::doQualityControl(QVector<double> *vec)
{
	const int win_len = 6;
	double S = 0;
	double S2 = 0;
	double S1 = 0;
	bool data_ok = true;
	for (int i = 0; i < vec->size(); i++)
	{
		S1 += vec->data()[i];
		S2 += vec->data()[i]*vec->data()[i];
		if (i >= win_len)
		{
			S1 -= vec->data()[i-win_len];
			S2 -= vec->data()[i-win_len]*vec->data()[i-win_len];
			S = S2/win_len - S1*S1/win_len/win_len;
			if (S == 0)
			{
				data_ok = false;
			}
		}
	}

	return data_ok;
}

void MainWindow::doSmoothing(QVector<double> *vec)
{
	if (vec->size() < 6) return;

	switch (processing_relax.smoothing_number)
	{
	case 1: vec->first() = (vec->at(1) + vec->at(2))/2; break;
	case 2: vec->first() = (8*vec->at(2) + 2*vec->at(3) - 4*vec->at(4))/6; break;
	case 3: vec->first() = (2*vec->at(2) + vec->at(3) - vec->at(5))/2; break;	
	}

	double alpha = processing_relax.alpha;
	for (int i = 1; i < vec->size(); i++)
	{
		*(vec->data() + i) = alpha*vec->at(i) + (1-alpha)*vec->at(i-1);
	}
}

void MainWindow::doExtrapolation(QVector<double> *vec)
{
	if (vec->size() < 6) return;

	switch (processing_relax.extrapolation_number)
	{	
	case 1:
		{			
			int I = processing_relax.I;
			int K = processing_relax.K;
			if (vec->size() < I || vec->size() < K) return; 
			
			double A = 0;
			double B = 0;
			double C = 0;
			double D = 0;
			double E = 0;
			for (int m = I; m < I + K; m++)
			{
				A += vec->at(m)*vec->at(m)*m*m;
				B += vec->at(m)*vec->at(m)*m;
				C += vec->at(m)*vec->at(m);
				if (vec->at(m) != 0) D += vec->at(m)*vec->at(m)*m*log(fabs(vec->at(m)));
				if (vec->at(m) != 0) E += vec->at(m)*vec->at(m)*log(fabs(vec->at(m)));
			}

			double a = 0;
			double b = 0;
			double zn = A*C - B*B;
			if (zn != 0)
			{
				a =-(E*B - D*C)/zn;
				if (a > 0) a= -a;
				b = (E*A - D*B)/zn;
			}

			for (int i = 0; i < I; i++)
			{
				*(vec->data() + i) = exp(a*i+b);
			}

			break;
		}
	default: break;
	}
}

void MainWindow::doZeroAdjustment(QVector<double> *vec)
{
	if (vec->size() >= 20) 
	{
		int full_size = vec->size();
		int start = 0.90*full_size;
		
		double mean = 0;
		for (int i = start; i < full_size; i++)
		{
			mean += vec->at(i);
		}
		mean /= (full_size-start);

		for (int i = 0; i < full_size; i++)
		{
			*(vec->data() + i) = vec->at(i) - mean;
		}
	}	
}

void MainWindow::calcT2Spectra(DataSets &forT2_spec_dss)
{
	if (forT2_spec_dss.isEmpty()) return;
	if (T2processing_pool.isBusy())
	{
		if (T2processing_pool.getT2Processors().isEmpty())
		{
			T2processing_pool.setBusy(false);
		}
		else 
		{
			bool all_finished = true;
			for (int i = 0; i < T2processing_pool.getT2Processors().count(); i++)
			{
				T2Processor *T2_proc = T2processing_pool.getT2Processors().at(i);
				if (!T2_proc->isCalcFinished()) all_finished = false;
			}
			if (all_finished) T2processing_pool.clearAll(); 
			else return;			
		}
	}
		
	for (int i = 0; i < forT2_spec_dss.count(); i++)
	{
		DataSet *ds = forT2_spec_dss.at(i);
		int num_data = ds->getDataName().split("#").last().toInt();
		int msg_uid = ds->getUId();
		uint8_t ds_comm_id = ds->getDataCode();
		int group_index = ds->getGroupIndex();
		QPair<bool,double> dpt = ds->getDepth();
		int experiment_id = ds->getExpId();
		
		int spec_size = processing_relax.T2_points;	
		processing_relax.regulParam = spec_size;
		
		double T2min = processing_relax.T2_from;
		double T2max = processing_relax.T2_to;
		double dT2 = (log(T2max)-log(T2min))/(spec_size-1);		
		
		QVector<double> *T2 = new QVector<double>(spec_size);			// Времена Т2, по которым строится спектр
		QVector<double> *out_y_vec = new QVector<double>(spec_size);
		QVector<uint8_t> *out_bad_vec = new QVector<uint8_t>(spec_size);
		
		for (int j = 0; j < spec_size; j++)
		{		
			T2->data()[j] = T2min * exp(j*dT2);							// Новый вариант		
			out_bad_vec->data()[j] = DATA_OK;
		}
		
		QString _ds_name_base = "nmr_relax_spectrum1#%1";				
		uint8_t out_comm_id = DT_SGN_SPEC1;
		if (ds->getDataCode() == DT_SGN_RELAX2) 
		{
			out_comm_id = DT_SGN_SPEC2;
			_ds_name_base = "nmr_relax_spectrum2#%1";	
		}
		else if (ds_comm_id == DT_SGN_RELAX3) 
		{
			out_comm_id = DT_SGN_SPEC3;
			_ds_name_base = "nmr_relax_spectrum3#%1";
		}
		//QString _ds_name_base = "nmr_relax_spectrum%1#%2";		
		//uint8_t out_comm_id = DT_SGN_SPEC1; 
		int channel_data_id = ds->getChannelId();
		DataSet *out_ds = new DataSet(_ds_name_base.arg(channel_data_id).arg(++num_data), msg_uid, out_comm_id, T2, out_y_vec, out_bad_vec);
		out_ds->setInitialDataSize(spec_size);	
		out_ds->setGroupIndex(0);
		out_ds->setChannelId(channel_data_id);
		out_ds->setDepth(dpt);
		out_ds->setExpId(experiment_id);
		
		T2Processor *t2_proc = new T2Processor(&processing_relax, ds, out_ds, i);		
		T2processing_pool.addT2Processor(t2_proc);
		//qDebug() << QString("T2processing_pool: %1 objects").arg(T2processing_pool.getT2Processors().count());
	}

	T2processing_pool.startT2Processing();
}

void MainWindow::plotSpectralData()
{
	DataSets spec_dss = T2processing_pool.getResultedDataSets();
	if (spec_dss.count() == 0) return;	
	
	dataset_storage->append(spec_dss);

	if (!nmrtool_state) return;		

	//relax_widget->refreshDataSets();
		
	DataSets logging_dss;
	int spec_color_number = 0;
	for (int i = 0; i < spec_dss.count(); i++)
	{
		DataSet *ds = spec_dss[i];
		QVector<double> *y_data = ds->getYData();
		QVector<double> *x_data = ds->getXData();
		QVector<uint8_t> *bad_map = ds->getBadData();	
		int channel_id = ds->getChannelId();
		ToolChannel *tool_channel = getToolChannel(channel_id);
		if (tool_channel == NULL) continue;

		uint8_t comm_id = ds->getDataCode();
		switch (comm_id)
		{
		case DT_SGN_SPEC1:
		case DT_SGN_SPEC2:
		case DT_SGN_SPEC3:
			{
				/*if (processing_relax.porosity_on)
				{
					for (int j = 0; j < y_data->size(); j++)
					{
						*(y_data->data()+j) *= tool_channel->normalize_coef1 * tool_channel->normalize_coef2;
					}
				}*/

				relax_widget->addDataSet(ds->getDataName(), relax_widget->getRelaxMathPlot(), x_data, y_data, bad_map, false, spec_color_number++);
				PlottedDataSet *pds = relax_widget->getDataSetLast();
				pds->setTypeId(comm_id);
				pds->setBasedDataSet(ds);
				logging_dss.append(ds);
				break;
			}		
		default: break;
		}		
	}	
	plotLoggingData(logging_dss);
	
	T2processing_pool.clearAll();
}


void MainWindow::viewWizard(bool flag)
{
	QAction *a = (QAction*)sender();
	if (a == ui->a_CommunicationLogMonitor)
	{
		dock_msgLog->setVisible(flag);
	}
	else if (a == ui->a_LoggingToolConsole)
	{
		dock_msgConnect->setVisible(flag);
	}
	else if (a == ui->a_SequenceWizard)
	{
		dock_sequenceProc->setVisible(flag);
	}
	else if (a == ui->a_MainToolBar)
	{
		ui->toolBarMain->setVisible(flag);
	}
	else if (a == ui->a_LoggingToolStatus)
	{
		dock_nmrtoolStatus->setVisible(flag);
	}
	else if (a == ui->a_DepthMonitor)
	{
		//dock_depthMonitor->setVisible(flag);
		dock_depthTemplate->setVisible(flag);
		
	}
	else if (a == ui->a_RxTxControl)
	{
		dock_RxTxControl->setVisible(flag);
	}
}

void MainWindow::saveAllSettings()
{
	// Processing settings
	app_settings->setValue("ProcessingSettings/QualityControl", processing_relax.is_quality_controlled);
	app_settings->setValue("ProcessingSettings/T2_from", processing_relax.T2_from);
	app_settings->setValue("ProcessingSettings/T2_to", processing_relax.T2_to);
	app_settings->setValue("ProcessingSettings/T2_min", processing_relax.T2_min);
	app_settings->setValue("ProcessingSettings/T2_max", processing_relax.T2_max);
	app_settings->setValue("ProcessingSettings/T2_cutoff", processing_relax.T2_cutoff);
	app_settings->setValue("ProcessingSettings/T2_cutoff_clay", processing_relax.T2_cutoff_clay);
	app_settings->setValue("ProcessingSettings/T2_points", processing_relax.T2_points);
	app_settings->setValue("ProcessingSettings/Iterations", processing_relax.iters);
	app_settings->setValue("ProcessingSettings/MCBWColor", QColorToQVar(processing_relax.MCBWcolor));
	app_settings->setValue("ProcessingSettings/MBVIColor", QColorToQVar(processing_relax.MBVIcolor));
	app_settings->setValue("ProcessingSettings/MFFIColor", QColorToQVar(processing_relax.MFFIcolor));
	
	// SDSP settings
	sdsp_widget->saveSettings();

	// Depth meter, step motor COM port settings
	//saveCOMSettings(COM_Port_depth, "DepthMeter");
	//saveCOMSettings(COM_Port_stepmotor, "StepMotor");
	saveSocketSettings(&dmeter_tcp_settings, "DepthMeter");
	saveSocketSettings(&stmotor_tcp_settings, "StepMotor");

}

void MainWindow::showPowerStatus(unsigned char pow_status)
{
	nmrtoolStatus->showPowerStatus(pow_status);
}

void MainWindow::showTemperatureStatus(unsigned char temp_status)
{
	nmrtoolStatus->showTemperatureStatus(temp_status);
}

bool MainWindow::dataIsExportingToOil(uint8_t data_code)
{	
	for (int i = 0; i < data_type_list_Oil.count(); i++)
	{
		DataType dt = data_type_list_Oil[i];
		if (dt.type == data_code && dt.flag) return true;
	}

	return false;
}

bool MainWindow::getExportingStateToOil(DataType &dt)
{
	QString s_path = QString("ExportDataToOil/%1").arg(dt.mnemonics);
	if (app_settings->contains(s_path))
	{
		bool state = app_settings->value(s_path).toBool();
		return state;
	}
	else return false;
}

bool MainWindow::dataIsExportingToFile(uint8_t data_code)
{	
	for (int i = 0; i < data_type_list_File.count(); i++)
	{
		DataType dt = data_type_list_File[i];
		if (dt.type == data_code && dt.flag) return true;
	}

	return false;
}

bool MainWindow::getExportingStateToFile(DataType &dt)
{
	QString s_path = QString("ExportDataToFile/%1").arg(dt.mnemonics);
	if (app_settings->contains(s_path))
	{
		bool state = app_settings->value(s_path).toBool();
		return state;
	}
	else return false;
}

void MainWindow::tabIsActivated(int index)
{		
	/*QWidget *widget = ui->tabWidget->widget(index);
	if (widget->objectName() == "tabSDSP") sdsptab_is_active = true;
	else sdsptab_is_active = false;
	
	bool isSDSP = false;
	for (int i = 0; i < ui->tabWidget->count(); i++)
	{
		QWidget *widget = ui->tabWidget->widget(i);
		if (widget->objectName() == "tabSDSP") isSDSP = true;
	}
	if (!isSDSP) ui->tabWidget->addTab(ui->tabSDSP, tr("SDSP"));
	
	
	a_start->setVisible(!sdsptab_is_active);
	a_stop->setVisible(!sdsptab_is_active);
	a_start_sdsp->setVisible(sdsptab_is_active);
	a_stop_sdsp->setVisible(sdsptab_is_active);

	dock_sequenceProc->setVisible(!sdsptab_is_active);	
	dock_sdspProc->setVisible(sdsptab_is_active);
	*/
}

void MainWindow::sdspIsActivated(bool flag)
{
	a_start->setVisible(!flag);
	a_stop->setVisible(!flag);
	a_start_sdsp->setVisible(flag);
	a_stop_sdsp->setVisible(flag);

	int sdsp_index = -1;
	for (int i = 0; i < ui->tabWidget->count(); i++)
	{
		QWidget *widget = ui->tabWidget->widget(i);
		QString tab_name = widget->objectName();
		if (tab_name == "SDSPWidget") sdsp_index = i;
	}
	if (flag && sdsp_index >= 0) ui->tabWidget->setCurrentIndex(sdsp_index);	
}

void MainWindow::saveNewCalibrCoefficientToCfg(double val, ToolChannel *channel)
{
	for (int i = 0; i < tools_settings.count(); i++)
	{
		QSettings *settings = tools_settings[i];
		if (current_tool.file_name == settings->fileName())
		{
			for (int j = 0; j < tool_channels.count(); j++)
			{
				ToolChannel *_channel = tool_channels[j];
				//if (channel->data_type == "NMR_CHANNEL")
				if (_channel == channel)
				{
					QString channel_rec = QString("channel#%1/normalize_coef1").arg(channel->channel_id);
					settings->setValue(channel_rec, val);
					return;
				}
			}			
		}
	}	
}

void MainWindow::saveNewCalibrCoefficient(double val, ToolChannel *channel)
{
	/*QString file_name = expScheduler->getDataFile();
	QSettings data_file(file_name, QSettings::IniFormat, this);

	for (int j = 0; j < tool_channels.count(); j++)
	{
		ToolChannel *_channel = tool_channels[j];		
		if (_channel->channel_id == channel->channel_id)
		{
			QString channel_rec = QString("channel#%1/normalize_coef1").arg(channel->channel_id);
			data_file.setValue(channel_rec, val);
			return;
		}
	}*/

	autocalibration_state = false;
}


ToolChannel* MainWindow::getToolChannel(int _channel_id)
{
	for (int i = 0; i < tool_channels.count(); i++)
	{
		ToolChannel *channel = tool_channels[i];
		int id = channel->channel_id;
		if (id == _channel_id) return channel;
	}

	return NULL;
}