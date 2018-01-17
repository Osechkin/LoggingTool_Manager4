#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QDir>

//#include <iostream>
//#include <Windows.h>
//#include <stdlib.h>

#include "../Communication/message_class.h"
//#include "../main_window.h"

#include "scheduler_wizard.h"


SchedulerWizard::SchedulerWizard(SequenceWizard *seq_wiz, DepthTemplateWizard *depth_wiz, NMRToolLinker *nmrtool_wiz, Clocker *clocker, QWidget *parent) : QWidget(parent), ui(new Ui::SchedulerWizard)
{
	ui->setupUi(this);

	sequence_wizard = seq_wiz;
	depth_wizard = depth_wiz;	
	nmrtool_linker = nmrtool_wiz;
	this->clocker = clocker;
	
	current_cmd = NULL;
	seq_already_finished = true;

	ui->tbtAdd->setIcon(QIcon(":/images/add_button.png"));
	ui->tbtRemove->setIcon(QIcon(":/images/disconnect.png"));
	ui->tbtOpen->setIcon(QIcon(":/images/open.png"));
	ui->tbtSave->setIcon(QIcon(":/images/save.png"));
		
	QStringList headlist;
	ui->treeWidgetParam->setColumnCount(3);
	headlist << tr("Parameter") << tr("Value") << tr("Units");
	ui->treeWidgetParam->setHeaderLabels(headlist);
	ui->treeWidgetParam->setColumnWidth(0,120);
	ui->treeWidgetParam->setColumnWidth(1,250);	
	ui->treeWidgetParam->header()->setFont(QFont("Arial", 10, 0, false));

	QMenu *menu_add = new QMenu(this);
	QAction *a_exec = new QAction("EXEC", this);
	QAction *a_distance_range = new QAction("POSITION_LOOP - END_LOOP", this);
	QAction *a_set_position = new QAction("SET_POSITION", this);
	QAction *a_loop = new QAction("LOOP - END_LOOP", this);
	QAction *a_sleep = new QAction("SLEEP", this);
	menu_add->addAction(a_exec);	
	menu_add->addAction(a_set_position);
	menu_add->addAction(a_loop);
	menu_add->addAction(a_distance_range);
	menu_add->addAction(a_sleep);
	ui->tbtAdd->setMenu(menu_add);
	//ui->tbtAdd->setPopupMode(QToolButton::InstantPopup);
	ui->tbtAdd->setPopupMode(QToolButton::MenuButtonPopup);

	QMenu *menu_remove = new QMenu(this);
	QAction *a_remove_all = new QAction("Remove All", this);
	menu_remove->addAction(a_remove_all);
	ui->tbtRemove->setMenu(menu_remove);
	ui->tbtRemove->setPopupMode(QToolButton::MenuButtonPopup);

	QMenu *menu_save = new QMenu(this);
	QAction *a_save = new QAction(tr("Save"), this);
	QAction *a_save_as = new QAction(tr("Save As..."), this);	
	menu_save->addAction(a_save);
	menu_save->addAction(a_save_as);
	ui->tbtSave->setMenu(menu_save);
	//ui->tbtSave->setPopupMode(QToolButton::InstantPopup);
	ui->tbtSave->setPopupMode(QToolButton::MenuButtonPopup);

	ui->tableWidgetExp->installEventFilter(this);
	ui->tableWidgetExp->setStyleSheet("QTableWidget::item {padding-left: 10px }");
	QItemSelectionModel *sm = ui->tableWidgetExp->selectionModel();
	connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(currentItemSelected(QModelIndex,QModelIndex)));

	scheduler_engine.clear();
	executingJSeq = NULL;

	connect(ui->tbtAdd, SIGNAL(clicked()), this, SLOT(addItemNOP()));	
	connect(a_exec, SIGNAL(triggered()), this, SLOT(addItem()));
	connect(a_set_position, SIGNAL(triggered()), this, SLOT(addItem()));
	connect(a_distance_range, SIGNAL(triggered()), this, SLOT(addItem()));
	connect(a_loop, SIGNAL(triggered()), this, SLOT(addItem()));
	connect(a_sleep, SIGNAL(triggered()), this, SLOT(addItem()));
	connect(ui->tbtRemove, SIGNAL(clicked()), this, SLOT(removeItem()));
	connect(a_remove_all, SIGNAL(triggered()), this, SLOT(removeAllItems()));

	/*main_win = NULL;
	foreach (QWidget *win, QApplication::topLevelWidgets())
	{
		if (MainWindow *mainWin = qobject_cast<MainWindow *>(win)) 
		{
			main_win = mainWin;		
			break;
		}
	}*/

	crc16_last_jseq = 0xFFFF;
}

SchedulerWizard::~SchedulerWizard()
{
	delete ui;
}

bool SchedulerWizard::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == ui->tableWidgetExp)
	{
		if (event->type() == QEvent::Resize)
		{
			int w = ui->tableWidgetExp->size().width();
			ui->tableWidgetExp->setColumnWidth(0,w);
			return true;
		}
	}
	return QWidget::eventFilter(obj, event);
}


