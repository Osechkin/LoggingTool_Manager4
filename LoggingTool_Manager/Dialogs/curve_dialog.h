#ifndef CURVE_DIALOG_H
#define CURVE_DIALOG_H

#include <QtGui>
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include "../GraphClasses/plotted_objects.h"

#include "ui_curve_dialog.h"


class CurveDialog : public QDialog, public Ui::CurveDialog
{
	Q_OBJECT

public:
	//explicit CurveDialog(QList<QwtPlotCurve*> *_curve_list, QwtCurveSettings *_def_curve_settings, bool _def_curve, QWidget *parent = 0);
	//explicit CurveDialog(QList<QwtPlotCurve*> *_curve_list, QWidget *parent = 0);
	explicit CurveDialog(QList<PlotCurve> &_curve_list, QWidget *parent = 0);
	~CurveDialog();
		
	void setCurveTitleEditable(bool flag) { cboxCurve->setEditable(flag); }
	bool isCurveTitleEditable() { return cboxCurve->isEditable(); }

	QList<QwtCurveSettings*> *getCurveSettingsList() { return curve_settings_list; }	

private:
	void initSampleCurve();
	void refreshSampleCurve();
	void setConnections();
	Qt::PenStyle getLineStyle(const QString str);
	QString getLineStyleStr(const Qt::PenStyle style);
	QwtSymbol::Style getSymbolStyle(const QString str);
	QString getSymbolStyleStr(const QwtSymbol::Style style);
	
	QList<QwtPlotCurve*> *curve_list;

	QList<QwtCurveSettings*> *curve_settings_list;	
	int curve_index;
	QwtSymbol *symbol;
	QPen pen;
	QwtPlotCurve *sample_curve;

private slots:
	void changeCurrentCurve(QString);
	//void changeCurrentCurve(QwtPlotCurve *curve);
	void changeCurrentCurve(QwtCurveSettings *curve_settings);
	//void setDefaultCurveSettings(QwtCurveSettings *_def_curve_settings);
	void changeLineColor();
	void changeLineType(QString);
	void changeLineWidth(int);
	void enableLine(bool);
	void enableSymbol(bool);
	void changeSymbolEdgeWidth(int);
	void changeSymbolEdgeColor();
	void changeSymbolType(QString);
	void changeSymbolSize(int);
	void changeSymbolFillColor();
	//void changeCurveSettingsMode(int state);
};

#endif // CURVE_DIALOG_H
