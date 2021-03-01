#include <QDateTime>
#include <QDebug>

#include "message_processor.h"
#include "../io_general.h"
#include "../tools_general.h"

#include "../Common/profiler.h"


MsgInfo::MsgInfo()
{
	UId = 0;
	registration_time = 0;

	msg = NULL;
	device_data = new DeviceData;
	packet_count = 0;
	parse_result = No_Parse;
};

MsgInfo::MsgInfo(COM_Message *_msg)
{
	msg = _msg;
	device_data = new DeviceData;
	device_data->uid = msg->getMsgId();
	
	parse_result = No_Parse;
	
	uint16_t packs = msg->getPackets().count();
	if (packs > 255) packet_count = 255;
	else packet_count = (uint8_t)packs;

	registrateMsg(msg);
}

MsgInfo::~MsgInfo()
{	
	if (device_data != NULL) 
	{
		delete device_data;	
		device_data = NULL;
	}
}

void MsgInfo::registrateMsg(COM_Message *_msg)
{
	UId = _msg->getMsgId();
	
	QDateTime cur_time = QDateTime::currentDateTime();
	uint64_t ctime_u64 = (uint64_t)(cur_time.toTime_t());
	
	uint64_t ms = (uint64_t)(cur_time.time().msec());
	registration_time = ctime_u64*1000 + ms;	
}


void MsgInfo::setDeviceDataMemo(QString _name, uint8_t comm_id, uint8_t type)
{
	device_data->name = _name;
	device_data->comm_id = comm_id;
	device_data->type = type;
}

void MsgInfo::extractAllAsText(QStringList &list, COM_Message *_msg)
{
	//if (list == NULL) return;
	list.clear();

	if (_msg == NULL) return;	

	QString header_str = SmartArrToHexString(_msg->getMsgHeader()->getByteArray());
	list.append(header_str);
	for (int i = 0; i < _msg->getPackets().count(); i++)
	{
		QString pack_str = SmartArrToHexString(_msg->getPackets()[i]->getByteArray());
		list.append(pack_str);
	}
}

void MsgInfo::extractAllAsBytes(SmartArr *arr, SmartArr *bad_map, COM_Message *_msg)
{	
	MTYPE msg_type = _msg->getMsgType();
	if (msg_type == MTYPE_SERVICE || msg_type == MTYPE_SHORT) 
	{		
		MsgHeader *header = _msg->getMsgHeader();
		SmartArr srv_arr = header->getSrvData();
		arr->allocf(srv_arr); 	
		bad_map->alloc(srv_arr.len);
		bad_map->filla((uint8_t)DATA_STATE_OK, srv_arr.len);
	}
	else if (msg_type == MTYPE_MULTYPACK)
	{				
		// читается id заголовка, чтобы исключать те пакеты, у которых id отличается от данного
		uint8_t header_id = _msg->getMsgHeader()->getSessionId();
		
		uint8_t pack_len = _msg->getMsgHeader()->getPackLen();
		uint8_t block_len = _msg->getMsgHeader()->getBlockLen();
		uint8_t block_count = pack_len/block_len;
		uint8_t errs_bytes = 2 * _msg->getMsgHeader()->getErrsCount();
		uint8_t pack_data_len = (block_len - errs_bytes)*block_count;
		uint8_t sent_packets = _msg->getMsgHeader()->getPackCount();		// число пакетов, которые должны были быть отправленными (но не факт, что дошли)
		if (sent_packets == 0) 
		{
			return;
		}

		// зарезервировать место для данных, которые должны были дойти
		uint16_t data_len = (uint16_t)(pack_data_len * sent_packets - (sent_packets-1)*(PACK_INFO_LEN+1)) - 1;
		SmartArr raw_data(data_len);
		SmartArr bad_data(data_len);
		bad_data.filla((uint8_t)DATA_STATE_FAIL, data_len);
		//qDebug() << "Data length: " << data_len;
		
		uint8_t pure_data_len = pack_data_len - PACK_INFO_LEN - 1;
		for (int i = 0; i < _msg->getPackets().count(); i++)
		{			
			MsgPacket *packet = _msg->getPackets().at(i);
			uint8_t pack_number = packet->getPacketNumber();
			SmartArr data = packet->getDataArray();
			
			uint8_t crc = Crc8(data.data.get(), data.len-1);
			if (pack_number == 1)
			{								
				memcpy(raw_data.data.get(), data.data.get(), (pack_data_len-1)*sizeof(uint8_t));
				uint8_t crc_pack = data.data[data.len-1];
				if (crc == crc_pack)
				{
					for (int j = 0; j < packet->getBlockMap().len; j++)
					{
						if (packet->getBlockMap().data[j] == DATA_STATE_OK)
						{
							uint16_t start_pos = (block_len-errs_bytes)*j;
							uint8_t len = block_len-errs_bytes;
							if (j == packet->getBlockMap().len-1) len -= 1;
							memset(bad_data.data.get() + start_pos, (uint8_t)DATA_STATE_OK, len*sizeof(uint8_t));
						}
					}
				}
				else 
				{
					return;
				}
			}
			else 
			{
				uint16_t pos = pure_data_len*(pack_number-1) + PACK_INFO_LEN;				
				memcpy(raw_data.data.get() + pos, data.data.get() + PACK_INFO_LEN, pure_data_len*sizeof(uint8_t));
				uint8_t crc_pack = data.data[data.len-1];
				if (crc == crc_pack)
				{
					for (int j = 0; j < packet->getBlockMap().len; j++)
					{
						if (packet->getBlockMap().data[j] == DATA_STATE_OK)
						{		
							uint16_t start_pos = (block_len-errs_bytes-PACK_INFO_LEN) + (j-1)*(block_len-errs_bytes) + pure_data_len*(pack_number-1) + PACK_INFO_LEN;
							uint8_t len = block_len-errs_bytes;
							if (j == 0) 
							{
								start_pos = (pure_data_len)*(pack_number-1) + PACK_INFO_LEN;
								len -= PACK_INFO_LEN;
							}
							else if (j == packet->getBlockMap().len-1)
							{
								len -= 1;
							}							
							//memset(bad_data.data + start_pos, (uint8_t)DATA_STATE_OK, len*sizeof(uint8_t));
							memset(bad_data.data.get() + start_pos, (uint8_t)DATA_STATE_OK, len*sizeof(uint8_t));
						}
					}
				}
				else
				{
					return;
				}
			}
		}

		if (raw_data.len == 0)
		{
			qDebug() << "Multipack message length error !";
		}

		arr->allocf(raw_data);
		bad_map->allocf(bad_data);

		raw_data.destroy();
		bad_data.destroy();
	}	
}

