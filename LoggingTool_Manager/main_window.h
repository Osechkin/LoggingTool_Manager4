#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets/QMainWindow>

#include <QMainWindow>
#include <QDockWidget>
#include <QMutex>
#include <QSplitter>

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"

#include "tools_general.h"
#include "Wizards/sequence_wizard.h"
//#include "Wizards/experiment_manager.h"
#include "Wizards/scheduler_wizard.h"
#include "Wizards/message_log_monitor.h"
#include "Wizards/nmrtool_connect_wizard.h"
#include "Wizards/depth_monitoring_wizard.h"
#include "Wizards/depth_template_wizard.h"
#include "Wizards/nmrtool_status_wizard.h"
#include "Wizards/rxtx_control_wizard.h"
#include "Wizards/rfpulse_control_wizard.h"
#include "Wizards/monitoring_wizard.h"
#include "Wizards/sdsp_wizard.h"
#include "Wizards/logging_widget.h"
#include "Wizards/freq_autoadjust_wizard.h"
#include "Communication/threads.h"
#include "Communication/message_processor.h"
#include "Communication/tcp_data_manager.h"
#include "Communication/cdiag_data_manager.h"
#include "Common/app_settings.h"
#include "Common/experiment_settings.h"
#include "Common/data_containers.h"
#include "Common/data_processing.h"
#include "GraphClasses/graph_classes.h"

#include "ui_main_window.h"


typedef QList<MsgInfo*> MsgInfoContainer;


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	GF_Data *getGFDataObj() { return gf_data; }	
	Communication_Settings *getCommSettings() { return comm_settings; }
	Clocker *getClocker() { return clocker; }
	QextSerialPort *getCOMPort() { return COM_Port->COM_port; }
	QMutex *getCOMMsgMutex() { return com_msg_mutex; }	
	QMutex *getDataSetMutex() { return dataset_mutex; }
	QSettings *getAppSettings() { return app_settings; }
	ProcessingRelax &getProcessingRelax() { return processing_relax; }
	QVector<ToolChannel*> &getCurrentToolChannels() { return tool_channels; }
	ToolChannel *getToolChannel(int _channel_id);

	bool isDefaultCommSettings() { return default_comm_settings_on; }
		
private:
	void setActions();
	void setConnections();
	void initCOMSettings(COM_PORT *com_port);
	void initCOMSettings(COM_PORT *com_port, QString objName);
	void saveCOMSettings(COM_PORT *com_port, QString objName);
	void initSaveDataAttrs();
	void initDataTypes();
	bool setupCOMPort(COM_PORT *com_port, QString _port_name);
	bool findAvailableCOMPort(COM_PORT *com_port);    
	void createTabs();
	void initExperimentBar();
	void initSettingsBar();
	void clearStartExperimentData();
	void initStatusBar();	// ���������������� StatusBar
	//void generateXData(const uint8_t cmd_id, QVector<double> *x_data);
	
	void setSequenceStatus(SeqStatus status);

	void addToDataSets(DataSet *ds) { dataset_storage->push_back(ds); }
	void plotData(DataSets &_dss);	
	void plotLoggingData(DataSets &_dss);
	void exportData(DataSets &dss, QList<QVector<uint8_t> > &gap, QList<QVector<double> > &full_xdata);		
	void prepareDataForNet(DataSets dss, QString &out_str);

	void lockDataSet() { dataset_mutex->lock(); }
	void unlockDataSet() { dataset_mutex->unlock(); }

	bool doQualityControl(QVector<double> *vec);

	void doSmoothing(QVector<double> *vec);
	void doExtrapolation(QVector<double> *vec);
	void doZeroAdjustment(QVector<double> *vec);

	void calcT2Spectra(DataSets &forT2_spec_dss);			// ��������� � ��������� � ������� LoggingWidget ("�������") ������� ������ ���������� �2 

	bool dataIsExportingToOil(uint8_t data_code);
	bool getExportingStateToOil(DataType &dt);
	bool dataIsExportingToFile(uint8_t data_code);
	bool getExportingStateToFile(DataType &dt);
	
	//void applyFPGAandDSPPrg();

	void initExperimentalInfo();
	void loadProcessingSettings();
	void loadCommSettings();

	void applyToolSettings();	
	
