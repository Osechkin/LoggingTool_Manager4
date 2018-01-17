#include "tcp_data_manager.h"


TcpDataManager::TcpDataManager(QMutex *ds_mutex, QObject *parent)
{
	//setParent(parent);

	dataset_mutex = ds_mutex;

	connection_state = State_No;
	tcp_widget = new TcpConnectionWidget;
	tcp_widget->setTitle(tr("TCP Clients (%1):").arg(0));
	tcp_widget->setReport(connection_state);
		
	tcp_communicator = new TcpCommunicator(3002, dataset_mutex);	

	connect(tcp_communicator, SIGNAL(auth_clients(int)), this, SLOT(reportClients(int)));
	//connect(tcp_communicator, SIGNAL(get_data(const QString&, int)), this, SIGNAL(get_data(const QString&, int)));
	connect(tcp_communicator, SIGNAL(get_data(const QString&, int)), this, SLOT(getData(const QString&, int)));
	connect(tcp_communicator, SIGNAL(send_to_sdsp(QByteArray&)), this, SIGNAL(send_to_sdsp(QByteArray&)));
	connect(this, SIGNAL(data_to_send(const QString&, QString&)), tcp_communicator, SLOT(sendDataToClient(const QString&, QString&)));
	connect(this, SIGNAL(data_to_send(const QString&, DataSets*)), tcp_communicator, SLOT(sendDataToClient(const QString&, DataSets*)));

	tcp_communicator->start(QThread::HighPriority);
}

TcpDataManager::~TcpDataManager()
{
	delete tcp_widget;

	tcp_communicator->exit();
	tcp_communicator->wait();
	delete tcp_communicator;
}

void TcpDataManager::resetTcpCommunicator(int port)
{
	tcp_widget->setTitle(tr("TCP Clients (%1):").arg(0));
	tcp_widget->setReport(connection_state);

	tcp_communicator->exit();
	tcp_communicator->wait();
	delete tcp_communicator;

	tcp_communicator = new TcpCommunicator(port, dataset_mutex);
	tcp_communicator->start(QThread::HighPriority);	
}

void TcpDataManager::reportClients(int count)
{
	QString text = QString(tr("TCP Clients")) + QString("(%1):").arg(count);
	tcp_widget->setTitle(text);
	if (count > 0) connection_state = State_OK;
	else connection_state = State_No;
	tcp_widget->setReport(connection_state);
}

void TcpDataManager::dataToSend(const QString &sock_id, QString &out_str)
{			
	emit data_to_send(sock_id, out_str);	
}

void TcpDataManager::dataToSend(const QString &sock_id, DataSets *dss)
{	
	qDebug() << "tcpDtaManager: emit data_to_send(...)";
	emit data_to_send(sock_id, dss);	
}

void TcpDataManager::getData(const QString& str, int index)
{	
	qDebug() << "TcpDataManager: emit get_gata(...)";
	emit get_data(str, index);
}