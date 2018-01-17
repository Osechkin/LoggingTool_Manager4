#include <QColorDialog>

#include "processing_settings_dialog.h"


PicFrame::PicFrame(QWidget * parent) : QFrame(parent, /*Qt::Window |*/Qt::FramelessWindowHint)
{
	setAttribute(Qt::WA_TranslucentBackground);

	pixmap.load(":pictures/images/formula3.png");
	resize(pixmap.size());
}

void PicFrame::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.drawPixmap(0, 0, width(), height(), pixmap);
}


ProcessingSettingsDialog::ProcessingSettingsDialog(const ProcessingRelax &proc_relax, QWidget *parent)
{
	this->setupUi(this);
	this->setParent(parent);

	gboxExtrapolation->setVisible(false);

	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	//this->layout()->setSizeConstraint(QLayout::SetFixedSize);

	//this->setWindowTitle(tr("Axis Settings Wizard"));
	this->setModal(true);
	//this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);  	
	setWindowFlags(Qt::Dialog|Qt::WindowTitleHint); // в заголовке окна остается только кнопка закрытия окна

	tbtInfo1->setIcon(QIcon(":/images/Info.png"));

	QLabel *lbl_Y04 = new QLabel(gboxExtrapolation);
	lbl_Y04->setText(QString("Y<sub>n</sub> = exp(a*i + b), where i &lt;= I"));
	horizontalLayout->addWidget(lbl_Y04);

	QSpacerItem *sp4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addSpacerItem(sp4);
	
	//processing_relax = proc_relax;
	is_centered = proc_relax.is_centered;
	is_smoothed = proc_relax.is_smoothed;
	is_extrapolated = proc_relax.is_extrapolated;
	is_quality_controlled = proc_relax.is_quality_controlled;
	smoothing_number = proc_relax.smoothing_number;
	extrapolation_number = proc_relax.extrapolation_number;
	alpha = proc_relax.alpha;
	I = proc_relax.I;
	K = proc_relax.K;

	T2_from = proc_relax.T2_from;
	T2_to = proc_relax.T2_to;
	T2_min = proc_relax.T2_min;
	T2_max = proc_relax.T2_max;
	T2_cutoff = proc_relax.T2_cutoff;
	T2_cutoff_clay = proc_relax.T2_cutoff_clay;
	T2_points = proc_relax.T2_points;
	iters = proc_relax.iters;
	MCBWcolor = proc_relax.MCBWcolor;
	MBVIcolor = proc_relax.MBVIcolor;
	MFFIcolor = proc_relax.MFFIcolor;

	switch (smoothing_number)
	{
	//case 1: rbt_Y01->setChecked(true); break;
	case 2: rbt_Y02->setChecked(true); break; //sbxI->setEnabled(false); sbxK->setEnabled(false); break;
	case 3: rbt_Y03->setChecked(true); break; //sbxI->setEnabled(false); sbxK->setEnabled(false); break;	
	}

	switch (extrapolation_number)
	{
	case 1: rbt_Y04->setChecked(true); sbxI->setEnabled(true); sbxK->setEnabled(true); break;
	}

	chboxQC->setChecked(is_quality_controlled);
	
	qwtCounter->setValue(alpha);
	sbxI->setValue(I);
	sbxK->setValue(K);

	gboxSmoothing->setChecked(is_smoothed);
	gboxExtrapolation->setChecked(is_extrapolated);
	chboxCentered->setChecked(is_centered);
		
	dsbxT2from->setValue(T2_from);
	dsbxT2to->setValue(T2_to);
	dsbxT2min->setValue(T2_min);
	dsbxT2max->setValue(T2_max);
	dsbxMCBWmin->setValue(T2_cutoff_clay);
	dsbxMBVImin->setValue(T2_cutoff);
	sbxIters->setValue(iters);
	sbxT2counts->setValue(T2_points);

	QPalette palette = ledMCBWcolor->palette();
	palette.setColor(ledMCBWcolor->backgroundRole(), MCBWcolor);	
	ledMCBWcolor->setPalette(palette);
	palette = ledMBVIcolor->palette();
	palette.setColor(ledMBVIcolor->backgroundRole(), MBVIcolor);	
	ledMBVIcolor->setPalette(palette);
	palette = ledMFFIcolor->palette();
	palette.setColor(ledMFFIcolor->backgroundRole(), MFFIcolor);	
	ledMFFIcolor->setPalette(palette);

	tbtMCBWcolor->setIcon(QIcon(":/images/color.png"));
	tbtMBVIcolor->setIcon(QIcon(":/images/color.png"));
	tbtMFFIcolor->setIcon(QIcon(":/images/color.png"));

	//connect(rbt_Y01, SIGNAL(toggled(bool)), this, SLOT(selectSmoothingNumber(bool)));
	connect(rbt_Y02, SIGNAL(toggled(bool)), this, SLOT(selectSmoothingNumber(bool)));
	connect(rbt_Y03, SIGNAL(toggled(bool)), this, SLOT(selectSmoothingNumber(bool)));
	connect(rbt_Y04, SIGNAL(toggled(bool)), this, SLOT(selectExtrapolationNumber(bool)));
	connect(qwtCounter, SIGNAL(valueChanged(double)), this, SLOT(setAlpha(double)));
	connect(sbxI, SIGNAL(valueChanged(int)), this, SLOT(setI(int)));
	connect(sbxK, SIGNAL(valueChanged(int)), this, SLOT(setK(int)));
	connect(gboxSmoothing, SIGNAL(toggled(bool)), this, SLOT(setSmoothing(bool)));
	connect(chboxCentered, SIGNAL(toggled(bool)), this, SLOT(setCentered(bool)));
	connect(gboxExtrapolation, SIGNAL(toggled(bool)), this, SLOT(setExtrapolated(bool)));
	connect(chboxQC, SIGNAL(toggled(bool)), this, SLOT(setQualityControlled(bool)));

	connect(dsbxT2from, SIGNAL(valueChanged(double)), this, SLOT(setT2from(double)));
	connect(dsbxT2to, SIGNAL(valueChanged(double)), this, SLOT(setT2to(double)));
	connect(dsbxT2min, SIGNAL(valueChanged(double)), this, SLOT(setT2min(double)));
	connect(dsbxT2max, SIGNAL(valueChanged(double)), this, SLOT(setT2max(double)));
	connect(dsbxMBVImin, SIGNAL(valueChanged(double)), this, SLOT(setT2cutoff(double)));
	connect(dsbxMCBWmin, SIGNAL(valueChanged(double)), this, SLOT(setT2cutoff_clay(double)));
	connect(sbxIters, SIGNAL(valueChanged(int)), this, SLOT(setIterations(int)));
	connect(sbxT2counts, SIGNAL(valueChanged(int)), this, SLOT(setT2points(int)));
	connect(tbtMCBWcolor, SIGNAL(clicked()), this, SLOT(setMCBWcolor()));
	connect(tbtMBVIcolor, SIGNAL(clicked()), this, SLOT(setMBVIcolor()));
	connect(tbtMFFIcolor, SIGNAL(clicked()), this, SLOT(setMFFIcolor()));
}

