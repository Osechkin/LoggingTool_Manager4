#include "../io_general.h"

#include "nmrtool_status_wizard.h"


NMRToolStatusWizard::NMRToolStatusWizard(QWidget *parent) : QWidget(parent), ui(new Ui::NMRToolStatusWizard)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	ui->ledPowerStatus->setText("");
	ui->ledTemperatureStatus->setText("");
	ui->ledDataTransfer->setText("");

	setMinimumWidth(160);
	resize(200, this->size().height());
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(onTime()));
	timer.start(1000);
}

NMRToolStatusWizard::~NMRToolStatusWizard()
{
	delete ui;
}

void NMRToolStatusWizard::showPowerStatus(unsigned char pow_status)
{
	switch (pow_status)
	{
	case PowerOK:
		{
			//ui->ledPowerStatus->setStyleSheet("QLineEdit { background-color : green; color : black; }");
			ui->ledPowerStatus->setStyleSheet("QLineEdit { background-color : rgb(170, 255, 0); color : black; }");
			ui->ledPowerStatus->setText("OK");
			power_time = QDateTime::currentDateTime();
			break;
		}
	case PowerLow:
		{
			ui->ledPowerStatus->setStyleSheet("QLineEdit { background-color : red; color : black; }");
			ui->ledPowerStatus->setText("LOW !");
			power_time = QDateTime::currentDateTime();
			break;
		}
	default: break;
	}
}

void NMRToolStatusWizard::showTemperatureStatus(unsigned char temp_status)
{
	switch (temp_status)
	{
	case TempOK:
		{
			ui->ledTemperatureStatus->setStyleSheet("QLineEdit { background-color : rgb(170, 255, 0); color : black; }");
			ui->ledTemperatureStatus->setText("OK");
			temp_time = QDateTime::currentDateTime();
			break;
		}
	case TempHigh:
		{
			ui->ledTemperatureStatus->setStyleSheet("QLineEdit { background-color : red; color : black; }");
			ui->ledTemperatureStatus->setText("HIGH !");
			temp_time = QDateTime::currentDateTime();
			break;
		}
	default: break;
	}
}

void NMRToolStatusWizard::onTime()
{
	QDateTime cur_time = QDateTime::currentDateTime();
	if (cur_time.toTime_t() - power_time.toTime_t() > 15) 
	{
		ui->ledPowerStatus->setStyleSheet("QLineEdit { background-color : lightgray; color : black; }");
		ui->ledPowerStatus->setText("-");
	}
	if (cur_time.toTime_t() - temp_time.toTime_t() > 30) 
	{
		ui->ledTemperatureStatus->setStyleSheet("QLineEdit { background-color : lightgray; color : black; }");
		ui->ledTemperatureStatus->setText("-");
	}
}

void NMRToolStatusWizard::changeLocation(Qt::DockWidgetArea area)
{
	if (area == Qt::RightDockWidgetArea || area == Qt::LeftDockWidgetArea)
	{
		//ui->gridLayout_2->addItem(ui->verticalSpacer, 1, 0, 1, 1);
		this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	}
	else if (area == Qt::BottomDockWidgetArea || area == Qt::NoDockWidgetArea)
	{
		//ui->gridLayout_2->removeItem(ui->verticalSpacer);
		this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	}
}