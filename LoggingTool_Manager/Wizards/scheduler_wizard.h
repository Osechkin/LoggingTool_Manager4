#ifndef SCHEDULER_WIZARD_H
#define SCHEDULER_WIZARD_H


#include <QSettings>
//#include <QList>
//#include <QDateTime>

//#include "sequence_wizard.h"
//#include "../Common/settings_tree.h"
//#include "../io_general.h"
//#include "sequence_wizard.h"

#include "../Common/settings_tree.h"
#include "../Common/experiment_scheduler.h"
#include "../Wizards/depth_template_wizard.h"
#include "../Wizards/sequence_wizard.h"
#include "../Wizards/nmrtool_connect_wizard.h"

#include "ui_scheduler_wizard.h"


//class MainWindow;

// виджет редактирования циклограммы измерений программой сигнального процессора
class SchedulerWizard : public QWidget, public Ui::SchedulerWizard
{
	Q_OBJECT

public:
	SchedulerWizard(SequenceWizard *seq_wiz, DepthTemplateWizard *depth_wiz, NMRToolLinker *nmrtool_wiz, Clocker *clocker, QWidget *parent = 0);
	~SchedulerWizard();

	Scheduler::Engine* getSchedulerEngine() { return &scheduler_engine; } 
	JSeqObject *getExecutingJSeqObject() { return executingJSeq; }
		
	bool generateDistanceScanPrg(QStringList &e);
	bool generateExecPrg(QStringList &e);
	bool isEmpty() { return scheduler_engine.getObjectList().isEmpty(); }
	void clear() { removeAllItems(); }
	bool scheduling(QStringList &e);
	void start();
	void stop();
	
public slots:
	void setJSeqList(QStringList _jseq_list) { jseq_list = _jseq_list; }
	//void setJSeqFile(const QString &_file_name) { jseq_file = _file_name; }
	void setDataFile(const QString &_file_name) { data_file = _file_name; }
	QString getDataFile();

	void setSeqStatus(unsigned char _seq_finished);		// информирует о завершении последовательности
	void setSeqStarted(bool flag);						// свидетельствует об успехе / неуспехе старта последовательности

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	
private:	
	QString generateDataFileName();
	//void loadSettings();

	void insertItem(int row, QString cmd);
	void removeItem(int row);

	void removeSchedulerObj(Scheduler::SchedulerObject *obj);
	
	void showItemParameters(Scheduler::SchedulerObject *obj);

private slots:
	void addItem();	
	void addItemNOP();
	void removeItem();
	void removeAllItems();
	void update();

	void process();
	void execute(Scheduler::SchedulerObject* obj);

	void currentItemSelected(QModelIndex index1, QModelIndex index2);

private:
	Ui::SchedulerWizard *ui;
	
	DepthTemplateWizard *depth_wizard;
	SequenceWizard *sequence_wizard;
	NMRToolLinker *nmrtool_linker;
	Clocker *clocker;

	QList<QTreeWidgetItem*> tree_items;	
		
	Scheduler::Engine scheduler_engine;
	JSeqObject *executingJSeq;

	QStringList jseq_list;
	//QString jseq_file;	
	QString data_file;

	/*QString datafile_path;
	QString datafile_prefix;
	QString datafile_postfix;
	QString datafile_extension;*/

	bool is_started;
	Scheduler::CommandController *current_cmd;
	Scheduler::SchedulerObjList obj_cmd_list;
	unsigned int crc16_last_jseq;

	bool calibration_state;

	bool seq_already_finished;		// флаг, свидетельствующий о том, что сигнал завершения последовательности уже принят и следующие такие сигналы игнорировать до успешного старта следующей последовательности 

signals:
	void finished();
	void started();
	void calibration_started();
	void calibration_finished();
};

#endif // SCHEDULER_WIZARD_H