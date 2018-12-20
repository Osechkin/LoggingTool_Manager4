#include "experiment_settings_dialog.h"


ExperimentSettingsDialog::ExperimentSettingsDialog(QSettings *settings, bool dont_show_again, QObject *parent /* = 0 */)
{
	setupUi(this);
	setWindowTitle(tr("Experiment Settings Dialog"));

	app_settings = settings;

	dont_show_again = dont_show_again;
	chboxDontShow->setChecked(dont_show_again);

	if (app_settings->contains("ExperimentSettings/WellUID")) well_UID = app_settings->value("ExperimentSettings/WellUID").toString();
	else well_UID = "";
	ledWellUID->setText(well_UID);

	if (app_settings->contains("ExperimentSettings/WellName")) well_name = app_settings->value("ExperimentSettings/WellName").toString();
	else well_name = "";
	ledWell->setText(well_name);

	if (app_settings->contains("ExperimentSettings/Field")) field_name = app_settings->value("ExperimentSettings/Field").toString();
	else field_name = "";
	ledField->setText(field_name);

	if (app_settings->contains("ExperimentSettings/Location")) location = app_settings->value("ExperimentSettings/Location").toString();
	else location = "";
	ledLocation->setText(location);

	if (app_settings->contains("ExperimentSettings/Province")) province = app_settings->value("ExperimentSettings/Province").toString();
	else province = "";
	ledProvince->setText(province);

	if (app_settings->contains("ExperimentSettings/Country")) country = app_settings->value("ExperimentSettings/Country").toString();
	else country = "";
	ledCountry->setText(country);

	if (app_settings->contains("ExperimentSettings/ServiceCompany")) service_company = app_settings->value("ExperimentSettings/ServiceCompany").toString();
	else service_company = "";
	ledServiceCompany->setText(service_company);

	if (app_settings->contains("ExperimentSettings/Company")) company = app_settings->value("ExperimentSettings/Company").toString();
	else company = "";
	ledCompany->setText(company);

	if (app_settings->contains("ExperimentSettings/Operator")) oper = app_settings->value("ExperimentSettings/Operator").toString();
	else oper = "";
	ledOper->setText(oper);

	if (app_settings->contains("Tool/Type")) tool = app_settings->value("Tool/Type").toString();
	else tool = "";
	ledTool->setText(tool);
	ledTool->setReadOnly(true);

	date = QDate::currentDate();
	dateEdit->setDate(date);

	setConnections();
}

void ExperimentSettingsDialog::setConnections()
{
	connect(ledWellUID, SIGNAL(textEdited(QString)), this, SLOT(setWellUID(QString)));
	connect(ledWell, SIGNAL(textEdited(QString)), this, SLOT(setWellName(QString)));
	connect(ledField, SIGNAL(textEdited(QString)), this, SLOT(setFieldName(QString)));
	connect(ledLocation, SIGNAL(textEdited(QString)), this, SLOT(setLocation(QString)));
	connect(ledProvince, SIGNAL(textEdited(QString)), this, SLOT(setProvince(QString)));
	connect(ledCountry, SIGNAL(textEdited(QString)), this, SLOT(setCountry(QString)));
	connect(ledServiceCompany, SIGNAL(textEdited(QString)), this, SLOT(setServiceCompany(QString)));
	connect(ledCompany, SIGNAL(textEdited(QString)), this, SLOT(setCompany(QString)));
	connect(ledOper, SIGNAL(textEdited(QString)), this, SLOT(setOperator(QString)));
	connect(dateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(changeDate(const QDate&)));
	connect(chboxDontShow, SIGNAL(toggled(bool)), this, SLOT(changeDontShow(bool)));
}

void ExperimentSettingsDialog::changeDate(const QDate &_date)
{
	date = _date;
}