#include <QTextStream>
#include <QFileDialog>

#include "LUSI.h"

using namespace LUSI;


QStringList LUSI::separate(QString _str, QString _separator, QString _ignore_list, bool _trim)
{
	int len = _str.length();
	QStringList out;

	QString acc = "";
	bool is_ignore = false;
	QString sym_ignore = "";
	for (int i = 0; i < len; i++)
	{
		QString sym = _str[i];
		if (is_ignore && sym == sym_ignore)
		{
			is_ignore = false;
			sym_ignore = "";
			acc += sym;
		}
		else if (!is_ignore && _ignore_list.contains(sym))
		{
			is_ignore = true;
			sym_ignore = sym;
			acc += sym;
		}
		else if (!is_ignore)
		{
			if (sym == _separator)
			{
				out << (_trim ? acc.trimmed() : acc);
				acc = "";
			}
			else 
			{
				acc += sym;
			}
		}
		else if (is_ignore)
		{
			acc += sym;
		}
	}
	if (!acc.isEmpty()) out << acc;

	return out;
}

bool LUSI::getStrName(QString &_str)
{
	int len = _str.length();
	if (len > 1)
	{
		if (_str.at(0) == "\"" && _str.at(len-1) == "\"") _str = _str.mid(1, len-2);
		else if (_str.at(0) == "'" && _str.at(len-1) == "'") _str = _str.mid(1, len-2);
		else return false;
	}
	else false;

	return true;
}


// ********************* Section in Sequence Manager *************************
bool LUSI::Section::setField(QString _name, QString _value)
{	
	if (_name.isEmpty()) return false;

	if (_name == "title") if (LUSI::getStrName(_value)) title = _value;	else return false;
	else return false;

	return true;
}

bool LUSI::Section::setField(QString _value, int _index)
{
	if (_index == 0) if (LUSI::getStrName(_value)) title = _value; else return false; 
	else return false;

	return true;
}
// ***************************************************************************


// ***************** Output section in Sequence Manager **********************
bool LUSI::Output::setField(QString _name, QString _value)
{	
	if (_name.isEmpty()) return false;

	if (_name == "title") if (LUSI::getStrName(_value)) title = _value;	else return false;
	else return false;

	return true;
}

bool LUSI::Output::setField(QString _value, int _index)
{
	if (_index == 0) if (LUSI::getStrName(_value)) title = _value; else return false;
	else return false;

	return true;
}
// ***************************************************************************


// ******************** Parameter in Sequence Manager ************************
bool LUSI::Parameter::setField(QString _name, QString _value)
{
	bool ok;
	if (_name.isEmpty()) return false;
	
	if (_name == "title") if (LUSI::getStrName(_value)) title = _value; else return false;
	else if (_name == "value") 
	{
		if (_value.contains("|")) { str_value = _value; value = 0; app_value = value; }
		else { value = _value.toDouble(&ok); if (!ok) return false; } 		
	}
	else if (_name == "min") min = _value.toDouble(&ok); if (!ok) return false;
	else if (_name == "max") max = _value.toDouble(&ok); if (!ok) return false;
	else if (_name == "units") if (LUSI::getStrName(_value)) units = _value; else return false;
	else if (_name == "uitype") if (LUSI::getStrName(_value)) uitype = _value; else return false;
	else if (_name == "comment") if (LUSI::getStrName(_value)) comment = _value; else return false;
	else if (_name == "ron") 
	{
		int _ron = _value.toInt(&ok); 
		if (ok) ron = _ron;
		else 
		{
			if (_value.toLower() == "true") ron = true; 
			else if (_value.toLower() == "false") ron = false;
			else return false;
		}
	}
	else if (_name == "formula") if (LUSI::getStrName(_value)) formula = _value; else return false;
	else return false;
	
	return true;
}

bool LUSI::Parameter::setField(QString _value, int _index)
{
	bool ok;
	switch (_index)
	{
	case 0: if (LUSI::getStrName(_value)) title = _value; else return false; break;
	case 1: 
		{
			if (_value.contains("|")) { str_value = _value; value = 0; app_value = value; }
			else { value = _value.toDouble(&ok); if (!ok) return false; else app_value = value; } 
			break; 
		}
	case 2: min = _value.toDouble(&ok); if (!ok) return false; break;
	case 3: max = _value.toDouble(&ok); if (!ok) return false; break;
	case 4: if (LUSI::getStrName(_value)) units = _value; else return false; break;
	case 5: if (LUSI::getStrName(_value)) uitype = _value; else return false; break;
	case 6: if (LUSI::getStrName(_value)) comment = _value; else return false; break;
	case 7: 
		{
			int _ron = _value.toInt(&ok); 
			if (ok) ron = _ron;
			else 
			{
				if (_value.toLower() == "true") ron = true; 
				else if (_value.toLower() == "false") ron = false;
				else return false;
			}
			break;
		}
	case 8: if (LUSI::getStrName(_value)) formula = _value; else return false; break;
	default: return false;
	}

	return true;
}
// ***************************************************************************


// ************************ DSP Processing Package ***************************
LUSI::ProcPackage::ProcPackage(QString _obj_name, QList<SeqInstrInfo> _instr_list) 
{ 
	setObjName(_obj_name); 
	instr_list = _instr_list; 
	type = Definition::ProcPackage; 
	id = 0;
}

bool LUSI::ProcPackage::setField(QString _name, QString _value)
{	
	if (_name.isEmpty()) return false;

	if (_name == "title") if (LUSI::getStrName(_value)) title = _value;	else return false;
	else return false;

	return true;
}

bool LUSI::ProcPackage::setField(QString _value, int _index)
{
	if (_index == 0) if (LUSI::getStrName(_value)) title = _value; else return false;
	else return false;

	return true;
}

uint8_t LUSI::ProcPackage::findInstrCode(const QString &_str, bool &_flag)
{
	uint8_t res = 0;
	_flag = false;
	for (int i = 0; i < instr_list.count(); i++)
	{
		if (_str == instr_list[i].instr_name) 
		{
			res = instr_list[i].instr_code;		
			_flag = true;
		}
	}
	return res;
}

