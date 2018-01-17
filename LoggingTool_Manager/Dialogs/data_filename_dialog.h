#ifndef DATA_FILENAME_DIALOG_H
#define DATA_FILENAME_DIALOG_H

#include "ui_data_filename_dialog.h"


class DataFileNameDialog : public QDialog, public Ui::DataFileNameDialog
{
	Q_OBJECT

public:
	DataFileNameDialog(QString _path, QString _filename, QWidget *parent = 0);
	
	QString getDataFileName() { return ledName->text(); }
};


#endif // DATA_FILENAME_DIALOG_H