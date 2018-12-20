#ifndef NMRTOOL_CONNECT_WIZARD_H
#define NMRTOOL_CONNECT_WIZARD_H

#include <QTextEdit>
#include <QVariantList>
#include <QElapsedTimer>
#include <QTimer>
#include <QSettings>
#include <QLabel>
#include <QTcpSocket>

#include "ui_nmrtool_connect_wizard.h"

#include "../io_general.h"
#include "../tools_general.h"
#include "../Communication/message_class.h"
#include "../Common/statusbar_widgets.h"


/*class TrafficWidget : public QFrame
{
	Q_OBJECT

public:
	explicit TrafficWidget(QWidget *parent = 0);
	~TrafficWidget() { };

	//enum MsgState { MsgOK, MsgPoor, MsgBad, MsgOff };	// MsgOK - ��� ������ � �������� ���������
	//													// MsgPoor - ������� ������ � ���� ������
	//													// MsgBad - ��������� �� ������� ������������
	//													// MsgOff - ��� ���������
	void setTitle(QString str);
	void setReportText(QString str);
	void setReport(double value, MsgState state);		// value - ������� ������ (������� ������� ������)

private:
	QLabel *lblLed;
	QLabel *lblTitle;
	QLabel *lblReport;	

private slots:
	void resetTrafficState();


signals:
	void clicked();
};


// ������ ��� StatusBar � ��������� ����� �� ������
class ConnectionWidget : public QFrame
{
	Q_OBJECT

public:
	explicit ConnectionWidget(QWidget *parent = 0);
	~ConnectionWidget() { };

	void setTitle(QString str);	
	void setReport(ConnectionState state);	
	void startBlinking() { blink_timer.start(); }
	void stopBlinking() { blink_timer.stop(); }

private:
	QLabel *lblLed;
	QLabel *lblTitle;	

	QTimer blink_timer;

private slots:
		void resetConnectionState();
		void blink();

signals:
	void clicked();
	//void connected(ConnectionState state);
};*/


// ������ ��� StatusBar � ��������� �������� ������ �� ������
class NMRToolLinker : public QWidget, public Ui::NMRToolLinker
{
	Q_OBJECT

public:
	explicit NMRToolLinker(TCP_Settings *nmrtool_socket, QSettings *settings, QWidget *parent = 0);
	~NMRToolLinker();

	TrafficWidget *getTrafficWidget() const { return trafficWidget; }
	ConnectionWidget *getConnectionWidget() const { return connWidget; }
	QTextEdit *getReportWidget() const { return ui->tedReport; }
	
	//QString getCOMPortName() { return com_port->COM_port->portName(); }	
	//PortSettings getPortSettings() { return com_port->COM_Settings; }
	ConnectionState getConnectionState() { return connection_state; }	

	QList<DeviceData*> *getMsgContainer() { return &msg_container; }

	Ui::NMRToolLinker *getUI() { return ui; }

		
public slots:	
	void showCmdResult(uint32_t _uid, QString obj_name, QVariantList data = QVariantList());
	void addText(QString text);
	//void refreshCOMPortSettings();	
	//bool openCOMPort();
	bool connectNMRToolSocket();
	void stopConnection();
	void searchForNMRTool();
	void startConnection(bool flag);
	void findNMRTool();
	void showMsgTrafficReport(int num, int total);	// ������� ���������� num ������� ������ �� total 
	void startNMRTool();
	void stopNMRTool();
	//void redrawWidget(Qt::DockWidgetArea location);
	void applyProcPrg(QVector<uint8_t> &proc_prg, QVector<uint8_t> &proc_instr);	// ��������� � ���������� ������ ����� ��������� ��������� ������ ���������� �����������
	void applyWinFuncParams(QVector<int> params);									// ��������� ����� ��������� ������� �������
	void applyMsgCommParams(QVector<uint8_t> params);								// ��������� ����� ��������� ��� �������� ������: ����� ������ ����-��������, ���-�� ������������ ������ � �.�.
	void sendDataToSDSP(QByteArray& arr);
	void sendDataToSDSP(QVector<int> &params);
	void sendToolSettings(QVector<int> params);
	void setDefaultCommSettingsState(bool state) { default_comm_settings_on = state; }

private:
	void setConnections();
	QString getCurrentTime();
	//void saveCOMPortSettings();

	Ui::NMRToolLinker *ui;
	TrafficWidget *trafficWidget;
	ConnectionWidget *connWidget;
	bool log_viz_state;
	bool default_comm_settings_on;
	
	QSettings *app_settings;

	//COM_PORT *com_port;
	//QStringList port_names;
	TCP_Settings *nmrtool_socket;
	ConnectionState connection_state;

	QList<DeviceData*> msg_container;	
	
private slots:	
	//void setCOMPort(QString str);
	//void setBaudrate(QString str);
	//void setDataBits(QString str);
	//void setParity(QString str);
	//void setStopBits(QString str);	
	void setLogVizState(bool flag);
	
signals:
	void send_msg(DeviceData*, const QString&);				// �������� ������� � message_processor (� �������� ����� - � ���������� ������)
	void place_to_statusbar(QString&);						// ���������� ������ �������� �� ������ ���������
	void start_experiment(bool);							// �����/���� �����������
	void start_sdsp_adjustment(bool);						// �����/���� ��������� ��������������� �������
	void cmd_resulted(uint8_t cmd, ConnectionState state);	// �������������� ���������� � ��������� ���������� ������� cmd
	//void cmd_resulted(uint8_t, int);						// �������������� ���������� � ��������� ���������� ������� cmd
	//void cmd_resulted(bool, uint32_t);						// �������������� ������������ ������������ SchedulerWizard � ���������� ���������� ������� DeviceData c ������ uid
	void control_nmrtool(bool);								// ��������� �����/��������� ��������� ��������� �� ���� ����������� ������� 
	void send_data_toSDSP(QByteArray&);						// ������ ��� ������ ������� � ��������������� ������
	void tool_settings_applied(bool);						// ������� �������� �������� �������� ����������� �������	
	void fpga_seq_started(bool);							// ������������� ������������ ������������ �� ������/�������� ������ ������������������ �� ��������� ������������
	void default_comm_settings(bool);						// 
};

#endif // NMRTOOL_CONNECT_WIZARD