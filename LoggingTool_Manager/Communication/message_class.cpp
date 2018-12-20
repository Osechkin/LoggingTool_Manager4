#include "message_class.h"
#include <QDebug>


uint8_t Crc8(uint8_t *pcBlock, uint16_t len)
{
	uint8_t crc = 0xFF; //исходное значение
	uint16_t i;
	
	if (len < 0) return crc;
	for(i = 0; i < len; i++) crc ^= pcBlock[i];

	return crc;
}

uint16_t Crc16(uint8_t *pcBlock, uint16_t len)
{
    uint16_t crc = 0xFFFF; //исходное значение
    uint16_t i;

    while( len-- )
    {
        crc ^= *pcBlock++ << 8;
        for( i = 0; i < 8; i++ )
            crc = crc & 0x8000 ? ( crc << 1 ) ^ 0x1021 : crc << 1; // Gx = 0x1021 = 1000000100001
    }

    return crc;//вернуть остаток
}


SmartArr::SmartArr()
{
	data = 0;
    //data = new uint8_t;
    len = 0;
}

SmartArr::SmartArr(int n)
{
    //data = new uint8_t[n];
    //memset(data, 0x0, n*sizeof(uint8_t));
	
	data = boost::make_shared<uint8_t[]>(n);
	memset(data.get(), 0x0, n*sizeof(uint8_t));
    len = n;
}

SmartArr::~SmartArr()
{
    //destroy();
}

void SmartArr::destroy()
{
	//delete[] data;
	len = 0;
}

void SmartArr::alloc(int n)
{    
    /*data = new uint8_t[n];	
    memset(data, 0x0, n*sizeof(uint8_t));
    len = n;*/
	
	data = boost::make_shared<uint8_t[]>(n);
	memset(data.get(), 0x0, n*sizeof(uint8_t));
	len = n;
}

void SmartArr::allocf(int n, uint8_t val)
{    
	alloc(n);
	fill(val);
}

void SmartArr::allocf(int n, uint16_t val)
{    
    alloc(n);
    fill(val);
}

void SmartArr::allocf(int n, uint32_t val)
{
    alloc(n);
    fill(val);
}

void SmartArr::allocf(SmartArr arr)
{
	if (len > 0) realloc(arr.len);
	else alloc(arr.len);
    //memcpy(data, arr.data, arr.len);
	memcpy(data.get(), arr.data.get(), arr.len);
}

void SmartArr::realloc(int n)
{
	//destroy();
	alloc(n);
}

int SmartArr::fill(uint8_t val)
{
	if (len > 0) data[0] = val;

	return 0;
}

int SmartArr::fill(uint16_t val)
{
    if (len == sizeof(uint8_t))
    {
        data[0] = (uint8_t)(val & 0xff);
    }
    else if (len >= sizeof(uint16_t))
    {
        data[0] = (uint8_t)(val & 0xff);
        data[1] = (uint8_t)((val >> 8) & 0xff);
    }

    return 0;
}

int SmartArr::fill(uint32_t val)
{
    if (len == sizeof(uint8_t))
    {
        data[0] = (uint8_t)(val & 0xff);
    }
    else if (len == sizeof(uint16_t))
    {
        data[0] = (uint8_t)(val & 0xff);
        data[1] = (uint8_t)((val >> 8) & 0xff);
    }
    else if (len == 3)
    {
        data[0] = (uint8_t)(val & 0xff);
        data[1] = (uint8_t)((val >> 8) & 0xff);
        data[2] = (uint8_t)((val >> 16) & 0xff);
    }
    else if (len >= sizeof(uint32_t))
    {
        data[0] = (uint8_t)(val & 0xff);
        data[1] = (uint8_t)((val >> 8) & 0xff);
        data[2] = (uint8_t)((val >> 16) & 0xff);
        data[3] = (uint8_t)((val >> 24) & 0xff);
    }

    return 0;
}

int SmartArr::filla(uint8_t val, uint16_t n)
{
	uint16_t sz = n;
	if (n > len) sz = len;
	//memset(data, val, sizeof(uint8_t)*sz);
	memset(data.get(), val, sizeof(uint8_t)*sz);

	return 0;
}

void SmartArr::concat(uint8_t *arr, uint16_t n)
{
    SmartArr new_data(len+n);
    //memcpy(new_data.data, data, len*sizeof(uint8_t));
    //memcpy(new_data.data+len*sizeof(uint8_t), arr, n*sizeof(uint8_t));
	memcpy(new_data.data.get(), data.get(), len*sizeof(uint8_t));
	memcpy(new_data.data.get()+len*sizeof(uint8_t), arr, n*sizeof(uint8_t));

	destroy();
	allocf(new_data);
	new_data.destroy();
}

void SmartArr::concat(SmartArr arr)
{
    //uint8_t *dat = arr.data;
	uint8_t *dat = arr.data.get();
    uint16_t size = arr.len;

    concat(dat, size);
}


DeviceData::DeviceData()
{
	uid = 0;
	name = "no_name";
	comm_id = NONE;
	type = MTYPE_SERVICE;
	fields = new QList<Field_Comm*>;
	elapsed_time = 0;
	life_time = 0;
	is_sent = false;
}

