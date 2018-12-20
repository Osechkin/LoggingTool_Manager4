#ifndef PRESSURE_UNIT_WIZARD_H
#define PRESSURE_UNIT_WIZARD_H

#include <QWidget>

#include "tools_general.h"
#include "../Communication/threads.h"
#include "../Common/statusbar_widgets.h"

#include "ui_pressure_unit_wizard.h"


class PressureUnit : public QWidget, public Ui::PressureUnitWizard
{
	Q_OBJECT

public:
	explicit PressureUnit(QWidget *parent = 0);
	~PressureUnit();

	//void saveSettings();

	QString getTitle() { return tr("Pressure Unit"); }
	
	/*double_b getDepth() { return double_b(true, distance-zero_pos); }
	QPair<double,double> getBounds() { return QPair<double,double>(lower_bound, upper_bound); }
	QPair<double,double> getFromTo() { return QPair<double,double>(from_pos, to_pos); }
	double getOrderedDepth() { return ui->dsboxSetPosition->value()/k_set_distance; }
	double getStep() { return step_pos; }
	double getZeroPos() { return zero_pos; }
	double getCalibrationLength() { return calibr_len; }
	double getCoreDiameter() { return core_diameter; }*/

	//void stopDepthMeter();
	//void startDepthMeter();

private:	
	void setConnection();	
	/*void setDepthCommunicatorConnections();
	void setStepMotorCommunicatorConnections();
	void showData(uint8_t type, double val);
	*/

private:
	Ui::PressureUnitWizard *ui;

	Clocker *clocker;
	QTimer timer;
	QTimer stepmotor_timer;

	QSettings *app_settings;

	/*
	LeuzeCommunicator *leuze_communicator;
	StepMotorCommunicator *stepmotor_communicator;
	ImpulsConnectionWidget *connectionWidget;
	*/

	/*
	double distance;
	double k_distance;
	double set_distance;
	double k_set_distance;
	int direction_coef;
	bool pos_is_set;

	QStringList distance_units_list;

	double from_pos;
	double to_pos;
	double step_pos;
	double zero_pos;
	double calibr_len;
	double core_diameter;
	double standard_porosity;
	double standard_diameter;
	double k_from;
	double k_to;
	double k_step;
	double k_zero;
	double k_calibr_len;
	double k_core_d;
	double k_standard_porosity;

	double lower_bound;
	double upper_bound;		

	bool distance_ok;

	bool is_connected;
	bool is_started;	
	//bool device_is_searching;
	*/

private slots:
	void openPressureUnit();
	void closePressureUnit();
	void stopPressureUnit();
	
	/*
	void connectAllMeters(bool flag);
	//void changeUnits(QString str);
	void getMeasuredData(uint32_t _uid, uint8_t _type, double val);
	void measureTimedOut(uint32_t _uid, uint8_t _type);
	void showErrorMsg(QString msg);
	void onTime();	

	void setNewTo(double val);
	void setNewFrom(double val);
	void setNewStep(double val);
	void setNewCoreDiameter(double val);

	void setZeroPos();
	void clearZeroPos();

public slots:
	void setPosition(double pos);

signals:
	void to_measure(uint32_t, uint8_t);		
	void connected(bool);
	void set_from_to_step(double, double, double);
	void new_core_diameter(double);
	void cmd_resulted(bool, uint32_t);
	*/
signals:
	void send_msg(DeviceData*, const QString&);
};

#endif // PRESSURE_UNIT_WIZARD_H