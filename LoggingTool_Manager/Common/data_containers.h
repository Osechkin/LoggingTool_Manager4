#ifndef DATA_CONTAINERS_H
#define DATA_CONTAINERS_H

#include <stdint.h>

#include <QtCore>
#include <QDateTime>
#include <QColor>


class DataSet
{
public:		
	DataSet();
	DataSet(uint32_t _uid, uint8_t _comm_id, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_data);
	DataSet(QString &_name, uint32_t _uid, uint8_t _comm_id, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_data);
	~DataSet();

	uint32_t getUId() { return UId; }
	void setUId(uint32_t _uid) { UId = _uid; }

	uint32_t getExpId() { return experiment_id; }
	void setExpId(uint32_t _exp_id) { experiment_id = _exp_id; }

	uint8_t getDataCode() { return comm_id; }
	void setDataCode(uint8_t _comm_id) { comm_id = _comm_id; }

	QString &getDataName() { return name; }
	void setDataName(QString &_str) { name = _str; }

	int getGroupIndex() { return group_index; }
	void setGroupIndex(int _index) { group_index = _index; }

	unsigned int getChannelId() { return channel_id; }
	void setChannelId(uint8_t _channel_id) { channel_id = _channel_id; }

	int getDataNum() { return data_num; }
	void setDataNum(int _data_num) { data_num = _data_num; }

	QDateTime getCreationTime() { return cr_time; }
	void setCreationTime( QDateTime _time) { cr_time = _time; }

	QPair<bool,double> getDepth() { return QPair<bool,double>(depth_flag,depth); }
	void setDepth(QPair<bool,double> val) { depth = val.second; depth_flag = val.first; }

	int getInitialDataSize() { return initial_size; }
	void setInitialDataSize(int val) { initial_size = val; }

	void copyTo(DataSet *_ds);

	QVector<double> *getXData() { return x; }
	QVector<double> *getYData() { return y; }
	QVector<uint8_t> *getBadData() { return bad_data; }
	void setXData(QVector<double>* _xvec) { x = _xvec; }
	void setYData(QVector<double>* _yvec) { y = _yvec; }
	//void setBadData(QVector<uint8_t>* _bad_map) { bad_data = _bad_map; }
	void setData(QVector<double>* _xvec, QVector<double>* _yvec, QVector<uint8_t>* _bad_map);

	double TE() { return te; }
	void setTE(double _te) { te = _te; }
	double TW() { return tw; }
	void setTW(double _tw) { tw = _tw; }
	double TD() { return td; }
	void setTD(double _td) { td = _td; }

private:
	QString name;
	uint32_t UId;	
	uint8_t	comm_id;			// код данных (в качестве кода данных используетс€, например, CPMG_PRG и пр.)
	int experiment_id;			// код эксперимента, в процессе которого были измерены данные 

	QVector<double> *x;
	QVector<double> *y;
	QVector<uint8_t> *bad_data;

	int group_index;			// групповой идентификатор
	unsigned int channel_id;	// номер канала данных (датчик яћ–1, датчик яћ–2 и т.д.)
	int data_num;				// номер набора данных в одной серии измерений (например, номер спада в двумерном яћ–)

	double bad_data_index;		// дол€ некорректных данных, которые не были успешно декодированы
	int initial_size;			// исходна€ длина вход€щих данных (вектор "y" уже содержит данные без искаженных помехами и пропущенных значений)

	double depth;				// глубина, на которой были измерены данные [m]
	bool depth_flag;			// показетель, €вл€етс€ ли глубина истинной (противоположный случай - отсоединилс€ глубиномер)
	QDateTime cr_time;			// врем€ создани€ набора данных

	double te;					// задержка TE в данных яћ–
	double tw;					// задержка TW в данных яћ–
	double td;					// задержка TD в данных яћ–
};

typedef QList<DataSet*> DataSets;


struct ProcessingRelax
{
	ProcessingRelax()
	{
		is_centered = false;
		is_smoothed = false;
		is_extrapolated = false;
		is_quality_controlled = false;

		smoothing_number = 2;
		extrapolation_number = 1;
		alpha = 0.25;
		I = 3;
		K = 15;

		T2_from = 100;
		T2_to = 3000;
		T2_points = 100;
		iters = 100;
		regulParam = T2_points;

		T2_min = 0.001;
		T2_max = 10000.0;
		T2_cutoff_clay = 3.0;
		T2_cutoff = 33.0;
		MCBWcolor = QColor(Qt::green);
		MBVIcolor = QColor(Qt::red);
		MFFIcolor = QColor(Qt::blue);

		win_aver_on = true;
		win_aver_len = 1;

		porosity_on = true;
	}

	bool is_centered;
	bool is_smoothed;
	bool is_extrapolated;
	bool is_quality_controlled;
	int smoothing_number;
	int extrapolation_number;
	double alpha;
	int I;
	int K;
		
	double T2_from;
	double T2_to;
	int T2_points;
	int iters;
	double regulParam;

	double T2_min;
	double T2_max;
	double T2_cutoff_clay;
	double T2_cutoff;
	QColor MCBWcolor;
	QColor MBVIcolor;
	QColor MFFIcolor;

	bool win_aver_on;
	int win_aver_len;

	bool porosity_on;		// отображать/не отображать бины в единицах пористости
};


struct DataType
{
	DataType(uint8_t _type, QString _mnemonics, bool _flag, QString _comment)
	{
		type = _type;
		flag = _flag;
		mnemonics = _mnemonics;
		comment = _comment;
	}

	static bool checkState(uint8_t _dtype, QList<DataType> &dtype_list, bool *_ok)
	{
		*_ok = false;
		bool res = false;
		for (int i = 0; i < dtype_list.count(); i++)
		{
			DataType dtype = dtype_list[i];
			if (dtype.type == _dtype) 
			{ 
				*_ok = true; 
				res = dtype.flag; 
				break;
			}
		}

		return res;
	}
	
	QString mnemonics;	
	QString comment;
	uint8_t type;
	bool flag;
};
typedef QList<DataType>	DataTypeList;


class DataSetWindow
{
public:
	DataSetWindow(int _max_size, bool _is_on = true)
	{
		max_size = _max_size;
		is_on = _is_on;
	}

	int max_data_len(int &index)
	{
		int max = 0;
		index = -1;
		for (int i = 0; i < y_vectors.count(); i++)
		{			
			if (y_vectors[i].count() > max) 
			{
				max = y_vectors[i].count();
				index = i;
			}
		}		

		return max;
	}

	int count() { return y_vectors.count(); }

	void clear()
	{
		x_vectors.clear();
		y_vectors.clear();
	}

	QList<QVector<double> > x_vectors;
	QList<QVector<double> > y_vectors;

	static int max_size;
	static bool is_on;
};

typedef QList<DataSetWindow> DataSetWindows;


#endif // DATA_CONTAINERS_H