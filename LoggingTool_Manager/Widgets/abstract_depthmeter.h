#ifndef ABSTRACT_DEPTHMETER_H
#define ABSTRACT_DEPTHMETER_H

#include <QObject>


typedef QPair<bool, double>		double_b;
typedef QPair<bool, int>		int_b;

class AbstractDepthMeter : public QWidget
{
public:
	AbstractDepthMeter() { }
	~AbstractDepthMeter() { }

	enum DepthMeterType { DepthEmulator, ImpulsUstye, InternalDepthMeter, LeuzeDistanceMeter, NoType };

	virtual void saveSettings() { }

	virtual QString getTitle() { return QString(""); }
	virtual DepthMeterType getType() { return NoType; }
	virtual double_b getDepth() { return double_b(false, 0.0); }
	virtual double_b getRate() { return double_b(false, 0.0); }
	virtual double_b getTension() { return double_b(false, 0.0); }
	virtual int_b getMagnetMarks() { return int_b(false,0); }
	
	virtual void stopDepthMeter() { }
	virtual void startDepthMeter() { }

};

#endif // ABSTRACT_DEPTHMETER_H