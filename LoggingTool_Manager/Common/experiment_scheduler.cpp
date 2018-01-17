#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>
#include <QGridLayout>
#include "QFileDialog"

#include "../Dialogs/data_filename_dialog.h"

#include "experiment_scheduler.h"


Scheduler::FileBrowser::FileBrowser(QString _file_name, QWidget *parent /* = 0 */)
{
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setObjectName(QStringLiteral("gridLayout"));
	gridLayout->setHorizontalSpacing(0);
	gridLayout->setVerticalSpacing(0);
	gridLayout->setContentsMargins(0, 0, 0, 0);
	ledFileName = new QLineEdit(this);
	ledFileName->setObjectName(QStringLiteral("ledFileName"));
	ledFileName->setReadOnly(true);

	gridLayout->addWidget(ledFileName, 0, 0, 1, 1);

	QToolButton *tbtDataFileName = new QToolButton(this);
	tbtDataFileName->setObjectName(QStringLiteral("tbtDataFileName"));
	tbtDataFileName->setText("...");
	tbtDataFileName->setToolTip(tr("Enter Data file Name"));

	gridLayout->addWidget(tbtDataFileName, 0, 1, 1, 1);

	connect(tbtDataFileName, SIGNAL(clicked()), this, SLOT(enterDataFileName()));

	ledFileName->setText(_file_name);

	QFileInfo file_info(_file_name);
	file_name = file_info.fileName();
	file_path = file_info.absoluteDir().path();
}

void Scheduler::FileBrowser::enterDataFileName()
{
	DataFileNameDialog dlg(file_path, file_name);
	if (dlg.exec()) file_name = dlg.getDataFileName();

	if (file_name.isEmpty()) ledFileName->setStyleSheet("QLineEdit {background-color: red; color: darkblue}");
	else ledFileName->setStyleSheet("QLineEdit {background-color: white; color: darkblue;}");

	QDir dir(file_path);
	QFileInfo file_info(dir, file_name);

	emit new_filename(file_info);

	QString canonical = file_info.absoluteFilePath();
	ledFileName->setText(canonical);
}


Scheduler::SchedulerObject::SchedulerObject(Command _type) 
{ 	
	type = _type; 
	switch (type)
	{
	case Command::Exec_Cmd:		
		cell_text_template = "<font size=3><font color=darkgreen>EXEC </font>( '<font color=blue>%1</font>', '<font color=blue>%2</font>' )</font>"; 
		cell_text = "";
		break;
	case Command::DistanceRange_Cmd:		
		cell_text_template = "<font size=3><font color=darkgreen>POSITION_LOOP </font>( <font color=blue>%1</font> : <font color=blue>%2</font> : <font color=blue>%3</font> , <font color=blue>%4</font> )</font>"; 
		cell_text = "";
		break;
	case Command::SetPosition_Cmd:		
		cell_text_template = "<font size=3><font color=darkgreen>SET_POSITION </font>( <font color=blue>%1</font> )</font>"; 
		cell_text = "";
		break;
	case Command::Loop_Cmd:		
		cell_text_template = "<font size=3><font color=darkgreen>LOOP </font>( <font color=blue>%1</font> )</font>"; 
		cell_text = "";
		break;
	case Command::End_Cmd:		
		cell_text_template = "<font size=3 color=darkgreen>END_LOOP</font>"; 
		cell_text = "";
		break;
	case Command::NoP_Cmd:		
		cell_text_template = "<font size=3 color=darkgreen></font>"; 
		cell_text = "";
		break;
	default:		
		cell_text_template = "<font size=3 color=darkgreen></font>"; 
		cell_text = "";
		break;
	}
}


Scheduler::Exec::Exec(QStringList jseqs, QString jseq_file, QString _data_file)
{
	type = Scheduler::Exec_Cmd;			
	jseq_list = jseqs;
	QFileInfo jseq_file_info(jseq_file);
	jseq_name = jseq_file_info.fileName();
	if (jseq_list.isEmpty()) jseq_name = "";
	else if (!jseq_list.contains(jseq_name)) jseq_name = jseq_list.first();

	data_file = _data_file;
	cell_text_template = QString("<font size=3><font color=darkgreen>EXEC </font>( '<font color=blue>%1</font>' )</font>");
	cell_text = cell_text_template.arg(jseq_name);

	Scheduler::SettingsItem *param_item_1 = new Scheduler::SettingsItem(tr("Sequence:"), Scheduler::ComboBox, "");
	Scheduler::SettingsItem *param_item_2 = new Scheduler::SettingsItem(tr("Data File:"), Scheduler::FileBrowse, "");
	param_objects.append(param_item_1);
	param_objects.append(param_item_2);
}

Scheduler::Exec::~Exec()
{
	qDeleteAll(param_objects.begin(), param_objects.end());
	param_objects.clear();
}