private:
	Ui::MainWindow *ui;
	QSettings *app_settings;	

	COM_PORT *COM_Port;
	Communication_Settings *comm_settings;
	COM_PORT *COM_Port_depth;				// COM-���� ��� �����������
	COM_PORT *COM_Port_stepmotor;			// COM-���� ��� ����� ���������� ������� ����������
	GF_Data *gf_data;	

	unsigned char tool_id;					// ����������������� ����� ����������� �������
	ToolInfo current_tool;
	bool current_tool_was_applied;			// ��������� ����, ��� ��������� ����������� ������� ���� ���������� (������ �� ����� ���� kmrk.cfg � �.�.)
	QVector<ToolChannel*> tool_channels;	// ������ � ����������� ������� ����������� �������
	QList<QSettings*> tools_settings;		// ����� �������� ��������� ���������� ��������
	QSettings *current_tool_settings;		// ���� �������� �������� ����������� �������
	
	QMutex *com_msg_mutex;	// ������� ��� ������������� ������� ������� � ������ ��������� COM_Message, ���������� �� ����������� ������ 
	QMutex *dataset_mutex;	// ������� ��� ������������� ������� ������� � ������ ���� DataSet

	Clocker *clocker;
	COMCommander *com_commander;
	
	MsgProcessor *msg_processor;
	TcpDataManager *tcp_data_manager;
	CDiagDataManager *cdiag_data_manager;
	QThread *thread_msg_processor; 

	MsgInfoContainer *msg_data_container;	
	DataSets *dataset_storage;

	//MonitoringPlot *tempPlot;
	RelaxationWidget *relax_widget;
	OscilloscopeWidget *osc_widget;
	MonitoringWidget *monitoring_widget;	
	SDSPWidget *sdsp_widget;
	LoggingWidget *logging_widget;
	
	// Main Tool Bar
	QAction *a_connect;
	QAction *a_disconnect;
	QAction *a_start;
	QAction *a_run;
	QAction *a_apply_prg;
	QAction *a_stop;
	QAction *a_break;
	QAction *a_start_sdsp;
	QAction *a_stop_sdsp;

	// Experiment Tool Bar
	QTimer experiment_timer;
	QTime starting_time;
	int obtained_data_counter;
	QLabel *lblStartTime;
	QLabel *lblDataCounter;
	QLabel *lblTool;
	
	// Status Bar
	QLabel *lblStatusInfo;

	// Output Objects
	MessageLogMonitor *msgLog;
	NMRToolLinker *nmrtoolLinker;
	//SignalProcessingWizard *signalProc;
	SequenceWizard *sequenceProc;
	SDSPWizard *sdspProc;
	SchedulerWizard *expScheduler;
	//ExperimentManager *expManager;
	//DepthMonitoringWizard *depthMonitor;

	DepthTemplateWizard *depthTemplate;
	NMRToolStatusWizard *nmrtoolStatus;
	RxTxControlWizard *rxtxControl;
	RFPulseControlWizard *rfpulseControl;
	FreqAutoadjustWizard *freqAutoadjust;

	QDockWidget *dock_msgConnect;
	QDockWidget *dock_msgLog;
	QDockWidget *dock_sequenceProc;
	QDockWidget *dock_sdspProc;
	QDockWidget *dock_expScheduler;
	QDockWidget *dock_depthMonitor;
	QDockWidget *dock_depthTemplate;
	QDockWidget *dock_nmrtoolStatus;
	QDockWidget *dock_RxTxControl;
	QDockWidget *dock_RFPulseControl;
	QDockWidget *dock_FreqAutoadjust;
	
	bool nmrtool_state;		// true = NMR Tool is started; false = NMR Tool is stopped.
	bool sdsptool_state;	// true = SDSP Tool is started; false = SDSP Tool is stopped.
	bool sdsptab_is_active;	// true = SDSP Tab Widget is activated; false = SDSP Tab Widget is not activated
	SeqStatus sequence_status;
	SeqStatus sdsp_sequence_status;

	DataSave save_data_attrs;
	//QFile *save_data_file;
	//bool start_data_export;				// ��������� ������������� ������� ����� ���� ��� �������� ���������� ������
	int experiment_id;					// ����� ������������. ������������� �� 1 ��� ������ ������� ��������� �� ������ "Start Sequence"
	
	ProcessingRelax processing_relax;
	ProcessingPool T2processing_pool;	// ���������� ������, �������������� ������� ������ �2

	DataSetWindows data_set_windows;
	DataTypeList data_type_list_Oil;	// ������ ������ ��� �������� � ���������-������� (Oil)
	DataTypeList data_type_list_File;	// ������ ������ ��� �������� �� ������� ����
		
	bool default_comm_settings_on;
	bool autocalibration_state;			// ��������� �������������� (��� NMR_Kern): �����������/�� ����������� (��� �� ���������� �����)
	
