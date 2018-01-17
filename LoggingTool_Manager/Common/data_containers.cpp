#include "data_containers.h"
#include "../io_general.h"


bool DataSetWindow::is_on = true;
int DataSetWindow::max_size = 1;


DataSet::DataSet()
{
	UId = 0;
	comm_id = 0;
	name = "";
		
	cr_time = QDateTime::currentDateTime();
	depth = 0;
	depth_flag = false;

	group_index = 0;
	channel_id = 0;
	data_num = 0;

	te = 0;
	tw = 0;
	td = 0;

	x = NULL;
	y = NULL;
	bad_data = NULL;	
	bad_data_index = 0;
	initial_size = 0;
}

DataSet::DataSet(uint32_t _uid, uint8_t _comm_id, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_data)
{
	UId = _uid;
	comm_id = _comm_id;
	name = "";
		
	cr_time = QDateTime::currentDateTime();
	depth = 0;
	depth_flag = false;

	group_index = 0;
	channel_id = 0;
	data_num = 0;

	te = 0;
	tw = 0;
	td = 0;

	x = _x;
	y = _y;
	initial_size = y->size();
	bad_data = _bad_data;	
	float cnt = 0;
	for (int i = 0; i < _bad_data->size(); i++) if (_bad_data->at(i) == BAD_DATA) cnt++;
	if (_bad_data->size() != 0) bad_data_index = cnt/_bad_data->size();
	else bad_data_index = 0;

	//delete _bad_data;
}

DataSet::DataSet(QString &_name, uint32_t _uid, uint8_t _comm_id, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_data)
{
	UId = _uid;
	comm_id = _comm_id;
	name = _name;
	
	cr_time = QDateTime::currentDateTime();
	depth = 0;
	depth_flag = false;

	group_index = 0;
	channel_id = 0;
	data_num = 0;

	te = 0;
	tw = 0;
	td = 0;

	x = _x;
	y = _y;
	initial_size = y->size();
	bad_data = _bad_data;	
	float cnt = 0;
	for (int i = 0; i < _bad_data->size(); i++) if (_bad_data->at(i) == BAD_DATA) cnt++;
	if (_bad_data->size() != 0) bad_data_index = cnt/_bad_data->size();
	else bad_data_index = 0;

	//delete _bad_data;
}

DataSet::~DataSet()
{
	delete x;
	delete y;
	delete bad_data;
}

void DataSet::copyTo(DataSet *_ds)
{
	DataSet *ds = this;

	uint32_t _uid = ds->getUId();
	uint8_t _comm_id = ds->getDataCode();
	uint32_t _exp_id = ds->getExpId();
	QString _name = ds->getDataName();
	int _initial_data_size = ds->getInitialDataSize();
	int _group_index = ds->getGroupIndex();
	int _data_num = ds->getDataNum();

	QDateTime _cr_time = ds->getCreationTime();

	QPair<bool, double> _depth = ds->getDepth();

	int x_size = ds->getXData()->size();
	int y_size = ds->getYData()->size();
	int bad_map_size = ds->getBadData()->size();

	QVector<double> *_x = new QVector<double>(x_size);
	QVector<double> *_y = new QVector<double>(y_size);
	QVector<uint8_t> *_bad_map = new QVector<uint8_t>(bad_map_size);
	memcpy(_x->data(), ds->getXData()->data(), x_size*sizeof(double));
	memcpy(_y->data(), ds->getYData()->data(), y_size*sizeof(double));
	memcpy(_bad_map->data(), ds->getBadData()->data(), bad_map_size*sizeof(uint8_t));

	//_ds = new DataSet(_name, _uid, _comm_id, _x, _y, _bad_map);
	//_ds->setData(_x, _y, _bad_map);
	_ds->setData(_x, _y, _bad_map);
	_ds->setInitialDataSize(_initial_data_size);
	_ds->setGroupIndex(_group_index);
	_ds->setDataCode(_comm_id);
	_ds->setDataNum(_data_num);
	_ds->setDataName(_name);
	_ds->setExpId(_exp_id);
	_ds->setUId(_uid);
	_ds->setCreationTime(_cr_time);
	_ds->setDepth(_depth);

	_ds->bad_data_index = ds->bad_data_index;
}

void DataSet::setData(QVector<double>* _xvec, QVector<double>* _yvec, QVector<uint8_t>* _bad_map)
{
	x = _xvec;
	y = _yvec;
	bad_data = _bad_map;
}