bool SchedulerWizard::scheduling(QStringList &e)
{
	bool res = true;
	e.clear();

	Scheduler::SchedulerObjList scheduler_objects = scheduler_engine.getObjectList();
	for (int i = 0; i < scheduler_objects.count(); i++)
	{
		Scheduler::SchedulerObject *obj = scheduler_objects[i];
		Scheduler::Command cmd = obj->type;

		switch (cmd)
		{
		case Scheduler::Loop_Cmd:
			{
				Scheduler::Loop *loop_obj = qobject_cast<Scheduler::Loop*>(obj);
				if (loop_obj) loop_obj->init();
				else
				{
					e.append(tr("Unknown object in line %1!").arg(i+1));
					res = false;
				}
				break;
			}
		case Scheduler::End_Cmd:
			{
				Scheduler::End *end_obj = qobject_cast<Scheduler::End*>(obj);
				if (end_obj) 
				{
					Scheduler::SchedulerObject *ref_obj = end_obj->ref_obj;
					if (!ref_obj) 
					{
						e.append(tr("Error in line %1: 'LOOP' was not found for 'END_LOOP'!").arg(i+1));
						res = false;
					}
				}
				else 
				{
					e.append(tr("Unknown object in line %1!").arg(i+1));
					res = false;
				}				
				break;
			}
		case Scheduler::DistanceRange_Cmd:
			{
				Scheduler::DistanceRange *end_range_obj = qobject_cast<Scheduler::DistanceRange*>(obj);
				if (end_range_obj) end_range_obj->init();
				else
				{
					e.append(tr("Unknown object in line %1!").arg(i+1));
					res = false;
				}
				break;
			}	
		case Scheduler::SetPosition_Cmd:
			{
				Scheduler::SetPosition *setpos_obj = qobject_cast<Scheduler::SetPosition*>(obj);
				if (!setpos_obj) 
				{
					e.append(tr("Unknown object in line %1!").arg(i+1));
					res = false;
				}
				break;
			}
		case Scheduler::Exec_Cmd:
			{
				Scheduler::Exec *exec_obj = qobject_cast<Scheduler::Exec*>(obj);
				if (!exec_obj) 
				{
					e.append(tr("Unknown object in line %1!").arg(i+1));
					res = false;
				}
				
				QStringList ee;
				QString jseq_name = exec_obj->jseq_name;
				if (!sequence_wizard->executeJSsequence(jseq_name, ee))
				{					
					e.append(tr("Error in line %1: Errors were found in Sequence %2!").arg(i+1).arg(jseq_name));
					res = false;
				}
				break;
			}
		case Scheduler::Sleep_Cmd:
			{
				Scheduler::Sleep *sleep_obj = qobject_cast<Scheduler::Sleep*>(obj);
				if (!sleep_obj) 
				{
					e.append(tr("Unknown object in line %1!").arg(i+1));
					res = false;
				}
				break;
			}
		default: break;
		}
	}

	return res;
}

void SchedulerWizard::start()
{
	if (scheduler_engine.isEmpty()) return;

	//Scheduler::SchedulerObject *cmd_obj = scheduler_engine.get(0); 
	//if (!cmd_obj) emit finished();

	ui->tbtAdd->setEnabled(false);
	ui->tbtRemove->setEnabled(false);
	ui->tbtOpen->setEnabled(false);
	ui->treeWidgetParam->setEnabled(false);

	calibration_state = false;
	is_started = true;	
	scheduler_engine.start();
	if (current_cmd)
	{
		delete current_cmd;
		current_cmd = NULL;
	}
	obj_cmd_list.clear();
	connect(clocker, SIGNAL(clock()), this, SLOT(process()));	
	
	crc16_last_jseq = 0;

	emit started();
}

void SchedulerWizard::stop()
{
	ui->tbtAdd->setEnabled(true);
	ui->tbtRemove->setEnabled(true);
	ui->tbtOpen->setEnabled(true);
	ui->treeWidgetParam->setEnabled(true);

	disconnect(clocker, SIGNAL(clock()), this, SLOT(process()));
	scheduler_engine.stop();
	is_started = false;

	if (current_cmd) 
	{ 
		delete current_cmd; 
		current_cmd = NULL; 
	}
	obj_cmd_list.clear();
	calibration_state = false;
}

void SchedulerWizard::process()
{
	if (current_cmd) 
	{
		if (current_cmd->isFinished())
		{
			delete current_cmd;
			current_cmd = NULL;
			executingJSeq = NULL;
		}
		return;
	}

	if (obj_cmd_list.isEmpty() && is_started && current_cmd == NULL) 
	{
		Scheduler::SchedulerObject *cmd_obj = scheduler_engine.next();
		if (!cmd_obj) 
		{
			is_started = false;
			executingJSeq = NULL;
			crc16_last_jseq = 0;
			stop();
			emit finished();
			return;
		}

		Scheduler::Command cmd_type = cmd_obj->type;
		switch (cmd_type)
		{
		case Scheduler::Exec_Cmd:			
			{
				obj_cmd_list.append(cmd_obj); 
				break;
			}
		case Scheduler::SetPosition_Cmd:	obj_cmd_list.append(cmd_obj); break;
		case Scheduler::Sleep_Cmd:			obj_cmd_list.append(cmd_obj); break;
		case Scheduler::DistanceRange_Cmd:	
			{
				Scheduler::DistanceRange *dist_obj = qobject_cast<Scheduler::DistanceRange*>(cmd_obj);
				if (dist_obj)
				{
					dist_obj->getNextPos();		
					if (!dist_obj->finished) obj_cmd_list.append(dist_obj); 						
				}
				break;
			}		
		case Scheduler::Loop_Cmd: 		
			{
				Scheduler::Loop *loop_obj = qobject_cast<Scheduler::Loop*>(cmd_obj);
				if (loop_obj)
				{
					loop_obj->getNextIndex();
					if (!loop_obj->finished) obj_cmd_list.append(loop_obj); 
				}
				break;				
			}
		case Scheduler::End_Cmd:		
		case Scheduler::NoP_Cmd: break;
		default: break;
		}
		
	}		
	else if (!obj_cmd_list.isEmpty() && is_started && current_cmd == NULL)
	{
		Scheduler::SchedulerObject *cmd_obj = obj_cmd_list.front();
		//obj_cmd_list.takeFirst();
		execute(cmd_obj);
	}  
}

