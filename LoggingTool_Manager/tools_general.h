#ifndef TOOLS_GENERAL_H
#define TOOLS_GENERAL_H

#include <QStringList>
#include <QTcpSocket>

#include "qextserialport.h"


typedef enum { 
	Depth,							// глубина 
	Distance,						// расстояние (для приборов с протяжной керна)
	Time,							// время
	Temperature,					// температура
	Concentration					// концентрация
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
	QStringList tab_widgets;					// список доступных вкладок в главном виджете приложения (полный список: "Logging", "DataPreview", "Oscilloscope", "Monitoring", "SDSP")
	QStringList seq_wizards;					// список виждетов для ввода последовательности и управления экспериментом (полный список: "SequenceWizard", "ExperimentScheduler")
	QStringList dock_widgets;					// список отображаемых "плавающих" виджетов (полный список: "LoggingToolConsole", "CommunicationLogMonitor", "FrequencyAutoTune", "RxTxControl", "RFPulseControl")
	QStringList depth_monitors;					// список глубиномеров и дальномеров (полный список: "DepthEmulator", "Impulse-Ustye", "InternalDepthMeter", "LeuzePositionMeter")
	QString info_bar;							// самоназвание прибора в InfoBar ("KMRK", "NMKT", "KERN")	
	ScannedQuantity scanned_quantity;			// сканируемая прибором величина: глубина (для каротажных приборов), позиция керна (для приборов ЯМР-Керн), время, температура, концентрация и т.д.
};


//#define NMR_SAMPLE_FREQ			(4*250000)		// частота дискретизации АЦП прибора ЯМР
//#define NMR_SAMPLE_FREQ_HALF	(NMR_SAMPLE_FREQ/2)	// половина частоты дискретизации АЦП прибота ЯМР


#define DU_DATA_LEN				45				// длина данных телеметрии (в байтах), получаемых из устройства DU

#define DIELECTR_MSG_LEN		20				// длина данных ответа диэлектрического прибора


struct COM_PORT
{
	PortSettings COM_Settings;
	QextSerialPort *COM_port;
	bool connect_state;				// открыт/закрыт
	bool auto_search;				// необходимо ли использовать процедуру автопоиска прибора ЯМР по COM-портам
};


struct Communication_Settings
{
	int packet_length;				// длина пакетов
	int block_length;				// длина блоков
	int errs_count;					// количество корректируемых ошибок в блоках данных
	int packet_delay;				// задержка между пакетами (мс)
	bool antinoise_coding;			// применять/не применять помехоустойчивое кодирование
	bool packlen_autoadjust;		// оптимизация длины пакетов (вкл./выкл.)
	bool noise_control;				// адаптивная подстройка параметров алгоритма помехоустойчивого кодирования в зависимости от помеховой обстановки (вкл./выкл.)
	bool interleaving;				// применение/не применение алгоритма "интерливинга" (перемешивания байтов в пакетах) 
};


struct TCP_Settings
{
	QTcpSocket *socket;
	QString ip_address;
	int port;
};


#endif // IO_GENERAL_H