DeviceData::DeviceData(uint8_t id, QString _name)
{
	uid = 0;
	name = _name;
	comm_id = id;
	elapsed_time = 0;
	life_time = 0;
	is_sent = false;

	fields = new QList<Field_Comm*>;
	fields->clear();

	switch (id)
	{	
	case NMRTOOL_IS_READY:
	case NMRTOOL_ISNOT_READY:
	//case DATA_STATE_OK:
	//case DATA_STATE_FAIL:
	case NONE:
		{
			type = MTYPE_SERVICE;
			break;
		}
	case DATA_OK:
	case BAD_DATA:
	case HEADER_OK:
	case GET_RELAX_DATA:
	case NMRTOOL_CONNECT:
	case NMRTOOL_CONNECT_DEF:
	case NMRTOOL_START:
	case NMRTOOL_STOP:
		{
			type = MTYPE_SHORT;
			break;
		}
	case GET_ECHO_DATA:
		{
			type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Echo Number", "1", Field_Comm::UINT16);
			fields->append(field1);
			break;
		}
	case GET_SPEC_DATA:
		{
			type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Spectrum Number", "1", Field_Comm::UINT16);
			fields->append(field1);
			break;
		}
	case GET_ADC_DATA:
		{
			type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Echo Number", "1", Field_Comm::UINT16);
			fields->append(field1);
			break;
		}		
	case LOG_TOOL_SETTINGS:
		{
			type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Logging tool settings", "1", Field_Comm::UINT32);
			fields->append(field1);
			break;
		}
	}
}

DeviceData::DeviceData(uint8_t id, QString _name, uint32_t _uid)
{
	//DeviceData(id, _name);
	uid = _uid;
	name = _name;
	comm_id = id;
	elapsed_time = 0;
	life_time = 0;
	is_sent = false;

	fields = new QList<Field_Comm*>;	
	linkCmdData(this);	
}

DeviceData::~DeviceData()
{
	for (int i = 0; i < fields->size(); i++)
	{
		delete fields->at(i);
	}	
	fields->clear();
	delete fields;
}

void DeviceData::linkCmdData(DeviceData *dev_data)
{
	switch (dev_data->comm_id)
	{	
	case NMRTOOL_IS_READY:
	case NMRTOOL_ISNOT_READY:
		//case DATA_STATE_OK:
		//case DATA_STATE_FAIL:
	case NONE:
		{
			dev_data->type = MTYPE_SERVICE;
			break;
		}
	case DATA_OK:
	case BAD_DATA:
	case HEADER_OK:
	case GET_RELAX_DATA:
	case NMRTOOL_CONNECT:
	case NMRTOOL_CONNECT_DEF:
	case NMRTOOL_START:
	case NMRTOOL_STOP:
	case SDSP_REQUEST_88:
	case SDSP_REQUEST_C8:
	case PRESS_UNIT_OPEN:
	case PRESS_UNIT_CLOSE:
	case PRESS_UNIT_STOP:
		{
			dev_data->type = MTYPE_SHORT;
			break;
		}
	case GET_ECHO_DATA:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Echo Number", "1", Field_Comm::UINT16);
			dev_data->fields->append(field1);
			break;
		}
	case GET_SPEC_DATA:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Spectrum Number", "1", Field_Comm::UINT16);
			dev_data->fields->append(field1);
			break;
		}
	case GET_ADC_DATA:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Echo Number", "1", Field_Comm::UINT16);
			dev_data->fields->append(field1);
			break;
		}	
	case DATA_PROC:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("FPGA Program", "1", Field_Comm::UINT8);
			Field_Comm *field2 = new Field_Comm("Processing Program", "2", Field_Comm::UINT8);
			dev_data->fields->append(field1);
			dev_data->fields->append(field2);
			break;
		}
	case SET_WIN_PARAMS:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("WinFunc Parameters", "1", Field_Comm::FLOAT);
			dev_data->fields->append(field1);
			break;
		}
	case SET_COMM_PARAMS:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Communication Parameters", "1", Field_Comm::UINT8);
			dev_data->fields->append(field1);
			break;
		}
	case SDSP_DATA:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("SDSP parameters", "1", Field_Comm::UINT8);
			dev_data->fields->append(field1);
			break;
		}
	case LOG_TOOL_SETTINGS:
		{
			dev_data->type = MTYPE_MULTYPACK;
			Field_Comm *field1 = new Field_Comm("Logging tool settings", "1", Field_Comm::UINT32);
			dev_data->fields->append(field1);
			break;
		}
	default: break;
	}
}


MsgHeader::MsgHeader(GF_Data *_gf_data)
{
	gf_data = _gf_data;
	srv_data.alloc(SRV_DATA_LEN);
	rec_data.alloc(HEAD_REC_LEN);
	byte_array.alloc(HEADER_LEN);

	setMsgType(MTYPE_SERVICE);
	setWRAddress(0,0);
	setSessionId(0);	
	setPackCount(0);
	setPackLen(0);
	setBlockLen(0);
	setErrsCount(0);
	
	setSrvData(srv_data);

	create_byte_arr();	
}

