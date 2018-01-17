#include "message_log_monitor.h"

MessageLogMonitor::MessageLogMonitor(QWidget *parent) : QWidget(parent), ui(new Ui::MessageLogMonitor)
{
	ui->setupUi(this);

	ui->tedLog->setLineWrapMode(QTextEdit::WidgetWidth); //te->setWordWrapMode(QTextOption::WordWrap);
	QPalette p = ui->tedLog->palette();
	p.setColor(QPalette::Base, QColor(Qt::white));
	ui->tedLog->setPalette(p);	

	connect(ui->tbtLogOnOff, SIGNAL(toggled(bool)), this, SLOT(setLogVizState(bool)));
	connect(ui->tbtClearAll, SIGNAL(clicked()), ui->tedLog, SLOT(clear()));

	log_viz_state = true;
	ui->tbtClearAll->setIcon(QIcon(":/images/Eraser.png"));	
	ui->tbtLogOnOff->setIcon(QIcon(":/images/Red Ball.png"));
	ui->tbtLogOnOff->setAutoRaise(true);
	ui->tbtLogOnOff->setCheckable(true);
	ui->tbtLogOnOff->setChecked(log_viz_state);
}

MessageLogMonitor::~MessageLogMonitor()
{
	delete ui;
}

void MessageLogMonitor::addText(QString text)
{
	if (log_viz_state) ui->tedLog->append(text);
}

void MessageLogMonitor::setLogVizState(bool flag)
{
	log_viz_state = flag;	
	if (!log_viz_state) ui->tbtLogOnOff->setIcon(QIcon(":/images/Red Ball.png"));
	else ui->tbtLogOnOff->setIcon(QIcon(":/images/green_ball.png"));
}