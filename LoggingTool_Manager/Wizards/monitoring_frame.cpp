#include "monitoring_frame.h"


PaintFrame_forMonitoring::PaintFrame_forMonitoring(QWidget *parent)
{
	setParent(parent);

	frame_height = 16;
	frame_width = 40;
	symbol_size = 6;

	this->setFixedHeight(frame_height);
	this->setFixedWidth(frame_width);

	curve_settings = new QwtCurveSettings;
	QBrush symbol_brush(Qt::red);
	symbol_brush.setStyle(Qt::SolidPattern);
	curve_settings->symbol->setStyle(QwtSymbol::Ellipse);
	curve_settings->symbol->setBrush(symbol_brush);
	curve_settings->symbol->setSize(QSize(symbol_size, symbol_size));
	QPen symbol_pen(Qt::black);
	symbol_pen.setWidth(1);
	curve_settings->symbol->setPen(symbol_pen);	
	curve_settings->pen.setColor(Qt::red);
	curve_settings->pen.setStyle(Qt::SolidLine);
	curve_settings->pen.setWidth(1);
	curve_settings->dataType = QwtCurveSettings::LinesAndSymbols;

}

PaintFrame_forMonitoring::PaintFrame_forMonitoring(QwtCurveSettings *c_settings, QWidget *parent)
{
	setParent(parent);

	frame_height = 16;
	frame_width = 40;
	symbol_size = 6;

	this->setFixedHeight(frame_height);
	this->setFixedWidth(frame_width);

	curve_settings = new QwtCurveSettings;
	QBrush symbol_brush(c_settings->symbol->brush().color());
	symbol_brush.setStyle(c_settings->symbol->brush().style());
	curve_settings->symbol->setStyle(c_settings->symbol->style());
	curve_settings->symbol->setBrush(symbol_brush);
	curve_settings->symbol->setSize(c_settings->symbol->size());
	QPen symbol_pen(c_settings->symbol->pen());
	symbol_pen.setWidth(1);
	curve_settings->symbol->setPen(symbol_pen);	
	curve_settings->pen.setColor(c_settings->pen.color());
	curve_settings->pen.setStyle(c_settings->pen.style());
	curve_settings->pen.setWidth(1);
	curve_settings->dataType = c_settings->dataType;
}


void PaintFrame_forMonitoring::paintEvent(QPaintEvent *event)
{	
	QPainter painter(this);
	painter.setClipRect(contentsRect());
	painter.fillRect(0, 0, frame_width, frame_height, Qt::white);

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
			painter.drawLine(5, frame_height/2-1, frame_width-5, frame_height/2-1);
			break;
		}		
	case CurveSettings::Symbols:
		{
			painter.setPen(epen);
			painter.setBrush(brush);			
			drawDataSymbol(&painter, curve_settings->symbol->style());
			break;
		}
	case CurveSettings::LinesAndSymbols:
		{
			painter.setPen(pen);
			painter.drawLine(5, frame_height/2-1, frame_width-5, frame_height/2-1);
			painter.setPen(epen);
			painter.setBrush(brush);			
			drawDataSymbol(&painter, curve_settings->symbol->style());
			break;
		}
	default: return;
	}	
}

