#ifndef PROFILER_H
#define PROFILER_H

#ifdef Q_OS_LINUX
#include <sys/time.h>
#else Q_OS_WIN
#include <time.h>
#include <Windows.h>
#endif

#include "QString"
#include "QList"


class Profiler
{
public:
	enum OutType { Now = 1, AppFinish = 2, NowAppFinish = 3 };	
	
	Profiler(OutType type);
	~Profiler();

	struct TimePeriod
	{
		qint64 from;
		qint64 to;
		QString comment;
		int id;
	};
	
	void reset();
	void tic(int id, QString comment);
	void toc(int id);		

private:
	QList<TimePeriod> proffy;
	OutType out_type;
};

static Profiler profiler(Profiler::AppFinish);

#endif // PROFILER_H