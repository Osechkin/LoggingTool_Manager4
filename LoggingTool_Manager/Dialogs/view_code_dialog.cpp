#include <QPushButton>
#include <QMessageBox>

#include "view_code_dialog.h"


ViewCodeDialog::ViewCodeDialog(LUSI::Sequence *seq, QWidget *parent)
{
	setupUi(this);
	this->setParent(parent);

	this->setWindowTitle(tr("View Code"));

	tbtHex->setIcon(QIcon(":/images/HEX.png"));
	tbtDec->setIcon(QIcon(":/images/DEC.png"));
	tbtByteCode->setIcon(QIcon(":/images/List.png"));

	tbtDec->setChecked(true);

	this->lusi_seq = seq;

	mode = Dec;	
	view_bytecode = false;
	showLUSISeqProgram();

	QPushButton * bOK = buttonBox->button(QDialogButtonBox::Ok);
	connect(bOK, SIGNAL(clicked()), this, SLOT(close()));
	connect(tbtDec, SIGNAL(clicked(bool)), this, SLOT(setDecMode(bool)));
	connect(tbtHex, SIGNAL(clicked(bool)), this, SLOT(setHexMode(bool)));	
	connect(tbtByteCode, SIGNAL(clicked(bool)), this, SLOT(viewByteCode(bool)));
}

void ViewCodeDialog::setMode(Mode _mode)
{
	mode = _mode;
	showLUSISeqProgram();
}

void ViewCodeDialog::showLUSISeqProgram()
{
	tedProgram->clear();
	for (int i = 0; i < lusi_seq->comprg_list.count(); i++)
	{
		LUSI::COMProgram *com_prg = lusi_seq->comprg_list[i];
		QString com_prg_name = com_prg->getTitle();
		
		QList<QVariantList> var_com_prg = lusi_seq->getVarComProgram(i);
		QString prg_str = comPrgToString(i);
		QString bytecode_str = "";
		if (view_bytecode) bytecode_str = comPrgToByteCodeString(i);
		//tedProgram->clear();

		QString str_ = "";
		if (view_bytecode) str_ = tr("<b>Byte-code:</b>");
		QString table_header = QString("<table><tr bgcolor=""#DDA0DD""><td><b>Sequence Commands ('%1'):</b></td><td>%2</td></tr>").arg(com_prg_name).arg(str_); 
		prg_str = "<tr><td>" + prg_str + "</td>" + "<td>" + bytecode_str + "</td>" + "</tr>";
		QString table_footer = "</table>";
		prg_str = table_header + prg_str + table_footer;
		tedProgram->insertHtml(prg_str);
	}
		
	tedProcessing->clear();
	for (int i = 0; i < lusi_seq->procdsp_list.count(); i++)
	{
		LUSI::ProcPackage *dsp_prg = lusi_seq->procdsp_list[i];
		QString proc_dsp_name = dsp_prg->getTitle();
		QString proc_dsp_id = QString::number(dsp_prg->getAppValue());

		//QList<QVariantList> var_dsp_prg = lusi_seq->getVarProcProgram(i);
		QString prg_str = procPrgToString(i);
		QString bytecode_str = "";
		if (view_bytecode) bytecode_str = procPrgToByteCodeString(i);
		//tedProcessing->clear();

		QString str_ = "";
		if (view_bytecode) str_ = tr("<b>Byte-code:</b>");
		QString table_header = QString("<table><tr bgcolor=""#DDA0DD""><td><b>Processing Package ('%1, id = %2'):</b></td><td>%3</td></tr>").arg(proc_dsp_name).arg(proc_dsp_id).arg(str_); 
		prg_str = "<tr><td>" + prg_str + "</td>" + "<td>" + bytecode_str + "</td>" + "</tr>";
		QString table_footer = "</table>";
		prg_str = table_header + prg_str + table_footer;
		tedProcessing->insertHtml(prg_str);
	}	
}


void ViewCodeDialog::setDecMode(bool flag)
{
	if (!flag) 
	{
		flag = true;	
		tbtDec->setChecked(flag);
		return;
	}

	tbtHex->setChecked(false);	
	setMode(Dec);	
}

void ViewCodeDialog::setHexMode(bool flag)
{
	if (!flag) 
	{
		flag = true;
		tbtHex->setChecked(flag);
		return;
	}

	tbtDec->setChecked(false);
	setMode(Hex);
}

void ViewCodeDialog::viewByteCode(bool flag)
{
	view_bytecode = flag;
	showLUSISeqProgram();
}


QString ViewCodeDialog::comPrgToString(int index)
{		
	QList<QVariantList> _prg = lusi_seq->getVarComProgram(index);

	int base = 10;
	QString pre = "";
	QString str_clr = "<font color=darkblue>";	
	if (mode == Hex) 
	{
		str_clr = "<font color=darkmagenta>";
		base = 16;
		pre = "0x";
	}

	QString num_clr = "<font color=grey>";
	QString mnem_clr = "<font color=darkgreen>";
	QString b234_clr = "<font color=blue>";
	QString comma_clr = "<font color=black>";
	
	QString res = "";
	QStringList bytes;
	for (int i = 0; i < _prg.count(); i++)
	{
		QVariantList _var = _prg[i];
		QString com_str = "";	
		QString byte_str = "";
		bool _ok;
		if (!_var.isEmpty()) com_str = _var.first().toString().toUpper();

		res += num_clr + QString::number(i+1) + ". </font>";
		res += mnem_clr + com_str + "</font>" + '\t';

		for (int j = 1; j < _var.count(); j++)
		{			
			uint8_t byte = _var[j].toUInt(&_ok);
			if (_ok) byte_str = QString::number(byte, base).toUpper();	
			if (j < _var.count()-1)
			{
				res += str_clr + pre + byte_str + "</font>" + comma_clr + ",</font>" + '\t';
			}
			else
			{
				res += str_clr + pre + byte_str + "</font>";
			}			
		}
		res += "<br>";
		bytes.clear();
	}
	
	return res;
}

