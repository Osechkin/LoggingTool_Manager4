#ifndef DEPTH_INTERNAL_WIDGET_H
#define DEPTH_INTERNAL_WIDGET_H

#include <QWidget>

#include "tools_general.h"
#include "../Communication/threads.h"
#include "../Common/statusbar_widgets.h"

#include "abstract_depthmeter.h"

#include "ui_depth_internal_widget.h"


class DepthInternalWidget : public AbstractDepthMeter, public Ui::InternalDepthMeter
{
	Q_OBJECT

public:
	explicit DepthInternalWidget(Clocker *_clocker, COM_PORT *com_port, QWidget *parent = 0);
	~DepthInternalWidget();

	void saveSettings() { }

	QString getTitle() { return tr("Internal DepthMeter"); }
	DepthMeterType getType() { return DepthMeterType::InternalDepthMeter; }

	double_b getDepth() { return double_b(true, depth); }
	double_b getRate() { return double_b(true, rate); }
	double_b getTension() { return double_b(true, tension); }
	int_b getMagnetMarks() { return int_b(true, magn_marks); }
	/*double newDepth(); */

	void stopDepthMeter() { }
	void startDepthMeter() { }

private:	
	void setConnection();	
	void setDepthCommunicatorConnections();
	void showData(uint8_t type, double val);

private:
	Ui::InternalDepthMeter *ui;
	
	Clocker *clocker;
	QTimer timer;

	COM_PORT *COM_Port;
	DepthCommunicator *depth_communicator;
	ImpulsConnectionWidget *connectionWidget;

	double depth;
	double rate;
	double tension;
	int magn_marks;
	bool depth_flag;
	bool rate_flag;
	bool tension_flag;
	bool magnmarks_flag;

	QStringList depth_units_list;
	QStringList rate_units_list;
	QStringList tension_units_list;
	double k_depth;
	double k_rate;
	double k_tension;
		
	bool depth_active;
	bool rate_active;
	bool tension_active;
	bool magnmarks_active;

	bool is_connected;
	bool is_started;
	bool device_is_searching;


private slots:
	void connectDepthMeter(bool flag);
	/*void changeUnits(QString str);*/
	void getMeasuredData(uint32_t _uid, uint8_t _type, double val);
	/*void measureTimedOut(uint32_t _uid, uint8_t _type);	
	void onTime();*/
	void includeParameter(int state);

public slots:

signals:
	//void to_measure(uint32_t, uint8_t);		
	void connected(bool);	

};

#endif // DEPTH_INTERNAL_WIDGET_H