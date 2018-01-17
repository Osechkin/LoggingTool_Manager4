#include "mark_dialog.h"

#include "../io_general.h"


MarkDialog::MarkDialog(int ds_count, QWidget *parent)
{
	setupUi(this);
	QRegExp rx("[0-9,-]{1,}");
	lineEdit->setValidator(new QRegExpValidator(rx, this));

	dataset_count = ds_count;
}

QList<int> MarkDialog::markedDataSets()
{
	QString str = lineEdit->text();

	QList<int> marked_list;
	QStringList str_list1 = str.split(",");
	if (str_list1.isEmpty()) str_list1.append(str);

	for (int i = 0; i < str_list1.count(); i++)
	{
		int _from = NAN;
		int _to = NAN;

		QString str1 = str_list1[i];
		if (str1.contains("-"))
		{
			QStringList str_list2 = str1.split("-");
			
			bool ok;
			_from = str_list2.first().toInt(&ok);
			if (!ok) _from = 1;

			_to = str_list2.last().toInt(&ok);
			if (!ok) _to = dataset_count;
		}
		else
		{
			bool ok;
			_from = str1.toInt(&ok);
			if (!ok) _from = NAN;
			_to = _from;
		}

		if (!(_from == NAN && _to == NAN))
		{
			for (int j = _from; j <= _to; j++)
			{
				if (!marked_list.contains(j) && j <= dataset_count) 
				{
					marked_list.append(j);
				}
			}
		}
	}

	return marked_list;
}