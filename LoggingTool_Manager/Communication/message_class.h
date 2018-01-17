#ifndef MESSAGE_CLASS_H
#define MESSAGE_CLASS_H

#include <QList>
#include <QString>
#include <QVector>
#include <QDebug>
#include <QMutex>

#include <boost\shared_ptr.hpp>
#include <boost\make_shared.hpp>

#include "../io_general.h"
#include "../Galois/gf_data.h"
#include "../io_containers.h"


#define START_BYTE			0xAA
#define STOP_BYTE			0x55


extern uint8_t Crc8(uint8_t *pcBlock, uint16_t len);
extern uint16_t Crc16( uint8_t *pcBlock, uint16_t len );


struct SmartArr
{
	SmartArr();
	SmartArr(int n);

	~SmartArr();

	void alloc(int n);

	void allocf(int n, uint8_t val);
	void allocf(int n, uint16_t val);
	void allocf(int n, uint32_t val);
	void allocf(SmartArr arr);

	void realloc(int n);

	int fill(uint8_t val);
	int fill(uint16_t val);
	int fill(uint32_t val);
	int filla(uint8_t val, uint16_t n);

	void concat(uint8_t *arr, uint16_t n);
	void concat(SmartArr arr);

	void destroy();

	//uint8_t *data;
	boost::shared_ptr<uint8_t[]> data;
	uint16_t len;
};


struct Field_Comm
{
	// UINT8 = uint8_t, UINT16 = uint16_t, UINT32 = uint32_t, FLOAT = float, DOUBLE = double;
	enum Value_Type { NOTYPE, UINT8, UINT16, UINT32, FLOAT, DOUBLE };
	
	Field_Comm()
	{
		name = "";
		code = 0;
		str_value = "";
		value_type = NOTYPE;		
		value = NULL;
		bad_data = NULL;
		tag = 0;
		channel = 0;
	}

	Field_Comm(QString _name, QString _str, Value_Type _type)
	{
		name = _name;
		code = 0;
		str_value = _str;
		value_type = _type;
		value = NULL;
		bad_data = NULL;
		tag = 0;
		channel = 0;
	}

	~Field_Comm()
	{
		if (value != NULL) delete value;
		if (bad_data != NULL) delete bad_data;		
	}
	
	bool isEmpty()
	{
		if (value == NULL) return true;
		else return value->isEmpty();
	}

	bool friend operator==(const Field_Comm& left, const Field_Comm& right)
	{
		return (left.name == right.name) && (left.str_value == right.str_value) && (left.value_type == right.value_type);
	}

	QString name;
	uint8_t	code;				// код данных (в качестве кода данных используется, например, CPMG_PRG и пр.)
	int tag;					// переменная, используемая по необходимости (например, в качкстве группового индекса) 
	uint8_t channel;			// код канала данных (например, первый датчик ЯМР, второй датчик, гамма-каротаж и т.д.)
	QString str_value;			// строковое представление данных - используется визуальными компонентами; может быть равным ""
	QVector<double> *value;
	QVector<uint8_t> *bad_data;	// индикатор сомнительных или "испорченных" данных (возникают при приеме данных). 
								// Длина value должна быть равна длине bad_data. Значения bad_data: DATA_OK/BAD_DATA
	Value_Type value_type;
};


struct DeviceData
{
	DeviceData();
	DeviceData(uint8_t id, QString _name);
	DeviceData(uint8_t id, QString _name, uint32_t _uid);
	~DeviceData();

	static void linkCmdData(DeviceData *dev_data);	
	
	//uint16_t getCommandLength();

	uint32_t uid;			// уникальный id для распознания отправки сообщения и затем распознаия ответа на него
	QString name;			// имя команды/набора данных
	uint8_t comm_id;		// код команды/набора данных
	uint8_t type;			// "Тип" команды, определяет тот или иной тип UART-сообщения для передачи (сервисное сообщение, короткое или многопакетное)
	