void LUSI::ProcPackage::exec(QVariantList _params_array)
{	
	if (_params_array.count() < 3) 
	{
		QString e = tr("Wrong format of instruction in Processing Package '%1'!").arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;	
		return;
	}

	bool _ok = false;
	uint8_t ins_code = findInstrCode(_params_array.first().toString(), _ok);
	if (!_ok) 
	{
		QString e = tr("Unknown Instruction '%1' in Processing Packet '%2'!").arg(_params_array.first().toString()).arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;
		return;
	}

	int N = _params_array[1].toUInt(&_ok);
	if (!_ok)
	{
		QString e = tr("Cannot read the number of parameters in instruction '%1' in Processing Packet '%2'!").arg(_params_array.first().toString()).arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;
		return;
	}

	int type = _params_array[2].toUInt(&_ok);
	if (!_ok)
	{
		QString e = tr("Cannot read data type in instruction '%1' in Processing Packet '%2'!").arg(_params_array.first().toString()).arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;
		return;
	}
	
	if (proc_program.isEmpty()) proc_program << id;
	proc_program << ins_code << N << type;
	for (int i = 0; i < N; i++)
	{
		/*
		uint8_t val = _params_array[i+3].toUInt(&_ok);
		if (!_ok)
		{
			QString e = tr("Cannot read parameter #%1 in instruction '%2' in Processing Packet '%3'!").arg(i+1).arg(_params_array.first().toString()).arg(title);
			elist << tr("<font color=red>JS: Error ! : </font>") + e;
			return;
		}
		proc_program << val;
		*/

		int param = _params_array[i+3].toInt(&_ok);
		if (_ok)
		{
			uint8_t byte1 = (uint8_t)(param & 0x000000FF); 
			uint8_t byte2 = (uint8_t)((param & 0x0000FF00) >> 8); 
			uint8_t byte3 = (uint8_t)((param & 0x00FF0000) >> 16);
			uint8_t byte4 = (uint8_t)((param & 0xFF000000) >> 24);

			proc_program << byte1 << byte2 << byte3 << byte4;
		}
		else
		{
			QString e = tr("Cannot read parameter #%1 in instruction '%2' in Processing Packet '%3'!").arg(i+1).arg(_params_array.first().toString()).arg(title);
			elist << tr("<font color=red>JS: Error ! : </font>") + e;
			return;
		}
	}	
	params_array = _params_array; 
	var_proc_program.append(_params_array);
}

void LUSI::ProcPackage::clear()
{
	proc_program.clear();
	var_proc_program.clear();
}
// ***************************************************************************


// ********************* FPGA command program object *************************
LUSI::COMProgram::COMProgram(QString _obj_name, QList<SeqCmdInfo> _com_list) 
{ 
	setObjName(_obj_name); 
	com_list = _com_list; 
	type = Definition::ComProgram; 
}

bool LUSI::COMProgram::setField(QString _name, QString _value)
{	
	if (_name.isEmpty()) return false;

	if (_name == "title") if (LUSI::getStrName(_value)) title = _value;	else return false;
	else return false;

	return true;
}

bool LUSI::COMProgram::setField(QString _value, int _index)
{
	if (_index == 0) if (LUSI::getStrName(_value)) title = _value; else return false; 
	else return false;

	return true;
}

uint8_t LUSI::COMProgram::findCmdCode(const QString &str, bool &flag)
{
	uint8_t res = 0;
	flag = false;
	for (int i = 0; i < com_list.count(); i++)
	{
		if (str == com_list[i].cmd_name) 
		{
			res = com_list[i].cmd_code;		
			flag = true;
		}
	}
	return res;
}

void LUSI::COMProgram::exec(QVariantList _params_array)
{	
	if ( !((_params_array.count() == 2) || (_params_array.count() == 4)) ) 
	{
		QString e = tr("Wrong command format in FPGA Program '%1'!").arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;	
		return;
	}

	bool _ok = false;
	uint8_t com_code = findCmdCode(_params_array.first().toString(), _ok);
	if (!_ok) 
	{
		QString e = tr("Unknown Command '%1' in FPGA Program '%2'!").arg(_params_array.first().toString()).arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;
		return;
	}
		
	QVariantList varray;
	if (_params_array.count() == 2)
	{
		uint32_t new_value = _params_array.last().toUInt(&_ok);
		if (_ok)
		{
			uint8_t byte4 = (uint8_t)(new_value & 0xFF);
			uint8_t byte3 = (uint8_t)((new_value & 0xFF00) >> 8);
			uint8_t byte2 = (uint8_t)((new_value & 0xFF0000) >> 16);
			com_program << com_code << byte2 << byte3 << byte4;	
			varray << _params_array.first() << byte2 << byte3 << byte4;
		}
		else 
		{
			QString e = tr("Cannot read parameter of Command '%1' in FPGA Program '%2'!").arg(_params_array.first().toString()).arg(title);
			elist << tr("<font color=red>JS: Error ! : </font>") + e;
			return;
		}			
	}
	else if (_params_array.count() == 4)
	{
		uint8_t byte2 = (uint8_t)(_params_array[1].toUInt(&_ok));
		uint8_t byte3 = (uint8_t)(_params_array[2].toUInt(&_ok));
		uint8_t byte4 = (uint8_t)(_params_array[3].toUInt(&_ok));
		com_program << com_code << byte2 << byte3 << byte4;	
		varray << _params_array.first() << byte2 << byte3 << byte4;
	}
	else
	{
		QString e = tr("Wrong number of parameters of Command '%1' in FPGA Program '%2'!").arg(_params_array.first().toString()).arg(title);
		elist << tr("<font color=red>JS: Error ! : </font>") + e;
		return;
	}
	
	params_array = _params_array; 
	var_com_program.append(varray);
}

void LUSI::COMProgram::clear()
{
	com_program.clear();
	var_com_program.clear();
}
// ***************************************************************************


// ***************************** Conditions **********************************
bool LUSI::Condition::setField(QString _name, QString _value)
{	
	if (_name.isEmpty()) return false;

	if (_name == "title") if (LUSI::getStrName(_value)) title = _value; else return false;
	else if (_name == "msg") if (LUSI::getStrName(_value)) msg = _value; else return false;
	else if (_name == "hint") if (LUSI::getStrName(_value)) hint = _value; else return false;
	else return false;
	
	return true;
}

bool LUSI::Condition::setField(QString _value, int _index)
{	
	switch (_index)
	{
	case 0: if (LUSI::getStrName(_value)) title = _value; else return false; break;
	case 1: if (LUSI::getStrName(_value)) msg = _value; else return false; break;
	case 2: if (LUSI::getStrName(_value)) hint = _value; else return false; break;	
	default: return false;
	}

	return true;
}
// ***************************************************************************


