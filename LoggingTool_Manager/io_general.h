#ifndef IO_GENERAL_H
#define IO_GENERAL_H

//#include "stdint.h"

#include <QString>
#include <QVector>


typedef signed __int8     int8_t;
typedef signed __int16    int16_t;
typedef signed __int32    int32_t;
typedef signed __int64	  int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;


static uint32_t session_uid = 0;


#define PowerLow			0x0
#define PowerOK				0x1
#define TempHigh			0x0
#define TempOK				0x1


#define NAN					0xFFFFFFFF
#define PlusInf				0x7FFFFFFF
#define MinusInf			0xFFFFFFFE
#define QNAN				std::numeric_limits<double>::quiet_NaN()


// I/O buffer where data is collected
#define IO_BUFF_SIZE        0x4000


// Maximum size of COM-message queue (maximum number of COM-messages in queue awaiting to send to NMR Tool)  
#define MAX_COM_MSGS		0x2


//------------------------ Device Addresses ------------------------------
#define PC_MAIN             0x01        // User's PC
#define NMR_TOOL            0x02        // NMR logging tool
#define COMM_TOOL           0x03        // Communication board
#define SDSP_TOOL			0xE0		// dielectric device (SDSP)
#define GAMMA_TOOL			0x05		// Gamma tool
#define TELEMETRY_DU_TOOL	0x06		// Telemetric board "DU"
#define DUMMY_TOOL			0xC8		// ��������� ����������


