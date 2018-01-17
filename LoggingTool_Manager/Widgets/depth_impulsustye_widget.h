#ifndef DEPTH_IMPULSUSTYE_WIDGET_H
#define DEPTH_IMPULSUSTYE_WIDGET_H

#include <QWidget>

#include "tools_general.h"
#include "../Communication/threads.h"
#include "../Common/statusbar_widgets.h"

#include "abstract_depthmeter.h"

#include "ui_depth_impulsustye_widget.h"


class DepthImpulsUstyeWidget : public AbstractDepthMeter, public Ui::DepthImpulsUstyeWidget
{
	Q_OBJECT

public:
	explicit DepthImpulsUstyeWidget(Clocker *_clocker, COM_PORT *com_port, QWidget *parent = 0);
	~DepthImpulsUstyeWidget();

	void saveSettings() { }

	QString getTitle() { return tr("Impuls-Ustye"); }
	DepthMeterType getType() { return DepthMeterType::ImpulsUstye; }

	double_b getDepth() { return double_b(true, depth); }
	double_b getRate() { return double_b(true, rate); }
	double_b getTension() { return double_b(false, 0.0); }
	int_b getMagnetMarks() { return int_b(false, 0); }
	double newDepth();

	void stopDepthMeter();
	void startDepthMeter();

private:	
	void setConnection();	
	void setDepthCommunicatorConnections();
	void showData(uint8_t type, double val);

private:
	Ui::DepthImpulsUstyeWidget *ui;
	
	Clocker *clocker;
	QTimer timer;

	COM_PORT *COM_Port;
	DepthCommunicator *depth_communicator;
	ImpulsConnectionWidget *connectionWidget;

	double depth;
	double rate;
	double tension;
	bool depth_flag;
	bool rate_flag;
	bool tension_flag;

	QStringList depth_units_list;
	QStringList rate_units_list;
	QStringList tension_units_list;
	double k_depth;
	double k_rate;
	double k_tension;
		
	bool depth_active;
	bool rate_active;
	bool tension_active;

	bool is_connected;
	bool is_started;
	bool device_is_searching;


private slots:
	void connectDepthMeter(bool flag);
	void changeUnits(QString str);
	void getMeasuredData(uint32_t _uid, uint8_t _type, double val);
	void measureTimedOut(uint32_t _uid, uint8_t _type);
	/*void setNewDepth(double val);
	void setNewRate(double val);
	void startDepth(bool flag);
	void showNewDepth();
	void stopDepth();
	void clocked();*/
	void includeParameter(int state);
	void onTime();

public slots:

signals:
	void to_measure(uint32_t, uint8_t);		
	void connected(bool);
	//void new_data(double, double);		// first variable is new depth, second variable is new rate

};

#endif // DEPTH_IMPULSUSTYE_WIDGET_H