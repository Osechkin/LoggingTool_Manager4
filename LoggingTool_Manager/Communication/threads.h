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
#define MSG_HEADER_DELAY	(1000)	// время ожидания конца заголовка сообщений (ms)
#define MSG_REQ_DELAY		(16000)	// Время ожидания отклика на отправленное сообщение (ms)
#define MSG_LIFETIME		(32000)	// "Время жизни" сообщения. Необходимо для удаления сообщений, которые так и не были отправлены	

#define SDSP_REQ_DELAY		(10000)	// время ожидания отклика на сообщение, отправленное в SDSP в режиме диагностики
#else
#define MSG_HEADER_DELAY	(1000)	// время ожидания конца заголовка сообщений (ms)
#define MSG_REQ_DELAY		(16000)	// Время ожидания отклика на отправленное сообщение (ms)
#define MSG_LIFETIME		(32000)	// "Время жизни" сообщения. Необходимо для удаления сообщений, которые так и не были отправлены	

#define SDSP_REQ_DELAY		(16000)	// время ожидания отклика на сообщение, отправленное в SDSP в режиме диагностики
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

	QByteArray prebuff;							// первичеый буффер, куда складываются символы из COM-порта (используется при ожидании заголовков только)
	QUEUE<uint8_t> *head_q;
	QUEUE<uint8_t> *body_q;
	COM_Message* msg_critical;					// критически важное сообщение, которое должно быть послано вне очереди
	COM_Message* msg_incomming;					// входящее сообщение
	COM_Message* msg_outcomming;				// исходящее сообщение
	volatile bool com_data_ready;				// флаг о приходе новой порции байт из COM-порта
	
	GF_Data *gf_data;
	//QextSerialPort *COM_port;
	TCP_Settings nmrtool_socket;
	Clocker *clocker;

	QMutex *com_msg_mutex;

	QFile *incomming_msg_log;

	//QList<DeviceData*> device_data_list;	
	QUEUE<DeviceData*> device_data_queue;		// очередь сообщений на отправку, поступивших из основной программы
	QUEUE<DeviceData*> sent_device_data_queue;	// список отправленных в каротажный прибор сообщений из очереди device_data_queue
	DeviceData* cur_device_data;
	MsgHeaderInfo last_request_msg;				// последнее отправленное сообщение (заголовок MsgHeader) - требуется для повтора отправки в случае прихода сообщения REPEAT_CMD 
		
	QList<COM_Message*> *com_msg_list;

	QList<QPair<uint8_t, uint32_t> > msg_ids_table;
	
	QTimer *msg_timer;
	
	volatile bool is_running;
	volatile bool is_freezed;

	volatile bool nmrtool_state;	// true - FPGA program in NMR Tool is started; false - is stopped
	volatile bool sdsptool_state;	// true - SDSP tool is started; false - is stopped
	volatile bool msg_started;		// now a message is receiving...
	volatile bool msg_answered;

	volatile MultyState msg_header_state;	// состояние приема заголовка сообщения (msg_header)
	volatile MultyState msg_packet_state;	// состояние приема очередного пакета сообщения (msg_body)
	volatile MultyState incom_msg_state;	// состояние всего принимаемого сообщения
	volatile int pack_counter;				// счетчик приходящих пакетов принимаемого сообщения

	int thread_id;
	
	bool interleaving_out;					// применение перемешивания (interleaving) для выходных данных
	bool on_break;							// показатель необходимости прервать выполнение операций.

	int msg_header_delay;					// время ожидания конца заголовка сообщений (ms)	
	int msg_life_time;						// "Время жизни" сообщения. Необходимо для удаления сообщений, которые так и не были отправлены	
	int sdsp_req_delay;						// время ожидания отклика на сообщение, отправленное в SDSP в режиме диагностики

public:
	static int msg_req_delay;				// Время ожидания отклика на отправленное сообщение (ms) - made static variable 5.07.2018

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
	void msg_state(int, int);					// Количество неуспешно декодированных блоков от общего числа блоков в пакете. 
												// Также применяется для сообщении успеха/неуспеха декодирования заголовков сообщений
	void COM_message(COM_Message*, uint32_t);
	//void bad_COM_message(COM_Message*);
	void bad_COM_message(QString);
	void store_COM_message(COM_Message*);	
	void device_data_timed_out(uint32_t _uid);
	//void device_data_timed_out(DeviceData*);
};



#define DEPTH_BUFF_SIZE		5
#define DEPTH_MAX_DELTA		10

// поток, устанавливающий связь с измерителем глубин Импульс,
// и проводящий измерения
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

	QUEUE<DepthMeterData*> depth_data_queue;		// очередь запросов на измерения
	QUEUE<DepthMeterData*> sent_depth_data_queue;	// список отправленных запросов из очереди device_data_queue

	QVector<double> depth_buffer;			// буфер типа FIFO последних DEPTH_BUFF_SIZE измрений глубины для удаления случайных скачков глубины

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


// поток, устанавливающий связь с лазерным дальномером Leuze ODS 96B,
// и проводящий измерения
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
	
	QString acc_data;								// приемник символов, поступающих по сети
	QVector<QPair<int, int> > distance_buffer;		// буфер типа FIFO последних измрений глубины (второй параметр), первый параметр - код ошибки (см. enum ErrCode)
														
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



// поток, устанавливающий связь с платой управления шаговым двигателем,
// и управляющий шаговым двигателем
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
	
	QString acc_data;								// приемник символов, поступающих по сети

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