QString MsgInfo::SmartArrToHexString(SmartArr &sarr)
{
	if (sarr.len == 0) return "";

	QString out = "";
	uint16_t len = sarr.len;
	//uint8_t *arr = sarr.data;
	uint8_t *arr = sarr.data.get();
	for (int i = 0; i < len; i++)
	{
		QString num = QString::number(arr[i], 16).toUpper();
		if (num.size() < 2) num = "0" + num;
		if (i != len-1) num += " ";
		out += num;
	}

	return out;
}


MsgProcessor::MsgProcessor(QMutex *_mutex)
{
	com_msg_mutex = _mutex;

	//msg_timer = new MsgTimer();
	//connect(msg_timer, SIGNAL(timeout()), this, SLOT(elapsedMsgTimer()));	
}

/*
void MsgProcessor::run()
{
	qDebug() << "MsgProcessor->run()-> ID = " << QThread::currentThreadId();

	exec();
}
*/

void MsgProcessor::showAllAsText(MsgInfo *msg_info)
{	
	if (msg_info == NULL) return;

	QStringList msgText;
	MsgInfo::extractAllAsText(msgText, msg_info->getCOMMessage());

	QString in_str = "";
	QString out_str = "";

	COM_Message *msg = msg_info->getCOMMessage();
	uint8_t writer = msg->getWriter();
	QString writer_str = "";
	QStringList colors_str;
	switch (writer)
	{
	case PC_MAIN: 
		{
			writer_str = "PC_Main ";
			colors_str << "magenta" << "blueViolet" << "darkMagenta";
			break;
		}
	case NMR_TOOL: 
		{
			writer_str = "NMR_Tool ";
			colors_str << "blue" << "royalBlue" << "mediumBlue";
			break;
		}
	default: 
		{
			writer_str = " ";
			colors_str << "black" << "black";
		}
	}

	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_str = "&lt;" + ctime.toString("d.M.yyyy hh:mm:ss.zzz") + "&gt;";

	QString id_str = QString("[ id=%1] : ").arg(msg_info->getCOMMessage()->getMsgId());

	QString header_str = "";
	if (msgText.size() > 0) header_str = QString("<font color=%1><b>").arg(colors_str[1]) + msgText[0]/*->at(0)*/ + "</b></font> ";

	QString packs_str = QString("<font color=%1>").arg(colors_str[2]);
	int packs = msg_info->getPacketCount();
	for (int i = 0; i < packs; i++)
	{
		QString pck_str = msgText[i+1]; //msgText->at(i+1);
		if (pck_str.length() >= 8) 
		{
			packs_str += QString("<br><font color=%1><b>").arg(colors_str[1]);
			packs_str += pck_str.mid(0,8) + "</b></font>" + pck_str.mid(8);			
		}
	}
	packs_str += "</font>";

	out_str += QString("<font color=%1><b>").arg(colors_str[0]) + writer_str + ctime_str + id_str + "</b>" + header_str + packs_str + "</font><br><br>";

	emit message_str(out_str); 
}

void MsgProcessor::showAllAsText(COM_Message *msg)
{	
	if (msg == NULL) return;
	QStringList msgText;
	MsgInfo::extractAllAsText(msgText, msg);

	QString in_str = "";
	QString out_str = "";

	uint8_t writer = msg->getWriter();
	QString writer_str = "";
	QStringList colors_str;
	switch (writer)
	{
	case PC_MAIN: 
		{
			writer_str = "PC_Main ";
			colors_str << "magenta" << "blueViolet" << "darkMagenta";
			break;
		}
	case NMR_TOOL: 
		{
			writer_str = "NMR_Tool ";
			colors_str << "blue" << "royalBlue" << "mediumBlue";
			break;
		}
	default: 
		{
			writer_str = " ";
			colors_str << "black" << "black";
		}
	}

	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_str = "&lt;" + ctime.toString("d.M.yyyy hh:mm:ss.zzz") + "&gt;";

	QString id_str = QString("[ id=%1] : ").arg(msg->getMsgId());

	QString header_str = "";
	QString packs_str = QString("<font color=%1>").arg(colors_str[2]);

	if (msg->getIOStatus() == COM_Message::HEADER_SENT)
	{
		if (msgText.size() > 0) header_str = QString("<font color=%1><b>").arg(colors_str[1]) + msgText[0]/*->at(0)*/ + "</b></font> ";
	}
	else  if (msg->getIOStatus() == COM_Message::SENT && msg->getMsgType() == MTYPE_MULTYPACK)
	{
		int packs = msg->getPackets().count();
		for (int i = 0; i < packs; i++)
		{
			QString pck_str = msgText[i+1]; 
			if (pck_str.length() >= 8) 
			{
				packs_str += QString("<br><font color=%1><b>").arg(colors_str[1]);
				packs_str += pck_str.mid(0,8) + "</b></font>" + pck_str.mid(8);			
			}
		}
	}
	else
	{
		if (msgText.size() > 0) header_str = QString("<font color=%1><b>").arg(colors_str[1]) + msgText[0]/*->at(0)*/ + "</b></font> ";

		//QString packs_str = QString("<font color=%1>").arg(colors_str[2]);
		int packs = msg->getPackets().count();
		for (int i = 0; i < packs; i++)
		{
			QString pck_str = msgText[i+1]; 
			if (pck_str.length() >= 8) 
			{
				packs_str += QString("<br><font color=%1><b>").arg(colors_str[1]);
				packs_str += pck_str.mid(0,8) + "</b></font>" + pck_str.mid(8);			
			}
		}
	}	
	packs_str += "</font>";

	out_str += QString("<font color=%1><b>").arg(colors_str[0]) + writer_str + ctime_str + id_str + "</b>" + header_str + packs_str + "</font><br><br>";

	emit message_str(out_str); 
}