void SchedulerWizard::execute(Scheduler::SchedulerObject* obj)
{	
	switch (obj->type)
	{
	case Scheduler::Exec_Cmd:
		{
			Scheduler::Exec *exec_obj = qobject_cast<Scheduler::Exec*>(obj);
			if (exec_obj)
			{
				QString jseq_name = exec_obj->jseq_name;
				data_file = exec_obj->data_file; 

				static int nnn = 0; nnn++;
				executingJSeq = sequence_wizard->getJSeqObject(jseq_name);
				
				QVector<uint8_t> proc_prg;
				QVector<uint8_t> proc_instr;
				bool res = sequence_wizard->getDSPPrg(jseq_name, proc_prg, proc_instr);
				
				int full_len = proc_prg.count() + proc_instr.count();
				QVector<uint8_t> jseq_arr(full_len);
				memcpy(jseq_arr.data(), proc_prg.data(), proc_prg.count());
				memcpy(jseq_arr.data()+proc_prg.count(), proc_instr.data(), proc_instr.count());
				unsigned int crc16_jseq = Crc16(jseq_arr.data(), full_len);
				if (crc16_jseq != crc16_last_jseq)
				{
					nmrtool_linker->applyProcPrg(proc_prg, proc_instr);
				}
				else 
				{
					nmrtool_linker->startNMRTool();
				}
				crc16_last_jseq = crc16_jseq;
					
				uint32_t uid = nmrtool_linker->getMsgContainer()->last()->uid;
				current_cmd = new Scheduler::CommandController(uid, Scheduler::Exec_Cmd);
			}
			break;
		}
	case Scheduler::DistanceRange_Cmd:
		{			
			Scheduler::DistanceRange *dist_range_obj = qobject_cast<Scheduler::DistanceRange*>(obj);
			if (dist_range_obj)
			{
				AbstractDepthMeter *abs_depthmeter = depth_wizard->getCurrentDepthMeter();
				if (abs_depthmeter)
				{
					if (abs_depthmeter->getType() == AbstractDepthMeter::LeuzeDistanceMeter)
					{
						if (LeuzeDistanceMeterWidget *leuze_meter = qobject_cast<LeuzeDistanceMeterWidget*>(abs_depthmeter))
						{
							removeSchedulerObj(obj);

							current_cmd = new Scheduler::CommandController(0, Scheduler::DistanceRange_Cmd);
							connect(leuze_meter, SIGNAL(cmd_resulted(bool, uint32_t)), current_cmd, SLOT(processResult(bool, uint32_t)));

							double pos = dist_range_obj->pos/100; // [cm] -> [m]
							leuze_meter->setPosition(pos);
							bool is_calibration_range = dist_range_obj->is_calibration_range;
							if (!calibration_state && is_calibration_range)
							{
								calibration_state = is_calibration_range;
								emit calibration_started();
							}
							else if (calibration_state && !is_calibration_range)
							{
								calibration_state = is_calibration_range;
								emit calibration_finished();
							}

							//current_cmd = new Scheduler::CommandController(0, Scheduler::DistanceRange_Cmd);
							//connect(leuze_meter, SIGNAL(cmd_resulted(bool, uint32_t)), current_cmd, SLOT(processResult(bool, uint32_t)));
						}
					}
				}
			}			
			break;			
		}
	case Scheduler::SetPosition_Cmd:
		{
			Scheduler::SetPosition *set_pos_obj = qobject_cast<Scheduler::SetPosition*>(obj);
			if (set_pos_obj)
			{
				AbstractDepthMeter *abs_depthmeter = depth_wizard->getCurrentDepthMeter();
				if (abs_depthmeter)
				{
					if (abs_depthmeter->getType() == AbstractDepthMeter::LeuzeDistanceMeter)
					{
						if (LeuzeDistanceMeterWidget *leuze_meter = qobject_cast<LeuzeDistanceMeterWidget*>(abs_depthmeter))
						{
							removeSchedulerObj(obj);

							double pos = set_pos_obj->position/100;	// [cm] -> [m]
							leuze_meter->setPosition(pos);

							current_cmd = new Scheduler::CommandController(0, Scheduler::SetPosition_Cmd);
							connect(leuze_meter, SIGNAL(cmd_resulted(bool, uint32_t)), current_cmd, SLOT(processResult(bool, uint32_t)));
						}
					}
				}
			}	
			break;
		}
	case Scheduler::Loop_Cmd:
		{
			break;
		}
	case Scheduler::End_Cmd:
	case Scheduler::Sleep_Cmd: break;
	default:
		break;
	}
	
}

