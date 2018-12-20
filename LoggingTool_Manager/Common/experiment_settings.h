#ifndef EXPERIMENT_SETTINGS_H
#define EXPERIMENT_SETTINGS_H

#include <stdint.h>

#include <QString>
#include <QStringList>
#include <QPair>
#include <QDateTime>

#include "../io_general.h"


// ��������� ��� �������� ������ ������������� �������������
struct SeqCmdInfo
{
	QString cmd_name;
	uint8_t cmd_code;
	QString comment;
};

// ��������� ��� �������� ���������� ��� ��������� ������ ���������� ����������� ������������� �������������
struct SeqInstrInfo
{
	QString instr_name;
	uint8_t instr_code;
	QString comment;
};

// ��������� ��� �������� ����� ������ ��� ��������� ���������� ����������� ������������� �������������
struct SeqDataTypeInfo
{	
	uint8_t datatype_code;
	QString comment;
};


// �������� ������ ��� ������ ������
struct DataSave
{
	static QString new_FileName(const DataSave& dsave)
	{
		QDateTime dateTime = QDateTime::currentDateTime();
		QString dateTime_str = dateTime.toString("yyyy-MM-dd_hh-mm-ss-zzz");
		QString file_name = "";
		if (!dsave.prefix.isEmpty()) file_name += dsave.prefix + "_";
		file_name += dateTime_str;
		if (!dsave.postfix.isEmpty()) file_name += "_" + dsave.postfix;
		file_name += "." + dsave.extension;

		return file_name;
	}

	QString file_name;
	QString path;
	QString prefix;
	QString postfix;
	QString extension;
	bool to_save;				// ����������/�� ���������� ��������� ����� � ����
	bool to_save_all;			// ���������� � ���� ��� ������ ��� ����������� �� ����, ��������� �� ����� � ��������� ��� ��� 
};

// �������� ������������, ������� �������� ����� ������� ������������
struct ExperimentSettings
{
	QString well_UID;
	QString well_name;
	QString field_name;
	QString location;
	QString province;
	QString country;
	QString service_company;
	QString company;
	QString oper;		// operator
	QString tool;
	QDate date;

	bool dont_show_again;
};


//static QList<SeqCmdInfo> seq_cmd_index;
//static QList<SeqInstrInfo> seq_instr_index;
//static QList<SeqDataTypeInfo> seq_datatype_index;


enum SeqStatus {	Seq_Changed,		// ��������� ���������� ������������������ ��� ��������� ��������� ���� ��������
					Seq_Appl_OK,		// ���������� ������������������ � ��������� ��������� ���� ������� �������� � FPGA � DSP
					Seq_Appl_Failed,	// ���������� ������������������ �/��� ��������� ��������� �� ���� ������� �������� � FPGA � DSP
					Seq_Not_Appl 
};
enum SeqParamType { Frequency, RF_Pulse, Interval, Counter, Phase, Constant, Probe, Processing, Arg, Output, No_Type };
enum SeqErrorsType { E_ParamsCount,		// �������� ���������� ���������� � ������������������
					E_FreqCount,		// ��������� ����� ����������-������ � ������������������
					E_NoFreqs,			// ������������������ �� �������� ����������-������
					E_IntervalCount,	// ��������� ����� ����������-���������� � ������������������
					E_NoIntervals,		// ��������� ������� �� ���������� � ������������������
					E_RFpulseCount,		// ��������� ����� �������������� ��������� � ������������������
					E_NoRFpulses,		// ��� �������������� ��������� � ������������������
					E_CounterCount,		// ��������� ����� ����������-��������� � ������������������
					E_PhaseCount,		// ��������� ����� ������� ���������� � ������������������
					E_ConstantCount,	// ��������� ����� ����������-��������
					E_CommandCount,		// ��������� ����� ������ ('Com_XXXX') � ������������������
					E_ProbeCount,		// ��������� ����� ��������
					E_BadFormula,		// ����������� ������ ������� ��� ������� ���� ������������ ��������� (������� �� ���� ��.�.)
					E_NoCommands,		// ��� ������ ('Com_XXXX') � ������������������
					E_ProcessingCount,	// ��������� ���������� ���������� ��������� ������
					E_OutputCount,		// ��������� ���������� ������� �������� ������ ����������� �������
					E_InstrPacksCount,	// ��������� ����� ������� ���������� ��������� ������� ��� ���������� ����������� ���������� 
					E_BadParameter		// ������ � ���������� ���������
};
typedef QPair<SeqErrorsType, QString>	SeqErrors;

