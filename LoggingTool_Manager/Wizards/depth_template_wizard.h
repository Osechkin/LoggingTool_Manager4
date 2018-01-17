#ifndef DEPTH_TEMPLATE_WIZARD_H
#define DEPTH_TEMPLATE_WIZARD_H

#include <QWidget>

#include "tools_general.h"
#include "../Common/statusbar_widgets.h"
#include "../Communication/threads.h"

#include "../Widgets/abstract_depthmeter.h"
#include "../Widgets/depth_emul_widget.h"
#include "../Widgets/depth_impulsustye_widget.h"
#include "../Widgets/depth_internal_widget.h"
#include "../Widgets/distance_meter_widget.h"

#include "ui_depth_template_wizard.h"


class MainWindow;


class DepthTemplateWizard : public QWidget, public Ui::DepthTemplateWizard
{
	Q_OBJECT

public:
	explicit DepthTemplateWizard(QSettings *_settings, COM_PORT *com_port, COM_PORT *com_port_stepmotor, QStringList depth_meter_list, Clocker *clocker, QWidget *parent = 0);
	~DepthTemplateWizard();

	double_b getDepthData() { return current_depth_meter->getDepth(); /*double_b(depth_flag, depth);*/ } 
	double_b getRateData() { return current_depth_meter->getRate(); /*double_b(rate_flag, rate);*/ }
	double_b getTensionData() { return current_depth_meter->getTension(); /*double_b(tension_flag, tension);*/ }

	AbstractDepthMeter *getCurrentDepthMeter() { return current_depth_meter; }

	void saveSettings() { current_depth_meter->saveSettings(); }

	ImpulsConnectionWidget *getConnectionWidget() { return connectionWidget; }
	bool isConnected() { return is_connected; }

	COM_PORT *getCOMPort() { return COM_Port; }
	COM_PORT *getCOMPortStepMotor() { return COM_Port_stepmotor; }

	Ui::DepthTemplateWizard *getUI() { return ui; }

private:
	void initCOMSettings(COM_PORT *com_port);
	void showData(uint8_t type, double val);

	void setConnection();
	void setDepthCommunicatorConnections();	
	bool isAvailableCOMPort(COM_PORT *_com_port, int port_num);
	QStringList availableCOMPorts();

private:
	Ui::DepthTemplateWizard *ui;
	ImpulsConnectionWidget *connectionWidget;

	QList<AbstractDepthMeter*> depth_meters;	
	AbstractDepthMeter *current_depth_meter;
	
	Clocker *clocker;
	QTimer timer;

	COM_PORT *COM_Port;
	COM_PORT *COM_Port_stepmotor;

	QSettings *app_settings;

	bool is_connected;
	bool depth_active;
	bool rate_active;
	bool tension_active;

private slots:	
	void onTime();	
	void changeDepthMeter(QString str);
	
signals:
	void to_measure(uint32_t, uint8_t);		
	void connected(bool);
	void timeout();
	void new_core_diameter(double);
};

#endif // DEPTH_TEMPLATE_WIZARD_H