// ******************* X-Axis values for measured data ***********************
LUSI::Argument::Argument(QString _obj_name) 
{ 
	setObjName(_obj_name); 
	type = Definition::Argument; 
	title = "";
	comment = "";
	units = "";
	size = 0;	

	TE = 0;
	TD = 0;
	TW = 0;
	
	group_index = 0;
}

bool LUSI::Argument::setField(QString _name, QString _value)
{
	bool ok;
	if (_name.isEmpty()) return false;

	if (_name == "title") if (LUSI::getStrName(_value)) title = _value; else return false;
	else if (_name == "comment") if (LUSI::getStrName(_value)) comment = _value; else return false;
	else if (_name == "units") if (LUSI::getStrName(_value)) units = _value; else return false;
	//else if (_name == "func") if (LUSI::getStrName(_value)) func = _value; else return false;
	//else if (_name == "size") size = _value.toInt(&ok); if (!ok) return false;	
	else return false;

	return true;
}

bool LUSI::Argument::setField(QString _value, int _index)
{
	bool ok;
	switch (_index)
	{
	case 0: if (LUSI::getStrName(_value)) title = _value; else return false; break;
	//case 1: size = _value.toInt(&ok); if (!ok) return false; break;
	case 1: if (LUSI::getStrName(_value)) comment = _value; else return false; break;
	case 2: if (LUSI::getStrName(_value)) units = _value; else return false; break;	
	//case 3: if (LUSI::getStrName(_value)) func = _value; else return false; break;
	default: return false;
	}

	return true;
}
// ***************************************************************************


// ***************************** LUSI Engine *********************************
LUSI::Engine::Engine(QScriptEngine *_qsript_engine, QList<SeqCmdInfo> _cmd_list, QList<SeqInstrInfo> _instr_list, QObject *parent /* = NULL */)
{	
	init(_qsript_engine, _cmd_list, _instr_list);
}

LUSI::Engine::~Engine()
{
	if (!obj_list.isEmpty())
	{
		qDeleteAll(obj_list.begin(), obj_list.end());
		obj_list.clear();
	}
}

void LUSI::Engine::init(QScriptEngine *_qsript_engine, QList<SeqCmdInfo> _cmd_list, QList<SeqInstrInfo> _instr_list)
{
	qscript_engine = _qsript_engine;
	instr_list = _instr_list;
	cmd_list = _cmd_list;

	package_was_started = false;
	started_package = "";

	comprg_was_started = false;
	started_comprg = "";
}

// Find a code of instruction in the processing package
uint8_t LUSI::Engine::findInstrCode(const QString &_str, bool &_flag)
{
	uint8_t res = 0;
	_flag = false;
	for (int i = 0; i < instr_list.count(); i++)
	{
		if (_str == instr_list[i].instr_name) 
		{
			res = instr_list[i].instr_code;
			_flag = true;
		}
	}
	return res;
}

// Find a code of command in the FPGA program
uint8_t LUSI::Engine::findCmdCode(const QString &str, bool &flag)
{
	uint8_t res = 0;
	flag = false;
	for (int i = 0; i < cmd_list.count(); i++)
	{
		if (str == cmd_list[i].cmd_name) 
		{
			res = cmd_list[i].cmd_code;		
			flag = true;
		}
	}
	return res;
}

// Search all quotation marks
void LUSI::Engine::findAllQuotas(QString &_seq, QIntVector &_quotas)
{		
	_quotas.clear(); 
	int n1 = _seq.indexOf("\"");
	while (n1 >= 0 && n1 < _seq.length()-1)
	{
		_quotas.append(n1);
		if (n1 < _seq.length()-1) n1 = _seq.indexOf("\"", n1+1);
		else n1 = -1;			
	}
} 

// Search all apostrophes
void LUSI::Engine::findAllApostrophes(QString &_seq, QIntVector &_apostrophes)
{
	_apostrophes.clear();
	int n1 = _seq.indexOf("'");
	while (n1 >= 0 && n1 < _seq.length()-1)
	{
		_apostrophes.append(n1);
		if (n1 < _seq.length()-1) n1 = _seq.indexOf("'", n1+1);
		else n1 = -1;			
	}
}

// Mark comments type of "//"
void LUSI::Engine::findAllSlashComments(QString &_seq, QIntPairVector &_slash_comments)
{
	_slash_comments.clear();

	QTextStream seq_stream(&_seq);	
	QString line = seq_stream.readLine();
	int line_count = 0;
	int seq_len = 0;
	QStringList comments_txt_sl;
	while (!line.isNull())
	{
		line_count++;

		int pos = line.indexOf("//");
		if (pos >= 0) 
		{
			_slash_comments.append(QPair<int,int>(pos+seq_len, seq_len+line.length()));
			comments_txt_sl << _seq.mid(pos+seq_len, line.length()-pos);			
		}		
		seq_len += line.length()+1;
		line = seq_stream.readLine();		
	}
}
	
// Remove comments type of "/*...*/"
void LUSI::Engine::findAllStarComments(QString &_seq, QIntPairVector &_star_comments)
{
	_star_comments.clear();

	QStringList comments_txt_st;
	int n1 = _seq.indexOf("/*");
	int n2 = -1;
	while (n1 >= 0 && n1 < _seq.length()-1)
	{
		n2 = _seq.indexOf("*/", n1+1);
		if (n2 >= 0 && n2 > n1)
		{			
			_star_comments.append(QPair<int,int>(n1, n2+2));
			comments_txt_st << _seq.mid(n1, n2-n1+2);
			n1 = _seq.indexOf("/*",n2+1);
			n2 = -1;
		}
		else break;		
	}
}

QPair<int,int> LUSI::Engine::nextQuoteZone(int const _pos, QIntVector const _quotas, int const _seq_len) const
{
	QPair<int,int> res(-1,-1);
	bool found = false;
	for (int i = 0; i < _quotas.count(); i++)
	{
		if (found) continue;

		if (_pos == _quotas[i])
		{
			res.first = _quotas[i];
			if (i == _quotas.count()-1) res.second = _seq_len-1;
			else res.second = _quotas[i+1];		
			found = true;
		}
	}
	return res;
}

QPair<int,int> LUSI::Engine::nextApostropheZone(int const _pos, QIntVector const _apostrophes, int const _seq_len) const
{
	QPair<int,int> res(-1,-1);
	bool found = false;
	for (int i = 0; i < _apostrophes.count(); i++)
	{
		if (found) continue;

		if (_pos == _apostrophes[i])
		{
			res.first = _apostrophes[i];
			if (i == _apostrophes.count()-1) res.second = _seq_len-1;
			else res.second = _apostrophes[i+1];		
			found = true;
		}
	}
	return res;
}

