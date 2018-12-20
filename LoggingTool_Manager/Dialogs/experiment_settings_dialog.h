#ifndef EXPERIMENT_SETTINGS_DIALOG_H
#define EXPERIMENT_SETTINGS_DIALOG_H


#include <QSettings>
#include <QDate>

#include "ui_experiment_settings_dialog.h"


class ExperimentSettingsDialog : public QDialog, Ui::ExperimentSettingsDialog
{
	Q_OBJECT

public:
	explicit ExperimentSettingsDialog(QSettings *settings, bool dont_show_again, QObject *parent = 0);	

	QString getWellUID() { return well_UID; }
	QString getWellName() { return well_name; }
	QString getFieldName() { return field_name; }
	QString getLocation() { return location; }
	QString getProvince() { return province; }
	QString getCountry() { return country; }
	QString getServiceCompany() { return service_company; }
	QString getCompany() { return company; }	
	QString getOperator() { return oper; }
	QString getTool() { return tool; }
	QDate getDate() { return date; }

	bool getDontShowAgain() { return dont_show_again; }

private:
	void setConnections();
	
	QString well_UID;
	QString well_name;
	QString field_name;
	QString location;
	QString province;
	QString country;
	QString service_company;
	QString company;	
	QString oper;
	QString tool;
	QDate date;

	bool dont_show_again;

	QSettings *app_settings;

private slots:	
	void setWellUID(QString text) { well_UID = text; }
	void setWellName(QString text) { well_name = text; }
	void setFieldName(QString text) { field_name = text; }
	void setLocation(QString text) { location = text; }
	void setProvince(QString text) { province = text; }
	void setCountry(QString text) { country = text; }
	void setServiceCompany(QString text) { service_company = text; }
	void setCompany(QString text) { company = text; }
	void setOperator(QString text) { oper = text; }
	void changeDate(const QDate &_date); // { date = _date; }
	void changeDontShow(bool flag) { dont_show_again = flag; }
	void setTool(QString text) { };

};

#endif // EXPERIMENT_SETTINGS_DIALOG_H