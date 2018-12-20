#include <QDesktopWidget>

#include "pressure_unit_wizard.h"


PressureUnit::PressureUnit(QWidget *parent)  : ui(new Ui::PressureUnitWizard)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	this->setObjectName("PressureUnitWizard");

	QDesktopWidget *d = QApplication::desktop();
	int cur_x = d->width();     // returns desktop width
	int cur_y = d->height();    // returns desktop height
	int fontSize = 10;
	int iconSize = 24;
	if (cur_x < 1920) 
	{
		fontSize = 9;
		iconSize = 16;
	}

	ui->lblShtok->setStyleSheet("QLabel { color: darkblue }");
	ui->lblVint->setStyleSheet("QLabel { color: darkblue }");
	ui->lblPruzhina->setStyleSheet("QLabel { color: darkblue }");
	ui->lblShtokUnits->setStyleSheet("QLabel { color: darkblue }");
	ui->lblVintUnits->setStyleSheet("QLabel { color: darkblue }");
	ui->lblKoncevik1->setStyleSheet("QLabel { color: darkblue }");
	ui->lblKoncevik2->setStyleSheet("QLabel { color: darkblue }");
	ui->ledShtok->setStyleSheet("QLineEdit { color: darkblue }");
	ui->ledVint->setStyleSheet("QLineEdit { color: darkblue }");
	ui->progressBarShtok->setStyleSheet("QProgressBar { color: darkblue }");
	ui->progressBarVint->setStyleSheet("QProgressBar { color: darkblue }");
	ui->progressBarPruzhina->setStyleSheet("QProgressBar { color: darkblue }");
	ui->pbtBack->setStyleSheet("QPushButton { color: black }");
	ui->pbtForward->setStyleSheet("QPushButton { color: black }");	
	ui->pbtStop->setStyleSheet("QPushButton { color: black }");

	ui->pbtBack->setText(tr("Close"));
	ui->pbtBack->setIcon(QIcon(":/images/play_back_all.png"));
	ui->pbtForward->setText(tr("Open"));
	ui->pbtForward->setIcon(QIcon(":/images/play_all.png"));
	ui->pbtStop->setText(tr("Stop"));
	ui->pbtStop->setIcon(QIcon(":/images/stop.png"));

	ui->progressBarVint->setValue(0);
	ui->progressBarShtok->setValue(0);
	ui->progressBarPruzhina->setValue(0);

	ui->label1->setMaximumSize(QSize(16, 16));
	ui->label1->setTextFormat(Qt::AutoText);
	ui->label1->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	ui->label1->setScaledContents(true);
	ui->label1->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	ui->label1->setWordWrap(false);
	ui->label1->setMargin(0);

	ui->label2->setMaximumSize(QSize(16, 16));
	ui->label2->setTextFormat(Qt::AutoText);
	ui->label2->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	ui->label2->setScaledContents(true);
	ui->label2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	ui->label2->setWordWrap(false);
	ui->label2->setMargin(0);

	setConnection();
}

PressureUnit::~PressureUnit()
{

}

void PressureUnit::setConnection()
{
	connect(ui->pbtForward, SIGNAL(clicked()), this, SLOT(openPressureUnit()));
	connect(ui->pbtBack, SIGNAL(clicked()), this, SLOT(closePressureUnit()));
	connect(ui->pbtStop, SIGNAL(clicked()), this, SLOT(stopPressureUnit()));
}

void PressureUnit::openPressureUnit()
{
	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(PRESS_UNIT_OPEN, "Move Pressure Unit forward", id);

	emit send_msg(device_data, this->objectName()); 
}

void PressureUnit::closePressureUnit()
{
	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(PRESS_UNIT_CLOSE, "Move Pressure Unit forward", id);

	emit send_msg(device_data, this->objectName()); 
}

void PressureUnit::stopPressureUnit()
{
	uint32_t id = COM_Message::generateMsgId();
	DeviceData *device_data = new DeviceData(PRESS_UNIT_STOP, "Move Pressure Unit forward", id);

	emit send_msg(device_data, this->objectName()); 
}