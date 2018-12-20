#include <QElapsedTimer>
#include <QTimer>
#include <QVector>
#include <QMetaType>
#include <cmath>

#include "threads.h"
#include "message_class.h"
#include "../main_window.h"

#include "../Common/profiler.h"


static int thid = 0;
static int com_cnt = 0;		// временная переменная для отладки

static int byte_counter = 0;
static bool pack_started = false;
static bool pack_stoped = false;


Clocker::Clocker(QObject *parent)
{
    thread_id = thid++;

    ms_period = CLOCK_PERIOD;
    is_running = false;
		
	connect(&timer, SIGNAL(timeout()), this, SIGNAL(clock()));
	timer.start(ms_period);
}

void Clocker::setPeriod(int ms)
{
    if (ms >= 0) ms_period = ms;
	timer.stop();
	timer.start(ms_period);
}

void Clocker::run()
{
    exec();
}

void Clocker::stopThread()
{
    is_running = false;
}


int COMCommander::msg_req_delay = MSG_REQ_DELAY;	// added 5.07.2018

COMCommander::COMCommander(MainWindow *main_win, QObject *parent)
{
	thread_id = thid++;
		
	nmrtool_socket = main_win->getNMRToolSocket();
	connect(nmrtool_socket.socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));

	gf_data = main_win->getGFDataObj();	
	clocker = main_win->getClocker();
	com_msg_mutex = main_win->getCOMMsgMutex();
	this->main_win = main_win;

	initMsgSettings();

	cur_device_data = NULL;
	
	com_msg_list = new QList<COM_Message*>;

	connect(this, SIGNAL(store_COM_message(COM_Message*)), this, SLOT(storeCOMMsg(COM_Message*)));
	connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	
	head_q = new QUEUE<uint8_t>(HEADER_LEN);
	body_q = new QUEUE<uint8_t>(IO_BUFF_SIZE);

	msg_critical = 0;
	msg_incomming = new COM_Message(new MsgHeader(gf_data)); 	
	msg_outcomming = new COM_Message(new MsgHeader(gf_data)); 	//NULL;

	msg_header_state = NOT_DEFINED;
	msg_packet_state = NOT_DEFINED;
	incom_msg_state = NOT_DEFINED;
	pack_counter = 0;
			
	nmrtool_state = false;
	sdsptool_state = false;
	interleaving_out = false;
	on_break = false;

	com_data_ready = false;
	is_running = true;

	incomming_msg_log = new QFile(QApplication::applicationDirPath() + "/incomming_msg_test.txt");

	msg_timer = new QTimer();	
	connect(msg_timer, SIGNAL(timeout()), this, SLOT(lifeTimeElapsed()));

}

COMCommander::~COMCommander()
{
	delete head_q;
	delete body_q;

	int sz = com_msg_list->size();
	for (int i = 0; i < sz; i++)
	{
		COM_Message *msg = com_msg_list->at(i);
		delete msg;	
		msg = NULL;
	}
	com_msg_list->clear();
	delete com_msg_list;
		
	delete msg_timer;	

	delete incomming_msg_log;

	delete msg_incomming;
	delete msg_outcomming;
}


void COMCommander::run()
{	
	is_freezed = false;
	is_running = true;

	exec();
}


void COMCommander::freeze()
{
	disconnect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = true;	
}

void COMCommander::wake()
{
	connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = false;	
}

void COMCommander::initMsgSettings()
{
	QSettings *app_settings = main_win->getAppSettings();
	QString header_delay_str = "MessageSettings/MsgHeaderDelay";
	QString req_delay_str = "MessageSettings/MsgReqDelay";
	QString life_time_str = "MessageSettings/MsgLifeTime";

	bool ok;
	if (app_settings->contains(header_delay_str)) msg_header_delay = app_settings->value(header_delay_str).toInt(&ok); else ok = false;
	if (!ok) msg_header_delay = MSG_HEADER_DELAY;
	if (app_settings->contains(req_delay_str)) msg_req_delay = app_settings->value(req_delay_str).toInt(&ok); else ok = false;
	if (!ok) msg_req_delay = MSG_REQ_DELAY;
	if (app_settings->contains(life_time_str)) msg_life_time = app_settings->value(life_time_str).toInt(&ok); else ok = false;
	if (!ok) msg_life_time = MSG_LIFETIME;

	sdsp_req_delay = SDSP_REQ_DELAY;
}

// Added 5.07.2018 --------------------------
void COMCommander::setMsgReqDelay(int _value)
{
	if (_value > 0) msg_req_delay = _value;
	else 
	{
		bool ok;
		QSettings *app_settings = main_win->getAppSettings();
		if (app_settings->contains("MessageSettings/MsgReqDelay")) msg_req_delay = app_settings->value("MessageSettings/MsgReqDelay").toInt(&ok); else ok = false;
		if (!ok) msg_req_delay = MSG_REQ_DELAY;
	}
}
// ------------------------------------------


void COMCommander::storeCOMMsg(COM_Message* _msg)
{
	com_msg_list->append(_msg);
}


void COMCommander::sendCOMMsg(COM_Message *msg)
{	
	char start_byte = START_BYTE;
	char stop_byte = STOP_BYTE;
	if (msg->getMsgType() == MTYPE::MTYPE_SERVICE || msg->getMsgType() == MTYPE::MTYPE_SHORT)
	{
		SmartArr arr;
		msg->getRawData(&arr);
		Sleep(20);

		nmrtool_socket.socket->write((char*)&start_byte, 1);
		nmrtool_socket.socket->write((char*)arr.data.get(), arr.len);
		nmrtool_socket.socket->write((char*)&stop_byte, 1);

		//msg->setStored(true);
		//emit store_COM_message(msg);		

		arr.destroy();	
	}
	else if (msg->getMsgType() == MTYPE::MTYPE_MULTYPACK)
	{
		if (msg->getIOStatus() == COM_Message::HEADER_SENT)
		{
			SmartArr arr;
			msg->getHeaderRawData(&arr);			
			Sleep(20);

			nmrtool_socket.socket->write((char*)&start_byte, 1);
			nmrtool_socket.socket->write((char*)arr.data.get(), arr.len);
			nmrtool_socket.socket->write((char*)&stop_byte, 1);

			msg->setStored(false);

			arr.destroy();
		}
		if (msg->getIOStatus() == COM_Message::SENT)
		{
			int packet_delay = main_win->getCommSettings()->packet_delay;
			qDebug() << msg->getPackets().count() << msg->getPackets().first()->getPacketLen();
			for (int i = 0; i < msg->getPackets().count(); i++)
			{
				Sleep(20);

				SmartArr arr;
				msg->getPacketRawData(&arr, i);		
				nmrtool_socket.socket->write((char*)(arr.data.get()), arr.len);		

				arr.destroy();
			}		

			//msg->setStored(true);
			//emit store_COM_message(msg);							
		}
	}
}


void COMCommander::treatCOMData(COM_Message *_msg)
{	
	int res = findMsgHeader(head_q, _msg);
	if (res == E_RS_OK)
	{
		res = checkMsgHeader(_msg);
		if (res == E_MSG_OK)
		{					
			msg_header_state = FINISHED;
			incom_msg_state = FINISHED;

			emit msg_state(0,1);	// заголовок раскодирован успешно
		}
		else
		{
			qDebug() << "bad msg header !";
			msg_header_state = FAILED;
			incom_msg_state = FAILED;

			emit msg_state(1,1);	// заголовок не был раскодирован успешно
			
			qDebug() << "msg id: " << _msg->getSessionId() << " bad data: " << _msg->getIOStatus();
			QString out_str = "";
			MsgProcessor::showBadMessageAsText(_msg, out_str);
			emit bad_COM_message(out_str);
		}

		head_q->clear();
		msg_timer->stop();
		return;
	}
	else if (res == E_RS_NOTFOUND)
	{
		msg_header_state = FAILED;
		incom_msg_state = FAILED;

		emit msg_state(1,1);		// заголовок не был раскодирован успешно
	}
	else if (res == E_RS_FATAL)
	{
		msg_header_state = FAILED;
		incom_msg_state = FAILED;

		emit msg_state(1,1);		// заголовок не был раскодирован успешно
	}
	head_q->clear();	
}


void COMCommander::searchMsgHeader(QUEUE<uint8_t> *_queue, QByteArray &str)
{	
	if (msg_header_state == NOT_DEFINED)
	{				
		for (int i = 0; i < str.count(); i++) 
		{
			uint8_t ch = str[i];			
			if (ch == START_BYTE && i == 0)
			{
				msg_header_state = STARTED;		
				_queue->clear();
				msg_timer->start(msg_header_delay);
			}
			else if (ch == STOP_BYTE && _queue->count() == HEADER_LEN)
			{
				incom_msg_state = STARTED;	
				break;
			}
			else _queue->put(ch);
		}		
	}
	else if (msg_header_state == STARTED)
	{
		for (int i = 0; i < str.count(); i++) 
		{
			uint8_t ch = str[i];			
			if (ch == STOP_BYTE && _queue->count() == HEADER_LEN)
			{
				incom_msg_state = STARTED;	
				break;
			}
			else _queue->put(ch);
		}		
	}
}


