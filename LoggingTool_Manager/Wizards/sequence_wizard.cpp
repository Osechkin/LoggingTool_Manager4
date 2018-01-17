#include <QDir>
#include <QTextCodec>
#include <QFileDialog>
#include <QMessageBox>
#include <QScriptEngine>

#include "sequence_wizard.h"
#include "../Dialogs/info_dialog.h"
#include "../Dialogs/view_code_dialog.h"
//#include "../Dialogs/export_settings_dialog.h"
#include "../Dialogs/sequence_info_dialog.h"


JSeqObject::JSeqObject(QList<SeqCmdInfo> &_cmd_list, QList<SeqInstrInfo> &_instr_list, QString _jseq_file)
{
	cmd_list = _cmd_list;
	instr_list = _instr_list;

	jseq_file = _jseq_file;

	js_engine = new QScriptEngine;
	lusi_engine = new LUSI::Engine;
	lusi_engine->init(js_engine, cmd_list, instr_list);	
	
	lusi_Seq = new LUSI::Sequence;	
}

JSeqObject::~JSeqObject()
{
	delete js_engine;
	delete lusi_engine;
	delete lusi_Seq;
}

bool JSeqObject::evaluate()
{
	QFile file(jseq_file);
	QString str = "";
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream text_stream(&file);
		str = text_stream.readAll();
		file.close();			
	}
	else 
	{
		lusi_Seq->seq_errors.append(tr("Cannot open the sequence file %1").arg(jseq_file));
		return false;
	}

	QFileInfo file_info(jseq_file);
	QString file_name = file_info.fileName();
	QString file_path = file_info.absoluteDir().path();
	lusi_Seq->setFilePathName(file_path, file_name);

	QStringList lusi_elist;
	lusi_engine->setLUSIscript(str);	
	if (lusi_engine->evaluate(lusi_elist))
	{
		LUSI::ObjectList *obj_list_global = lusi_engine->getObjList();
		if (obj_list_global->isEmpty())
		{
			lusi_Seq->seq_errors.append(tr("Sequence program for Logging Tool was not found in %1 file!").arg(jseq_file));			
		}
		else
		{
			QString js_script = lusi_engine->getJSscript();		
			QScriptValue qscrpt_value = js_engine->evaluate(js_script);
			if (qscrpt_value.isError())
			{
				lusi_Seq->js_error = tr("Runtime error executing JavaScript code of the Pulse Sequence!");
				lusi_Seq->comprg_list.clear();
				lusi_Seq->proc_programs.clear();	
				lusi_Seq->js_script.clear();
			}
			else
			{
				lusi_Seq->clear();
				lusi_Seq->js_script = js_script;
				lusi_Seq->setObjects(obj_list_global);				
			}	
		}				
	}
	else
	{
		// There are errors in LUSI code !
		LUSI::ObjectList *obj_list_global = lusi_engine->getObjList();
		if (obj_list_global->isEmpty())
		{
			lusi_Seq->seq_errors.append(tr("Sequence program for Logging Tool was not found in %1 file!").arg(jseq_file));	
		}	
		lusi_Seq->clear();
		lusi_Seq->setObjects(obj_list_global);		

		if (!lusi_engine->getErrorList().isEmpty()) return false;		
	}
	return true;
}

bool JSeqObject::partOf(QList<JSeqObject*> &jseq_list, int &index)
{	
	index = -1;
	for (int i = 0; i < jseq_list.count(); i++)
	{
		if (jseq_list.at(i)->jseq_file == this->jseq_file) 
		{
			index = i;
			return true;
		}
	}
	return false;
}


SequenceWizard::SequenceWizard(QSettings *settings, QWidget *parent) : QWidget(parent), ui(new Ui::SequenceWizard)
{
	ui->setupUi(this);
	ui->pbtRefresh->setIcon(QIcon(":/images/Refresh.png"));
	ui->pbtViewCode->setIcon(QIcon(":/images/view_text.png"));
	//ui->pbtExportSettings->setIcon(QIcon(":/images/export_data.png"));
	ui->tbtAddSequence->setIcon(QIcon(":/images/add_script.png"));
	ui->tbtRefreshSeqList->setIcon(QIcon(":/images/Refresh.png"));
	ui->tbtInfo->setIcon(QIcon(":/images/Info.png"));

	ui->tbtAddSequence->setToolTip(tr("Refresh Sequence file to the List"));
	ui->tbtRefreshSeqList->setToolTip(tr("Refresh Sequence file List"));

	ui->tbtAddSequence->setVisible(false);
	ui->tbtRefreshSeqList->setVisible(true);

	QStringList headlist;
	ui->treeWidget->setColumnCount(3);
	headlist << tr("Parameter") << tr("Value") << tr("Units");
	ui->treeWidget->setHeaderLabels(headlist);
	ui->treeWidget->setColumnWidth(0,300);
	ui->treeWidget->setColumnWidth(1,75);	

	ui->treeWidget->header()->setFont(QFont("Arial", 10, 0, false));		

	readSequenceCmdIndex();
	readSequenceInstrIndex();
	
	cur_jseq_object = NULL;
	QString seq_path = QDir::currentPath() + "/Sequences";
	bool res = findSequenceScripts(file_list, path_list, seq_path);
	if (res)
	{
		for (int i = 0; i < file_list.count(); i++)
		{
			QString file_name = path_list[i] + "/" + file_list[i];
			QFile file(file_name);
			QString str = "";
			if (file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				JSeqObject *jseq_object = new JSeqObject(seq_cmd_index, seq_instr_index, file_name);
				jseq_objects.append(jseq_object);
			}
		}

		if (!jseq_objects.isEmpty()) 
		{
			cur_jseq_object = jseq_objects.first();
			script_debugger.attachTo(cur_jseq_object->js_engine);
			cur_jseq_object->evaluate();

			showLUSISeqParameters();
			showLUSISeqMemo();		

			ui->cboxSequences->addItems(file_list);
			ui->ledSeqName->setText(cur_jseq_object->lusi_Seq->name);
		}
	}	

	app_settings = settings;
	readDataFileSettings();

	setConnections();
}


