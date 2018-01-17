#include <QGridLayout>

#include "statusbar_widgets.h"


ConnectionWidget::ConnectionWidget(QWidget *parent)
{
	if (parent != 0) this->setParent(parent);

	this->setGeometry(QRect(309, 40, 219, 21));
	this->setFrameShape(QFrame::NoFrame);
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setContentsMargins(1, 1, 1, 1);
	lblTitle = new QLabel(this);
	lblTitle->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	lblTitle->setText(tr("Connection to NMRTool:"));
	gridLayout->addWidget(lblTitle, 0, 0, 1, 1);

	lblLed = new QLabel(this);
	lblLed->setMaximumSize(QSize(16, 16));
	lblLed->setTextFormat(Qt::AutoText);
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	lblLed->setScaledContents(true);
	lblLed->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	lblLed->setWordWrap(false);
	lblLed->setMargin(0);
	gridLayout->addWidget(lblLed, 0, 1, 1, 1);

	blink_timer.setInterval(200);

	connect(lblTitle, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));
	connect(lblLed, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));	
	connect(&blink_timer, SIGNAL(timeout()), this, SLOT(blink()));
}

void ConnectionWidget::setTitle(QString str)
{
	lblTitle->setText(str);
}

void ConnectionWidget::setReport(ConnectionState state)
{	
	switch (state)
	{	
	case State_OK:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/green_ball.png")));
		break;
	case State_No:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
		break;
	default: break;
	}
}

void ConnectionWidget::resetConnectionState()
{
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
}

void ConnectionWidget::blink()
{
	static int b = 0;
	if (b > 1) b = 0;

	if (b == 0) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	else if (b == 1) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Yellow Ball.png")));
	b++;
}


// *********** Traffic Widget for Status Bar **************
TrafficWidget::TrafficWidget(QWidget *parent)
{
	if (parent != 0) this->setParent(parent);

	this->setGeometry(QRect(309, 40, 219, 21));
	this->setFrameShape(QFrame::NoFrame);
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setContentsMargins(1, 1, 1, 1);
	lblTitle = new QLabel(this);
	lblTitle->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	lblTitle->setText(tr("Traffic:"));
	gridLayout->addWidget(lblTitle, 0, 0, 1, 1);

	lblLed = new QLabel(this);
	lblLed->setMaximumSize(QSize(16, 16));
	lblLed->setTextFormat(Qt::AutoText);
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	lblLed->setScaledContents(true);
	lblLed->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	lblLed->setWordWrap(false);
	lblLed->setMargin(0);
	gridLayout->addWidget(lblLed, 0, 1, 1, 1);

	lblReport = new QLabel(this);
	lblReport->setFrameShape(QFrame::StyledPanel);
	lblReport->setFrameStyle(QFrame::Sunken);
	lblReport->setText(tr("0% Bad Blocks"));
	lblReport->setAutoFillBackground(true);
	gridLayout->addWidget(lblReport, 0, 2, 1, 1);

	connect(lblTitle, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));
	connect(lblLed, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));
	connect(lblReport, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));	
}

void TrafficWidget::setTitle(QString str)
{
	lblTitle->setText(str);
}

void TrafficWidget::setReportText(QString str)
{
	lblReport->setText(str);
}

void TrafficWidget::setReport(double value, MsgState state)
{
	QPalette p = lblReport->palette();
	if (value < 0) 
	{
		setReportText(tr("0.0% Bad Blocks"));
		p.setColor(QPalette::Background, QColor(0xF0, 0xF0, 0xF0, 0xFF));	// light gray
		lblReport->setPalette(p);
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	}
	else if (value == 0)
	{
		setReportText(tr("0.0% Bad Blocks"));
		p.setColor(QPalette::Background, QColor(0x99, 0xFF, 0x00, 0xFF));	// light green
		lblReport->setPalette(p);		
	}
	else if (value < 25)
	{
		setReportText(tr("%1% Bad Blocks").arg(QString::number(value,'f',1)));
		p.setColor(QPalette::Background, QColor(0xFF, 0x99, 0x33, 0xFF));	// orange
		lblReport->setPalette(p);		
	}
	else if (value <= 100)
	{
		setReportText(tr("%1% Bad Blocks").arg(QString::number(value,'f',1)));
		p.setColor(QPalette::Background, QColor(Qt::red));
		lblReport->setPalette(p);		
	}

	switch (state)
	{
	case MsgOff:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
		break;
	case MsgOK:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/green_ball.png")));
		break;
	case MsgPoor:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Orange Ball.png")));
		break;
	case MsgBad:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
		break;
	default: break;
	}

	QTimer::singleShot(100, this, SLOT(resetTrafficState()));
}

void TrafficWidget::resetTrafficState()
{
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
}


