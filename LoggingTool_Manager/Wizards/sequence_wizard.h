#ifndef SEQUENCE_WIZARD_H
#define SEQUENCE_WIZARD_H

#pragma once

#include <QSettings>
#include <QList>
#include <QDateTime>
#include <QFileInfo>
#include <QScriptEngine>
#include <QScriptEngineDebugger>

//#include "../main_window.h"
#include "../Common/app_settings.h"
#include "../Common/settings_tree.h"
#include "../io_general.h"
#include "../Common/experiment_settings.h"
#include "../LUSI/LUSI.h"

#include "ui_sequence_wizard.h"


class JSeqObject : public QObject
{	
	Q_OBJECT

public:
	JSeqObject(QList<SeqCmdInfo> &_cmd_list, QList<SeqInstrInfo> &_instr_list, QString _jseq_file = "");
	~JSeqObject();

	bool evaluate();		// Output : false - cannot open jseq file, true - successfully open jseq file and start it (no mater a result of execution)
	bool partOf(QList<JSeqObject*> &jseq_list, int &index);

	QScriptEngine *js_engine;	
	LUSI::Engine *lusi_engine;
	LUSI::Sequence *lusi_Seq;
	QString jseq_file;	
	QString data_file;

private:
	QList<SeqCmdInfo> cmd_list;
	QList<SeqInstrInfo> instr_list;
};
typedef QList<JSeqObject*>	JSeqObjectList;



// виджет редактирования циклограммы измерений программой сигнального процессора
class SequenceWizard : public QWidget, public Ui::SequenceWizard
{
	Q_OBJECT

public:
	explicit SequenceWizard(QSettings *settings, QWidget *parent = 0);
	explicit SequenceWizard(QWidget *parent = 0) { }
	~SequenceWizard();

	//LUSI::Sequence *getCurrentSequence() { return &cur_lusi_Seq; }	
	LUSI::Sequence* getCurrentSequence(); 
	bool getDSPPrg(QVector<uint8_t> &_prg, QVector<uint8_t> &_instr);
	bool getDSPPrg(QString &jseq_name, QVector<uint8_t> &_prg, QVector<uint8_t> &_instr);

	bool compileSequence(const QString &text, QByteVector &instr_prg, QByteVector &comm_prg, QStringList &e);
	bool executeJSsequence(const QString &text, QStringList &e);
	bool executeJSsequence();

	QList<SeqCmdInfo> *getSeqCmdInfo() { return &seq_cmd_index; }
	QList<SeqInstrInfo> *getSeqInstrIndex() { return &seq_instr_index; }
	QList<SeqDataTypeInfo> *getSeqDataTypeInfo() { return &seq_datatype_index; }

	QStringList getSeqFileList() { return file_list; }
	QStringList getSeqPathList() { return path_list; }
	DataSave getDataFileSettings() { return save_data; }
	void readDataFileSettings();

	JSeqObject *getCurJSeqObject() { return cur_jseq_object; }
	JSeqObject *getJSeqObject(QString jseq_file);

	//void refreshArgFormula();
	
	static QString simplifyFormula(QString _formula, Sequence *_seq);	
	static double calcArgument(double index, Argument* arg, bool *_ok);
	static int calcActualDataSize(QString &formula, bool *_ok);
	static QString getStrItemNumber(QString text, int index, int base);	

	Ui::SequenceWizard *getUI() { return ui; }

protected:
	Ui::SequenceWizard *ui;
	QSettings *app_settings;

	QList<SeqCmdInfo> seq_cmd_index;			// "словарь" команд интервального программатора
	QList<SeqInstrInfo> seq_instr_index;		// "словарь" инструкций для обработки данных сигнальным процессором интервального программатора
	QList<SeqDataTypeInfo> seq_datatype_index;	// "словарь" типов данных для обработки сигнальным процессором интервального программатора

	//QSettings *sequence_proc;
	QStringList file_list;
	QStringList path_list;
	//QString jseq_file;
	
	////QScriptEngine *engine;
	QScriptEngineDebugger script_debugger;
	////LUSI::Engine lusi_engine;
	////LUSI::Sequence cur_lusi_Seq;

	JSeqObjectList jseq_objects;
	JSeqObject *cur_jseq_object;

protected:
	void readSequenceCmdIndex();
	void readSequenceInstrIndex();
	void readSequenceDataTypeIndex();

	bool findSequenceScripts(QStringList &files, QStringList &pathes, QString &seq_path);
	QSettings *initSequenceScript(QString file_name);
	//void parseSequenceScript(QSettings *settings, Sequence &seq);	

	//void showSeqParameters();
	//void showSequenceMemo(Sequence &seq);

	void showLUSISeqParameters();
	void showLUSISeqMemo();
	

private:
	void setConnections();

	//void redrawSeqParameters();
	void clearCTreeWidget();
	
	uint8_t findCmdCode(const QString &str, bool &flag);
	uint8_t findInstrCode(const QString &str, bool &flag);
	QString findCmdMnemonics(const uint8_t value, bool &flag);		
	QString findInstrMnemonics(const uint8_t value, bool &flag);
	QList<uint8_t> findCmdValue(const QString &str, const Sequence &seq);
	QList<uint8_t> findInstrValue(const QString &str, const Sequence &seq, bool &flag);
	bool SequenceWizard::findSequenceScripts(QStringList &files, QStringList &pathes);
		
	QList<CTreeWidgetItem*> c_items;
	QList<CTreeWidgetItem*> c_title_items;
	CTreeWidgetItem *c_item_selected;	
	
	DataSave save_data;

private slots:		
	void descriptionLinkActivated(const QString &link);
	void treeWidgetActivated(QTreeWidgetItem* item, int index);
	bool changeCurrentSequence(const QString &text);		
	void paramValueChanged(QObject *obj, QVariant &value);
	void paramEditingFinished(QObject *obj);
	void addSequence();
	void refreshSequence();
	void refreshSequenceList();
	void viewCode();
	//void setExportSettings();
	void showSequenceInfo();
	//void triggerJSerror();	
	
signals:
	void sequence_changed();
	void new_sequence_file(const QFileInfo&);
	void save_data_changed(DataSave&);
};

#endif // SEQUENCE_WIZARD