SequenceWizard::~SequenceWizard()
{
	clearCTreeWidget();

	qDeleteAll(jseq_objects.begin(), jseq_objects.end());
	jseq_objects.clear();

	delete ui;	
}

void SequenceWizard::setConnections()
{
	connect(ui->lblDescription, SIGNAL(linkActivated(const QString &)), this, SLOT(descriptionLinkActivated(const QString &)));
	connect(ui->cboxSequences, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeCurrentSequence(const QString &)));

	//connect(ui->tbtAddSequence, SIGNAL(clicked()), this, SLOT(addSequence()));
	connect(ui->pbtViewCode, SIGNAL(clicked()), this, SLOT(viewCode()));
	connect(ui->pbtRefresh, SIGNAL(clicked()), this, SLOT(refreshSequence()));
	connect(ui->tbtInfo, SIGNAL(clicked()), this, SLOT(showSequenceInfo()));
	connect(ui->tbtRefreshSeqList, SIGNAL(clicked()), this, SLOT(refreshSequenceList()));
	
	connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(treeWidgetActivated(QTreeWidgetItem*,int)));
}


bool SequenceWizard::findSequenceScripts(QStringList &files, QStringList &pathes)
{
	files.clear();
	pathes.clear();

	QString path = QDir::currentPath() + "/Sequences";
	QDir dir(path);	
	QStringList file_list = dir.entryList(QDir::Files | QDir::NoSymLinks);

	QStringList res;
	for (int i = 0; i < file_list.count(); i++)
	{
		if (file_list[i].split(".").last().toLower() == QString("seq")) 
		{
			files.append(file_list[i]);			
			pathes.append(path);
		}
	}

	return !files.isEmpty();
}

bool SequenceWizard::findSequenceScripts(QStringList &files, QStringList &pathes, QString &seq_path)
{
	files.clear();
	pathes.clear();

	QDir dir(seq_path);
	QStringList file_list = dir.entryList(QDir::Files | QDir::NoSymLinks);

	QStringList res;
	for (int i = 0; i < file_list.count(); i++)
	{
		if (file_list[i].split(".").last().toLower() == QString("jseq")) 
		{
			files.append(file_list[i]);
			pathes.append(seq_path);
		}
	}

	return !files.isEmpty();
}

QSettings* SequenceWizard::initSequenceScript(QString file_name)
{
	QSettings *settings = new QSettings(file_name, QSettings::IniFormat);	

#ifdef Q_OS_WIN
	QString code_page = "Windows-1251";
#elif Q_OS_LINUX
	QString code_page = "UTF-8";
#endif
	QTextCodec *codec = QTextCodec::codecForName(code_page.toUtf8());
	settings->setIniCodec(codec);

	return settings;
}


void SequenceWizard::showLUSISeqParameters()
{
	clearCTreeWidget();
	ui->treeWidget->clear();
	if (cur_jseq_object == NULL) return;
	
	LUSI::Sequence *cur_lusi_Seq = cur_jseq_object->lusi_Seq;
	for (int i = 0; i < cur_lusi_Seq->section_list.count(); i++)
	{
		LUSI::Section *sec = cur_lusi_Seq->section_list[i];
		if (sec)
		{
			QString title = sec->getTitle();

			QList<CSettings> title_settings_list;
			CSettings title_settings_probe("label", title);
			title_settings_probe.title_flag = true;
			title_settings_probe.text_color = QColor(Qt::darkRed);
			title_settings_probe.background_color = QColor(Qt::blue).lighter(170);
			title_settings_list.append(title_settings_probe);

			CTreeWidgetItem *c_title = new CTreeWidgetItem(ui->treeWidget, 0, title_settings_list);
			c_title_items.append(c_title);
			c_title->show();

			for (int j = 0; j < sec->getParameters().count(); j++)
			{
				LUSI::Parameter *param = qobject_cast<LUSI::Parameter*>(sec->getParameters()[j]);
				if (param)
				{
					QList<CSettings> item_settings_list;

					CSettings item_settings1("label", param->getTitle());
					item_settings1.hint = QString("%1 : %2").arg(param->getObjName()).arg(param->getComment());
					item_settings1.text_color = QColor(Qt::darkBlue);		
					item_settings1.background_color = QColor(Qt::blue).lighter(190);

					QString ui_type = param->getUIType();
					CSettings item_settings3(ui_type.toLower(), param->getValue());
					if (ui_type.toLower() == "combobox") 
					{
						item_settings3.value = param->getStrValue();	
						item_settings3.data_type = String_Data;
					}
					else if (ui_type.toLower() == "checkbox") 
					{
						item_settings3.check_state = param->getValue();	
						item_settings3.value = "no_text";
						item_settings3.data_type = Bool_Data;
					}
					else 
					{
						item_settings3.data_type = Double_Data;
					}
					QString str_minmax = QString("[ %1 ... %2 ]").arg(param->getMin()).arg(param->getMax());	
					item_settings3.hint = str_minmax;
					double d_min = (double)param->getMin();
					double d_max = (double)param->getMax();		
					item_settings3.min_max = QPair<double,double>(d_min,d_max);		
					item_settings3.read_only = param->getReadOnly();
					item_settings3.text_color = QColor(Qt::darkMagenta);
					item_settings3.background_color = QColor(Qt::blue).lighter(190);

					CSettings item_settings4("label", " " + param->units);		
					item_settings4.text_color = QColor(Qt::darkBlue);		
					item_settings4.background_color = QColor(Qt::blue).lighter(190);

					item_settings_list << item_settings1 << item_settings3 << item_settings4;
					CTreeWidgetItem *c_item = new CTreeWidgetItem(ui->treeWidget, c_title->getQSubTreeWidgetItem(), item_settings_list);					
					connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
					c_item->linkObject(param);
					c_items.append(c_item);
					c_item->show();
				}
			}
		}
	}

	for (int i = 0; i < cur_lusi_Seq->output_list.count(); i++)
	{
		LUSI::Output *output_sec = cur_lusi_Seq->output_list[i];
		if (output_sec)
		{
			QString title = output_sec->getTitle();

			QList<CSettings> title_settings_list;
			CSettings title_settings_probe("label", title);
			title_settings_probe.title_flag = true;
			title_settings_probe.text_color = QColor(Qt::darkRed);
			title_settings_probe.background_color = QColor(Qt::blue).lighter(170);
			title_settings_list.append(title_settings_probe);

			CTreeWidgetItem *c_title = new CTreeWidgetItem(ui->treeWidget, 0, title_settings_list);
			c_title_items.append(c_title);
			c_title->show();

			for (int j = 0; j < output_sec->getParameters().count(); j++)
			{
				LUSI::Parameter *param = qobject_cast<LUSI::Parameter*>(output_sec->getParameters()[j]);
				if (param)
				{
					QList<CSettings> item_settings_list;

					CSettings item_settings1("label", param->getTitle());
					item_settings1.hint = param->getComment();
					item_settings1.text_color = QColor(Qt::darkBlue);		
					item_settings1.background_color = QColor(Qt::blue).lighter(190);

					QString ui_type = param->getUIType();
					CSettings item_settings2(ui_type.toLower(), param->getValue());
					if (ui_type.toLower() == "checkbox") item_settings2.value = QVariant("no_text");
					QString str_minmax = QString("[ %1 ... %2 ]").arg(param->getMin()).arg(param->getMax());	
					item_settings2.data_type = Bool_Data;
					item_settings2.hint = str_minmax;
					double d_min = (double)param->getMin();
					double d_max = (double)param->getMax();		
					item_settings2.min_max = QPair<double,double>(d_min,d_max);		
					item_settings2.read_only = param->getReadOnly();
					item_settings2.text_color = QColor(Qt::darkMagenta);
					item_settings2.background_color = QColor(Qt::blue).lighter(190);

					item_settings_list << item_settings1 << item_settings2;
					CTreeWidgetItem *c_item = new CTreeWidgetItem(ui->treeWidget, c_title->getQSubTreeWidgetItem(), item_settings_list);
					connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
					c_item->linkObject(param);
					c_items.append(c_item);
					c_item->show();
				}
			}
		}
	}
}


