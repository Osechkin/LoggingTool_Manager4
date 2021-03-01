#ifndef THREADS_H
#define THREADS_H

#include <QThread>
#include <QMutex>
#include <QVector>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>

#include "message_class.h"
#include "qextserialport.h"
#include "../io_containers.h"
#include "../tools_general.h"
#include "../Common/data_containers.h"

class MainWindow;

#ifndef DEBUG_MODE
#define MSG_HEADER_DELAY	(1000)	// ����� �������� ����� ��������� ��������� (ms)
#define MSG_REQ_DELAY		(16000)	// ����� �������� ������� �� ������������ ��������� (ms)
#define MSG_LIFETIME		(32000)	// "����� �����" ���������. ���������� ��� �������� ���������, ������� ��� � �� ���� ����������	

#define SDSP_REQ_DELAY		(10000)	// ����� �������� ������� �� ���������, ������������ � SDSP � ������ �����������
#else
#define MSG_HEADER_DELAY	(1000)	// ����� �������� ����� ��������� ��������� (ms)
#define MSG_REQ_DELAY		(16000)	// ����� �������� ������� �� ������������ ��������� (ms)
#define MSG_LIFETIME		(32000)	// "����� �����" ���������. ���������� ��� �������� ���������, ������� ��� � �� ���� ����������	

#define SDSP_REQ_DELAY		(16000)	// ����� �������� ������� �� ���������, ������������ � SDSP � ������ �����������
#endif

#define CLOCK_PERIOD		(50)


class Clocker : public QThread
{
    Q_OBJECT

public:
    explicit Clocker(QObject *parent = 0);

    void setPeriod(int ms);
    int period() { return ms_period; }
    int id() { return thread_id; }

public slots:
    void stopThread();

private:
    volatile int ms_period;
    volatile bool is_running;
    int thread_id;
	QTimer timer;

protected:
    void run();

signals:
    void clock();

};


class COMCommander : public QThread
{
	Q_OBJECT

public:
	explicit COMCommander(MainWindow *main_win, QObject *parent = 0);
	~COMCommander();
	
	void freeze();
	void wake();
	int id() { return thread_id; }
	bool getNMRToolState() { return nmrtool_state; }
	bool getSDSPToolstate() { return sdsptool_state; }	
	
private:	
	void initMsgSettings();
	//int findMsgHeader(QUEUE<uint8_t>* _queue);
	int findMsgHeader(QUEUE<uint8_t>* _queue, COM_Message *_msg);
	int checkMsgHeader(COM_Message *_msg);
	void searchMsgHeader(QUEUE<uint8_t> *_queue, QByteArray &str);
	
	//bool findMsgBody(bool &len_ok);
	void searchPackets(QByteArray &str, COM_Message *_msg, QUEUE<uint8_t>* _queue, QList<int> *shifts);
	void sendCOMMsg(COM_Message *msg);
	//void treatCOMData(QByteArray _str);
	//void treatCOMData(QByteArray _str, COM_Message *_msg);
	void treatCOMData(COM_Message *_msg);
	void executeServiceMsg(COM_Message *_msg);
	void executeShortMsg(COM_Message *_msg);
	void responseMultypackHeader(COM_Message *_msg, uint32_t _uid);
	void executeMultyPackMsg(COM_Message *_msg, uint32_t _uid, bool res);
	void clearCOMMsg(COM_Message *_msg);
	bool decodePackets(QUEUE<uint8_t> *data_q, COM_Message *_msg);	
	void decodePackByteArray(SmartArr byte_arr, int block_len, GF_Data *gf_data, SmartArr &res_byte_arr, SmartArr &res_data_arr, SmartArr &res_rs_arr, SmartArr &block_map);
	void repairPackData(MsgPacket *_pack);
	//void readFromCOMPort();
	
	void lock_COM_Msg() { com_msg_mutex->lock(); }
	void unlock_COM_Msg() { com_msg_mutex->unlock(); }

	void applyInterleavingOut(bool flag) { interleaving_out = flag; }
	bool interleaved_out() { return interleaving_out; }
		
	static int estimateBestPackLen(int data_len, int block_len, int rs_part_len);

private:
	MainWindow *main_win;

