#ifndef SEQUENCE_INFO_DIALOG_H
#define SEQUENCE_INFO_DIALOG_H

#include <QtGui>

#include "ui_sequence_info_dialog.h"


class SequenceInfoDialog : public QDialog, public Ui::SequenceInfoDialog
{
	Q_OBJECT

public:
	explicit SequenceInfoDialog(QString &info, QWidget *parent = 0)
	{
		this->setupUi(this);
		this->setParent(parent);

		textEdit->insertHtml(info);
	}
};

#endif // SEQUENCE_INFO_DIALOG_H