void COMCommander::onDataAvailable()
{
	static QList<int> byte_shifts;

#ifdef MSG_TEST
	QString fileName = QApplication::applicationDirPath() + "/incomming_msg_test.txt";
	QFile file(fileName); 
	if (!file.open(QIODevice::ReadOnly)) 
	{ 
		qDebug() << tr("Cannot open file 'incomming_msg_test.txt' for reading"); 
		return; 
	} 

	while (!file.atEnd())
	{
		QByteArray barr = file.readLine();

		QString all_text = QString::fromLocal8Bit(barr);
		if (all_text.size() < 2) continue;

		QStringList qhead = all_text.split("  ").first().split(" ");
		QStringList qbody = all_text.split("  ").at(1).split(" ");

		MsgHeader test_header();
		MsgHeaderInfo test_hdr_info;
		test_hdr_info.start_marker = qhead[0].toUInt();
		test_hdr_info.w_addr = qhead[1].toUInt() & 0x0F;	
		test_hdr_info.r_addr = (qhead[1].toUInt() & 0xF0) >> 4;
		test_hdr_info.id = qhead[2].toUInt();
		test_hdr_info.pack_count = qhead[3].toUInt();
		test_hdr_info.pack_len = qhead[4].toUInt();
		test_hdr_info.block_len = qhead[5].toUInt();
		test_hdr_info.err_count = qhead[6].toUInt();
		test_hdr_info.crc = qhead[7].toUInt();

		MsgHeader *test_hdr = new MsgHeader(&test_hdr_info, gf_data);
		COM_Message *test_msg = new COM_Message(test_hdr);

		QByteArray barr_qbody;
		for (int i = 0; i < qbody.count(); i++) barr_qbody.append(qbody[i].toUInt());
		QUEUE<uint8_t> test_body_q;
		searchPackets(barr_qbody, test_msg, &test_body_q, &byte_shifts);
		for (int i = 0; i < byte_shifts.count(); i++) 
		{
			byte_shifts[i] = byte_shifts[i]/test_hdr_info.pack_len - 1;
		}

		QList<QVector<uint8_t> > test_packs; 
		for (int i = 0; i < test_hdr_info.pack_count; i++)
		{
			QVector<uint8_t> pack_vec(test_hdr_info.pack_len);
			for (int j = 0; j < test_hdr_info.pack_len; j++)
			{
				//pack_vec.data()[j] = (uint8_t)(qbody[i*test_hdr_info.pack_len + j].toUInt());
				if (i*test_hdr_info.pack_len + j < test_body_q.count()) pack_vec.data()[j] = (uint8_t)(test_body_q.at(i*test_hdr_info.pack_len + j));
			}
			test_packs.push_back(pack_vec);
		}

		QList<QVector<uint8_t> > test_block_data; 
		bool res = decodePackets(&test_body_q, test_msg);
		for (int i = 0; i < test_msg->getPackets().count(); i++)
		{
			MsgPacket *test_pack = test_msg->getPackets().at(i);
			QVector<uint8_t> test_block(test_pack->getBlockMap().len);
			memcpy(test_block.data(), test_pack->getBlockMap().data.get(), test_pack->getBlockMap().len);
			test_block_data.push_back(test_block);
		}

		for (int i = 0; i < byte_shifts.count(); i++)
		{
			int index = byte_shifts[i];
			MsgPacket *pack = test_msg->getPackets()[index];
			QVector<uint8_t> pack_block(pack->getBlockMap().len);
			memcpy(pack_block.data(), pack->getBlockMap().data.get(), pack->getBlockMap().len);

			int pre_errors = 0;
			for (int j = 0; j < pack_block.count(); j++) 
			{
				if (pack_block.data()[j] == DATA_STATE_FAIL) pre_errors++;
			}

			int shift_begin = -1;		// номер блока, с которого начался сдвиг
			int j = pack_block.count()-1;
			while (pack_block[j] == DATA_STATE_FAIL && j >= 0) 
			{
				shift_begin = j--; 
			}
			if (shift_begin >= 0)
			{
				SmartArr byte_arr = pack->getByteArray();
				int pos1 = shift_begin*test_hdr_info.block_len;
				int len1 = byte_arr.len - pos1 - 1;
				memcpy(byte_arr.data.get()+pos1, byte_arr.data.get()+pos1+1, len1*sizeof(uint8_t));
				SmartArr res_data_arr(pack->getPacketLen() - 2*pack->getErrsCount()*pack->getBlockCount());	
				SmartArr bad_map_arr(pack->getBlockCount());				
				SmartArr res_byte_arr(pack->getPacketLen());
				SmartArr res_rs_arr(2*pack->getErrsCount()*pack->getBlockCount());
				decodePackByteArray(byte_arr, test_hdr_info.block_len, gf_data, res_byte_arr, res_data_arr, res_rs_arr, bad_map_arr);

				int post_errors = 0;
				for (int j = 0; j < bad_map_arr.len; j++) 
				{
					uint8_t state = bad_map_arr.data.get()[j];
					if (state == DATA_STATE_FAIL) post_errors++;
				}
				if (post_errors < pre_errors)
				{
					memcpy(pack->getByteArray().data.get(), res_byte_arr.data.get(), res_byte_arr.len*sizeof(uint8_t));
					memcpy(pack->getDataArray().data.get(), res_data_arr.data.get(), res_data_arr.len*sizeof(uint8_t));					
					memcpy(pack->getRecData().data.get(), res_rs_arr.data.get(), res_rs_arr.len*sizeof(uint8_t));
					memcpy(pack->getBlockMap().data.get(), bad_map_arr.data.get(), bad_map_arr.len*sizeof(uint8_t));
				}				
			}
		}

		test_msg->setIOStatus(COM_Message::COMPLETED);
		test_msg->setStored(true);

		emit COM_message(test_msg, 0);
	}
#endif

	com_data_ready = true;

	QByteArray str = nmrtool_socket.socket->readAll();
	if (!str.isEmpty())
	{		
		if (msg_header_state < FINISHED)
		{
			searchMsgHeader(head_q, str);
			uint8_t sz = head_q->count();

			if (msg_header_state == STARTED && sz < HEADER_LEN) return;
			else if (msg_header_state < FINISHED && incom_msg_state == STARTED && sz == HEADER_LEN)
			{
				treatCOMData(msg_incomming);
			}
		}

		if (incom_msg_state == FINISHED)
		{
			// если заголовок служебного сообщения (а значит и все служебное сообщение) успешно принят и декодирован
			if (msg_incomming->getMsgHeader()->getStartMarker() == MTYPE_SERVICE)
			{
				msg_incomming->setIOStatus(COM_Message::COMPLETED);
				
				lock_COM_Msg();
				COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
				msg_incomming->copyCOMMsg(msg);

				emit COM_message(msg, 0);
				unlock_COM_Msg();

				executeServiceMsg(msg_incomming);
				head_q->clear();
				msg_header_state = NOT_DEFINED;
				incom_msg_state = NOT_DEFINED;
			}
			// если заголовок короткого сообщения (а значит и все служебное сообщение) успешно принят и декодирован
			if (msg_incomming->getMsgHeader()->getStartMarker() == MTYPE_SHORT)
			{				
				msg_incomming->setIOStatus(COM_Message::COMPLETED);

				uint32_t dev_data_uid = 0;
				if (!sent_device_data_queue.empty())
				{					
					DeviceData *device_data = sent_device_data_queue.at(0);	
					dev_data_uid = device_data->uid;
				}
				COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
				msg_incomming->copyCOMMsg(msg);

				lock_COM_Msg();
				emit COM_message(msg, dev_data_uid); 
				unlock_COM_Msg();

				executeShortMsg(msg_incomming);

				head_q->clear();				
				msg_header_state = NOT_DEFINED;
				incom_msg_state = NOT_DEFINED;				
			}
			// если успешно принят и декодирован заголовок многопакетного сообщения
			else if (msg_incomming->getMsgHeader()->getStartMarker() == MTYPE_MULTYPACK)
			{		
				lock_COM_Msg();
				msg_incomming->setIOStatus(COM_Message::NOT_COMPLETED);		
				uint32_t uid = 0;
				if (!sent_device_data_queue.empty())
				{
					DeviceData *device_data = sent_device_data_queue.at(0);		
					uid = device_data->uid;
				}

				COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
				msg_incomming->copyCOMMsg(msg);

				emit COM_message(msg, uid);

				// --- responseMultypackHeader(msg_incomming, uid) -----
				MsgHeaderInfo hdr_info;
				hdr_info.start_marker = MTYPE_SHORT;
				hdr_info.w_addr = PC_MAIN;
				hdr_info.r_addr = NMR_TOOL;
				hdr_info.id = msg_incomming->getMsgHeader()->getSessionId();
				hdr_info.srv_data.alloc(SRV_DATA_LEN);
				hdr_info.srv_data.data[0] = HEADER_OK;

				MsgHeader *hdr = new MsgHeader(&hdr_info, gf_data);
				COM_Message *msg_hdr_ok = new COM_Message(hdr);
				msg_hdr_ok->setIOStatus(COM_Message::COMPLETED);

				last_request_msg.start_marker = MTYPE_SHORT;
				last_request_msg.w_addr = PC_MAIN;
				last_request_msg.r_addr = NMR_TOOL;
				last_request_msg.id = hdr_info.id;
				last_request_msg.srv_data.alloc(SRV_DATA_LEN);
				last_request_msg.srv_data.data[0] = HEADER_OK;

				msleep(1);				// нужна, т.к. программа на DSP не успевает перейти в режим ожидания сообщения HEADER_OK
				sendCOMMsg(msg_hdr_ok);	

				int pack_len = msg_incomming->getMsgHeader()->getPackLen();
				int pack_count = msg_incomming->getMsgHeader()->getPackCount();
				int pack_delays = main_win->getCommSettings()->packet_delay * (pack_count - 1);
				int packs_delay = 1000 + pack_delays*2.0; // здесь 1000 - перевод из [с] в [мс], 2.0 - 100% запас по времени
				msg_timer->start(packs_delay);
				unlock_COM_Msg();

				lock_COM_Msg();
				emit COM_message(msg_hdr_ok, uid);

				body_q->clear();
				byte_shifts.clear();
				incom_msg_state = PACKS_STARTED;

				byte_counter = 0;
				pack_started = false;
				pack_stoped = false;

				unlock_COM_Msg();
			}	
		}
		// если incom_msg_state = FAILED 
		else if (incom_msg_state == FAILED )
		{				
			msg_incomming->clearCOMMsg();

			head_q->clear();
			body_q->clear();
			msg_header_state = NOT_DEFINED;
			incom_msg_state = NOT_DEFINED;
		}
		// если incom_msg_state = TIMED_OUT
		else if (incom_msg_state == TIMED_OUT)
		{			
			if (msg_incomming->getMsgType() == MTYPE_MULTYPACK && !body_q->empty())
			{
				incom_msg_state = PACKS_FINISHED;
			}
			else
			{				
				msg_incomming->clearCOMMsg();

				head_q->clear();
				body_q->clear();
				msg_header_state = NOT_DEFINED;
				incom_msg_state = NOT_DEFINED;
			}				
		}
		else if (incom_msg_state == PACKS_STARTED)
		{						
			unsigned long packs_count = (unsigned long)msg_incomming->getMsgHeader()->getPackCount();
			unsigned long packs_len = (unsigned long)msg_incomming->getMsgHeader()->getPackLen();

			searchPackets(str, msg_incomming, body_q, &byte_shifts);

			int sz = body_q->count();
			if (sz == 0) 
			{
				return;
			}

			if (sz >= packs_count*packs_len) incom_msg_state = PACKS_FINISHED;					
		}

		if (incom_msg_state == PACKS_FINISHED)
		{				
			unsigned long packs_len = (unsigned long)msg_incomming->getMsgHeader()->getPackLen();
			unsigned long packs_count = (unsigned long)msg_incomming->getMsgHeader()->getPackCount();
			for (int i = 0; i < byte_shifts.count(); i++) 
			{
				byte_shifts[i] = byte_shifts[i]/packs_len - 1;
			}
			for (int i = 0; i < byte_shifts.count(); i++) 
			{
				if (byte_shifts[i] < 0 || byte_shifts[i] >= packs_count)
				{
					byte_shifts.clear();
					break;
				}
			}

			if (body_q->empty())
			{
				return;
			}
			bool res = decodePackets(body_q, msg_incomming);
			if (!res)
			{
				qDebug() << "Bad decoding of multypack message !";

				msg_incomming->clearCOMMsg();

				body_q->clear();
				head_q->clear();

				incom_msg_state = NOT_DEFINED;
				msg_header_state = NOT_DEFINED;

				return;
			}

			if (!byte_shifts.isEmpty())
			{
				for (int i = 0; i < byte_shifts.count(); i++)
				{
					int index = byte_shifts[i];
					MsgPacket *pack = msg_incomming->getPackets()[index];
					repairPackData(pack);
				}
				byte_shifts.clear();
			}

			msg_incomming->setIOStatus(COM_Message::COMPLETED);

			uint32_t dev_data_uid = 0;	
			if (!sent_device_data_queue.empty())
			{
				DeviceData *device_data = sent_device_data_queue.get();		
				dev_data_uid = device_data->uid;
			}

			COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
			msg_incomming->copyCOMMsg(msg);

			lock_COM_Msg();
			emit COM_message(msg, dev_data_uid);			
			unlock_COM_Msg();

			executeMultyPackMsg(msg_incomming, dev_data_uid, res);

#ifdef ENCODING_TEST
			bool bad_data_flag = true;
			for (int k = 0; k < msg_incomming->getPackets().count(); k++)
			{
				MsgPacket *pack = msg_incomming->getPackets()[k];				
				for (int m = 0; m < pack->getBlockMap().len; m++)
				{
					if (pack->getBlockMap().data[m] != DATA_STATE_OK) 
					{
						bad_data_flag = false;
					}
				}
			}
			if (!bad_data_flag)
			{
				qDebug() << "Bad blocks were found !";
				incomming_msg_log->open(QIODevice::Append);
				QString incomming_msg_str = "";
				SmartArr msg_hdr;
				SmartArr msg_body;
				msg_incomming->getHeaderRawData(&msg_hdr);
				for (int k = 0; k < msg_hdr.len; k++)
				{
					uint8_t ch = msg_hdr.data[k];
					QString ch_str = QString::number(ch);
					if (k < msg_hdr.len-1) ch_str += " ";
					incomming_msg_str += ch_str;
				}					
				incomming_msg_str += "  ";
				msg_incomming->getBodyRawData(&msg_body);
				for (int k = 0; k < msg_body.len; k++)
				{
					uint8_t ch = msg_body.data[k];
					QString ch_str = QString::number(ch);
					if (k < msg_body.len-1) ch_str += " ";
					incomming_msg_str += ch_str;
				}				
				incomming_msg_str += "\n\n";
				QTextStream stream(incomming_msg_log);
				stream << incomming_msg_str;

				msg_hdr.destroy();
				msg_body.destroy();
				incomming_msg_log->close();
			}
#endif

			head_q->clear();
			body_q->clear();
			msg_header_state = NOT_DEFINED;
			incom_msg_state = NOT_DEFINED;
		}
	}	
}

