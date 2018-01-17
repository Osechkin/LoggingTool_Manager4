#include "qwt_plot_intervalcurve.h"

#include "rfpulse_control_wizard.h"


RFPulseControlWizard::RFPulseControlWizard(QWidget *parent) : QWidget(parent), ui(new Ui::RFPulseControlWizard)
{
	ui->setupUi(this);
		
	// RFP Plot Settings
	ui->qwtPlot_RFP->setCanvasBackground(QBrush(QColor(Qt::white)));

	QwtPlotGrid *grid_RFP = new QwtPlotGrid;
	grid_RFP->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_RFP->enableX(true);	
	grid_RFP->enableY(true);	
	grid_RFP->attach(ui->qwtPlot_RFP);	

	QwtPlotPanner *panner_RFP = new QwtPlotPanner(ui->qwtPlot_RFP->canvas());
	panner_RFP->setAxisEnabled(QwtPlot::yRight, false);
	panner_RFP->setMouseButton(Qt::RightButton);	

	QwtPlotPicker *picker_RFP = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlot_RFP->canvas());	
	picker_RFP->setRubberBandPen(QColor(Qt::green));
	picker_RFP->setRubberBand(QwtPicker::CrossRubberBand);
	picker_RFP->setTrackerPen(QColor(Qt::darkMagenta));

	zoomer_RFP = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot_RFP->canvas());
	zoomer_RFP->setEnabled(true);	
	const QColor c_RFP(Qt::magenta);
	zoomer_RFP->setRubberBandPen(c_RFP);
	zoomer_RFP->setTrackerPen(c_RFP);		

	QwtText yleft_title_RFP;
	QString yleft_title_text_RFP = QString("RF pulse, mV");
	yleft_title_RFP.setText(yleft_title_text_RFP);
	yleft_title_RFP.setColor(QColor(Qt::darkBlue));
	QFont yleft_font_RFP = ui->qwtPlot_RFP->axisTitle(QwtPlot::yLeft).font();
	QFont yleft_axis_font_RFP = ui->qwtPlot_RFP->axisFont(QwtPlot::yLeft);
	yleft_axis_font_RFP.setPointSize(8);
	yleft_axis_font_RFP.setBold(false);
	ui->qwtPlot_RFP->setAxisFont(QwtPlot::yLeft, yleft_axis_font_RFP);
	yleft_font_RFP.setPointSize(8);
	yleft_font_RFP.setBold(false);
	yleft_title_RFP.setFont(yleft_font_RFP);
	ui->qwtPlot_RFP->setAxisTitle(QwtPlot::yLeft, yleft_title_RFP);

	QwtText xbotom_title_RFP = ui->qwtPlot_RFP->axisTitle(QwtPlot::xBottom);
	QString xbotom_title_text_RFP = QString("Time, %1").arg(tr("mks"));
	xbotom_title_RFP.setText(xbotom_title_text_RFP);
	xbotom_title_RFP.setColor(QColor(Qt::darkBlue));
	QFont xbottom_font_RFP = ui->qwtPlot_RFP->axisTitle(QwtPlot::xBottom).font();
	QFont xbottom_axis_font_RFP = ui->qwtPlot_RFP->axisFont(QwtPlot::xBottom);
	xbottom_axis_font_RFP.setPointSize(8);
	xbottom_axis_font_RFP.setBold(false);
	ui->qwtPlot_RFP->setAxisFont(QwtPlot::xBottom, xbottom_axis_font_RFP);
	xbottom_font_RFP.setBold(false);
	xbottom_font_RFP.setPointSize(8);
	xbotom_title_RFP.setFont(xbottom_font_RFP);
	ui->qwtPlot_RFP->setAxisTitle(QwtPlot::xBottom, xbotom_title_RFP);
		
	QFont tab_font_RFP = ui->tabWidgetRFP->tabBar()->font();
	tab_font_RFP.setBold(false);
	ui->tabWidgetRFP->tabBar()->setFont(tab_font_RFP);
		
	pds_created_RFP = false;
	y_max = 0;
}

RFPulseControlWizard::~RFPulseControlWizard()
{
	delete ui;
}


double RFPulseControlWizard::maxValue(double *_vec, int n)
{
	if (n <= 0) return 0;

	double max = _vec[0];
	for (int i = 1; i < n; i++)
	{
		if (_vec[i] > max) max = _vec[i];
	}

	return max;
}