void PaintFrame_forMonitoring::drawDataSymbol(QPainter *painter, QwtSymbol::Style symType)
{
	QPolygonF polygon;
	qreal x;
	qreal y;    

	switch (symType)
	{
	case QwtSymbol::Ellipse: // Circle        
		painter->drawEllipse((frame_width-symbol_size)/2, (frame_height-symbol_size)/2-1, symbol_size, symbol_size);
		break;

	case QwtSymbol::Rect: // Rect
		painter->drawRect((frame_width-symbol_size)/2, (frame_height-symbol_size)/2-1, symbol_size, symbol_size);
		break;

	case QwtSymbol::UTriangle: // UTriangle
		x = (frame_width-symbol_size)/2; y = (frame_height+symbol_size)/2;
		polygon << QPointF(x, y);
		x = frame_width/2; y = (frame_height-symbol_size)/2;
		polygon << QPointF(x, y);
		x = (frame_width+symbol_size)/2; y = (frame_height+symbol_size)/2;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::DTriangle: // DTriangle        
		x = (frame_width-symbol_size)/2; y = (frame_height-symbol_size)/2;
		polygon << QPointF(x, y);
		x = (frame_width+symbol_size)/2; y = (frame_height-symbol_size)/2;
		polygon << QPointF(x, y);
		x = frame_width/2; y = (frame_height+symbol_size)/2;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::RTriangle: // RTriangle
		x = (frame_width-symbol_size)/2; y = (frame_height+symbol_size)/2;
		polygon << QPointF(x, y);
		x = (frame_width-symbol_size)/2; y = (frame_height-symbol_size)/2;
		polygon << QPointF(x, y);
		x = (frame_width+symbol_size)/2; y = frame_height/2;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::LTriangle: // LTriangle        
		x = (frame_width-symbol_size)/2; y = frame_height/2;
		polygon << QPointF(x, y);
		x = (frame_width+symbol_size)/2; y = (frame_height-symbol_size)/2;
		polygon << QPointF(x, y);
		x = (frame_width+symbol_size)/2; y = (frame_height+symbol_size)/2;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::Diamond: // Diamond        
		x = (frame_width-symbol_size)/2; y = frame_height/2;
		polygon << QPointF(x, y);
		x = frame_width/2; y = (frame_height-symbol_size)/2;
		polygon << QPointF(x, y);
		x = (frame_width+symbol_size)/2; y = frame_height/2;
		polygon << QPointF(x, y);
		x = frame_width/2; y = (frame_height+symbol_size)/2;
		polygon << QPointF(x, y);
		painter->drawPolygon(polygon);
		break;

	case QwtSymbol::Cross: // Cross
		painter->drawLine(frame_width/2, (frame_height-symbol_size)/2, frame_width/2, (frame_height+symbol_size)/2);
		painter->drawLine((frame_width-symbol_size)/2, frame_height/2, (frame_width+symbol_size)/2, frame_height/2);
		break;

	case QwtSymbol::XCross: // XCross
		painter->drawLine((frame_width-symbol_size)/2, (frame_height-symbol_size)/2, (frame_width+symbol_size)/2, (frame_height+symbol_size)/2);
		painter->drawLine((frame_width-symbol_size)/2, (frame_height+symbol_size)/2, (frame_width+symbol_size)/2, (frame_height-symbol_size)/2);
		break;

	default:
		painter->drawEllipse((frame_width-symbol_size)/2, (frame_height-symbol_size)/2, symbol_size, symbol_size);
	}
}


MonitoringFrame::MonitoringFrame(ScannedQuantity _data_type, QWidget *parent /* = 0 */) : QWidget(parent), ui(new Ui::MonitoringFrame)
{
	ui->setupUi(this);

	ui->qwtPlot->setAxisMaxMajor(QwtPlot::xBottom, 5);
	ui->qwtPlot->setAxisMaxMajor(QwtPlot::xTop, 5);
	ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft, false);
	ui->qwtPlot->setAxisAutoScale(QwtPlot::yRight, false);

	plot_grid = new QwtPlotGrid;
	plot_grid->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	plot_grid->enableX(true);	
	plot_grid->enableY(true);	
	plot_grid->attach(ui->qwtPlot);

	plot_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlot->canvas());	
	plot_picker->setRubberBandPen(QColor(Qt::green));
	plot_picker->setRubberBand(QwtPicker::CrossRubberBand);
	plot_picker->setTrackerPen(QColor(Qt::darkMagenta));

	plot_zoomer = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot->canvas());
	plot_zoomer->setEnabled(true);
	const QColor c_Rx(Qt::magenta);
	plot_zoomer->setRubberBandPen(c_Rx);
	plot_zoomer->setTrackerPen(c_Rx);
	connect(plot_zoomer, SIGNAL(zoomed(const QRectF &)), this, SLOT(zoomed(const QRectF &)));

	QPalette p = this->palette(); 
	//p.setColor(QPalette::Background, QColor(Qt::blue).lighter(195));
	p.setColor(QPalette::Background, QColor(Qt::white));
	this->setAutoFillBackground(true);
	this->setPalette(p);
	this->show();
	setStyleSheet(QString("QwtPlotCanvas {" "    background: white;" "}"));

	QStringList type_list;
	type_list << "Temperature" << "Voltage" << "Current" << "No Type";
	ui->cboxScannedQuantity->addItems(type_list);

	//data_type = MonitoringData::NoType;
	data_type = _data_type;
	switch (data_type)
	{
	case ScannedQuantity::Temperature: 
		ui->cboxScannedQuantity->setCurrentText("Temperature");
		break;
	case ScannedQuantity::Voltage:
		ui->cboxScannedQuantity->setCurrentText("Voltage");
		break;
	case ScannedQuantity::Current:
		ui->cboxScannedQuantity->setCurrentText("Current");
		break;
	case ScannedQuantity::NoType:
		ui->cboxScannedQuantity->setCurrentText("No Type");
		break;
	default:
		ui->cboxScannedQuantity->setCurrentText("No Type");
		break;
	}

	items_count = 0;
	active = false;
}