bool SequenceWizard::executeJSsequence()
{	
	LUSI::ObjectList *obj_list_global = cur_jseq_object->lusi_engine->getObjList();
	if (obj_list_global->isEmpty()) return false;

	cur_jseq_object->lusi_Seq->reset();
	cur_jseq_object->lusi_engine->reset();
	QString js_script = cur_jseq_object->lusi_engine->getJSscript();	
	if (js_script.isEmpty()) 
	{
		cur_jseq_object->lusi_Seq->js_error = tr("Available JavaScript code was not found in the Pulse Sequence!");
		cur_jseq_object->lusi_Seq->comprg_list.clear();
		cur_jseq_object->lusi_Seq->proc_programs.clear();	
		return false;
	}

	QScriptValue qscrpt_value = cur_jseq_object->js_engine->evaluate(js_script);	
	if (qscrpt_value.isError())
	{
		cur_jseq_object->lusi_Seq->js_error = tr("Runtime error executing JavaScript code of the Pulse Sequence!");
		cur_jseq_object->lusi_Seq->comprg_list.clear();
		cur_jseq_object->lusi_Seq->proc_programs.clear();	
		return false;
	}
	else
	{
		cur_jseq_object->lusi_Seq->clear();
		cur_jseq_object->lusi_Seq->setObjects(obj_list_global);				
	}

	return true;
}

bool SequenceWizard::executeJSsequence(const QString &text, QStringList &e)
{
	JSeqObject *jseq_obj = NULL;
	for (int i = 0; i < jseq_objects.count(); i++)
	{
		jseq_obj = jseq_objects[i];
		QFileInfo fi(jseq_obj->jseq_file);
		if (fi.fileName() == text) break;		
		jseq_obj = NULL;
	}
	if (!jseq_obj)
	{
		e.append(tr("Error: Sequence %1 was not found!").arg(text));
		return false;
	}
	
	if (!jseq_obj->lusi_engine->getErrorList().isEmpty())
	{
		jseq_obj->lusi_Seq->js_error = tr("Errors were found in Sequence %1!").arg(text);
		jseq_obj->lusi_Seq->comprg_list.clear();
		jseq_obj->lusi_Seq->proc_programs.clear();	
		e.append(jseq_obj->lusi_Seq->js_error);
		return false;
	}

	QString js_script = jseq_obj->lusi_engine->getJSscript();
	if (jseq_obj->lusi_Seq->com_programs.isEmpty() || js_script.isEmpty())
	{
		jseq_obj->evaluate();
		if (!jseq_obj->lusi_Seq->js_error.isEmpty())
		{
			jseq_obj->lusi_Seq->js_error = tr("Errors were found in Sequence %1!").arg(text);
			jseq_obj->lusi_Seq->comprg_list.clear();
			jseq_obj->lusi_Seq->proc_programs.clear();	
			e.append(jseq_obj->lusi_Seq->js_error);
			return false;
		}
		else return true;
	}
		
	jseq_obj->lusi_Seq->reset();
	jseq_obj->lusi_engine->reset();
	QScriptValue qscrpt_value = jseq_obj->js_engine->evaluate(js_script);	
	if (qscrpt_value.isError())
	{
		jseq_obj->lusi_Seq->js_error = tr("Runtime error executing JavaScript code of the Pulse Sequence!");
		jseq_obj->lusi_Seq->comprg_list.clear();
		jseq_obj->lusi_Seq->proc_programs.clear();	
		e.append(jseq_obj->lusi_Seq->js_error);
		return false;
	}
	else
	{
		jseq_obj->lusi_Seq->clear();
		LUSI::ObjectList *obj_list_global = jseq_obj->lusi_engine->getObjList();
		jseq_obj->lusi_Seq->setObjects(obj_list_global);				
	}

	return true;
}