QString SchedulerWizard::getDataFile()
{
	/*QString _data_file = "";
	if (executingJSeq) 
	{
		_data_file = executingJSeq->data_file;
	}

	return _data_file;*/

	return data_file;
}

void SchedulerWizard::setSeqStatus(unsigned char _seq_finished)
{
	if (!current_cmd) return;
	if (current_cmd->getType() != Scheduler::Exec_Cmd) return;
	if (seq_already_finished) return;

	if (_seq_finished)
	{
		current_cmd->processResult(true, 0);
		seq_already_finished = true;
	}
}

void SchedulerWizard::setSeqStarted(bool flag)
{
	if (obj_cmd_list.isEmpty()) return;
	Scheduler::SchedulerObject *cmd_obj = obj_cmd_list.front();
	if (cmd_obj->type != Scheduler::Exec_Cmd) return;

	Scheduler::Exec *exec_obj = qobject_cast<Scheduler::Exec*>(cmd_obj);
	if (!exec_obj) return;

	if (flag) 
	{
		seq_already_finished = false;
		obj_cmd_list.takeFirst();	
	}
	else
	{		
		QString jseq_name = exec_obj->jseq_name;
		data_file = exec_obj->data_file; 

		QVector<uint8_t> proc_prg;
		QVector<uint8_t> proc_instr;
		bool res = sequence_wizard->getDSPPrg(jseq_name, proc_prg, proc_instr);

		int full_len = proc_prg.count() + proc_instr.count();
		QVector<uint8_t> jseq_arr(full_len);
		memcpy(jseq_arr.data(), proc_prg.data(), proc_prg.count());
		memcpy(jseq_arr.data()+proc_prg.count(), proc_instr.data(), proc_instr.count());
		unsigned int crc16_jseq = Crc16(jseq_arr.data(), full_len);
		if (crc16_jseq != crc16_last_jseq)
		{
			nmrtool_linker->applyProcPrg(proc_prg, proc_instr);
		}
		else 
		{
			nmrtool_linker->startNMRTool();
		}
		crc16_last_jseq = crc16_jseq;
	}
}

bool SchedulerWizard::generateDistanceScanPrg(QStringList &e)
{
	Scheduler::SchedulerObjList cmd_obj_list;

	Scheduler::DistanceRange *dist_range_obj = new Scheduler::DistanceRange;
	AbstractDepthMeter *abs_depthmeter = depth_wizard->getCurrentDepthMeter();
	if (abs_depthmeter)
	{
		if (abs_depthmeter->getType() == AbstractDepthMeter::LeuzeDistanceMeter)
		{
			if (LeuzeDistanceMeterWidget *leuze_meter = qobject_cast<LeuzeDistanceMeterWidget*>(abs_depthmeter))
			{
				QPair<double,double> bounds = leuze_meter->getBounds();
				QPair<double,double> from_to = leuze_meter->getFromTo();
				double _step = leuze_meter->getStep();
				double calibr_range = leuze_meter->getCalibrationLength();
				bool _do_calibr = false;
				
				dist_range_obj->setFromToStep(from_to, _step, _do_calibr);
				dist_range_obj->setBounds(bounds);
				dist_range_obj->setCalibrationLength(calibr_range);
			}				
		}
		else
		{
			e.append(tr("Error: Cannot find Leuze distance meter!"));
			return false;
		}
	}
	connect(dist_range_obj, SIGNAL(changed()), this, SLOT(update()));

	data_file = generateDataFileName();
	QString jseq_file = sequence_wizard->getCurJSeqObject()->jseq_file;
	Scheduler::Exec *exec_obj = new Scheduler::Exec(jseq_list, jseq_file, data_file);
	connect(exec_obj, SIGNAL(changed()), this, SLOT(update()));
	
	Scheduler::End *end_obj = new Scheduler::End;
	end_obj->ref_obj = dist_range_obj;
	
	cmd_obj_list.append(dist_range_obj);	// Add POSITION_LOOP
	cmd_obj_list.append(exec_obj);			// Add EXEC
	cmd_obj_list.append(end_obj);			// Add END_LOOP

	
	for (int i = 0; i < cmd_obj_list.count(); i++)
	{
		int rows = ui->tableWidgetExp->rowCount();
		int cur_row = ui->tableWidgetExp->currentRow();	

		Scheduler::SchedulerObject *cmd_obj = cmd_obj_list.at(i);
		if (cur_row < 0 || cur_row == rows-1) 
		{		
			ui->tableWidgetExp->setRowCount(rows+1);
			scheduler_engine.add(cmd_obj);

			QLabel *lb = new QLabel(cmd_obj->cell_text);
			ui->tableWidgetExp->setCellWidget(rows, 0, lb);
			ui->tableWidgetExp->setCurrentCell(rows, 0);		
		}
		else 
		{		
			scheduler_engine.insert(cur_row, cmd_obj);			
			insertItem(cur_row, cmd_obj->cell_text);			
		}	
	}

	return true;
}

