#include <QColorDialog>

#include "curve_dialog.h"
#include "../GraphClasses/plotted_objects.h"
#include "../GraphClasses/graph_classes.h"


//CurveDialog::CurveDialog(QList<QwtPlotCurve*> *_curve_list, QWidget *parent)
CurveDialog::CurveDialog(QList<PlotCurve> &_curve_list, QWidget *parent)
{
	this->setupUi(this);
	this->setParent(parent);
	this->setWindowTitle(tr("Curve Settings Dialog"));

	//curve_list = _curve_list;
	curve_list = new QList<QwtPlotCurve*>;
	for (int i = 0; i < _curve_list.count(); i++)
	{
		if (_curve_list[i].ds->isHeldOn())
		{
			curve_list->append(_curve_list[i].curve);
		}
	}
	
	qwtSample->enableAxis(QwtPlot::yLeft, false);
	qwtSample->enableAxis(QwtPlot::yRight, false);
	qwtSample->enableAxis(QwtPlot::xBottom, false);
	qwtSample->enableAxis(QwtPlot::xTop, false);

	qwtSample->setCanvasBackground(QBrush(QColor(Qt::white)));

	tbtSymbolEdgeColor->setIcon(QIcon(":/images/color.png"));
	tbtSymbolColor->setIcon(QIcon(":/images/color.png"));
	tbtLineColor->setIcon(QIcon(":/images/color.png"));

	QStringList symbol_list;
	symbol_list << tr("Circle") << tr("Rectangle") << tr("Diamond") << tr("D-Triangle") 
		<< tr("U-Triangle") << tr("L-Triangle") << tr("R-Triangle") << tr("Cross") << tr("X-Cross");
	cboxSymbolType->addItems(symbol_list);

	cboxLineType->addItems(QStringList() << tr("SolidLine") << tr("DashLine") << tr("DotLine") << tr("DashDotLine") << tr("DashDotDotLine"));

	QStringList curves;
	for (int i = 0; i < curve_list->count(); i++) curves.append(curve_list->at(i)->title().text());
	cboxCurve->addItems(curves);
		
	curve_settings_list = new QList<QwtCurveSettings*>;
	for (int i = 0; i < curve_list->count(); i++)
	{
		QwtSymbol *cur_symbol = new QwtSymbol;
		QwtCurveSettings *curve_settings = new QwtCurveSettings;
		curve_settings->symbol = cur_symbol;

		curve_settings->title = curve_list->at(i)->title().text();
		QBrush brush;
		QPen symbol_pen;
		QwtSymbol::Style symbol_style;
		QSize symbol_size;
		if (curve_list->at(i)->symbol()) 
		{
			brush = curve_list->at(i)->symbol()->brush();
			symbol_pen = curve_list->at(i)->symbol()->pen();
			symbol_style = curve_list->at(i)->symbol()->style();
			symbol_size = curve_list->at(i)->symbol()->size();
		}
		curve_settings->symbol->setStyle(symbol_style);
		curve_settings->symbol->setBrush(brush);
		curve_settings->symbol->setPen(symbol_pen);
		curve_settings->symbol->setSize(symbol_size);
		curve_settings->pen = curve_list->at(i)->pen();
		curve_settings_list->append(curve_settings);
	}
	if (!curve_list->isEmpty()) curve_index = 0;
	else curve_index = -1;

	initSampleCurve();
	setConnections();	
}

CurveDialog::~CurveDialog()
{
	for (int i = 0; i < curve_settings_list->size(); i++)
	{
		QwtCurveSettings *settings = curve_settings_list->at(i);		
		delete settings;
	}

	delete curve_settings_list;
}


