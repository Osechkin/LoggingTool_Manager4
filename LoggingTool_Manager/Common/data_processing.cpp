#include "data_processing.h"

#include "DMDeconv/DMDeconv.h"


ProcessingPool::ProcessingPool()
{
	ready_counter = 0;
	out_dss.clear();

	is_busy = false;
}

bool ProcessingPool::addT2Processor(T2Processor *t2_proc)
{
	if (is_busy) return false;
	
	t2_procs.append(t2_proc);	
	out_dss.append(NULL);

	return true;
}

void ProcessingPool::startT2Processing()
{
	if (!t2_procs.isEmpty()) is_busy = true;
	else 
	{
		is_busy = false;
		return;
	}

	static int thread_id = 1;
	//qDebug() << QString("Totally t2_procs: %1").arg(t2_procs.count()); 
	for (int i = 0; i < t2_procs.count(); i++)
	{
		//qDebug() << QString("Moving t2_proc#%1 to QThread...").arg(i); 
		T2Processor *t2_proc = t2_procs.at(i);
		QThread *t2_thread = new QThread;
		t2_thread->setObjectName(QString("T2Processor%1-%2").arg(thread_id++).arg(i+1));
		t2_proc->moveToThread(t2_thread);

		connect(t2_thread, SIGNAL(started()), t2_proc, SLOT(process()));
		connect(t2_proc, SIGNAL(finished()), this, SLOT(obtainResult()));
		connect(t2_proc, SIGNAL(quit_thread()), t2_thread, SLOT(quit()));
		connect(t2_thread, SIGNAL(finished()), t2_thread, SLOT(deleteLater()));		
		connect(t2_thread, SIGNAL(finished()), t2_proc, SLOT(deleteLater()));
		
		t2_thread->start();
	}	
}

void ProcessingPool::obtainResult()
{
	//qDebug() << "ObtainResult !";
	T2Processor *t2_proc = (T2Processor*)sender();
	if (!t2_proc) return;

	ready_counter++;

	int index = t2_proc->getNumberId();
	if (index < t2_procs.count())
	{
		out_dss[index] = t2_proc->getResultedDataSet();
		t2_proc->quitThread();				
	}

	if (ready_counter >= t2_procs.count())
	{
		//is_busy = false;
		emit data_ready();
	}
}

DataSets &ProcessingPool::getInitialDataSets()
{
	DataSets out;
	for (int i = 0; i < t2_procs.count(); i++)
	{
		T2Processor *t2_proc = t2_procs.at(i);
		out.append(t2_proc->getInitialDataSet());
	}

	return out;
}

void ProcessingPool::clearAll()
{
	/*for (int i = 0; i < t2_procs.count(); i++)
	{
		T2Processor *t2_proc = t2_procs[i];
		t2_proc->deleteLater();
	}*/
	t2_procs.clear();

	ready_counter = 0;
	id_list.clear();
	out_dss.clear();				

	is_busy = false;
}


T2Processor::T2Processor(ProcessingRelax *_processing_params, DataSet *_ds, DataSet *_out_ds, int _number_id)
{
	number_id = _number_id;	// располагать в итоговом контейнере структур DataSet в соответствии с номером number_id

	this->ds = _ds;
	this->out_ds = _out_ds;
	this->proc_params = _processing_params;	

	calc_finished = false;
}

void T2Processor::process()
{
	dm_deconv::mat1D *A = ds->getYData();
	dm_deconv::mat1D *ticks = ds->getXData();
	dm_deconv::mat1D *T2 = out_ds->getXData();
	dm_deconv::mat1D *Spectrum = out_ds->getYData();

	double Alpha = proc_params->regulParam;
	int Iters = proc_params->iters;

	// Start calculations !
	dm_deconv::errCode ErrCode;
	dm_deconv::do_dm_deconv(A, ticks, T2, Iters, Alpha, Spectrum, ErrCode);	
	calc_finished = true;

	emit finished();
}