private slots:    
	void connectToNMRTool(bool flag);   
	void disconnectFromNMRTool();
	void setCOMPortSettings();  	
	void setCommunicationSettings();
	void setTCPConnectionSettings();	
	void setDepthMeterSettings();
	void setStepMotorCOMSettings();
	void setProcessingSettings();
	void setSequenceChanged();
	void showAboutDialog();
	void setExportToOilData();
	void setExportToFileData();
	void setSaveDataAttrs(DataSave &attrs);
	void stopNMRTool(bool flag);
	void startNMRTool(bool flag);
	void startSDSPTool(bool flag);
	void stopSDSPTool(bool flag);
	void applyDSPProcPrg(bool flag);						// ��������� � ���������� ������ ����� ��������� ��������� ������
	void applyWinFuncParams(QVector<int>& params);			// ��������� � ���������� ������ ��������� ������� �������
	void setMovingAveragingSettings(bool is_on, int count);
	void applySDSPParams(QVector<int>& params);				// ��������� � ���������� ������ ��������� ���������������� �������
	void breakAllActions();
		
	void startExperiment(bool flag);
	void setExperimentalInfo();
	void setDataFileSettings();
	void setDefaultCommSettings(bool state); // { default_comm_settings_on = state; }
	void resetCommSettings();

	void storeMsgData(MsgInfo* msg_info);
	//void plotData(DeviceData *device_data);		
	void sendDataToNetClients(const QString &sock_id, int index);
	void sendToSDSP(QByteArray& arr);	
	void depthDepthMeterConnected(bool flag);
	void placeInfoToStatusBar(QString& str);				// ���������� ���� �� StatusBar (lblStatusInfo)
	void placeInfoToExpToolBar(QString& str_count, QString &str_time, QString& tool_info); // ���������� ���� �� ������ ������������ ExpToolBar (������� ������, ����� �� ������ ���������, �������� �������)
	void placeInfoToExpToolBar(QString& str_count, QString &str_time);	// ���������� ���� �� ������ ������������ ExpToolBar (������� ������ � ����� �� ������ ���������)
	void placeInfoToExpToolBar(QString& tool_info);						// ���������� ���� �� ������ ������������ ExpToolBar (�������� �������)

	void treatNewData(DeviceData *device_data);				// ���������� ��������� � ����������� ������� ������ (����������, �������� � ���� � �.�.)	
	void setCmdResult(uint8_t cmd, ConnectionState state);	// ���������� ��������� ���������� ��������, ������������ � ui->toolBar � ���������� ������
	void showPowerStatus(unsigned char pow_status);
	void showTemperatureStatus(unsigned char temp_status);
	void tabIsActivated(int index);	
	void sdspIsActivated(bool flag);
	void viewWizard(bool flag);
	void saveAllSettings();
	void clearStatusBar();
	void plotSpectralData();
	void setToolId(unsigned char id);
	void changeLoggingTool();
	void setToolSettingsApplied(bool flag);

	void setExpSchedulerFinished();
	void setExpSchedulerStarted();

	void saveNewCalibrCoefficientToCfg(double val, ToolChannel *channel);	// ��������� ����� ������������� ����������� � ���� tool.cfg
	void saveNewCalibrCoefficient(double val, ToolChannel *channel);		// ��������� ����� ������������� ����������� � ���� ������

private:
	void loadToolsSettings();
	void loadAppSettings();
	void setToolChannels(QSettings *settings);
	double getDepthDisplacement(uint8_t _channel_id, QVector<ToolChannel*> &_tool_channels);
	void sendToolSettings();
	//int getDataTypeId(QString str, bool *res = 0); 

signals:
	void stop_clocker();
	void stop_com_commander();
	void control_sdsptool(bool);							// ��������� ����� SDSP ������� �� ������� ���������� ������ 
	void sdsp_is_enabled(bool);								// ������������� ����� ��������������� ������ �������� ��� ��������� (����� ���������� ������ ���������, �� �� ��������� ��������� ���������)
	
};

#endif // MAIN_WINDOW_H