void CurveDialog::setConnections()
{
	connect(cboxCurve, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeCurrentCurve(QString)));

	connect(tbtLineColor, SIGNAL(clicked()), this, SLOT(changeLineColor()));
	connect(cboxLineType, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeLineType(QString)));
	connect(sbxLineThickness, SIGNAL(valueChanged(int)), this, SLOT(changeLineWidth(int)));

	connect(cboxSymbolType, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeSymbolType(QString)));
	connect(sbxSymbolSize, SIGNAL(valueChanged(int)), this, SLOT(changeSymbolSize(int)));
	connect(tbtSymbolColor, SIGNAL(clicked()), this, SLOT(changeSymbolFillColor()));
	connect(sbxSymbolEdgeWidth, SIGNAL(valueChanged(int)), this, SLOT(changeSymbolEdgeWidth(int)));
	connect(tbtSymbolEdgeColor, SIGNAL(clicked()), this, SLOT(changeSymbolEdgeColor())); 

	connect(gbxLine, SIGNAL(toggled(bool)), this, SLOT(enableLine(bool)));
	connect(gbxSymbol, SIGNAL(toggled(bool)), this, SLOT(enableSymbol(bool)));
}


void CurveDialog::initSampleCurve()
{
	sample_curve = new QwtPlotCurve("SampleCurve");
	
	if (curve_settings_list->isEmpty()) return;

	symbol = curve_settings_list->first()->symbol;
	pen = curve_settings_list->first()->pen;
		
	double x[11], y[11];
	for (int i = 0; i < 11; i++)
	{
		x[i] = (double)i;
		y[i] = 25 - (x[i]-5)*(x[i]-5);
	}
	sample_curve->setSamples(&x[0], &y[0], 11);
	sample_curve->attach(qwtSample);

	QPalette p = ledLineColor->palette();
	p.setBrush(ledLineColor->backgroundRole(), QBrush(pen.color()));
	ledLineColor->setPalette(p);

	sbxLineThickness->setValue(pen.width());
	cboxLineType->setCurrentText(getLineStyleStr(pen.style()));

	cboxSymbolType->setCurrentText(getSymbolStyleStr(symbol->style()));
	
	QSize sz = symbol->size();
	sbxSymbolSize->setValue(sz.width());
	
	QPalette pb = ledSymbolColor->palette();
	pb.setBrush(ledSymbolColor->backgroundRole(), QBrush(symbol->brush().color()));
	ledSymbolColor->setPalette(pb);

	QPen pen_edge = symbol->pen();
	sbxSymbolEdgeWidth->setValue(pen_edge.width());
	QPalette pe = ledSymbolEdgeColor->palette();
	pe.setBrush(ledSymbolEdgeColor->backgroundRole(), QBrush(pen_edge.color()));
	ledSymbolEdgeColor->setPalette(pe);

	refreshSampleCurve();
}

void CurveDialog::refreshSampleCurve()
{
	QwtSymbol *_symbol = new QwtSymbol(symbol->style(), symbol->brush(), symbol->pen(), symbol->size());
	sample_curve->setSymbol(_symbol);
	sample_curve->setPen(pen);

	qwtSample->replot();
}


void CurveDialog::changeCurrentCurve(QString str)
{	
	for (int i = 0; i < curve_settings_list->size(); i++)
	{
		QString curve_name = curve_settings_list->at(i)->title;
		if (curve_name == str) 
		{
			symbol = curve_settings_list->at(i)->symbol;
			pen = curve_settings_list->at(i)->pen;
			curve_index = i;
		}
	}
	if (curve_index < 0) return;

	changeCurrentCurve(curve_settings_list->at(curve_index));
}

