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

	//enum MsgState { MsgOK, MsgPoor, MsgBad, MsgOff };	// MsgOK - нет ошибок в принятом сообщении
	//													// MsgPoor - имеются ошибки в ряде блоков
	//													// MsgBad - сообщение не удалось восстановить
	//													// MsgOff - нет сообщений
	void setTitle(QString str);
	void setReportText(QString str);
	void setReport(double value, MsgState state);		// value - процент ошибок (процент сбойных блоков)

private:
	QLabel *lblLed;
	QLabel *lblTitle;
	QLabel *lblReport;	

private slots:
	void resetTrafficState();


signals:
	void clicked();
};


// виджет для StatusBar о состоянии связи по кабелю
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


// виджет для StatusBar о состоянии передачи данных по кабелю
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
	void showMsgTrafficReport(int num, int total);	// принято количество num сбойных блоков из total 
	void startNMRTool();
	void stopNMRTool();
	//void redrawWidget(Qt::DockWidgetArea location);
	void applyProcPrg(QVector<uint8_t> &proc_prg, QVector<uint8_t> &proc_instr);	// отправить в каротажный прибор новую программу обработки данных сигнальным процессором
	void applyWinFuncParams(QVector<int> params);									// отправить новые параметры оконных функций
	void applyMsgCommParams(QVector<uint8_t> params);								// отправить новые параметры для передачи данных: блина блоков Рида-Соломона, кол-во исправляемых ошибок и т.д.
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
	void send_msg(DeviceData*, const QString&);				// отправка запроса в message_processor (в конечном итоге - в каротажный прибор)
	void place_to_statusbar(QString&);						// размещение строки символов на строке состояния
	void start_experiment(bool);							// старт/стоп эксперимент
	void start_sdsp_adjustment(bool);						// старт/стоп настройки диэлектического прибора
	void cmd_resulted(uint8_t cmd, ConnectionState state);	// информирование приложения о состоянии выполнения команды cmd
	//void cmd_resulted(uint8_t, int);						// информирование приложения о состоянии выполнения команды cmd
	//void cmd_resulted(bool, uint32_t);						// информирование планировщика эксперимента SchedulerWizard о результате выполнения команды DeviceData c данным uid
	void control_nmrtool(bool);								// выставить старт/окончание программы измерений на ПЛИС каротажного прибора 
	void send_data_toSDSP(QByteArray&);						// данные для прямой оправки в диэлектрический прибор
	void tool_settings_applied(bool);						// признак успешной отправки настроек каротажного прибора	
	void fpga_seq_started(bool);							// информиование планировшика эксперимента об успехе/неуспехе старта последовательности из программы эксперимента
	void default_comm_settings(bool);						// 
};

#endif // NMRTOOL_CONNECT_WIZARD