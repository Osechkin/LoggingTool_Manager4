#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <QtCore>
#include <QMutex>

#include "message_class.h"


class MsgInfo
{
public:
	enum ParsingResult { Message_OK,			// принятое сообщение COM_Message успешно разобрано					
						No_Parse,				// входящее сообщение еще не разобрано или было создано рабочей станцией оператора для отправки
						Data_NotFound,			// при изъятии данных из принятого сообщения COM_Message самих данных не обнаружено
						Command_NotFound,		// невозможно прочитать команду в первом пакете сообщения COM_Message
						Bad_Command,			// неизвестная команда в сообщении COM_Message 
						Bad_DataLength,			// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 2 байта (длина данных) + 1 байт (crc)
						Unknown_Error			// ошибка неизвестной природы (может возникнуть при неудачной работе функции extractAllAsBytes(...))
					 };

	MsgInfo();
	MsgInfo(COM_Message *_msg);
	~MsgInfo();

	COM_Message *getCOMMessage() { return msg; }		
	uint8_t getPacketCount() { return packet_count; }
	DeviceData *getDeviceData() { return device_data; }

	void setParsingResult(ParsingResult _result) { parse_result = _result; }
	ParsingResult getParsingResult() { return parse_result; }

	void registrateMsg(COM_Message *_msg);
	static void extractAllAsText(QStringList &list, COM_Message *_msg);
	static void extractAllAsBytes(SmartArr *arr, SmartArr *bad_map, COM_Message *_msg);
	static QString SmartArrToHexString(SmartArr &sarr);
	void addField(Field_Comm &_field);
	void setDeviceDataMemo(QString _name, uint8_t comm_id, uint8_t type);

private:	
	uint32_t UId;
	uint64_t registration_time;
	ParsingResult parse_result;
	COM_Message *msg;
	DeviceData *device_data;	
	uint8_t packet_count;
};


struct IncomMsg
{
	COM_Message *msg;
	uint32_t msg_id;
};

class MsgProcessor : public QObject // public QThread  //public QObject 
{
	Q_OBJECT

public:	
	MsgProcessor(QMutex *_mutex);
	~MsgProcessor()  { /*emit finished();*/ }

	static void showBadMessageAsText(COM_Message *msg, QString &out_str);

//protected:
//	void run();

public slots:
	void sendMsg(DeviceData *device_data, const QString &obj_name);	

private:
	QString SmartArrToHexString(SmartArr &sarr);	
	void showAllAsText(MsgInfo *msg_info);
	void showAllAsText(COM_Message *msg);	
	//void processMsgFromCOMCommander();
	MsgInfo::ParsingResult extractData(MsgInfo *msg_info);
	COM_Message* copyCOMMessage(COM_Message *_msg);
	MsgHeader* copyMsgHeader(MsgHeader *_hdr);
	MsgPacket* copyMsgPacket(MsgPacket* _pack);

	int calcClk(int val);
	
	void lockMsg() { com_msg_mutex->lock(); }
	void unlockMsg() { com_msg_mutex->unlock(); }
	QMutex *com_msg_mutex;

	//volatile bool is_running;

	//IncomMsg incom_msg;
	//volatile bool msg_is_coming;

	QList<DeviceData*> waiting_list;
	QStringList waiting_obj_list;
	
private slots:
	void receiveMsgFromCOMComander(COM_Message *_msg, uint32_t _uid);	
	void receiveBadMsgFromCOMCommander(/*COM_Message *_msg*/ QString out_str);
	void reportNoResponse(uint32_t _uid);
	void treatIncommingData(uint32_t _uid);
	//void reportNoResponse(DeviceData* device_data);
	void sendMsg(DeviceData *device_data);	
	//void elapsedMsgTimer();
	
signals: 
	void message_str(QString);
	void message_str_to_Tool_console(QString);
	//void plot_data(DeviceData *);
	//void export_data(DeviceData *);
	void new_data(DeviceData *);
	void message_info(MsgInfo *);
	void execute_cmd(DeviceData*);
	void show_cmd_result(uint32_t, QString, QVariantList);	
	void power_status(unsigned char);	
	void fpga_seq_status(unsigned char);	// proger sequence completed/not completed (1/0)
	//void finished();
	void send_to_cdiag(QVector<uint8_t>);
	void apply_tool_id(unsigned char);
	//void send_to_sdsp
};

#endif // MESSAGE_PROCESSOR_H