void MsgProcessor::showBadMessageAsText(COM_Message *msg, QString &out_str)
{
	if (msg == NULL) return;
	QStringList msgText;
	MsgInfo::extractAllAsText(msgText, msg);

	QString in_str = "";
	//QString out_str = "";
		
	QString writer_str = "NMR_Tool ";
	QStringList colors_str;
	colors_str << "darkRed" << "red" << "red";

	QDateTime ctime = QDateTime::currentDateTime();
	QString ctime_str = "&lt;" + ctime.toString("d.M.yyyy hh:mm:ss.zzz") + "&gt;";

	QString id_str = QString("[ id=%1] : ").arg(msg->getMsgId());

	QString header_str = "";
	QString packs_str = QString("<font color=%1>").arg(colors_str[2]);

	if (msg->getIOStatus() == COM_Message::HEADER_SENT)
	{
		if (msgText.size() > 0) header_str = QString("<font color=%1><b>").arg(colors_str[1]) + msgText[0] + "</b></font> ";
	}
	else  if (msg->getIOStatus() == COM_Message::SENT && msg->getMsgType() == MTYPE_MULTYPACK)
	{
		int packs = msg->getPackets().count();
		for (int i = 0; i < packs; i++)
		{
			QString pck_str = msgText[i+1]; //->at(i+1);
			if (pck_str.length() >= 8) 
			{
				packs_str += QString("<br><font color=%1><b>").arg(colors_str[1]);
				packs_str += pck_str.mid(0,8) + "</b></font>" + pck_str.mid(8);			
			}
		}
	}
	else
	{
		if (msgText.size() > 0) header_str = QString("<font color=%1><b>").arg(colors_str[1]) + msgText[0]/*->at(0)*/ + "</b></font> ";

		//QString packs_str = QString("<font color=%1>").arg(colors_str[2]);
		int packs = msg->getPackets().count();
		for (int i = 0; i < packs; i++)
		{
			QString pck_str = msgText[i+1]; //->at(i+1);
			if (pck_str.length() >= 8) 
			{
				packs_str += QString("<br><font color=%1><b>").arg(colors_str[1]);
				packs_str += pck_str.mid(0,8) + "</b></font>" + pck_str.mid(8);			
			}
		}
	}	
	packs_str += "</font>";

	out_str += QString("<font color=%1><b>").arg(colors_str[0]) + writer_str + ctime_str + id_str + "</b>" + header_str + packs_str + "</font><br><br>";

	//emit message_str(out_str); 
}

void MsgProcessor::receiveBadMsgFromCOMCommander(/*COM_Message *_msg*/ QString out_str)
{
	//lockMsg();	

	//qDebug() << "msg id: " << _msg->getSessionId() << " bad data: " << _msg->getIOStatus();
		
	//QString out_str = "";
	//showBadMessageAsText(_msg, out_str);

	emit message_str(out_str); 

	//unlockMsg();	

}


void MsgProcessor::receiveMsgFromCOMComander(COM_Message *_msg, uint32_t _uid)
{
	lockMsg();
	showAllAsText(_msg);
	uint8_t wr = _msg->getWriter();
	COM_Message::IOStatus io_status = _msg->getIOStatus();
	//uint8_t session_id = _msg->getMsgHeader()->getSessionId();
	//unlockMsg();

	// обработка данных для отображения на графике, записи в файл и т.д.
	if (wr == NMR_TOOL && io_status == COM_Message::COMPLETED)
	{
		//uint8_t session_id = _msg->getMsgHeader()->getSessionId();
		//lockMsg();
		MsgInfo *msg_info = new MsgInfo(_msg);
		MsgInfo::ParsingResult parse_result = extractData(msg_info);
		msg_info->setParsingResult(parse_result);
		//unlockMsg();
		
		if (parse_result == MsgInfo::Message_OK)
		{
			DeviceData *device_data = msg_info->getDeviceData();
			uint8_t cmd = device_data->comm_id;
			switch (cmd)
			{
			case NMRTOOL_DATA:
				{
					emit new_data(device_data);
					break;
				}
			case NMRTOOL_CONNECT:
			case NMRTOOL_CONNECT_DEF:
				{
					qDebug("NMR Tool connected");

					SmartArr arr = _msg->getMsgHeader()->getShortData();
					unsigned char device_id = (arr.data[1]);
					
					emit apply_tool_id(device_id);
					
					treatIncommingData(_uid);
					break;
				}
			/*case LOG_TOOL_SETTINGS:
				{
					SmartArr arr = _msg->getMsgHeader()->getShortData();
					unsigned char device_id = (arr.data[1]);
					//treatIncommingData(_uid);
					//unlockMsg();

					emit apply_tool_id(device_id);

					treatIncommingData(_uid);
					break;
				}*/
			case BAD_DATA:
				{
					SmartArr arr = _msg->getMsgHeader()->getShortData();
					int err_code = arr.data[1];
					/* Error codes from Logging Tool:
					MSG_OK = 0,								// многопакетное сообщение было успешно принято и раскодировано
					MSG_NO_PACKS = 1,						// пакеты многопакетного сообщения не поступили
					MSG_DATA_NOT_ALL = 2,					// данные (пакеты) не поступили в полном обхеме для их расшифровки
					MSG_DECODE_ERR = 3,						// фатальная ошибка при раскодировании принятых пакетов
					MSG_BAD_PACKETS = 4,					// пакеты не прошли проверку на crc и т.п.
					MSG_EXTRACT_ERR = 5						// ошибка при извлечении данных из пакетов
					*/
					QString err_str = "";
					switch (err_code)
					{
					case 1: err_str = "Packets were not received!"; break;
					case 2: err_str = "Some packets were lost!"; break;
					case 3: err_str = "Decoding error!"; break;
					case 4: err_str = "Bad CRC!"; break;
					case 5: err_str = "Damaged data was extracted!"; break;
					}
					QDateTime ctime = QDateTime::currentDateTime();
					QString ctime_str = "&lt;" + ctime.toString("hh:mm:ss") + "&gt;: ";
					QString err_msg = QString(tr("Logging Tool cannot receive and process data. Error message: %1 ").arg(err_str));
					err_msg = "<font color=red>" + ctime_str + err_msg + "</font>"; 

					emit message_str_to_Tool_console(err_msg);
					break;
				}
			case DATA_OK:
			case DIEL_DATA_READY:
				{
					treatIncommingData(_uid);
					break;
				}
			case NMRTOOL_IS_READY:
				{
					// Биты состояния прибора в 5-ом байте служебного сообщения (BYTE1 - см. описание в message_class.h) о готовности прибора к сеансу связи:
					// бит 0 - напряжение питания: 							0/1 - понижено / нормальное
					// бит 1 - готовность телеметрии: 						0/1 - не готова / готова
					// бит 2 - состояние программатора ПЛИС: 				0/1 - запущен командой proger_start() / остановлен командой proger_stop()
					// бит 3 - состояние программы в программаторе ПЛИС: 	0/1 - исполняется / завершилась					
					SmartArr arr = _msg->getMsgHeader()->getShortData();
					unsigned char pow_status = (arr.data[1] & 0x1);
					unsigned char telemetry_on = (arr.data[1] & 0x2);		// see COMCommander::executeServiceMsg
					unsigned char proger_started = (arr.data[1] & 0x4);		// интервальный программатор ПЛИС был запущен командой proger_start()
					unsigned char seq_finished = (arr.data[1] & 0x8);		// закончилась или нет последовательность в программаторе ПЛИС командой COM_STOP
					
					emit power_status(pow_status);
					//emit fpga_seq_status(seq_finished);					// replaced 5.07.2018 with line below					
					emit fpga_seq_status(arr.data[1]);
					//qDebug() << QString("tool status = %1; proger_started = %2; seq_finished = %3").arg(arr.data[1]).arg(proger_started).arg(seq_finished);
					
					break;
				}
			default: 
				{
					break;
				}
			}
		}	
		else 
		{
			int tt = 0;
			qDebug() << "Cannot parse COM_message!";
		}
	}
	else if (wr == PC_MAIN && (io_status == COM_Message::COMPLETED || io_status == COM_Message::SENT))
	{
		//MsgInfo *msg_info = new MsgInfo(msg);		
		//emit message_info(msg_info);	
		//qDebug() << "111";
	}	
		
	unlockMsg();	
}

