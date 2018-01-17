#ifndef CDIAG_DATA_MANAGER_H
#define CDIAG_DATA_MANAGER_H

#include <QtCore>

#include "threads.h"
#include "../Common/data_containers.h"
#include "../Common/statusbar_widgets.h"



class CDiagDataManager : public QObject
{
	Q_OBJECT

public:
	CDiagDataManager(Clocker *clocker, QObject *parent = 0);
	~CDiagDataManager();

	void dataToSend(const QString &sock_id, QString &out_str);
	void dataToSend(QVector<uint8_t> *vec);

	CDiagConnectionWidget *getTcpConnectionWidget() const { return cdiag_widget; }
	CDiagCommunicator *getCDiagCommunicator() { return cdiag_communicator; }

	void resetCDiagCommunicator(int port);

private:
	CDiagCommunicator *cdiag_communicator;

	CDiagConnectionWidget *cdiag_widget;
	ConnectionState connection_state;

	Clocker *clocker;
		
private slots:
	void reportClients(int count);
	void getData(const QString& str, int index);

signals:
	void get_data(const QString&, int);
	void data_to_send(const QString&, QString&);
	void data_to_send(QVector<uint8_t>*);
	void send_to_sdsp(QByteArray&);
};

#endif // CDIAG_DATA_MANAGER_H
