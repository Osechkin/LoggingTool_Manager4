#ifndef EXPORT_SETTINGS_DIALOG_H
#define EXPORT_SETTINGS_DIALOG_H


#include <QSettings>

//#include "tools_general.h"

#include "ui_export_settings_dialog.h"


class ExportSettingsDialog : public QDialog, Ui::ExportSettingsDialog
{
	Q_OBJECT

public:
	explicit ExportSettingsDialog(QSettings *settings, bool to_save, QObject *parent = 0);	
	
	QString getFileName() { return file_name; }
	QString getPath() { return path; }
	QString getPrefix() { return file_prefix; }		
	QString getPostfix() { return file_postfix; }
	QString getExtension() { return extension; }
	bool isDataExported() { return save_data; }
	bool isAllDataExported() { return save_all_data; }
		
private:
	void setConnections();
	void setFileName();
	
	QString file_name;
	QString path;
	QString file_prefix;
	QString file_postfix;
	QString extension;
	bool save_data;
	bool save_all_data;

	QSettings *app_settings;

private slots:
	void setPath(QString str);
	void browseFolder();
	void setSaveLoggingData(bool flag);	
	void setFilePrefix(QString str);
	void setFilePostfix(QString str);
	void setExtension(QString str);
	void setExportDataPolicy(QString str);
};

#endif // EXPORT_SETTINGS_DIALOG_H