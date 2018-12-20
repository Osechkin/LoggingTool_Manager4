#ifndef LUSI_H
#define LUSI_H

#include "stdarg.h"

#include <QObject>
#include <QString>
#include <QPair>
#include <QVector>
#include <QScriptEngine>

#include "../Common/experiment_settings.h"


#ifndef uint8_t
typedef unsigned __int8   uint8_t;
#endif

#ifndef uint16_t
typedef unsigned __int16   uint16_t;
#endif

#ifndef uint32_t
typedef unsigned __int32   uint32_t;
#endif


typedef QVector<int>					QIntVector;
typedef QVector<QPair<int,int> >		QIntPairVector;
typedef QList<QPair<QString,QString> >	QStringPairList;
typedef QVector<uint8_t>				QByteVector;



namespace LUSI
{
	static QStringList separate(QString _str, QString _separator, QString _ignore_list, bool _trim = true);
	static bool getStrName(QString &_str);

	
	struct Definition
	{
		enum Type { Main, Section, Output, Parameter, ProcPackage, ComProgram, Condition, Argument, Unknown };

		void clear() { type = Unknown; name = ""; fields.clear(); }

		Type type;
		QString name;
		QStringPairList fields;
	};

	
	class Object : public QObject
	{
		Q_OBJECT

	public:
		Object() { obj_name = ""; }

		QString getObjName() const { return obj_name; }
		void setObjName(QString _obj_name) { obj_name = _obj_name; }

		Definition::Type getType() const { return type; }
		QStringList getErrorList() const { return elist; }

	public:
		QString obj_name;

	protected:
		Definition::Type type;
		QStringList elist;			// error list
	};
	

