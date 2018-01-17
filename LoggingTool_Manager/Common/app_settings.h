#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QVariant>
#include <QColor>
#include <QFont>
#include <QString>
#include <QDateTime>

#include "../io_general.h"

#include "qwt_plot_grid.h"
#include "qwt_scale_engine.h"

#include "qextserialport.h"


#define GET_STRING(x) #x


static QString toAlignedString(int len, double val)
{
	QString out = QString::number(val);
	
	int d = len - out.length();
	if (val < 0)
	{				
		if (d > 0) 
		{
			for (int i = 0; i < d; i++) out += "0";
		}
		else out = out.mid(0, len);
	}
	else
	{
		if (d > 0)
		{
			QString z_str = "";
			for (int i = 0; i < d; i++) z_str += "0";
			out = z_str + out;
		}		
	}

	return out;
}

static QString toAlignedSpacedString(int num, int str_len, QString sym)
{
	QString str_comm = QString("%1").arg(num);
	int str_comm_len = str_comm.length();
	str_comm = sym.repeated(str_len).replace(0,str_comm_len, str_comm);

	return str_comm;
}

static bool QStringToBool(const QString &str)
{
	bool res = false;
	if (str.toLower() == "true") res = true;

	return res;
}

static QVariant QColorToQVar(const QColor &color)
{
	QString r = QString::number(color.red());	
	QString g = QString::number(color.green());
	QString b = QString::number(color.blue());
	QString a = QString::number(color.alpha());

	return QVariant(r + "/" + g + "/" + b + "/" + a);
}

static QColor QVarToQColor(const QVariant &v)
{
	QStringList c_list = v.toString().split("/");
	if (c_list.count() != 4) return QColor();

	return QColor(c_list[0].toInt(), c_list[1].toInt(), c_list[2].toInt(), c_list[3].toInt());
}

static QVariant QFontToQVar(const QFont &font)
{
	QString fm = font.family();
	QString sz = QString::number(font.pointSize());
	
	QString res = fm + "/" + sz;
	if (font.bold()) res += "/bold"; 
	if (font.italic()) res += "/italic";
	if (font.underline()) res += "/underline";

	return QVariant(res);
}

static QFont QVarToQFont(const QVariant &v)
{
	QStringList f_list = v.toString().split("/");
	if (f_list.count() < 2) return QFont();

	QFont font;
	font.setFamily(f_list[0]);
	font.setPointSize(f_list[1].toInt());
	if (f_list.count() > 2) font.setBold(QStringToBool(f_list[2]));
	if (f_list.count() > 3) font.setItalic(QStringToBool(f_list[3]));
	if (f_list.count() > 4) font.setUnderline(QStringToBool(f_list[4]));

	return font;
}

static QVariant QtPenStyleToQVar(const QPen &pen)
{
	Qt::PenStyle pen_style = pen.style();
	QString res = "";
	switch (pen_style)
	{
	case Qt::SolidLine:		 res = "SolidLine"; break;
	case Qt::DashLine:		 res = "DashLine"; break;		
	case Qt::DotLine:		 res = "DotLine"; break;		
	case Qt::DashDotLine:	 res = "DashDotLine"; break;		
	case Qt::DashDotDotLine: res = "DashDotDotLine"; break;		
	default: break;	
	}

	return QVariant(res);
}

static Qt::PenStyle QVarToQtPenStyle(const QVariant &v)
{
	QString str_style = v.toString();
	if (str_style == "SolidLine")			return Qt::SolidLine;
	else if (str_style == "DashLine")		return Qt::DashLine;
	else if (str_style == "DotLine")		return Qt::DotLine;
	else if (str_style == "DashDotLine")	return Qt::DashDotLine;
	else if (str_style == "DashDotDotLine") return Qt::DashDotDotLine;
	
	return Qt::NoPen;
}


static QVariant ScaleToQVar(QwtScaleEngine *engine)
{
	bool ret = (dynamic_cast<QwtLinearScaleEngine*>(engine));
	if (ret == 0) return QVariant("Log10_Scale");
	else return QVariant("Linear_Scale");
}

static QString toString(const ParityType parity_type)
{
	switch (parity_type)
	{
	case ParityType::PAR_NONE:		return GET_STRING(PAR_NONE);
	case ParityType::PAR_EVEN:		return GET_STRING(PAR_EVEN);
	case ParityType::PAR_ODD:		return GET_STRING(PAR_ODD);
	case ParityType::PAR_MARK:		return GET_STRING(PAR_MARK);
	case ParityType::PAR_SPACE:		return GET_STRING(PAR_SPACE);
	default: break;
	}

	return QString("");
}

static QString toString(const StopBitsType stop_bits)
{
	switch (stop_bits)
	{
	case StopBitsType::STOP_1:		return GET_STRING(STOP_1);
	case StopBitsType::STOP_1_5:	return GET_STRING(STOP_1_5);
	case StopBitsType::STOP_2:		return GET_STRING(STOP_2);	
	default: break;
	}

	return QString("");
}

static QString toString(const FlowType flow_type)
{
	switch (flow_type)
	{
	case FlowType::FLOW_OFF:		return GET_STRING(FLOW_OFF);
	case FlowType::FLOW_HARDWARE:	return GET_STRING(FLOW_HARDWARE);
	case FlowType::FLOW_XONXOFF:	return GET_STRING(FLOW_XONXOFF);	
	default: break;
	}

	return QString("");
}

#endif // APP_SETTINGS