QPair<int,int> LUSI::Engine::nextSlashCommentZone(int const _pos, QIntPairVector const _slash_comments) const
{
	QPair<int,int> res(-1,-1);
	bool found = false;
	for (int i = 0; i < _slash_comments.count(); i++)
	{
		if (found) continue;

		if (_pos == _slash_comments[i].first)
		{
			res.first = _slash_comments[i].first;
			res.second = _slash_comments[i].second;		
			found = true;
		}
	}
	return res;
}

QPair<int,int> LUSI::Engine::nextStarCommentZone(int const _pos, QIntPairVector const _star_comments) const
{
	QPair<int,int> res(-1,-1);
	bool found = false;
	for (int i = 0; i < _star_comments.count(); i++)
	{
		if (found) continue;

		if (_pos == _star_comments[i].first)
		{
			res.first = _star_comments[i].first;
			res.second = _star_comments[i].second;		
			found = true;
		}
	}
	return res;
}

/*
void Engine::findDeadZones(QString &_seq, QIntPairVector &_dead_zones)
{
	_dead_zones.clear();

	QIntVector quotas;
	QIntVector apostrophes;
	QIntPairVector slash_comments;
	QIntPairVector star_comments;

	findAllQuotas(_seq, quotas);
	findAllApostrophes(_seq, apostrophes);
	findAllSlashComments(_seq, slash_comments);
	findAllStarComments(_seq, star_comments);
		
	int pos = 0;
	bool started = false;
	while (pos < _seq.length())
	{
		QPair<int,int> start_quota = nextQuoteZone(pos, quotas, _seq.length());
		QPair<int,int> start_apostroph = nextApostropheZone(pos, apostrophes, _seq.length());
		QPair<int,int> start_slash_comm = nextSlashCommentZone(pos, slash_comments);
		QPair<int,int> start_star_comm = nextStarCommentZone(pos, star_comments);

		if (start_quota.first >= 0) 
		{ 
			_dead_zones.append(QPair<int,int>(start_quota.first, start_quota.second)); 
			pos = start_quota.second+1; 
		}
		else if (start_apostroph.first >= 0) 
		{
			_dead_zones.append(QPair<int,int>(start_apostroph.first, start_apostroph.second));
			pos = start_apostroph.second+1;
		}
		else if (start_slash_comm.first >= 0) 
		{ 
			_dead_zones.append(QPair<int,int>(start_slash_comm.first, start_slash_comm.second)); 
			pos = start_slash_comm.second+1; 
		}
		else if (start_star_comm.first >= 0) 
		{ 
			_dead_zones.append(QPair<int,int>(start_star_comm.first, start_star_comm.second));
			pos = start_star_comm.second+1;
		}
		else pos++;
	}	
}
*/

QString LUSI::Engine::removeComments(QString &_text)
{
	QString out = _text;
	QIntPairVector _comment_zones;
		
	QIntVector quotas;
	QIntVector apostrophes;
	QIntPairVector slash_comments;
	QIntPairVector star_comments;

	findAllQuotas(_text, quotas);
	findAllApostrophes(_text, apostrophes);
	findAllSlashComments(_text, slash_comments);
	findAllStarComments(_text, star_comments);

	int pos = 0;
	bool started = false;
	while (pos < _text.length())
	{
		QPair<int,int> start_quota = nextQuoteZone(pos, quotas, _text.length());
		QPair<int,int> start_apostroph = nextApostropheZone(pos, apostrophes, _text.length());
		QPair<int,int> start_slash_comm = nextSlashCommentZone(pos, slash_comments);
		QPair<int,int> start_star_comm = nextStarCommentZone(pos, star_comments);

		if (start_quota.first >= 0) 
		{ 
			//_dead_zones.append(QPair<int,int>(start_quota.first, start_quota.second)); 
			pos = start_quota.second+1; 
		}
		else if (start_apostroph.first >= 0) 
		{
			//_dead_zones.append(QPair<int,int>(start_apostroph.first, start_apostroph.second));
			pos = start_apostroph.second+1;
		}
		else if (start_slash_comm.first >= 0) 
		{ 
			_comment_zones.append(QPair<int,int>(start_slash_comm.first, start_slash_comm.second)); 
			pos = start_slash_comm.second+1; 
		}
		else if (start_star_comm.first >= 0) 
		{ 
			_comment_zones.append(QPair<int,int>(start_star_comm.first, start_star_comm.second));
			pos = start_star_comm.second+1;
		}
		else pos++;
	}	

	for (int i = _comment_zones.count()-1; i >= 0; --i)
	{
		int len = _comment_zones[i].second - _comment_zones[i].first;
		out.remove(_comment_zones[i].first, len);
	}

	return out;
}

