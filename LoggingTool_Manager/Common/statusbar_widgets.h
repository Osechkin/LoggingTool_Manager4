#ifndef STATUSBAR_WIDGETS_H
#define STATUSBAR_WIDGETS_H

#include <QFrame>
#include <QLabel>
#include <QTimer>

#include "../io_general.h"


class TrafficWidget : public QFrame
{
	Q_OBJECT

public:
	explicit TrafficWidget(QWidget *parent = 0);
	~TrafficWidget() { };
		
	void setTitle(QString str);
	void setReportText(QString str);
	void setReport(double value, MsgState state);		// value - процент ошибок (процент сбойных блоков)

private:
	QLabel *lblLed;
	QLabel *lblTitle;
	QLabel *lblReport;	

private slots:
	void resetTrafficState();

signals:
	void clicked();
};


// виджет для StatusBar о состоянии связи по кабелю
class ConnectionWidget : public QFrame
{
	Q_OBJECT

public:
	explicit ConnectionWidget(QWidget *parent = 0);
	~ConnectionWidget() { };

	void setTitle(QString str);	
	void setReport(ConnectionState state);	
	void startBlinking() { blink_timer.start(); }
	void stopBlinking() { blink_timer.stop(); }

private:
	QLabel *lblLed;
	QLabel *lblTitle;	

	QTimer blink_timer;

private slots:
	void resetConnectionState();
	void blink();

signals:
	void clicked();	
};


// виджет для StatusBar о состоянии связи с TCP клиентами
class TcpConnectionWidget : public QFrame
{
	Q_OBJECT

public:
	explicit TcpConnectionWidget(QWidget *parent = 0);
	~TcpConnectionWidget() { };

	void setTitle(QString str);	
	void setReport(ConnectionState state);
	void startBlinking() { blink_timer.start(); }
	void stopBlinking() { blink_timer.stop(); }

private:
	QLabel *lblLed;
	QLabel *lblTitle;	

	QTimer blink_timer;

private slots:
	void resetConnectionState();
	void blink();

signals:
	void clicked();	

};


// виджет для StatusBar о состоянии связи с CDiag клиентами
class CDiagConnectionWidget : public QFrame
{
	Q_OBJECT

public:
	explicit CDiagConnectionWidget(QWidget *parent = 0);
	~CDiagConnectionWidget() { };

	void setTitle(QString str);	
	void setReport(ConnectionState state);
	void startBlinking() { blink_timer.start(); }
	void stopBlinking() { blink_timer.stop(); }

private:
	QLabel *lblLed;
	QLabel *lblTitle;	

	QTimer blink_timer;

	private slots:
		void resetConnectionState();
		void blink();

signals:
		void clicked();	

};


// виджет для StatusBar о состоянии связи с измерителем глубины Импульс
class ImpulsConnectionWidget : public QFrame
{
	Q_OBJECT

public:
	explicit ImpulsConnectionWidget(QWidget *parent = 0);
	~ImpulsConnectionWidget();

	void setTitle(QString str);	
	void setReport(ConnectionState state);
	void startBlinking() { blink_timer.start(); }
	void stopBlinking() { blink_timer.stop(); }

private:
	QLabel *lblLed;
	QLabel *lblTitle;	

	QTimer blink_timer;

private slots:
	void resetConnectionState();
	void blink();

signals:
	void clicked();	
};


#endif // STATUSBAR_WIDGETS