	QByteArray prebuff;							// ��������� ������, ���� ������������ ������� �� COM-����� (������������ ��� �������� ���������� ������)
	QUEUE<uint8_t> *head_q;
	QUEUE<uint8_t> *body_q;
	COM_Message* msg_critical;					// ���������� ������ ���������, ������� ������ ���� ������� ��� �������
	COM_Message* msg_incomming;					// �������� ���������
	COM_Message* msg_outcomming;				// ��������� ���������
	volatile bool com_data_ready;				// ���� � ������� ����� ������ ���� �� COM-�����
	
	GF_Data *gf_data;
	//QextSerialPort *COM_port;
	TCP_Settings nmrtool_socket;
	Clocker *clocker;

	QMutex *com_msg_mutex;

	QFile *incomming_msg_log;

	//QList<DeviceData*> device_data_list;	
	QUEUE<DeviceData*> device_data_queue;		// ������� ��������� �� ��������, ����������� �� �������� ���������
	QUEUE<DeviceData*> sent_device_data_queue;	// ������ ������������ � ���������� ������ ��������� �� ������� device_data_queue
	DeviceData* cur_device_data;
	MsgHeaderInfo last_request_msg;				// ��������� ������������ ��������� (��������� MsgHeader) - ��������� ��� ������� �������� � ������ ������� ��������� REPEAT_CMD 
		
	QList<COM_Message*> *com_msg_list;

	QList<QPair<uint8_t, uint32_t> > msg_ids_table;
	
	QTimer *msg_timer;
	
	volatile bool is_running;
	volatile bool is_freezed;

	volatile bool nmrtool_state;	// true - FPGA program in NMR Tool is started; false - is stopped
	volatile bool sdsptool_state;	// true - SDSP tool is started; false - is stopped
	volatile bool msg_started;		// now a message is receiving...
	volatile bool msg_answered;

	volatile MultyState msg_header_state;	// ��������� ������ ��������� ��������� (msg_header)
	volatile MultyState msg_packet_state;	// ��������� ������ ���������� ������ ��������� (msg_body)
	volatile MultyState incom_msg_state;	// ��������� ����� ������������ ���������
	volatile int pack_counter;				// ������� ���������� ������� ������������ ���������

	int thread_id;
	
	bool interleaving_out;					// ���������� ������������� (interleaving) ��� �������� ������
	bool on_break;							// ���������� ������������� �������� ���������� ��������.

	int msg_header_delay;					// ����� �������� ����� ��������� ��������� (ms)	
	int msg_life_time;						// "����� �����" ���������. ���������� ��� �������� ���������, ������� ��� � �� ���� ����������	
	int sdsp_req_delay;						// ����� �������� ������� �� ���������, ������������ � SDSP � ������ �����������

public:
	static int msg_req_delay;				// ����� �������� ������� �� ������������ ��������� (ms) - made static variable 5.07.2018

private:
	void showBadMessageAsText(COM_Message *msg, QString &text);
	
public slots:
	void addCmdToQueue(DeviceData* dev_data);
	void setNMRToolState(bool flag) { nmrtool_state = flag; }
	void setSDSPToolState(bool flag) { sdsptool_state = flag; }
	void breakAllActions(); //{ on_break = true; }
	void stopThread() { is_running = false; }
	void sendToSDSP(QByteArray& arr);
	void setMsgReqDelay(int _value);		// Added 5.07.2018

private slots:
	void timeClocked();
	void lifeTimeElapsed();
	void onDataAvailable();
	void storeCOMMsg(COM_Message* _msg);
		
protected:
	void run();

signals:    
	void error_msg(QString);
	void msg_state(int, int);					// ���������� ��������� �������������� ������ �� ������ ����� ������ � ������. 
												// ����� ����������� ��� ��������� ������/�������� ������������� ���������� ���������
	void COM_message(COM_Message*, uint32_t);
	//void bad_COM_message(COM_Message*);
	void bad_COM_message(QString);
	void store_COM_message(COM_Message*);	
	void device_data_timed_out(uint32_t _uid);
	//void device_data_timed_out(DeviceData*);
};



#define DEPTH_BUFF_SIZE		5
#define DEPTH_MAX_DELTA		10

// �����, ��������������� ����� � ����������� ������ �������,
// � ���������� ���������
class DepthCommunicator : public QThread
{
	Q_OBJECT

public:
	explicit DepthCommunicator(QTcpSocket *socket, Clocker *clocker, QObject *parent = 0);
	~DepthCommunicator();