bool SchedulerWizard::generateExecPrg(QStringList &e)
{
	Scheduler::SchedulerObjList cmd_obj_list;

	data_file = generateDataFileName();
	QString jseq_file = sequence_wizard->getCurJSeqObject()->jseq_file;
	Scheduler::Exec *exec_obj = new Scheduler::Exec(jseq_list, jseq_file, data_file);
	connect(exec_obj, SIGNAL(changed()), this, SLOT(update()));
	
	cmd_obj_list.append(exec_obj);			// Add EXEC
	
	for (int i = 0; i < cmd_obj_list.count(); i++)
	{
		int rows = ui->tableWidgetExp->rowCount();
		int cur_row = ui->tableWidgetExp->currentRow();	

		Scheduler::SchedulerObject *cmd_obj = cmd_obj_list.at(i);
		if (cur_row < 0 || cur_row == rows-1) 
		{		
			ui->tableWidgetExp->setRowCount(rows+1);
			scheduler_engine.add(cmd_obj);

			QLabel *lb = new QLabel(cmd_obj->cell_text);
			ui->tableWidgetExp->setCellWidget(rows, 0, lb);
			ui->tableWidgetExp->setCurrentCell(rows, 0);		
		}
		else 
		{		
			scheduler_engine.insert(cur_row, cmd_obj);			
			insertItem(cur_row, cmd_obj->cell_text);			
		}	
	}

	return true;
}


void SchedulerWizard::addItemNOP()
{
	int rows = ui->tableWidgetExp->rowCount();
	int cur_row = ui->tableWidgetExp->currentRow();	

	if (cur_row < 0 || cur_row == rows-1) 
	{
		ui->tableWidgetExp->setRowCount(rows+1);
		Scheduler::NOP *cmd_nop = new Scheduler::NOP;
		scheduler_engine.add(cmd_nop);
		
		QLabel *lb = new QLabel(cmd_nop->cell_text);
		ui->tableWidgetExp->setCellWidget(rows, 0, lb);
		ui->tableWidgetExp->setCurrentCell(rows, 0);		
	}
	else 
	{
		Scheduler::NOP *cmd_nop = new Scheduler::NOP;
		scheduler_engine.insert(cur_row, cmd_nop);
		insertItem(cur_row, cmd_nop->cell_text);		
	}
}

void SchedulerWizard::addItem()
{	
	QAction *a = qobject_cast<QAction*>(sender());
	if (!a) return;
	QString txt = a->text();

	Scheduler::SchedulerObjList cmd_obj_list;	
	if (txt == "EXEC")
	{
		data_file = generateDataFileName();
		QString jseq_file = sequence_wizard->getCurJSeqObject()->jseq_file;
		Scheduler::Exec *exec_obj = new Scheduler::Exec(jseq_list, jseq_file, data_file);
		connect(exec_obj, SIGNAL(changed()), this, SLOT(update()));
		cmd_obj_list.append(exec_obj);
	}
	else if (txt == "POSITION_LOOP - END_LOOP")	  
	{ 
		Scheduler::DistanceRange *dist_range_obj = new Scheduler::DistanceRange;
		AbstractDepthMeter *abs_depthmeter = depth_wizard->getCurrentDepthMeter();
		if (abs_depthmeter)
		{
			if (abs_depthmeter->getType() == AbstractDepthMeter::LeuzeDistanceMeter)
			{
				if (LeuzeDistanceMeterWidget *leuze_meter = qobject_cast<LeuzeDistanceMeterWidget*>(abs_depthmeter))
				{
					double zero_pos = leuze_meter->getZeroPos();
					QPair<double,double> bounds = leuze_meter->getBounds();
					bounds.first -= zero_pos;
					bounds.second -= zero_pos;

					QPair<double,double> from_to = leuze_meter->getFromTo();										
					double _step = leuze_meter->getStep();
					double _calibr_len = leuze_meter->getCalibrationLength();
					
					dist_range_obj->setFromToStep(from_to, _step, true);
					dist_range_obj->setBounds(bounds);
					dist_range_obj->setCalibrationLength(_calibr_len);
				}				
			}
		}

		connect(dist_range_obj, SIGNAL(changed()), this, SLOT(update()));

		cmd_obj_list.append(dist_range_obj); 		
		//cmd_obj_list.append(new Scheduler::End); 

		Scheduler::End *end_obj = new Scheduler::End;
		end_obj->ref_obj = dist_range_obj;

		cmd_obj_list.append(end_obj); 
	}
	else if (txt == "SET_POSITION")	
	{
		Scheduler::SetPosition *dist_obj = new Scheduler::SetPosition;
		AbstractDepthMeter *abs_depthmeter = depth_wizard->getCurrentDepthMeter();
		if (abs_depthmeter)
		{
			if (abs_depthmeter->getType() == AbstractDepthMeter::LeuzeDistanceMeter)
			{
				if (LeuzeDistanceMeterWidget *leuze_meter = qobject_cast<LeuzeDistanceMeterWidget*>(abs_depthmeter))
				{
					QPair<double,double> bounds = leuze_meter->getBounds();
					double leuze_pos = 100*leuze_meter->getOrderedDepth(); // convert to [cm]
					dist_obj->changePosition(leuze_pos);
					dist_obj->setBounds(bounds);
				}				
			}
		}
		connect(dist_obj, SIGNAL(changed()), this, SLOT(update()));
		cmd_obj_list.append(dist_obj);
	}
	else if (txt == "LOOP - END_LOOP")			  
	{ 
		Scheduler::Loop *loop_obj = new Scheduler::Loop;
		connect(loop_obj, SIGNAL(changed()), this, SLOT(update()));

		cmd_obj_list.append(loop_obj); 
		//cmd_obj_list.append(new Scheduler::End); 

		Scheduler::End *end_obj = new Scheduler::End;
		end_obj->ref_obj = loop_obj;

		cmd_obj_list.append(end_obj);
	}	
	else if (txt == "SLEEP")	
	{
		Scheduler::Sleep *sleep_obj = new Scheduler::Sleep;
		connect(sleep_obj, SIGNAL(changed()), this, SLOT(update()));

		cmd_obj_list.append(sleep_obj); 		
	}
	if (cmd_obj_list.isEmpty()) return;

	for (int i = 0; i < cmd_obj_list.count(); i++)
	{
		int rows = ui->tableWidgetExp->rowCount();
		int cur_row = ui->tableWidgetExp->currentRow();	

		Scheduler::SchedulerObject *cmd_obj = cmd_obj_list.at(i);
		if (cur_row < 0) 
		{			
			scheduler_engine.insert(0, cmd_obj);
			insertItem(0, cmd_obj->cell_text);
		}
		else if (cur_row == rows-1)
		{
			scheduler_engine.add(cmd_obj);
			ui->tableWidgetExp->setRowCount(rows+1);
			QLabel *lb = new QLabel(cmd_obj->cell_text);
			ui->tableWidgetExp->setCellWidget(rows, 0, lb);
			ui->tableWidgetExp->setCurrentCell(rows, 0);			
		}
		else
		{			
			scheduler_engine.insert(cur_row+1, cmd_obj);
			insertItem(cur_row+1, cmd_obj->cell_text);
		}		
	}	
}