void MsgProcessor::sendMsg(DeviceData *device_data)
{
	waiting_list.push_back(device_data);
	QString sender_name = ((QObject*)sender())->objectName();
	waiting_obj_list.push_back(sender_name);
	
	emit execute_cmd(device_data);
}

void MsgProcessor::sendMsg(DeviceData *device_data, const QString &obj_name)
{
	waiting_list.push_back(device_data);	
	waiting_obj_list.push_back(obj_name);

	emit execute_cmd(device_data);
}


void MsgProcessor::treatIncommingData(uint32_t _uid)
{	
	for (int i = waiting_list.count()-1; i >= 0; --i)
	{
		DeviceData *dev_data = waiting_list.at(i);
		if (dev_data->uid == _uid) 
		{
			QString obj_name = waiting_obj_list.takeAt(i);

			dev_data = waiting_list.takeAt(i);
			//delete dev_data;

			//QString report = tr("Response to the request was received!");
			QVariantList vlist; vlist << true;
			emit show_cmd_result(_uid, obj_name, vlist);

			return;
		}
	}
}

void MsgProcessor::reportNoResponse(uint32_t _uid)
{
	for (int i = waiting_list.count()-1; i >= 0; --i)
	{
		DeviceData *dev_data = waiting_list.at(i);
		if (dev_data->uid == _uid) 
		{
			QString obj_name = waiting_obj_list.takeAt(i);
			
			dev_data = waiting_list.takeAt(i);
						
			QVariantList vlist; vlist << false; // << report;
			emit show_cmd_result(_uid, obj_name, vlist);

			return;
		}
	}
}


QString MsgProcessor::SmartArrToHexString(SmartArr &sarr)
{
	if (sarr.len == 0) return "";

	QString out = "";
	uint16_t len = sarr.len;
	//uint8_t *arr = sarr.data;
	uint8_t *arr = sarr.data.get();
	for (int i = 0; i < len; i++)
	{
		QString num = QString::number(arr[i], 16).toUpper();
		if (num.size() < 2) num = "0" + num;
		if (i != len-1) num += " ";
		out += num;
	}

	return out;
}