void COMCommander::searchPackets(QByteArray &str, COM_Message *_msg, QUEUE<uint8_t>* _queue, QList<int> *shifts)
{
	unsigned long packs_count = (unsigned long)_msg->getMsgHeader()->getPackCount();
	unsigned long packs_len = (unsigned long)_msg->getMsgHeader()->getPackLen();
		
	for (int i = 0; i < str.count(); i++) 
	{
		uint8_t ch = str[i];
		if (!pack_started) 
		{
			if (ch == START_BYTE)
			{
				pack_started = true;
				byte_counter = 0;					
			}
			continue;
		}

		if (pack_started)
		{
			if (byte_counter < packs_len)
			{
				_queue->put(ch);
				byte_counter++;
			}
			else if (byte_counter == packs_len)
			{
				if (ch == STOP_BYTE)
				{
					pack_started = false;
					pack_stoped = true;
					byte_counter = 0;
				}
				else 
				{
					if (!_queue->empty()) 
					{
						if (ch == START_BYTE && _queue->last() == STOP_BYTE)
						{
							pack_started = true;
							pack_stoped = false;
							byte_counter = 0;
						}
						else
						{
							shifts->append(_queue->count());
						}
					}
					else
					{
						return;
					}
				}
			}
		}
	}
}

void COMCommander::repairPackData(MsgPacket *_pack)
{
	QVector<uint8_t> pack_block(_pack->getBlockMap().len);
	memcpy(pack_block.data(), _pack->getBlockMap().data.get(), _pack->getBlockMap().len);

	int pre_errors = 0;
	for (int j = 0; j < pack_block.count(); j++) 
	{
		if (pack_block.data()[j] == DATA_STATE_FAIL) pre_errors++;
	}

	int shift_begin = -1;		// номер блока, с которого начался сдвиг
	for (int j = pack_block.count()-1; j >= 0; j--)
	{
		if (pack_block[j] == DATA_STATE_FAIL) shift_begin = j;
		else break;
	}
	if (shift_begin >= 0)
	{
		SmartArr byte_arr = _pack->getByteArray();
		int pos1 = shift_begin*msg_incomming->getMsgHeader()->getBlockLen();
		int len1 = byte_arr.len - pos1 - 1;
		memcpy(byte_arr.data.get()+pos1, byte_arr.data.get()+pos1+1, len1*sizeof(uint8_t));
		SmartArr res_data_arr(_pack->getPacketLen() - 2*_pack->getErrsCount()*_pack->getBlockCount());	
		SmartArr bad_map_arr(_pack->getBlockCount());				
		SmartArr res_byte_arr(_pack->getPacketLen());
		SmartArr res_rs_arr(2*_pack->getErrsCount()*_pack->getBlockCount());
		decodePackByteArray(byte_arr, msg_incomming->getMsgHeader()->getBlockLen(), gf_data, res_byte_arr, res_data_arr, res_rs_arr, bad_map_arr);

		int post_errors = 0;
		for (int j = 0; j < bad_map_arr.len; j++) 
		{
			uint8_t state = bad_map_arr.data.get()[j];
			if (state == DATA_STATE_FAIL) post_errors++;
		}
		if (post_errors < pre_errors)
		{
			memcpy(_pack->getByteArray().data.get(), res_byte_arr.data.get(), res_byte_arr.len*sizeof(uint8_t));
			memcpy(_pack->getDataArray().data.get(), res_data_arr.data.get(), res_data_arr.len*sizeof(uint8_t));					
			memcpy(_pack->getRecData().data.get(), res_rs_arr.data.get(), res_rs_arr.len*sizeof(uint8_t));
			memcpy(_pack->getBlockMap().data.get(), bad_map_arr.data.get(), bad_map_arr.len*sizeof(uint8_t));
		}				
	}
}