void SchedulerWizard::insertItem(int row, QString cmd)
{
	if (row < 0) return;

	ui->tableWidgetExp->insertRow(row);
	
	QLabel *lb = new QLabel(cmd);
	ui->tableWidgetExp->setCellWidget(row, 0, lb);
	ui->tableWidgetExp->setCurrentCell(row, 0);
}

void SchedulerWizard::removeItem(int row)
{
	int rows = scheduler_engine.getObjectList().count();
	if (row < 0 || row >= rows) return;
	
	ui->tableWidgetExp->removeRow(row);
	scheduler_engine.remove(row);
}


void SchedulerWizard::removeItem()
{
	int cur_row = ui->tableWidgetExp->currentRow();	
	int table_rows = ui->tableWidgetExp->rowCount();
	if (cur_row < 0) return;	

	Scheduler::SchedulerObject *del_obj1 = scheduler_engine.getObjectList()[cur_row];
	Scheduler::Command cmd_type = del_obj1->type;
	if (cmd_type == Scheduler::Command::Loop_Cmd || cmd_type == Scheduler::Command::DistanceRange_Cmd)
	{		
		Scheduler::SchedulerObject *del_obj2 = NULL;
		int rows = scheduler_engine.getObjectList().count();
		for (int i = 0; i < rows; i++)
		{
			Scheduler::Command cmd_type = scheduler_engine.getObjectList()[i]->type;
			if (cmd_type == Scheduler::Command::End_Cmd)
			{				
				Scheduler::End *del_end = qobject_cast<Scheduler::End*>(scheduler_engine.getObjectList()[i]);
				if (del_end)
				{
					Scheduler::SchedulerObject *ref_obj = del_end->ref_obj;
					if (ref_obj == del_obj1) del_obj2 = del_end;					
				}

				int pos_del1 = scheduler_engine.getObjectList().indexOf(del_obj1);
				ui->tableWidgetExp->removeRow(pos_del1);
				scheduler_engine.remove(pos_del1);

				int pos_del2 = scheduler_engine.getObjectList().indexOf(del_obj2);
				ui->tableWidgetExp->removeRow(pos_del2);
				scheduler_engine.remove(pos_del2);
				break;
			}
		}		
	}
	else if (cmd_type == Scheduler::Command::End_Cmd)
	{
		Scheduler::End *end_obj = qobject_cast<Scheduler::End*>(del_obj1);
		Scheduler::SchedulerObject *del_obj2 = end_obj->ref_obj;
		
		int pos_del1 = scheduler_engine.getObjectList().indexOf(del_obj1);
		ui->tableWidgetExp->removeRow(pos_del1);
		scheduler_engine.remove(pos_del1);

		int pos_del2 = scheduler_engine.getObjectList().indexOf(del_obj2);
		ui->tableWidgetExp->removeRow(pos_del2);
		scheduler_engine.remove(pos_del2);
	}
	else
	{
		ui->tableWidgetExp->removeRow(cur_row);
		scheduler_engine.remove(cur_row);
	}
}

void SchedulerWizard::removeAllItems()
{
	int rows = scheduler_engine.getObjectList().count();

	for (int i = 0; i < rows; i++)
	{
		removeItem(0);
	}
}