MsgHeader::MsgHeader(MsgHeaderInfo *_hd_info, GF_Data *_gf_data)
{	
	gf_data = _gf_data;
	byte_array.alloc(HEADER_LEN);
	rec_data.alloc(HEAD_REC_LEN);
	srv_data.alloc(SRV_DATA_LEN);

	start_marker = _hd_info->start_marker;
	wr_addr = ((_hd_info->r_addr & 0x0F) << 4) | (_hd_info->w_addr & 0x0F);	
	session_id = _hd_info->id;
	pack_count = 0;
	pack_len = 0;
	block_len = 0;
	err_count = 0;

	if (start_marker == MTYPE_SERVICE) setSrvData(_hd_info->srv_data);
	else if (start_marker == MTYPE_SHORT) setShortData(_hd_info->srv_data);
	else if (start_marker == MTYPE_MULTYPACK)
	{
		pack_count = _hd_info->pack_count;
		pack_len = _hd_info->pack_len;
		block_len = _hd_info->block_len;
		err_count = _hd_info->err_count;
	}	
	create_byte_arr();
}

MsgHeader::~MsgHeader()
{
	/*srv_data.destroy();
	rec_data.destroy();
	byte_array.destroy();*/
}

void MsgHeader::setInfoData(MsgHeaderInfo _info, GF_Data *_gf_data)
{
	gf_data = _gf_data;

	setMsgType((MTYPE)_info.start_marker);
	setWRAddress(_info.w_addr, _info.r_addr);
	if (_info.id == 0) generateSessionId();
	else setSessionId(_info.id);

	srv_data.alloc(SRV_DATA_LEN);
	if (getStartMarker() == MTYPE_SERVICE || getStartMarker() == MTYPE_SHORT)
	{
		setSrvData(_info.srv_data);
	}
	else if (getStartMarker() == MTYPE_MULTYPACK)
	{
		setPackCount(_info.pack_count);
		setPackLen(_info.pack_len);
		setBlockLen(_info.block_len);
		setErrsCount(_info.err_count);
	}

	rec_data.alloc(HEAD_REC_LEN);

	create_byte_arr();
}

void MsgHeader::fillAllData(MsgHeaderInfo *_info)
{
	start_marker = _info->start_marker;
	wr_addr	= (_info->w_addr & 0x0F) | ((_info->r_addr & 0x0F) << 4);
	session_id = _info->id;
	pack_count = _info->pack_count;
	pack_len = _info->pack_len;
	block_len = _info->block_len;
	err_count = _info->err_count;
	crc	= _info->crc;

	//memcpy(srv_data.data, _info->srv_data.data, SRV_DATA_LEN);
	//memset(rec_data.data, 0x0, HEAD_REC_LEN);
	memcpy(srv_data.data.get(), _info->srv_data.data.get(), SRV_DATA_LEN);
	memset(rec_data.data.get(), 0x0, HEAD_REC_LEN);
}

void MsgHeader::clearMsgHeader()
{
	start_marker = 0;
	wr_addr = 0;
	session_id = 0;
	pack_count = 0;
	pack_len = 0;
	block_len = 0;
	err_count = 0;
	crc = 0;
	rec_data.filla(0, HEAD_REC_LEN);
	srv_data.filla(0, SRV_DATA_LEN);
	byte_array.filla(0, HEADER_LEN);	
}

void MsgHeader::copyMsgHeader(MsgHeader *dst_hdr)
{
	/*
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
	*/	

	dst_hdr->setWRAddress(this->getWriter(), this->getReader());
	dst_hdr->setSessionId(this->getSessionId());
	dst_hdr->setPackCount(this->getPackCount());
	dst_hdr->setPackLen(this->getPackLen());
	dst_hdr->setBlockLen(this->getBlockLen());
	dst_hdr->setErrsCount(this->getErrsCount());	
	
	dst_hdr->setCRC(this->getCRC());
	dst_hdr->setMsgType(this->getMsgType());
	dst_hdr->setSrvData(this->getSrvData());	
}

void MsgHeader::setMsgType(MTYPE type)
{
	start_marker = (uint8_t)type;
	create_byte_arr();
}

void MsgHeader::setWRAddress(uint8_t w, uint8_t r)
{
	wr_addr = ((r & 0x0F) << 4) | (w & 0x0F);
	create_byte_arr();
}

void MsgHeader::setReader(uint8_t r)
{
	wr_addr = (wr_addr & 0x0F) | ((r & 0x0F) << 4);
	create_byte_arr();
}

void MsgHeader::setWriter(uint8_t w)
{
	wr_addr = ((wr_addr & 0xF0) | (w & 0x0F));
	create_byte_arr();
}

uint8_t MsgHeader::generateSessionId()
{
	static uint8_t msg_id = 0;
	
	return ++msg_id;
}

void MsgHeader::setSessionId(uint8_t val)
{
	session_id = val;
	create_byte_arr();
}

bool MsgHeader::setSrvData(SmartArr &arr)
{
	if (arr.len > SRV_DATA_LEN) return false;
	//memset(srv_data.data, 0x0, SRV_DATA_LEN*sizeof(uint8_t));
	//memcpy(srv_data.data, arr.data, sizeof(uint8_t)*arr.len);
	memset(srv_data.data.get(), 0x0, SRV_DATA_LEN*sizeof(uint8_t));
	memcpy(srv_data.data.get(), arr.data.get(), sizeof(uint8_t)*arr.len);
	create_byte_arr();

	return true;
}

bool MsgHeader::setShortData(SmartArr &arr)
{
	return setSrvData(arr);
}

void MsgHeader::setPackCount(uint8_t val)
{
	pack_count = val;
	create_byte_arr();
}

void MsgHeader::setPackLen(uint8_t val)
{
	pack_len = val;
	create_byte_arr();
}

