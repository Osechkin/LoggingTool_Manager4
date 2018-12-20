#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>

#include "distance_meter_widget.h"


static uint32_t uid = 0;

LeuzeDistanceMeterWidget::LeuzeDistanceMeterWidget(QSettings *_settings, Clocker *_clocker, COM_PORT *com_port, COM_PORT *stepmotor_com_port, QWidget *parent /* = 0 */)  : ui(new Ui::LeuzeDistanceMeter)
{
	ui->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

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
	//double ratio = sqrt(cur_x*cur_x + cur_y*cur_y)/sqrt(1920*1920 + 1080*1080);
	QFont font = ui->tabWidget->font();
	//font.setPointSize(10*ratio);
	font.setPointSize(fontSize);
	ui->cboxDistance->setFont(font);
	ui->lblPosition->setFont(font);
	ui->ledDistance->setFont(font);
	ui->lblSetPosition->setFont(font);
	ui->dsboxSetPosition->setFont(font);
	ui->label_7->setFont(font);
	ui->pbtSet->setFont(font);
	ui->pbtBegin->setFont(font);
	ui->pbtBack->setFont(font);
	ui->pbtForward->setFont(font);
	ui->pbtEnd->setFont(font);
	ui->pbtBegin->setIconSize(QSize(iconSize,iconSize));
	ui->pbtBack->setIconSize(QSize(iconSize,iconSize));
	ui->pbtForward->setIconSize(QSize(iconSize,iconSize));
	ui->pbtEnd->setIconSize(QSize(iconSize,iconSize));
	ui->lblCoreDiameter->setFont(font);
	ui->dsboxCoreDiameter->setFont(font);
	ui->label_5->setFont(font);
	ui->pbtConnect->setFont(font);
	ui->lblFrom->setFont(font);
	ui->dsboxFrom->setFont(font);
	ui->label->setFont(font);
	ui->lblTo->setFont(font);
	ui->dsboxTo->setFont(font);
	ui->label_2->setFont(font);
	ui->lblStep->setFont(font);
	ui->dsboxStep->setFont(font);
	ui->label_3->setFont(font);
	ui->lblCalibrLen->setFont(font);
	ui->dsboxCalibrLength->setFont(font);
	ui->label_4->setFont(font);
	ui->lblPorosity->setFont(font);
	ui->dsboxPorosity->setFont(font);
	ui->label_8->setFont(font);
	ui->lblZero->setFont(font);
	ui->dsboxZero->setFont(font);
	ui->label_6->setFont(font);
	ui->pbtSetZero->setFont(font);
	ui->pbtClear->setFont(font);

	app_settings = _settings;

	ui->ledDistance->setText("");
	ui->cboxDistance->setStyleSheet("QComboBox { color: darkblue }");
	//ui->cboxSetDistance->setStyleSheet("QComboBox { color: darkblue }");
	
	ui->lblPosition->setStyleSheet("QLabel { color: darkblue }");
	ui->ledDistance->setStyleSheet("QLineEdit { color: darkblue }");
	ui->tabWidget->setStyleSheet("QTableWidget { color: darkblue }");
	ui->lblSetPosition->setStyleSheet("QLabel { color: darkblue }");

	ui->lblFrom->setStyleSheet("QLabel { color: darkblue }");
	ui->lblTo->setStyleSheet("QLabel { color: darkblue }");
	ui->lblStep->setStyleSheet("QLabel { color: darkblue }");
	ui->lblZero->setStyleSheet("QLabel { color: darkblue }");
	ui->lblCalibrLen->setStyleSheet("QLabel { color: darkblue }");
	ui->lblCoreDiameter->setStyleSheet("QLabel { color: darkblue }");
	ui->lblPorosity->setStyleSheet("QLabel { color: darkblue }");
	//ui->cboxFrom->setStyleSheet("QComboBox { color: darkblue }");
	//ui->cboxTo->setStyleSheet("QComboBox { color: darkblue }");
	//ui->cboxStep->setStyleSheet("QComboBox { color: darkblue }");
	//ui->cboxFrom->setStyleSheet("QComboBox { color: darkblue }");
	//ui->cboxZero->setStyleSheet("QComboBox { color: darkblue }");
	
	ui->ledDistance->setStyleSheet("QLineEdit { color: darkGreen }");
	ui->dsboxSetPosition->setStyleSheet("QDoubleSpinBox { color: darkBlue }");	

	ui->dsboxFrom->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->dsboxTo->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->dsboxStep->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->dsboxZero->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->dsboxCalibrLength->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->dsboxCoreDiameter->setStyleSheet("QDoubleSpinBox { color: darkblue }");
	ui->dsboxPorosity->setStyleSheet("QDoubleSpinBox { color: darkGreen }");

	ui->pbtBack->setIcon(QIcon(":/images/play_back.png"));
	ui->pbtBegin->setIcon(QIcon(":/images/play_begin.png"));
	ui->pbtForward->setIcon(QIcon(":/images/play.png"));
	ui->pbtEnd->setIcon(QIcon(":/images/play_end.png"));
	ui->pbtSet->setIcon(QIcon(":/images/apply.png"));
	
	ui->pbtConnect->setText(tr("Connect to Rock Movement System"));
	ui->pbtConnect->setIcon(QIcon(":/images/add.png"));
	
	connectionWidget = new ImpulsConnectionWidget(this);
	connectionWidget->setReport(ConnectionState::State_No);

	distance = 0.001;		// always in meters
	k_distance = 100;		// for default units ([cm])
	distance_ok = true;

	set_distance = 1;		// [m]
	k_set_distance = 100;	// [cm]
	direction_coef = -1;	
	pos_is_set = false;

	from_pos = 0.00;		// [m]
	to_pos = 1.00;			// [m]
	step_pos = 0.01;		// [m]
	zero_pos = 0;			// [m]
	calibr_len = 0;			// [m]
	core_diameter = 0.1;	// [m]
	standard_porosity = 30;	// [%]	
	k_from = 100;			// for [cm]
	k_to = 100;				// for [cm]
	k_step = 100;			// for [cm]
	k_zero = 100;			// for [cm]
	k_calibr_len = 100;		// for [cm]
	k_core_d = 1000;		// for [mm]
	k_standard_porosity = 1;// to [%]

	upper_bound = 1.65;	// m
	lower_bound = 0.55;	// m	

	bool _ok;
	double _from_pos = from_pos;
	double _to_pos = to_pos;
	double _step_pos = step_pos;
	double _zero_pos = zero_pos;
	double _calibr_len = calibr_len;
	double _upper_bound = upper_bound;
	double _lower_bound = lower_bound;
	double _core_diameter = core_diameter;
	double _standard_porosity = standard_porosity;
	double _standard_diameter = standard_diameter;
	if (app_settings->contains("CoreTransportSystem/From")) _from_pos = app_settings->value("CoreTransportSystem/From").toDouble(&_ok);
	if (_ok) from_pos = _from_pos;
	if (app_settings->contains("CoreTransportSystem/To")) _to_pos = app_settings->value("CoreTransportSystem/To").toDouble(&_ok);
	if (_ok) to_pos = _to_pos;
	if (app_settings->contains("CoreTransportSystem/Step")) _step_pos = app_settings->value("CoreTransportSystem/Step").toDouble(&_ok);
	if (_ok) step_pos = _step_pos;
	if (app_settings->contains("CoreTransportSystem/Zero")) _zero_pos = app_settings->value("CoreTransportSystem/Zero").toDouble(&_ok);
	if (_ok) zero_pos = _zero_pos;
	if (app_settings->contains("CoreTransportSystem/LowerBound")) _lower_bound = app_settings->value("CoreTransportSystem/LowerBound").toDouble(&_ok);
	if (_ok) lower_bound = _lower_bound;
	if (app_settings->contains("CoreTransportSystem/UpperBound")) _upper_bound = app_settings->value("CoreTransportSystem/UpperBound").toDouble(&_ok);
	if (_ok) upper_bound = _upper_bound;
	if (app_settings->contains("CoreTransportSystem/CalibrationLength")) _calibr_len = app_settings->value("CoreTransportSystem/CalibrationLength").toDouble(&_ok);
	if (_ok) calibr_len = _calibr_len;
	if (app_settings->contains("CoreTransportSystem/CoreDiameter")) _core_diameter = app_settings->value("CoreTransportSystem/CoreDiameter").toDouble(&_ok);
	if (_ok) core_diameter = _core_diameter;
	if (app_settings->contains("CoreTransportSystem/StandardPorosity")) _standard_porosity = app_settings->value("CoreTransportSystem/StandardPorosity").toDouble(&_ok);
	if (_ok) standard_porosity = _standard_porosity;
	if (app_settings->contains("CoreTransportSystem/StandardCoreDiameter")) _standard_diameter = app_settings->value("CoreTransportSystem/StandardCoreDiameter").toDouble(&_ok);
	if (_ok) standard_diameter = _standard_diameter;
	
	distance_units_list << "mm" << "cm" << "m";	
		
	ui->cboxDistance->addItems(distance_units_list);
	ui->cboxDistance->setCurrentIndex(1);
	ui->cboxDistance->setEnabled(true);
	
	ui->ledDistance->setText("");	
		
	ui->dsboxSetPosition->setMinimum(-200);
	ui->dsboxSetPosition->setMaximum(200);
	ui->dsboxSetPosition->setSingleStep(1);
	ui->dsboxSetPosition->setValue(set_distance*k_set_distance);

	ui->dsboxFrom->setMinimum(-200);
	ui->dsboxFrom->setMaximum(200);
	ui->dsboxFrom->setSingleStep(1);
	ui->dsboxFrom->setValue(from_pos*k_from);

	ui->dsboxTo->setMinimum(-200);
	ui->dsboxTo->setMaximum(200);
	ui->dsboxTo->setSingleStep(1);
	ui->dsboxTo->setValue(to_pos*k_to);

	ui->dsboxStep->setMinimum(0.1);
	ui->dsboxStep->setMaximum(200);
	ui->dsboxStep->setSingleStep(0.1);
	ui->dsboxStep->setValue(step_pos*k_step);

	ui->dsboxZero->setMinimum(-200);
	ui->dsboxZero->setMaximum(200);
	ui->dsboxZero->setSingleStep(1);
	ui->dsboxZero->setValue(zero_pos*k_zero);

	ui->dsboxCalibrLength->setMinimum(0);
	ui->dsboxCalibrLength->setMaximum(99);
	ui->dsboxCalibrLength->setSingleStep(1);
	ui->dsboxCalibrLength->setValue(calibr_len*k_calibr_len);

	ui->dsboxCoreDiameter->setMinimum(0.1);
	ui->dsboxCoreDiameter->setMaximum(500);
	ui->dsboxCoreDiameter->setSingleStep(10);
	ui->dsboxCoreDiameter->setValue(core_diameter*k_core_d);

	ui->dsboxPorosity->setMinimum(0);
	ui->dsboxPorosity->setMaximum(100);
	ui->dsboxPorosity->setSingleStep(1);
	ui->dsboxPorosity->setValue(standard_porosity*k_standard_porosity);

	is_connected = false;
	ui->framePosControl->setEnabled(is_connected);
	//device_is_searching = false;

	clocker = _clocker;
	COM_Port = com_port;
	stepmotor_COM_Port = stepmotor_com_port;
	leuze_communicator = NULL;
	stepmotor_communicator = NULL;
		
	timer.start(250);	
	stepmotor_timer.start(250);

	setConnection();
}