void SequenceWizard::showLUSISeqMemo()
{	
	if (cur_jseq_object == NULL) return;
	LUSI::Sequence *cur_lusi_Seq = cur_jseq_object->lusi_Seq;
	LUSI::Engine *cur_lusi_Engine = cur_jseq_object->lusi_engine;

	bool parse_errs = false;
	if (!cur_lusi_Seq->seq_errors.isEmpty()) parse_errs = true;
	
	bool program_errs = false;
	if (!cur_lusi_Seq->comprg_errors.isEmpty()) program_errs = true;
	
	bool fpga_errs = false;
	if (!cur_lusi_Seq->procdsp_errors.isEmpty()) fpga_errs = true;

	bool cond_errs = false;
	if (!cur_lusi_Seq->cond_errors.isEmpty()) cond_errs = true;

	bool lusi_errs = false;
	if (!cur_lusi_Engine->getErrorList().isEmpty()) lusi_errs = true;

	bool js_error = !cur_lusi_Seq->js_error.isEmpty();
	
	QString memo = "";
	memo += QString("<font color = darkblue>%1</font> ").arg(tr("Parsing & Run-time Errors:"));
	if (parse_errs || js_error || lusi_errs) memo += QString("<a href=#parse_error><font color=red><b><u>%1</u></b></font></a><br>").arg(tr("Found!"));
	else memo += QString("<font color=darkgreen>%1</font><br>").arg(tr("Not Found."));
	
	memo += QString("<font color = darkblue>%1</font> ").arg(tr("Errors in the FPGA Program:"));
	if (fpga_errs) memo += QString("<a href=#fpga_error><font color=red><b><u>%1</u></b></font></a><br>").arg(tr("Found!"));
	else memo += QString("<font color=darkgreen>%1</font><br>").arg(tr("Not Found."));

	memo += QString("<font color = darkblue>%1</font> ").arg(tr("Errors in the DSP Processing Program:"));
	if (program_errs) memo += QString("<a href=#dsp_error><font color=red><b><u>%1</u></b></font></a><br>").arg(tr("Found!"));
	else memo += QString("<font color=darkgreen>%1</font><br>").arg(tr("Not Found."));
	
	if (cur_lusi_Seq->cond_list.isEmpty()) memo += QString("<font color = red>%1</font> ").arg(tr("Conditions to check the Parameters weren't found in the Sequence!"));
	else 
	{
		memo += QString("<font color = darkblue>%1</font> ").arg(tr("Check the Sequence Parameters:"));
		if (cond_errs) memo += QString("<a href=#params_error><font color=red><b><u>%1</u></b></font></a>").arg(tr("Incorrect Parameters were found!"));
		else memo += QString("<font color=darkgreen>%1</font>").arg(tr("OK!"));	
	}

	ui->lblDescription->setOpenExternalLinks(false);
	ui->lblDescription->setText(memo);
}


void SequenceWizard::clearCTreeWidget()
{
	if (!c_items.isEmpty()) qDeleteAll(c_items);
	c_items.clear();

	if (!c_title_items.isEmpty()) qDeleteAll(c_title_items);
	c_title_items.clear();
}


void SequenceWizard::descriptionLinkActivated(const QString &link)
{
	if (cur_jseq_object == NULL) return;
	LUSI::Sequence *cur_lusi_Seq = cur_jseq_object->lusi_Seq;
	LUSI::Engine *cur_lusi_Engine = cur_jseq_object->lusi_engine;

	QString err_msg = "<b>" + tr("Sequence file parsing and execution errors:") + "</b><br><br><font color=red>";

	if( link == "#parse_error" || link == "#fpga_error" || link == "#dsp_error" || link == "#params_error" )
	{		
		int cnt = 1;
		
		QStringList lusi_errors = cur_lusi_Engine->getErrorList();
		for (int i = 0; i < lusi_errors.count(); i++)
		{
			err_msg += tr("%1. %2<br>").arg(cnt++).arg(lusi_errors[i]);				
		}

		QStringList seq_errors = cur_lusi_Seq->seq_errors;
		for (int i = 0; i < seq_errors.count(); i++)
		{
			err_msg += tr("%1. %2<br>").arg(cnt++).arg(seq_errors[i]);				
		}
		
		QStringList comprg_errors = cur_lusi_Seq->comprg_errors;
		for (int i = 0; i < comprg_errors.count(); i++)
		{
			err_msg += tr("%1. %2<br>").arg(cnt++).arg(comprg_errors[i]);		
		}

		QStringList procdsp_errors = cur_lusi_Seq->procdsp_errors;
		for (int i = 0; i < procdsp_errors.count(); i++)
		{
			err_msg += tr("%1. %2<br>").arg(cnt++).arg(procdsp_errors[i]);		
		}

		QStringList cond_errors = cur_lusi_Seq->cond_errors;		
		for (int i = 0; i < cond_errors.count(); i++)
		{
			err_msg += tr("%1. %2<br>").arg(cnt++).arg(cond_errors[i]);		
		}

		QString runtime_error = cur_lusi_Seq->js_error;		
		err_msg += tr("%1. %2<br>").arg(cnt++).arg(runtime_error);	

		err_msg += "<br><br>";		
		err_msg += "</font>";

		InfoDialog info_dlg(err_msg, this);
		if (info_dlg.exec()) return;
	}	
}


bool SequenceWizard::changeCurrentSequence(const QString &text)
{
	if (jseq_objects.isEmpty()) return false;
	
	int index = -1;
	for (int i = 0; i < jseq_objects.count(); i++)
	{
		QFileInfo file_info(jseq_objects[i]->jseq_file);
		QString file_name = file_info.fileName();
		if (file_name == text) 
		{ 
			index = i; 
			break; 
		}
	}
	if (index < 0) return false;
	else cur_jseq_object = jseq_objects[index];
	
	script_debugger.detach();
	script_debugger.attachTo(cur_jseq_object->js_engine);

	if (cur_jseq_object->lusi_Seq->com_programs.isEmpty())
	{
		cur_jseq_object->evaluate();
	}	

	showLUSISeqParameters();
	showLUSISeqMemo();

	emit sequence_changed();

	return true;	
}