MonitoringFrame::~MonitoringFrame()
{
	delete ui;
}

void MonitoringFrame::mousePressEvent( QMouseEvent *event )
{
	// do not firing any event besides code below because event allready handled
	event->accept();

	QPalette p = this->palette(); 
	p.setColor(QPalette::Background, QColor(Qt::blue).lighter(190));	
	this->setAutoFillBackground(true);
	this->setPalette(p);
	this->show();

	active = true;
	emit activated();
}

void MonitoringFrame::zoomed(const QRectF &rect)
{
	QwtPlot *qwtPlot = ui->qwtPlot;
	void *qwtplot_obj = (void*)qwtPlot;
	emit plot_rescaled(qwtplot_obj);
}

void MonitoringFrame::setDefaultPalette()
{
	QPalette p = this->palette(); 
	p.setColor(QPalette::Background, QColor(Qt::white));
	this->setAutoFillBackground(true);
	this->setPalette(p);
	this->show();
}

void MonitoringFrame::clearAll()
{
	QwtPlot *qwtPlot = this->plot();
	bool autoDelete = false;
	qwtPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, autoDelete);
	
	QObjectList louts = this->legend_lout()->children();
	for (int i = 0; i < louts.count(); i++)
	{
		QHBoxLayout *item = qobject_cast<QHBoxLayout*>(louts.at(i));		
		while (QLayoutItem *sub_item = item->takeAt(0))
		{
			QWidget *widget = sub_item->widget();
			item->removeWidget(widget);
			delete widget;			
		}
		this->legend_lout()->removeItem(item);
	}
	qDeleteAll(louts.begin(), louts.end());
	louts.clear();

	ui->frameCurves->repaint();

	items_count = 0;
}

void MonitoringFrame::addCurve(QwtPlotCurve *_curve)
{
	QHBoxLayout *hout = new QHBoxLayout;
	hout->setSpacing(3);
	hout->setContentsMargins(0, 0, 0, 0);

	QwtCurveSettings c_settings;
	c_settings.dataType = QwtCurveSettings::DataType::LinesAndSymbols;
	QPen curve_pen = _curve->pen();
	c_settings.pen = curve_pen;
	QwtSymbol *sym = c_settings.symbol;
	sym->setBrush(_curve->symbol()->brush());
	sym->setPen(_curve->symbol()->pen());
	PaintFrame_forMonitoring *pframe = new PaintFrame_forMonitoring(&c_settings);	
	hout->addWidget(pframe);

	QLabel *label = new QLabel;	
	label->setText(_curve->title().text());
	QFont label_font = label->font();
	label_font.setPointSize(8);
	label->setFont(label_font);
	QPalette label_palette = label->palette();
	label_palette.setColor(QPalette::WindowText, _curve->pen().color());
	label_palette.setColor(QPalette::Background, Qt::white);
	label->setAutoFillBackground(true);
	label->setPalette(label_palette);

	hout->addWidget(label);
	legend_lout()->addLayout(hout);

	getCurveFrame()->repaint();
		
	_curve->attach(ui->qwtPlot);
	ui->qwtPlot->replot();

	items_count++;
}