struct Sequence_Cmd
{	
	int cmd_number;					// ����� �������, ������������ � ������������������ (��������, ��� 'Com_0004' cmd_number = 4)
	QString str_byte1;				// ��������� ��������� ������� ����� ������� 
	QString str_bytes234;			// ��������� ��������� ������ 2,3 � 4 (���� ������ � ������� ������������������)
	uint8_t byte1;					// ���� 1 ������� ������� 'Com_XXXX' 
	uint8_t byte2;					// ���� 2 ������� ������� 'Com_XXXX' 
	uint8_t byte3;					// ���� 3 ������� ������� 'Com_XXXX' 
	uint8_t byte4;					// ���� 4 ������� ������� 'Com_XXXX' 
	bool flag;						// ���� ������/�������� ������ ������� Sequence_Cmd �� *.seq �����
};

struct Sequence_Instr
{
	int instr_number;				// ����� �����������, ������������ � ������������������ (��������, ��� 'Ins_0004' cmd_number = 4)
	QString str_byte1;				// ��������� ��������� ������� ����� ���������� ()
	QString str_byte2;				// ��������� ������������� ������� ����� ���������� (����� ���������� ���������)
	QStringList str_params;			// ��������� ��������� ������� ��� � ��������� ������������� 
	QString str_byte3;				// ��������� ��������� �������� ����� ���������� (��� ������)
	uint8_t byte1;					// ���� 1 ������� ���������� 'Ins_XXXX' 
	uint8_t byte2;					// ���� 2 ������� ���������� 'Ins_XXXX' 
	uint8_t byte3;					// ���� 3 ������� ���������� 'Ins_XXXX' 
	QList<uint8_t> param_bytes;		// ���������� ������������� ���������� ��������� ������ ���
	bool flag;						// ���� ������/�������� ������ ���������� Sequence_Instr �� *.seq �����
};

struct Sequence_InstrPack
{
	~Sequence_InstrPack()
	{
		pack_number = 0;
		pack_name = "";

		if (!instr_list.isEmpty())
		{
			qDeleteAll(instr_list);
		}
	}

	int pack_number;					// ����� ������ ����������
	QString pack_name;					// ��� ������ ����������
	QList<Sequence_Instr*> instr_list;	// ������ ���������� ��������� ������� ���
};


struct Sequence_Param
{	
	Sequence_Param()
	{
		name = "";
		par_number = 0;
		min_value = -8388608;
		max_value = 8388607;
		def_value = NAN;
		app_value = NAN;
		caption = "";
		description = "";
		type = No_Type;
		formula = "";
		visible = true;
		read_only = false;
		flag = false;		
	}

	QString name;						// ��� ��������� ��� ����������
	int par_number;						// ����� ��������� � ������� ������� ����� ������������������ (��������, ��� "Par_0001" par_number = 1 � �.�.)
	int32_t min_value;					// ����������� ���������� ��������
	int32_t max_value;					// ������������ ���������� ��������
	int32_t def_value;					// �������� ��������� �� ���������
	int32_t app_value;					// ����������� �������� ���������
	QString caption;					// �������� ���������
	QString description;				// �������� ���������
	SeqParamType type;					// ��� ��������� (�������, �������� ������� � �.�.)
	QString formula;					// ������� ��������� ���������� �������� app_value/def_value � ��������� �������� ��� ����
	QString units;						// ������� ���������
	bool visible;						// ����������/�� ���������� �������� � CTreeWidget
	bool read_only;						// ���������/��������� �������������� ���������
	bool flag;							// ���� ������/�������� ������ ��������� Sequence_Param �� *.seq �����
};

struct Argument
{
	Argument()
	{
		name = "";
		par_number = 0;
		index_name = "i";
		caption = "";
		description = "";
		formula = "";
		cur_formula = "";
		actual_points = 0;
		pts_formula = "";

		units = "";
		flag = false;
	}

	QString name;						// ��� ��������� ��� ����������
	int par_number;						// ����� ��������� � ������� ������� ����� ������������������ (��������, ��� "Par_0001" par_number = 1 � �.�.)
	QString index_name;					// �������� ������� � ������� (��������, "�" ��� "i" � �.�.)
	QString caption;					// �������� ���������
	QString description;				// �������� ���������
	QString formula;					// ������� ��������� ���������� �������� app_value/def_value � ��������� �������� ��� ����
	QString cur_formula;				// 
	int actual_points;					// ���������� ��������� � �������, ������� ������ ���� �������� (������������ ��� �������������� ������)
	QString pts_formula;				// ������� ��� ������� ��������� actual_points
	QString units;						// ������� ���������
	bool flag;							// ���� ������/�������� ������ ��������� Sequence_Param �� *.seq �����
	QVector<double> xdata;				// ������ ����������� �� ������� formula �������� "�" ��� ���������� ������ 
};

