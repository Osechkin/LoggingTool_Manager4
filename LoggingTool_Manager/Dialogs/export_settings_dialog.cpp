#include <QFileDialog>
#include <QDateTime>

#include "export_settings_dialog.h"


ExportSettingsDialog::ExportSettingsDialog(QSettings *settings, bool to_save, QObject *parent)
{
	setupUi(this);
	setWindowTitle(tr("Export Data Dialog"));

	app_settings = settings;

	save_data = to_save;
	save_all_data = false;
	gbxSaveData->setChecked(save_data);

	if (app_settings->contains("SaveDataSettings/Path")) path = app_settings->value("SaveDataSettings/Path").toString();
	else
	{
		QString cur_dir = QCoreApplication::applicationDirPath();		
		path = cur_dir;
		app_settings->setValue("SaveDataSettings/Path", path);
	}
	ledPath->setReadOnly(false);
	ledPath->setText(path);
	
	if (app_settings->contains("SaveDataSettings/Prefix")) file_prefix = app_settings->value("SaveDataSettings/Prefix").toString();
	else
	{
		file_prefix = "data";
		app_settings->setValue("SaveDataSettings/Prefix", file_prefix);
	}	
	ledFilePrefix->setText(file_prefix);

	file_postfix = "";
	if (app_settings->contains("SaveDataSettings/Postfix")) file_postfix = app_settings->value("SaveDataSettings/Postfix").toString();
	else
	{
		file_postfix = "";
		app_settings->setValue("SaveDataSettings/Postfix", file_postfix);
	}
	ledFilePostfix->setText(file_postfix);

	QStringList items;
	items << tr("dat") << tr("txt");	
	if (app_settings->contains("SaveDataSettings/Extension")) extension = app_settings->value("SaveDataSettings/Extension").toString();
	else
	{
		extension = "dat";
		app_settings->setValue("SaveDataSettings/Extension", extension);
	}
	if (!items.contains(extension)) items << extension;	
	cboxExtension->addItems(items);
	cboxExtension->setCurrentText(extension);

	cboxExportPolicy->addItems(QStringList() << tr("Export only correct data") << tr("Export all data"));
	cboxExportPolicy->setCurrentIndex(0);

	setFileName();
	setConnections();
}

void ExportSettingsDialog::setConnections()
{
	connect(gbxSaveData, SIGNAL(toggled(bool)), this, SLOT(setSaveLoggingData(bool)));
	connect(ledPath, SIGNAL(textEdited(QString)), this, SLOT(setPath(QString)));
	connect(pbtBrowse, SIGNAL(clicked()), this, SLOT(browseFolder()));	
	connect(ledFilePrefix, SIGNAL(textChanged(QString)), this, SLOT(setFilePrefix(QString)));
	connect(ledFilePostfix, SIGNAL(textChanged(QString)), this, SLOT(setFilePostfix(QString)));
	connect(cboxExtension, SIGNAL(currentIndexChanged(QString)), this, SLOT(setExtension(QString)));
	connect(cboxExtension, SIGNAL(editTextChanged(QString)), this, SLOT(setExtension(QString)));
	connect(cboxExportPolicy, SIGNAL(currentIndexChanged(QString)), this, SLOT(setExportDataPolicy(QString)));
}

void ExportSettingsDialog::setFileName()
{
	QDateTime dateTime = QDateTime::currentDateTime();
	QString dateTime_str = dateTime.toString("yyyy-MM-dd_hh-mm-ss-zzz");
	file_name = "";
	if (!file_prefix.isEmpty()) file_name += file_prefix + "_";
	file_name += dateTime_str;
	if (!file_postfix.isEmpty()) file_name += "_" + file_postfix;
	file_name += "." + extension;
	lblFileName->setText("<font color=darkBlue>" + file_name + "</font>");
}

void ExportSettingsDialog::browseFolder()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setOption(QFileDialog::ShowDirsOnly);

	QString dirname = QFileDialog::getExistingDirectory(this, tr("Select a Directory"), QDir::currentPath() );
	if( !dirname.isNull() )
	{
		ledPath->setText(dirname);
		path = dirname;
		setFileName();
	}	
}

void ExportSettingsDialog::setExportDataPolicy(QString str)
{
	QComboBox *cbox = qobject_cast<QComboBox*>(sender());
	if (cbox)
	{
		if (str == tr("Export only correct data")) save_all_data = false;
		else if (str == tr("Export all data")) save_all_data = true;
	}
}

void ExportSettingsDialog::setSaveLoggingData(bool flag)
{
	save_data = flag;
}


void ExportSettingsDialog::setPath(QString str)
{
	path = str;
	setFileName();
}

void ExportSettingsDialog::setFilePrefix(QString str)
{
	file_prefix = str;
	setFileName();
}

void ExportSettingsDialog::setFilePostfix(QString str)
{
	file_postfix = str;
	setFileName();
}

void ExportSettingsDialog::setExtension(QString str)
{
	extension = str;
	setFileName();
}