bool LUSI::Engine::findLUSIDefinition(QString &_str, Definition &_def, QString &_e)
{
	_def.clear();
	_e = "";
		
	_str = _str.trimmed();
	_str = _str.simplified();
	//_str = removeComments(_str);

	// проверка на соответствие строки макроопределению LUSI
	QRegExp rx("^(@section|@output|@parameter|@proc|@comm|@condition|@argument)\\s+.*$");	
	int pos = 0;
	if ((pos = rx.indexIn(_str, pos)) != -1)    
	{
		QString m = rx.capturedTexts().first();
		if (m.length() != _str.length()) return false;
	}
	else return false;	

	// поиск левой части макроопределения (до знака равно и после @section, @proc и пр.)
	int s1 = _str.indexOf("{");
	int s2 = _str.indexOf("=");
	if (s1 > s2 || (s1 < 0 && s2 > 5))
	{
		QStringList p1 = _str.split("=");
		if (p1.count() < 2) 
		{
			_e = tr("Cannot find object name or list of values.");
			return false;
		}
		
		QRegExp rx_var("(^_+[a-zA-Z]+\\w*[^_]{1}$)|([a-zA-Z]+\\w*[^_]{1}$)");

		QString left_part = p1.takeFirst();
		QString right_part = p1.join("=");
		if (_str.indexOf("@section") == 0) 
		{			
			QString def_name = left_part.split("@section").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::Section;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong section name!");
				return false;
			}
		}
		else if (_str.indexOf("@output") == 0) 
		{			
			QString def_name = left_part.split("@output").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::Output;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong output section name!");
				return false;
			}
		}
		else if (_str.indexOf("@parameter") == 0) 
		{
			QString def_name = left_part.split("@parameter").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::Parameter;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong parameter name!");
				return false;
			}			
		}
		else if (_str.indexOf("@proc") == 0) 
		{
			QString def_name = left_part.split("@proc").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::ProcPackage;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong processing package name!");
				return false;
			}			
		}
		else if (_str.indexOf("@comm") == 0) 
		{
			QString def_name = left_part.split("@comm").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::ComProgram;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong FPGA program name!");
				return false;
			}			
		}
		else if (_str.indexOf("@condition") == 0) 
		{
			QString def_name = left_part.split("@condition").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::Condition;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong Condition name!");
				return false;
			}
		}
		else if (_str.indexOf("@argument") == 0) 
		{
			QString def_name = left_part.split("@argument").last().simplified();
			if ((pos = rx_var.indexIn(def_name,0)) != -1)
			{
				_def.type = Definition::Argument;
				_def.name = def_name;
			}
			else
			{
				_e = tr("Wrong Argument name!");
				return false;
			}			
		}
		else 
		{
			_e = tr("Unknown definition type!");
			return false;
		}
	}
	else 
	{
		_e = tr("Cannot determine a definition type!");
		return false;
	}

	// поиск правой части макроопределения после знака равно и внутри {...}
	int s3 = _str.lastIndexOf("}");
	QString d1 = _str.mid(s1+1, s3-s1-1).simplified();

	// разбор на компоненты макроопределения внутри { ... }  
	// Пример: { 12.0, min=0.0, name="NMR Tool" } будет разобрано так: QStringList() << "12.0" << "min=0.0" << "name="NMR Tool""
	// Затем найденные компоненты будут представлены в виде ("имя поля" - "значение")
	QStringList operand_list = LUSI::separate(d1, ",", "\"'");	
	QRegExp rx1("(^'[^']+')|^(\"[^\"]+\")");							// поиск выражений типа "Name" или 'Name'
	QRegExp rx2("\\w+\\s?=\\s?('[^']+')|(\\w+=\"[^\"]+\")");			// поиск выражений типа name = "Mutumba" или name = 'Mutumba'
	QRegExp rx3("\\w+\\s?=\\s?[-+]?[0-9]*.?[0-9]+([eE][-+]?[0-9]+)?");	// поиск выражений типа value = 12.0
	for (int i = 0; i < operand_list.count(); i++)
	{ 
		QPair<QString,QString> operand;
		QString operand_str = operand_list[i].trimmed();
						
		bool ok;
		double v = operand_str.toDouble(&ok);
		if (ok) 
		{
			_def.fields << QPair<QString,QString>("", operand_str);			
		}
		else
		{
			if (rx1.indexIn(operand_str, 0) != -1)  
			{		
				QString m = rx1.capturedTexts().first();
				if (m == operand_str) 
				{					
					//int len = operand_str.length();
					_def.fields << QPair<QString,QString>("", operand_str); //operand_str.mid(1,len-2) - чтоб без кавычек и апострофов на концах
				}				
			}
			else if (rx2.indexIn(operand_str, 0) != -1)
			{
				QString m = rx2.capturedTexts().first();
				if (m == operand_str) 
				{		
					QStringList lst = operand_str.split("=");
					QString operand_var = "";
					QString operand_value = "";
					if (!lst.isEmpty()) operand_var = lst.takeAt(0).trimmed();
					operand_value = lst.join("=").trimmed();
					
					//int len = operand_value.length();
					_def.fields << QPair<QString,QString>(operand_var, operand_value);	// operand_value.mid(1,len-2)
				}
			}
			else if (rx3.indexIn(operand_str, 0) != -1)
			{
				QString m = rx3.capturedTexts().first();
				if (m == operand_str) 
				{		
					QStringList lst = operand_str.split("=");
					QString operand_var = "";
					QString operand_value = "";
					if (lst.count() == 2) 
					{
						operand_var = lst[0].trimmed();
						operand_value = lst[1].trimmed();
					}
					v = operand_value.toDouble(&ok);
					if (ok) 
					{
						_def.fields << QPair<QString,QString>(operand_var, operand_value);			
					}					
				}
			}
			else
			{
				_e = tr("Cannot parse ' %1 '!").arg(operand_str);
				return false;
			}
		}
	}			
	
	return true;
}

bool LUSI::Engine::findLUSIMacros(QString &_str, QString &_e)
{
	QRegExp rx_bgn("^#begin([^)]+).*$");
	int pos = rx_bgn.indexIn(_str, 0);
	if (pos >= 0)
	{
		QString block_name = _str.mid(pos).split(")").first().split("#begin(").last().trimmed();
		if (!block_name.isEmpty())
		{
			for (int i = 0; i < obj_list.count(); i++)
			{
				if (obj_list[i]->getType() == Definition::ProcPackage && obj_list[i]->getObjName() == block_name) 
				{
					started_package = block_name;		
					package_was_started = true;
					return true;
				}
				else if (obj_list[i]->getType() == Definition::ComProgram && obj_list[i]->getObjName() == block_name) 
				{
					started_comprg = block_name;		
					comprg_was_started = true;
					return true;
				}
			}
		}
		else 
		{
			_e = "Wrong object name was found!";
			return false;
		}
	}
	else if (_str.contains("#end"))
	{
		package_was_started = false;
		started_package = "";
		comprg_was_started = false;
		started_comprg = "";

		return true;
	}

	return false;
}


void LUSI::Engine::clear()
{	
	lusi_script = "";
	js_script = "";
	if (!obj_list.isEmpty())
	{
		qDeleteAll(obj_list.begin(), obj_list.end());
		obj_list.clear();
	}
	started_package = "";
	package_was_started = false;
	started_comprg = "";
	comprg_was_started = "";

	error_list.clear();
}

void LUSI::Engine::reset()
{
	for (int i = 0; i < obj_list.count(); i++)
	{
		LUSI::Object *lusi_obj = obj_list[i];
		switch (lusi_obj->getType())
			{
		case LUSI::Definition::ComProgram:
			{
				LUSI::COMProgram *comprg_obj = qobject_cast<LUSI::COMProgram*>(lusi_obj);
				if (comprg_obj) comprg_obj->clear();
				break;
			}
		case LUSI::Definition::ProcPackage:
			{
				LUSI::ProcPackage *procpack_obj = qobject_cast<LUSI::ProcPackage*>(lusi_obj);
				if (procpack_obj) procpack_obj->clear();
				break;
			}
		default: break;
		}
	}
}