	QList<Field_Comm*> *fields;	// данные 
	
	int elapsed_time;		// время, прошедшее с момента отправки данных объекта в каротажный прибор
	int life_time;			// счетчик времни, прошедшего с момента записи объекта DeviceData в очередь на отправку в каротажный прибор,
							// т.е. "время жизни" объекта
	bool is_sent;			// индикатор отправлены ли данные объекта в каротажный прибор 
};
typedef QList<DeviceData*> DeviceDataList;


struct MsgHeaderInfo
{
	MsgHeaderInfo()
	{
		start_marker = 0;
		w_addr = 0;
		r_addr = 0;
		id = 0;
		pack_count = 0;
		pack_len = 0;
		block_len = 0;
		err_count = 0;
		crc = 0;
		srv_data.data = NULL;
		srv_data.len = 0;
	}

	~MsgHeaderInfo()
	{
		srv_data.destroy();
	}

	void clear()
	{
		start_marker = 0;
		w_addr = 0;
		r_addr = 0;
		id = 0;
		pack_count = 0;
		pack_len = 0;
		block_len = 0;
		err_count = 0;
		crc = 0;
		srv_data.len = 0;
	}
	
	void setSrvData(uint8_t *data, int len)
	{
		if (srv_data.len > 0) srv_data.destroy();		
		srv_data.alloc(len);
		
		//memcpy(srv_data.data, data, sizeof(uint8_t)*len);
		memcpy(srv_data.data.get(), data, sizeof(uint8_t)*len);
	}
	void setSrvData(SmartArr sarr)
	{
		srv_data.allocf(sarr);
	}

	uint8_t start_marker;			// маркер начала заголовка сообщения; также является id типа отправляемого сообщения
	uint8_t w_addr;					// адреса устройства-отправителя сообщения 
	uint8_t r_addr;					// адреса устройства-адресата сообщения 
	uint8_t id;						// идентификатор сообщения
	uint8_t pack_count;				// число пакетов в сообщении
	uint8_t pack_len;				// длина пакетов в байтах
	uint8_t block_len;				// длина блоков Рида-Соломона, из которых сосотоят данные в пакете
	uint8_t err_count;				// число восстанавливаемых ошибок в каждом блоке 
	uint8_t crc;					// контрольная сумма
	SmartArr srv_data;				// служебные данные (только для случая служебных сообщений, у которых вместо полей 
};



