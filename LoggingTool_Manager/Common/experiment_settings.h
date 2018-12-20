#ifndef EXPERIMENT_SETTINGS_H
#define EXPERIMENT_SETTINGS_H

#include <stdint.h>

#include <QString>
#include <QStringList>
#include <QPair>
#include <QDateTime>

#include "../io_general.h"


// структура для хранения команд интервального программатора
struct SeqCmdInfo
{
	QString cmd_name;
	uint8_t cmd_code;
	QString comment;
};

// структура для хранения инструкций для обработки данных сигнальным процессором интервального программатора
struct SeqInstrInfo
{
	QString instr_name;
	uint8_t instr_code;
	QString comment;
};

// структура для хранения типов данных для обработки сигнальным процессором интервального программатора
struct SeqDataTypeInfo
{	
	uint8_t datatype_code;
	QString comment;
};


// атрибуты файлов для записи данных
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
	bool to_save;				// записывать/не записывать измренные даные в файл
	bool to_save_all;			// записывать в файл все данные вне зависимости от того, совпадает их длина с ожидаемой или нет 
};

// атрибуты эксперимента, которые вводятся перед стартом эксперимента
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


enum SeqStatus {	Seq_Changed,		// параметры импульсной последовательности или программы обработки были изменены
					Seq_Appl_OK,		// импульсная последовательность и программа обработки были успешно записаны в FPGA и DSP
					Seq_Appl_Failed,	// импульсная последовательность и/или программа обработки не были успешно записаны в FPGA и DSP
					Seq_Not_Appl 
};
enum SeqParamType { Frequency, RF_Pulse, Interval, Counter, Phase, Constant, Probe, Processing, Arg, Output, No_Type };
enum SeqErrorsType { E_ParamsCount,		// неверное количество параметров в последовательности
					E_FreqCount,		// ошибочное число параметров-частот в последовательности
					E_NoFreqs,			// последовательность не содержит параметров-частот
					E_IntervalCount,	// ошибочное число параметров-интервалов в последовательности
					E_NoIntervals,		// интервалы времени не обнаружены в последовательности
					E_RFpulseCount,		// ошибочное число радиочастотных импульсов в последовательности
					E_NoRFpulses,		// нет радиочастотных импульсов в последовательности
					E_CounterCount,		// ошибочное число параметров-счетчиков в последовательности
					E_PhaseCount,		// ошибочное число фазовых параметров в последовательности
					E_ConstantCount,	// ошибочное число параметров-констант
					E_CommandCount,		// ошибочное число команд ('Com_XXXX') в последовательности
					E_ProbeCount,		// ошибочное число датчиков
					E_BadFormula,		// некорректно задана формула или фомлула дает некорректный результат (деление на ноль ит.д.)
					E_NoCommands,		// нет команд ('Com_XXXX') в последовательности
					E_ProcessingCount,	// ошибочное количество параметров обработки данных
					E_OutputCount,		// ошибочное количество наборов выходных данных каротажного прибора
					E_InstrPacksCount,	// ошибочное число пакетов инструкций обработки сигнала ЯМР программой сигнального процессора 
					E_BadParameter		// ошибка в синтаксисе параметра
};
typedef QPair<SeqErrorsType, QString>	SeqErrors;

struct Sequence_Cmd
{	
	int cmd_number;					// номер команды, определенный в последовательности (например, для 'Com_0004' cmd_number = 4)
	QString str_byte1;				// строковая мнемоника первого байта команды 
	QString str_bytes234;			// строковая мнемоника байтов 2,3 и 4 (если задано в скрипте последовательности)
	uint8_t byte1;					// байт 1 текущей команды 'Com_XXXX' 
	uint8_t byte2;					// байт 2 текущей команды 'Com_XXXX' 
	uint8_t byte3;					// байт 3 текущей команды 'Com_XXXX' 
	uint8_t byte4;					// байт 4 текущей команды 'Com_XXXX' 
	bool flag;						// флаг успеха/неуспеха чтения команды Sequence_Cmd из *.seq файла
};

struct Sequence_Instr
{
	int instr_number;				// номер интструкции, определенный в последовательности (например, для 'Ins_0004' cmd_number = 4)
	QString str_byte1;				// строковая мнемоника первого байта инструкции ()
	QString str_byte2;				// строковое представление второго байта инструкции (число параметров обработки)
	QStringList str_params;			// параметры обработки сигнала ЯМР в строковом представлении 
	QString str_byte3;				// строковая мнемоника третьего байта инструкции (тип данных)
	uint8_t byte1;					// байт 1 текущей инструкции 'Ins_XXXX' 
	uint8_t byte2;					// байт 2 текущей инструкции 'Ins_XXXX' 
	uint8_t byte3;					// байт 3 текущей инструкции 'Ins_XXXX' 
	QList<uint8_t> param_bytes;		// побайтовое представление параметров обработки данных ЯМР
	bool flag;						// флаг успеха/неуспеха чтения инструкции Sequence_Instr из *.seq файла
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

