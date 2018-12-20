#include <QApplication>
#include <QDesktopWidget>

#include "tools_general.h"

#include "depth_emul_widget.h"


DepthEmulatorWidget::DepthEmulatorWidget(Clocker *_clocker, QWidget *parent) : ui(new Ui::DepthEmulatorWidget)
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
	
	QFont font1 = ui->lblDepth->font();
	font1.setPointSize(fontSize);
	ui->lblDepth->setText("<font color=darkblue>Depth:</font>");
	
	font1.setBold(false);
	ui->lblDepth->setFont(font1);
	
	ui->lblRate->setText("<font color=darkblue>Rate:</font>");	
	ui->lblRate->setFont(font1);

	ui->cboxDepth->setFont(font1);
	ui->cboxRate->setFont(font1);

	ui->pbtStart->setFont(font1);
	ui->pbtStop->setFont(font1);
	ui->pbtStart->setIconSize(QSize(iconSize,iconSize));
	ui->pbtStop->setIconSize(QSize(iconSize,iconSize));
	ui->pbtStart->setIcon(QIcon(":/images/start.png"));
	ui->pbtStop->setIcon(QIcon(":/images/stop.png"));

	ui->cboxDepth->setStyleSheet("QComboBox { color: darkblue }");
	ui->cboxRate->setStyleSheet("QComboBox { color: darkblue }");

	depth_units_list << "m" << "ft";
	rate_units_list << "m/hr" << "m/s"<< "ft/s" << "ft/hr";
	ui->cboxDepth->addItems(depth_units_list);
	ui->cboxRate->addItems(rate_units_list);

	ui->dsbxDepth->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	ui->dsbxRate->setStyleSheet("QDoubleSpinBox { color: darkGreen }");
	font1.setBold(true);
	ui->dsbxDepth->setFont(font1);
	ui->dsbxRate->setFont(font1);
	
	k_depth = 1;
	k_rate = 3600;
	
	depth = 0;
	rate = 0;	
	//depth_flag = true;
	//rate_flag = false;	

	is_started = false;

	clocker = _clocker;
	dt = clocker->period()/1000.0;

	timer.setInterval(1000);
	timer.start();
		
	setConnection();	
}

DepthEmulatorWidget::~DepthEmulatorWidget()
{
	delete ui;		
}

void DepthEmulatorWidget::setConnection()
{
	connect(ui->cboxDepth, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->cboxRate, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeUnits(QString)));
	connect(ui->dsbxDepth, SIGNAL(valueChanged(double)), this, SLOT(setNewDepth(double)));
	connect(ui->dsbxRate, SIGNAL(valueChanged(double)), this, SLOT(setNewRate(double)));

	connect(ui->pbtStart, SIGNAL(clicked(bool)), this, SLOT(startDepth(bool)));
	connect(ui->pbtStop, SIGNAL(clicked()), this, SLOT(stopDepth()));

	connect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
	connect(&timer, SIGNAL(timeout()), this, SLOT(showNewDepth()));
}

void DepthEmulatorWidget::stopDepthMeter()
{
	timer.stop();

	disconnect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
}

void DepthEmulatorWidget::startDepthMeter()
{
	timer.start(1000);

	connect(clocker, SIGNAL(clock()), this, SLOT(clocked()));
}

void DepthEmulatorWidget::changeUnits(QString str)
{
	QComboBox *cbox = (QComboBox*)sender();
	if (!cbox) return;

	if (cbox == ui->cboxDepth)
	{		
		if (str == depth_units_list[0]) k_depth = 1;				// [m]
		else if (str == depth_units_list[1]) k_depth = 1/0.3048;	// [ft]
		else k_depth = 0;
		
		showData(DEPTH_DATA, depth);
	}
	else if (cbox == ui->cboxRate)
	{		
		if (str == rate_units_list[0]) { k_rate = 3600;	ui->dsbxRate->setSingleStep(1.0); }					// [m/hrs]
		else if (str == rate_units_list[1]) { k_rate = 1; ui->dsbxRate->setSingleStep(0.01); }				// [m/s]
		else if (str == rate_units_list[2]) { k_rate = 3600/0.3048; ui->dsbxRate->setSingleStep(5.0); }		// [ft/hrs] 
		else if (str == rate_units_list[3]) { k_rate = 1/0.3048; ui->dsbxRate->setSingleStep(0.05); }		// [ft/s]
		else k_rate = 0;

		showData(RATE_DATA, rate);
	}	
}

void DepthEmulatorWidget::showNewDepth()
{
	if (is_started)
	{
		ui->dsbxDepth->setValue(depth);
	}	
}

void DepthEmulatorWidget::startDepth(bool flag)
{
	if (!flag) 
	{
		ui->pbtStart->setChecked(true);
		return;
	}

	is_started = flag;
}

void DepthEmulatorWidget::stopDepth()
{
	ui->pbtStart->setChecked(false);
	is_started = false;
}

void DepthEmulatorWidget::clocked()
{
	if (is_started) 
	{
		depth = depth - rate*dt;
	}
}

double DepthEmulatorWidget::newDepth()
{	
	//depth = depth - rate*dt;
	return depth;
}

void DepthEmulatorWidget::setNewDepth(double val)
{
	depth = val/k_depth;
}

void DepthEmulatorWidget::setNewRate(double val)
{
	rate = val/k_rate;
}

void DepthEmulatorWidget::showData(uint8_t type, double val)
{		
	switch (type)
	{
	case DEPTH_DATA:	
		{				
			disconnect(ui->dsbxDepth, SIGNAL(valueChanged(double)), this, SLOT(setNewDepth(double)));
			depth = val;
			ui->dsbxDepth->setValue(depth * k_depth);
			connect(ui->dsbxDepth, SIGNAL(valueChanged(double)), this, SLOT(setNewDepth(double)));
			break;
		}
	case RATE_DATA:		
		{
			disconnect(ui->dsbxRate, SIGNAL(valueChanged(double)), this, SLOT(setNewRate(double)));
			rate = val;  
			ui->dsbxRate->setValue(rate * k_rate);
			connect(ui->dsbxRate, SIGNAL(valueChanged(double)), this, SLOT(setNewRate(double)));
			break;
		}	
	case DEVICE_SEARCH:	
		{
			ui->dsbxDepth->setValue(0); 
			ui->dsbxRate->setValue(0);			
			break;
		}
	default: break;
	}
}