// ******* TCP Communication Widget for Status Bar *******
TcpConnectionWidget::TcpConnectionWidget(QWidget *parent)
{
	if (parent != 0) this->setParent(parent);

	this->setGeometry(QRect(309, 40, 219, 21));
	this->setFrameShape(QFrame::NoFrame);
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setContentsMargins(1, 1, 1, 1);
	lblTitle = new QLabel(this);
	lblTitle->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	lblTitle->setText(tr("Connected TCP Clients:"));
	gridLayout->addWidget(lblTitle, 0, 0, 1, 1);

	lblLed = new QLabel(this);
	lblLed->setMaximumSize(QSize(16, 16));
	lblLed->setTextFormat(Qt::AutoText);
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	lblLed->setScaledContents(true);
	lblLed->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	lblLed->setWordWrap(false);
	lblLed->setMargin(0);
	gridLayout->addWidget(lblLed, 0, 1, 1, 1);

	blink_timer.setInterval(200);

	connect(lblTitle, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));
	connect(lblLed, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));	
	connect(&blink_timer, SIGNAL(timeout()), this, SLOT(blink()));
}

void TcpConnectionWidget::setTitle(QString str)
{
	lblTitle->setText(str);
}

void TcpConnectionWidget::setReport(ConnectionState state)
{	
	switch (state)
	{	
	case State_OK:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/green_ball.png")));
		break;
	case State_No:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
		break;
	default: break;
	}
}

void TcpConnectionWidget::resetConnectionState()
{
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
}

void TcpConnectionWidget::blink()
{
	static int b = 0;
	if (b > 1) b = 0;

	if (b == 0) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	else if (b == 1) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Yellow Ball.png")));
	b++;
}


// ******* CDiag Communication Widget for Status Bar *******
CDiagConnectionWidget::CDiagConnectionWidget(QWidget *parent)
{
	if (parent != 0) this->setParent(parent);

	this->setGeometry(QRect(309, 40, 219, 21));
	this->setFrameShape(QFrame::NoFrame);
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setContentsMargins(1, 1, 1, 1);
	lblTitle = new QLabel(this);
	lblTitle->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	lblTitle->setText(tr("Connected CDiag Clients:"));
	gridLayout->addWidget(lblTitle, 0, 0, 1, 1);

	lblLed = new QLabel(this);
	lblLed->setMaximumSize(QSize(16, 16));
	lblLed->setTextFormat(Qt::AutoText);
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	lblLed->setScaledContents(true);
	lblLed->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	lblLed->setWordWrap(false);
	lblLed->setMargin(0);
	gridLayout->addWidget(lblLed, 0, 1, 1, 1);

	blink_timer.setInterval(200);

	connect(lblTitle, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));
	connect(lblLed, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));	
	connect(&blink_timer, SIGNAL(timeout()), this, SLOT(blink()));
}

void CDiagConnectionWidget::setTitle(QString str)
{
	lblTitle->setText(str);
}

void CDiagConnectionWidget::setReport(ConnectionState state)
{	
	switch (state)
	{	
	case State_OK:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/green_ball.png")));
		break;
	case State_No:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
		break;
	default: break;
	}
}

void CDiagConnectionWidget::resetConnectionState()
{
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
}

void CDiagConnectionWidget::blink()
{
	static int b = 0;
	if (b > 1) b = 0;

	if (b == 0) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	else if (b == 1) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Yellow Ball.png")));
	b++;
}


// ******* Impuls Communication Widget for Status Bar *******
ImpulsConnectionWidget::ImpulsConnectionWidget(QWidget *parent)
{
	if (parent != 0) this->setParent(parent);

	this->setGeometry(QRect(309, 40, 219, 21));
	this->setFrameShape(QFrame::NoFrame);
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setContentsMargins(1, 1, 1, 1);
	lblTitle = new QLabel(this);
	lblTitle->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	lblTitle->setText(tr("Connection to DepthMeter:"));
	gridLayout->addWidget(lblTitle, 0, 0, 1, 1);

	lblLed = new QLabel(this);
	lblLed->setMaximumSize(QSize(16, 16));
	lblLed->setTextFormat(Qt::AutoText);
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
	lblLed->setScaledContents(true);
	lblLed->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
	lblLed->setWordWrap(false);
	lblLed->setMargin(0);
	gridLayout->addWidget(lblLed, 0, 1, 1, 1);

	blink_timer.setInterval(200);

	connect(lblTitle, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));
	connect(lblLed, SIGNAL(linkActivated(const QString&)), this, SIGNAL(clicked()));	
	connect(&blink_timer, SIGNAL(timeout()), this, SLOT(blink()));
}

ImpulsConnectionWidget::~ImpulsConnectionWidget()
{
	//delete lblLed;
	//delete lblTitle;
	
}

void ImpulsConnectionWidget::setTitle(QString str)
{
	lblTitle->setText(str);
}

void ImpulsConnectionWidget::setReport(ConnectionState state)
{	
	switch (state)
	{	
	case State_OK:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/green_ball.png")));
		break;
	case State_No:
		lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Red Ball.png")));
		break;
	default: break;
	}
}

void ImpulsConnectionWidget::resetConnectionState()
{
	lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
}

void ImpulsConnectionWidget::blink()
{
	static int b = 0;
	if (b > 1) b = 0;

	if (b == 0) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Grey Ball.png")));
	else if (b == 1) lblLed->setPixmap(QPixmap(QString::fromUtf8(":/images/Yellow Ball.png")));
	b++;
}