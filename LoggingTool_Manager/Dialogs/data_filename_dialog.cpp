#include "data_filename_dialog.h"

DataFileNameDialog::DataFileNameDialog(QString _path, QString _filename, QWidget *parent /* = 0 */)
{
	setupUi(this);

	ledName->setText(_filename);
	ledPath->setText(_path);
}