//QRegExp rx_float("(^[+-]?\d*[.]?\d*[eE]?[+-]?\d+$)");						// регулярное выражение для поиска чисел с плавающей точкой
//QRegExp rx_var("(^_+[a-zA-Z]+\w*[^_]{1}$)|([a-zA-Z]+\w*[^_]{1}$)");		// регулярное выражение для поиска имен переменных
bool LUSI::Engine::evaluate(QStringList &_elist)
{
	qDeleteAll(obj_list.begin(), obj_list.end());
	obj_list.clear();

	DefinitionList definitions;		
	startLusiing(lusi_script, _elist, definitions, js_script);
	error_list = _elist;
	if (_elist.isEmpty()) return true;
	else return false;	
}

void LUSI::Engine::startLusiing(QString _script, QStringList &_elist, DefinitionList &definitions, QString &_js_script)
{
	_elist.clear();

	_script = removeComments(_script);
	QStringList str_lines = _script.split("\n");
	QStringList js_script_list;
	
	LUSI::Main *seqMain = new LUSI::Main("main");
	QScriptValue objectValue = qscript_engine->newQObject(seqMain);
	qscript_engine->globalObject().setProperty(seqMain->getObjName(), objectValue);
	obj_list.append(seqMain);

	int str_count = 0;
	int group_index = 1;
	int pack_id = 1;
	while (str_count < str_lines.count())
	{
		QString txt = str_lines[str_count];

		// Find postfix "~" and replace with ".value"
		int pos = -1;
		while ((pos = txt.indexOf("~")) >= 0)
		{
			txt.replace(pos, 1, ".getValue()");
		}

		// Find LUSI macrodefinitions "@..." and create the appropriate objects
		LUSI::Definition lusi_def;
		QString e = "";
		if (findLUSIDefinition(txt, lusi_def, e))
		{
			definitions.append(lusi_def);
			txt = QString("/* %1 */").arg(txt);  

			QStringPairList field_list = lusi_def.fields;
			Definition::Type def_type = lusi_def.type;
			QString def_name = lusi_def.name;

			switch (def_type)
			{
			case LUSI::Definition::Section:
				{
					LUSI::Section *seqSection = new LUSI::Section(def_name);
					
					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqSection->setField(field.second, cnt++);
						else seqSection->setField(field.first, field.second);
					}

					QScriptValue objectValue = qscript_engine->newQObject(seqSection);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqSection);
					break;
				}	
			case LUSI::Definition::Output:
				{
					LUSI::Output *seqOutput = new LUSI::Output(def_name);

					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqOutput->setField(field.second, cnt++);
						else seqOutput->setField(field.first, field.second);
					}

					QScriptValue objectValue = qscript_engine->newQObject(seqOutput);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqOutput);
					break;
				}	
			case LUSI::Definition::Parameter:
				{
					LUSI::Parameter *seqParam = new LUSI::Parameter(def_name);

					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqParam->setField(field.second, cnt++);
						else seqParam->setField(field.first, field.second);
					}

					QScriptValue objectValue = qscript_engine->newQObject(seqParam);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqParam);
					break;
				}
			case LUSI::Definition::ProcPackage:
				{
					LUSI::ProcPackage *seqProcPackage = new LUSI::ProcPackage(def_name, instr_list);

					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqProcPackage->setField(field.second, cnt++);
						else seqProcPackage->setField(field.first, field.second);
					}
					seqProcPackage->setId(pack_id++);

					QScriptValue objectValue = qscript_engine->newQObject(seqProcPackage);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqProcPackage);
					break;
				}
			case LUSI::Definition::ComProgram:
				{
					LUSI::COMProgram *seqCOMProgram = new LUSI::COMProgram(def_name, cmd_list);

					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqCOMProgram->setField(field.second, cnt++);
						else seqCOMProgram->setField(field.first, field.second);
					}

					QScriptValue objectValue = qscript_engine->newQObject(seqCOMProgram);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqCOMProgram);
					break;
				}
			case LUSI::Definition::Condition:
				{
					LUSI::Condition *seqCondition = new LUSI::Condition(def_name);

					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqCondition->setField(field.second, cnt++);
						else seqCondition->setField(field.first, field.second);
					}

					QScriptValue objectValue = qscript_engine->newQObject(seqCondition);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqCondition);
					break;
				}
			case LUSI::Definition::Argument:
				{
					LUSI::Argument *seqArg = new LUSI::Argument(def_name);

					int cnt = 0;
					for (int i = 0; i < field_list.count(); i++)
					{
						QPair<QString, QString> field = field_list[i];
						if (field.first.isEmpty()) seqArg->setField(field.second, cnt++);
						else seqArg->setField(field.first, field.second);
					}
					seqArg->setGroupIndex(group_index++);

					QScriptValue objectValue = qscript_engine->newQObject(seqArg);
					qscript_engine->globalObject().setProperty(def_name, objectValue);

					obj_list.append(seqArg);
					break;
				}
			}
		}
		else if (!e.isEmpty())
		{
			_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
		}

		// find LUSI macro '#begin(...)' or '#end'
		bool just_started = false;
		if (findLUSIMacros(txt, e))
		{
			txt = QString("/* %1 */").arg(txt);  
			just_started = true;
		}
		else if (!e.isEmpty())
		{
			_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
		}

		// if a processing package was started (#begin(packName) was found) 
		if (package_was_started && !started_package.isEmpty() && !just_started)
		{
			QString instr = txt.split("(").first().trimmed();
			if (!instr.isEmpty() && instr.mid(0,4) == "INS_")
			{
				bool code = 0;
				findInstrCode(instr, code);
				if (code)
				{
					QStringList param_list;
					param_list = txt.split(instr+"(").last().split(")").first().split(",");
					if (param_list.count() > 1)
					{
						bool ok;
						int num = param_list[0].trimmed().toInt(&ok);
						if (ok) 
						{
							if (param_list.count() == num + 2)
							{
								QStringList params;

								params.append(param_list[0].trimmed());

								int type = param_list[1].trimmed().toInt(&ok);
								if (ok) params.append(param_list[1].trimmed());
								//else if (qscript_engine->globalObject().property(param_list[1].trimmed()).isValid()) 
								else if (qscript_engine->globalObject().property(param_list[1].trimmed()).isObject())
								{
									params.append(param_list[1].trimmed() + ".getAppValue()");
								}
								else
								{
									// wrong data type !
									//e = tr("Wrong data type!");
									//_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
									//break;
									params.append(param_list[1].trimmed());
								}
								
								for (int i = 0; i < num; i++)
								{
									//if (qscript_engine->globalObject().property(param_list[2+i].trimmed()).isValid()) 
									if (qscript_engine->globalObject().property(param_list[2+i].trimmed()).isObject()) 
									{
										params.append(param_list[2+i].trimmed() + ".getAppValue()");
									}
									else params.append(param_list[2+i].trimmed());									
								}
								txt = QString("%1.exec(['%2',%3])\t\t // %4").arg(started_package).arg(instr).arg(params.join(", ")).arg(txt);
							}
							else
							{
								// Wrong number of parameters in function!
								e = tr("Wrong number of parameters in function '%1'!").arg(instr);
								_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
								//break;
							}
						}
						else
						{
							// cannot read the number of function parameters!
							e = tr("Cannot read the number of function parameters!");
							_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
							//break;
						}
					}
					else 
					{
						// Wrong number of parameters in function!
						e = tr("Wrong number of parameters in function '%1'!").arg(instr);
						_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
						//break;
					}
				}
				else
				{
					// Wrong number of parameters in function!
					e = tr("Unknown instruction '%1'!").arg(instr);
					_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
					//break;
				}
			}			
		}
		// if a FPGA command program was started (#begin(comPrgName) was found) 
		else if (comprg_was_started && !started_comprg.isEmpty() && !just_started)
		{
			QStringList param_list = txt.simplified().split(" ");
			QString comm = param_list.takeFirst().trimmed();				//txt.split("(").first().trimmed();
			if (!comm.isEmpty() && comm.mid(0,4) == "COM_")
			{				
				bool code = 0;
				findCmdCode(comm, code);
				if (code)
				{
					param_list = param_list.join("").split(",");					
					
					QStringList params;
					if (param_list.count() == 1)
					{
						bool _ok;						
						//if (qscript_engine->globalObject().property(param_list.first().trimmed()).isValid()) 
						if (qscript_engine->globalObject().property(param_list.first().trimmed()).isObject()) 
						{
							params.append(param_list.first().trimmed() + ".getAppValue()");
						}
						else params.append(param_list.first().trimmed());	

						txt = QString("%1.exec(['%2',%3])\t\t // %4").arg(started_comprg).arg(comm).arg(params.join(", ")).arg(txt);
					}
					else if (param_list.count() == 3)
					{
						for (int i = 0; i < param_list.count(); i++)
						{
							//if (qscript_engine->globalObject().property(param_list[i].trimmed()).isValid()) 
							if (qscript_engine->globalObject().property(param_list[i].trimmed()).isObject()) 
							{
								params.append(param_list[i].trimmed() + ".getAppValue()");
							}
							else params.append(param_list[i].trimmed());									
						}

						txt = QString("%1.exec(['%2',%3])\t\t // %4").arg(started_comprg).arg(comm).arg(params.join(", ")).arg(txt);
					}
					else
					{
						// Wrong number of parameters in function!
						e = tr("Wrong number of parameters in FPGA command '%1'!").arg(comm);
						_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
						//break;
					}
				}
				else
				{
					// Wrong number of parameters in function!
					e = tr("Unknown FPGA command '%1'!").arg(comm);
					_elist << tr("<font color=red>LUSI: Error in line %1 : </font>").arg(str_count+1) + e;
					//break;
				}
			}			
		}

		js_script_list.append(txt);

		str_count++;
	}

	_js_script = js_script_list.join("\n");
}