void MsgHeader::setBlockLen(uint8_t val)
{
	block_len = val;
	create_byte_arr();
}

void MsgHeader::setErrsCount(uint8_t val)
{
	err_count = val;
	create_byte_arr();
}

void MsgHeader::setCRC(uint8_t _crc)
{
	crc = _crc;
}

uint8_t MsgHeader::getWriter()
{
	return wr_addr & 0x0F;
}

uint8_t MsgHeader::getReader()
{
	return (wr_addr & 0xF0) >> 4;
}

uint8_t MsgHeader::calcCRC()
{
	//uint8_t _crc = Crc8(byte_array.data, HEAD_INFO_LEN-1);
	uint8_t _crc = Crc8(byte_array.data.get(), HEAD_INFO_LEN-1);
	return (uint8_t)(_crc & 0xFF);	
}

void MsgHeader::calcRecoveryPart()
{	
	GFPoly *a = GFPoly_alloc();
	GFPoly_init(HEAD_INFO_LEN-1, a);
	//memcpy(a->data, byte_array.data, HEAD_INFO_LEN*sizeof(uint8_t));
	memcpy(a->data, byte_array.data.get(), HEAD_INFO_LEN*sizeof(uint8_t));
	GFPoly_self_inv(a);

	GFPoly *r = GFPoly_alloc();	
	GF *gf = gf_data->gf;
	int index = gf_data->index_hdr;
	GFPoly *g = gf_data->gf_polys[index];

	// for debugging !
	//uint8_t bytes[HEADER_LEN];
	//memcpy(&bytes[0], byte_array.data, (HEADER_LEN)*sizeof(uint8_t));
			
	int res = RS_encode(a, g, gf, r); 
	if (res != E_RS_OK) 
	{
		GFPoly_destroy(a);
		GFPoly_destroy(r);
		free(a);
		free(r);

		return;
	}
	
	//GFPoly_self_inv(r);
	//memcpy(rec_data.data, r->data, sizeof(uint8_t)*HEAD_REC_LEN);
	uint16_t r_pow = r->power;
	if (r_pow > HEAD_REC_LEN) r_pow = HEAD_REC_LEN;
	for (int i = 0; i <= r_pow; i++) rec_data.data[i] = r->data[r_pow-i];
	GFPoly_destroy(a);
	GFPoly_destroy(r);
	free(a);
	free(r);
}

void MsgHeader::create_byte_arr()
{
	byte_array.data[0] = start_marker;
	byte_array.data[1] = wr_addr;
	byte_array.data[2] = session_id;
	if (start_marker == MTYPE_SERVICE || start_marker == MTYPE_SHORT)
	{
		//memcpy(byte_array.data + SRV_DATA_PTR, srv_data.data, sizeof(uint8_t)*SRV_DATA_LEN);
		memcpy(byte_array.data.get() + SRV_DATA_PTR, srv_data.data.get(), sizeof(uint8_t)*SRV_DATA_LEN);
	}
	else 
	{
		byte_array.data[3] = pack_count;
		byte_array.data[4] = pack_len;
		byte_array.data[5] = block_len;
		byte_array.data[6] = err_count;
	}

	uint8_t sbytes[HEAD_REC_LEN];
	//memcpy(&sbytes[0], srv_data.data, HEAD_REC_LEN*sizeof(uint8_t));
	memcpy(&sbytes[0], srv_data.data.get(), HEAD_REC_LEN*sizeof(uint8_t));
	
	crc = calcCRC();
	byte_array.data[7] = crc;
	calcRecoveryPart();

	//memcpy(byte_array.data+HEAD_INFO_LEN, rec_data.data, sizeof(uint8_t)*HEAD_REC_LEN);
	memcpy(byte_array.data.get()+HEAD_INFO_LEN, rec_data.data.get(), sizeof(uint8_t)*HEAD_REC_LEN);

	uint8_t bytes[HEADER_LEN];
	memcpy(&bytes[0], byte_array.data.get(), HEADER_LEN*sizeof(uint8_t));
	int tt = 0;
}



MsgPacket::MsgPacket(uint8_t _pack_num, MsgHeader *_msg_header, bool interleaved)
{
	msg_header = _msg_header;
	gf_data = _msg_header->getGFData();
	
	if (interleaved) start_marker = MTYPE_PACKET_ILV;
	else start_marker = MTYPE_PACKET;

	session_id = msg_header->getSessionId();
	packet_number = _pack_num;
	block_len = msg_header->getBlockLen();
	errs_count = msg_header->getErrsCount();
			
	//packet_len = ((msg_header->getPackLen())/block_len)*block_len;
	packet_len = msg_header->getPackLen();
	//data_len = (block_len-2*errs_count)*(packet_len/block_len)-(PACK_INFO_LEN+1);
	data_len = (block_len-2*errs_count)*(packet_len/block_len);
	
	block_count = packet_len/block_len;
	rec_data.alloc(2*errs_count*block_count);
	//data_arr.alloc(data_len+4);		// +4 - плюс место для служебных данных пакета: стартовый маркер, id сообщения, номер пакета и crc
	data_arr.alloc(data_len);
	byte_arr.alloc(packet_len);
	block_map.allocf(block_count, (uint8_t)NAN_DATA);	
	//create_byte_arr();
}

MsgPacket::~MsgPacket()
{
	data_arr.destroy();
	byte_arr.destroy();
	rec_data.destroy();
	block_map.destroy();
}