Scheduler::DistanceRange::DistanceRange()
{
	type = Scheduler::DistanceRange_Cmd;		
	cell_text_template = "<font size=3><font color=darkgreen>POSITION_LOOP </font>( <font color=blue>%1</font> : <font color=blue>%2</font> : <font color=blue>%3</font> , <font color=blue>%4</font> )</font>";
	cell_text = "";
	from = 0;
	to = 0;
	step = 0;
	pos = nan;
	do_calibr = true;
	finished = false;
	is_calibration_range = false;

	Scheduler::SettingsItem *param_item_1 = new Scheduler::SettingsItem(tr("From:"), Scheduler::SpinBox, " cm");	
	param_objects.append(param_item_1);	

	Scheduler::SettingsItem *param_item_2 = new Scheduler::SettingsItem(tr("To:"), Scheduler::SpinBox, " cm");	
	param_objects.append(param_item_2);	

	Scheduler::SettingsItem *param_item_3 = new Scheduler::SettingsItem(tr("Step:"), Scheduler::SpinBox, " cm");	
	param_objects.append(param_item_3);	

	Scheduler::SettingsItem *param_item_4 = new Scheduler::SettingsItem(tr("Auto-calibration:"), Scheduler::ComboBox, "");	
	param_objects.append(param_item_4);	
}

Scheduler::DistanceRange::~DistanceRange()
{
	qDeleteAll(param_objects.begin(), param_objects.end());
	param_objects.clear();
}

void Scheduler::DistanceRange::setBounds(QPair<double,double> bounds)
{
	lower_bound = 100*bounds.first;		// convert to [cm]
	upper_bound = 100*bounds.second;	// convert to [cm]
}

void Scheduler::DistanceRange::setCalibrationLength(double _len)
{
	calibr_len = 100*_len;				// convert to [cm]
}

void Scheduler::DistanceRange::setFromToStep(QPair<double,double> from_to, double _step, bool _do_calibr)
{
	from = 100*from_to.first;			// convert to [cm]
	to = 100*from_to.second;			// convert to [cm]
	step = 100*_step;					// convert to [cm]
	do_calibr = _do_calibr;			

	QString do_calibr_str = (do_calibr ? "TRUE" : "FALSE");
	cell_text = cell_text_template.arg(from).arg(step).arg(to).arg(do_calibr_str);		// must be in [cm] !
}

void Scheduler::DistanceRange::changeFrom(double val)
{
	from = val; 
	QString do_calibr_str = (do_calibr ? "TRUE" : "FALSE");
	cell_text = cell_text_template.arg(from).arg(step).arg(to).arg(do_calibr_str);		// must be in [cm] !
	
	emit changed();
}

void Scheduler::DistanceRange::changeTo(double val)
{
	to = val; 
	QString do_calibr_str = (do_calibr ? "TRUE" : "FALSE");
	cell_text = cell_text_template.arg(from).arg(step).arg(to).arg(do_calibr_str);		// must be in [cm] !
	
	emit changed();
}

void Scheduler::DistanceRange::changeStep(double val)
{
	step = val; 
	QString do_calibr_str = (do_calibr ? "TRUE" : "FALSE");
	cell_text = cell_text_template.arg(from).arg(step).arg(to).arg(do_calibr_str);		// must be in [cm] !
	
	emit changed();
}

void Scheduler::DistanceRange::changeDoCalibr(const QString& text)
{
	if (text == "TRUE") do_calibr = true;
	else do_calibr = false;

	QString do_calibr_str = (do_calibr ? "TRUE" : "FALSE");
	cell_text = cell_text_template.arg(from).arg(step).arg(to).arg(do_calibr_str);		// must be in [cm] !

	emit changed();
}

double Scheduler::DistanceRange::getNextPos()
{
	if (pos != pos) 
	{
		if (do_calibr) pos = from - calibr_len;
		else pos = from;
	}
	else if (from < to)
	{
		if (pos <= to-step) pos += step;
		else finished = true;
	}
	else if (from > to)
	{
		if (pos >= from+step) pos -= step;
		else finished = true;
	}

	if (do_calibr)
	{
		is_calibration_range = (from-pos) > 0;		
	}

	return pos;
}


Scheduler::SetPosition::SetPosition(double _position)
{
	type = Scheduler::SetPosition_Cmd;		
	cell_text_template = "<font size=3><font color=darkgreen>SET_POSITION </font>( <font color=blue>%1</font> )</font>"; 	
	position = _position;		
	cell_text = cell_text_template.arg(position);

	Scheduler::SettingsItem *param_item_1 = new Scheduler::SettingsItem(tr("Position:"), Scheduler::DoubleSpinBox, " cm");	
	param_objects.append(param_item_1);	
}

Scheduler::SetPosition::~SetPosition()
{
	qDeleteAll(param_objects.begin(), param_objects.end());
	param_objects.clear();
}

void Scheduler::SetPosition::changePosition(double _pos)
{
	position = _pos; 
	cell_text = cell_text_template.arg(position);		// must be in [cm] !
	emit changed();
}