void ProcessingSettingsDialog::selectSmoothingNumber(bool flag)
{
	QRadioButton *rbt = (QRadioButton*)sender();
	if (rbt == rbt_Y02 && flag) smoothing_number = 2;
	else if (rbt == rbt_Y03 && flag) smoothing_number = 3;	
}

void ProcessingSettingsDialog::selectExtrapolationNumber(bool flag)
{
	QRadioButton *rbt = (QRadioButton*)sender();
	if (rbt == rbt_Y04 && flag) extrapolation_number = 1;
}

void ProcessingSettingsDialog::setQualityControlled(bool flag)
{
	is_quality_controlled = flag;
}

void ProcessingSettingsDialog::setAlpha(double value)
{
	alpha = value;	
}

void ProcessingSettingsDialog::setI(int value)
{
	I = value;	
}

void ProcessingSettingsDialog::setK(int value)
{
	K = value;	
}

void ProcessingSettingsDialog::setSmoothing(bool flag)
{
	is_smoothed = flag;	
}

void ProcessingSettingsDialog::setCentered(bool flag)
{
	is_centered = flag;	
}

void ProcessingSettingsDialog::setExtrapolated(bool flag)
{
	is_extrapolated = flag;
}

void ProcessingSettingsDialog::setT2from(double value)
{
	if (value >= T2_to)
	{
		dsbxT2from->setValue(T2_to-1.0);
		value = dsbxT2from->value();
	}
	T2_from = value;

	/*QColor dsbx_color = QColor(0,0,0);
	if (value >= T2_to) 
	{
		dsbx_color = QColor(255, 0, 0);
	}	

	QPalette p = dsbxT2from->palette();
	p.setColor(QPalette::Text, dsbx_color);
	dsbxT2from->setPalette(p);
	dsbxT2from->update();*/


}