/* Структура принимаемых по UART сообщений, отправленных рабочей станцией оператора на поверхности:
 *
 * |-----------|	|-----------|	|----------|	...		|-----------|
 *   заголовок		   пакет 1		  пакет 2				   пакет N
 *
 *
 *
 *  Заголовок сообщений (класс MsgHeader):
 *
 *  1 байт	1 байт	1 байт	1 байт	1 байт	1 байт	1 байт	1 байт	   ( 4 байта )
 *  |-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|----- ... -----|
 *	 MARK	 ADDR	  ID	 NPACK	 LPACK	LBLOCK	 NERRS	 CRC8		 RSBYTES
 *
 *	 MARK	- стартовый маркер заголовка сообщения. Также кодирует тип сообщения (см. MTYPE в файле io_general.h)
 *	 ADDR	- адреса устройства-отправителя и устройства-адресата сообщения. Формат: биты 0-3 - адрес отправителя, биты 4-7 - адрес приемника
 *	 ID		- номер сообщения (всего от 0 до 255, т.е. номер сообщения не обязан быть уникальным в течение всей работы прибора ЯМК)
 *	 NPACK	- число пакетов в сообщении
 *	 LPACK	- длина пакетов в сообщении
 *	 LBLOCK	- длина блоков, на которые разбиваются пакеты для блочного кодирования Рида-Соломона
 *	 NERRS	- число ошибок, которые могут быть восстановлены в каждом блоке Рида-Соломона
 *	 CRC8	- контрольная сумма, которая вычисляется по байтам с MARK по NERRS
 *	 RSBYTES- проверочные байты, предназначенные для устранения ошибок в заголовке сообщения. Обычно число проверочных байт = 4
 *
 *
 *
 *  Пакеты сообщений (класс MsgPacket):
 *
 *  1 байт  1 байт  1 байт        N байт           1 байт
 *  |-----| |-----| |-----| |-------...--------| |-----|
 *   MARK     ID      NUM           Data           CRC8
 *
 *  MARK	- маркер пакета сообщения.
 *  ID		- номер сообщения (всего от 0 до 255, т.е. номер сообщения не обязан быть уникальным в течение всей работы прибора ЯМК)
 *  NUM		- номер пакета
 *  CRC8	- контрольная сумма по всему пакету, которая вычисляется с позиции MARK включительно
 *
 *
 *  После формирования пакета, производится кодирование по методу Рида-Соломона. 
 *  В результате закодированный пакет представляет собой последовательность РС-блоков:
 *  
 *  LBLOCK байт   LBLOCK байт       ...    LBLOCK байт
 *  |----------|  |----------|             |----------|
 *  Packet Data   Packet Data              Packet Data
 *
 *  LBLOCK	- длина блоков, на которые разбиваются пакеты для блочного кодирования Рида-Соломона (см. выше описание структуры MsgHeader)
 *
 *
 *  При этом структура каждого блока:
 *
 *     M байт    K байт
 *  |---------| |-------|			M + K = LBLOCK
 *     DATA      RSBYTES
 *
 *  DATA	- кодируемые данные. Здесь M = LBLOCK - K = LBLOCK - 2*NERRS (про NERRS см. описание MsgHeader выше)
 *  RSBYTES - проверочные байты, предназначенные для устранения ошибок в текущем блоке сообщения. Здесь K = 2*NERRS
 *
 *  
 *
 *
 *	Короткое сообщение не содержит пакетов. а вся информация пересылается в теле заголовка. 
 *	В этом случае структура короткого сообщения следующая:
 *	
 *	Заголовок сообщений (класс MsgHeader):
 *
 *  1 байт	1 байт	1 байт	1 байт	1 байт	1 байт	1 байт	1 байт	   ( 4 байта )
 *  |-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|----- ... -----|
 *	 MARK	 ADDR	  ID	 BYTE0	 BYTE1	 BYTE2	 BYTE3	 CRC8		 RSBYTES
 *
 *	 MARK	- стартовый маркер заголовка сообщения. Также кодирует тип сообщения (см. MTYPE в файле io_general.h)
 *	 ADDR	- адреса устройства-отправителя и устройства-адресата сообщения. Формат: биты 0-3 - адрес отправителя, биты 4-7 - адрес приемника
 *	 ID		- номер сообщения (всего от 0 до 255, т.е. номер сообщения не обязан быть уникальным в течение всей работы прибора ЯМК)
 *	 BYTE0 - BYTE3	- байты данных в сообщении
 *	 CRC8	- контрольная сумма, которая вычисляется по байтам с MARK по NERRS
 *	 RSBYTES- проверочные байты, предназначенные для устранения ошибок в заголовке сообщения. Обычно число проверочных байт = 4
 *
 *
 *
 * 
 *	Служебное сообщение о готовности прибора к сеансу приема/передачи по каротажному кабелю содержит в себе следующую информацию:
 *	BYTE0:
 *	биты 0-7 - код передаваемой команды (например, DATA_OK - см. команды в io_general.h )
 *
 *	BYTE1:
 *	бит 0 - напряжение питания: 						0/1 - понижено / нормальное
 *	бит 1 - готовность телеметрии: 						0/1 - не готова / готова
 *	бит 2 - состояние программатора ПЛИС: 				0/1 - запущен командой proger_start() / остановлен командой proger_stop()
 *	бит 3 - состояние программы в программаторе ПЛИС: 	0/1 - исполняется / завершилась
 *
 */
