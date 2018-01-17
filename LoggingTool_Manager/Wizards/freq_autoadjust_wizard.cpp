#include "qwt_plot_textlabel.h"
#include "qwt_plot_marker.h"

#include "freq_autoadjust_wizard.h"


FreqAutoadjustWizard::FreqAutoadjustWizard(QWidget *parent) : QWidget(parent), ui(new Ui::tabFreqAutoadjust)
{
	ui->setupUi(this);

	// Frequency Autotune Plot Settings
	ui->qwtPlot_Freq->setCanvasBackground(QBrush(QColor(Qt::white)));

	QwtPlotGrid *grid_Freq = new QwtPlotGrid;
	grid_Freq->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_Freq->enableX(true);	
	grid_Freq->enableY(true);	
	grid_Freq->attach(ui->qwtPlot_Freq);

	QwtPlotPanner *panner_Freq = new QwtPlotPanner(ui->qwtPlot_Freq->canvas());
	panner_Freq->setAxisEnabled(QwtPlot::yRight, false);
	panner_Freq->setMouseButton(Qt::RightButton);	

	QwtPlotPicker *picker_Freq = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlot_Freq->canvas());	
	picker_Freq->setRubberBandPen(QColor(Qt::green));
	picker_Freq->setRubberBand(QwtPicker::CrossRubberBand);
	picker_Freq->setTrackerPen(QColor(Qt::darkMagenta));

	zoomer_Freq = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot_Freq->canvas());
	zoomer_Freq->setEnabled(true);	
	const QColor c_Freq(Qt::darkMagenta);
	zoomer_Freq->setRubberBandPen(c_Freq);
	zoomer_Freq->setTrackerPen(c_Freq);		

	QwtText yleft_title_Freq;
	QString yleft_title_text_Freq = QString("NMR signal, a.u.");
	yleft_title_Freq.setText(yleft_title_text_Freq);
	yleft_title_Freq.setColor(QColor(Qt::darkBlue));
	QFont yleft_font_Freq = ui->qwtPlot_Freq->axisTitle(QwtPlot::yLeft).font();
	QFont yleft_axis_font_Freq = ui->qwtPlot_Freq->axisFont(QwtPlot::yLeft);
	yleft_axis_font_Freq.setPointSize(8);
	yleft_axis_font_Freq.setBold(false);
	ui->qwtPlot_Freq->setAxisFont(QwtPlot::yLeft, yleft_axis_font_Freq);
	yleft_font_Freq.setPointSize(8);
	yleft_font_Freq.setBold(false);
	yleft_title_Freq.setFont(yleft_font_Freq);
	ui->qwtPlot_Freq->setAxisTitle(QwtPlot::yLeft, yleft_title_Freq);

	QwtText xbotom_title_Freq = ui->qwtPlot_Freq->axisTitle(QwtPlot::xBottom);
	QString xbotom_title_text_Freq = QString("dF, %1").arg(tr("kHz"));
	xbotom_title_Freq.setText(xbotom_title_text_Freq);
	xbotom_title_Freq.setColor(QColor(Qt::darkBlue));
	QFont xbottom_font_Freq = ui->qwtPlot_Freq->axisTitle(QwtPlot::xBottom).font();
	QFont xbottom_axis_font_Freq = ui->qwtPlot_Freq->axisFont(QwtPlot::xBottom);
	xbottom_axis_font_Freq.setPointSize(8);
	xbottom_axis_font_Freq.setBold(false);
	ui->qwtPlot_Freq->setAxisFont(QwtPlot::xBottom, xbottom_axis_font_Freq);
	xbottom_font_Freq.setBold(false);
	xbottom_font_Freq.setPointSize(8);
	xbotom_title_Freq.setFont(xbottom_font_Freq);
	ui->qwtPlot_Freq->setAxisTitle(QwtPlot::xBottom, xbotom_title_Freq);
	
	QFont tab_font_Tune = ui->tabWidgetFreq->tabBar()->font();
	tab_font_Tune.setBold(false);
	ui->tabWidgetFreq->tabBar()->setFont(tab_font_Tune);
		
	// To paint whole qwtPlot widget
	//ui->qwtPlot_Freq->setAutoFillBackground(true);
	//QPalette p = ui->qwtPlot_Freq->palette();
	//p.setColor(QPalette::Window, QColor(Qt::white));
	//ui->qwtPlot_Freq->setPalette(p);

	pds_created_Freq = false;
	y_max = 0;
}

FreqAutoadjustWizard::~FreqAutoadjustWizard()
{
	delete ui;
}


double FreqAutoadjustWizard::maxValue(double *_vec, int n)
{
	if (n <= 0) return 0;

	double max = _vec[0];
	for (int i = 1; i < n; i++)
	{
		if (_vec[i] > max) max = _vec[i];
	}

	return max;
}

void FreqAutoadjustWizard::setDataSet(QString _name, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map)
{
	removeDataSet();

	curve_settings_Freq = new QwtCurveSettings;
	curve_settings_Freq->dataType = QwtCurveSettings::LinesAndSymbols;
	curve_settings_Freq->title = "Freq-curve";

	curve_settings_Freq->symbol->setBrush(QBrush(QColor(Qt::red)));
	curve_settings_Freq->symbol->setSize(4);
	curve_settings_Freq->symbol->setStyle(QwtSymbol::Ellipse);

	QPen symbol_pen(Qt::SolidLine);
	symbol_pen.setColor(QColor(Qt::blue));
	symbol_pen.setWidth(1);
	curve_settings_Freq->symbol->setPen(symbol_pen);

	curve_settings_Freq->pen.setStyle(Qt::SolidLine);
	curve_settings_Freq->pen.setColor(QColor(Qt::darkMagenta));
	curve_settings_Freq->pen.setWidth(1);		

	PlottedDataSet *dataset = new PlottedDataSet(_name, 0, _x, _y, _bad_map, curve_settings_Freq);
	data_Freq = dataset;
	pds_created_Freq = true;

	plotData();	
}

