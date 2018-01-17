#ifndef SIGNAL_PROCESSING_WIZARD_H
#define SIGNAL_PROCESSING_WIZARD_H


#include <QSettings>

#include "../Common/settings_tree.h"
#include "../io_general.h"

#include "ui_processing_wizard.h"


// виджет конструирования циклограммы обработки данных АЦП программой сигнального процессора
class SignalProcessingWizard : public QWidget, public Ui::SignalProcessingWizard
{
	Q_OBJECT

public:
	explicit SignalProcessingWizard(QWidget *parent = 0);
	~SignalProcessingWizard();

	void getProcPrg(QVector<uint8_t> &proc) { proc.clear(); proc = cmd_seq; } 

private:
	void setConnections();
	void treeWidgetActivated(QTreeWidgetItem *item, int col);
	
	Ui::SignalProcessingWizard *ui;
	QSettings *signal_procs;

	QList<CTreeWidgetItem*> c_items;
	CTreeWidgetItem *c_item_selected;

	QVector<uint8_t> cmd_seq;

private slots:
	void showProcParameters(QString text);
	void applyProc();

};

#endif // SIGNAL_PROCESSING_WIZARD