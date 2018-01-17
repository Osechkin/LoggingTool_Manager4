#ifndef MESSAGE_LOG_MONITOR_H
#define MESSAGE_LOG_MONITOR_H

#include <QTextEdit>

#include "ui_message_log_monitor.h"


class MessageLogMonitor : public QWidget, public Ui::MessageLogMonitor
{
	Q_OBJECT

public:
	explicit MessageLogMonitor(QWidget *parent = 0);
	~MessageLogMonitor();

	QTextEdit *getLogWidget() { return ui->tedLog; }
	bool getLogVizState() { return log_viz_state; }

	Ui::MessageLogMonitor *getUI() { return ui; }

private:
	Ui::MessageLogMonitor *ui;
	bool log_viz_state;

public slots:
	void addText(QString text);	
	void setLogVizState(bool flag);
};

#endif // MESSAGE_LOG_MONITOR_H