	class Main : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString name READ getName WRITE setName);
		Q_PROPERTY(QString author READ getAuthor WRITE setAuthor);
		Q_PROPERTY(QString description READ getDescription WRITE setDescription);
		Q_PROPERTY(QString datetime READ getDateTime WRITE setDateTime);
		Q_PROPERTY(int msg_req_delay READ getMsgReqDelay WRITE setMsgReqDelay);

	public:
		Main(QString _obj_name) { setObjName(_obj_name); type = Definition::Main; msg_req_delay = 0; }

	public slots:
		QString getName() const { return name; }
		void setName(QString _name) { name = _name; }

		QString getAuthor() const { return author; }
		void setAuthor(QString _author) { author = _author; }

		QString getDescription() const { return description; }
		void setDescription(QString _description) { description = _description; }

		QString getDateTime() const { return datetime; }
		void setDateTime(QString _datetime) { datetime = _datetime; }

		int getMsgReqDelay() const { return msg_req_delay; }
		void setMsgReqDelay(int _msg_req_delay) { msg_req_delay = _msg_req_delay; }

	public:
		QString name;
		QString author;
		QString description;
		QString datetime;
		int msg_req_delay;			// �������� �� �������� ������ �� ������������ ��������� (� ms) - �������������� �������� MSG_REQ_DELAY, ��������� �� *.ini �����
	};

	class Section : public Object
	{
		Q_OBJECT
					
		Q_PROPERTY(QString title READ getTitle WRITE setTitle);
		Q_PROPERTY(QObject* last_parameter READ last WRITE append);
		
	public:
		Section(QString _obj_name) { setObjName(_obj_name); type = Definition::Section; }
				
		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }

		QObjectList getParameters() { return parameters; }		
		void clearParameters() { parameters.clear(); }

		int size() const { return parameters.size(); }

		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);

	public slots:
		void append(QObject* _obj) { parameters.append(_obj); last_parameter = _obj; }
		QObject* last() const { return parameters.last(); }

	public:
		QString title;

	private:
		QObject* last_parameter;
		QObjectList parameters;
	};


	class Output : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString title READ getTitle WRITE setTitle);
		Q_PROPERTY(QObject* last_parameter READ last WRITE append);

	public:
		Output(QString _obj_name) { setObjName(_obj_name); type = Definition::Output; }

		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }

		QObjectList getParameters() { return parameters; }		

		int size() const { return parameters.size(); }

		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);

		public slots:
			void append(QObject* _obj) { parameters.append(_obj); last_parameter = _obj; }
			QObject* last() const { return parameters.last(); }

	public:
		QString title;

	private:
		QObject* last_parameter;
		QObjectList parameters;
	};

	
	class Parameter : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString title READ getTitle WRITE setTitle);
		Q_PROPERTY(double min READ getMin WRITE setMin);
		Q_PROPERTY(double max READ getMax WRITE setMax);
		Q_PROPERTY(double value READ getValue WRITE setValue);
		Q_PROPERTY(QString comment READ getComment WRITE setComment);
		Q_PROPERTY(QString units READ getUnits WRITE setUnits);
		Q_PROPERTY(QString formula READ getFormula WRITE setFormula);
		Q_PROPERTY(QString uitype READ getUIType WRITE setUIType);
		Q_PROPERTY(bool ron READ getReadOnly WRITE setReadOnly);

	public:
		Parameter(QString _obj_name)
		{
			setObjName(_obj_name);
			title = ""; 
			value = 0; 
			min = -8388608;
			max = 8388607;
			comment = "";
			units = "";
			ron = false;
			app_value = (int)value; 
			type = Definition::Parameter;
			uitype = "dspinbox";
			str_value = "";
		}
		Parameter(QString _obj_name, QString _title) 
		{ 
			setObjName(_obj_name);
			title = _title; 
			value = 0; 
			min = -8388608;
			max = 8388607;
			comment = "";
			units = "";
			ron = false;
			app_value = (int)value; 
			type = Definition::Parameter;
			uitype = "dspinbox";
			str_value = "";
		}	
		Parameter(QString _obj_name, QString _title, double _value) 
		{ 
			setObjName(_obj_name);
			title = _title; 
			value = _value; 
			min = -8388608;
			max = 8388607;
			comment = "";
			units = "";
			ron = false;
			app_value = (int)value; 
			type = Definition::Parameter;
			uitype = "dspinbox";
			str_value = "";
		}		

	public slots:
		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }

		double getMin() const { return min; }
		void setMin(double _min) { min = _min; }

		double getMax() const { return max; }
		void setMax(double _max) { max = _max; }

		double getValue() const { return value; }
		void setValue(double _value) { value = _value; }

		QString getComment() const { return comment; }
		void setComment(QString _comment) { comment = _comment; }

		QString getUnits() const { return units; }
		void setUnits(QString _units) { units = _units; }

		QString getFormula() const { return formula; }
		void setFormula(QString _formula) { formula = _formula; }

		QString getUIType() const { return uitype; }
		void setUIType(QString _uitype) { uitype = _uitype; }

		bool getReadOnly() const { return ron; }
		void setReadOnly(bool _ron) { ron = _ron; }

		void exec(double _avalue) { app_value = (int)_avalue; }
		int getAppValue() const { return app_value; }

	public:
		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);		
		void setStrValue(QString _str_value) { str_value = _str_value; }
		QString getStrValue() const { return str_value; }
		
	public:
		QString title;			// �������� ��������� (������������ � ������� �������������������) 
		double min;				// ����������� �������� value
		double max;				// ������������ �������� value		
		QString comment;		// ����������� (������������ � ���� ToolTip)
		QString units;			// ������� ���������
		QString formula;		// ������� ������� ������������ ��� ������������� ������������� �������� (�� ������������ �� ��������) 
		QString uitype;			// ��� ����������� �������� ����� �������� ��������� : 'spinbox', 'checkbox', 'combobox'
		bool ron;				// ����������/������������ ��������

	private:
		double value;			// �������� ��������� (������������ � ������������� ��������)
		int app_value;			// �������� ���������, ����������� � ���������� ��� ������������� ������������� � ����������� ����������
		QString str_value;		// ������� �������� ���������, ����������� ��� ����������� ��������, ���� �������� ����� ��� ccombobox
	};


	class ProcPackage : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString title READ getTitle WRITE setTitle);	
		Q_PROPERTY(QVariantList params_array READ getParamsArray WRITE exec);
		Q_PROPERTY(int id READ getAppValue WRITE setId);
		
	public:
		ProcPackage(QString _obj_name, QList<SeqInstrInfo> _instr_list);

		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);		

		QByteVector getProcProgram() const { return proc_program; }
		QVariantList getParamsArray() const { return params_array; }
		QList<QVariantList> getVarProcProgram() const { return var_proc_program; }
		QStringList getErrorList() const { return elist; }
		void clear();

	public slots:
		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }
		
		void exec(QVariantList _params_array);

		void setId(int _id) { id = _id; }
		int getAppValue() const { return id; }

	public:
		QString title;

	private:
		uint8_t findInstrCode(const QString &_str, bool &_flag);
					
	private:		
		QByteVector proc_program;
		QList<QVariantList> var_proc_program;
		QVariantList params_array;
		QList<SeqInstrInfo> instr_list;
		int id;
	};


	class COMProgram : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString title READ getTitle WRITE setTitle);	
		Q_PROPERTY(QVariantList params_array READ getParamsArray WRITE exec);

	public:
		COMProgram(QString _obj_name, QList<SeqCmdInfo> _com_list);

		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);		

		uint8_t findCmdCode(const QString &_str, bool &_flag);
		QByteVector getComProgram() const { return com_program; }
		QVariantList getParamsArray() const { return params_array; }
		QList<QVariantList> getVarComProgram() const { return var_com_program; }
		QStringList getErrorList() const { return elist; }
		void clear();

	public slots:
		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }

		void exec(QVariantList _params_array);

	public:
		QString title;

	private:		
		QByteVector com_program;
		QList<QVariantList> var_com_program;
		QVariantList params_array;
		QList<SeqCmdInfo> com_list; 		
	};
	

	class Condition : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString title READ getTitle WRITE setTitle);
		Q_PROPERTY(QString msg READ getMsg WRITE setMsg);
		Q_PROPERTY(QString hint READ getHint WRITE setHint);
		Q_PROPERTY(bool result READ getResult WRITE exec);

	public:
		Condition(QString _obj_name) { setObjName(_obj_name); type = Definition::Condition; }

		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }

		QString getMsg() const { return msg; }
		void setMsg(QString _msg) { msg = _msg; }

		QString getHint() const { return hint; }
		void setHint(QString _hint) { hint = _hint; }

		bool getResult() const { return result; }

		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);
				
	public slots:
		void exec(double _res) { result = (_res >= 0); }

	public:
		QString title;
		QString msg;
		QString hint;

		bool result;
	};


	class Argument : public Object
	{
		Q_OBJECT

		Q_PROPERTY(QString title READ getTitle WRITE setTitle);
		Q_PROPERTY(QString comment READ getComment WRITE setComment);
		Q_PROPERTY(QString units READ getUnits WRITE setUnits);
		//Q_PROPERTY(QString func READ getFunc WRITE setFunc);
		Q_PROPERTY(int size READ getSize WRITE setSize);
		Q_PROPERTY(QVariantList points READ getPoints WRITE setPoints);
		Q_PROPERTY(double TE READ getTE WRITE setTE);
		Q_PROPERTY(double TW READ getTW WRITE setTW);
		Q_PROPERTY(double TD READ getTD WRITE setTD);
		Q_PROPERTY(int group_index READ getAppValue WRITE setGroupIndex);

	public:
		Argument(QString _obj_name);

	public slots:
		QString getTitle() const { return title; }
		void setTitle(QString _title) { title = _title; }
			
		QString getComment() const { return comment; }
		void setComment(QString _comment) { comment = _comment; }

		QString getUnits() const { return units; }
		void setUnits(QString _units) { units = _units; }

		//QString getFunc() const { return func; }
		//void setFunc(QString _func) { func = _func; }

		int getSize() const { return size; }
		void setSize(int _size) { size = _size; }

		QVariantList getPoints() const { return points; }

		double getTE() const { return TE; }
		void setTE(double _te) { TE = _te; }

		double getTW() const { return TW; }
		void setTW(double _tw) { TW = _tw; }

		double getTD() const { return TD; }
		void setTD(double _td) { TD = _td; }

		int getAppValue() const { return group_index; }
		void setGroupIndex(int _group_index) { group_index = _group_index; }

		bool setField(QString _name, QString _value);
		bool setField(QString _value, int _index);
		
	//public slots:
		void setPoints(QVariantList _points) { points = _points; }

	public:
		QString title;
		QString comment;
		QString units;
		//QString func;
		int size;
		QVariantList points;
		double TE;				// �������� �������� TE � ������������������� ���
		double TW;				// �������� �������� TW � ������������������� ���
		double TD;				// �������� �������� TD � ������������������� ���
		int group_index;
	};

	

	typedef QList<LUSI::Definition>		DefinitionList;
	typedef QList<LUSI::Object*>		ObjectList;

	class Engine : public QObject
	{
		Q_OBJECT

	public:
		Engine(QObject *parent = NULL) { }
		Engine(QScriptEngine *_qsript_engine, QList<SeqCmdInfo> _cmd_list, QList<SeqInstrInfo> _instr_list, QObject *parent = NULL);
		~Engine();

		void init(QScriptEngine *_qsript_engine, QList<SeqCmdInfo> _cmd_list, QList<SeqInstrInfo> _instr_list);
		bool evaluate(QStringList &_elist);
		void clear();
		void reset();
		void setLUSIscript(QString _script) { lusi_script = _script; }
		QString getLUSIscript() const { return lusi_script; }
		QString getJSscript() const { return js_script; }
		ObjectList *getObjList() { return &obj_list; }
		bool findLUSIDefinition(QString &_str, Definition &_def, QString &_e);
		bool findLUSIMacros(QString &_str, QString &_e);
		//bool getXVector(QString arg_name, int size, QVector<double> &xvec);
		QStringList getErrorList() const { return error_list; }
		
	private:
		void findAllQuotas(QString &_seq, QIntVector &_quotas);
		void findAllApostrophes(QString &_seq, QIntVector &_apostrophes);
		void findAllSlashComments(QString &_seq, QIntPairVector &_slash_comments);
		void findAllStarComments(QString &_seq, QIntPairVector &_star_comments);
		QPair<int,int> nextQuoteZone(int const _pos, QIntVector const _quotas, int const _seq_len) const;
		QPair<int,int> nextApostropheZone(int const _pos, QIntVector const _apostrophes, int const _seq_len) const;
		QPair<int,int> nextSlashCommentZone(int const _pos, QIntPairVector const _slash_comments) const;
		QPair<int,int> nextStarCommentZone(int const _pos, QIntPairVector const _star_comments) const;
		//void findDeadZones(QString &_seq, QIntPairVector &_dead_zones);
		QString removeComments(QString &_text);
				
		void startLusiing(QString _script, QStringList &_elist, DefinitionList &definitions, QString &_js_script);
		uint8_t findInstrCode(const QString &_str, bool &_flag);
		uint8_t findCmdCode(const QString &_str, bool &_flag);
		
	private:
		QScriptEngine *qscript_engine;
		QList<SeqInstrInfo> instr_list;
		QList<SeqCmdInfo> cmd_list;
		QString lusi_script;
		QString js_script;
		ObjectList obj_list;
		QString started_package;
		bool package_was_started;
		QString started_comprg;
		bool comprg_was_started;
		QStringList error_list;
	};
	
		

	struct Sequence
	{		
		Sequence();
		Sequence(ObjectList *_obj_list, QString _js_script, QStringList _elist = QStringList());

		void setObjects(LUSI::ObjectList *_obj_list);
		void setFilePathName(QString _path, QString _file_name) { file_name = _file_name; file_path = _path; }
		QList<QVariantList> getVarComProgram(int index);			// ��������� ��� ������������� ������������� 
		QList<QVariantList> getVarProcProgram(int index);			// ��������� ��������� ������ ��� ����������� ���������� 
		LUSI::Argument* getArgument(int _group_index);				// ������� ������-�������� �� ��� ���������� �������, ���� ������ �� ������, �� ������� 0
		void clear();
		void reset();

		QString name;							// �������� ������������������
		QString author;							// ����� ������������������
		QString datetime;						// ���� � ����� �������� ������������������	
		QString description;					// �������� ������������������

		QString file_name;						// ��� ����� ������������������
		QString file_path;						// ���� � ����� ������������������

		QString js_script;						// ��������� ������������������ �� JavaScript

		QList<QByteVector> com_programs;		// ������ �������� ��� ������������� ������������� (������ ����)
		QList<QByteVector> proc_programs;		// ������ ������� ��������� ������ ��� ��������� NMR_Tool �� DSP
		QList<LUSI::Parameter*> param_list;		// ������ ���������� ������������������
		QList<LUSI::Section*> section_list;		// ������ �������� ����������		
		QList<LUSI::Output*> output_list;		// ������ �������� �������� ����������
		QList<LUSI::Argument*> arg_list;		// ������ ����������-���������� ��� ���������� ��� �� ��� �������� ������. ���������� ��� ������������� ������ 
		QList<LUSI::Condition*> cond_list;		// ������ ����������� ���������, ����������� ������������ ������� ���������� ������������������
		QList<LUSI::COMProgram*> comprg_list;	// ������ �������� ��� ������������� �������������
		QList<LUSI::ProcPackage*> procdsp_list;	// ������ ������� ��������� ������ ���������� �����������
		LUSI::Main *main_object;				// ������ LUSI::Main
		QStringList seq_errors;					// ������, ������������ ��� ������� ����� ������������������ (��� ������ comprg_errors � procdsp_errors !)
		QStringList comprg_errors;				// ������, ������������ ��� ������� ��������� ��� ������������� �������������
		QStringList procdsp_errors;				// ������, ������������ ��� ������� ������� ��������� ������ ��� DSP
		QStringList cond_errors;				// ������, ����������� ��� ������������ ������� ������������ �������� ���������� � ������������������
		QString js_error;						// ������, ��������� ��� ���������� ���� JavaScript
	};
}


#endif // LUSI_H