#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H


#include "data_containers.h"


// Задача, которая должна быть выполнена в отдельном потоке
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
	int number_id;		// идентификатор-счетчик DataSet. Необходим для того, чтобы итоговые наборы данных out_ds расположить в том же порядке, что и ds
	bool calc_finished;	// = true, если вычисления закончились

	DataSet *ds;
	DataSet *out_ds;
	ProcessingRelax *proc_params;	

signals:
	void finished();
	void quit_thread();
};


// класс с отдельными задачами, которые нужно выполнить последовательно/параллельно. 
// Задачи представляют один "пул" задач. Например, три спада поперечной намагниченности, которые 
// относятся к одному акту измерений
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
	QList<T2Processor*> t2_procs;	// контейнер 
	int ready_counter;				// счетчик выполненных задач
	QList<int> id_list;				// список идентификаторов задач (T2Processor) в порядке их поступления
	
	DataSets out_dss;				// контейнер готовых спектров Т2

	bool is_busy;

signals:	
	void data_ready();
};


// хранилище выполняемых групп задач (например, поступили три спада поперечной намагниченности, которые нужно обработать, 
// затем до того, как эти три задачи /Task/ были завершены, послупило еще три спада. В итоге существует две группы задач по три в каждой.)
typedef QList<ProcessingPool*>		ProcessingHolder;	



#endif // DATA_PROCESSING_H