void FreqAutoadjustWizard::removeDataSet()
{
	if (pds_created_Freq) 
	{
		curve_Freq->detach();
		qwttext_max_freq->detach();
		max_line->detach();
		ui->qwtPlot_Freq->replot();

		delete data_Freq;		
		delete curve_Freq;
		delete qwttext_max_freq;
		delete max_line;

		pds_created_Freq = false;
	}
}


void FreqAutoadjustWizard::plotData()
{	
	int sz = data_Freq->getYData()->size();
	if (sz == 0) return;

	double *y_data = new double[sz];
	double *x_data = new double[sz];

	double _max = data_Freq->getYData()->first();
	double freq_max = data_Freq->getXData()->first();
	for (int i = 1; i < data_Freq->getYData()->count(); i++)
	{
		if (data_Freq->getYData()->data()[i] > _max) 
		{
			_max = data_Freq->getYData()->data()[i];
			freq_max = data_Freq->getXData()->data()[i];
		}
	}

	memcpy(x_data, data_Freq->getXData()->data(), sz*sizeof(double));
	memcpy(y_data, data_Freq->getYData()->data(), sz*sizeof(double));

	curve_Freq = new QwtPlotCurve(data_Freq->getName());
	curve_Freq->setRenderHint(QwtPlotItem::RenderAntialiased);

	QwtCurveSettings *cs = data_Freq->getCurveSettings();				
	QColor c = QColor(Qt::magenta);
	c.setAlpha( 150 );
	curve_Freq->setPen(QPen(Qt::NoPen));
	curve_Freq->setBrush(c);
	//curve->setPen(curve_pen);

	QwtCurveSettings::DataType data_type = cs->dataType; 
	if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve_Freq->setStyle(QwtPlotCurve::Lines);
	else curve_Freq->setStyle(QwtPlotCurve::NoCurve);

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
	curve_Freq->setSymbol(sym);

	curve_Freq->setSamples(x_data, y_data, sz);
	curve_Freq->attach(ui->qwtPlot_Freq);

	//double max = 1.25*ui->qwtPlot->axisScaleDiv(QwtPlot::yLeft).interval().maxValue();
	double max = 1.25*maxValue(y_data, sz);
	ui->qwtPlot_Freq->setAxisScale(QwtPlot::yLeft, 0, max);		

	qwttext_max_freq = new QwtPlotTextLabel();
	QString freq_text = QString("Fmax = %1 kHz").arg(QString::number(freq_max, 'g', 4)); 
	QwtText title_maxfreq(freq_text);
	title_maxfreq.setRenderFlags( Qt::AlignRight | Qt::AlignTop );
	QFont font_maxfreq;
	font_maxfreq.setBold( true );
	font_maxfreq.setPointSize(10);
	title_maxfreq.setFont(font_maxfreq);	
	title_maxfreq.setColor(Qt::magenta);
	qwttext_max_freq->setText(title_maxfreq);
	qwttext_max_freq->attach(ui->qwtPlot_Freq);

	/*marker = new QwtPlotMarker();
	marker->attach(ui->qwtPlot_Freq);
	marker->setValue(0.0, 0.0); // location of the label in plot coordinates
	marker->setLabel("title");*/

	max_line = new QwtPlotMarker;
	max_line->setLineStyle(QwtPlotMarker::VLine);
	max_line->setLinePen(QPen(Qt::magenta, 1));
	max_line->attach(ui->qwtPlot_Freq);
	max_line->setXValue(freq_max);

	ui->qwtPlot_Freq->replot();

	delete x_data;
	delete y_data;
	
}

/*
void FreqAutoadjustWizard::changeLocation(Qt::DockWidgetArea area)
{
	if (area == Qt::RightDockWidgetArea || area == Qt::LeftDockWidgetArea)
	{
		//ui->gridLayout_3->removeWidget(ui->tabWidgetAFR);
		//ui->gridLayout_3->removeWidget(ui->tabWidgetRFP);
		//
		//ui->gridLayout_3->addWidget(ui->tabWidgetAFR, 0, 0, 1, 1);
		//ui->gridLayout_3->addWidget(ui->tabWidgetRFP, 1, 0, 1, 1);

		ui->tabWidgetAFR->setTabPosition(QTabWidget::South);		
	}
	else if (area == Qt::BottomDockWidgetArea || area == Qt::NoDockWidgetArea)
	{
		//ui->gridLayout_2->removeWidget(ui->tabWidgetAFR);
		//ui->gridLayout_2->removeWidget(ui->tabWidgetRFP);
		//
		//ui->gridLayout_2->addWidget(ui->tabWidgetAFR, 0, 0, 1, 1);
		//ui->gridLayout_2->addWidget(ui->tabWidgetRFP, 0, 1, 1, 1);

		ui->tabWidgetAFR->setTabPosition(QTabWidget::West);		
	}
}
*/