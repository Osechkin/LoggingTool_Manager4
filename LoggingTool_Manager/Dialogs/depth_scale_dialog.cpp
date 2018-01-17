#include "depth_scale_dialog.h"


DepthScaleDialog::DepthScaleDialog(double from, double to, QWidget *parent /* = 0 */)
{
	setupUi(this);

	QDoubleValidator *v = new QDoubleValidator(this);
	QLocale loc = QLocale::c();
	loc.setNumberOptions(QLocale::RejectGroupSeparator | QLocale::OmitGroupSeparator);
	v->setLocale(loc);

	//ledFrom->setValidator(new QDoubleValidator(this));
	//ledTo->setValidator(new QDoubleValidator(this));	
	ledFrom->setValidator(v);
	ledTo->setValidator(v);
	
	ledFrom->setText(QString::number(from));
	ledTo->setText(QString::number(to));	
}

bool DepthScaleDialog::getMinimumDepth(double& min_depth)
{
	bool ok;
	min_depth = ledFrom->text().toDouble(&ok);

	return ok;
}

bool DepthScaleDialog::getMaximumDepth(double& max_depth)
{
	bool ok;
	max_depth = ledTo->text().toDouble(&ok);

	return ok;
}