QString ViewCodeDialog::comPrgToByteCodeString(int index)
{
	QByteVector _byte_vec = lusi_seq->com_programs[index];
	if (_byte_vec.count() % 4 > 0) return "";
	
	int base = 10;
	QString pre = "";
	QString str_clr = "<font color=darkblue>";	
	if (mode == Hex) 
	{
		str_clr = "<font color=darkmagenta>";
		base = 16;
		pre = "0x";
	}

	QString num_clr = "<font color=grey>";
	QString mnem_clr = "<font color=darkgreen>";
	QString comma_clr = "<font color=black>";
	
	QString res = "";	
	int cnt = 1;
	for (int i = 0; i < _byte_vec.count(); i += 4)
	{		
		QString com_str = QString::number(_byte_vec[i], base).toUpper();
		QString byte_str = "";
		
		res += num_clr + QString::number(cnt++) + ". </font>";
		res += mnem_clr + pre + com_str + "</font>, ";

		for (int j = i+1; j < i+4; j++)
		{			
			uint8_t byte = _byte_vec[j];
			byte_str = QString::number(byte, base).toUpper();	
			if (j % 4 < 3)
			{
				res += str_clr + pre + byte_str + "</font>" + comma_clr + ",</font>" + '\t';
			}
			else
			{
				res += str_clr + pre + byte_str + "</font>";
			}			
		}
		res += "<br>";		
	}
	
	return res;
}

QString ViewCodeDialog::procPrgToByteCodeString(int index)
{
	QByteVector _byte_vec = lusi_seq->proc_programs[index];
	
	int base = 10;
	QString pre = "";
	QString str_clr = "<font color=darkblue>";	
	QString len_clr = "<font color=darkblue>";
	QString type_clr = "<font color=darkblue>";
	if (mode == Hex) 
	{
		str_clr = "<font color=darkmagenta>";
		len_clr = "<font color=darkmagenta>";
		type_clr = "<font color=darkmagenta>";
		base = 16;
		pre = "0x";
	}

	QString num_clr = "<font color=grey>";
	QString mnem_clr = "<font color=darkgreen>";	
	QString comma_clr = "<font color=black>";

	QString res = "";	
	int cnt = 1;
	int i = 1;
	while (i < _byte_vec.count())
	{		
		QString instr_str = QString::number(_byte_vec[i++], base).toUpper();		
		res += num_clr + QString::number(cnt++) + ". </font>";
		res += mnem_clr + pre + instr_str + "</font>, ";
		
		int ins_len = _byte_vec[i++];
		QString len_str = QString::number(ins_len, base).toUpper();		
		res += len_clr + pre + len_str + "</font>, ";

		QString type_str = QString::number(_byte_vec[i++], base).toUpper();		
		if (ins_len == 0) res += type_clr + pre + type_str + "</font>";
		else res += type_clr + pre + type_str + "</font>, ";

		QString byte_str = "";
		int number_size = 4;	// 4 bytes for each value
		for (int j = 0; j < ins_len*number_size; j++)
		{			
			if ( (i+j) < _byte_vec.count() )  
			{
				uint8_t byte = _byte_vec[i+j];
				byte_str = QString::number(byte, base).toUpper();	
				if (j < number_size*ins_len-1)
				{
					res += str_clr + pre + byte_str + "</font>" + comma_clr + ",</font>" + '\t';
				}
				else
				{
					res += str_clr + pre + byte_str + "</font>";
				}
			}			 
		}
		i += number_size*ins_len;

		res += "<br>";		
	}

	return res;
}


QString ViewCodeDialog::procPrgToString(int index)
{
	QList<QVariantList> _prg = lusi_seq->getVarProcProgram(index);

	int base = 10;
	QString pre = "";
	QString str_clr = "<font color=darkblue>";	
	if (mode == Hex) 
	{
		str_clr = "<font color=darkmagenta>";
		base = 16;
		pre = "0x";
	}

	QString num_clr = "<font color=grey>";
	QString mnem_clr = "<font color=darkgreen>";	
	QString comma_clr = "<font color=black>";
	QString service_clr = "<font color=blue>";
	
	QString res = "";
	QStringList bytes;
	for (int i = 0; i < _prg.count(); i++)
	{
		QVariantList _var = _prg[i];
		QString instr_str = "";	
		QString byte_str = "";
		bool _ok;
		if (!_var.isEmpty()) instr_str = _var.first().toString().toUpper();

		res += num_clr + QString::number(i+1) + ". </font>";
		res += mnem_clr + instr_str + "</font>( ";

		res += service_clr + _var[1].toString() + "</font>" + comma_clr + ", </font>";
		res += service_clr + _var[2].toString() + "</font>";
		if (_var.count() == 3)
		{
			res += str_clr + pre + byte_str + "</font> )";
		}
		else
		{
			res += comma_clr + ", </font>";
			for (int j = 3; j < _var.count(); j++)
			{			
				int byte = _var[j].toInt(&_ok);
				if (_ok) byte_str = QString::number(byte, base).toUpper();	
				if (j < _var.count()-1)
				{
					res += str_clr + pre + byte_str + "</font>" + comma_clr + ",</font>" + '\t';
				}
				else
				{
					res += str_clr + pre + byte_str + "</font> )";
				}			
			}
		}
		
		res += "<br>";
		bytes.clear();
	}

	return res;	
}