void CurveDialog::changeCurrentCurve(QwtCurveSettings *curve_settings)
{
	if (curve_settings == NULL) return;

	curve_index = -1;
	for (int i = 0; i < curve_settings_list->size(); i++) 
	{
		if (curve_settings->title == curve_settings_list->at(i)->title) curve_index = i;
	}
	if (curve_index < 0) return;

	symbol = curve_settings->symbol;
	if (!symbol) gbxSymbol->setChecked(false);
	else
	{
		if (symbol->style() == QwtSymbol::NoSymbol) gbxSymbol->setChecked(false);
		else gbxSymbol->setChecked(true);
	}

	QPen _pen = curve_settings->pen;
	if (_pen.style() == Qt::NoPen) gbxLine->setChecked(false);
	else gbxLine->setChecked(true);

	pen = _pen;

	QString line_style = getLineStyleStr(pen.style());
	cboxLineType->setCurrentText(line_style);
	sbxLineThickness->setValue(pen.width());
	QPalette p = ledLineColor->palette();
	p.setBrush(ledLineColor->backgroundRole(), QBrush(pen.color()));
	ledLineColor->setPalette(p);

	QString symbol_style = getSymbolStyleStr(symbol->style());
	cboxSymbolType->setCurrentText(symbol_style);
	sbxSymbolSize->setValue(symbol->size().width());
	p = ledSymbolColor->palette();
	p.setBrush(ledSymbolColor->backgroundRole(), QBrush(symbol->brush().color()));
	ledSymbolColor->setPalette(p);
	sbxSymbolEdgeWidth->setValue(symbol->pen().width());
	p = ledSymbolEdgeColor->palette();
	p.setBrush(ledSymbolEdgeColor->backgroundRole(), QBrush(symbol->pen().color()));
	ledSymbolEdgeColor->setPalette(p);

	refreshSampleCurve();
}


void CurveDialog::enableLine(bool flag)
{
	if (!flag) 	pen.setStyle(Qt::NoPen);		
	else pen.setStyle(getLineStyle(cboxLineType->currentText()));
		
	curve_settings_list->at(curve_index)->pen.setStyle(pen.style());

	refreshSampleCurve();
}

void CurveDialog::enableSymbol(bool flag)
{
	if (!flag) symbol->setStyle(QwtSymbol::NoSymbol);	
	else symbol->setStyle(getSymbolStyle(cboxSymbolType->currentText()));

	curve_settings_list->at(curve_index)->symbol->setStyle(symbol->style());

	refreshSampleCurve();
}

void CurveDialog::changeLineColor()
{
	QColor line_color = pen.color();
	line_color = QColorDialog::getColor(line_color);
	if (line_color.isValid())
	{
		QPalette p = ledLineColor->palette();
		p.setBrush(ledLineColor->backgroundRole(), QBrush(line_color));
		ledLineColor->setPalette(p);

		pen.setColor(line_color);
		curve_settings_list->at(curve_index)->pen.setColor(pen.color());

		refreshSampleCurve();
	}
}

void CurveDialog::changeLineType(QString str)
{
	pen.setStyle(getLineStyle(str));
	curve_settings_list->at(curve_index)->pen.setStyle(pen.style());

	refreshSampleCurve();
}

void CurveDialog::changeLineWidth(int width)
{
	pen.setWidth(width);
	curve_settings_list->at(curve_index)->pen.setWidth(pen.width());

	refreshSampleCurve();
}

void CurveDialog::changeSymbolEdgeWidth(int width)
{
	QPen e_pen = symbol->pen();
	e_pen.setWidth(width);
	symbol->setPen(e_pen);
	curve_settings_list->at(curve_index)->symbol->setPen(e_pen);

	refreshSampleCurve();
}

void CurveDialog::changeSymbolEdgeColor()
{
	QPen edge_pen = symbol->pen();
	QColor edge_color = edge_pen.color();
	edge_color = QColorDialog::getColor(edge_color);
	if (edge_color.isValid())
	{
		QPalette p = ledSymbolEdgeColor->palette();
		p.setBrush(ledSymbolEdgeColor->backgroundRole(), QBrush(edge_color));
		ledSymbolEdgeColor->setPalette(p);

		edge_pen.setColor(edge_color);
		symbol->setPen(edge_pen);
		curve_settings_list->at(curve_index)->symbol->setPen(edge_pen);

		refreshSampleCurve();
	}
}

void CurveDialog::changeSymbolType(QString str)
{
	symbol->setStyle(getSymbolStyle(str));
	curve_settings_list->at(curve_index)->symbol->setStyle(symbol->style());

	refreshSampleCurve();
}

