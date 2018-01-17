#ifndef EXPORT_TOOIL_DIALOG_H
#define EXPORT_TOOIL_DIALOG_H


#include <QtGui>

#include "../Common/data_containers.h"
#include "../Common/settings_tree.h"

#include "ui_toOil_export_dialog.h"


class ToOilExportDialog : public QDialog, public Ui::ToOilExportDialog
{
	Q_OBJECT

public:
	explicit ToOilExportDialog(DataTypeList &_dt_list, QWidget *parent = 0);
	~ToOilExportDialog();

	void setLblTitle(QString str);
	void setWinTitle(QString str);

	DataTypeList &getDataTypeList() { return dt_list; }
	bool isSaving() { return saving; }
	
private:
	void showParameters();
	void clearCTreeWidget();
	void setConnections();

	bool saving;

	DataTypeList dt_list;
	
	QList<CTreeWidgetItem*> c_items;
	QList<CTreeWidgetItem*> c_title_items;
	CTreeWidgetItem *c_item_selected;

private slots:
	void paramValueChanged(QObject *obj, QVariant &value);
	void saveSettings();

};

#endif // EXPORT_TOOIL_DIALOG_H