#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H


#include "data_containers.h"


// ������, ������� ������ ���� ��������� � ��������� ������
struct ProcessingTask
{
	int id;

};


class T2Processor : public QObject
{
	Q_OBJECT

public:
	T2Processor(ProcessingRelax *_processing_params, DataSet *_ds, DataSet *_out_ds, int _number_id);
	~T2Processor() { }

	void quitThread() { emit quit_thread(); }

	void setNumberId(int num) { number_id = num; }
	int getNumberId() { return number_id; }
	bool isCalcFinished() { return calc_finished; }

	DataSet *getResultedDataSet() { return out_ds; }
	DataSet *getInitialDataSet() { return ds; }

public slots:
	void process();

private:
	int number_id;		// �������������-������� DataSet. ��������� ��� ����, ����� �������� ������ ������ out_ds ����������� � ��� �� �������, ��� � ds
	bool calc_finished;	// = true, ���� ���������� �����������

	DataSet *ds;
	DataSet *out_ds;
	ProcessingRelax *proc_params;	

signals:
	void finished();
	void quit_thread();
};


// ����� � ���������� ��������, ������� ����� ��������� ���������������/�����������. 
// ������ ������������ ���� "���" �����. ��������, ��� ����� ���������� ���������������, ������� 
// ��������� � ������ ���� ���������
class ProcessingPool : public QObject
{
	Q_OBJECT

public:
	ProcessingPool();
	
	bool addT2Processor(T2Processor *t2_proc);
	void startT2Processing();

	QList<T2Processor*> &getT2Processors() { return t2_procs; }

	DataSets &getResultedDataSets() { return out_dss; }
	DataSets &getInitialDataSets();

	void clearAll();

	bool isBusy() { return is_busy; }
	void setBusy(bool flag) { is_busy = flag; }
	
private slots:
	void obtainResult();	

private:
	QList<T2Processor*> t2_procs;	// ��������� 
	int ready_counter;				// ������� ����������� �����
	QList<int> id_list;				// ������ ��������������� ����� (T2Processor) � ������� �� �����������
	
	DataSets out_dss;				// ��������� ������� �������� �2

	bool is_busy;

signals:	
	void data_ready();
};


// ��������� ����������� ����� ����� (��������, ��������� ��� ����� ���������� ���������������, ������� ����� ����������, 
// ����� �� ����, ��� ��� ��� ������ /Task/ ���� ���������, ��������� ��� ��� �����. � ����� ���������� ��� ������ ����� �� ��� � ������.)
typedef QList<ProcessingPool*>		ProcessingHolder;	



#endif // DATA_PROCESSING_H