int COMCommander::findMsgHeader(QUEUE<uint8_t>* _queue, COM_Message *_msg)
{
	int cnt = _queue->count();
	if (cnt > (HEADER_LEN))
	{
		_queue->clear();
		return E_RS_LEN;
	}
	if (cnt != HEADER_LEN)  return E_RS_LEN;

	int sz = HEADER_LEN;
	uint8_t arr[HEADER_LEN];

	int i;
	for (i = 0; i < sz; i++)
	{
		uint8_t ch = _queue->at(i);
		arr[i] = ch;
	}

	GFPoly *dist = GFPoly_alloc();
	dist->data = &arr[0];
	dist->power = sz-1;

	GF_Data *_gf_data = _msg->getGFData();
	GF *gf = _gf_data->gf;
	int index = _gf_data->index_hdr;
	GFPoly *g = _gf_data->gf_polys[index];

	GFPoly *dec = GFPoly_alloc();
	GFPoly_self_inv(dist);
	int res = RS_decode(dist,g,gf,dec);
	if (res == E_RS_FATAL || res == E_RS_NOTFOUND)
	{		
		if (dec->power != NoD) GFPoly_destroy(dec);
		free(dist);
		free(dec);

		QString str_err = "";
		for (i = 0; i < sz; i++)
		{
			str_err += QString::number(arr[i]) + " ";			
		}
		qDebug() << "err_header: " + str_err; 

		return res;
	}
	clearCOMMsg(_msg);
	MsgHeader *_msg_header = _msg->getMsgHeader();	
	GFPoly_self_inv(dec);
	
	MsgHeaderInfo _msg_info; 
	_msg_info.start_marker = dec->data[0];
	_msg_info.w_addr = (dec->data[1] & 0x0F);
	_msg_info.r_addr = (dec->data[1] & 0xF0) >> 4;
	_msg_info.id = dec->data[2];
	_msg_info.pack_count = dec->data[3];
	_msg_info.pack_len = dec->data[4];
	_msg_info.block_len = dec->data[5];
	_msg_info.err_count = dec->data[6];
	_msg_info.crc = dec->data[7];
	_msg_info.setSrvData(dec->data+3, SRV_DATA_LEN);
	
	_msg_header->fillAllData(&_msg_info);
	GFPoly_self_inv(dist);
	memcpy(_msg_header->getByteArray().data.get(), dist->data, HEADER_LEN*sizeof(uint8_t));
	memcpy(_msg_header->getRecData().data.get(), dist->data+HEAD_INFO_LEN, SRV_DATA_LEN*sizeof(uint8_t));
	_msg->setSessionId(_msg_info.id);
	_msg->setWriter(_msg_info.w_addr);
	_msg->setReader(_msg_info.r_addr);
	
	GFPoly_destroy(dec);
	free(dist);
	free(dec);

	return res;
}


int COMCommander::checkMsgHeader(COM_Message *_msg)
{
	if (_msg == NULL) return E_MSG_FAIL;
	MsgHeader *_msg_header = _msg->getMsgHeader();
	
	// проверка типа сообщения на принадлежность к одному из известных типов
	uint8_t type = _msg_header->getStartMarker();
	if (type != MTYPE_SERVICE && type != MTYPE_SHORT && type != MTYPE_MULTYPACK && type != MTYPE_PACKET) return E_MSG_TYPE;

	uint8_t arr[HEAD_INFO_LEN-1];
	if (type == MTYPE_MULTYPACK)			// обыкновенное многопакетное (от 1 и больше) сообщение
	{
		if ( _msg_header->getErrsCount() > MAX_REC_ERRS) return E_MSG_REC_LEN;				// количество исправляемых ошибок не может быть > MAX_REC_ERRS
		if ( 2*(_msg_header->getErrsCount()) >= _msg_header->getPackLen() ) return E_MSG_LEN;		// длина всего тела сообщения должна быть больше длины проверочной части
		
		arr[0] = _msg_header->getStartMarker();
		arr[1] = _msg_header->getWrRdAddress();
		arr[2] = _msg_header->getSessionId();
		arr[3] = _msg_header->getPackCount();
		arr[4] = _msg_header->getPackLen();
		arr[5] = _msg_header->getBlockLen();
		arr[6] = _msg_header->getErrsCount();
	}
	else if (type == MTYPE_SERVICE)		// служебное сообщение
	{		
		arr[0] = _msg_header->getStartMarker();
		arr[1] = _msg_header->getWrRdAddress();
		arr[2] = _msg_header->getSessionId();
		SmartArr _srv_data = _msg_header->getSrvData();
		for (int i = 0; i < SRV_DATA_LEN; i++) arr[3+i] = _srv_data.data[i];
	}
	else if (type == MTYPE_SHORT)		// короткое сообщение
	{		
		arr[0] = _msg_header->getStartMarker();
		arr[1] = _msg_header->getWrRdAddress();
		arr[2] = _msg_header->getSessionId();
		SmartArr _shrt_data = _msg_header->getShortData();
		for (int i = 0; i < SRV_DATA_LEN; i++) arr[3+i] = _shrt_data.data[i];
	}

	// проверка контрольной суммы заголовка сообщения
	uint8_t crc = Crc8(&arr[0], HEAD_INFO_LEN-1);
	if (crc != _msg_header->getCRC()) return E_MSG_CRC;

	return E_MSG_OK;
}

