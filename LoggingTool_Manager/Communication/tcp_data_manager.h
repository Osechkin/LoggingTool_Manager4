#ifndef TCP_DATA_MANAGER_H
#define TCP_DATA_MANAGER_H

#include <QtCore>

#include "threads.h"
#include "../Common/data_containers.h"
#include "../Common/statusbar_widgets.h"


class TcpDataManager : public QObject
{
	Q_OBJECT

public:
	TcpDataManager(QMutex *ds_mutex, QObject *parent = 0);
	~TcpDataManager();

	void dataToSend(const QString &sock_id, QString &out_str);
	void dataToSend(const QString &sock_id, DataSets *dss);

	TcpConnectionWidget *getTcpConnectionWidget() const { return tcp_widget; }
	TcpCommunicator *getTcpCommunicator() { return tcp_communicator; }
	
	void resetTcpCommunicator(int port);

private:
	TcpCommunicator *tcp_communicator;

	TcpConnectionWidget *tcp_widget;
	ConnectionState connection_state;

	void lockDataSet() { dataset_mutex->lock(); }
	void unlockDataSet() { dataset_mutex->unlock(); }
	QMutex *dataset_mutex;
	
private slots:
	void reportClients(int count);
	void getData(const QString& str, int index);

signals:
	void get_data(const QString&, int);
	void data_to_send(const QString&, QString&);
	void data_to_send(const QString&, DataSets*);
	void send_to_sdsp(QByteArray&);
};

#endif // TCP_DATA_MANAGER_H