#ifndef MARK_DIALOG_H
#define MARK_DIALOG_H

#include <QtGui>

#include "ui_mark_dialog.h"


class MarkDialog : public QDialog, public Ui::MarkDialog
{
	Q_OBJECT

public:
	explicit MarkDialog(int ds_count, QWidget *parent = 0);

	QList<int> markedDataSets();

private:
	int dataset_count;
};

#endif // MARK_DIALOG_H