class MsgHeader
{
public: 
	MsgHeader(GF_Data *_gf_data);
	MsgHeader(MsgHeaderInfo *_hd_info, GF_Data *_gf_data);
	~MsgHeader();

	void setInfoData(MsgHeaderInfo _info, GF_Data *_gf_data);
	void fillAllData(MsgHeaderInfo *_info);
	void setMsgType(MTYPE type);
	void setReader(uint8_t r);
	void setWriter(uint8_t w);
	void setWRAddress(uint8_t w, uint8_t r);	
	void setSessionId(uint8_t val);
	void setPackCount(uint8_t val);
	void setPackLen(uint8_t val);
	void setBlockLen(uint8_t val);
	void setErrsCount(uint8_t val);
	void setCRC(uint8_t _crc);

	bool setSrvData(SmartArr &arr);
	bool setShortData(SmartArr &arr);

	static uint8_t generateSessionId();	
	
	uint8_t getStartMarker() { return start_marker; }
	uint8_t getWriter();
	uint8_t getReader();
	uint8_t getWrRdAddress() { return wr_addr; }
	uint8_t getSessionId() { return session_id; }
	uint8_t getPackCount() { return pack_count; }
	uint8_t getPackLen() { return pack_len; }
	uint8_t getBlockLen() { return block_len; }
	uint8_t getErrsCount() { return err_count; }
	uint8_t getCRC() { return crc; }
	SmartArr getRecData() { return rec_data; }
	SmartArr getSrvData() { return srv_data; }
	SmartArr getShortData() { return srv_data; }

	//MsgHeaderInfo getMsgHeaderInfo();
	MTYPE getMsgType() { return (MTYPE)start_marker; }

	uint8_t getDataLen() { return HEAD_INFO_LEN; }
	uint8_t getRecDataLen() { return HEAD_REC_LEN; }
	uint8_t getHeaderLen() { return HEADER_LEN; }

	SmartArr getByteArray() { return byte_array; }

	GF_Data *getGFData() { return gf_data; }

	void clearMsgHeader();
	void copyMsgHeader(MsgHeader *dst_hdr);

private:
	uint8_t calcCRC();
	void create_byte_arr();
	void calcRecoveryPart();	// вычислить проверочную часть для заголовка

	uint8_t start_marker;		// маркер начала заголовка сообщения; также является id типа отправляемого сообщения
	uint8_t wr_addr;			// адреса устройства-отправителя сообщения и устройства-адресата
	uint8_t session_id;			// идентификатор сеанса сообщений
	uint8_t pack_count;			// число пакетов в сообщении
	uint8_t pack_len;			// длина пакетов в байтах
	uint8_t block_len;			// длина блоков Рида-Соломона, из которых сосотоят данные в пакете
	uint8_t err_count;			// число восстанавливаемых ошибок в каждом блоке 
	uint8_t crc;				// контрольная сумма crc8, расчитанная по данным с start_marker по err_count
	SmartArr rec_data;			// проверочная часть массива Рида-Соломона
	SmartArr srv_data;			// служебные данные (только для случая служебных сообщений, у которых вместо полей 
								// pack_count, pack_len, block_len и err_count присутствует массив srv_data)

	SmartArr byte_array;		// байтовая форма отпрвляемого заголовка сообщения

	GF_Data *gf_data;
};



class MsgPacket
{
public:
	MsgPacket(uint8_t _pack_num, MsgHeader *_msg_header, bool interleaved = false);
	~MsgPacket();
	
	MsgHeader *getMsgHeader() { return msg_header; }

	uint8_t getStartmarker() { return start_marker; }
	uint8_t getSessionId() { return session_id; }
	uint8_t getPacketNumber() { return packet_number; }
	uint8_t getBlockLen() { return block_len; }
	uint8_t getErrsCount() { return errs_count; }
	uint8_t getCRC() { return crc; }
	uint8_t getPacketLen() { return packet_len; }
	uint8_t getDataLen() { return data_len; }
	uint8_t getBlockCount() { return block_count; }
	
