#ifndef NMRTOOL_STATUS_WIZARD_H
#define NMRTOOL_STATUS_WIZARD_H

#include <QTimer>
#include <QDateTime>

#include "ui_nmrtool_status_wizard.h"


class NMRToolStatusWizard : public QWidget, public Ui::NMRToolStatusWizard
{
	Q_OBJECT

public:
	explicit NMRToolStatusWizard(QWidget *parent = 0);
	~NMRToolStatusWizard();

	void showPowerStatus(unsigned char pow_status);
	void showTemperatureStatus(unsigned char temp_status);

	Ui::NMRToolStatusWizard *getUI() { return ui; }

private:		
	Ui::NMRToolStatusWizard *ui;
		
	QTimer timer;
	QDateTime power_time; 
	QDateTime temp_time;
	
private slots:	
	void changeLocation(Qt::DockWidgetArea area);
	void onTime();

};

#endif // NMRTOOL_STATUS_WIZARD_H