LeuzeDistanceMeterWidget::~LeuzeDistanceMeterWidget()
{
	delete ui;
	delete connectionWidget;
	if (leuze_communicator != NULL) delete leuze_communicator;
	if (stepmotor_communicator != NULL) delete stepmotor_communicator;
}


void LeuzeDistanceMeterWidget::setConnection()
{
	//connect(ui->cboxDistance, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	//connect(ui->cboxFrom, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	//connect(ui->cboxTo, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	//connect(ui->cboxStep, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	//connect(ui->cboxZero, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	//connect(ui->cboxSetDistance, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->dsboxTo, SIGNAL(valueChanged(double)), this, SLOT(setNewTo(double)));
	connect(ui->dsboxFrom, SIGNAL(valueChanged(double)), this, SLOT(setNewFrom(double)));
	connect(ui->dsboxStep, SIGNAL(valueChanged(double)), this, SLOT(setNewStep(double)));
	connect(ui->dsboxCoreDiameter, SIGNAL(valueChanged(double)), this, SLOT(setNewCoreDiameter(double)));

	connect(ui->pbtBack, SIGNAL(toggled(bool)), this, SLOT(moveBack(bool)));
	connect(ui->pbtForward, SIGNAL(toggled(bool)), this, SLOT(moveForward(bool)));
	connect(ui->pbtBegin, SIGNAL(toggled(bool)), this, SLOT(moveBack(bool)));
	connect(ui->pbtEnd, SIGNAL(toggled(bool)), this, SLOT(moveForward(bool)));
	connect(ui->pbtSet, SIGNAL(toggled(bool)), this, SLOT(setPosition(bool)));
	connect(ui->pbtSetZero, SIGNAL(clicked()), this, SLOT(setZeroPos()));
	connect(ui->pbtClear, SIGNAL(clicked()), this, SLOT(clearZeroPos()));
	
	connect(ui->pbtConnect, SIGNAL(toggled(bool)), this, SLOT(connectAllMeters(bool)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(onTime()));
}


void LeuzeDistanceMeterWidget::setDepthCommunicatorConnections()
{
	connect(leuze_communicator, SIGNAL(measured_data(uint32_t, uint8_t, double)), this, SLOT(getMeasuredData(uint32_t, uint8_t, double)));
	connect(leuze_communicator, SIGNAL(data_timed_out(uint32_t, uint8_t)), this, SLOT(measureTimedOut(uint32_t, uint8_t)));
	connect(leuze_communicator, SIGNAL(error_msg(QString)), this, SLOT(showErrorMsg(QString)));
	connect(this, SIGNAL(to_measure(uint32_t, uint8_t)), leuze_communicator, SLOT(toMeasure(uint32_t, uint8_t)));
}

void LeuzeDistanceMeterWidget::setStepMotorCommunicatorConnections()
{
	
}

void LeuzeDistanceMeterWidget::moveBack(bool flag)
{	
	QPushButton *pbt = qobject_cast<QPushButton*>(sender());
	if (!pbt) return;

	pos_is_set = false;		// игнорировать заданное значение, т.е. просто двигать до остановки пользователем или до концевика

	if (!flag)
	{
		stepmotor_communicator->toSend("\SP*DS*");		// Stop step motor		
		return;
	}

	QString str_cmd = "";
	if (pbt == ui->pbtBack) 
	{
		str_cmd = "\EN*DR*SD200*MV*";			// it was 50 (changed 7.07.2018)

		ui->pbtForward->setChecked(false);
		ui->pbtEnd->setChecked(false);		
		ui->pbtBegin->setChecked(false);
		ui->pbtSet->setChecked(false);
	}
	else if (pbt == ui->pbtBegin) 
	{
		str_cmd = "\EN*DR*SD1000*MV*";

		ui->pbtForward->setChecked(false);
		ui->pbtEnd->setChecked(false);
		ui->pbtBack->setChecked(false);		
		ui->pbtSet->setChecked(false);
	}
	else return;

	/*if (!flag)
	{
		stepmotor_communicator->toSend("\SP*DS*");		// Stop step motor		
	}
	else*/
	{
		if (distance > lower_bound && distance < upper_bound)
		{
			stepmotor_communicator->toSend(str_cmd);	// Move step motor left
		}
		else 
		{
			stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
			ui->pbtBack->setChecked(false);
			ui->pbtForward->setChecked(false);
			ui->pbtBegin->setChecked(false);
			ui->pbtEnd->setChecked(false);
			ui->pbtSet->setChecked(false);
		}
	}
}

void LeuzeDistanceMeterWidget::moveForward(bool flag)
{
	QPushButton *pbt = qobject_cast<QPushButton*>(sender());
	if (!pbt) return;

	pos_is_set = false;

	if (!flag)
	{
		stepmotor_communicator->toSend("\SP*DS*");		// Stop step motor		
		return;
	}

	QString str_cmd = "";
	if (pbt == ui->pbtForward) 
	{
		str_cmd = "\EN*DL*SD200*MV*";		// it was 50 (changed 7.07.2018)

		ui->pbtBack->setChecked(false);		
		ui->pbtBegin->setChecked(false);		
		ui->pbtEnd->setChecked(false);
		ui->pbtSet->setChecked(false);
	}
	else if (pbt == ui->pbtEnd) 
	{
		str_cmd = "\EN*DL*SD1000*MV*";

		ui->pbtBack->setChecked(false);		
		ui->pbtBegin->setChecked(false);	
		ui->pbtForward->setChecked(false);		
		ui->pbtSet->setChecked(false);
	}
	else return;

	/*if (!flag)
	{
		stepmotor_communicator->toSend("\SP*DS*");		// Stop step motor		
	}
	else*/
	{
		if (distance > lower_bound && distance < upper_bound)
		{			
			stepmotor_communicator->toSend(str_cmd);	// Move step motor left
		}
		else 
		{
			stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
			ui->pbtBack->setChecked(false);
			ui->pbtForward->setChecked(false);
			ui->pbtBegin->setChecked(false);
			ui->pbtEnd->setChecked(false);
			ui->pbtSet->setChecked(false);
		}
	}
}

void LeuzeDistanceMeterWidget::setPosition(double pos)
{
	pos += zero_pos;
	if (pos == distance) emit cmd_resulted(true, 0); // if position is ready reached

	if (!is_connected) 
	{
		connectAllMeters(true);
	}
	if (!is_connected)
	{
		emit cmd_resulted(false, 0);
		return;
	}

	set_distance = pos;
	// Added 6.07.2018 - чтобы вылечить глюк движения не в ту сторону, т.к. distance не успел измениться со значения по умолчанию = 0.001
	if (distance <= 0.001)
	{
		pos_is_set = true;
		direction_coef = -1;
		stepmotor_communicator->toSend("\EN*DR*SD1000*MV*");												// Move step motor left, high speed
	}
	// ----------------------------------------------------------------------------------------------------------------------------------
	else if (set_distance > distance )
	{
		pos_is_set = true;
		direction_coef = 1;
		if (set_distance - distance < 0.011) stepmotor_communicator->toSend("\EN*DL*SD50*MV*");				// Move step motor right, very low speed 
		else if (set_distance - distance < 0.03) stepmotor_communicator->toSend("\EN*DL*SD200*MV*");		// Move step motor right, low speed		 
		else stepmotor_communicator->toSend("\EN*DL*SD1000*MV*");											// Move step motor right, high speed	 
	}
	else if (set_distance < distance )
	{
		pos_is_set = true;
		direction_coef = -1;
		if (distance - set_distance < 0.011) stepmotor_communicator->toSend("\EN*DR*SD50*MV*");				// Move step motor left, very low speed	
		else if (distance - set_distance < 0.03) stepmotor_communicator->toSend("\EN*DR*SD200*MV*");		// Move step motor left, low speed		
		else stepmotor_communicator->toSend("\EN*DR*SD1000*MV*");											// Move step motor left, high speed
	}
	else 
	{
		stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
		ui->pbtBack->setChecked(false);
		ui->pbtForward->setChecked(false);
		ui->pbtBegin->setChecked(false);
		ui->pbtEnd->setChecked(false);
		ui->pbtSet->setChecked(false);

		emit cmd_resulted(true, 0);
	}
}

void LeuzeDistanceMeterWidget::setPosition(bool flag)
{	
	if (!flag)
	{
		stepmotor_communicator->toSend("\SP*DS*");		// Stop step motor		
	}
	else
	{		
		ui->pbtBack->setChecked(false);
		ui->pbtForward->setChecked(false);		
		ui->pbtBegin->setChecked(false);			
		ui->pbtEnd->setChecked(false);		
		
		//set_distance = ui->dsboxSetPosition->value()/k_set_distance;
		set_distance = ui->dsboxSetPosition->value()/k_set_distance + zero_pos;
		if (set_distance > distance )
		{
			pos_is_set = true;
			direction_coef = 1;
			if (set_distance - distance < 0.011) stepmotor_communicator->toSend("\EN*DL*SD50*MV*");				// Move step motor right, very low speed				
			else if (set_distance - distance < 0.03) stepmotor_communicator->toSend("\EN*DL*SD200*MV*");		// Move step motor right, low speed			
			else stepmotor_communicator->toSend("\EN*DL*SD1000*MV*");											// Move step motor right, high speed
			
		}
		else if (set_distance < distance )
		{
			pos_is_set = true;
			direction_coef = -1;
			if (distance - set_distance < 0.011) stepmotor_communicator->toSend("\EN*DR*SD50*MV*");				// Move step motor left, very low speed		
			else if (distance - set_distance < 0.03) stepmotor_communicator->toSend("\EN*DR*SD200*MV*");		// Move step motor left, low speed			
			else stepmotor_communicator->toSend("\EN*DR*SD1000*MV*");											// Move step motor left, high speed
		}
		else 
		{
			stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
			ui->pbtBack->setChecked(false);
			ui->pbtForward->setChecked(false);
			ui->pbtBegin->setChecked(false);
			ui->pbtEnd->setChecked(false);
			ui->pbtSet->setChecked(false);
		}
	}
}

void LeuzeDistanceMeterWidget::setZeroPos()
{
	zero_pos = distance;
	ui->dsboxZero->setValue(zero_pos*100);	// to [cm]

	//ui->dsboxFrom->setValue((from_pos-zero_pos)/k_from);
	//ui->dsboxTo->setValue((to_pos-zero_pos)/k_to);
}

void LeuzeDistanceMeterWidget::clearZeroPos()
{
	zero_pos = 0;
	ui->dsboxZero->setValue(zero_pos*100);	// to [cm]
}

void LeuzeDistanceMeterWidget::stopDepthMeter()
{
	//ui->pbtConnect->setChecked(false);

	timer.stop();

	//disconnect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
}

void LeuzeDistanceMeterWidget::startDepthMeter()
{
	timer.start(250);

	//connect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
}


void LeuzeDistanceMeterWidget::setNewTo(double val)
{
	to_pos = val / k_to;	
	saveSettings();
}

void LeuzeDistanceMeterWidget::setNewFrom(double val)
{
	from_pos = val / k_from;	
	saveSettings();
}

void LeuzeDistanceMeterWidget::setNewStep(double val)
{
	step_pos = val / k_step;
	saveSettings();
}

void LeuzeDistanceMeterWidget::setNewCoreDiameter(double val)
{
	core_diameter = val / k_core_d;

	emit new_core_diameter(core_diameter);
	saveSettings();
}

/*void LeuzeDistanceMeterWidget::changeUnits(QString str)
{
	QComboBox *cbox = (QComboBox*)sender();
	if (!cbox) return;

	if (cbox == ui->cboxDistance)
	{		
		if (str == distance_units_list[0]) k_distance = 1000;		// [mm]
		else if (str == distance_units_list[1]) k_distance = 100;	// [cm]
		else if (str == distance_units_list[2]) k_distance = 1;		// [m]
		else k_distance = 1;

		ui->ledDistance->setText(QString::number(k_distance*distance));		
	}
	else if (cbox == ui->cboxSetDistance)
	{				
		if (str == distance_units_list[0])							// [mm]
		{
			ui->dsboxSetPosition->setMinimum(150);
			ui->dsboxSetPosition->setMaximum(2000);
			ui->dsboxSetPosition->setSingleStep(1);
			k_set_distance = 1000;		
		}
		else if (str == distance_units_list[1])						// [cm]
		{
			ui->dsboxSetPosition->setMinimum(15);
			ui->dsboxSetPosition->setMaximum(200);
			ui->dsboxSetPosition->setSingleStep(1);
			k_set_distance = 100;		
		}
		else if (str == distance_units_list[2])						// [m]
		{
			ui->dsboxSetPosition->setMinimum(0.15);
			ui->dsboxSetPosition->setMaximum(2.00);
			ui->dsboxSetPosition->setSingleStep(0.01);
			k_set_distance = 1;		
		}
		else k_set_distance = 1;

		ui->dsboxSetPosition->setValue(k_set_distance*set_distance);		
	}
	else if (cbox == ui->cboxFrom)
	{
		disconnect(ui->dsboxFrom, SIGNAL(valueChanged(double)), this, SLOT(setNewFrom(double)));
		if (str == distance_units_list[0]) 
		{
			k_from = 1000;											// [mm]
			ui->dsboxFrom->setMinimum(150);
			ui->dsboxFrom->setMaximum(2000);
			ui->dsboxFrom->setSingleStep(1);
		}
		else if (str == distance_units_list[1]) 
		{
			k_from = 100;											// [cm]
			ui->dsboxFrom->setMinimum(15);
			ui->dsboxFrom->setMaximum(200);
			ui->dsboxFrom->setSingleStep(1);
		}
		else if (str == distance_units_list[2]) 
		{
			k_from = 1;												// [m]
			ui->dsboxFrom->setMinimum(0.150);
			ui->dsboxFrom->setMaximum(2.000);
			ui->dsboxFrom->setSingleStep(0.01);
		}
		else k_from = 1;

		ui->dsboxFrom->setValue(k_from*from_pos);	
		connect(ui->dsboxFrom, SIGNAL(valueChanged(double)), this, SLOT(setNewFrom(double)));
	}
	else if (cbox == ui->cboxTo)
	{
		disconnect(ui->dsboxTo, SIGNAL(valueChanged(double)), this, SLOT(setNewTo(double)));
		if (str == distance_units_list[0]) 
		{
			k_to = 1000;											// [mm]
			ui->dsboxTo->setMinimum(150);
			ui->dsboxTo->setMaximum(2000);
			ui->dsboxTo->setSingleStep(1);
		}
		else if (str == distance_units_list[1]) 
		{
			k_to = 100;												// [cm]
			ui->dsboxTo->setMinimum(15);
			ui->dsboxTo->setMaximum(200);
			ui->dsboxTo->setSingleStep(1);
		}
		else if (str == distance_units_list[2]) 
		{
			k_to = 1;												// [m]
			ui->dsboxTo->setMinimum(0.150);
			ui->dsboxTo->setMaximum(2.000);
			ui->dsboxTo->setSingleStep(0.01);
		}
		else k_to = 1;

		ui->dsboxTo->setValue(k_to*to_pos);	
		connect(ui->dsboxTo, SIGNAL(valueChanged(double)), this, SLOT(setNewTo(double)));
	}
	else if (cbox == ui->cboxStep)
	{
		disconnect(ui->dsboxStep, SIGNAL(valueChanged(double)), this, SLOT(setNewStep(double)));
		if (str == distance_units_list[0]) 
		{
			k_step = 1000;											// [mm]
			ui->dsboxStep->setMinimum(1);
			ui->dsboxStep->setMaximum(2000);
			ui->dsboxStep->setSingleStep(1);
		}
		else if (str == distance_units_list[1]) 
		{
			k_step = 100;											// [cm]
			ui->dsboxStep->setMinimum(0.1);
			ui->dsboxStep->setMaximum(200);
			ui->dsboxStep->setSingleStep(0.1);
		}
		else if (str == distance_units_list[2]) 
		{
			k_step = 1;												// [m]
			ui->dsboxStep->setMinimum(0.001);
			ui->dsboxStep->setMaximum(2.000);
			ui->dsboxStep->setSingleStep(0.001);
		}
		else k_step = 1;

		ui->dsboxStep->setValue(k_step*step_pos);	
		connect(ui->dsboxStep, SIGNAL(valueChanged(double)), this, SLOT(setNewStep(double)));
	}
	//else if (cbox == ui->cboxZero)
	//{
	//	if (str == distance_units_list[0]) 
	//	{
	//		k_zero = 1000;											// [mm]
	//		ui->dsboxZero->setMinimum(150);
	//		ui->dsboxZero->setMaximum(2000);
	//		ui->dsboxZero->setSingleStep(1);
	//	}
	//	else if (str == distance_units_list[1]) 
	//	{
	//		k_zero = 100;											// [cm]
	//		ui->dsboxZero->setMinimum(15);
	//		ui->dsboxZero->setMaximum(200);
	//		ui->dsboxZero->setSingleStep(1);
	//	}
	//	else if (str == distance_units_list[2]) 
	//	{
	//		k_zero = 1;												// [m]
	//		ui->dsboxZero->setMinimum(0.150);
	//		ui->dsboxZero->setMaximum(2.000);
	//		ui->dsboxZero->setSingleStep(0.01);
	//	}
	//	else k_zero = 1;

	//	ui->dsboxZero->setValue(k_zero*zero_pos);	
	//}
}*/


void LeuzeDistanceMeterWidget::connectAllMeters(bool flag)
{
	/* Temporary commented !
	if (!flag)
	{
		if (COM_Port->COM_port != NULL) 
		{
			COM_Port->COM_port->close();		
		}	
		if (stepmotor_COM_Port->COM_port != NULL) 
		{
			stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
			stepmotor_COM_Port->COM_port->close();	
			emit cmd_resulted(false, 0);				// inform expScheduler
		}

		if (leuze_communicator != NULL)
		{
			leuze_communicator->exit();
			leuze_communicator->wait();
			delete leuze_communicator;	
			leuze_communicator = NULL;
		}
		if (stepmotor_communicator != NULL)
		{
			stepmotor_communicator->exit();
			stepmotor_communicator->wait();
			delete stepmotor_communicator;	
			stepmotor_communicator = NULL;
		}

		timer.stop();
		is_connected = false;		
		emit connected(is_connected);
				
		ui->pbtConnect->setText(tr("Connect to Rock Movement System"));
		ui->pbtConnect->setIcon(QIcon(":/images/add.png"));
		ui->ledDistance->setText("");		
	}	
	else
	{		
		// Leuze destance meter initialization
		if (COM_Port->COM_port != NULL)
		{
			QString COMPort_Name = COM_Port->COM_port->portName();
			if (COMPort_Name.isEmpty()) 
			{
				int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Distance Meter!"), QMessageBox::Ok, QMessageBox::Ok);
				return;
			}

			COM_Port->COM_port->close();
			COM_Port->COM_port->setPortName(COMPort_Name);

			if (leuze_communicator != NULL)
			{
				leuze_communicator->exit();
				leuze_communicator->wait();
				delete leuze_communicator;
				leuze_communicator = NULL;
			}			
			bool res = COM_Port->COM_port->open(QextSerialPort::ReadWrite);
			if (res) 	
			{				
				leuze_communicator = new LeuzeCommunicator(COM_Port->COM_port, clocker);
				setDepthCommunicatorConnections();
				leuze_communicator->start(QThread::NormalPriority);

				is_connected = true;
				emit connected(true);

				timer.start(250);
				//QThread::msleep(500);		// sleep 500 ms - попытка решить проблему неправильного движения керна в начальный момент (из-за того, что иногда distance = 0.001 в момент старта)
								
				ui->pbtConnect->setText(tr("Disconnect from Rock Movement System"));
				ui->pbtConnect->setIcon(QIcon(":/images/remove.png"));				
			}
			else
			{
				is_connected = false;
				emit connected(false);

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open COM-Port (%1)!").arg(COMPort_Name), QMessageBox::Ok, QMessageBox::Ok);
			}
		}
		else
		{
			int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Distance Meter!"), QMessageBox::Ok, QMessageBox::Ok);
		}	

		// Step Motor controller initialization
		if (stepmotor_COM_Port->COM_port != NULL)
		{
			QString stepmotor_COMPort_Name = stepmotor_COM_Port->COM_port->portName();
			if (stepmotor_COMPort_Name.isEmpty()) 
			{
				int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Step Motor controller!"), QMessageBox::Ok, QMessageBox::Ok);
				return;
			}

			stepmotor_COM_Port->COM_port->close();
			stepmotor_COM_Port->COM_port->setPortName(stepmotor_COMPort_Name);

			if (stepmotor_communicator != NULL)
			{
				stepmotor_communicator->exit();
				stepmotor_communicator->wait();
				delete stepmotor_communicator;
				stepmotor_communicator = NULL;
			}			
			bool res = stepmotor_COM_Port->COM_port->open(QextSerialPort::ReadWrite);
			if (res) 	
			{				
				stepmotor_communicator = new StepMotorCommunicator(stepmotor_COM_Port->COM_port, clocker);
				setStepMotorCommunicatorConnections();
				stepmotor_communicator->start(QThread::NormalPriority);

				//is_connected = true;
				//emit connected(true);
								
				ui->pbtConnect->setText(tr("Disconnect from Rock Movement System"));
				ui->pbtConnect->setIcon(QIcon(":/images/remove.png"));				
			}
			else
			{
				is_connected = false;
				emit connected(false);

				int ret = QMessageBox::warning(this, tr("Warning!"), tr("Cannot open COM-Port (%1)!").arg(stepmotor_COMPort_Name), QMessageBox::Ok, QMessageBox::Ok);
			}
		}
		else
		{
			int ret = QMessageBox::warning(this, tr("Warning!"), tr("No available COM-Port was found to connect to Step Motor controller!"), QMessageBox::Ok, QMessageBox::Ok);
		}	
	}	
	*/
	ui->framePosControl->setEnabled(is_connected);
}

void LeuzeDistanceMeterWidget::showErrorMsg(QString msg)
{
	stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
	emit cmd_resulted(false, 0);				// Inform expScheduler  

	QPalette palette; 
	palette.setColor(QPalette::Base,Qt::red);
	palette.setColor(QPalette::Text,Qt::darkGreen);
	ui->ledDistance->setPalette(palette);

	ui->ledDistance->setText(msg);
}

void LeuzeDistanceMeterWidget::getMeasuredData(uint32_t _uid, uint8_t _type, double val)
{
	if (!is_connected) is_connected = true;
	/*if (device_is_searching) 
	{
		timer.start();
		device_is_searching = false;
	}*/

	switch (_type)
	{
	case DEPTH_DATA:	distance = val/1000;	break;
	//case DEVICE_SEARCH:	device_is_searching = false; break;
	default: break;
	}

	if (distance < lower_bound || distance > upper_bound)
	{
		distance_ok = false;
		stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
		ui->pbtBack->setChecked(false);
		ui->pbtForward->setChecked(false);

		emit cmd_resulted(true, 0);
	}
	else distance_ok = true;

	if (pos_is_set)
	{
		if (fabs(distance - set_distance) < 0.011 && direction_coef == -1)
		{		
			stepmotor_communicator->toSend("\EN*DR*SD50*MV*");			// Set motion slow 		
		}
		else if (fabs(distance - set_distance) < 0.03 && direction_coef == -1)
		{		
			stepmotor_communicator->toSend("\EN*DR*SD200*MV*");			// Set motion slow 		
		}
		else if (fabs(set_distance - distance) < 0.011 && direction_coef == 1)
		{		
			stepmotor_communicator->toSend("\EN*DL*SD50*MV*");			// Set motion slow 		
		}
		else if (fabs(set_distance - distance) < 0.03 && direction_coef == 1)
		{		
			stepmotor_communicator->toSend("\EN*DL*SD200*MV*");			// Set motion slow 		
		}

		if (distance <= set_distance && direction_coef == -1)
		{
			distance_ok = true;
			pos_is_set = false;
			stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
			ui->pbtBack->setChecked(false);
			ui->pbtForward->setChecked(false);
			ui->pbtSet->setChecked(false);

			emit cmd_resulted(true, 0);
		}
		else if (distance >= set_distance && direction_coef == 1)
		{
			distance_ok = true;
			pos_is_set = false;
			stepmotor_communicator->toSend("\SP*DS*");	// Stop step motor
			ui->pbtBack->setChecked(false);
			ui->pbtForward->setChecked(false);
			ui->pbtSet->setChecked(false);

			emit cmd_resulted(true, 0);
		}
	}
	
	showData(_type, val);
}

void LeuzeDistanceMeterWidget::measureTimedOut(uint32_t _uid, uint8_t _type)
{	
	switch (_type)
	{
	case DEPTH_DATA:	ui->ledDistance->setText("");	break;
	case DEVICE_SEARCH:	
		{
			ui->ledDistance->setText("");
			connectionWidget->setReport(ConnectionState::State_No); 
			break;
		}
	default: break;
	}
	
	connectionWidget->setReport(ConnectionState::State_No);

	emit cmd_resulted(false, 0);
}

void LeuzeDistanceMeterWidget::showData(uint8_t type, double val)
{	
	switch (type)
	{
	case DEPTH_DATA:	
		{
			QPalette palette;					
			if (distance_ok)
			{
				palette.setColor(QPalette::Base,Qt::white);
				palette.setColor(QPalette::Text,Qt::darkGreen);
				 //ui->ledDistance->setStyleSheet("QLineEdit {background-color: white;}");
			}
			else
			{
				palette.setColor(QPalette::Base,Qt::red);
				palette.setColor(QPalette::Text,Qt::darkGreen);
				//ui->ledDistance->setStyleSheet("QLineEdit {background-color: red;}");
			}
			ui->ledDistance->setPalette(palette);

			//double value = distance * k_distance;  	
			double value = distance * k_distance - zero_pos*k_zero;  	
			QString str_value = QString::number(value,'g',4);				
			ui->ledDistance->setText(str_value);
			
			break;
		}	
	case DEVICE_SEARCH:	
		{
			ui->ledDistance->setText(""); 			
			break;
		}
	default: break;
	}

	connectionWidget->setReport(ConnectionState::State_OK);
}

void LeuzeDistanceMeterWidget::onTime()
{	
	if (is_connected) emit to_measure(++uid, DEPTH_DATA);		
}

void LeuzeDistanceMeterWidget::saveSettings()
{
	app_settings->setValue("CoreTransportSystem/From", from_pos);	
	app_settings->setValue("CoreTransportSystem/To", to_pos);
	app_settings->setValue("CoreTransportSystem/Step", step_pos);
	app_settings->setValue("CoreTransportSystem/Zero", zero_pos);
	//app_settings->setValue("CoreTransportSystem/LowerBound", lower_bound);
	//app_settings->setValue("CoreTransportSystem/UpperBound", upper_bound);
	//app_settings->setValue("CoreTransportSystem/CalibrationLength", calibr_len);
	app_settings->setValue("CoreTransportSystem/CoreDiameter", core_diameter);	
	app_settings->sync();
}