#define DT_NS_FID_ORG		0x5A		// �������� ������ ���� ��� (uint16_t) (2^19 of Galois)
#define DT_NS_SE_ORG		0x4B		// �������� ������ ���� �� (uint16_t) (2^20 of Galois)
#define DT_SGN_FID_ORG		0x75		// �������� ������ ��� (uint16_t) (2^21 of Galois)
#define DT_SGN_SE_ORG		0xEA		// �������� ������ �� (uint16_t) (2^22 of Galois)
#define DT_NS_FID 			0xC9		// �������� ������ ���� ��� (float) (2^23 of Galois)
#define DT_NS_SE			0x8F		// �������� ������ ���� �� (float) (2^24 of Galois)
#define DT_SGN_FID			0x03		// �������� ������ ��� (float) (2^25 of Galois)
#define DT_SGN_SE			0x06		// �������� ������ �� (float) (2^26 of Galois)
#define DT_NS_QUAD_FID_RE	0x0C		// ����������� ��������������� �������� ������ ���� ��� - �������� ����� (float) (2^27 of Galois)
#define DT_NS_QUAD_FID_IM	0x18		// ����������� ��������������� �������� ������ ���� ��� - ������ ����� (float) (2^28 of Galois)
#define DT_NS_QUAD_SE_RE	0x30		// ����������� ��������������� �������� ������ ���� �� - �������� ����� (float) (2^29 of Galois)
#define DT_NS_QUAD_SE_IM	0x60		// ����������� ��������������� �������� ������ ���� �� - ������ ����� (float) (2^30 of Galois)
#define DT_SGN_QUAD_FID_RE	0xC0		// ����������� ��������������� �������� ������ ��� - �������� ����� (float) (2^31 of Galois)
#define DT_SGN_QUAD_FID_IM	0x9D		// ����������� ��������������� �������� ������ ��� - ������ ����� (float) (2^32 of Galois)
#define DT_SGN_QUAD_SE_RE	0x27		// ����������� ��������������� �������� ������ �� - �������� ����� (float) (2^33 of Galois)
#define DT_SGN_QUAD_SE_IM	0x4E		// ����������� ��������������� �������� ������ �� - ������ ����� (float) (2^34 of Galois)
#define DT_NS_QUAD_FID		0x9C		// ����������� ��������������� �������� ������ ���� ��� (float) (2^35 of Galois)
#define DT_NS_QUAD_SE		0x25		// ����������� ��������������� �������� ������ ���� �� (float) (2^36 of Galois)
#define DT_SGN_QUAD_FID		0x4A		// ����������� ��������������� �������� ������ ��� (float) (2^37 of Galois)
#define DT_SGN_QUAD_SE		0x94		// ����������� ��������������� �������� ������ �� (2^38 of Galois)
#define DT_NS_FFT_FID		0xD4		// ������ ���� ��� ��������� ��� (float) (2^41 of Galois)
#define DT_NS_FFT_SE		0xB5		// ������ ���� ��� ��������� �� (float) (2^42 of Galois)
#define DT_SGN_FFT_FID		0x77		// ������ ������� ��� ��������� ��� (float) (2^43 of Galois)
#define DT_SGN_FFT_SE		0xEE		// ������ ������� ��� ��������� �� (float) (2^44 of Galois)
#define DT_NS_FFT_FID_RE	0xC1		// ������ ���� ��� ��������� ��� - �������� ����� (float) (2^45 of Galois)
#define DT_NS_FFT_SE_RE		0x9F		// ������ ���� ��� ��������� �� - �������� ����� (float) (2^46 of Galois)
#define DT_SGN_FFT_FID_RE	0x23		// ������ ������� ��� ��������� ��� - �������� ����� (float) (2^47 of Galois)
#define DT_SGN_FFT_SE_RE	0x46		// ������ ������� ��� ��������� �� - �������� ����� (float) (2^48 of Galois)
#define DT_NS_FFT_FID_IM	0x8C		// ������ ���� ��� ��������� ��� - ������ ����� (float) (2^49 of Galois)
#define DT_NS_FFT_SE_IM		0x05		// ������ ���� ��� ��������� �� - ������ ����� (float) (2^50 of Galois)
#define DT_SGN_FFT_FID_IM	0x0A		// ������ ������� ��� ��������� ��� - ������ ����� (float) (2^51 of Galois)
#define DT_SGN_FFT_SE_IM	0x14		// ������ ������� ��� ��������� �� - ������ ����� (float) (2^52 of Galois)
#define DT_SGN_RELAX		0x28		// ������ �� ����� ������� ��������������� (float) (2^53 of Galois)
#define DT_SGN_POWER_SE		0x50		// ������ �� ������� �������� ������� �� (float) (2^54 of Galois)
#define DT_SGN_POWER_FID	0xA0		// ������ �� ������� �������� ������� ��� (float) (2^55 of Galois)
#define DT_NS_POWER_SE		0x5D		// ������ �� ������� �������� ���� �� (float) (2^56 of Galois)
#define DT_NS_POWER_FID		0xBA		// ������ �� ������� �������� ���� ��� (float) (2^57 of Galois)
#define DT_DIEL				0x69		// ������ ���������������� �������� (float) (2^58 of Galois)
#define DT_GAMMA			0xD2		// ������ �����-�������� (float) (2^59 of Galois)
#define DT_SGN_FFT_FID_AM	0x6F		// ����������� ������ ����� ������� ��� ��������� ��� (float) (2^61 of Galois)
#define DT_NS_FFT_FID_AM	0xDE		// ����������� ������ ����� ���� ��� ��������� ��� (float) (2^62 of Galois)
#define DT_SGN_FFT_SE_AM	0xA1		// ����������� ������ ����� ������� ��� ��������� �� (float) (2^63 of Galois)
#define DT_NS_FFT_SE_AM		0x5F		// ����������� ������ ����� ���� ��� ��������� �� (float) (2^64 of Galois)
#define DT_AFR1_RX			0xBE		// ��� ��������� ������� 1 (float) (2^65 of Galois)
#define DT_AFR2_RX			0x61		// ��� ��������� ������� 2 (float) (2^66 of Galois)
#define DT_AFR3_RX			0xC2		// ��� ��������� ������� 3 (float) (2^67 of Galois)
#define DT_DU				0x99		// ���������� �� ���������� DU (27*uint8_t) (2^68 of Galois)
#define DT_DU_T				0x2F		// ����������� ���������� DU (9*uint8_t) (2^69 of Galois)
#define DT_DU_I				0x5E		// ���� ���� ���������� DU (3*uint8_t) (2^70 of Galois)
#define DT_DU_U				0xBC		// ���������� ���������� DU (3*uint8_t) (2^71 of Galois)
#define DT_PU				0x65		// ���������� �� ���������� PU (27*uint8_t) (2^72 of Galois)
#define DT_PU_T				0xCA		// ����������� ���������� PU (9*uint8_t) (2^73 of Galois)
#define DT_AVER_RELAX		0x89		// ����������� ������ ������ ���������� ��������������� � ������� 1 (2^74 of Galois)
#define DT_TU				0x0F		// ���������� �� ���������� TU (27*uint8_t) (2^75 of Galois)
#define DT_TU_T				0x1E		// ����������� ���������� TU (9*uint8_t) (2^76 of Galois)
#define DT_RFP				0x3C		// ������ �� ��������������� �������� (float) (2^77 of Galois)
#define DT_RFP2				0x6A		// ������ �� ��������������� �������� (float) (2^40 of Galois)
#define DT_DIEL_ADJUST		0xD6		// ������ ��������� ������� ���������������� �������� (2^85 of Galois)
#define DT_SGN_RELAX2		0xDF		// �������������� ������ �� ������� ������� (2^90 of Galois)
#define DT_AFR1_TX			0xA3		// ��� ����������� ������� 1 (float) (2^91 of Galois)
#define DT_AFR2_TX			0x5B		// ��� ����������� ������� 2 (float) (2^92 of Galois)
#define DT_AFR3_TX			0xB6		// ��� ����������� ������� 3 (float) (2^93 of Galois)
#define DT_SGN_RELAX3		0x71		// �������������� ������ � �������� ������� (float) (2^94 of Galois)
#define DT_SGN_SPEC1		0xE2		// ������ ������ �2 �� ����������� ����� ���������� ��������������� �������� 1 (2^95 of Galois)
#define DT_SGN_SPEC2		0xD9		// ������ ������ �2 �� ����������� ����� ���������� ��������������� �������� 2 (2^96 of Galois)
#define DT_SGN_SPEC3		0xAF		// ������ ������ �2 �� ����������� ����� ���������� ��������������� �������� 3 (2^97 of Galois)
#define DT_AVER_RELAX2		0x43		// ����������� ������ ������ ���������� ��������������� � ������� 2 (2^98 of Galois)
#define DT_AVER_RELAX3		0x86		// ����������� ������ ������ ���������� ��������������� � ������� 3 (2^99 of Galois)
#define DT_SOLID_ECHO		0x22		// �����-��� � ������� 1 (2^101 of Galois)
#define DT_T1T2_NMR			0x44		// ������ ��������� �1-�2 �������������	(2^102 of Galois)
#define DT_DsT2_NMR			0x88		// ������ ��������� ��� �������������-������������� (�1-Ds ���)	(2^103 of Galois)
#define DT_FREQ_TUNE		0x0D		// ������ ������������� ������� ��� ��� ���� (2^104 of Galois)


