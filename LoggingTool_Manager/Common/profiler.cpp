#include "QDebug"

#include "profiler.h"


Profiler::Profiler(OutType type) 
{ 
	out_type = type; 
};

Profiler::~Profiler()
{
	if (out_type == AppFinish || out_type == NowAppFinish)
	{
		qDebug() << "\nProfiling Final Report:";
		for (int i = 0; i < proffy.count(); i++)
		{
			TimePeriod *tp = &proffy[i];
			QString str = QString("ID = %1\t%2\t\t\t\t\t%3 [ms]").arg(tp->id).arg(tp->comment).arg(tp->to - tp->from);
			qDebug() << str;
		}
		qDebug() << " ";
	}	
}

void Profiler::reset()
{
	proffy.clear();
}

void Profiler::tic(int id, QString comment)
{
	TimePeriod tp;
	tp.id = id;
	tp.comment = comment;
	
	long mt = 0;
#ifdef Q_OS_LINUX
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	mt = (long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
#else Q_OS_WIN	
	//mt = GetTickCount();	
	LARGE_INTEGER freq;
	if(!QueryPerformanceFrequency(&freq)) return;		
	double d_freq = double(freq.QuadPart)/1000.0;
	
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);	
	mt = t.QuadPart/d_freq;
#endif

	tp.from = mt;
	tp.to = mt;

	proffy.push_back(tp);
}

void Profiler::toc(int id)
{
	int index = -1; 
	for (int i = 0; i < proffy.count(); i++)
	{
		if (proffy[i].id == id) index = i;
	}
	if (index < 0) return;

	TimePeriod *tp = &proffy[index];	
	qint64 mt = 0;
#ifdef Q_OS_LINUX
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	mt = (long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
#else Q_OS_WIN
	//mt = GetTickCount();
	LARGE_INTEGER freq;
	if(!QueryPerformanceFrequency(&freq)) return;		
	double d_freq = double(freq.QuadPart)/1000.0;

	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);	
	mt = t.QuadPart/d_freq;
#endif
	tp->to = mt;

	if (out_type == Now || out_type == NowAppFinish)
	{
		QString str = QString("ID = %1:\t\t%2\t\t\t\t\t\t\t\t\t%3 [ms]").arg(tp->id).arg(tp->comment).arg(tp->to - tp->from);
		qDebug() << str;
	}
}