MsgInfo::ParsingResult MsgProcessor::extractData(MsgInfo *msg_info)
{	
	SmartArr data;
	SmartArr bad_map;
	MsgInfo::extractAllAsBytes(&data, &bad_map, msg_info->getCOMMessage()); 	

	//uint8_t temp_arr[160];
	//memset(&temp_arr[0], 0x00, 160*sizeof(uint8_t));
	//memcpy(&temp_arr[0], &data.data[0], data.len*sizeof(uint8_t));

	int start_pos = 0;
	MTYPE msg_type = msg_info->getCOMMessage()->getMsgType();
	if (msg_type == MTYPE_MULTYPACK) 
	{		
		start_pos = PACK_INFO_LEN;
		if (data.len == 0) return MsgInfo::Unknown_Error;										// некорректно отработала функция extractAllAsBytes(...)
		if (bad_map.len == 0) return MsgInfo::Unknown_Error;									// некорректно отработала функция extractAllAsBytes(...)
	}

	if (data.data == 0) return MsgInfo::Unknown_Error;											// некорректно отработала функция extractAllAsBytes(...)

	if (data.len < start_pos+1 || bad_map.len < start_pos+1) return MsgInfo::Data_NotFound;		// при изъятии данных из принятого сообщения самих данных не обнаружено

	while (start_pos < data.len)
	{
		if (bad_map.data[start_pos] == (uint8_t)DATA_STATE_FAIL) 
		{
			data.destroy();
			bad_map.destroy();
			return MsgInfo::Bad_Command;				// невозможно прочитать команду
		}


		uint8_t cmd = data.data[start_pos];
		switch (cmd)
		{
		case NMRTOOL_IS_READY:
			{
				msg_info->setDeviceDataMemo("NMR Tool is Ready", NMRTOOL_IS_READY, MTYPE_SERVICE);
				start_pos += SRV_DATA_LEN;
				break;
			}
		case NMRTOOL_ISNOT_READY:
			{
				msg_info->setDeviceDataMemo("NMR Tool isn't Ready", NMRTOOL_ISNOT_READY, MTYPE_SERVICE);
				start_pos += SRV_DATA_LEN;
				break;
			}	
		case HEADER_OK:
			{
				msg_info->setDeviceDataMemo("Header is OK", HEADER_OK, MTYPE_SHORT);
				start_pos += SHORT_DATA_LEN;
				break;
			}
		case DT_SGN_SE_ORG:	
		case DT_NS_SE_ORG:
		case DT_SGN_FID_ORG:
		case DT_NS_FID_ORG:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных

				uint16_t byte_counter = cmd_data_len;								
				uint16_t nmr_data_len = cmd_data_len/sizeof(uint8_t);					
				QVector<double> *org_data = new QVector<double>(nmr_data_len);
				QVector<uint8_t> *bad_org_data = new QVector<uint8_t>(nmr_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;			

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				int16_t _b = 0;
				while (byte_counter > 0)
				{					
					_b = (int16_t)(A*data.data[pos] - B);
					org_data->data()[cnt] = (double)_b;

					bad_org_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_org_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);
					pos += sizeof(uint8_t);	
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "ADC Data";
				field->code = cmd;
				field->value = org_data;
				field->str_value = "";
				field->value_type = Field_Comm::UINT16;
				field->bad_data = bad_org_data;
				field->tag = (int)group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("ADC Data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_SGN_SE:
		case DT_NS_SE:		
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t nmr_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *se_data = new QVector<double>(nmr_data_len);
				QVector<uint8_t> *bad_se_data = new QVector<uint8_t>(nmr_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;	

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					_b = A*data.data[pos] - B;
					se_data->data()[cnt] = (double)_b;

					bad_se_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_se_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "Preprocessed ADC Data";
				field->code = cmd;
				field->value = se_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_se_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Preprocessed ADC Data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}
				break;
			}
		case DT_RFP:
		case DT_RFP2:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t nmr_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *se_data = new QVector<double>(nmr_data_len);
				QVector<uint8_t> *bad_se_data = new QVector<uint8_t>(nmr_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;	

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					_b = A*data.data[pos] - B;
					se_data->data()[cnt] = (double)_b;

					bad_se_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_se_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "RF-Pulse Data";
				field->code = cmd;
				field->value = se_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_se_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("RF-Pulse Data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}
				break;
			}
		case DT_NS_QUAD_FID_RE:
		case DT_NS_QUAD_FID_IM:
		case DT_NS_QUAD_SE_RE:
		case DT_NS_QUAD_SE_IM:
		case DT_SGN_QUAD_FID_RE:
		case DT_SGN_QUAD_FID_IM:
		case DT_SGN_QUAD_SE_RE:
		case DT_SGN_QUAD_SE_IM:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t qd_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *qd_data = new QVector<double>(qd_data_len);
				QVector<uint8_t> *bad_qd_data = new QVector<uint8_t>(qd_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					_b = A*data.data[pos] - B;
					qd_data->data()[cnt] = (double)_b;

					bad_qd_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_qd_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "ADC Quadrature";
				field->code = cmd;
				field->value = qd_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_qd_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("ADC Quadrature", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

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
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t fft_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *fft_data = new QVector<double>(fft_data_len);
				QVector<uint8_t> *bad_fft_data = new QVector<uint8_t>(fft_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					_b = A*data.data[pos] - B;
					fft_data->data()[cnt] = (double)_b;

					bad_fft_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_fft_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "FFT Spectrum";
				field->code = cmd;
				field->value = fft_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_fft_data;
				field->tag = (int)group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("FFT Spectrum", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_SGN_FFT_FID_AM:
		case DT_NS_FFT_FID_AM:
		case DT_SGN_FFT_SE_AM:
		case DT_NS_FFT_SE_AM:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t fft_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *fft_data = new QVector<double>(fft_data_len);
				QVector<uint8_t> *bad_fft_data = new QVector<uint8_t>(fft_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					_b = A*data.data[pos] - B;
					fft_data->data()[cnt] = (double)_b;

					bad_fft_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_fft_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "FFT Amplitude Spectrum";
				field->code = cmd;
				field->value = fft_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_fft_data;
				field->tag = (int)group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("FFT Amplitude Spectrum", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_SGN_POWER_SE:
		case DT_SGN_POWER_FID:
		case DT_NS_POWER_SE:
		case DT_NS_POWER_FID:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t ps_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *ps_data = new QVector<double>(ps_data_len);
				QVector<uint8_t> *bad_ps_data = new QVector<uint8_t>(ps_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					_b = A*data.data[pos] - B;
					ps_data->data()[cnt] = (double)_b;

					bad_ps_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_ps_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "Power Spectrum";
				field->code = cmd;
				field->value = ps_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_ps_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Power Spectrum", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_SGN_RELAX:
		case DT_SGN_RELAX2:
		case DT_SGN_RELAX3:		
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t relax_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *relax_data = new QVector<double>(relax_data_len);
				QVector<uint8_t> *bad_relax_data = new QVector<uint8_t>(relax_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);

				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					if (data.data[pos] == 0xff) 
					{
						uint32_t nan_32 = 0xffffffff;
						float *nan = (float*)&nan_32;
						relax_data->data()[cnt] = *nan;
					}
					else
					{
						_b = A*data.data[pos] - B;
						relax_data->data()[cnt] = (double)_b;
					}						

					bad_relax_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_relax_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "CPMG Relax data";
				field->code = cmd;
				field->value = relax_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_relax_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("CPMG Relax data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}		
		case DT_T1T2_NMR:
		case DT_DsT2_NMR:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
					if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t relax_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *relax_data = new QVector<double>(relax_data_len);
				QVector<uint8_t> *bad_relax_data = new QVector<uint8_t>(relax_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);

				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					if (data.data[pos] == 0xff) 
					{
						uint32_t nan_32 = 0xffffffff;
						float *nan = (float*)&nan_32;
						relax_data->data()[cnt] = *nan;
					}
					else
					{
						_b = A*data.data[pos] - B;
						relax_data->data()[cnt] = (double)_b;
					}						

					bad_relax_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_relax_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "T1T2 2D-NMR data";
				field->code = cmd;
				field->value = relax_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_relax_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("T1T2 2D-NMR data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_SOLID_ECHO:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
					if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t solidecho_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *solidecho_data = new QVector<double>(solidecho_data_len);
				QVector<uint8_t> *bad_solidecho_data = new QVector<uint8_t>(solidecho_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);

				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					if (data.data[pos] == 0xff) 
					{
						uint32_t nan_32 = 0xffffffff;
						float *nan = (float*)&nan_32;
						solidecho_data->data()[cnt] = *nan;
					}
					else
					{
						_b = A*data.data[pos] - B;
						solidecho_data->data()[cnt] = (double)_b;
					}						

					bad_solidecho_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_solidecho_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "Solid Echo data";
				field->code = cmd;
				field->value = solidecho_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_solidecho_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Solid Echo data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}
				break;
			}
		case DT_AFR1_RX:
		case DT_AFR2_RX:
		case DT_AFR3_RX:
		case DT_AFR1_TX:
		case DT_AFR2_TX:
		case DT_AFR3_TX:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t afr1_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *afr1_data = new QVector<double>(afr1_data_len);
				QVector<uint8_t> *bad_afr1_data = new QVector<uint8_t>(afr1_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);

				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					if (data.data[pos] == 0xff) 
					{
						uint32_t nan_32 = 0xffffffff;
						float *nan = (float*)&nan_32;
						afr1_data->data()[cnt] = *nan;
					}
					else
					{
						_b = A*data.data[pos] - B;
						afr1_data->data()[cnt] = (double)_b;
					}						

					bad_afr1_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_afr1_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "Frequency Response data";
				field->code = cmd;
				field->value = afr1_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_afr1_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Frequency Response data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_FREQ_TUNE:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
					if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t freq_autotune_data_len = cmd_data_len/sizeof(uint8_t);
				QVector<double> *freq_autotune_data = new QVector<double>(freq_autotune_data_len);
				QVector<uint8_t> *bad_freq_autotune_data = new QVector<uint8_t>(freq_autotune_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint32_t a32 = 0;
				uint32_t b32 = 0;
				memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); pos += sizeof(float);
				memcpy((uint8_t*)&b32, &data.data[pos], sizeof(float)); pos += sizeof(float);

				float A = *(float*)&a32;
				float B = *(float*)&b32;
				start_pos += 2*sizeof(float);

				float _b = 0;
				while (byte_counter > 0)
				{					
					if (data.data[pos] == 0xff) 
					{
						uint32_t nan_32 = 0xffffffff;
						float *nan = (float*)&nan_32;
						freq_autotune_data->data()[cnt] = *nan;
					}
					else
					{
						_b = A*data.data[pos] - B;
						freq_autotune_data->data()[cnt] = (double)_b;
					}						

					bad_freq_autotune_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(uint8_t);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_freq_autotune_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(uint8_t);	
					pos += sizeof(uint8_t);
					start_pos += sizeof(uint8_t);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "Frequency Autotune";
				field->code = cmd;
				field->value = freq_autotune_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_freq_autotune_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Frequency Autotune", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_GAMMA:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;								
				uint16_t gamma_data_len = cmd_data_len/sizeof(float);
				QVector<double> *gamma_data = new QVector<double>(gamma_data_len);				
				QVector<uint8_t> *bad_gamma_data = new QVector<uint8_t>(gamma_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				float _b = 0;
				while (byte_counter > 0)
				{					
					uint32_t a32 = 0;					
					memcpy((uint8_t*)&a32, &data.data[pos], sizeof(float)); 

					_b = *(float*)&a32;
					gamma_data->data()[cnt] = _b;

					bad_gamma_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(float);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_gamma_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;

					byte_counter -= sizeof(float);	
					pos += sizeof(float);
					start_pos += sizeof(float);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "Gamma data";
				field->code = cmd;
				field->value = gamma_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_gamma_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Gamma data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_DIEL:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;
				uint16_t diel_data_len = 8;
				QVector<double> *diel_data = new QVector<double>(diel_data_len);
				QVector<uint8_t> *bad_diel_data = new QVector<uint8_t>(diel_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				QVector<uint8_t> temp(DIELECTR_MSG_LEN);				
				memcpy(&temp[0], &data.data[pos], DIELECTR_MSG_LEN*sizeof(uint8_t)); 
				uint16_t head = ((temp[1] << 8) | temp[0]);				

				uint16_t csumm = 0;
				for (int i = 2; i < DIELECTR_MSG_LEN - 2; i++)
				{
					csumm += temp[i];
				}
				uint16_t crc16 = ((temp[DIELECTR_MSG_LEN - 1] << 8) | temp[DIELECTR_MSG_LEN - 2]);

				if (head ==  0xFAFA /*&& const1 == 0x64 && const2 == 0x3E8*/ && csumm == crc16)
				{
					pos += 2;
					int cnt = 0;
					bool is_data_bad = false;
					for (int i = 2; i < DIELECTR_MSG_LEN-2; i += 2)
					{
						uint16_t val = ((temp[i+1] << 8) | temp[i]);
						diel_data->data()[cnt] = (double)val;

						bad_diel_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
						int pp = sizeof(uint16_t);						
						while (--pp >= 0) 
						{
							if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) 
							{
								bad_diel_data->data()[cnt] = (uint8_t)BAD_DATA;
								is_data_bad = true;
							}
						}
						cnt++;
						pos += 2;
					}

					if (!is_data_bad) emit send_to_cdiag(temp); 
				}
				else
				{
					for (int i = 0; i < diel_data_len; i++)
					{
						bad_diel_data->data()[i] = (uint8_t)BAD_DATA;
					}
				}

				start_pos += DIELECTR_MSG_LEN;

				Field_Comm *field = new Field_Comm;
				field->name = "SDSP data";
				field->code = cmd;
				field->value = diel_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_diel_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("SDSP data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case DT_DIEL_ADJUST:
			{
				qDebug() << "extract DT_DIEL_ADJUST";
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;		
				uint16_t diel_data_len = cmd_data_len/sizeof(float);
				QVector<double> *diel_data = new QVector<double>(diel_data_len);				
				QVector<uint8_t> *bad_diel_data = new QVector<uint8_t>(diel_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				float *temp = (float*)(data.data.get() + pos);
				for (int i = 0; i < diel_data_len; i++)
				{
					uint16_t val = temp[i];
					diel_data->data()[i] = (double)val;

					bad_diel_data->data()[i] = (uint8_t)DATA_STATE_OK;
					int pp = sizeof(float);
					while (--pp >= 0) 
					{
						if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_diel_data->data()[cnt] = (uint8_t)BAD_DATA;
					}
					cnt++;
					pos += sizeof(float);
					start_pos += sizeof(float);
				}

				Field_Comm *field = new Field_Comm;
				field->name = "SDSP adjustment data";
				field->code = cmd;
				field->value = diel_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_diel_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("SDSP adjustment data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}
				break;
			}
		case DT_DU:		
		case DT_PU:
		case DT_TU:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;	// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL) // ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);	// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;	
				uint16_t dt_data_len = 2;
				QVector<double> *dt_data = new QVector<double>(dt_data_len);				
				QVector<uint8_t> *bad_dt_data = new QVector<uint8_t>(dt_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				uint8_t temp[DU_DATA_LEN];				
				memcpy(&temp[0], &data.data[pos], DU_DATA_LEN*sizeof(uint8_t)); 

				int bias = 0;

				// Measured temperatures
				int Clk = calcClk((int)temp[bias++]);
				double Rc = 10000;				
				int Nc = (temp[bias] << 8) | (temp[bias+1]);
				double t = Clk*Nc/8.0e6;
				double C = t/Rc/log(2);
				bias += 2;

				Clk = calcClk((int)temp[bias++]);
				Nc = (temp[bias] << 8) | (temp[bias+1]);
				t = Clk*Nc/8.0e6;
				double R1 = PlusInf;
				if (C != 0) R1 = t/C/log(2) - 10000;
				bias += 2;

				dt_data->data()[cnt] = R1;
				bad_dt_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
				int pp = 9;
				while (--pp >= 0) 
				{
					if (pp > 5) pp = 5;
					if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_dt_data->data()[cnt] = (uint8_t)BAD_DATA;
				}
				cnt++;

				Clk = calcClk((int)temp[bias++]);
				Nc = (temp[bias] << 8) | (temp[bias+1]);
				t = Clk*Nc/8.0e6;
				double R2 = PlusInf;
				if (C != 0) R2 = t/C/log(2) - 10000;
				bias += 2;

				dt_data->data()[cnt] = R2;
				bad_dt_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
				pp = 9;
				while (--pp >= 0) 
				{
					if (pp == 5) pp -= 3;
					if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_dt_data->data()[cnt] = (uint8_t)BAD_DATA;
				}
				cnt++;

				start_pos += cmd_data_len;

				Field_Comm *fieldT = new Field_Comm;				
				switch (cmd)
				{
				case DT_DU: 
					fieldT->code = DT_DU_T; 
					fieldT->name = "DU Temperature data"; 
					msg_info->setDeviceDataMemo("DU Monitoring data", NMRTOOL_DATA, MTYPE_MULTYPACK);
					break;
				case DT_PU: 
					fieldT->code = DT_PU_T; 
					fieldT->name = "PU Temperature data"; 
					msg_info->setDeviceDataMemo("PU Monitoring data", NMRTOOL_DATA, MTYPE_MULTYPACK);
					break;				
				case DT_TU: 
					fieldT->code = DT_TU_T; 
					fieldT->name = "TU Temperature data"; 
					msg_info->setDeviceDataMemo("TU Monitoring data", NMRTOOL_DATA, MTYPE_MULTYPACK);
					break;
				}	
				fieldT->value = dt_data;
				fieldT->str_value = "";
				fieldT->value_type = Field_Comm::FLOAT;
				fieldT->bad_data = bad_dt_data;
				fieldT->tag = group_index;
				fieldT->channel = channel_data_id;
				msg_info->getDeviceData()->fields->append(fieldT);							

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
			case DT_T:
				{
					if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;				// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
					if (bad_map.data[start_pos+1] == DATA_STATE_FAIL)								// ошибка в коде канала данных () !
						if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command;	// ошибка в групповом индексе данных команды !
					if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

					uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);				// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
					uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
					uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
					if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
					start_pos += 5;																	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


					//uint16_t byte_counter = cmd_data_len;	
					//uint16_t dt_data_len = cmd_data_len/2;
										
					uint16_t byte_counter = cmd_data_len;
					uint16_t dt_data_len = byte_counter/1;		// данные с температурных датчиков поступают в виде: "номер датчика (1 байт)" - "температура (1 байт)" - ... - (т.е. и т.д.)
					QVector<double> *dt_data = new QVector<double>(dt_data_len);		// укладываются данные температуры так же: "номер датчика (1 байт)" - "температура (1 байт)" - ... - (т.е. и т.д.)		
					QVector<uint8_t> *bad_dt_data = new QVector<uint8_t>(dt_data_len);

					uint16_t pos = start_pos;
					uint32_t cnt = 0;
					uint8_t temp[16];																// 8x2 bytes of data
					memcpy(&temp[0], &data.data[pos], 16*sizeof(uint8_t)); 
					for (int i = 0; i < byte_counter; i += 2)
					{
						int chNum = (uint)temp[i];
						dt_data->data()[cnt] = (double)chNum; 
						bad_dt_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
						int TT = (uint)temp[i+1];
						dt_data->data()[cnt+1] = (double)TT; 
						bad_dt_data->data()[cnt+1] = (uint8_t)DATA_STATE_OK;

						int pp = sizeof(uint16_t);
						while (--pp >= 0) 
						{
							if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_dt_data->data()[cnt] = (uint8_t)BAD_DATA;
						}
						cnt += 2;
						pos += sizeof(uint16_t);
						start_pos += sizeof(uint16_t);
					}

					//start_pos += byte_counter;

					Field_Comm *fieldT = new Field_Comm;				
					fieldT->code = DT_T; 
					fieldT->name = "SPVP Temperature data"; 
					msg_info->setDeviceDataMemo("SPVP Monitoring data", NMRTOOL_DATA, MTYPE_MULTYPACK);
					fieldT->value = dt_data;
					fieldT->str_value = "";
					fieldT->value_type = Field_Comm::FLOAT;
					fieldT->bad_data = bad_dt_data;
					fieldT->tag = group_index;
					fieldT->channel = channel_data_id;
					msg_info->getDeviceData()->fields->append(fieldT);

					if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
					else 
					{
						return MsgInfo::Message_OK;
					}

					break;
				}
			case DT_U:
				{
					if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;				// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
					if (bad_map.data[start_pos+1] == DATA_STATE_FAIL)								// ошибка в коде канала данных () !
						if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command;	// ошибка в групповом индексе данных команды !
					if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

					uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);				// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
					uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
					uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
					if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
					start_pos += 5;																	// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


					uint16_t byte_counter = cmd_data_len;	
					uint16_t du_data_len = cmd_data_len/2;
					//uint16_t byte_counter = 8;	
					//uint16_t du_data_len = byte_counter/1;
					QVector<double> *du_data = new QVector<double>(du_data_len);				
					QVector<uint8_t> *bad_du_data = new QVector<uint8_t>(du_data_len);

					uint16_t pos = start_pos;
					uint32_t cnt = 0;
					uint8_t temp[8];																// 4x2 bytes of data
					memcpy(&temp[0], &data.data[pos], 8*sizeof(uint8_t)); 					
					for (int i = 0; i < byte_counter; i += 2)
					{
						int UU = ((uint)temp[i] << 8) | (uint)temp[i+1];
						du_data->data()[cnt] = 3.3*UU/1024.0; 
						bad_du_data->data()[cnt] = (uint8_t)DATA_STATE_OK;

						int pp = sizeof(uint16_t);
						while (--pp >= 0) 
						{
							if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) bad_du_data->data()[cnt] = (uint8_t)BAD_DATA;
						}
						cnt++;
						pos += sizeof(uint16_t);
						start_pos += sizeof(uint16_t);
					}

					//start_pos += byte_counter;

					Field_Comm *fieldU = new Field_Comm;				
					fieldU->code = DT_U; 
					fieldU->name = "SPVP Voltage data"; 
					msg_info->setDeviceDataMemo("SPVP Monitoring data", NMRTOOL_DATA, MTYPE_MULTYPACK);
					fieldU->value = du_data;
					fieldU->str_value = "";
					fieldU->value_type = Field_Comm::FLOAT;
					fieldU->bad_data = bad_du_data;
					fieldU->tag = group_index;
					fieldU->channel = channel_data_id;
					msg_info->getDeviceData()->fields->append(fieldU);							

					if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
					else 
					{
						return MsgInfo::Message_OK;
					}

					break;
				}
		case DT_PRESS_UNIT:
			{
				if (data.len < PACK_INFO_LEN + 6) return MsgInfo::Bad_DataLength;						// недостаточно данных. Должно быть как минимум: 3 байта (заголовок пакета) + 1байт (команда) + 1 байт (индекс канала данных) + 1 байт (индекс группы данных) + 2 байта (длина данных) + 1 байт (crc)
				if (bad_map.data[start_pos+1] == DATA_STATE_FAIL)										// ошибка в коде канала данных () !
				if (bad_map.data[start_pos+2] == BAD_DATA ) return MsgInfo::Bad_Command;				// ошибка в групповом индексе данных команды !
				if (bad_map.data[start_pos+3] == DATA_STATE_FAIL || bad_map.data[start_pos+4] == BAD_DATA ) return MsgInfo::Bad_Command; // ошибка в байтах длины данных команды !

				uint8_t channel_data_id = (uint8_t)(data.data[start_pos+1] + 1);						// номера каналов данных при передаче в каротажный прибор отсчитываются от нуля
				uint16_t group_index = (uint16_t)(data.data[start_pos+2]);
				uint16_t cmd_data_len = ((uint16_t)(data.data[start_pos+4]) << 8) | (uint16_t)(data.data[start_pos+3]);
				if (cmd_data_len > data.len-PACK_INFO_LEN-6) cmd_data_len = data.len - PACK_INFO_LEN-6;	// если длина данных в пакете меньше, чем указано после байта команды			
				start_pos += 5;																			// = 5, т.к. 5 = 1 байт комманды + 2 байта групповой индекс + 2 байта длины данных


				uint16_t byte_counter = cmd_data_len;
				uint16_t press_unit_data_len = 3;
				QVector<double> *press_unit_data = new QVector<double>(press_unit_data_len);
				QVector<uint8_t> *bad_press_unit_data = new QVector<uint8_t>(press_unit_data_len);

				uint16_t pos = start_pos;
				uint32_t cnt = 0;

				QVector<uint32_t> temp(PRESS_UNIT_LEN);				
				memcpy(&temp[0], &data.data[pos], PRESS_UNIT_LEN*sizeof(uint32_t)); 
								
				{
					pos += 2;
					int cnt = 0;
					bool is_data_bad = false;
					for (int i = 0; i < PRESS_UNIT_LEN; i++)
					{
						uint16_t val = temp[i];	//((temp[i+1] << 8) | temp[i]);
						press_unit_data->data()[cnt] = (double)val;

						bad_press_unit_data->data()[cnt] = (uint8_t)DATA_STATE_OK;
						int pp = sizeof(uint16_t);						
						while (--pp >= 0) 
						{
							if (bad_map.data[pos+pp] == DATA_STATE_FAIL ) 
							{
								bad_press_unit_data->data()[cnt] = (uint8_t)BAD_DATA;
								is_data_bad = true;
							}
						}
						cnt++;
						pos++;
					}
					//if (!is_data_bad) emit send_to_cdiag(temp); 
				}
				
				start_pos += PRESS_UNIT_LEN;

				Field_Comm *field = new Field_Comm;
				field->name = "Pressure Unit data";
				field->code = cmd;
				field->value = press_unit_data;
				field->str_value = "";
				field->value_type = Field_Comm::FLOAT;
				field->bad_data = bad_press_unit_data;
				field->tag = group_index;
				field->channel = channel_data_id;

				msg_info->setDeviceDataMemo("Pressure Unit data", NMRTOOL_DATA, MTYPE_MULTYPACK);
				msg_info->getDeviceData()->fields->append(field);

				if (data.data[start_pos] == 0xFF) start_pos++;		// if next byte is a separator between data arrays
				else 
				{
					return MsgInfo::Message_OK;
				}

				break;
			}
		case NMRTOOL_CONNECT:
			{
				msg_info->setDeviceDataMemo("Connect to Logging Tool", NMRTOOL_CONNECT, MTYPE_SHORT);
				start_pos += SRV_DATA_LEN;
				break;
			}
		case NMRTOOL_CONNECT_DEF:
			{
				msg_info->setDeviceDataMemo("Connect to Logging Tool", NMRTOOL_CONNECT_DEF, MTYPE_SHORT);
				start_pos += SRV_DATA_LEN;
				break;
			}
		case DATA_OK:
			{
				msg_info->setDeviceDataMemo("Data is OK", DATA_OK, MTYPE_SHORT);
				start_pos += SRV_DATA_LEN;
				break;
			}
		case BAD_DATA:
			{
				msg_info->setDeviceDataMemo("Bad Data", BAD_DATA, MTYPE_SHORT);
				start_pos += SRV_DATA_LEN;
				break;
			}
		case DIEL_DATA_READY:
			{
				msg_info->setDeviceDataMemo("SDSP data is ready", DIEL_DATA_READY, MTYPE_SHORT);
				start_pos += SHORT_DATA_LEN;
				break;
			}
		default: 
			{
				uint8_t tt[4096];
				memcpy(&tt[0], data.data.get(), data.len*sizeof(uint8_t));
				data.destroy();
				bad_map.destroy();
				return MsgInfo::Bad_Command;
			}
		}	
	}	

	data.destroy();
	bad_map.destroy();

	return MsgInfo::Message_OK;
}

int MsgProcessor::calcClk(int val)
{
	int Clk = val;
	switch (Clk)
	{
	case 1: Clk = 1; break;
	case 2: Clk = 8; break;
	case 3: Clk = 64; break;
	case 4: Clk = 256; break;
	case 5: Clk = 1024; break;
	}

	return Clk;
}


MsgHeader *MsgProcessor::copyMsgHeader(MsgHeader *_hdr)
{
	MsgHeaderInfo info;
	info.start_marker = _hdr->getStartMarker();
	info.id = _hdr->getSessionId();
	info.w_addr = _hdr->getWriter();
	info.r_addr = _hdr->getReader();
	info.pack_count = _hdr->getPackCount();
	info.pack_len = _hdr->getPackLen();
	info.block_len = _hdr->getBlockLen();
	info.err_count = _hdr->getErrsCount();
	info.crc = _hdr->getCRC();
	SmartArr srv_data = _hdr->getSrvData();	

	MsgHeader *msg_header = new MsgHeader(&info, _hdr->getGFData());
	memcpy(msg_header->getByteArray().data.get(), _hdr->getByteArray().data.get(), HEADER_LEN*sizeof(uint8_t));
	memcpy(msg_header->getRecData().data.get(), _hdr->getRecData().data.get(), SRV_DATA_LEN*sizeof(uint8_t));
	
	return msg_header;
}

MsgPacket *MsgProcessor::copyMsgPacket(MsgPacket* _pack)
{
	MsgPacket *new_pack = new MsgPacket(_pack->getPacketNumber(), _pack->getMsgHeader(), _pack->interleaved());
	if (new_pack->setData(_pack)) return new_pack;
	else
	{
		delete new_pack;
		return 0;
	}

	return new_pack;
}

COM_Message *MsgProcessor::copyCOMMessage(COM_Message *_msg)
{
	MsgHeader *hdr = copyMsgHeader(_msg->getMsgHeader());

	COM_Message *new_msg = new COM_Message(hdr);	
	new_msg->setSessionId(_msg->getSessionId());
	new_msg->setWriter(_msg->getWriter());
	new_msg->setReader(_msg->getReader());
	new_msg->setPriority(_msg->getPriority());
	new_msg->setIOStatus(_msg->getIOStatus());

	for (int i = 0; i < _msg->getPackets().count(); i++)
	{
		MsgPacket *src_pack = _msg->getPackets()[i];		
		MsgPacket *new_pack = copyMsgPacket(src_pack);
		if (new_pack) new_msg->addPacket(new_pack);
		else return 0;
	}	

	new_msg->getParcelList().clear();
	QList<int> parcel_list = _msg->getParcelList();	
	for (int i = 0; i < parcel_list.count(); i++)
	{
		new_msg->addParcel(parcel_list[i]);
	}

	return new_msg;
}
