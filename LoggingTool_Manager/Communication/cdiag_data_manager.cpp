#include "cdiag_data_manager.h"


CDiagDataManager::CDiagDataManager(Clocker *clocker, QObject *parent)
{	
	this->clocker = clocker;

	connection_state = State_No;
	cdiag_widget = new CDiagConnectionWidget;
	cdiag_widget->setTitle(tr("CDiag Clients (%1):").arg(0));
	cdiag_widget->setReport(connection_state);

	cdiag_communicator = new CDiagCommunicator(3004);	

	connect(clocker, SIGNAL(clock()), cdiag_communicator, SLOT(calcClocks()));
	connect(cdiag_communicator, SIGNAL(auth_clients(int)), this, SLOT(reportClients(int)));
	connect(cdiag_communicator, SIGNAL(get_data(const QString&, int)), this, SLOT(getData(const QString&, int)));
	connect(cdiag_communicator, SIGNAL(send_to_sdsp(QByteArray&)), this, SIGNAL(send_to_sdsp(QByteArray&)));
	connect(this, SIGNAL(data_to_send(const QString&, QString&)), cdiag_communicator, SLOT(sendDataToClient(const QString&, QString&)));
	connect(this, SIGNAL(data_to_send(QVector<uint8_t>*)), cdiag_communicator, SLOT(sendDataToClient(QVector<uint8_t>*)));

	cdiag_communicator->start(QThread::HighPriority);
}

CDiagDataManager::~CDiagDataManager()
{
	delete cdiag_widget;

	cdiag_communicator->exit();
	cdiag_communicator->wait();
	delete cdiag_communicator;
}

void CDiagDataManager::resetCDiagCommunicator(int port)
{
	cdiag_widget->setTitle(tr("CDiag Clients (%1):").arg(0));
	cdiag_widget->setReport(connection_state);

	cdiag_communicator->exit();
	cdiag_communicator->wait();
	delete cdiag_communicator;

	cdiag_communicator = new CDiagCommunicator(port);
	cdiag_communicator->start(QThread::HighPriority);	
}

void CDiagDataManager::reportClients(int count)
{
	QString text = QString(tr("CDiag Clients")) + QString("(%1):").arg(count);
	cdiag_widget->setTitle(text);
	if (count > 0) connection_state = State_OK;
	else connection_state = State_No;
	cdiag_widget->setReport(connection_state);
}

void CDiagDataManager::dataToSend(const QString &sock_id, QString &out_str)
{			
	emit data_to_send(sock_id, out_str);	
}

void CDiagDataManager::dataToSend(QVector<uint8_t> *vec)
{	
	QString str = "";
	for (int i = 0; i < vec->size(); i++) str += QString("%1 ").arg(vec->at(i));
	qDebug() << "CDiagDataManager: emit data_to_send(...): " + str;

	emit data_to_send(vec);	
}

void CDiagDataManager::getData(const QString& str, int index)
{	
	qDebug() << "CDiagDataManager: emit get_gata(...)";
	emit get_data(str, index);
}