bool SequenceWizard::compileSequence(const QString &text, QByteVector &instr_prg, QByteVector &comm_prg, QStringList &e)
{
	if (jseq_objects.isEmpty()) 
	{
		e.append("<font color=red>" + tr("Sequences weren't found!") + "</font>");
		return false;
	}

	int index = -1;
	for (int i = 0; i < jseq_objects.count(); i++)
	{
		QFileInfo file_info(jseq_objects[i]->jseq_file);
		QString file_name = file_info.fileName();
		if (file_name == text) 
		{ 
			index = i; 
			break; 
		}
	}
	if (index < 0) 
	{
		e.append("<font color=red>" + tr("Sequence %1 wasn't found!").arg(text) + "</font>");
		return false;
	}

	JSeqObject *jseq_obj = jseq_objects[index];	
	if (jseq_obj->lusi_Seq->com_programs.isEmpty())
	{
		jseq_obj->evaluate();
	}	

	LUSI::Sequence *lusi_Seq = jseq_obj->lusi_Seq;

	bool flag = true;
	if (!lusi_Seq->seq_errors.isEmpty()) { e << lusi_Seq->seq_errors; flag = false; }
	if (!lusi_Seq->comprg_errors.isEmpty()) { e << lusi_Seq->comprg_errors; flag = false; }
	if (!lusi_Seq->procdsp_errors.isEmpty()) { e << lusi_Seq->procdsp_errors; flag = false; }
	if (!lusi_Seq->cond_errors.isEmpty()) { e << lusi_Seq->cond_errors; flag = false; }
	if (!lusi_Seq->js_error.isEmpty()) { e << lusi_Seq->js_error; flag = false; }

	if (!flag) return false;

	comm_prg.clear();
	QByteVector cmd_prg = lusi_Seq->com_programs.first();	// временно ! ѕринимаетс€ только перва€ программа дл€ интервального программатора
	if (cmd_prg.isEmpty()) return false;
	comm_prg << cmd_prg;

	instr_prg.clear();
	if (lusi_Seq->proc_programs.isEmpty()) return false;
	for (int i = 0; i < lusi_Seq->proc_programs.count(); i++)
	{
		QByteVector instr_pack = lusi_Seq->proc_programs[i];
		instr_prg << instr_pack;
		if (i < lusi_Seq->proc_programs.count()-1)
		{
			instr_prg.append(0xFF);
		}
	}

	return true;
}

void SequenceWizard::addSequence()
{
	QString full_fileName = QFileDialog::getOpenFileName(this, tr("Add Sequence File"), QDir::currentPath(), tr("Sequences (*.seq)"));
	if (full_fileName.isEmpty()) return;

	QString fileName = full_fileName.split("/").last();
	QString pathName = full_fileName.split("/" + fileName).first();

	file_list.append(fileName);
	path_list.append(pathName);
	ui->cboxSequences->addItem(fileName);
	ui->cboxSequences->setCurrentText(fileName);
}


void SequenceWizard::refreshSequence()
{
	if (!cur_jseq_object) return;
	
	if (!cur_jseq_object->evaluate())
	{
		QMessageBox msgBox;							// то сообщить пользователю 
		msgBox.setText(tr("Warning !"));
		msgBox.setInformativeText(tr("Cannot open file %1 !").arg(cur_jseq_object->jseq_file));
		msgBox.setStandardButtons(QMessageBox::Ok);		
		int ret = msgBox.exec();
	}
	else
	{
		QString cur_fileName = ui->cboxSequences->currentText();
		//changeCurrentSequence(cur_fileName);		// обновить все данные текущей последовательности 

		showLUSISeqParameters();
		showLUSISeqMemo();

		emit sequence_changed();
	}
}


void SequenceWizard::refreshSequenceList()
{	
	disconnect(ui->cboxSequences, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeCurrentSequence(const QString &)));
	ui->cboxSequences->clear();
	connect(ui->cboxSequences, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeCurrentSequence(const QString &)));
	
	qDeleteAll(jseq_objects.begin(), jseq_objects.end());
	jseq_objects.clear();

	QString seq_path = QDir::currentPath() + "/Sequences";
	bool res = findSequenceScripts(file_list, path_list, seq_path);
	if (res)
	{
		for (int i = 0; i < file_list.count(); i++)
		{
			QString file_name = path_list[i] + "/" + file_list[i];
			QFile file(file_name);
			QString str = "";
			if (file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				JSeqObject *jseq_object = new JSeqObject(seq_cmd_index, seq_instr_index, file_name);
				jseq_objects.append(jseq_object);
			}
		}

		if (!jseq_objects.isEmpty()) 
		{
			cur_jseq_object = jseq_objects.first();
			script_debugger.attachTo(cur_jseq_object->js_engine);
			cur_jseq_object->evaluate();

			showLUSISeqParameters();
			showLUSISeqMemo();		

			ui->cboxSequences->addItems(file_list);
			ui->ledSeqName->setText(cur_jseq_object->lusi_Seq->name);
		}
	}	
}


void SequenceWizard::readSequenceCmdIndex()
{
	//QFile inputFile(":/text_files/sequence_commands.txt");
	QString path = QDir::currentPath() + "/Conf/";
	QFile inputFile(path + "sequence_commands.txt");
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (!line.isEmpty())
			{
				QStringList str_list = line.split('\t');
				if (str_list.size() >= 2)
				{							
					bool ok;
					uint8_t code = (uint8_t)str_list[1].toUInt(&ok);
					if (ok) 
					{
						SeqCmdInfo cmd_info;
						cmd_info.cmd_name = str_list.first();
						cmd_info.cmd_code = code;
						if (str_list.size() > 2) cmd_info.comment = str_list[2];
						seq_cmd_index.append(cmd_info);
					}
				}
			}			
		}
		inputFile.close();
	}
}