void COMCommander::executeServiceMsg(COM_Message *_msg)
{
	msg_timer->stop();	

	if (!sent_device_data_queue.empty()) return;
	
	SmartArr arr = _msg->getMsgHeader()->getShortData();
	uint8_t comm = arr.data[0];	
	uint8_t byte0 = arr.data[1];

	// Биты состояния прибора в 5-ом байте служебного сообщения (arr.data[1] - в данном случае) о готовности прибора к сеансу связи:
	// бит 0 - напряжение питания: 							0/1 - понижено / нормальное
	// бит 1 - готовность телеметрии: 						0/1 - не готова / готова
	// бит 2 - состояние программатора ПЛИС: 				0/1 - запущен командой proger_start() / остановлен командой proger_stop()
	// бит 3 - состояние программы в программаторе ПЛИС: 	0/1 - исполняется / завершилась
	bool telemetry_on = (byte0 & 0x2) > 0;
	uint8_t incomming_id = _msg->getMsgHeader()->getSessionId();

	bool proger_started = (byte0 & 0x4);		// интервальный программатор ПЛИС был запущен командой proger_start()
	bool seq_finished = (byte0 & 0x8);		// закончилась или нет последовательность в программаторе ПЛИС командой COM_STOP
	
	//if (seq_finished || !proger_started) nmrtool_state = false;
	//if (!proger_started) nmrtool_state = false;
	nmrtool_state = proger_started;
	//if (seq_finished) nmrtool_state = true;

	switch (comm)
	{
	case NMRTOOL_IS_READY:
		{
			if (device_data_queue.empty() && (nmrtool_state | telemetry_on | sdsptool_state) && !seq_finished) // if NMR Tool is ready to read messages or if telemetry is ready
			{
				lock_COM_Msg();
				MsgHeaderInfo hdr_info;
				hdr_info.start_marker = MTYPE_SHORT;
				hdr_info.w_addr = PC_MAIN;
				hdr_info.r_addr = NMR_TOOL;
				if (incomming_id == 0) hdr_info.id = MsgHeader::generateSessionId();
				else hdr_info.id = incomming_id;
				hdr_info.srv_data.alloc(SRV_DATA_LEN);
				hdr_info.srv_data.data[0] = GET_DATA;	
		
				MsgHeader *hdr = new MsgHeader(&hdr_info, gf_data);
				COM_Message *msg = new COM_Message(hdr);
				msg->setIOStatus(COM_Message::COMPLETED);				
				
				last_request_msg.start_marker = MTYPE_SHORT;
				last_request_msg.w_addr = PC_MAIN;
				last_request_msg.r_addr = NMR_TOOL;
				last_request_msg.id = hdr_info.id;
				last_request_msg.srv_data.alloc(SRV_DATA_LEN);
				last_request_msg.srv_data.data[0] = GET_DATA;

				sendCOMMsg(msg);

				emit COM_message(msg, 0);				
				unlock_COM_Msg();

			}
			else if (!device_data_queue.empty())
			{
				DeviceData *device_data = device_data_queue.get();					
				uint32_t uid = device_data->uid;
				uint8_t comm_id = device_data->comm_id;			// id команды для каротажного прибора 
				uint8_t type = device_data->type; 				
				QList<Field_Comm*> *fields = device_data->fields;
				
				MsgHeaderInfo hdr_info;
				hdr_info.start_marker = type;
				hdr_info.w_addr = PC_MAIN;
				hdr_info.r_addr = NMR_TOOL;
				if (incomming_id == 0) hdr_info.id = MsgHeader::generateSessionId();
				else hdr_info.id = incomming_id;
				if (type == MTYPE_SHORT)
				{
					uint8_t cmd_data[SHORT_DATA_LEN];
					hdr_info.srv_data.alloc(SHORT_DATA_LEN);
					hdr_info.srv_data.data[0] = comm_id;
					if (fields->size() > 0)
					{
						Field_Comm::Value_Type v_type = fields->first()->value_type;
						double par = fields->first()->value->at(0);
						switch (v_type)
						{
						case Field_Comm::UINT16:
							{
								uint16_t par_u16 = (uint16_t)par;
								hdr_info.srv_data.data[1] = (uint8_t)(par_u16 & 0x00FF);
								hdr_info.srv_data.data[2] = (uint8_t)(par_u16 >> 8);
								break;
							}
						case Field_Comm::UINT8:
							{
								uint16_t par_u8 = (uint8_t)par;
								hdr_info.srv_data.data[1] = par_u8;
								break;
							}
						default: break;
						}
					}	

					lock_COM_Msg();			
					MsgHeader *hdr = new MsgHeader(&hdr_info, gf_data);
					COM_Message *msg = new COM_Message(hdr);
					msg->setIOStatus(COM_Message::COMPLETED);	
					//msg_outcomming = msg;					
					//unlock_COM_Msg();

					sendCOMMsg(msg);
					//qDebug() << "outcomming short msg";
					device_data->is_sent = true;
					uint32_t dev_data_uid = device_data->uid;
					sent_device_data_queue.put(device_data);		

					//lock_COM_Msg();
					emit COM_message(msg, dev_data_uid);	
					unlock_COM_Msg();
				}
				else if (type == MTYPE_MULTYPACK)
				{									
					Communication_Settings *comm_settings = main_win->getCommSettings();
					uint8_t temp_data[65535];
					memset(&temp_data[0], 0x00, sizeof(uint8_t));					
					int pos = 0;															// счетчик байт в массиве данных, приготовленных для передачи
					temp_data[pos++] = comm_id;

					for (int i = 0; i < fields->size(); i++)
					{
						int ptype = fields->at(i)->value_type;
						switch (ptype)
						{
						case Field_Comm::UINT8:		
						{							
							int loc_len = fields->at(i)->value->size() * sizeof(uint8_t);
							uint16_t field_len = (uint16_t)loc_len;							// длина данных в байтах для текущего поля
							if (i > 0) 
							{
								comm_id = fields->at(i)->code;
								temp_data[pos++] = comm_id;
							}
							temp_data[pos++] = (uint8_t)(field_len & 0x00FF);				// длина данных в байтах для текущего поля (младший байт)
							temp_data[pos++] = (uint8_t)((field_len >> 8) & 0x00FF);		// длина данных в байтах для текущего поля (старший байт)

							for (int j = 0; j < fields->at(i)->value->size(); j++)
							{
								uint8_t val = (uint8_t)fields->at(i)->value->at(j);
								temp_data[pos++] = val;
							}									
							if (i < fields->size()-1) temp_data[pos++] = (uint8_t)NAN_DATA;	// разделитель между полями-данными

							break;
						}
						case Field_Comm::FLOAT:
						{
							int loc_len = fields->at(i)->value->size() * sizeof(float);
							uint16_t field_len = (uint16_t)loc_len;							// длина данных в байтах для текущего поля
							if (i > 0) 
							{
								comm_id = fields->at(i)->code;
								temp_data[pos++] = comm_id;
							}
							temp_data[pos++] = (uint8_t)(field_len & 0x00FF);				// длина данных в байтах для текущего поля (младший байт)
							temp_data[pos++] = (uint8_t)((field_len >> 8) & 0x00FF);		// длина данных в байтах для текущего поля (старший байт)

							for (int j = 0; j < fields->at(i)->value->size(); j++)
							{								
								float value = (float)fields->at(i)->value->at(j);
								memcpy(&temp_data[pos], (uint8_t*)(&value), sizeof(float));
								pos += sizeof(float);
							}							
							if (i < fields->size()-1) temp_data[pos++] = (uint8_t)NAN_DATA;	// разделитель между полями-данными

							break;
						}
						/*case Field_Comm::UINT16:	data_len += device_data->fields->at(i)->value->size() * sizeof(uint16_t); break;						
						case Field_Comm::DOUBLE:	data_len += device_data->fields->at(i)->value->size() * sizeof(double); break;*/
						case Field_Comm::UINT32:	
							{
								int loc_len = fields->at(i)->value->size() * sizeof(float);
								uint16_t field_len = (uint16_t)loc_len;							// длина данных в байтах для текущего поля
								if (i > 0) 
								{
									comm_id = fields->at(i)->code;
									temp_data[pos++] = comm_id;
								}
								temp_data[pos++] = (uint8_t)(field_len & 0x00FF);				// длина данных в байтах для текущего поля (младший байт)
								temp_data[pos++] = (uint8_t)((field_len >> 8) & 0x00FF);		// длина данных в байтах для текущего поля (старший байт)

								for (int j = 0; j < fields->at(i)->value->size(); j++)
								{								
									int value = (int)fields->at(i)->value->at(j);
									memcpy(&temp_data[pos], (uint8_t*)(&value), sizeof(int));
									pos += sizeof(int);
								}							
								if (i < fields->size()-1) temp_data[pos++] = (uint8_t)NAN_DATA;	// разделитель между полями-данными

								break;
							}
						default: break;
						}						
					}

					int data_len = pos;
					int pack_len = main_win->getCommSettings()->packet_length;
					if (main_win->getCommSettings()->packlen_autoadjust) pack_len = estimateBestPackLen(data_len, comm_settings->block_length, 2*comm_settings->errs_count);
					int pack_count = (int)ceil(data_len/(double)pack_len);
										
					hdr_info.block_len = comm_settings->block_length;
					hdr_info.err_count = comm_settings->errs_count;
					hdr_info.pack_count = pack_count;
					hdr_info.pack_len = pack_len;
					gf_data->index_body = comm_settings->errs_count - 1;
														
					MsgHeader *outcomming_hdr = msg_outcomming->getMsgHeader();					
					outcomming_hdr->setInfoData(hdr_info, gf_data);
					msg_outcomming->setSessionId(outcomming_hdr->getSessionId());
					msg_outcomming->generateMsgId();
					msg_outcomming->setWriter(outcomming_hdr->getWriter());
					msg_outcomming->setReader(outcomming_hdr->getReader());
					msg_outcomming->setPriority(COM_Message::NORMAL_PRIORITY);
										
					uint16_t dpos = 0;
					int pack_number = 1;
					while (dpos < data_len)
					{
						MsgPacket* pack = new MsgPacket(pack_number++, outcomming_hdr, interleaved_out());
						pack->pushData(&temp_data[0], (uint16_t)data_len, dpos);
						msg_outcomming->addPacket(pack);
					}
					outcomming_hdr->setPackCount(pack_number-1);
					msg_outcomming->setIOStatus(COM_Message::HEADER_SENT);	
					
					sendCOMMsg(msg_outcomming);	
					
					device_data->is_sent = true;
					uint32_t dev_data_uid = device_data->uid;
					sent_device_data_queue.put(device_data);

					lock_COM_Msg();
					COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
					msg_outcomming->copyCOMMsg(msg);
					
					//lock_COM_Msg();
					emit COM_message(msg, dev_data_uid);
					unlock_COM_Msg();
				}		
			}			
			//last_request_msg.start_marker = NONE;

			break;
		}
	case NMRTOOL_ISNOT_READY:
		{
			last_request_msg.start_marker = NONE;

			if (!nmrtool_state) return;			
			break;
		}
	default: break;		
	}
}

