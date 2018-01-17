#include <QtGui>

#include "plotted_objects.h"
#include "io_general.h"


PaintFrame::PaintFrame(QWidget *parent) : QWidget(parent)
{
	setParent(parent);

	this->setFixedHeight(20);
	this->setFixedWidth(60);

	/*symSize = 10;
	penWidth = 1;
	symStyle = CurveSettings::Circle; // Circle type is default
	symEdge = 1;
	symColor = Qt::red;
	symEdgeColor = Qt::black;
	penColor = Qt::black;
	penStyle = Qt::SolidLine;
	dataType = CurveSettings::LinesAndSymbols; // "Lines and Symbols" is default data type*/

	curve_settings = new QwtCurveSettings;
	QBrush symbol_brush(Qt::red);
	symbol_brush.setStyle(Qt::SolidPattern);
	curve_settings->symbol->setStyle(QwtSymbol::Ellipse);
	curve_settings->symbol->setBrush(symbol_brush);
	curve_settings->symbol->setSize(QSize(10,10));
	QPen symbol_pen(Qt::black);
	symbol_pen.setWidth(1);
	curve_settings->symbol->setPen(symbol_pen);	
	curve_settings->pen.setColor(Qt::red);
	curve_settings->pen.setStyle(Qt::SolidLine);
	curve_settings->pen.setWidth(1);
	curve_settings->dataType = QwtCurveSettings::LinesAndSymbols;
}

PaintFrame::~PaintFrame()
{
	delete curve_settings;
}

void PaintFrame::setDataType(QwtCurveSettings::DataType _dataType)
{
	curve_settings->dataType = _dataType;
}

void PaintFrame::setLineSettings(int penWidth, QColor penColor)
{
	curve_settings->pen.setWidth(penWidth);
	curve_settings->pen.setColor(penColor);
}

void PaintFrame::setLineStyle(Qt::PenStyle style)
{	
	curve_settings->pen.setStyle(style);
}

void PaintFrame::setSymbolSettings(QwtSymbol::Style symStyle, int symSize, int symEdge, QColor symColor, QColor symEdgeColor)
{
	QPen symbol_pen(symEdgeColor);
	QBrush symbol_brush(symColor);
	symbol_pen.setWidth(symEdge);
	curve_settings->symbol->setStyle(symStyle);
	curve_settings->symbol->setSize(QSize(symSize,symSize));
	curve_settings->symbol->setBrush(symbol_brush);
	curve_settings->symbol->setPen(symbol_pen);
}

void PaintFrame::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setClipRect(contentsRect());
	painter.fillRect(0,0,60,20,Qt::white);

	QPen pen = QPen(curve_settings->pen.color());
	pen.setWidth(curve_settings->pen.width());	
	pen.setStyle(curve_settings->pen.style());
	QBrush brush = QBrush(curve_settings->symbol->brush());
	QPen epen = QPen(curve_settings->symbol->pen().color());
	epen.setWidth(curve_settings->symbol->pen().width());

	switch (curve_settings->dataType)
	{
	case CurveSettings::Lines:
		{
			painter.setPen(pen);
			painter.drawLine(5,9,55,9);
			break;
		}		
	case CurveSettings::Symbols:
		{
			painter.setPen(epen);
			painter.setBrush(brush);
			//drawDataSymbol(&painter, symStyle);
			drawDataSymbol(&painter, curve_settings->symbol->style());
			break;
		}
	case CurveSettings::LinesAndSymbols:
		{
			painter.setPen(pen);
			painter.drawLine(5,9,55,9);
			painter.setPen(epen);
			painter.setBrush(brush);
			//drawDataSymbol(&painter, symStyle);
			drawDataSymbol(&painter, curve_settings->symbol->style());
			break;
		}
	default: return;
	}	
}