void MsgPacket::pushData(SmartArr arr, uint16_t &start_pos)
{
	data_arr.data[0] = start_marker;
	data_arr.data[1] = session_id;
	data_arr.data[2] = packet_number;	

	uint8_t block_cnt = packet_len/block_len;									// кол-во блоков в пакете
	uint8_t data_block_len = block_len - 2*errs_count;							// длина данных в одном блоке
	uint16_t full_pdata_len = data_block_len * block_cnt - PACK_INFO_LEN - 1;	// длина всех данных в пакете
	uint16_t rem_data_len = arr.len - start_pos;								// длина данных от позиции pos и до конца массива arr

	uint16_t ex_data = 0;														// число нулей, которыми нужно дозабить пакет, если
	if (rem_data_len <= full_pdata_len) ex_data = full_pdata_len - rem_data_len;// длина данных в arr меньше длины чистых данных в пакете

	uint16_t dlen = full_pdata_len;												// длина данных, которые будут помещены в пакет из data
	if (full_pdata_len > rem_data_len) dlen = rem_data_len;

	//memcpy(data_arr.data + PACK_INFO_LEN, arr.data + start_pos, sizeof(uint8_t)*dlen);
	//if (ex_data > 0) memset(data_arr.data + dlen + PACK_INFO_LEN, 0x0, sizeof(uint8_t)*ex_data);		// дозаполнить нулями
	memcpy(data_arr.data.get() + PACK_INFO_LEN, arr.data.get() + start_pos, sizeof(uint8_t)*dlen);
	if (ex_data > 0) memset(data_arr.data.get() + dlen + PACK_INFO_LEN, 0x0, sizeof(uint8_t)*ex_data);	// дозаполнить нулями

	data_arr.data[data_len-1] = calcCRC();
	
	create_byte_arr();
		
	start_pos += (uint16_t)data_len;
}

void MsgPacket::pushData(uint8_t *arr, uint16_t arr_len, uint16_t &start_pos)
{
	//uint8_t test_arr[20];
	//memcpy(&test_arr[0], data_arr.data, (block_len * (packet_len/block_len))*sizeof(uint8_t));

	data_arr.data[0] = start_marker;
	data_arr.data[1] = session_id;
	data_arr.data[2] = packet_number;	

	//memcpy(&test_arr[0], data_arr.data, (block_len * (packet_len/block_len))*sizeof(uint8_t));

	uint8_t block_cnt = packet_len/block_len;									// кол-во блоков в пакете
	uint8_t data_block_len = block_len - 2*errs_count;							// длина данных в одном блоке
	uint16_t full_pdata_len = data_block_len * block_cnt - PACK_INFO_LEN - 1;	// длина всех данных в пакете
	uint16_t rem_data_len = arr_len - start_pos;								// длина данных от позиции pos и до конца массива arr

	uint16_t ex_data = 0;														// число нулей, которыми нужно дозабить пакет, если
	if (rem_data_len <= full_pdata_len) ex_data = full_pdata_len - rem_data_len;// длина данных в arr меньше длины чистых данных в пакете

	uint16_t dlen = full_pdata_len;												// длина данных, которые будут помещены в пакет из data
	if (full_pdata_len > rem_data_len) dlen = rem_data_len;

	//memcpy(data_arr.data + PACK_INFO_LEN, arr + start_pos, sizeof(uint8_t)*dlen);
	//if (ex_data > 0) memset(data_arr.data + dlen + PACK_INFO_LEN, 0x0, sizeof(uint8_t)*ex_data);		// дозаполнить нулями
	memcpy(data_arr.data.get() + PACK_INFO_LEN, arr + start_pos, sizeof(uint8_t)*dlen);
	if (ex_data > 0) memset(data_arr.data.get() + dlen + PACK_INFO_LEN, 0x0, sizeof(uint8_t)*ex_data);	// дозаполнить нулями

	data_arr.data[data_len-1] = calcCRC();

	//memcpy(&test_arr[0], data_arr.data, (block_len * (packet_len/block_len))*sizeof(uint8_t));	

	create_byte_arr();

	//memcpy(&test_arr[0], byte_arr.data, (block_len * (packet_len/block_len))*sizeof(uint8_t));	

	start_pos += (uint16_t)data_len-PACK_INFO_LEN-1;
}