void SequenceWizard::readSequenceInstrIndex()
{
	//QFile inputFile(":/text_files/sequence_instructions.txt");
	QString path = QDir::currentPath() + "/Conf/";
	QFile inputFile(path + "sequence_instructions.txt");
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (!line.isEmpty())
			{
				QStringList str_list = line.split('\t');
				if (str_list.size() >= 2)
				{							
					bool ok;
					uint8_t code = (uint8_t)str_list[1].toUInt(&ok);
					if (ok) 
					{
						SeqInstrInfo instr_info;
						instr_info.instr_name = str_list.first();
						instr_info.instr_code = code;
						if (str_list.size() > 2) instr_info.comment = str_list[2];
						seq_instr_index.append(instr_info);
					}
				}
			}			
		}
		inputFile.close();
	}
}

void SequenceWizard::readSequenceDataTypeIndex()
{
	//QFile inputFile(":/text_files/sequence_datatypes.txt");
	QString path = QDir::currentPath() + "/Conf/";
	QFile inputFile(path + "sequence_datatypes.txt");
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (!line.isEmpty())
			{
				QStringList str_list = line.split('\t');
				if (str_list.size() >= 1)
				{							
					bool ok;
					uint8_t code = (uint8_t)str_list.first().toUInt(&ok);
					if (ok) 
					{
						SeqDataTypeInfo datatype_info;						
						datatype_info.datatype_code = code;
						if (str_list.size() > 1) datatype_info.comment = str_list[1];
						seq_datatype_index.append(datatype_info);
					}
				}
			}			
		}
		inputFile.close();
	}
}


void SequenceWizard::paramValueChanged(QObject *obj, QVariant &value)
{
	CTreeWidgetItem *ctwi = qobject_cast<CTreeWidgetItem*>(sender());
	if (ctwi)
	{
		LUSI::Parameter *cur_param = (LUSI::Parameter*)ctwi->getLinkedObject();
		if (cur_param)
		{
			CTextEdit *cted = qobject_cast<CTextEdit*>(obj);
			if (cted)
			{
				bool ok;
				double d_value = value.toDouble(&ok); //cted->text().toDouble(&ok);
				if (ok) 
				{
					cur_param->setValue(d_value);
					cur_param->exec(d_value);		// appValue = value

					executeJSsequence();
					showLUSISeqMemo();				

					emit sequence_changed();
					return;
				}
			}

			CComboBox *ccbox = qobject_cast<CComboBox*>(obj);
			if (ccbox)
			{
				bool ok;
				int index_value = value.toInt();

				cur_param->setValue(index_value);
				cur_param->exec(index_value);		// appValue = value

				executeJSsequence();
				showLUSISeqMemo();				

				emit sequence_changed();
				return;		
			}

			CCheckBox *chbox = qobject_cast<CCheckBox*>(obj);
			if (chbox)
			{						
				int32_t bool_value = (int32_t)value.toBool();
				cur_param->setValue(bool_value);
				cur_param->exec(bool_value);		// appValue = value

				executeJSsequence();
				showLUSISeqMemo();				
				
				emit sequence_changed();
				return;
			}

			CDSpinBox *cdsbox = qobject_cast<CDSpinBox*>(obj);
			if (cdsbox)
			{
				bool ok;
				double d_value = value.toDouble(&ok);
				if (ok) 
				{
					cur_param->setValue(d_value);
					cur_param->exec(d_value);		// appValue = value
					
					executeJSsequence();
					showLUSISeqMemo();				

					emit sequence_changed();					
					return;
				}
			}

			CSpinBox *csbox = qobject_cast<CSpinBox*>(obj);
			if (csbox)
			{
				bool ok;
				double i_value = value.toInt(&ok);
				if (ok) 
				{
					cur_param->setValue(i_value);
					cur_param->exec(i_value);		// appValue = value

					executeJSsequence();
					showLUSISeqMemo();				

					emit sequence_changed();					
					return;
				}
			}
		}		
	}
}


void SequenceWizard::paramEditingFinished(QObject *obj)
{
	CTreeWidgetItem *ctwi = qobject_cast<CTreeWidgetItem*>(sender());
	if (ctwi)
	{
		LUSI::Parameter *cur_param = (LUSI::Parameter*)ctwi->getLinkedObject();
		if (cur_param)
		{
			CTextEdit *cted = qobject_cast<CTextEdit*>(obj);
			if (cted)
			{
				bool ok;
				double d_value = cted->text().toDouble(&ok);
				if (ok) 
				{
					cur_param->setValue(d_value);
					emit sequence_changed();
					return;
				}
			}

			CComboBox *ccbox = qobject_cast<CComboBox*>(obj);
			if (ccbox)
			{
				bool ok;
				QString str_value = ccbox->currentText();
				// ...
				return;			
			}

			CDSpinBox *cdsbox = qobject_cast<CDSpinBox*>(obj);
			if (cdsbox)
			{				
				double d_value = cdsbox->value();					
				cur_param->setValue(d_value);
								
				executeJSsequence();
				showLUSISeqMemo();				
				
				emit sequence_changed();

				return;
			}

			CSpinBox *csbox = qobject_cast<CSpinBox*>(obj);
			if (csbox)
			{				
				int i_value = csbox->value();					
				cur_param->setValue(i_value);

				executeJSsequence();
				showLUSISeqMemo();				

				emit sequence_changed();

				return;
			}
		}		
	}
}

void SequenceWizard::treeWidgetActivated(QTreeWidgetItem* item, int index)
{
	for (int i = 0; i < c_items.count(); i++)
	{
		QTreeWidgetItem *sub_item = c_items[i]->getQSubTreeWidgetItem();
		if (sub_item == item)
		{
			c_item_selected = c_items[i];			
		}
	}
}

uint8_t SequenceWizard::findCmdCode(const QString &str, bool &flag)
{
	uint8_t res = 0;
	flag = false;
	for (int i = 0; i < seq_cmd_index.count(); i++)
	{
		if (str == seq_cmd_index[i].cmd_name) 
		{
			res = seq_cmd_index[i].cmd_code;		
			flag = true;
		}
	}
	return res;
}

QString SequenceWizard::findCmdMnemonics(uint8_t value, bool &flag)
{
	QString res = "";
	flag = false;
	for (int i = 0; i < seq_cmd_index.count(); i++)
	{
		if (value == seq_cmd_index[i].cmd_code)
		{
			res = seq_cmd_index[i].cmd_name;
			flag = true;
		}
	}
	return res;
}