void ProcessingSettingsDialog::setT2to(double value)
{
	if (value <= T2_from)
	{
		dsbxT2to->setValue(T2_from+1.0);
		value = dsbxT2to->value();
	}
	T2_to = value;
}

void ProcessingSettingsDialog::setT2min(double value)
{
	if (value >= T2_max)
	{
		dsbxT2max->setValue(T2_min+1.0);
		value = dsbxT2max->value();
	}
	T2_min = value;
}

void ProcessingSettingsDialog::setT2max(double value)
{
	if (value <= T2_min)
	{
		dsbxT2min->setValue(T2_max-1.0);
		value = dsbxT2min->value();
	}
	T2_max = value;
}

void ProcessingSettingsDialog::setT2cutoff(double value)
{
	if (value <= T2_cutoff_clay)
	{
		dsbxMBVImin->setValue(T2_cutoff_clay+1.0);
		value = dsbxMBVImin->value();
	}
	T2_cutoff = value;
}

void ProcessingSettingsDialog::setT2cutoff_clay(double value)
{
	if (value >= T2_cutoff)
	{
		dsbxMCBWmin->setValue(T2_cutoff-1.0);
		value = dsbxMBVImin->value();
	}
	T2_cutoff_clay = value;
}

void ProcessingSettingsDialog::setT2points(int value)
{
	T2_points = value;
}

void ProcessingSettingsDialog::setIterations(int value)
{
	iters = value;
}

void ProcessingSettingsDialog::setMCBWcolor()
{
	QColor color = QColorDialog::getColor(MCBWcolor);
	if (color.isValid())
	{		
		QPalette p = ledMCBWcolor->palette();
		p.setBrush(ledMCBWcolor->backgroundRole(), QBrush(color));		
		ledMCBWcolor->setPalette(p);

		MCBWcolor = color;
	}
}

void ProcessingSettingsDialog::setMBVIcolor()
{
	QColor color = QColorDialog::getColor(MBVIcolor);
	if (color.isValid())
	{
		QPalette p = ledMBVIcolor->palette();
		p.setBrush(ledMBVIcolor->backgroundRole(), QBrush(color));
		ledMBVIcolor->setPalette(p);

		MBVIcolor = color;
	}
}

void ProcessingSettingsDialog::setMFFIcolor()
{
	QColor color = QColorDialog::getColor(MFFIcolor);
	if (color.isValid())
	{
		QPalette p = ledMFFIcolor->palette();
		p.setBrush(ledMFFIcolor->backgroundRole(), QBrush(color));
		ledMFFIcolor->setPalette(p);

		MFFIcolor = color;
	}
}