/*void MsgPacket::pushData(SmartArr data, uint16_t &pos)
{
	uint8_t arr[MAX_PACK_LEN];	
	memset(data.data, 0x0, sizeof(uint8_t)*packet_len);

	uint16_t dpos = pos;														// позиция, с которой забираются данные из data
	
	uint8_t block_cnt = packet_len/block_len;									// кол-во блоков в пакете
	uint8_t data_block_len = block_len - 2*errs_count;							// длина данных в одном блоке
	uint16_t full_pdata_len = data_block_len * block_cnt - PACK_INFO_LEN - 1;	// длина всех данных в пакете
	uint16_t rem_data_len = data_len - dpos;									// длина данных от позиции pos и до конца массива data

	uint16_t ex_data = 0;														// число нулей, которыми нужно дозабить пакет, если
	if (rem_data_len <= full_pdata_len) ex_data = full_pdata_len - rem_data_len;// длина данных в data меньше длины чистых данных в пакете

	uint16_t dlen = full_pdata_len;												// длина данных, которые будут помещены в пакет из data
	if (full_pdata_len > rem_data_len) dlen = rem_data_len;

	uint8_t tdata[MAX_PACK_LEN];												// массив данных для записи в пакет (с ex_data доп. нулей - при необходимости)
	tdata[0] = start_marker;
	tdata[1] = session_id;
	tdata[2] = packet_number;
	memcpy(&tdata[PACK_INFO_LEN], data.data + dpos, sizeof(uint8_t)*dlen);
	if (ex_data > 0) memset(&tdata[dlen + PACK_INFO_LEN], 0x0, sizeof(uint8_t)*ex_data);		// дозаполнить нулями
	uint8_t tdata_len = data_block_len * block_cnt;								// длина массива данных tdata
	uint8_t crc = Crc8(&tdata[0], tdata_len-1);									// контрольная сумма по данным, помещаемым в пакет
	tdata[tdata_len - 1] = crc;

	uint8_t apos = 0;															// текущая позиция данных в массиве arr
	uint8_t tpos = 0;															// текущая позиция данных в массиве tdata
	uint8_t rec_data[2*MAX_REC_ERRS];
	uint16_t rec_len = 2 * errs_count;
	/*int i = 0;
	while (block_cnt-- > 0)
	{
		//uint8_t ready_arr[18];
		memcpy(&arr[apos], &tdata[tpos], data_block_len*sizeof(uint8_t));
		//int i; for (i = 0; i < data_block_len; i++) ready_arr[i] = tdata[tpos+i];
		calcRecoveryPart(&arr[apos], &rec_data[0], _msg_pack, _gf_data);
		memcpy(&arr[apos + data_block_len], &rec_data[0], rec_len*sizeof(uint8_t));

		apos += block_len;
		tpos += data_block_len;
	}*/

	/*create_byte_arr();

	memcpy(&_msg_pack->data[0], &arr[0], pack_len*sizeof(uint8_t));
	_msg_pack->data_len = pack_len - PACK_SRV_LEN;
	_msg_pack->crc = crc;

	*pos = dpos + dlen;
}*/

bool MsgPacket::setData(MsgPacket *src)
{
	MsgHeader *src_header = src->getMsgHeader();
	if (!src_header) return false;

	if (msg_header->getPackLen() != src_header->getPackLen()) return false;
	if (msg_header->getBlockLen() != src_header->getBlockLen()) return false;
	if (msg_header->getErrsCount() != src_header->getErrsCount()) return false;

	block_map.destroy();
	rec_data.destroy();
	data_arr.destroy();
	byte_arr.destroy();

	block_map.alloc(src->getBlockMap().len);
	rec_data.alloc(src->getRecData().len);
	data_arr.alloc(src->getDataArray().len);
	byte_arr.alloc(src->getByteArray().len);
	
	//memcpy(block_map.data, src->getBlockMap().data, block_map.len*sizeof(uint8_t));
	//memcpy(rec_data.data, src->getRecData().data, rec_data.len*sizeof(uint8_t));
	//memcpy(data_arr.data, src->getDataArray().data, data_arr.len*sizeof(uint8_t));
	//memcpy(byte_arr.data, src->getByteArray().data, byte_arr.len*sizeof(uint8_t));
	memcpy(block_map.data.get(), src->getBlockMap().data.get(), block_map.len*sizeof(uint8_t));
	memcpy(rec_data.data.get(), src->getRecData().data.get(), rec_data.len*sizeof(uint8_t));
	memcpy(data_arr.data.get(), src->getDataArray().data.get(), data_arr.len*sizeof(uint8_t));
	memcpy(byte_arr.data.get(), src->getByteArray().data.get(), byte_arr.len*sizeof(uint8_t));
	
	return true;
}

uint8_t MsgPacket::calcCRC()
{
	//uint8_t _crc = Crc8(data_arr.data, (uint16_t)(data_len+3));
	//uint8_t _crc = Crc8(data_arr.data, (uint16_t)(data_len-1));
	uint8_t _crc = Crc8(data_arr.data.get(), (uint16_t)(data_len-1));
	return (uint8_t)(_crc & 0xFF);
}

void MsgPacket::calcRecoveryPart(uint8_t *arr, uint16_t pos, uint8_t num_block)
{
	uint8_t test_arr[10];

	GFPoly *a = GFPoly_alloc();
	uint8_t len = block_len-2*errs_count;
	GFPoly_init(len-1, a);
	memcpy(a->data, arr+pos, len*sizeof(uint8_t));
	//memcpy(&test_arr[0], a->data, len*sizeof(uint8_t));
	GFPoly_self_inv(a);
	//memcpy(&test_arr[0], a->data, len*sizeof(uint8_t));

	GFPoly *r = GFPoly_alloc();	
	GF *gf = gf_data->gf;
	int index = gf_data->index_body;
	GFPoly *g = gf_data->gf_polys[index];

	int res = RS_encode(a, g, gf, r); 
	if (res != E_RS_OK) 
	{
		GFPoly_destroy(a);
		GFPoly_destroy(r);
		free(a);
		free(r);
	}

	GFPoly_self_inv(r);
	//memcpy(rec_data.data + 2*errs_count*num_block, r->data, 2*errs_count*sizeof(uint8_t));	
	memcpy(rec_data.data.get() + 2*errs_count*num_block, r->data, 2*errs_count*sizeof(uint8_t));	

	GFPoly_destroy(a);
	GFPoly_destroy(r);
	free(a);
	free(r);
}