QList<uint8_t> SequenceWizard::findCmdValue(const QString &str, const Sequence &seq)
{
	QList<uint8_t> res;
	for (int i = 0; i < seq.param_list.count(); i++)
	{
		if (str == seq.param_list[i]->name)
		{			
			int32_t new_value = seq.param_list[i]->app_value;
			uint8_t byte4 = (uint8_t)(new_value & 0xFF);
			uint8_t byte3 = (uint8_t)((new_value & 0xFF00) >> 8);
			uint8_t byte2 = (uint8_t)((new_value & 0xFF0000) >> 16);
			res << byte2 << byte3 << byte4;		

			return res;
		}
	}

	return res;
}

QString SequenceWizard::findInstrMnemonics(const uint8_t value, bool &flag)
{
	QString res = "";
	flag = false;
	for (int i = 0; i < seq_instr_index.count(); i++)
	{
		if (value == seq_instr_index[i].instr_code)
		{
			res = seq_instr_index[i].instr_name;
			flag = true;
		}
	}
	return res;
}

uint8_t SequenceWizard::findInstrCode(const QString &str, bool &flag)
{
	uint8_t res = 0;
	flag = false;
	for (int i = 0; i < seq_instr_index.count(); i++)
	{
		if (str == seq_instr_index[i].instr_name) 
		{
			res = seq_instr_index[i].instr_code;		
			flag = true;
		}
	}
	return res;
}

QList<uint8_t> SequenceWizard::findInstrValue(const QString &str, const Sequence &seq, bool &flag)
{
	QList<uint8_t> res;
	flag = false;
	for (int i = 0; i < seq.param_list.count(); i++)
	{
		if (str == seq.param_list[i]->name)
		{			
			int32_t new_value = seq.param_list[i]->app_value;
			uint8_t byte1 = (uint8_t)(new_value & 0x000000FF);
			uint8_t byte2 = (uint8_t)((new_value & 0x0000FF00) >> 8);
			uint8_t byte3 = (uint8_t)((new_value & 0x00FF0000) >> 16);
			uint8_t byte4 = (uint8_t)((new_value & 0xFF000000) >> 24);
			res << byte1 << byte2 << byte3 << byte4;		

			flag = true;
			return res;
		}
	}

	return res;
}


void SequenceWizard::viewCode()
{
	if (!cur_jseq_object) return;

	////ViewCodeDialog view_code_dlg(&cur_lusi_Seq);
	ViewCodeDialog view_code_dlg(cur_jseq_object->lusi_Seq);
	if (view_code_dlg.exec());	
}

/*
void SequenceWizard::setExportSettings()
{
	ExportSettingsDialog dlg(app_settings, save_data.to_save);
	if (dlg.exec())
	{
		save_data.file_name = dlg.getFileName();
		save_data.path = dlg.getPath();
		save_data.prefix = dlg.getPrefix();
		save_data.postfix = dlg.getPostfix();
		save_data.extension = dlg.getExtension();
		save_data.to_save = dlg.isDataExported();
		save_data.to_save_all = dlg.isAllDataExported();

		app_settings->setValue("SaveDataSettings/Path", save_data.path);
		app_settings->setValue("SaveDataSettings/Prefix", save_data.prefix);			
		app_settings->setValue("SaveDataSettings/Postfix", save_data.postfix);
		app_settings->setValue("SaveDataSettings/Extension", save_data.extension);
		
		emit save_data_changed(save_data);
	}
}
*/

void SequenceWizard::readDataFileSettings()
{
	QString path = "";
	if (app_settings->contains("SaveDataSettings/Path")) path = app_settings->value("SaveDataSettings/Path").toString();
	else
	{
		QString cur_dir = QCoreApplication::applicationDirPath();		
		path = cur_dir;
		app_settings->setValue("SaveDataSettings/Path", path);
	}
	
	QString file_prefix = "";
	if (app_settings->contains("SaveDataSettings/Prefix")) file_prefix = app_settings->value("SaveDataSettings/Prefix").toString();
	else
	{
		file_prefix = "data";
		app_settings->setValue("SaveDataSettings/Prefix", file_prefix);
	}	
	
	QString file_postfix = "";
	if (app_settings->contains("SaveDataSettings/Postfix")) file_postfix = app_settings->value("SaveDataSettings/Postfix").toString();
	else
	{
		file_postfix = "";
		app_settings->setValue("SaveDataSettings/Postfix", file_postfix);
	}
	
	QString extension = "";
	QStringList items;
	items << tr("dat") << tr("txt");	
	if (app_settings->contains("SaveDataSettings/Extension")) extension = app_settings->value("SaveDataSettings/Extension").toString();
	else
	{
		extension = "dat";
		app_settings->setValue("SaveDataSettings/Extension", extension);
	}
	if (!items.contains(extension)) items << extension;	

	save_data.path = path;
	save_data.postfix = file_postfix;
	save_data.prefix = file_prefix;
	save_data.extension = extension;	
	save_data.to_save = true;
}

QString SequenceWizard::getStrItemNumber(QString text, int index, int base)
{
	QString res = "";

	QString num = QString::number(index);

	QString pre = "_";
	for (int i = 0; i < base-num.size(); i++) pre += "0";
	res += text + pre + num;

	return res;
}

LUSI::Sequence* SequenceWizard::getCurrentSequence() 
{ 	 
	LUSI::Sequence *seq = NULL;
	if (cur_jseq_object) seq = cur_jseq_object->lusi_Seq;

	return seq;
}