void SchedulerWizard::update()
{
	Scheduler::SchedulerObject *obj_sender = qobject_cast<Scheduler::SchedulerObject*>(sender());
	Scheduler::Command cmd_sender = obj_sender->type;

	int rows = scheduler_engine.getObjectList().count();
	for (int i = 0; i < rows; i++)
	{
		Scheduler::SchedulerObject *obj = scheduler_engine.getObjectList()[i];
		if (obj == obj_sender)
		{
			switch (cmd_sender)
			{
			case Scheduler::Command::DistanceRange_Cmd:	
				{
					QLabel *lb = qobject_cast<QLabel*>(ui->tableWidgetExp->cellWidget(i,0));
					Scheduler::DistanceRange *dist_range_obj = qobject_cast<Scheduler::DistanceRange*>(obj_sender);
					dist_range_obj->cell_text = dist_range_obj->cell_text_template.arg(dist_range_obj->from).arg(dist_range_obj->step).arg(dist_range_obj->to).arg(dist_range_obj->do_calibr ? "TRUE" : "FALSE");
					lb->setText(dist_range_obj->cell_text);
					break;
				}
			case Scheduler::Command::Exec_Cmd:		
				{
					QLabel *lb = qobject_cast<QLabel*>(ui->tableWidgetExp->cellWidget(i,0));
					Scheduler::Exec *exec_obj = qobject_cast<Scheduler::Exec*>(obj_sender);
					exec_obj->cell_text = exec_obj->cell_text_template.arg(exec_obj->jseq_name);
					lb->setText(exec_obj->cell_text);
					break;
				}
			case Scheduler::Command::Loop_Cmd:			
				{
					QLabel *lb = qobject_cast<QLabel*>(ui->tableWidgetExp->cellWidget(i,0));
					Scheduler::Loop *loop_obj = qobject_cast<Scheduler::Loop*>(obj_sender);
					loop_obj->cell_text = loop_obj->cell_text_template.arg(loop_obj->counts);
					lb->setText(loop_obj->cell_text);
					break;
				}
			case Scheduler::Command::SetPosition_Cmd:	
				{
					QLabel *lb = qobject_cast<QLabel*>(ui->tableWidgetExp->cellWidget(i,0));
					Scheduler::SetPosition *setpos_obj = qobject_cast<Scheduler::SetPosition*>(obj_sender);
					setpos_obj->cell_text = setpos_obj->cell_text_template.arg(setpos_obj->position);
					lb->setText(setpos_obj->cell_text);
					break;
				}			
			case Scheduler::Command::Sleep_Cmd:	
				{
					QLabel *lb = qobject_cast<QLabel*>(ui->tableWidgetExp->cellWidget(i,0));
					Scheduler::Sleep *sleep_obj = qobject_cast<Scheduler::Sleep*>(obj_sender);
					sleep_obj->cell_text = sleep_obj->cell_text_template.arg(sleep_obj->delay);
					lb->setText(sleep_obj->cell_text);
					break;
				}	
			case Scheduler::Command::End_Cmd:			break;
			case Scheduler::Command::NoP_Cmd:			break;
			default: break;
			}
		}
	}
}

// Read text in tableWidgetExp excluding HTML QString text:
// QString  txt = qobject_cast<QLabel*>(ui->tableWidgetExp->cellWidget(cur_row,0))->text().remove(QRegExp("<[^>]*>"));


void SchedulerWizard::removeSchedulerObj(Scheduler::SchedulerObject *obj)
{
	for (int i = 0; i < obj_cmd_list.count(); i++)
	{
		Scheduler::SchedulerObject *o = obj_cmd_list[i];
		if (obj == o) obj_cmd_list.removeAt(i);
	}
}