// ������� �������� ������������ ��������� ������������������
struct Condition
{
	Condition()
	{
		name = "";
		par_number = 0;
		caption = "";
		msg = "";
		recommendation = "";
		formula = "";
		app_value = 0;
		flag = false;
	}

	QString name;						// ��� ��������� ��� ����������
	int par_number;						// ����� ��������� � ������� ������� ����� ������������������ (��������, ��� "Par_0001" par_number = 1 � �.�.)
	QString caption;					// �������� ������������ ���������
	QString msg;						// �������� ������ � ������������������
	QString recommendation;				// ������������ �� ���������� ������
	QString formula;					// ������� ���������� ������������ ��������. ���� �������� ������������, �� ������� ������ ������� ���������� � ������������������
	int32_t app_value;					// �������� ���������, ����������� �� ������� formula
	int flag;							// ���� ������/�������� ������ ��������� Conditions �� *.seq �����, ����� ���������� >0 ��� <=0 �������� ��������� formula (��.����)
										// bit0 = 1: formula OK, bit0 = 0: formula Bad; bit1 = 1: result of formula OK, bit1 = 0: result of formula Bad.
};

struct Sequence
{
	Sequence()
	{
		clear();
	}

	~Sequence()
	{
		qDeleteAll(param_list);
		qDeleteAll(cmd_list);
		qDeleteAll(instr_pack_list);
		qDeleteAll(arg_list);
		qDeleteAll(cond_list);
	}

	void clear()
	{
		name = "";
		author = "";
		date_time = QDateTime::currentDateTime();		
		freq_count = 0;
		interval_count = 0;
		rf_pulse_count = 0;
		counter_count = 0;
		phase_count = 0;
		constant_count = 0;
		probe_count = 0;
		commands_count = 0;
		instr_packs_count = 0;
		processing_count = 0;
		output_count = 0;
		args_count = 0;
		conds_count = 0;

		seq_errors.clear();

		if (!param_list.isEmpty())
		{
			qDeleteAll(param_list);
			param_list.clear();
		}

		if (!cmd_list.isEmpty())
		{
			qDeleteAll(cmd_list);
			cmd_list.clear();
		}

		if (!arg_list.isEmpty())
		{
			qDeleteAll(arg_list);
			arg_list.clear();
		}

		if (!cond_list.isEmpty())
		{
			qDeleteAll(cond_list);
			cond_list.clear();
		}

		if (!instr_pack_list.isEmpty())
		{
			for (int i = 0; i < instr_pack_list.count(); i++)
			{
				Sequence_InstrPack* instr_pack = instr_pack_list[i];
				delete instr_pack;
			}	
			instr_pack_list.clear();
			instr_packs_count = 0;
		}
	}

	QString name;						// �������� ������������������
	QString author;						// ����� ������������������
	QDateTime date_time;				// ���� � ����� �������� ������������������	
	QString description;				// �������� ������������������
	int freq_count;						// ����� ����������-������ � ������������������
	int interval_count;					// ����� ��������� ���������� � ������������������
	int rf_pulse_count;					// ����� �������������� ���������
	int counter_count;					// ����� ���������
	int phase_count;					// ����� ������� ����������
	int constant_count;					// ����� ����������-��������
	int probe_count;					// ����� ��������
	int commands_count;					// ����� ������ (�������� 'Commands_Number' � �������) � ������������������ 
	int instr_packs_count;				// ����� ������� ���������� (�������� 'Instructions_Packs_Number' � �������) � ������������������
	int processing_count;				// ����� ���������� ��� ��������� ������� ��� 
	int output_count;					// ����� �������� ������� ������, ����������� ���������� ��������
	int args_count;						// ����� ����������-���������� ��� ���������� �������� �� ��� Ox (�������� ��������������)
	int conds_count;					// ����� ������� �������� ������������ ��������� ������������������
	QList<Sequence_Param*> param_list;	// ������ ���������� ������������������
	QList<Sequence_Cmd*> cmd_list;		// ������ ������ ������������������ (� ������ ������� ������ �� �������)
	QList<Sequence_InstrPack*> instr_pack_list;	// ������ ������� ���������� ��� ��������� ������� ��� ���������� ����������� ���������� 
	QList<Argument*> arg_list;			// ������ ����������-���������� ��� ���������� ��� �� ��� �������� ������. ���������� ��� ������������� ������ 
	QList<Condition*> cond_list;		// ������ ����������� ���������, ����������� ������������ ������� ���������� ������������������
	QList<SeqErrors> seq_errors;		// ������, ������������ ��� ������� ����� ������������������	 
};

#endif // EXPERIMENT_SETTINGS