/*bool LUSI::Engine::getXVector(QString arg_name, int size, QVector<double> &xvec)
{
	xvec.clear();
	bool res = false;

	for (int i = 0; i < obj_list.count(); i++)
	{
		Object *lusi_obj = obj_list[i];
		if (lusi_obj->getType() == Definition::Argument)
		{
			if (LUSI::Argument *arg_obj = qobject_cast<LUSI::Argument*>(lusi_obj))
			{
				if (arg_obj->getObjName() == arg_name)
				{
					QString func_name = arg_obj->getFunc();
					
					QScriptValue func = qscript_engine->globalObject().property(func_name);
					if (func.isValid())
					{
						QScriptValue qval = func.call(QScriptValue(), QScriptValueList() << size);
						if (qval.isValid())
						{							
							QVariantList var_list = qval.toVariant().toList();							
							xvec.resize(var_list.count());
							bool _ok;
							for (int j = 0; j < var_list.count(); j++)
							{
								xvec[j] = var_list[j].toDouble(&_ok);
							}
							res = true;
						}						
					}					
				}
			}
		}
	}
	//QScriptValue add = qscript_engine->globalObject().property("add");
	//qDebug() << add.call(QScriptValue(), QScriptValueList() << 1 << 2).toNumber(); // 3

	return res;
}*/

LUSI::Sequence::Sequence()
{
	name = "";
	author = "";
	description = "";
	datetime = "";

	js_script = "";

	file_path = "";
	file_name = "";

	comprg_errors.clear();
	procdsp_errors.clear();
	seq_errors.clear();
	cond_errors.clear();
	js_error = "";

	main_object = NULL;
}