void MsgPacket::create_byte_arr()
{	
	uint8_t test_arr[20];
	uint16_t pack_data_len = block_len - 2*errs_count;
	for (uint8_t i = 0; i < block_count; i++)
	{
		uint16_t pos = (uint16_t)(i*pack_data_len);
		//calcRecoveryPart(data_arr.data, pos, i);
		//memcpy(byte_arr.data + i*block_len, data_arr.data + pos, sizeof(uint8_t)*pack_data_len);		
		//memcpy(byte_arr.data + i*block_len+pack_data_len, rec_data.data + i*2*errs_count, sizeof(uint8_t)*2*errs_count);		
		calcRecoveryPart(data_arr.data.get(), pos, i);
		memcpy(&test_arr[0], rec_data.data.get() + i*2*errs_count, sizeof(uint8_t)*2*errs_count);
		memcpy(byte_arr.data.get() + i*block_len, data_arr.data.get() + pos, sizeof(uint8_t)*pack_data_len);		
		memcpy(byte_arr.data.get() + i*block_len+pack_data_len, rec_data.data.get() + i*2*errs_count, sizeof(uint8_t)*2*errs_count);
	}
}


COM_Message::COM_Message(GF_Data *_gf_data)
{
	msg_header = new MsgHeader(_gf_data);
	gf_data = msg_header->getGFData();

	session_id = 0;
	msg_id = generateMsgId();
	writer = 0;
	reader = 0;

	priority = NORMAL_PRIORITY;
	io_status = NOT_DEFINED;
	storage_status = false;

	interleave_state = false;
}

COM_Message::COM_Message(MsgHeader *_msg_header, QList<MsgPacket*> _packet_list)
{
	msg_header = _msg_header;	
	for (int i = 0; i < _packet_list.count(); i++) packet_list.append(_packet_list.at(i));

	session_id = msg_header->getSessionId();
	msg_id = generateMsgId();
	writer = msg_header->getWriter();
	reader = msg_header->getReader();

	priority = NORMAL_PRIORITY;
	io_status = NOT_DEFINED;
	storage_status = false;
	
	parcel_list << 0;
	
	interleave_state = false;

	gf_data = msg_header->getGFData();
}

COM_Message::~COM_Message()
{
	if (msg_header != NULL) delete msg_header;
	msg_header = NULL;

	if (!packet_list.isEmpty()) 
	{
		qDeleteAll(packet_list);
		packet_list.clear();
	}
}

uint32_t COM_Message::generateMsgId()
{
	static uint32_t _id = 0;
	
	return _id++;
}

uint8_t COM_Message::getCmd()
{
	uint8_t cmd = NAN_DATA;
	MTYPE msg_type = getMsgType();
	if (msg_type == MTYPE_SERVICE) 
	{
		MsgHeader *header = getMsgHeader();
		cmd = header->getSrvData().data[0];	
	}
	else if (msg_type == MTYPE_SHORT) 
	{
		MsgHeader *header = getMsgHeader();
		cmd = header->getShortData().data[0];	
	}
	else if (msg_type == MTYPE_MULTYPACK)
	{
		if (getPackets().isEmpty()) return cmd;
		if (getPackets().at(0)->getByteArray().len < MIN_PACK_LEN) return cmd;
		
		cmd = getPackets().at(0)->getByteArray().data[PACK_INFO_LEN];
	}	

	return cmd;
}

void COM_Message::getRawData(SmartArr *arr)
{
	uint16_t total_len = msg_header->getHeaderLen() + packet_list.count()*msg_header->getPackLen();
	arr->alloc(total_len);

	//memcpy(arr->data, msg_header->getByteArray().data, sizeof(uint8_t)*msg_header->getHeaderLen());
	memcpy(arr->data.get(), msg_header->getByteArray().data.get(), sizeof(uint8_t)*msg_header->getHeaderLen());
	uint16_t pos = (uint16_t)(msg_header->getHeaderLen());
	for (uint16_t i = 0; i < packet_list.count(); i++)
	{		
		//memcpy(arr->data + pos, packet_list.at(i)->getByteArray().data, msg_header->getPackLen());
		memcpy(arr->data.get() + pos, packet_list.at(i)->getByteArray().data.get(), msg_header->getPackLen());
		pos += (uint16_t)(msg_header->getPackLen());
	}
}

void COM_Message::getHeaderRawData(SmartArr *arr)
{
	uint16_t total_len = msg_header->getHeaderLen();
	arr->alloc(total_len);

	//memcpy(arr->data, msg_header->getByteArray().data, sizeof(uint8_t)*msg_header->getHeaderLen());	
	memcpy(arr->data.get(), msg_header->getByteArray().data.get(), sizeof(uint8_t)*msg_header->getHeaderLen());	
}