// ------------------ UART Commands for c6x program ----------------------
#define NONE                0x00        // No data or no command
#define DATA_OK				0x01		// Incoming data is OK ( 2^1 Galois)
#define BAD_DATA			0x02		// Incoming data is failed ( 2^2 Galois)
#define NMRTOOL_IS_READY	0x04		// NMR Tool is ready to get data/commands from PC_Main (2^3 Galois)
#define NMRTOOL_ISNOT_READY 0x08		// NMR Tool is not ready to receive data/commands from PC_Main (2^4 Galois)
#define HEADER_OK			0x10		// Header of Multypacket message was received and decoded successfully
#define GET_DATA			0x74		// Get actual data (2^10 of Galois)
#define GET_RELAX_DATA      0xE8        // Get only relaxation data	(2^11 of Galois)
#define NMRTOOL_CONNECT		0x13		// Connect to NMR Tool (2^14 of Galois)
#define NMRTOOL_START		0x26		// Stop FPGA program (2^15 of Galois)
#define NMRTOOL_STOP		0x4C		// Start FPGA program (2^16 of Galois)
#define NMRTOOL_DATA		0x35		// NMR Tool data (any) (2^39 of Galois)
#define REPEAT_CMD			0xBB		// Request to Repeat data (2^83 of Galois)
#define DIEL_ADJUST_START	0xB1		// Request to Repeat data (2^86 of Galois)
#define DIEL_DATA_READY		0xE1		// SDSP data ready to send (2^89 of Galois)
#define NMRTOOL_CONNECT_DEF	0x1A		// first connect to LoggingTool (2^105 of Galois)