void CurveDialog::changeSymbolSize(int size)
{
	symbol->setSize(size, size);
	curve_settings_list->at(curve_index)->symbol->setSize(symbol->size());

	refreshSampleCurve();
}

void CurveDialog::changeSymbolFillColor()
{
	QBrush brush = symbol->brush();
	QColor sym_color = brush.color();
	sym_color = QColorDialog::getColor(sym_color);
	if (sym_color.isValid())
	{
		QPalette p = ledSymbolColor->palette();
		p.setBrush(ledSymbolColor->backgroundRole(), QBrush(sym_color));
		ledSymbolColor->setPalette(p);

		QBrush new_brush(sym_color);
		symbol->setBrush(new_brush);
		curve_settings_list->at(curve_index)->symbol->setBrush(symbol->brush());

		refreshSampleCurve();
	}
}


Qt::PenStyle CurveDialog::getLineStyle(const QString str)
{
	if (str.toLower() == tr("SolidLine").toLower())				return Qt::PenStyle::SolidLine;
	else if (str.toLower() == tr("DashLine").toLower())			return Qt::PenStyle::DashLine;
	else if (str.toLower() == tr("DotLine").toLower())			return Qt::PenStyle::DotLine;
	else if (str.toLower() == tr("DashDotLine").toLower())		return Qt::PenStyle::DashDotLine;
	else if (str.toLower() == tr("DashDotDotLine").toLower())	return Qt::PenStyle::DashDotDotLine;
	else return Qt::PenStyle::SolidLine;
}

QString CurveDialog::getLineStyleStr(const Qt::PenStyle style)
{
	switch (style)
	{
	case Qt::SolidLine:			return tr("SolidLine");
	case Qt::DashLine:			return tr("DashLine");
	case Qt::DotLine:			return tr("DotLine");
	case Qt::DashDotLine:		return tr("DashDotLine");
	case Qt::DashDotDotLine:	return tr("DashDotDotLine");
	default: break; 
	}

	return tr("SolidLine");
}

QwtSymbol::Style CurveDialog::getSymbolStyle(const QString str)
{
	if (str.toLower() == tr("Circle").toLower())			return QwtSymbol::Ellipse;
	else if (str.toLower() == tr("Rectangle").toLower())	return QwtSymbol::Rect;
	else if (str.toLower() == tr("Diamond").toLower())		return QwtSymbol::Diamond;
	else if (str.toLower() == tr("D-Triangle").toLower())	return QwtSymbol::DTriangle;
	else if (str.toLower() == tr("U-Triangle").toLower())	return QwtSymbol::UTriangle;
	else if (str.toLower() == tr("L-Triangle").toLower())	return QwtSymbol::LTriangle;
	else if (str.toLower() == tr("R-Triangle").toLower())	return QwtSymbol::RTriangle;
	else if (str.toLower() == tr("Cross").toLower())		return QwtSymbol::Cross;
	else if (str.toLower() == tr("X-Cross").toLower())		return QwtSymbol::XCross;
	else return QwtSymbol::NoSymbol;
}

QString CurveDialog::getSymbolStyleStr(const QwtSymbol::Style style)
{
	switch (style)
	{
	case QwtSymbol::Ellipse:	return tr("Circle");
	case QwtSymbol::Rect:		return tr("Rectangle");
	case QwtSymbol::Diamond:	return tr("Diamond");
	case QwtSymbol::DTriangle:	return tr("D-Triangle");
	case QwtSymbol::UTriangle:	return tr("U-Triangle");
	case QwtSymbol::LTriangle:	return tr("L-Triangle");
	case QwtSymbol::RTriangle:	return tr("R-Triangle");
	case QwtSymbol::Cross:		return tr("Cross");
	case QwtSymbol::XCross:		return tr("X-Cross");
	default: break; 
	}

	return tr("Circle");
}