	void freeze();
	void wake();
	int id() { return thread_id; }
	//bool getNMRToolState() { return nmrtool_state; }

	//void setPort(QextSerialPort *com_port);
	void setSocket(QTcpSocket *socket);

private:		
	void sendRequestToCOM(DepthMeterData *dmd);	
	//void treatCOMData(QByteArray _str, COM_Message *_msg);	

	//QextSerialPort *COM_port;	
	QTcpSocket *socket;
	Clocker *clocker;
	QTimer *msg_timer;
	
	QUEUE<uint8_t> *data_q;	

	QUEUE<DepthMeterData*> depth_data_queue;		// ������� �������� �� ���������
	QUEUE<DepthMeterData*> sent_depth_data_queue;	// ������ ������������ �������� �� ������� device_data_queue

	QVector<double> depth_buffer;			// ����� ���� FIFO ��������� DEPTH_BUFF_SIZE �������� ������� ��� �������� ��������� ������� �������

	volatile bool is_running;
	volatile bool is_freezed;
		
	int thread_id;
							
public slots:
	void toMeasure(uint32_t uid, uint8_t data_type);

private slots:
	void timeClocked();
	//void lifeTimeElapsed();
	void onDataAvailable();

protected:
	void run();

signals:    
	void error_msg(QString);
	void measured_data(uint32_t, uint8_t, double);	// uint32_t uid, uint8_t type, double result
	void data_timed_out(uint32_t, uint8_t);			// uint32_t uid, uint8_t type
	void frozen(bool);

};


// �����, ��������������� ����� � �������� ����������� Leuze ODS 96B,
// � ���������� ���������
class LeuzeCommunicator : public QThread
{
	Q_OBJECT

public:
	explicit LeuzeCommunicator(QTcpSocket *socket, Clocker *clocker, QObject *parent = 0);
	~LeuzeCommunicator();

	enum ErrCode { NoError = 0, NoSignal = 1, UnknownError = 2 };

	void freeze();
	void wake();
	int id() { return thread_id; }
	
	//void setPort(QextSerialPort *com_port);
	void setSocket(QTcpSocket *socket);

private:		
	void sendRequestToCOM();	

private:
	//QextSerialPort *COM_port;	
	QTcpSocket *socket;
	Clocker *clocker;
	
	QString acc_data;								// �������� ��������, ����������� �� ����
	QVector<QPair<int, int> > distance_buffer;		// ����� ���� FIFO ��������� �������� ������� (������ ��������), ������ �������� - ��� ������ (��. enum ErrCode)
														
	volatile bool is_running;
	volatile bool is_freezed;

	int thread_id;

public slots:
	void toMeasure(uint32_t uid, uint8_t data_type);

private slots:	
	void onDataAvailable();
	void timeClocked();

protected:
	void run();

signals:    
	void error_msg(QString);
	void measured_data(uint32_t, uint8_t, double);	// uint32_t uid, uint8_t type, double result
	void data_timed_out(uint32_t, uint8_t);			// uint32_t uid, uint8_t type
	void frozen(bool);

};



// �����, ��������������� ����� � ������ ���������� ������� ����������,
// � ����������� ������� ����������
class StepMotorCommunicator : public QThread
{
	Q_OBJECT

public:
	explicit StepMotorCommunicator(QTcpSocket *socket, QObject *parent = 0);
	~StepMotorCommunicator();

	void freeze();
	void wake();
	int id() { return thread_id; }
	
	//void setPort(QextSerialPort *com_port);
	void setSocket(QTcpSocket *socket);

private:		
	void sendRequestToCOM(QByteArray *arr);		

private:
	//QextSerialPort *COM_port;	
	QTcpSocket *socket;
	
	QString acc_data;								// �������� ��������, ����������� �� ����

	volatile bool is_running;
	volatile bool is_freezed;

	int thread_id;

public slots:
	void toSend(QString cmd);

private slots:	
	void onDataAvailable();

protected:
	void run();

signals:    
	void error_msg(QString);
	void measured_data(uint32_t, uint8_t, double);	// uint32_t uid, uint8_t type, double result
	void data_timed_out(uint32_t, uint8_t);			// uint32_t uid, uint8_t type
	void frozen(bool);

};


#endif // THREADS_H