// Format of command:
// Command (1 byte)



#define GET_ECHO_DATA		0x68
#define GET_SPEC_DATA		0x63
#define GET_ADC_DATA        0x4B		// Get original data (echo) obtained from ADC


#define FOREVER             0xFF
#define NAN_DATA			0xFF		// No available data


// Set apodizing function for echoes and fft-spectra
#define SET_ECHO_WINDOW     0x45
#define SET_SPEC_WINDOW     0x46
// Format of commands:
//Command     Type_of_func              X0                  Sigma
//(1 byte)   (1 byte - uint8_t)  (2 bytes - uint16_t)   (4 bytes - float)
// Types of functions:  0x00 - no function,    0x01 - Gauss,  0x02 - Double-Gauss
// Example: 0x45    0x01    0x00    0xF0     0x44    0x7A    0x00    0x00 :   Set Gauss function to echo with X0 = 240, Sigma = 1000

// -------------------------------------------------------------------------


// Send data from c6x to PC
#define ORG_NMR_SIGNAL		0x87		// �������� ������, ���������� � ��� (2^13 of Galois)
#define ECHO_DATA           0x4F
#define RELAX_DATA			0xCD		// �������� �������������� ������ (2^12 of Galois)
#define SPEC_DATA           0x5A
// Format of commands:
// Command    Data Length (bytes)               Data                
//(1 byte)   (2 bytes - uint16_t)      (up to ~56000 bytes) Note: Max. data length ~ 255 packets * (255 - recovery bytes - 4)
// -------------------------------------------------------------------------


// Send data from PC to c6x
#define DATA_PROC			0x98		// ��������� ��� ��������� ������� ��� � ��������� ������, ���������� ���������� ����������� (2^17 of Galois)
#define FPGA_PRG			0x2D		// ��������� ��������� ������ ���, ���������� ���������� ����������� (2^18 of Galois)
#define SET_WIN_PARAMS		0xB9		// Set parameters of apodizing and window functions (2^60 of Galois)
#define SET_COMM_PARAMS		0x78		// ��������� ��������: ����� ������ ����-��������, ���-�� ������������ ������ � �.�. (2^78 of Galois)
#define SDSP_DATA			0x6B		// �������� ���������� ��������� � ��������������� ������ (2^84 of Galois)
#define SDSP_REQUEST_C8		0x7F		// �������� ������� 0xC8 �� ������ � ��������������� ������ (2^87 of Galois)
#define SDSP_REQUEST_88		0xFE		// �������� ������� 0x88 �� ������ � ��������������� ������ (2^88 of Galois)
#define LOG_TOOL_SETTINGS	0x11		// �������� ���������� ��������� ����������� ������� (��. QVector<ToolChannel*> tool_channels) (2^100 of Galois)
#define PRESS_UNIT_OPEN		0x34		// ��������� "����" ���������� ������� (2^106 of Galois)
#define PRESS_UNIT_CLOSE	0xD0		// ��������� "����" ���������� ������� (2^108 of Galois)
#define PRESS_UNIT_STOP		0xD1		// ���������� ��������"����" ���������� �������
// Format of commands:
// Command    Data Length (bytes)               Data                
//(1 byte)   (2 bytes - uint16_t)      (up to ~56000 bytes) Note: Max. data length ~ 255 packets * (255 - recovery bytes - 4)
// -------------------------------------------------------------------------



