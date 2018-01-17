#ifndef TOOLS_DIALOG_H
#define TOOLS_DIALOG_H

#include <QtGui>

#include "../tools_general.h"

#include "ui_tools_dialog.h"


class ToolsDialog : public QDialog, public Ui::LoggingToolDialog
{
	Q_OBJECT

public:
	explicit ToolsDialog(QList<ToolInfo> tools_list, int cur_index = 0, QWidget *parent = 0);
	
	void setCurrentTool(QString file_name);
	void setMessage(QString txt) { label->setText(txt); }
	void blockToolSelection(bool flag) { cboxTools->setEnabled(!flag); }

	QString getSelectedTool() { return current_tool; }
	QString getSelectedToolFile() { return current_tool_file; }
	int getSelectedToolId() { return current_tool_id; }
	QList<ToolInfo> &getToolsInfo() { return tools; }

	QStringList getTabWidgets() { return current_tool_tab_widgets; }
	QStringList getSeqWizards() { return current_tool_seq_wizards; }
	QStringList getDepthMonitors() { return current_tool_depth_monitors; }
	QStringList getDockWidgets() { return current_tool_dock_widgets; }
	QString getInfoBarText() { return current_tool_info_bar; } 

private slots:
	void setActiveTool(const QString &text);
	
private:
	QString current_tool;
	QString current_tool_file;
	int current_tool_id;

	QStringList current_tool_tab_widgets;
	QStringList current_tool_seq_wizards;
	QStringList current_tool_dock_widgets;
	QStringList current_tool_depth_monitors;
	QString current_tool_info_bar;	

	QList<ToolInfo> tools;
};

#endif // TOOLS_DIALOG_H