void PaintFrame::drawDataSymbol(QPainter *painter, QwtSymbol::Style symType)
{
	QPolygonF polygon;
	qreal x;
	qreal y;    

	switch (symType)
	{
	case QwtSymbol::Ellipse: // Circle        
		painter->drawEllipse(25,4,10,10);
		break;

	case QwtSymbol::Rect: // Rect
		painter->drawRect(25,4,10,10);
		break;

	case QwtSymbol::UTriangle: // UTriangle
		x = 25; y = 13;
		polygon << QPointF(x, y);
		x = 30; y = 3;
		polygon << QPointF(x, y);
		x = 35; y = 13;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::DTriangle: // DTriangle        
		x = 25; y = 3;
		polygon << QPointF(x, y);
		x = 35; y = 3;
		polygon << QPointF(x, y);
		x = 30; y = 13;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::RTriangle: // RTriangle
		x = 25; y = 13;
		polygon << QPointF(x, y);
		x = 25; y = 3;
		polygon << QPointF(x, y);
		x = 35; y = 8;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::LTriangle: // LTriangle        
		x = 25; y = 8;
		polygon << QPointF(x, y);
		x = 35; y = 3;
		polygon << QPointF(x, y);
		x = 35; y = 13;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::Diamond: // Diamond        
		x = 25; y = 8;
		polygon << QPointF(x, y);
		x = 30; y = 3;
		polygon << QPointF(x, y);
		x = 35; y = 8;
		polygon << QPointF(x, y);
		x = 30; y = 13;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::Cross: // Cross
		painter->drawLine(30,4,30,14);
		painter->drawLine(25,9,35,9);
		break;

	case QwtSymbol::XCross: // XCross
		painter->drawLine(25,4,35,14);
		painter->drawLine(25,14,35,4);
		break;

	default:
		painter->drawEllipse(25,4,10,10);
	}
}

PlottedDataSet::PlottedDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QObject *parent)
{
	setParent(parent);
	data_set = NULL;

	id = generateID();

	name = _name;	
	data_plot = _plot;
	if (data_plot != 0) data_plot_name = data_plot->objectName();
	else data_plot_name = "";

	//points_count = _y->size();
	
	x = _x;
	y = _y;
	bad_map = _bad_map;
	points_count = 0;
	for (int i = 0; i < y->size(); i++)
	{
		if (!isNAN(y->data()[i]) && bad_map->data()[i] == DATA_OK) points_count++;
	}
	
	hold_on = false;	
	mark = false;
	
	QwtCurveSettings *def_curve_setings = data_plot->getDefaultCurveSettings(0);

	curve_settings = new QwtCurveSettings;
	curve_settings->dataType = def_curve_setings->dataType;
	curve_settings->title = def_curve_setings->title;
	curve_settings->symbol->setBrush(def_curve_setings->symbol->brush());
	curve_settings->symbol->setSize(def_curve_setings->symbol->size().width());
	curve_settings->symbol->setStyle(def_curve_setings->symbol->style());
	curve_settings->symbol->setPen(def_curve_setings->symbol->pen());
	curve_settings->pen = def_curve_setings->pen;
	curve_settings->z = 0;

	pframe = new PaintFrame;
	pframe->setDataType(curve_settings->dataType);
	pframe->setLineSettings(curve_settings->pen.width(), curve_settings->pen.color());
	pframe->setLineStyle(curve_settings->pen.style());
	pframe->setSymbolSettings(curve_settings->symbol->style(),
							  curve_settings->symbol->size().width(),
							  curve_settings->symbol->pen().width(),
							  curve_settings->symbol->brush().color(),
							  curve_settings->symbol->pen().color());

	c_twi = NULL;
	parent_dataset = NULL;
}

