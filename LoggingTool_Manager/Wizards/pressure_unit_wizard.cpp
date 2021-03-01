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

	data.mtr_adc_value = 0;
	data.mtr_counter = 0;
	data.mtr_status = 28;		// = 00011100 bits
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

void PressureUnit::setData(QVector<double> *_data)
{
	unsigned int mtr_adc_value = (unsigned int)(_data->first());
	signed int mtr_counter = (int)(_data->at(1));
	unsigned int mtr_status = (unsigned int)(_data->at(2));

	data.mtr_adc_value = mtr_adc_value;
	data.mtr_counter = mtr_counter;
	data.mtr_status = mtr_status;
	x1 = int(data.mtr_counter*DL/KRED*1000)/4.0;	// *1000 чтобы перевести в мм, 4 - количество полюсов двигателя (дают 4 импульса за оборот)
	double k = 1.0;									// калибровочный коэффициент - определить !
	x2 = int(data.mtr_adc_value*k*1000);			// *1000 чтобы перевести в мм

	int koncevik1 = (data.mtr_status & 0x8) >> 3;
	int koncevik2 = (data.mtr_status & 0x10) >> 4;

	if (koncevik1 > 0) ui->label1->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	else ui->label1->setPixmap(QPixmap(QString::fromUtf8(":/images/Gray Ball.png")));

	if (koncevik2 > 0) ui->label2->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	else ui->label2->setPixmap(QPixmap(QString::fromUtf8(":/images/Gray Ball.png")));

	QString x1_str = QString::number(x1);
	QString x2_str = QString::number(x2);
	ui->ledVint->setText(x1_str);
	ui->ledShtok->setText(x2_str);
}