	SmartArr getByteArray() { return byte_arr; }
	SmartArr getDataArray() { return data_arr; }
	SmartArr getBlockMap() { return block_map; }
	SmartArr getRecData() { return rec_data; }

	bool interleaved() { return (start_marker == MTYPE_PACKET_ILV ? true : false); }

	// функция pushData "забирает" из массива arr кол-во байт, равное data_len начиная с позиции start_pos. 
	// При этом, start_pos перемещается в позицию start_pos + data_len. Массив arr не изменяется.
	//void pushData(SmartArr arr, uint16_t &start_pos);
	void pushData(SmartArr data, uint16_t &pos);
	void pushData(uint8_t *arr, uint16_t arr_len, uint16_t &start_pos);
	// setData() записывает данные (byte_array, data_array, block_map, rec_data) из пакета dst в текущий пакет 
	// возвращает false, если копируемые данные не удоблетворяют требований пакета-приемника по длине и пр. параметрам из MsgHeader
	bool setData(MsgPacket *src);

private:	
	void create_byte_arr();
	uint8_t calcCRC();
	void calcRecoveryPart(uint8_t *arr, uint16_t pos, uint8_t num_block);
	// функция calcRecoveryPart вычисляет массив проверочных байт для массива arr, начиная с позиции pos.
	// Длина данных (в байтах), для которых вычисляется массив проверочных байт, равна block_len - 2*errs_count,
	// результат копируется в массив rec_data с позиции 2*errs_count*num_block в количестве 2*errs_count байт.
	// Нумерация блоков (num_blocks) начинается с 0.
	
	uint8_t start_marker;		// маркер начала пакета
	uint8_t session_id;			// id сеанса
	uint8_t packet_number;		// номер пакета
	uint8_t crc;

	uint8_t block_len;			// длина блоков Рида-Соломона
	uint8_t errs_count;			// кол-во исправляемых ошибок в каждом блоке Рида-Соломона
	uint8_t data_len;			// длина данных, содержащихся в пакете. data_len = (packet_len \ block_len)*block_len (с учетом служебных данных пакета, составляющих 4 байта)
	uint8_t packet_len;			// длина пакета. 
	uint8_t block_count;		// кол-во блоков Рида-Соломона в пакете

	MsgHeader *msg_header;
	GF_Data *gf_data;
	
	SmartArr data_arr;			// исходные данные, переданые в пакет (без шапки и проверочных байт блоков Рида-Соломона)
	SmartArr byte_arr;			// готовый к передаче поток данных, представляющий собой закодированные данные методом блочного кодирования Рида-Соломона
	
	SmartArr block_map;			// массив-карта блоков: 
	SmartArr rec_data;			// временный массив для хранения блоков Рида-Соломона
};


class COM_Message
{
public:
	COM_Message(GF_Data *_gf_data);
	COM_Message(MsgHeader *_msg_header, QList<MsgPacket*> _packet_list = QList<MsgPacket*>());
	~COM_Message();

	void clearMsgHeader();
	void clearPacketList();
	void clearCOMMsg();
	void copyCOMMsg(COM_Message *dst_msg);

	enum Priority { HIGH_PRIORITY, NORMAL_PRIORITY };
	Priority getPriority() { return priority; }
	void setPriority(Priority _priority) { priority = _priority; }

	enum IOStatus { NOT_DEFINED, NOT_COMPLETED, COMPLETED, HEADER_SENT, SENT };
	IOStatus getIOStatus () { return io_status; }
	void setIOStatus(IOStatus _io_status) { io_status = _io_status; }
		
	MTYPE getMsgType() { return (MTYPE)msg_header->getStartMarker(); }
	uint8_t getWriter() { return writer; }
	uint8_t getReader() { return reader; }
	uint8_t getSessionId() { return session_id; }
	uint8_t getCmd();

