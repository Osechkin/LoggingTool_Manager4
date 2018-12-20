#ifndef TOOLS_GENERAL_H
#define TOOLS_GENERAL_H

#include <QStringList>
#include <QTcpSocket>

#include "qextserialport.h"


typedef enum { 
	Depth,							// ������� 
	Distance,						// ���������� (��� �������� � ��������� �����)
	Time,							// �����
	Temperature,					// �����������
	Concentration					// ������������
} ScannedQuantity;	



struct ToolInfo
{	
	ToolInfo(int _id = 0, QString _type = "", QString _file_name = "")
	{
		id = _id;
		type = _type;
		file_name = _file_name;
		scanned_quantity = ScannedQuantity::Depth;
	}

	int id;
	QString type;
	QString file_name;
	QStringList tab_widgets;					// ������ ��������� ������� � ������� ������� ���������� (������ ������: "Logging", "DataPreview", "Oscilloscope", "Monitoring", "SDSP")
	QStringList seq_wizards;					// ������ �������� ��� ����� ������������������ � ���������� ������������� (������ ������: "SequenceWizard", "ExperimentScheduler")
	QStringList dock_widgets;					// ������ ������������ "���������" �������� (������ ������: "LoggingToolConsole", "CommunicationLogMonitor", "FrequencyAutoTune", "RxTxControl", "RFPulseControl")
	QStringList depth_monitors;					// ������ ������������ � ����������� (������ ������: "DepthEmulator", "Impulse-Ustye", "InternalDepthMeter", "LeuzePositionMeter")
	QString info_bar;							// ������������ ������� � InfoBar ("KMRK", "NMKT", "KERN")	
	ScannedQuantity scanned_quantity;			// ����������� �������� ��������: ������� (��� ���������� ��������), ������� ����� (��� �������� ���-����), �����, �����������, ������������ � �.�.
};


//#define NMR_SAMPLE_FREQ			(4*250000)		// ������� ������������� ��� ������� ���
//#define NMR_SAMPLE_FREQ_HALF	(NMR_SAMPLE_FREQ/2)	// �������� ������� ������������� ��� ������� ���


#define DU_DATA_LEN				45				// ����� ������ ���������� (� ������), ���������� �� ���������� DU

#define DIELECTR_MSG_LEN		20				// ����� ������ ������ ���������������� �������


struct COM_PORT
{
	PortSettings COM_Settings;
	QextSerialPort *COM_port;
	bool connect_state;				// ������/������
	bool auto_search;				// ���������� �� ������������ ��������� ���������� ������� ��� �� COM-������
};


struct Communication_Settings
{
	int packet_length;				// ����� �������
	int block_length;				// ����� ������
	int errs_count;					// ���������� �������������� ������ � ������ ������
	int packet_delay;				// �������� ����� �������� (��)
	bool antinoise_coding;			// ���������/�� ��������� ���������������� �����������
	bool packlen_autoadjust;		// ����������� ����� ������� (���./����.)
	bool noise_control;				// ���������� ���������� ���������� ��������� ����������������� ����������� � ����������� �� ��������� ���������� (���./����.)
	bool interleaving;				// ����������/�� ���������� ��������� "������������" (������������� ������ � �������) 
};


struct TCP_Settings
{
	QTcpSocket *socket;
	QString ip_address;
	int port;
};


#endif // IO_GENERAL_H