void COMCommander::executeShortMsg(COM_Message *_msg)
{
	msg_timer->stop();	

	SmartArr arr = _msg->getMsgHeader()->getShortData();
	uint8_t comm = arr.data[0];
	uint8_t comm_param = arr.data[1]; 
	uint8_t incomming_id = _msg->getMsgHeader()->getSessionId();
	uint8_t session_id = _msg->getSessionId();

	switch (comm)
	{
	case DATA_OK:
	case NMRTOOL_CONNECT:
	case NMRTOOL_CONNECT_DEF:
		{
			if (!sent_device_data_queue.empty())
			{
				sent_device_data_queue.get();
			}
			break;
		}
	case DIEL_DATA_READY:
		{
			if (!sent_device_data_queue.empty())
			{
				sent_device_data_queue.get();
			}
			sdsptool_state = true;
			break;
		}
	case BAD_DATA:
		{
			if (!sent_device_data_queue.empty())
			{
				sent_device_data_queue.get();
			}
			sdsptool_state = false;
			
			if (comm_param > 0)
			{
				lock_COM_Msg();
				COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
				_msg->copyCOMMsg(msg);			
				
				emit COM_message(msg, 0);
				unlock_COM_Msg();
			}		

			break;
		}
	case HEADER_OK:
		{
			if (!sent_device_data_queue.empty()) 
			{
				//DeviceData *device_data = sent_device_data_queue.get();
				DeviceData *device_data = sent_device_data_queue.at(0);		
				uint32_t uid = device_data->uid;
				//uint8_t comm_id = device_data->comm_id;			// id команды для каротажного прибора 

				if (msg_outcomming->getSessionId() == session_id)
				{
					device_data->is_sent = true;	
					msg_outcomming->setIOStatus(COM_Message::SENT);

					//lock_COM_Msg();	
					msleep(1);
					sendCOMMsg(msg_outcomming);						
					//unlock_COM_Msg();
										
					lock_COM_Msg();
					COM_Message *msg = new COM_Message(new MsgHeader(gf_data));
					msg_outcomming->copyCOMMsg(msg);
					//unlock_COM_Msg();

					//lock_COM_Msg();
					emit COM_message(msg, uid);
					unlock_COM_Msg();

					msg_outcomming->clearCOMMsg();
				}
			}			
			break;
		}
	case REPEAT_CMD:
		{
			/*if (last_request_msg.start_marker != NONE) 
			{
				lock_COM_Msg();
				MsgHeader *hdr = new MsgHeader(&last_request_msg, gf_data);
				COM_Message *msg = new COM_Message(hdr);
				msg->setIOStatus(COM_Message::COMPLETED);
				
				sendCOMMsg(msg);	
				emit COM_message(msg, 0);
				unlock_COM_Msg();

				uint8_t last_cmd = last_request_msg.srv_data.data.get()[0];
				qDebug() << QString("Last CMD ( %1 ) was repeated!").arg(last_cmd);
			}*/
			qDebug() << "REPEAT_CMD was received.";
			
			break;
		}
	default: break;
	}
}

void COMCommander::responseMultypackHeader(COM_Message *_msg, uint32_t _uid)
{
	MsgHeaderInfo hdr_info;
	hdr_info.start_marker = MTYPE_SHORT;
	hdr_info.w_addr = PC_MAIN;
	hdr_info.r_addr = NMR_TOOL;
	hdr_info.id = _msg->getMsgHeader()->getSessionId();
	hdr_info.srv_data.alloc(SRV_DATA_LEN);
	hdr_info.srv_data.data[0] = HEADER_OK;

	lock_COM_Msg();
	MsgHeader *hdr = new MsgHeader(&hdr_info, gf_data);
	COM_Message *msg = new COM_Message(hdr);
	msg->setIOStatus(COM_Message::COMPLETED);
	//unlock_COM_Msg();	

	last_request_msg.start_marker = MTYPE_SHORT;
	last_request_msg.w_addr = PC_MAIN;
	last_request_msg.r_addr = NMR_TOOL;
	last_request_msg.id = hdr_info.id;
	last_request_msg.srv_data.alloc(SRV_DATA_LEN);
	last_request_msg.srv_data.data[0] = HEADER_OK;

	//msg_queue->put(msg);
	//unlock_COM_Msg();	
	//msleep(1);				// нужна, т.к. программа на DSP не успевает перейти в режим ожидания сообщения HEADER_OK
	sendCOMMsg(msg);	
	//unlock_COM_Msg();

	int pack_len = _msg->getMsgHeader()->getPackLen();
	int pack_count = _msg->getMsgHeader()->getPackCount();
	int pack_delays = main_win->getCommSettings()->packet_delay * (pack_count - 1);
	int packs_delay = 1000 + pack_delays*2.0; // здесь 1000 - перевод из [с] в [мс], 2.0 - 100% запас по времени
	msg_timer->start(packs_delay);

	//lock_COM_Msg();
	emit COM_message(msg, _uid);
	unlock_COM_Msg();
}

bool COMCommander::decodePackets(QUEUE<uint8_t> *data_q, COM_Message *_msg)
{	
	if (data_q->count() == 0) return false;
		
	_msg->clearPacketList();	
	
	GF *gf = _msg->getGFData()->gf;
	int index = _msg->getMsgHeader()->getErrsCount() - 1;
	GFPoly *g = _msg->getGFData()->gf_polys[index];

	int pack_count = _msg->getMsgHeader()->getPackCount();
	int pack_len = _msg->getMsgHeader()->getPackLen();
	int block_len = _msg->getMsgHeader()->getBlockLen();

	int pack_number = 0;
	while (data_q->count() > 0)
	{		
		MsgPacket *pack = new MsgPacket(++pack_number, _msg->getMsgHeader());
		uint16_t pack_data_len =  pack->getDataLen();
		SmartArr data_arr = pack->getDataArray();
		SmartArr byte_arr = pack->getByteArray();
		SmartArr block_map = pack->getBlockMap();
		data_arr.filla(NAN_DATA, pack_data_len);

		int error_counter = 0;
		int cnt = 0;
		uint16_t pack_byte_counter = 0;
		uint8_t block_counter = 0;
		while (pack_byte_counter < pack_len && data_q->count() > 0)
		{
			GFPoly *dist = GFPoly_alloc();
			dist->data = (guint8*)calloc(block_len, sizeof(guint8));
			uint8_t block_byte_counter = 0; 			
			while (block_byte_counter < block_len && data_q->count() > 0)
			{
				uint8_t ch = data_q->get();
				byte_arr.data[pack_byte_counter++] = ch;
				dist->data[block_byte_counter] = ch;
				dist->power = block_byte_counter++;				
			}

			if ((++block_counter) > (pack_len/block_len))
			{
				block_map.data[block_counter-1] = DATA_STATE_FAIL;
				error_counter++;
				continue;
			}

			if (block_byte_counter == block_len)
			{
				int len = _msg->getMsgHeader()->getBlockLen() - 2*_msg->getMsgHeader()->getErrsCount();
				
				GFPoly *dec = GFPoly_alloc();
				GFPoly_self_inv(dist);
				int res = RS_decode(dist,g,gf,dec);
				if (res == E_RS_OK && len > 0) 
				{
					block_map.data[block_counter-1] = DATA_STATE_OK;
					GFPoly_self_inv(dec);					
					//memcpy(data_arr.data+cnt, dec->data, len*sizeof(uint8_t)); 	
					memcpy(data_arr.data.get()+cnt, dec->data, len*sizeof(uint8_t)); 
				}
				else if (len <= 0) 
				{					
					GFPoly_destroy(dist);					
					free(dist);
					delete pack;

					return false;	// fatal error
				}
				else 
				{					
					block_map.data[block_counter-1] = DATA_STATE_FAIL;
					if (dist != NULL)
					GFPoly_self_inv(dist);					
					//memcpy(data_arr.data+cnt, dist->data, len*sizeof(uint8_t));
					memcpy(data_arr.data.get()+cnt, dist->data, len*sizeof(uint8_t));
					error_counter++;
				}
				cnt += len;
				
				GFPoly_destroy(dec);
				free(dec);
			}
			
			block_map.len = block_counter;
			GFPoly_destroy(dist);
			free(dist);			
		}			
		
		_msg->addPacket(pack);	
		if (_msg->getPackets().count() >= pack_count) 
		{
			data_q->clear();
		}

		emit msg_state(error_counter, pack_len/block_len);
	}	
	
	return true;
}

void COMCommander::decodePackByteArray(SmartArr byte_arr, int block_len, GF_Data *gf_data, SmartArr &res_byte_arr, SmartArr &res_data_arr, SmartArr &res_rs_arr, SmartArr &block_map)
{
	
	GF *gf = gf_data->gf;
	int index = gf_data->index_body;
	GFPoly *g = gf_data->gf_polys[index];
		
	int pack_len = byte_arr.len;
	int num_blocks = pack_len/block_len;
	int errs_count = index+1;	
	int pack_data_len =  pack_len - 2*errs_count*num_blocks;
	int block_data_len = block_len - 2*errs_count;

	for (int i = 0; i < num_blocks; i++)
	{
		GFPoly *dist = GFPoly_alloc();
		dist->data = (guint8*)calloc(block_len, sizeof(guint8));
		memcpy(dist->data, byte_arr.data.get()+i*block_len, block_len*sizeof(uint8_t));
		dist->power = block_len-1;

		GFPoly *dec = GFPoly_alloc();
		GFPoly_self_inv(dist);
		int res = RS_decode(dist,g,gf,dec);
		if (res == E_RS_OK) block_map.data[i] = DATA_STATE_OK;
		else block_map.data[i] = DATA_STATE_FAIL;
		
		GFPoly_self_inv(dec);				
		memcpy(res_byte_arr.data.get()+i*block_len, dec->data, block_len*sizeof(uint8_t)); 
		memcpy(res_data_arr.data.get()+i*block_data_len, dec->data, block_data_len*sizeof(uint8_t));
		memcpy(res_rs_arr.data.get() + i*2*errs_count, dec->data + block_data_len, 2*errs_count*sizeof(uint8_t));
		
		GFPoly_destroy(dec);
		free(dec);

		GFPoly_destroy(dist);
		free(dist);			
	}	
}