PlottedDataSet::PlottedDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, QObject *parent)
{
	setParent(parent);
	data_set = NULL;

	id = generateID();

	name = _name;	
	data_plot = _plot;
	
	//data_plot_name = data_plot->objectName();
	if (data_plot != 0) data_plot_name = data_plot->objectName();
	else data_plot_name = "";

	x = _x;
	y = _y;
	bad_map = _bad_map;
	points_count = 0;
	for (int i = 0; i < y->size(); i++)
	{
		if (!isNAN(y->data()[i]) && bad_map->data()[i] == DATA_OK) points_count++;
	}
	
	hold_on = false;	
	mark = false;
		
	curve_settings = _settings;

	pframe = new PaintFrame;
	pframe->setDataType(curve_settings->dataType);
	pframe->setLineSettings(curve_settings->pen.width(), curve_settings->pen.color());
	pframe->setLineStyle(curve_settings->pen.style());
	pframe->setSymbolSettings(curve_settings->symbol->style(),
							  curve_settings->symbol->size().width(),
							  curve_settings->symbol->pen().width(),
							  curve_settings->symbol->brush().color(),
							  curve_settings->symbol->pen().color());

	c_twi = NULL;
	parent_dataset = NULL;
}

PlottedDataSet::~PlottedDataSet()
{
	delete pframe;
	delete curve_settings;
	//delete x;
	//delete y;
	//delete bad_map;
}

int PlottedDataSet::generateID()
{
	static int _id = 0;
	return ++_id;
}

void PlottedDataSet::changeDataSetState(QObject *obj, QVariant &value)
{
	if (CCheckBox *chbx = qobject_cast<CCheckBox*>(obj))
	{
		QString object_name = chbx->objectName();		
		if (object_name.split("_").last().contains("mark"))
		{
			bool ok;
			Qt::CheckState state = (Qt::CheckState)value.toInt(&ok);
			if (ok) this->markDataSet(state == Qt::Checked ? true : false);
		}		
		else if (object_name.split("_").last().contains("hold"))
		{
			bool ok;
			Qt::CheckState state = (Qt::CheckState)value.toInt(&ok);
			if (ok) this->holdDataSetOn(state == Qt::Checked ? true : false);
		}
	}
	
}

void PlottedDataSet::setDefaultCurveSettings(QwtCurveSettings *_def_curve_settings, int index)
{
	if (index < data_plot->getDefaultCurveSettingsList().count())
	{
		QwtCurveSettings *cs = data_plot->getDefaultCurveSettings(index);
		cs->dataType = _def_curve_settings->dataType;
		cs->title = _def_curve_settings->title;
		cs->symbol->setBrush(_def_curve_settings->symbol->brush());
		cs->symbol->setSize(_def_curve_settings->symbol->size());
		cs->symbol->setStyle(_def_curve_settings->symbol->style());
		cs->symbol->setPen(_def_curve_settings->symbol->pen());
		cs->pen = _def_curve_settings->pen;
	}	
}

void PlottedDataSet::setDefaultCurveSettings(int index)
{
	if (index < data_plot->getDefaultCurveSettingsList().count())
	{
		QwtCurveSettings *def_curve_settings = data_plot->getDefaultCurveSettings(index);

		delete curve_settings;
		curve_settings = new QwtCurveSettings;

		curve_settings->dataType = def_curve_settings->dataType;
		curve_settings->title = def_curve_settings->title;
		curve_settings->symbol->setBrush(def_curve_settings->symbol->brush());
		curve_settings->symbol->setSize(def_curve_settings->symbol->size().width());
		curve_settings->symbol->setStyle(def_curve_settings->symbol->style());
		curve_settings->symbol->setPen(def_curve_settings->symbol->pen());
		curve_settings->pen = def_curve_settings->pen;

		pframe = new PaintFrame;
		pframe->setDataType(curve_settings->dataType);
		pframe->setLineSettings(curve_settings->pen.width(), curve_settings->pen.color());
		pframe->setLineStyle(curve_settings->pen.style());
		pframe->setSymbolSettings(curve_settings->symbol->style(),
		curve_settings->symbol->size().width(),
		curve_settings->symbol->pen().width(),
		curve_settings->symbol->brush().color(),
		curve_settings->symbol->pen().color());		
	}	
}