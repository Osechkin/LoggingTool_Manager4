#include "../Common/app_settings.h"
#include "tools_dialog.h"


/*ToolsDialog::ToolsDialog(QStringList &tools_list, QString msg, QWidget *parent)
{
	setupUi(this); 		
	label->setText(msg);

	current_tool_id = 0;
	current_tool_file = "";
	current_tool = "";

	QStringList tools_files = tools_list;
	for (int i = 0; i < tools_files.count(); i++)
	{
		QString tool_file = tools_files[i];
		QSettings *settings = new QSettings(tool_file, QSettings::IniFormat);
		
		int uid = 0; 
		QString tool_type = "";
		bool ok = false;
		if (settings->contains("Tool/uid")) uid = settings->value("Tool/uid").toInt(&ok); 
		else uid = 0;
		if (settings->contains("Tool/type")) tool_type = settings->value("Tool/type").toString(); 
		
		if (ok && uid > 0 && !tool_type.isEmpty()) 
		{
			bool tool_exist = false;
			for (int j = 0; j < tools.count(); j++)
			{
				ToolInfo tool_info = tools[j];
				if (tool_info.id == uid) tool_exist = true;
			}

			if (!tool_exist)
			{
				ToolInfo tool_info(uid, tool_type, tool_file); 
				tools.append(tool_info);
			}
		}
		
		delete settings;
	}

	QStringList tools_types;
	for (int i = 0; i < tools.count(); i++)
	{
		tools_types.append(tools[i].type);
	}

	cboxTools->addItems(tools_types);
	current_tool = cboxTools->currentText();
	for (int i = 0; i < tools.count(); i++)
	{
		ToolInfo tool_info = tools[i];
		if (tool_info.type == current_tool)
		{
			current_tool_file = tool_info.file_name;
			current_tool_id = tool_info.id;
		}
	}
	
	connect(cboxTools, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setActiveTool(const QString&)));
}*/

ToolsDialog::ToolsDialog(QList<ToolInfo> tools_list, int cur_index, QWidget *parent)
{
	setupUi(this);

	
	tools = tools_list;

	//current_tool_id = tools[cur_index].id;
	current_tool_file = tools[cur_index].file_name;
	current_tool = tools[cur_index].type;
	current_tool_tab_widgets = tools[cur_index].tab_widgets;
	current_tool_depth_monitors = tools[cur_index].depth_monitors;
	current_tool_seq_wizards = tools[cur_index].seq_wizards;
	current_tool_info_bar = tools[cur_index].info_bar;

	QStringList tools_types;
	for (int i = 0; i < tools.count(); i++)
	{
		tools_types.append(tools[i].type);
	}
	if (!tools_types.isEmpty() && cur_index < tools_types.count())
	{
		cboxTools->addItems(tools_types);
		cboxTools->setCurrentIndex(cur_index);
	}	

	connect(cboxTools, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setActiveTool(const QString&)));	
}

void ToolsDialog::setActiveTool(const QString &text)
{	
	for (int i = 0; i < tools.count(); i++)
	{
		ToolInfo tool_info = tools[i];
		if (tool_info.type == text)
		{
			current_tool = text;
			current_tool_file = tool_info.file_name;
			current_tool_id = tool_info.id;

			current_tool_tab_widgets = tool_info.tab_widgets;
			current_tool_depth_monitors = tool_info.depth_monitors;
			current_tool_seq_wizards = tool_info.seq_wizards;
			current_tool_info_bar = tool_info.info_bar;
		}
	}
}

void ToolsDialog::setCurrentTool(QString file_name)
{
	bool is_existed = false;
	QString type = "";
	int id = 0;
	QStringList tab_widgets;
	QStringList seq_wizards;
	QStringList depth_monitors;
	QString info_bar;

	for (int i = 0; i < tools.count(); i++)
	{
		ToolInfo tool_info = tools[i];
		if (tool_info.file_name == file_name) 
		{
			is_existed = true; 
			type = tool_info.type;
			id = tool_info.id;

			tab_widgets = tool_info.tab_widgets;
			seq_wizards = tool_info.seq_wizards;
			depth_monitors = tool_info.depth_monitors;
			info_bar = tool_info.info_bar;
		}
	}

	if (is_existed)
	{
		cboxTools->setCurrentText(type);
		current_tool_id = id;
		current_tool = type;
		current_tool_file = file_name;

		current_tool_tab_widgets = tab_widgets;
		current_tool_seq_wizards = seq_wizards;
		current_tool_depth_monitors = depth_monitors;
		current_tool_info_bar = info_bar;
	}
}