/*void COM_Message::getBodyRawData(SmartArr *arr)
{
	uint16_t total_len = packet_list.count()*msg_header->getPackLen();
	arr->alloc(total_len);
	
	uint16_t pos = 0;
	for (uint16_t i = 0; i < packet_list.count(); i++)
	{		
		//memcpy(arr->data + pos, packet_list.at(i)->getByteArray().data, msg_header->getPackLen());
		memcpy(arr->data.get() + pos, packet_list.at(i)->getByteArray().data.get(), msg_header->getPackLen());
		pos += (uint16_t)(msg_header->getPackLen());
	}
}*/
void COM_Message::getBodyRawData(SmartArr *arr)
{
	int pack_len = msg_header->getPackLen();
	uint16_t total_len = packet_list.count()*pack_len;
	if (total_len == 0) return;

	arr->alloc(total_len);

	uint16_t pos = 0;
	if (interleaved())
	{
		SmartArr il_pos;
		COM_Message::getInterleavedPositions(pack_len, &il_pos);
		for (uint16_t i = 0; i < packet_list.count(); i++)
		{
			SmartArr pack_arr;
			pack_arr.allocf(packet_list.at(i)->getByteArray());
			uint8_t *data = packet_list.at(i)->getByteArray().data.get();			
			for (int j = 1; j < pack_arr.len; j++)
			{
				uint8_t index = il_pos.data.get()[j];
				pack_arr.data.get()[j] = data[index];
			}
			memcpy(arr->data.get() + pos, data, msg_header->getPackLen());
			pos += (uint16_t)(msg_header->getPackLen());
		}
	}
	else
	{
		for (uint16_t i = 0; i < packet_list.count(); i++)
		{
			memcpy(arr->data.get() + pos, packet_list.at(i)->getByteArray().data.get(), msg_header->getPackLen());
			pos += (uint16_t)(msg_header->getPackLen());
		}
	}
}

void COM_Message::getPacketRawData(SmartArr *arr, int index)
{
	int pack_len = msg_header->getPackLen();
	
	arr->alloc(pack_len);

	if (interleaved())
	{
		SmartArr il_pos;
		COM_Message::getInterleavedPositions(pack_len, &il_pos);
		
		SmartArr pack_arr;
		pack_arr.allocf(packet_list.at(index)->getByteArray());
		uint8_t *data = packet_list.at(index)->getByteArray().data.get();			
		for (int j = 1; j < pack_arr.len; j++)
		{
			uint8_t index = il_pos.data.get()[j];
			pack_arr.data.get()[j] = data[index];
		}
		memcpy(arr->data.get(), data, msg_header->getPackLen());		
	}
	else
	{
		memcpy(arr->data.get(), packet_list.at(index)->getByteArray().data.get(), msg_header->getPackLen());		
	}
}

void COM_Message::addPacket(MsgPacket* _pack)
{
	if (!_pack) return;
	
	packet_list.push_back(_pack);
	parcel_list << packet_list.count();	
}

void COM_Message::clearMsgHeader()
{
	msg_header->clearMsgHeader();
	
	int num = parcel_list.indexOf(0);
	if (num >= 0) parcel_list.removeAt(num);
}

void COM_Message::clearPacketList()
{
	qDeleteAll(packet_list);
	packet_list.clear();

	int num = parcel_list.indexOf(0);
	parcel_list.clear();
	if (num >= 0) parcel_list.push_front(0);
}

void COM_Message::clearCOMMsg()
{
	clearMsgHeader();

	session_id = 0;
	msg_id = 0; //generateMsgId();
	writer = 0;
	reader = 0;

	priority = NORMAL_PRIORITY;
	io_status = NOT_DEFINED;
	storage_status = false;

	interleave_state = false;	

	clearPacketList();
}

void COM_Message::copyCOMMsg(COM_Message *dst_msg)
{	
	//MsgHeader *dst_hdr = new MsgHeader(this->getGFData());
	MsgHeader *dst_hdr = dst_msg->getMsgHeader();
	MsgHeader *src_hdr = this->getMsgHeader();	
	src_hdr->copyMsgHeader(dst_hdr);
	
	dst_msg->setInterleavedState(this->interleaved());
	dst_msg->setIOStatus(this->getIOStatus());
	dst_msg->setPriority(this->getPriority());
	dst_msg->setStored(this->stored());
	dst_msg->setWriter(this->getWriter());
	dst_msg->setReader(this->getReader());
	dst_msg->setSessionId(this->getSessionId());
	dst_msg->generateMsgId();
		
	int pack_count = this->getPackets().count();
	for (int i = 0; i < pack_count; i++)
	{
		MsgPacket *src_pack = this->getPackets().at(i);
		//MsgPacket *dst_pack = new MsgPacket(*src_pack);
		
		MsgPacket *dst_pack = new MsgPacket(i+1, dst_hdr);
		dst_pack->setData(src_pack);
		dst_msg->addPacket(dst_pack);
	}

	/*int parcel_count = this->getParcelCount();
	dst_msg->getParcelList().clear();
	for (int i = 0; i < parcel_count; i++)
	{
		int parcel = this->getParcelList().at(i);
		dst_msg->addParcel(parcel);
	}*/
}

bool COM_Message::addParcel(uint8_t index)
{
	bool res = false;
	if (parcel_list.indexOf(index) < 0) 
	{
		parcel_list.append(index);
		res = true;
	}
	return res;
}

void COM_Message::setInterleavedState(bool flag)
{
	interleave_state = flag;	
}

void COM_Message::getInterleavedPositions(uint8_t len, SmartArr *arr)
{
	arr->alloc((int)len);
	uint8_t *data = arr->data.get();

	int shift = int(sqrt(len)+0.5);
	int from = 0;
	int index = from;

	int i;
	for (i = 0; i < len; i++)
	{
		if (index < len)
		{
			data[i] = index;			
		}
		else
		{
			from++;
			index = from;
			data[i] = index;			
		}
		index += shift;
	}
}
