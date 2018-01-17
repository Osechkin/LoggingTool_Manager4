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
	uint8_t	code;				// ��� ������ (� �������� ���� ������ ������������, ��������, CPMG_PRG � ��.)
	int tag;					// ����������, ������������ �� ������������� (��������, � �������� ���������� �������) 
	uint8_t channel;			// ��� ������ ������ (��������, ������ ������ ���, ������ ������, �����-������� � �.�.)
	QString str_value;			// ��������� ������������� ������ - ������������ ����������� ������������; ����� ���� ������ ""
	QVector<double> *value;
	QVector<uint8_t> *bad_data;	// ��������� ������������ ��� "�����������" ������ (��������� ��� ������ ������). 
								// ����� value ������ ���� ����� ����� bad_data. �������� bad_data: DATA_OK/BAD_DATA
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

	uint32_t uid;			// ���������� id ��� ����������� �������� ��������� � ����� ���������� ������ �� ����
	QString name;			// ��� �������/������ ������
	uint8_t comm_id;		// ��� �������/������ ������
	uint8_t type;			// "���" �������, ���������� ��� ��� ���� ��� UART-��������� ��� �������� (��������� ���������, �������� ��� �������������)
	
	QList<Field_Comm*> *fields;	// ������ 
	
	int elapsed_time;		// �����, ��������� � ������� �������� ������ ������� � ���������� ������
	int life_time;			// ������� ������, ���������� � ������� ������ ������� DeviceData � ������� �� �������� � ���������� ������,
							// �.�. "����� �����" �������
	bool is_sent;			// ��������� ���������� �� ������ ������� � ���������� ������ 
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

	uint8_t start_marker;			// ������ ������ ��������� ���������; ����� �������� id ���� ������������� ���������
	uint8_t w_addr;					// ������ ����������-����������� ��������� 
	uint8_t r_addr;					// ������ ����������-�������� ��������� 
	uint8_t id;						// ������������� ���������
	uint8_t pack_count;				// ����� ������� � ���������
	uint8_t pack_len;				// ����� ������� � ������
	uint8_t block_len;				// ����� ������ ����-��������, �� ������� �������� ������ � ������
	uint8_t err_count;				// ����� ����������������� ������ � ������ ����� 
	uint8_t crc;					// ����������� �����
	SmartArr srv_data;				// ��������� ������ (������ ��� ������ ��������� ���������, � ������� ������ ����� 
};



