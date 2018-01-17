#ifndef INFO_DIALOG_H
#define INFO_DIALOG_H

#include <QtGui>

#include "ui_info_dialog.h"


class InfoDialog : public QDialog, public Ui::InfoDialog
{
	Q_OBJECT

public:
	explicit InfoDialog(QString &info_text, QWidget *parent = 0)
	{
		setupUi(this);
		textEdit->insertHtml(info_text);
	}
};

#endif // INFO_DIALOG_H