//------------------------- Ids of Service data ----------------------------
#define DATA_STATE_OK		0x01        // Info about state of data (OK!)
#define DATA_STATE_FAIL		0xFF        // Info about state of data (FAIL!)



//---------------------- Start markers of UART Msgs ------------------------
typedef enum { 							// ������� ������ UART ���������, ������������ ����� ��� ���������
	MTYPE_MULTYPACK =		0x45,		// ������ ������ ��������� �������������� ���������
	MTYPE_PACKET =			0x63, 		// ������ ������ ������
	MTYPE_PACKET_ILV =		0x78,		// ������ ������ ������������ (interleaving) ������
	MTYPE_SERVICE =			0x91, 		// ������ ���������� ���������
	MTYPE_SHORT =			0xB7		// ������ ��������� ��������� (�� ��������� ���������� ���������� ���������)
} MTYPE;
//--------------------------------------------------------------------------



#define HEAD_REC_LEN		4			// ����� ����������� ����� ��������� ���������
#define HEAD_INFO_LEN		8			// ����� �������������� ����� ��������� ���������
#define HEADER_LEN			HEAD_REC_LEN + HEAD_INFO_LEN	// ����� ��������� ��������� (msg_header)
#define MAX_REC_ERRS		32			// ������������ ����� �������������� ������ � ����������
#define SRV_DATA_LEN		4			// ����� ������ � ��������� ���������
#define SHORT_DATA_LEN		4			// ����� ������ � �������� ���������
#define SRV_DATA_PTR		HEAD_INFO_LEN - SRV_DATA_LEN -1	// ��������� �� ������ ��������� ������ � ��������� ���� MTYPE_SERVICE
#define PACK_INFO_LEN		3			// ����� �������������� ����� ������ (����� ��������� ������)
#define MIN_PACK_LEN		PACK_INFO_LEN+2	// ����������� ����� ������
#define MAX_PACK_LEN		254			// ������������ ����� ������


// ������������ �������� COMCommander
typedef enum {  						// �����, ����������������� � �������, � ������� ��������� ����������� �� UART ���������
	NOT_DEFINED,						// �������������� ��������� (�������� ���������)
	STARTED,							// ������� ����� ���������
	FINISHED,							// ����� ��������� �������� (� ��������� ������� ������������)
	FAILED,								// ��������� �������, �� �� ����� ���� ������� ������������
	PACKS_STARTED,						// ������� ����� ������� �������������� ���������
	PACKS_FINISHED,						// ���������� ����� ������ �������������� ���������
	TIMED_OUT							// ������� �����, ���������� �� ����� ���������
} MultyState;


// ������������ NMRToolLinker
typedef enum { 
	MsgOK,								// ��� ������ � �������� ���������
	MsgPoor,							// ������� ������ � ���� ������
	MsgBad,								// ��������� �� ������� ������������
	MsgOff								// ��� ���������
} MsgState;	


typedef enum { 
	State_OK,							// ����� ����������� 
	State_Connecting,					// � �������� ������������ �����
	State_No							// ��� �����
} ConnectionState;	


// ******************** Depth Meter parameters ******************

// ���� ���������� �������
#define DEPTH_DATA			0xA1		// �������
#define RATE_DATA			0xA4		// �������� ������/�������
#define TENSION_DATA		0xA8		// ���������
#define MAGN_LABELS			0xA9		// �������� �����
#define DEVICE_SEARCH		0x01		// ����� �����������


#define DEPTH_DATA_LEN		3			// ����� ���������� �� ����������� ������ � ������
#define LEUZE_DATA_LEN		9			// ����� �����, ����������� �� Leuze ODSL 96B (� ������)


#define DEPTH_REQ_TIMEOUT	250			// �����, ����������� �� ��������� ������ �� ������ �� �����������
 
// *************************************************************