void Scheduler::SetPosition::setBounds(QPair<double,double> bounds)
{
	lower_bound = 100*bounds.first;		// convert to [cm]
	upper_bound = 100*bounds.second;	// convert to [cm]
}


Scheduler::Loop::Loop()
{
	type = Scheduler::Loop_Cmd;		
	index = 0;	
	counts = 1;
	lower_bound = 1;
	finished = false;

	cell_text_template = QString("<font size=3><font color=darkgreen>LOOP </font>( <font color=blue>%1</font> )</font>");
	cell_text = cell_text_template.arg(counts);

	Scheduler::SettingsItem *param_item_1 = new Scheduler::SettingsItem(tr("Counts:"), Scheduler::SpinBox, "");	
	param_objects.append(param_item_1);	
}

Scheduler::Loop::~Loop()
{
	qDeleteAll(param_objects.begin(), param_objects.end());
	param_objects.clear();
}

void Scheduler::Loop::changeCounts(int val)
{
	counts = val; 
	cell_text = cell_text_template.arg(counts);		
	emit changed();
}


Scheduler::Sleep::Sleep()
{
	type = Scheduler::Sleep_Cmd;
	delay = 10;
	upper_bound = 86400;

	cell_text_template = QString("<font size=3><font color=darkgreen>SLEEP </font>( <font color=blue>%1</font> )</font>");
	cell_text = cell_text_template.arg(delay);

	Scheduler::SettingsItem *param_item_1 = new Scheduler::SettingsItem(tr("Delay:"), Scheduler::SpinBox, " sec");	
	param_objects.append(param_item_1);	
}

Scheduler::Sleep::~Sleep()
{
	qDeleteAll(param_objects.begin(), param_objects.end());
	param_objects.clear();
}

void Scheduler::Sleep::changeDelay(int val)
{
	delay = val; 
	cell_text = cell_text_template.arg(delay);		
	emit changed();
}


Scheduler::End::End()
{
	type = Scheduler::End_Cmd;		
	cell_text_template = QString("<font size=3 color=darkgreen>END_LOOP</font>");
	cell_text = QString("<font size=3 color=darkgreen>END_LOOP</font>");;
	ref_obj = NULL;
}


Scheduler::NOP::NOP()
{
	type = Scheduler::NoP_Cmd;		
	cell_text_template = QString("<font size=3 color=darkgreen></font>");
	cell_text = QString("<font size=3 color=darkgreen></font>");
}


Scheduler::Engine::~Engine()
{
	clear();
}

void Scheduler::Engine::setPos(Scheduler::SchedulerObject* obj)
{
	if (!obj) return;

	cur_pos = -1;
	for (int i = 0; i < obj_list.count(); i++)
	{
		SchedulerObject *cur_obj = obj_list.at(i);
		if (cur_obj == obj) cur_pos = i;
	}
}

Scheduler::SchedulerObject* Scheduler::Engine::get(int index) 
{ 
	Scheduler::SchedulerObject *obj = NULL;
	if (index < obj_list.count()) 
	{
		obj = obj_list.at(index);
		if (obj->type == Scheduler::End_Cmd)
		{
			if (Scheduler::End *end_obj = qobject_cast<Scheduler::End*>(obj)) 
			{
				if (Scheduler::Loop *loop_obj = qobject_cast<Scheduler::Loop*>(end_obj->ref_obj))
				{
					if (loop_obj->finished) 
					{
						obj = get(index+1);
						cur_pos = index+1;
					}
					else 
					{
						setPos(loop_obj);
						obj = loop_obj;
						cur_pos++;
					}
				}
				else if (Scheduler::DistanceRange *dist_range_obj = qobject_cast<Scheduler::DistanceRange*>(end_obj->ref_obj))
				{
					if (dist_range_obj->finished) 
					{
						obj = get(index+1);
						cur_pos = index+1;
					}
					else 
					{
						setPos(dist_range_obj);
						obj = dist_range_obj;
						cur_pos++;
					}
				}
			}
		}		
	}
	
	return obj; 
}

Scheduler::SchedulerObject* Scheduler::Engine::next()
{
	if (cur_pos < 0) return NULL;	// Experiment hasn't started yet
	else if (cur_pos >= obj_list.count()) { cur_pos = -1; return NULL; }
	else return get(cur_pos++);
}

void Scheduler::Engine::remove(int index)
{
	Scheduler::SchedulerObject *obj = obj_list.takeAt(index);
	delete obj;	
}

void Scheduler::Engine::clear()
{
	qDeleteAll(obj_list.begin(), obj_list.end());
	obj_list.clear();
	cur_pos = -1;
}

Scheduler::SchedulerObject* Scheduler::Engine::take(int index)
{
	if (index < 0) return NULL;	
	return obj_list.takeAt(index);
}


void Scheduler::CommandController::processResult(bool flag, uint32_t _uid)
{
	if (_uid == cmd_uid || _uid == 0)
	{
		job_finished = flag;
	}
}