void SchedulerWizard::showItemParameters(Scheduler::SchedulerObject *obj)
{	
	//ui->treeWidgetParam->clear();
	
	QWidgetList widget_params;
	Scheduler::Command obj_type = obj->type;
	switch (obj_type)
	{
	case Scheduler::Command::DistanceRange_Cmd:	
		{
			Scheduler::DistanceRange *dist_range_obj = qobject_cast<Scheduler::DistanceRange*>(obj);
			if (!dist_range_obj) return;

			QDoubleSpinBox *dsboxFrom = new QDoubleSpinBox;				
			dsboxFrom->setMinimum(dist_range_obj->lower_bound);
			dsboxFrom->setMaximum(dist_range_obj->upper_bound);
			dsboxFrom->setValue(dist_range_obj->from);	
			connect(dsboxFrom, SIGNAL(valueChanged(double)), dist_range_obj, SLOT(changeFrom(double)));

			QDoubleSpinBox *dsboxTo = new QDoubleSpinBox;				
			dsboxTo->setMinimum(dist_range_obj->lower_bound);
			dsboxTo->setMaximum(dist_range_obj->upper_bound);
			dsboxTo->setValue(dist_range_obj->to);	
			connect(dsboxTo, SIGNAL(valueChanged(double)), dist_range_obj, SLOT(changeTo(double)));

			QDoubleSpinBox *dsboxStep = new QDoubleSpinBox;					
			dsboxStep->setValue(dist_range_obj->step);	
			dsboxStep->setMinimum(0.2);		// minimal step is 0.2 cm
			connect(dsboxStep, SIGNAL(valueChanged(double)), dist_range_obj, SLOT(changeStep(double)));
					
			QComboBox *cboxDoCalibr = new QComboBox;
			cboxDoCalibr->addItems(QStringList() << "TRUE" << "FALSE");
			cboxDoCalibr->setCurrentText(dist_range_obj->do_calibr ? "TRUE" : "FALSE");
			connect(cboxDoCalibr, SIGNAL(currentIndexChanged(const QString&)), dist_range_obj, SLOT(changeDoCalibr(const QString&)));

			widget_params << dsboxFrom << dsboxTo << dsboxStep << cboxDoCalibr;

			break;
		}
	case Scheduler::Command::Exec_Cmd:
		{
			Scheduler::Exec *exec_obj = qobject_cast<Scheduler::Exec*>(obj);
			if (!exec_obj) return;
						
			QComboBox *cboxJSeqs = new QComboBox;
			cboxJSeqs->addItems(exec_obj->jseq_list);
			cboxJSeqs->setCurrentText(exec_obj->jseq_name);
			connect(cboxJSeqs, SIGNAL(currentIndexChanged(const QString&)), exec_obj, SLOT(changeJSeq(const QString&)));

			Scheduler::FileBrowser *fileBrowser = new Scheduler::FileBrowser(exec_obj->data_file);
			fileBrowser->setToolTip(exec_obj->data_file);
			connect(fileBrowser, SIGNAL(new_filename(const QFileInfo&)), exec_obj, SLOT(editFileName(const QFileInfo&)));

			//QWidgetList widget_params;
			widget_params << cboxJSeqs << fileBrowser;

			break;
		}
	case Scheduler::Command::SetPosition_Cmd:	
		{
			Scheduler::SetPosition *setpos_obj = qobject_cast<Scheduler::SetPosition*>(obj);
			if (!setpos_obj) return;

			QDoubleSpinBox *dsboxPos = new QDoubleSpinBox;				
			dsboxPos->setMinimum(setpos_obj->lower_bound);
			dsboxPos->setMaximum(setpos_obj->upper_bound);
			dsboxPos->setValue(setpos_obj->position);	
			connect(dsboxPos, SIGNAL(valueChanged(double)), setpos_obj, SLOT(changePosition(double)));

			widget_params << dsboxPos;

			break;
		}
	case Scheduler::Command::Loop_Cmd:			
		{
			Scheduler::Loop *loop_obj = qobject_cast<Scheduler::Loop*>(obj);
			if (!loop_obj) return;

			QSpinBox *sboxCounts = new QSpinBox;				
			sboxCounts->setMinimum(loop_obj->lower_bound);
			sboxCounts->setValue(loop_obj->counts);	
			connect(sboxCounts, SIGNAL(valueChanged(int)), loop_obj, SLOT(changeCounts(int)));
						
			widget_params << sboxCounts;

			break;
		}
	case Scheduler::Command::Sleep_Cmd:			
		{
			Scheduler::Sleep *sleep_obj = qobject_cast<Scheduler::Sleep*>(obj);
			if (!sleep_obj) return;

			QSpinBox *sboxDelay = new QSpinBox;				
			sboxDelay->setMinimum(1);
			sboxDelay->setMaximum(sleep_obj->upper_bound);
			sboxDelay->setValue(sleep_obj->delay);	
			connect(sboxDelay, SIGNAL(valueChanged(int)), sleep_obj, SLOT(changeDelay(int)));

			widget_params << sboxDelay;

			break;
		}
	case Scheduler::Command::End_Cmd:			break;
	case Scheduler::Command::NoP_Cmd:			break;
	default: break;
	}

	int items_count = obj->param_objects.count();
	for (int i = 0; i < items_count; i++)
	{
		Scheduler::SettingsItem *param_item = obj->param_objects[i];
		QLabel *lbl_title = new QLabel(param_item->title);
		QPalette palette = lbl_title->palette();
		palette.setColor(QPalette::Text, (QColor(Qt::darkBlue)));		
		lbl_title->setPalette(palette);
		QWidget *widget = widget_params[i];
		QLabel *lbl_units = new QLabel(param_item->units);
		lbl_units->setPalette(palette);

		QTreeWidgetItem *twi = new QTreeWidgetItem(ui->treeWidgetParam);
		ui->treeWidgetParam->setItemWidget(twi, 0, lbl_title);
		ui->treeWidgetParam->setItemWidget(twi, 1, widget);
		ui->treeWidgetParam->setItemWidget(twi, 2, lbl_units);				
	}
}

void SchedulerWizard::currentItemSelected(QModelIndex index1, QModelIndex index2)
{
	ui->treeWidgetParam->clear();

	int row = index1.row();
	if (row < 0) return;
	Scheduler::SchedulerObject *obj = scheduler_engine.get(row);
	if (!obj) return;

	showItemParameters(obj);
}

QString SchedulerWizard::generateDataFileName()
{
	DataSave data_save = sequence_wizard->getDataFileSettings();
	QString datafile_path = data_save.path;
	QString datafile_prefix = data_save.prefix;
	QString datafile_postfix = data_save.postfix;
	QString datafile_extension = data_save.extension;

	QDateTime dateTime = QDateTime::currentDateTime();
	QString dateTime_str = dateTime.toString("yyyy-MM-dd_hh-mm-ss-zzz");
	QDir dir(datafile_path);
	QString file_name = "";
	if (!datafile_prefix.isEmpty()) file_name += datafile_prefix + "_";
	file_name += dateTime_str;
	if (!datafile_postfix.isEmpty()) file_name += "_" + datafile_postfix;
	static int file_counter = 1;
	file_name += "_" + QString::number(file_counter++);
	file_name += "." + datafile_extension;
	
	QFileInfo file_info(dir, file_name);	
	return file_info.absoluteFilePath();
}