	void setMsgId(uint32_t _msg_id) { msg_id = _msg_id; }
	static uint32_t generateMsgId();
	uint32_t getMsgId() { return msg_id; }

	void setSessionId(uint8_t _id) { session_id = _id; }
	void setWriter(uint8_t _w) { writer = _w; }
	void setReader(uint8_t _r) { reader = _r; }
	
	void getHeaderRawData(SmartArr *arr);
	void getBodyRawData(SmartArr *arr);
	void getPacketRawData(SmartArr *arr, int index);
	void getRawData(SmartArr *arr); 
	QList<MsgPacket*> getPackets() { return packet_list; }
	MsgHeader *getMsgHeader() { return msg_header; }
	GF_Data *getGFData() { return gf_data; }
	void addPacket(MsgPacket* _pack);  

	bool interleaved() { return interleave_state; }
	void setInterleavedState(bool flag);
	static void getInterleavedPositions(uint8_t len, SmartArr *arr);
	void interleavePacketData(SmartArr *arr);
	
	int getParcelCount() { return parcel_list.count(); }
	QList<int> getParcelList() { return parcel_list; }
	bool addParcel(uint8_t index);

	void setStored(bool flag) { storage_status = flag; }
	bool stored() { return storage_status; }

	/*void setPackStarted(bool flag) { pack_started = flag; }
	void setPackStoped(bool flag) { pack_stoped = flag; }
	bool packIsStarted() { return pack_started; }
	bool packIsStoped() { return pack_stoped; }
	static uint8_t getStartByte() { return START_BYTE; }
	static uint8_t getStopByte() { return STOP_BYTE; }*/

private:
	MsgHeader *msg_header;
	QList<MsgPacket*> packet_list;

	QList<int> parcel_list;
	
	uint8_t writer;
	uint8_t reader;
	uint8_t session_id;
	uint32_t msg_id;
	
	bool interleave_state;	// признак перемешанных данных в пакетах

	Priority priority;
	IOStatus io_status;		// флаг завершения процедуры передачи или приема сообщения (особо важен при приеме/передаче многопакетного сообщения)	
	bool storage_status;	// флаг, показывающий было ли сообщение передано в хранилище сообщений
	
	GF_Data *gf_data;	
};

/*
// структура, предназначенная для контроля обмена сообщениями
struct Scheduler
{
	Scheduler() 
	{ 
		id = 0; 
		started = false; 
	}

	uint8_t id;					// id сеанса передачи
	QList<uint16_t> delays;		// задержки (в [ms]) ожидания ответа на заголовок сообщения, пакеты и т.д. 
	QList<uint16_t> report;		// отчет о соблюдении задержек в расписании "передача сообщения - ответ на передачу" 
	bool started;				// флаг состояния объекта Scheduler (работает/не работает)
};
*/


struct DepthMeterData
{	
	DepthMeterData()
	{
		uid = 0;
		type = DEVICE_SEARCH;
		is_sent = false;
		elapsed_time = 0;
		life_time = 0;
		data = NAN;
	}
	DepthMeterData(uint32_t _uid, uint8_t _type)
	{
		uid = _uid;
		type = _type;
		is_sent = false;
		elapsed_time = 0;
		life_time = 0;
		data = NAN;
	}
	
	uint32_t uid;			// уникальный номер запроса к глубиномеру
	uint8_t type;			// "Тип" команды, определяет тот или иной тип измерений (глубина, скорость спуска, натяжение)
	
	int elapsed_time;		// время, прошедшее с момента отправки данных объекта в каротажный прибор
	int life_time;			// счетчик времни, прошедшего с момента записи объекта DepthMeterData в очередь на измерения, т.е. "время жизни" объекта	

	bool is_sent;			// индикатор отправки запроса в глубиномер Импульс

	double data;			// результат измерений
};

#endif // MESSAGE_CLASS_H