	int pack_number;					// номер пакета инструкций
	QString pack_name;					// имя пакета инструкций
	QList<Sequence_Instr*> instr_list;	// пакеты инструкций обработки сигнала ЯМР
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

	QString name;						// имя параметра как переменной
	int par_number;						// номер параметра в текущем разделе файла последовательности (например, для "Par_0001" par_number = 1 и т.д.)
	int32_t min_value;					// минимальное допустимое значение
	int32_t max_value;					// максимальное допустимое значение
	int32_t def_value;					// значение параметра по умолчанию
	int32_t app_value;					// примененное значение параметра
	QString caption;					// название параметра
	QString description;				// описание параметра
	SeqParamType type;					// тип параметра (частота, интервал времени и т.д.)
	QString formula;					// формула пересчета введенного значения app_value/def_value к итоговому значению для ПЛИС
	QString units;						// Единицы измерения
	bool visible;						// отображать/не отображать параметр в CTreeWidget
	bool read_only;						// разрешить/запретить редактирование параметра
	bool flag;							// флаг успеха/неуспеха чтения параметра Sequence_Param из *.seq файла
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

	QString name;						// имя параметра как переменной
	int par_number;						// номер параметра в текущем разделе файла последовательности (например, для "Par_0001" par_number = 1 и т.д.)
	QString index_name;					// название индекса в формуле (например, "х" или "i" и т.д.)
	QString caption;					// название параметра
	QString description;				// описание параметра
	QString formula;					// формула пересчета введенного значения app_value/def_value к итоговому значению для ПЛИС
	QString cur_formula;				// 
	int actual_points;					// количество элементов в массиве, которое должно быть измерено (используется для релаксационных спадов)
	QString pts_formula;				// формула для расчета параметра actual_points
	QString units;						// Единицы измерения
	bool flag;							// флаг успеха/неуспеха чтения параметра Sequence_Param из *.seq файла
	QVector<double> xdata;				// вектор расчитанных по формуле formula значений "х" для измеренных данных 
};

// Элемент проверки правильности написания последовательности
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

	QString name;						// имя параметра как переменной
	int par_number;						// номер параметра в текущем разделе файла последовательности (например, для "Par_0001" par_number = 1 и т.д.)
	QString caption;					// название проверочного параметра
	QString msg;						// описание ошибки в последовательности
	QString recommendation;				// рекомендации по устранению ошибки
	QString formula;					// формула вычисления проверякмого значения. Если значение отрицательно, то имеется ошибка задания параметров в последовательности
	int32_t app_value;					// значение параметра, посчитанное по формуле formula
	int flag;							// флаг успеха/неуспеха чтения параметра Conditions из *.seq файла, также определяет >0 или <=0 значение выражения formula (см.ниже)
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

	QString name;						// название последовательности
	QString author;						// автор последовательности
	QDateTime date_time;				// дата и время создания последовательности	
	QString description;				// описание последовательности
	int freq_count;						// число параметров-частот в последовательности
	int interval_count;					// число временных интервалов в последовательности
	int rf_pulse_count;					// число радиочастотных импульсов
	int counter_count;					// число счетчиков
	int phase_count;					// число фазовых параметров
	int constant_count;					// число параметров-констант
	int probe_count;					// число датчиков
	int commands_count;					// число команд (параметр 'Commands_Number' в скрипте) в последовательности 
	int instr_packs_count;				// число пакетов инструкций (параметр 'Instructions_Packs_Number' в скрипте) в последовательности
	int processing_count;				// число параметров для обработки сигнала ЯМР 
	int output_count;					// число выходных наборов данных, присылаемых каротажным прибором
	int args_count;						// число параметров-аргументов для вычисления отсчетов по оси Ox (является необязательным)
	int conds_count;					// число условий проверки правильности написания последовательности
	QList<Sequence_Param*> param_list;	// список параметров последовательности
	QList<Sequence_Cmd*> cmd_list;		// список команд последовательности (в списке следуют строго по порядку)
	QList<Sequence_InstrPack*> instr_pack_list;	// список пакетов инструкций для обработки сигнала ЯМР программой сигнального процессора 
	QList<Argument*> arg_list;			// список параметров-аргументов для построения оси Ох для входящих данных. Обязателен для релаксационых спадов 
	QList<Condition*> cond_list;		// список проверочных праметров, проверяющих правильность задания параметров последовательности
	QList<SeqErrors> seq_errors;		// ошибки, обнаруженные при разборе файла последовательности	 
};

#endif // EXPERIMENT_SETTINGS