void RFPulseControlWizard::setDataSet(QString _name, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map)
{
	removeDataSet();

	curve_settings_RFP = new QwtCurveSettings;
	curve_settings_RFP->dataType = QwtCurveSettings::LinesAndSymbols;
	curve_settings_RFP->title = "RFPulse-curve";

	curve_settings_RFP->symbol->setBrush(QBrush(QColor(Qt::red)));
	curve_settings_RFP->symbol->setSize(4);
	curve_settings_RFP->symbol->setStyle(QwtSymbol::Ellipse);

	QPen symbol_pen(Qt::SolidLine);
	symbol_pen.setColor(QColor(Qt::blue));
	symbol_pen.setWidth(1);
	curve_settings_RFP->symbol->setPen(symbol_pen);

	curve_settings_RFP->pen.setStyle(Qt::SolidLine);
	curve_settings_RFP->pen.setColor(QColor(Qt::darkMagenta));
	curve_settings_RFP->pen.setWidth(1);

	PlottedDataSet *dataset = new PlottedDataSet(_name, 0, _x, _y, _bad_map, curve_settings_RFP);
	data_RFP = dataset;
	pds_created_RFP = true;

	plotData();
}

PlottedDataSet* RFPulseControlWizard::getDataSet()
{
	return data_RFP;	
}

/*
void RFPulseControlWizard::plotData()
{	
	int sz = data_RFP->getYData()->size();
	
	double *y_data1 = new double[sz];	
	double *x_data = new double[sz];

	memcpy(x_data, data_RFP->getXData()->data(), sz*sizeof(double));
	memcpy(y_data1, data_RFP->getYData()->data(), sz*sizeof(double));
	
	curve_RFP1 = new QwtPlotCurve(data_RFP->getName());
	curve_RFP1->setRenderHint(QwtPlotItem::RenderAntialiased);

	QwtCurveSettings *cs = data_RFP->getCurveSettings();		
	
	QwtCurveSettings::DataType data_type = cs->dataType; 
	if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve_RFP1->setStyle(QwtPlotCurve::Lines);
	else curve_RFP1->setStyle(QwtPlotCurve::NoCurve);

	QwtSymbol *sym = new QwtSymbol;
	QwtSymbol::Style sym_style = cs->symbol->style();
	sym->setStyle(sym_style);
	sym->setSize(cs->symbol->size());
	QPen sym_pen = sym->pen();
	sym_pen.setColor(cs->symbol->pen().color());
	sym_pen.setWidth(cs->symbol->pen().width());
	sym->setPen(sym_pen);
	QBrush sym_brush = sym->brush();
	sym_brush.setColor(cs->symbol->brush().color());
	sym_brush.setStyle(Qt::SolidPattern);
	sym->setBrush(sym_brush);
	curve_RFP1->setSymbol(sym);

	curve_RFP1->setSamples(x_data, y_data1, sz);
	curve_RFP1->attach(ui->qwtPlot_RFP);
	
	double max = 1.25*maxValue(y_data1, sz);
	ui->qwtPlot_RFP->setAxisScale(QwtPlot::yLeft, -max, max);		
		
	ui->qwtPlot_RFP->replot();

	delete x_data;
	delete y_data1;	
}*/