int COMCommander::estimateBestPackLen(int data_len, int block_len, int rs_part_len)
{
	int blocks = ceil((double)data_len/(block_len-rs_part_len));
	int full_data_len = blocks*block_len;
			
	bool ready = false;
	int pack_len = floor(254.0/block_len)*block_len;
	int new_pack_len = pack_len;
	int pack_count = ceil((double)full_data_len/(new_pack_len-PACK_INFO_LEN-1));
	while (pack_count*(new_pack_len-PACK_INFO_LEN-1) - full_data_len > 0)
	{
		pack_len = new_pack_len;
		new_pack_len -= block_len;
	}

	return pack_len;
}



void COMCommander::executeMultyPackMsg(COM_Message *_msg, uint32_t _uid, bool res)
{	
	msg_timer->stop();
	
	MsgHeaderInfo hdr_info;
	hdr_info.start_marker = MTYPE_SHORT;
	hdr_info.w_addr = PC_MAIN;
	hdr_info.r_addr = NMR_TOOL;
	hdr_info.id = _msg->getMsgHeader()->getSessionId();	
	hdr_info.srv_data.alloc(SRV_DATA_LEN);
	if (res) hdr_info.srv_data.data[0] = DATA_OK;
	else hdr_info.srv_data.data[0] =BAD_DATA;

	last_request_msg.start_marker = MTYPE_SHORT;
	last_request_msg.w_addr = PC_MAIN;
	last_request_msg.r_addr = NMR_TOOL;
	last_request_msg.id = hdr_info.id;
	last_request_msg.srv_data.alloc(SRV_DATA_LEN);
	last_request_msg.srv_data.data[0] = hdr_info.srv_data.data[0];
	
	lock_COM_Msg();
	MsgHeader *hdr = new MsgHeader(&hdr_info, gf_data);
	COM_Message *msg = new COM_Message(hdr);
	msg->setIOStatus(COM_Message::COMPLETED);
	
	msleep(1);
	sendCOMMsg(msg);	
	
	//lock_COM_Msg();
	emit COM_message(msg, _uid);
	unlock_COM_Msg();
}


void COMCommander::clearCOMMsg(COM_Message *_msg)
{
	//lock_COM_Msg();
	_msg->clearMsgHeader();
	_msg->clearPacketList();	
	//unlock_COM_Msg();	
}

void COMCommander::addCmdToQueue(DeviceData* dev_data)
{	
	device_data_queue.put(dev_data);
}

void COMCommander::breakAllActions()
{
	MsgHeaderInfo hdr_info;
	hdr_info.start_marker = MTYPE_SHORT;
	hdr_info.w_addr = PC_MAIN;
	hdr_info.r_addr = NMR_TOOL;
	hdr_info.id = 0;	
	hdr_info.srv_data.alloc(SRV_DATA_LEN);
	hdr_info.srv_data.data[0] = NMRTOOL_STOP;	

	sdsptool_state = false;
	
	lock_COM_Msg();
	MsgHeader *hdr = new MsgHeader(&hdr_info, gf_data);
	COM_Message *msg = new COM_Message(hdr);
	msg->setIOStatus(COM_Message::COMPLETED);
	//unlock_COM_Msg();

	sendCOMMsg(msg);

	//lock_COM_Msg();
	emit COM_message(msg, 0);
	unlock_COM_Msg();

	on_break = true;
}

void COMCommander::sendToSDSP(QByteArray& arr)
{
	nmrtool_socket.socket->write((char*)(arr.data(), arr.size()));
}

void COMCommander::timeClocked()
{
	if (!device_data_queue.empty())
	{		
		DeviceData *device_data = device_data_queue.at(0);			
		device_data->life_time += clocker->period();		
		if (nmrtool_state && device_data->life_time > msg_life_time)
		{
			incom_msg_state = TIMED_OUT;			
			DeviceData *_device_data = device_data_queue.get();
			uint32_t uid = _device_data->uid;
			
			emit device_data_timed_out(uid);
		}	
		else if (!nmrtool_state && device_data->life_time > sdsp_req_delay)
		{
			incom_msg_state = TIMED_OUT;			
			DeviceData *_device_data = device_data_queue.get();
			uint32_t uid = _device_data->uid;

			emit device_data_timed_out(uid);
		}		
	}

	if (!sent_device_data_queue.empty())
	{
		DeviceData *device_data = sent_device_data_queue.at(0);
		device_data->elapsed_time += clocker->period();
		device_data->life_time += clocker->period();	

		if (nmrtool_state && (device_data->elapsed_time > msg_req_delay || device_data->life_time > msg_life_time))
		{
			incom_msg_state = TIMED_OUT;	
			DeviceData *_device_data = sent_device_data_queue.get();
			uint32_t uid = _device_data->uid;

			emit device_data_timed_out(uid);
		}
		/*else if (!nmrtool_state)
		{
			incom_msg_state = TIMED_OUT;	
			DeviceData *_device_data = sent_device_data_queue.get();
			if (_device_data->comm_id == DATA_PROC && (device_data->elapsed_time > MSG_REQ_DELAY || device_data->life_time > MSG_LIFETIME))
			{
				uint32_t uid = _device_data->uid;
				emit device_data_timed_out(uid);
			}
			else if (_device_data->comm_id != DATA_PROC && (device_data->elapsed_time > sdsp_req_delay || device_data->life_time > sdsp_req_delay))
			{
				uint32_t uid = _device_data->uid;
				emit device_data_timed_out(uid);
			}
		}*/
		else if (!nmrtool_state && (device_data->elapsed_time > sdsp_req_delay || device_data->life_time > sdsp_req_delay))
		{
			incom_msg_state = TIMED_OUT;	
			DeviceData *_device_data = sent_device_data_queue.get();
			uint32_t uid = _device_data->uid;

			emit device_data_timed_out(uid);
		}
	}	

	if (on_break)
	{
		for (int i = 0; i < device_data_queue.count(); i++)
		{
			DeviceData *_device_data = device_data_queue.get();
			uint32_t uid = _device_data->uid;

			emit device_data_timed_out(uid);
		}

		for (int i = 0; i < sent_device_data_queue.count(); i++)
		{
			DeviceData *_device_data = sent_device_data_queue.get();
			uint32_t uid = _device_data->uid;

			emit device_data_timed_out(uid);
		}

		on_break = false;
	}
}

void COMCommander::lifeTimeElapsed()
{
	if (incom_msg_state == STARTED || incom_msg_state == PACKS_STARTED) 
	{
		incom_msg_state = TIMED_OUT;	

		head_q->clear();
		body_q->clear();
		msg_header_state = NOT_DEFINED;
		incom_msg_state = NOT_DEFINED;
	}
	
	else return;
}


void COMCommander::showBadMessageAsText(COM_Message *msg, QString &text)
{
	if (msg == NULL) return;
	QStringList msgText;
	MsgInfo::extractAllAsText(msgText, msg);

	QString in_str = "";
	QString out_str = "";

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

	//emit message_str(out_str); 
}



DepthCommunicator::DepthCommunicator(QTcpSocket *socket, Clocker *clocker, QObject *parent)
{
	thread_id = thid++;	

	//COM_port = com_port;	
	this->socket = socket;
	connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
		
	data_q = new QUEUE<uint8_t>(DEPTH_DATA_LEN);	

	this->clocker = clocker;	
}

DepthCommunicator::~DepthCommunicator()
{
	delete data_q;
	
	for (int i = 0; i < depth_data_queue.count(); i++)
	{
		DepthMeterData *depth_data = depth_data_queue.get();
		delete depth_data;
	}

	for (int i = 0; i < sent_depth_data_queue.count(); i++)
	{
		DepthMeterData *depth_data = sent_depth_data_queue.get();
		delete depth_data;
	}
}

void DepthCommunicator::timeClocked()
{
	if (!sent_depth_data_queue.empty())
	{		
		DepthMeterData *sent_depth_data = sent_depth_data_queue.at(0);
		sent_depth_data->life_time += clocker->period();		
		
		if (sent_depth_data->life_time > DEPTH_REQ_TIMEOUT)
		{
			uint32_t uid = sent_depth_data->uid;
			uint8_t type = sent_depth_data->type;

			sent_depth_data = sent_depth_data_queue.get();
			delete sent_depth_data;			
			
			emit data_timed_out(uid, type);			
		}		
	}
	else
	{
		if (depth_data_queue.empty()) return;

		DepthMeterData *depth_data = depth_data_queue.get();
		sent_depth_data_queue.put(depth_data);
		sendRequestToCOM(depth_data);		
	}	
}

/*
void DepthCommunicator::setPort(QextSerialPort *com_port)
{
	COM_port = com_port; 
	connect(COM_port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}
*/

void DepthCommunicator::setSocket(QTcpSocket *socket)
{
	this->socket = socket; 
	connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}

void DepthCommunicator::sendRequestToCOM(DepthMeterData *dmd)
{	
	dmd->is_sent = true;
	
	uint8_t type[] = { dmd->type };
	if (type[0] == DEVICE_SEARCH) type[0] = DEPTH_DATA;

	//COM_port->write((char*)(&type[0]), 1);	
	socket->write((char*)(&type[0]), 1);
}