// Tool Channel types
#define NMR_CHANNEL			0x01		// ������ ������ ���
//#define NMR_CHANNEL_2		0x02		// ������ ������ ���
#define GK_CHANNEL			0x03		// �����-������
#define SDSP_CHANNEL		0x04		// ��������������� ������
#define AFR_CHANNEL			0x05		// ����������-��������� �������������� ������� ������� ���
//#define AFR_CHANNEL_2		0x06		// ����������-��������� �������������� ������� ������� ���
#define RF_PULSE_CHANNEL	0x07		// �����-��������� ������� ������� ������� ���
//#define RF_PULSE_CHANNEL_2	0x08		// �����-��������� ������� ������� ������� ���



struct ToolChannel
{
	ToolChannel(uint8_t _channel_id, QString _data_type, uint8_t _frq_set_num, QString _name,
				double _depth_displ = 0,
				double _norm_coef1 = 1,
				double _norm_coef2 = 1,
				double _meas_freq = 0,
				double _sample_freq = 0,
				double _field_gradient = 0,
				uint32_t _addr_rx = 0,
				uint32_t _addr_tx = 0,
				uint32_t _frq1 = 0, 
				uint32_t _frq2 = 0, 
				uint32_t _frq3 = 0, 
				uint32_t _frq4 = 0, 
				uint32_t _frq5 = 0, 
				uint32_t _frq6 = 0) 
	{
		channel_id = _channel_id;
		data_type = _data_type;
		frq_set_num = _frq_set_num;
		name = _name;
		frq1 = _frq1;
		frq2 = _frq2;
		frq3 = _frq3;
		frq4 = _frq4;
		frq5 = _frq5;
		frq6 = _frq6;
		addr_rx = _addr_rx;
		addr_tx = _addr_tx;
		depth_displ = _depth_displ;
		normalize_coef1 = _norm_coef1;
		normalize_coef2 = _norm_coef2;
		meas_frq = _meas_freq;
		sample_freq = _sample_freq;		
		field_gradient = _field_gradient;

		//calibration_state = false;
	}	

public:
	uint8_t channel_id;				// id ������
	QString data_type;				// ��� ������ (���, �����-������� � �.�.)
	uint8_t frq_set_num;			// ����� ������ ������
	QString name;					// �������� ������
	uint32_t frq1;					// ������� #1
	uint32_t frq2;					// ������� #2
	uint32_t frq3;					// ������� #3
	uint32_t frq4;					// ������� #4
	uint32_t frq5;					// ������� #5
	uint32_t frq6;					// ������� #6
	uint32_t addr_rx;				// ����� ���������
	uint32_t addr_tx;				// ����� �����������
	double depth_displ;				// �������� ����� ������ ������		
	double normalize_coef1;			// ������������� ���������� #1 (������ ������� ������� � ����������)	
	double normalize_coef2;			// ������������� ���������� #1 (������ ������� ������� � ����������)
	double meas_frq;				// �������, �� ������� ���������� ���������
	double sample_freq;				// ������� ������������� (��� ���)	
	double field_gradient;			// �������� ���������� ����

private:
	//bool calibration_state;			// ������������� ��� �� ������������� ������ ��� �� ������� ������
	QVector<double> calibration_store;// ��������� ������ ��� ����������

public:
	//void setCalibrated(bool state) { calibration_state = state; }
	//bool isCalibrated() { return calibration_state; }
	
	void addCalibrationData(double val) { calibration_store.append(val); }
	bool calibrationStoreIsEmpty() { return calibration_store.isEmpty(); }
	void clearCalibrationStore() { calibration_store.clear(); }
	double mean90()
	{
		double S = 0;

		if (!calibration_store.isEmpty())
		{
			int store_size = calibration_store.size();
			QVector<double> temp(store_size);
			memcpy(temp.data(), calibration_store.data(), store_size*sizeof(double));
			qSort(temp);

			double _max = temp.last();			
			int N = 0;
			for (int j = 0; j < temp.count(); j++)
			{
				if (temp.data()[j] >= 0.90*_max) 
				{
					S = S + temp.data()[j];
					N += 1;
				}
			}
			if (N > 0) S = S/N;			
		}

		return S;
	}
};


#endif // IO_GENERAL_H