bool SequenceWizard::getDSPPrg(QVector<uint8_t> &_prg, QVector<uint8_t> &_instr)
{	
	LUSI::Sequence *cur_lusi_Seq = cur_jseq_object->lusi_Seq;

	bool flag = true;
	if (!cur_lusi_Seq->seq_errors.isEmpty()) flag = false;
	if (!cur_lusi_Seq->comprg_errors.isEmpty()) flag = false;
	if (!cur_lusi_Seq->procdsp_errors.isEmpty()) flag = false;
	if (!cur_lusi_Seq->cond_errors.isEmpty()) flag = false;
	if (!cur_lusi_Seq->js_error.isEmpty()) flag = false;

	if (!flag) return false;

	_prg.clear();
	QByteVector cmd_prg = cur_lusi_Seq->com_programs.first();	// временно ! ѕринимаетс€ только перва€ программа дл€ интервального программатора
	if (cmd_prg.isEmpty()) return false;
	_prg << cmd_prg;
		
	_instr.clear();
	if (cur_lusi_Seq->proc_programs.isEmpty()) return false;
	for (int i = 0; i < cur_lusi_Seq->proc_programs.count(); i++)
	{
		QByteVector instr_pack = cur_lusi_Seq->proc_programs[i];
		_instr << instr_pack;
		if (i < cur_lusi_Seq->proc_programs.count()-1)
		{
			_instr.append(0xFF);
		}
	}

	return true;
}

bool SequenceWizard::getDSPPrg(QString &jseq_name, QVector<uint8_t> &_prg, QVector<uint8_t> &_instr)
{
	//LUSI::Sequence *cur_lusi_Seq = cur_jseq_object->lusi_Seq;
	LUSI::Sequence *lusi_Seq = NULL;
	for (int i = 0; i < jseq_objects.count(); i++)
	{
		JSeqObject *jseq_obj = jseq_objects.at(i);
		lusi_Seq = jseq_obj->lusi_Seq;
		if (lusi_Seq->file_name == jseq_name) break;
	}
	if (lusi_Seq == NULL) return false;

	bool flag = true;
	if (!lusi_Seq->seq_errors.isEmpty()) flag = false;
	if (!lusi_Seq->comprg_errors.isEmpty()) flag = false;
	if (!lusi_Seq->procdsp_errors.isEmpty()) flag = false;
	if (!lusi_Seq->cond_errors.isEmpty()) flag = false;
	if (!lusi_Seq->js_error.isEmpty()) flag = false;

	if (!flag) return false;

	_prg.clear();
	QByteVector cmd_prg = lusi_Seq->com_programs.first();	// временно ! ѕринимаетс€ только перва€ программа дл€ интервального программатора
	if (cmd_prg.isEmpty()) return false;
	_prg << cmd_prg;

	_instr.clear();
	if (lusi_Seq->proc_programs.isEmpty()) return false;
	for (int i = 0; i < lusi_Seq->proc_programs.count(); i++)
	{
		QByteVector instr_pack = lusi_Seq->proc_programs[i];
		_instr << instr_pack;
		if (i < lusi_Seq->proc_programs.count()-1)
		{
			_instr.append(0xFF);
		}
	}

	return true;
}

JSeqObject *SequenceWizard::getJSeqObject(QString jseq_file)
{	
	for (int i = 0; i < jseq_objects.count(); i++)
	{
		JSeqObject *jseq_obj = jseq_objects[i];
		QFileInfo file_info(jseq_obj->jseq_file);
		QString jseq_obj_file = file_info.fileName();
		if (jseq_file == jseq_obj_file)
		{
			return jseq_obj;
		}
	}

	return NULL;
}


QString SequenceWizard::simplifyFormula(QString _formula, Sequence *_seq)
{
	QList<QPair<int,int> > list1;	
	for (int i = 0; i < _seq->param_list.count(); i++)
	{
		Sequence_Param *param = _seq->param_list[i];
		int name_len = param->name.length();
		list1 << QPair<int,int>(i,name_len);
	}
	if (list1.isEmpty()) return "";

	for (int i = 0; i < list1.count(); i++)
	{
		int index = i;
		QPair<int,int> _min = QPair<int,int>(list1[i].first,list1[i].second);		
		for (int j = i+1; j < list1.count(); j++)
		{
			if (_min.second > list1[j].second) 
			{
				_min = list1[j];
				index = j;
			}
		}
		list1.removeAt(index);
		list1.push_front(_min);
	}

	for (int i = 0; i < list1.count(); i++)
	{
		int index = list1[i].first;
		Sequence_Param *param = _seq->param_list[index];		
		if (_formula.contains(param->name))
		{
			QString num_str = QString::number(param->def_value);
			_formula = _formula.replace(param->name, num_str, Qt::CaseInsensitive);
		}
	}

	return _formula;
}

double SequenceWizard::calcArgument(double index, Argument* arg, bool *_ok)
{
	QString _formula = arg->cur_formula;
	QString _idx = arg->index_name;
	double res = 0;

	bool ok = false;
	if (_formula.contains(_idx))
	{
		QString num_str = QString::number(index);
		_formula = _formula.replace(_idx, num_str, Qt::CaseInsensitive);

		QScriptEngine loc_engine;
		QScriptValue qscr_val = loc_engine.evaluate(_formula);
		if (!qscr_val.isError()) 
		{
			res = qscr_val.toVariant().toDouble(&ok);	
			if (!ok) res = 0;
			*_ok = ok;
		}		
		else *_ok = false;		
	}
	else *_ok = false;

	return res;
}

int SequenceWizard::calcActualDataSize(QString &formula, bool *_ok)
{
	double res = 0;

	bool ok = false;	
	QScriptEngine loc_engine;
	QScriptValue qscr_val = loc_engine.evaluate(formula);
	if (!qscr_val.isError()) 
	{
		res = qscr_val.toVariant().toInt(&ok);	
		if (!ok) res = 0;
		*_ok = ok;
	}		
	else *_ok = false;		

	return res;
}


void SequenceWizard::showSequenceInfo()
{
	if (!cur_jseq_object) return;

	LUSI::Sequence *cur_lusi_Seq = cur_jseq_object->lusi_Seq;
	QString memo = "<font color = darkblue>Authors:</font> " + cur_lusi_Seq->author + "<br><br>";
	memo += "<font color = darkblue>Created:</font> " + cur_lusi_Seq->datetime + "<br><br>";
	memo += "<font color = darkblue>Description:</font> " + cur_lusi_Seq->description + "<br><br>";

	SequenceInfoDialog dlg(memo);
	dlg.exec();
}