LUSI::Sequence::Sequence(ObjectList *_obj_list, QString _js_script, QStringList _elist)
{			
	js_script = _js_script;

	seq_errors = _elist;

	for (int i = 0; i < _obj_list->size(); i++)
	{
		LUSI::Object *lusi_obj = qobject_cast<LUSI::Object*>(_obj_list->at(i));
		seq_errors << lusi_obj->getErrorList();

		LUSI::Definition::Type obj_type = lusi_obj->getType();
		switch (obj_type)
		{
		case LUSI::Definition::ProcPackage:
			{
				LUSI::ProcPackage *obj = qobject_cast<LUSI::ProcPackage*>(_obj_list->at(i));
				QByteVector byte_code;
				byte_code << obj->getProcProgram();
				proc_programs.append(byte_code);				
				if (!obj->getErrorList().isEmpty()) procdsp_errors << obj->getErrorList();
				break;
			}
		case LUSI::Definition::ComProgram:
			{
				LUSI::COMProgram *obj = qobject_cast<LUSI::COMProgram*>(_obj_list->at(i));
				QByteVector byte_code;
				byte_code << obj->getComProgram();
				com_programs.append(byte_code);				
				if (!obj->getErrorList().isEmpty()) comprg_errors << obj->getErrorList();
				break;
			}				
		case LUSI::Definition::Section:
			{
				LUSI::Section *s_obj = qobject_cast<LUSI::Section*>(_obj_list->at(i));
				section_list.append(s_obj);
				if (!s_obj->getErrorList().isEmpty()) seq_errors << s_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Output:
			{
				LUSI::Output *o_obj = qobject_cast<LUSI::Output*>(_obj_list->at(i));
				output_list.append(o_obj);
				if (!o_obj->getErrorList().isEmpty()) seq_errors << o_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Parameter:
			{
				LUSI::Parameter *p_obj = qobject_cast<LUSI::Parameter*>(_obj_list->at(i));
				param_list.append(p_obj);
				if (!p_obj->getErrorList().isEmpty()) seq_errors << p_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Argument:
			{
				LUSI::Argument *a_obj = qobject_cast<LUSI::Argument*>(_obj_list->at(i));
				arg_list.append(a_obj);
				if (!a_obj->getErrorList().isEmpty()) seq_errors << a_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Condition:
			{
				LUSI::Condition *c_obj = qobject_cast<LUSI::Condition*>(_obj_list->at(i));
				cond_list.append(c_obj);
				if (!c_obj->getErrorList().isEmpty()) cond_errors << c_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Main:
			{
				LUSI::Main *m_obj = qobject_cast<LUSI::Main*>(_obj_list->at(i));
				main_object = m_obj;
				if (!m_obj->getErrorList().isEmpty()) seq_errors << m_obj->getErrorList();

				name = main_object->getName();
				author = main_object->getAuthor();
				description = main_object->getDescription();
				datetime = main_object->getDateTime();
				break;						
			}
		}
	}
}

void LUSI::Sequence::setObjects(LUSI::ObjectList *_obj_list)
{
	if (!_obj_list) return;
	
	for (int i = 0; i < _obj_list->size(); i++)
	{
		LUSI::Object *lusi_obj = qobject_cast<LUSI::Object*>(_obj_list->at(i));
		seq_errors << lusi_obj->getErrorList();

		LUSI::Definition::Type obj_type = lusi_obj->getType();
		switch (obj_type)
		{
		case LUSI::Definition::ProcPackage:
			{
				LUSI::ProcPackage *obj = qobject_cast<LUSI::ProcPackage*>(_obj_list->at(i));				
				QByteVector byte_code;
				byte_code << obj->getProcProgram();
				proc_programs.append(byte_code);		
				procdsp_list.append(obj);
				if (!obj->getErrorList().isEmpty()) procdsp_errors << obj->getErrorList();
				break;
			}
		case LUSI::Definition::ComProgram:
			{
				LUSI::COMProgram *obj = qobject_cast<LUSI::COMProgram*>(_obj_list->at(i));
				QByteVector byte_code;
				byte_code << obj->getComProgram();
				com_programs.append(byte_code);		
				comprg_list.append(obj);
				if (!obj->getErrorList().isEmpty()) comprg_errors << obj->getErrorList();
				break;
			}				
		case LUSI::Definition::Section:
			{
				LUSI::Section *s_obj = qobject_cast<LUSI::Section*>(_obj_list->at(i));
				section_list.append(s_obj);
				if (!s_obj->getErrorList().isEmpty()) seq_errors << s_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Output:
			{
				LUSI::Output *o_obj = qobject_cast<LUSI::Output*>(_obj_list->at(i));
				output_list.append(o_obj);
				if (!o_obj->getErrorList().isEmpty()) seq_errors << o_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Parameter:
			{
				LUSI::Parameter *p_obj = qobject_cast<LUSI::Parameter*>(_obj_list->at(i));
				param_list.append(p_obj);
				if (!p_obj->getErrorList().isEmpty()) seq_errors << p_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Argument:
			{
				LUSI::Argument *a_obj = qobject_cast<LUSI::Argument*>(_obj_list->at(i));
				arg_list.append(a_obj);
				if (!a_obj->getErrorList().isEmpty()) seq_errors << a_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Condition:
			{
				LUSI::Condition *c_obj = qobject_cast<LUSI::Condition*>(_obj_list->at(i));
				cond_list.append(c_obj);
				if (!c_obj->getErrorList().isEmpty()) cond_errors << c_obj->getErrorList();
				break;						
			}
		case LUSI::Definition::Main:
			{
				LUSI::Main *m_obj = qobject_cast<LUSI::Main*>(_obj_list->at(i));
				main_object = m_obj;
				if (!m_obj->getErrorList().isEmpty()) seq_errors << m_obj->getErrorList();

				name = main_object->getName();
				author = main_object->getAuthor();
				description = main_object->getDescription();
				datetime = main_object->getDateTime();
				break;						
			}
		}
	}
}

QList<QVariantList> LUSI::Sequence::getVarComProgram(int index)
{
	LUSI::COMProgram *obj = comprg_list[index];	
	return obj->getVarComProgram();	
}

QList<QVariantList> LUSI::Sequence::getVarProcProgram(int index)
{	
	LUSI::ProcPackage *obj = procdsp_list[index];
	return obj->getVarProcProgram();	
}

LUSI::Argument* LUSI::Sequence::getArgument(int _group_index)
{
	if (arg_list.isEmpty()) return NULL;

	for (int i = 0; i < arg_list.count(); i++)
	{
		LUSI::Argument *arg_obj = arg_list[i];
		if (arg_obj->getAppValue() == _group_index) return arg_obj;
	}

	return NULL;
}

void LUSI::Sequence::clear()
{
	name = "";
	author = "";
	datetime = "";
	description = "";

	js_script = "";
	seq_errors.clear();

	com_programs.clear();
	proc_programs.clear();	
	comprg_list.clear();
	procdsp_list.clear();
	
	param_list.clear();
	section_list.clear();
	arg_list.clear();
	cond_list.clear();
	output_list.clear();
		
	comprg_errors.clear();
	procdsp_errors.clear();
	cond_errors.clear();
	js_error.clear();
	
	main_object = NULL;
}

void LUSI::Sequence::reset()
{
	com_programs.clear();
	proc_programs.clear();
	comprg_list.clear();
	procdsp_list.clear();	
	js_error.clear();

	for (int i = 0; i < section_list.count(); i++)
	{
		LUSI::Section *sec = section_list[i];
		sec->clearParameters();
	}
}