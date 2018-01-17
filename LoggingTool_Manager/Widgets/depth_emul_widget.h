#ifndef DEPTH_EMUL_WIDGET_H
#define DEPTH_EMUL_WIDGET_H

#include <QWidget>

#include "tools_general.h"
#include "../Communication/threads.h"

#include "abstract_depthmeter.h"

#include "ui_depth_emul_widget.h"


class DepthEmulatorWidget : public AbstractDepthMeter, public Ui::DepthEmulatorWidget
{
	Q_OBJECT

public:
	explicit DepthEmulatorWidget(Clocker *_clocker, QWidget *parent = 0);
	~DepthEmulatorWidget();

	void saveSettings() { }

	QString getTitle() { return tr("Depth Emulator"); }
	DepthMeterType getType() { return DepthMeterType::DepthEmulator; }

	double_b getDepth() { return double_b(true, depth); }
	double_b getRate() { return double_b(true, rate); }
	double_b getTension() { return double_b(false, 0.0); }
	int_b getMagnetMarks() { return int_b(false, 0); }
	double newDepth();

	void stopDepthMeter();
	void startDepthMeter();

private:	
	void setConnection();	
	void showData(uint8_t type, double val);

private:
	Ui::DepthEmulatorWidget *ui;

	Clocker *clocker;
	QTimer timer;
		
	QStringList depth_units_list; 
	QStringList rate_units_list; 

	double depth;
	double rate;	
	double dt;
	//bool depth_flag;
	//bool rate_flag;	

	double k_depth;
	double k_rate;

	bool is_started;
		

private slots:
	void changeUnits(QString str);
	void setNewDepth(double val);
	void setNewRate(double val);
	void startDepth(bool flag);
	void showNewDepth();
	void stopDepth();
	void clocked();

public slots:
	
signals:
	void new_data(double, double);		// first variable is new depth, second variable is new rate
	
};

#endif // DEPTH_EMUL_WIDGET_H