void RFPulseControlWizard::plotData()
{	
	int sz = data_RFP->getYData()->size();
	
	/*double *y_data1 = new double[sz];
	double *y_data2 = new double[sz];
	double *x_data = new double[sz];*/

	/*memcpy(x_data, data_RFP->getXData()->data(), sz*sizeof(double));
	memcpy(y_data1, data_RFP->getYData()->data(), sz*sizeof(double));
	for (int i = 0; i < sz; i++)
	{
		y_data2[i] = -y_data1[i];
	}*/

	QVector<QwtIntervalSample> rangeDataY(sz);	
	for (int i = 0; i < sz; i++)
	{
		double x = data_RFP->getXData()->data()[i];
		double y = data_RFP->getYData()->data()[i];

		QPair<double, double> range1(y, -y);		
		rangeDataY[i] = QwtIntervalSample(x, QwtInterval(range1.first, range1.second));
	}

	/*curve_RFP1 = new QwtPlotCurve(data_RFP->getName());
	curve_RFP1->setRenderHint(QwtPlotItem::RenderAntialiased);

	curve_RFP2 = new QwtPlotCurve(data_RFP->getName());
	curve_RFP2->setRenderHint(QwtPlotItem::RenderAntialiased);*/

	//QwtCurveSettings *cs = data_RFP->getCurveSettings();	

	//QwtCurveSettings::DataType data_type = cs->dataType; 
	//if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve_RFP1->setStyle(QwtPlotCurve::Lines);
	//else curve_RFP1->setStyle(QwtPlotCurve::NoCurve);
	//curve_RFP1->setStyle(QwtPlotCurve::Lines);
	//curve_RFP2->setStyle(QwtPlotCurve::Lines);
	
	/*QPen p = QPen(Qt::magenta);
	QColor c = QColor(Qt::darkBlue);
	c.setAlpha( 150 );
	curve_RFP1->setPen(p);
	curve_RFP1->setBrush(c);		
	curve_RFP2->setPen(p);
	curve_RFP2->setBrush(c);*/
	
	/*QwtSymbol *sym = new QwtSymbol;
	QwtSymbol::Style sym_style = cs->symbol->style();
	sym->setStyle(sym_style);
	sym->setSize(cs->symbol->size());
	QPen sym_pen = sym->pen();
	sym_pen.setColor(cs->symbol->pen().color());
	sym_pen.setWidth(cs->symbol->pen().width());
	sym->setPen(sym_pen);
	QBrush sym_brush = sym->brush();
	sym_brush.setColor(cs->symbol->brush().color());
	sym_brush.setStyle(Qt::SolidPattern);
	sym->setBrush(sym_brush);
	curve_RFP1->setSymbol(sym);*/

	/*QPen curve_pen(Qt::darkMagenta);
	curve_pen.setWidth(3);
	curve_RFP1->setPen(curve_pen);
	curve_RFP2->setPen(curve_pen);

	curve_RFP1->setSamples(x_data, y_data1, sz);
	curve_RFP1->attach(ui->qwtPlot_RFP);

	curve_RFP2->setSamples(x_data, y_data2, sz);
	curve_RFP2->attach(ui->qwtPlot_RFP);*/

	double max = 1.25*maxValue(data_RFP->getYData()->data(), sz);
	if (max > y_max) 
	{
		if (max < 10000) y_max = max;
	}
	ui->qwtPlot_RFP->setAxisScale(QwtPlot::yLeft, -y_max, y_max);
	ui->qwtPlot_RFP->setAxisScale(QwtPlot::xBottom, data_RFP->getXData()->first(), data_RFP->getXData()->last());
	
	insertCurveBar("RF-Pulse", rangeDataY, QColor(Qt::darkMagenta), QColor(Qt::magenta));

	//ui->qwtPlot_RFP->replot();

	/*delete x_data;
	delete y_data1;	
	delete y_data2;*/
}

void RFPulseControlWizard::insertCurveBar(const QString &title, const QVector<QwtIntervalSample>& samples, const QColor &bg_color, const QColor &curve_color)
{
	d_intervalCurve = new QwtPlotIntervalCurve(title);
	d_intervalCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
	d_intervalCurve->setPen(Qt::white);

	QColor bg(bg_color);
	bg.setAlpha(150);
	QColor cv(curve_color);
	d_intervalCurve->setBrush(QBrush(bg));
	d_intervalCurve->setPen(QPen(cv,2));
	d_intervalCurve->setStyle(QwtPlotIntervalCurve::Tube);

	d_intervalCurve->setSamples(samples);
	//d_intervalCurve->setAxes(QwtPlot::yLeft, QwtPlot::xBottom);

	d_intervalCurve->attach(ui->qwtPlot_RFP);
	ui->qwtPlot_RFP->replot();
}


void RFPulseControlWizard::removeDataSet()
{
	if (pds_created_RFP)
	{
		//curve_RFP1->detach();
		//curve_RFP2->detach();
		d_intervalCurve->detach();
		ui->qwtPlot_RFP->replot();

		delete data_RFP;		
		delete d_intervalCurve;
		//delete curve_RFP1;
		//delete curve_RFP2;

		pds_created_RFP = false;
	}	
}


void RFPulseControlWizard::changeLocation(Qt::DockWidgetArea area)
{
	if (area == Qt::RightDockWidgetArea || area == Qt::LeftDockWidgetArea)
	{		
		ui->tabWidgetRFP->setTabPosition(QTabWidget::South);
	}
	else if (area == Qt::BottomDockWidgetArea || area == Qt::NoDockWidgetArea)
	{		
		ui->tabWidgetRFP->setTabPosition(QTabWidget::West);
	}
}