/* ��������� ����������� �� UART ���������, ������������ ������� �������� ��������� �� �����������:
 *
 * |-----------|	|-----------|	|----------|	...		|-----------|
 *   ���������		   ����� 1		  ����� 2				   ����� N
 *
 *
 *
 *  ��������� ��������� (����� MsgHeader):
 *
 *  1 ����	1 ����	1 ����	1 ����	1 ����	1 ����	1 ����	1 ����	   ( 4 ����� )
 *  |-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|----- ... -----|
 *	 MARK	 ADDR	  ID	 NPACK	 LPACK	LBLOCK	 NERRS	 CRC8		 RSBYTES
 *
 *	 MARK	- ��������� ������ ��������� ���������. ����� �������� ��� ��������� (��. MTYPE � ����� io_general.h)
 *	 ADDR	- ������ ����������-����������� � ����������-�������� ���������. ������: ���� 0-3 - ����� �����������, ���� 4-7 - ����� ���������
 *	 ID		- ����� ��������� (����� �� 0 �� 255, �.�. ����� ��������� �� ������ ���� ���������� � ������� ���� ������ ������� ���)
 *	 NPACK	- ����� ������� � ���������
 *	 LPACK	- ����� ������� � ���������
 *	 LBLOCK	- ����� ������, �� ������� ����������� ������ ��� �������� ����������� ����-��������
 *	 NERRS	- ����� ������, ������� ����� ���� ������������� � ������ ����� ����-��������
 *	 CRC8	- ����������� �����, ������� ����������� �� ������ � MARK �� NERRS
 *	 RSBYTES- ����������� �����, ��������������� ��� ���������� ������ � ��������� ���������. ������ ����� ����������� ���� = 4
 *
 *
 *
 *  ������ ��������� (����� MsgPacket):
 *
 *  1 ����  1 ����  1 ����        N ����           1 ����
 *  |-----| |-----| |-----| |-------...--------| |-----|
 *   MARK     ID      NUM           Data           CRC8
 *
 *  MARK	- ������ ������ ���������.
 *  ID		- ����� ��������� (����� �� 0 �� 255, �.�. ����� ��������� �� ������ ���� ���������� � ������� ���� ������ ������� ���)
 *  NUM		- ����� ������
 *  CRC8	- ����������� ����� �� ����� ������, ������� ����������� � ������� MARK ������������
 *
 *
 *  ����� ������������ ������, ������������ ����������� �� ������ ����-��������. 
 *  � ���������� �������������� ����� ������������ ����� ������������������ ��-������:
 *  
 *  LBLOCK ����   LBLOCK ����       ...    LBLOCK ����
 *  |----------|  |----------|             |----------|
 *  Packet Data   Packet Data              Packet Data
 *
 *  LBLOCK	- ����� ������, �� ������� ����������� ������ ��� �������� ����������� ����-�������� (��. ���� �������� ��������� MsgHeader)
 *
 *
 *  ��� ���� ��������� ������� �����:
 *
 *     M ����    K ����
 *  |---------| |-------|			M + K = LBLOCK
 *     DATA      RSBYTES
 *
 *  DATA	- ���������� ������. ����� M = LBLOCK - K = LBLOCK - 2*NERRS (��� NERRS ��. �������� MsgHeader ����)
 *  RSBYTES - ����������� �����, ��������������� ��� ���������� ������ � ������� ����� ���������. ����� K = 2*NERRS
 *
 *  
 *
 *
 *	�������� ��������� �� �������� �������. � ��� ���������� ������������ � ���� ���������. 
 *	� ���� ������ ��������� ��������� ��������� ���������:
 *	
 *	��������� ��������� (����� MsgHeader):
 *
 *  1 ����	1 ����	1 ����	1 ����	1 ����	1 ����	1 ����	1 ����	   ( 4 ����� )
 *  |-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|-----|	|----- ... -----|
 *	 MARK	 ADDR	  ID	 BYTE0	 BYTE1	 BYTE2	 BYTE3	 CRC8		 RSBYTES
 *
 *	 MARK	- ��������� ������ ��������� ���������. ����� �������� ��� ��������� (��. MTYPE � ����� io_general.h)
 *	 ADDR	- ������ ����������-����������� � ����������-�������� ���������. ������: ���� 0-3 - ����� �����������, ���� 4-7 - ����� ���������
 *	 ID		- ����� ��������� (����� �� 0 �� 255, �.�. ����� ��������� �� ������ ���� ���������� � ������� ���� ������ ������� ���)
 *	 BYTE0 - BYTE3	- ����� ������ � ���������
 *	 CRC8	- ����������� �����, ������� ����������� �� ������ � MARK �� NERRS
 *	 RSBYTES- ����������� �����, ��������������� ��� ���������� ������ � ��������� ���������. ������ ����� ����������� ���� = 4
 *
 *
 *
 * 
 *	��������� ��������� � ���������� ������� � ������ ������/�������� �� ����������� ������ �������� � ���� ��������� ����������:
 *	BYTE0:
 *	���� 0-7 - ��� ������������ ������� (��������, DATA_OK - ��. ������� � io_general.h )
 *
 *	BYTE1:
 *	��� 0 - ���������� �������: 						0/1 - �������� / ����������
 *	��� 1 - ���������� ����������: 						0/1 - �� ������ / ������
 *	��� 2 - ��������� ������������� ����: 				0/1 - ������� �������� proger_start() / ���������� �������� proger_stop()
 *	��� 3 - ��������� ��������� � ������������� ����: 	0/1 - ����������� / �����������
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
	void calcRecoveryPart();	// ��������� ����������� ����� ��� ���������

	uint8_t start_marker;		// ������ ������ ��������� ���������; ����� �������� id ���� ������������� ���������
	uint8_t wr_addr;			// ������ ����������-����������� ��������� � ����������-��������
	uint8_t session_id;			// ������������� ������ ���������
	uint8_t pack_count;			// ����� ������� � ���������
	uint8_t pack_len;			// ����� ������� � ������
	uint8_t block_len;			// ����� ������ ����-��������, �� ������� �������� ������ � ������
	uint8_t err_count;			// ����� ����������������� ������ � ������ ����� 
	uint8_t crc;				// ����������� ����� crc8, ����������� �� ������ � start_marker �� err_count
	SmartArr rec_data;			// ����������� ����� ������� ����-��������
	SmartArr srv_data;			// ��������� ������ (������ ��� ������ ��������� ���������, � ������� ������ ����� 
								// pack_count, pack_len, block_len � err_count ������������ ������ srv_data)

	SmartArr byte_array;		// �������� ����� ������������ ��������� ���������

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

	// ������� pushData "��������" �� ������� arr ���-�� ����, ������ data_len ������� � ������� start_pos. 
	// ��� ����, start_pos ������������ � ������� start_pos + data_len. ������ arr �� ����������.
	//void pushData(SmartArr arr, uint16_t &start_pos);
	void pushData(SmartArr data, uint16_t &pos);
	void pushData(uint8_t *arr, uint16_t arr_len, uint16_t &start_pos);
	// setData() ���������� ������ (byte_array, data_array, block_map, rec_data) �� ������ dst � ������� ����� 
	// ���������� false, ���� ���������� ������ �� ������������� ���������� ������-��������� �� ����� � ��. ���������� �� MsgHeader
	bool setData(MsgPacket *src);

private:	
	void create_byte_arr();
	uint8_t calcCRC();
	void calcRecoveryPart(uint8_t *arr, uint16_t pos, uint8_t num_block);
	// ������� calcRecoveryPart ��������� ������ ����������� ���� ��� ������� arr, ������� � ������� pos.
	// ����� ������ (� ������), ��� ������� ����������� ������ ����������� ����, ����� block_len - 2*errs_count,
	// ��������� ���������� � ������ rec_data � ������� 2*errs_count*num_block � ���������� 2*errs_count ����.
	// ��������� ������ (num_blocks) ���������� � 0.
	
	uint8_t start_marker;		// ������ ������ ������
	uint8_t session_id;			// id ������
	uint8_t packet_number;		// ����� ������
	uint8_t crc;

	uint8_t block_len;			// ����� ������ ����-��������
	uint8_t errs_count;			// ���-�� ������������ ������ � ������ ����� ����-��������
	uint8_t data_len;			// ����� ������, ������������ � ������. data_len = (packet_len \ block_len)*block_len (� ������ ��������� ������ ������, ������������ 4 �����)
	uint8_t packet_len;			// ����� ������. 
	uint8_t block_count;		// ���-�� ������ ����-�������� � ������

	MsgHeader *msg_header;
	GF_Data *gf_data;
	
	SmartArr data_arr;			// �������� ������, ��������� � ����� (��� ����� � ����������� ���� ������ ����-��������)
	SmartArr byte_arr;			// ������� � �������� ����� ������, �������������� ����� �������������� ������ ������� �������� ����������� ����-��������
	
	SmartArr block_map;			// ������-����� ������: 
	SmartArr rec_data;			// ��������� ������ ��� �������� ������ ����-��������
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
	
	bool interleave_state;	// ������� ������������ ������ � �������

	Priority priority;
	IOStatus io_status;		// ���� ���������� ��������� �������� ��� ������ ��������� (����� ����� ��� ������/�������� �������������� ���������)	
	bool storage_status;	// ����, ������������ ���� �� ��������� �������� � ��������� ���������
	
	GF_Data *gf_data;	
};

/*
// ���������, ��������������� ��� �������� ������ �����������
struct Scheduler
{
	Scheduler() 
	{ 
		id = 0; 
		started = false; 
	}

	uint8_t id;					// id ������ ��������
	QList<uint16_t> delays;		// �������� (� [ms]) �������� ������ �� ��������� ���������, ������ � �.�. 
	QList<uint16_t> report;		// ����� � ���������� �������� � ���������� "�������� ��������� - ����� �� ��������" 
	bool started;				// ���� ��������� ������� Scheduler (��������/�� ��������)
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
	
	uint32_t uid;			// ���������� ����� ������� � �����������
	uint8_t type;			// "���" �������, ���������� ��� ��� ���� ��� ��������� (�������, �������� ������, ���������)
	
	int elapsed_time;		// �����, ��������� � ������� �������� ������ ������� � ���������� ������
	int life_time;			// ������� ������, ���������� � ������� ������ ������� DepthMeterData � ������� �� ���������, �.�. "����� �����" �������	

	bool is_sent;			// ��������� �������� ������� � ���������� �������

	double data;			// ��������� ���������
};

#endif // MESSAGE_CLASS_H