void DepthCommunicator::toMeasure(uint32_t uid, uint8_t data_type)
{
	DepthMeterData *dmd = new DepthMeterData(uid, data_type);
	depth_data_queue.put(dmd);
}

void DepthCommunicator::onDataAvailable()
{
	//QByteArray str = COM_port->readAll();	
	QByteArray str = socket->readAll();

	if (!str.isEmpty())
	{	
		for (int i = 0; i < str.count(); i++) 
		{
			uint8_t ch = str[i];
			data_q->put(ch);
		}
		int sz = data_q->count(); 	

		if (sz != 3) return;

		uint32_t byte1 = data_q->get();
		uint32_t byte2 = data_q->get();
		uint32_t byte3 = data_q->get();

		uint32_t res_u32 = (byte1 << 16) | (byte2 << 8) | byte3;
		if (sent_depth_data_queue.empty()) return;
		else 
		{
			for (int i = 0; i < sent_depth_data_queue.count(); i++)
			{
				DepthMeterData *dmd = sent_depth_data_queue.get();
				uint32_t uid = dmd->uid;
				uint8_t type = dmd->type;
				delete dmd;

				double res = res_u32/100.0;

				// фильтр резких выбросов глубины 
				if (!depth_buffer.isEmpty())
				{
					double last_depth = depth_buffer.last();
					if (fabs(last_depth - res) > DEPTH_MAX_DELTA) return;

					if (depth_buffer.count() >= DEPTH_BUFF_SIZE) depth_buffer.pop_front();
					depth_buffer.push_back(res);					
				}
				else depth_buffer.push_back(res);
				// *******************************

				emit measured_data(uid, type, res);
			}			
		}
	}
}

void DepthCommunicator::run()
{
	if (!socket)
	{
		freeze();
		emit error_msg("Bad pointer to Depth Meter socket!");
		return;
	}
	if (!socket->isOpen())
	{
		freeze();
		emit error_msg("Depth Meter socket is closed!");
		return;
	}

	connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));

	is_freezed = false;
	is_running = true;

	exec();
}


void DepthCommunicator::freeze()
{
	disconnect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = true;	

	emit frozen(is_freezed);
}

void DepthCommunicator::wake()
{
	connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = false;	

	emit frozen(is_freezed);
}


LeuzeCommunicator::LeuzeCommunicator(QTcpSocket *socket, Clocker *clocker, QObject *parent)
{
	thread_id = thid++;	

	//COM_port = com_port;	
	//COM_port->flush();
	this->socket = socket;

	connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
	
	this->clocker = clocker;	
}

LeuzeCommunicator::~LeuzeCommunicator()
{
	
}

/*
void LeuzeCommunicator::setPort(QextSerialPort *com_port)
{
	COM_port = com_port; 
	COM_port->flush();

	connect(COM_port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}
*/

void LeuzeCommunicator::setSocket(QTcpSocket *socket)
{
	this->socket = socket; 
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}

/*
void LeuzeCommunicator::toMeasure(uint32_t uid, uint8_t data_type)
{
	if (distance_buffer.isEmpty()) return;
	if (data_type != DEPTH_DATA) return;

	double res = 0;
	for (int i = 0; i < distance_buffer.count(); i++)
	{
		res += distance_buffer[i];
	}
	res /= distance_buffer.count();
	distance_buffer.clear();

	emit measured_data(uid, DEPTH_DATA, res);
}


void LeuzeCommunicator::onDataAvailable()
{
	QByteArray byte_data = COM_port->readAll();	

	if (!byte_data.isEmpty())
	{
		acc_data += byte_data;
		QStringList str_data = acc_data.split('\r');
		
		bool _ok = false;
		int sz = str_data.count();
		for (int i = 0; i < sz; i++)
		{			
			if (str_data.first().size() == LEUZE_DATA_LEN)
			{				
				int value = str_data.first().toInt(&_ok);
				if (_ok) distance_buffer.push_back(value);
			}
			str_data.takeFirst();				
		}	
		
		if (_ok) acc_data = "";
	}	
}
*/

void LeuzeCommunicator::toMeasure(uint32_t uid, uint8_t data_type)
{	
	if (data_type != DEPTH_DATA) return;
		
	sendRequestToCOM();

	if (!distance_buffer.isEmpty()) 
	{
		switch (distance_buffer.last().first)
		{
		case NoError:		emit measured_data(uid, DEPTH_DATA, distance_buffer.last().second); break;
		case NoSignal:		emit error_msg(tr("No Signal!")); break;
		case UnknownError:	emit error_msg(tr("Error!")); break;
		}
		distance_buffer.clear();
	}
}

void LeuzeCommunicator::sendRequestToCOM()
{	
	char req[4];
	req[0] = '*';
	req[1] = '0';
	req[2] = 'M';
	req[3] = '#';

	socket->write((char*)(&req[0]), 4);	
}

void LeuzeCommunicator::onDataAvailable()
{
	QByteArray byte_data = socket->readAll();	
	acc_data += byte_data;

	if (acc_data.count() < LEUZE_DATA_LEN) return;
	else if (acc_data.count() == LEUZE_DATA_LEN)
	{
		if (acc_data.at(0) == 0x2A && acc_data.at(LEUZE_DATA_LEN-1) == 0x23)
		{
			QString str_data = acc_data.split('*').last().split('#').first();
			
			int err_code = NoError;
			if (str_data.contains("65535")) err_code = NoSignal;
			if (str_data.contains("9999")) err_code = UnknownError;
			
			distance_buffer.clear();

			bool _ok;
			int value = str_data.toInt(&_ok);				
			if (_ok) distance_buffer.push_back(QPair<int,int>(err_code,value));
			else distance_buffer.push_back(QPair<int,int>(UnknownError,0));
			
			acc_data.clear();			
		}
	}
	else acc_data.clear();	
}


void LeuzeCommunicator::run()
{
	if (!socket)
	{
		freeze();
		emit error_msg("Bad pointer to Leuze Distance Meter object!");
		return;
	}
	if (!socket->isOpen())
	{
		freeze();
		emit error_msg("Socket to Leuze Distance Meter is closed!");
		return;
	}

	connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));

	is_freezed = false;
	is_running = true;

	/*is_running = true;
	while (is_running)
	{
		msleep(1);
	}*/
	exec();
}


void LeuzeCommunicator::timeClocked()
{
	
}

void LeuzeCommunicator::freeze()
{
	disconnect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = true;	

	emit frozen(is_freezed);
}

void LeuzeCommunicator::wake()
{
	connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = false;	

	emit frozen(is_freezed);
}



StepMotorCommunicator::StepMotorCommunicator(QTcpSocket *socket, QObject *parent)
{
	thread_id = thid++;	

	//COM_port = com_port;	
	this->socket = socket;
	connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));	
}

StepMotorCommunicator::~StepMotorCommunicator()
{
	
}

/*
void StepMotorCommunicator::setPort(QextSerialPort *com_port)
{
	COM_port = com_port; 
	connect(COM_port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}
*/
void StepMotorCommunicator::setSocket(QTcpSocket *socket)
{
	this->socket = socket;
	connect(socket, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}

void StepMotorCommunicator::sendRequestToCOM(QByteArray *arr)
{		
	socket->write(arr->data(), arr->size());	
}

void StepMotorCommunicator::toSend(QString cmd)
{
	QByteArray arr = cmd.toLocal8Bit();
	sendRequestToCOM(&arr);
}

void StepMotorCommunicator::onDataAvailable()
{
	QByteArray byte_data = socket->readAll();	

	if (!byte_data.isEmpty())
	{
		acc_data += byte_data;
		QStringList str_data = acc_data.split('*');
		int block_count = str_data.count();
		if (block_count > 2)
		{
			for (int i = 0; i < block_count-1; i++)
			{
				QString req_str = str_data.first();
				if (req_str == "E10") 
				{
					str_data.takeFirst();					
				}
				else if (req_str == "E15")
				{
					emit error_msg(tr("Step Motor Error: Bad RS-232!"));
					return;
				}
				else if (req_str == "E16")
				{
					emit error_msg(tr("Step Motor Error: Bad command!"));
					return;
				}
				else if (req_str == "E19")
				{
					emit error_msg(tr("Step Motor Error: Bad command parameter!"));
					return;
				}
			}
			acc_data = str_data.last();
		}
	}	
}

void StepMotorCommunicator::run()
{
	if (!socket)
	{
		freeze();
		emit error_msg("Bad pointer to socket of the Step Motor!");
		return;
	}
	if (!socket->isOpen())
	{
		freeze();
		emit error_msg("Socket to Step Motor is closed!");
		return;
	}

	//connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));

	is_freezed = false;
	is_running = true;

	exec();
}


void StepMotorCommunicator::freeze()
{
	//disconnect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = true;	

	emit frozen(is_freezed);
}

void StepMotorCommunicator::wake()
{
	//connect(clocker, SIGNAL(clock()), this, SLOT(timeClocked()));
	is_freezed = false;	

	emit frozen(is_freezed);
}
