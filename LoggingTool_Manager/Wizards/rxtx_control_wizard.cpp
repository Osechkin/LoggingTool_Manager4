#include "rxtx_control_wizard.h"


RxTxControlWizard::RxTxControlWizard(QWidget *parent) : QWidget(parent), ui(new Ui::RxTxControlWizard)
{
	ui->setupUi(this);

	// Rx Plot Settings
	ui->qwtPlot_Rx->setCanvasBackground(QBrush(QColor(Qt::white)));
		
	QwtPlotGrid *grid_Rx = new QwtPlotGrid;
	grid_Rx->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_Rx->enableX(true);	
	grid_Rx->enableY(true);	
	grid_Rx->attach(ui->qwtPlot_Rx);	

	QwtPlotPanner *panner_Rx = new QwtPlotPanner(ui->qwtPlot_Rx->canvas());
	panner_Rx->setAxisEnabled(QwtPlot::yRight, false);
	panner_Rx->setMouseButton(Qt::RightButton);	

	QwtPlotPicker *picker_Rx = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlot_Rx->canvas());	
	picker_Rx->setRubberBandPen(QColor(Qt::green));
	picker_Rx->setRubberBand(QwtPicker::CrossRubberBand);
	picker_Rx->setTrackerPen(QColor(Qt::darkMagenta));

	zoomer_Rx = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot_Rx->canvas());
	zoomer_Rx->setEnabled(true);
	const QColor c_Rx(Qt::magenta);
	zoomer_Rx->setRubberBandPen(c_Rx);
	zoomer_Rx->setTrackerPen(c_Rx);

	QwtText yleft_title_Rx;
	QString yleft_title_text_Rx = QString("NMR signal, a.u.");
	yleft_title_Rx.setText(yleft_title_text_Rx);
	yleft_title_Rx.setColor(QColor(Qt::darkBlue));
	QFont yleft_font_Rx = ui->qwtPlot_Rx->axisTitle(QwtPlot::yLeft).font();
	QFont yleft_axis_font_Rx = ui->qwtPlot_Rx->axisFont(QwtPlot::yLeft);
	yleft_axis_font_Rx.setPointSize(8);
	yleft_axis_font_Rx.setBold(false);
	ui->qwtPlot_Rx->setAxisFont(QwtPlot::yLeft, yleft_axis_font_Rx);
	yleft_font_Rx.setPointSize(8);
	yleft_font_Rx.setBold(false);
	yleft_title_Rx.setFont(yleft_font_Rx);
	ui->qwtPlot_Rx->setAxisTitle(QwtPlot::yLeft, yleft_title_Rx);

	QwtText xbotom_title_Rx = ui->qwtPlot_Rx->axisTitle(QwtPlot::xBottom);
	QString xbotom_title_text_Rx = QString("dF, %1").arg(tr("kHz"));
	xbotom_title_Rx.setText(xbotom_title_text_Rx);
	xbotom_title_Rx.setColor(QColor(Qt::darkBlue));
	QFont xbottom_font_Rx = ui->qwtPlot_Rx->axisTitle(QwtPlot::xBottom).font();
	QFont xbottom_axis_font_Rx = ui->qwtPlot_Rx->axisFont(QwtPlot::xBottom);
	xbottom_axis_font_Rx.setPointSize(8);
	xbottom_axis_font_Rx.setBold(false);
	ui->qwtPlot_Rx->setAxisFont(QwtPlot::xBottom, xbottom_axis_font_Rx);
	xbottom_font_Rx.setBold(false);
	xbottom_font_Rx.setPointSize(8);
	xbotom_title_Rx.setFont(xbottom_font_Rx);
	ui->qwtPlot_Rx->setAxisTitle(QwtPlot::xBottom, xbotom_title_Rx);


	// Tx Plot Settings
	ui->qwtPlot_Tx->setCanvasBackground(QBrush(QColor(Qt::white)));

	QwtPlotGrid *grid_Tx = new QwtPlotGrid;
	grid_Tx->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_Tx->enableX(true);	
	grid_Tx->enableY(true);	
	grid_Tx->attach(ui->qwtPlot_Tx);	

	QwtPlotPanner *panner_Tx = new QwtPlotPanner(ui->qwtPlot_Tx->canvas());
	panner_Tx->setAxisEnabled(QwtPlot::yRight, false);
	panner_Tx->setMouseButton(Qt::RightButton);	

	QwtPlotPicker *picker_Tx = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlot_Tx->canvas());	
	picker_Tx->setRubberBandPen(QColor(Qt::green));
	picker_Tx->setRubberBand(QwtPicker::CrossRubberBand);
	picker_Tx->setTrackerPen(QColor(Qt::darkMagenta));

	zoomer_Tx = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot_Tx->canvas());
	zoomer_Tx->setEnabled(true);
	const QColor c_Tx(Qt::magenta);
	zoomer_Tx->setRubberBandPen(c_Tx);
	zoomer_Tx->setTrackerPen(c_Tx);

	QwtText yleft_title_Tx;
	QString yleft_title_text_Tx = QString("NMR signal, a.u.");
	yleft_title_Tx.setText(yleft_title_text_Tx);
	yleft_title_Tx.setColor(QColor(Qt::darkBlue));
	QFont yleft_font_Tx = ui->qwtPlot_Tx->axisTitle(QwtPlot::yLeft).font();
	QFont yleft_axis_font_Tx = ui->qwtPlot_Tx->axisFont(QwtPlot::yLeft);
	yleft_axis_font_Tx.setPointSize(8);
	yleft_axis_font_Tx.setBold(false);
	ui->qwtPlot_Tx->setAxisFont(QwtPlot::yLeft, yleft_axis_font_Tx);
	yleft_font_Tx.setPointSize(8);
	yleft_font_Tx.setBold(false);
	yleft_title_Tx.setFont(yleft_font_Tx);
	ui->qwtPlot_Tx->setAxisTitle(QwtPlot::yLeft, yleft_title_Tx);

	QwtText xbotom_title_Tx = ui->qwtPlot_Tx->axisTitle(QwtPlot::xBottom);
	QString xbotom_title_text_Tx = QString("dF, %1").arg(tr("kHz"));
	xbotom_title_Tx.setText(xbotom_title_text_Tx);
	xbotom_title_Tx.setColor(QColor(Qt::darkBlue));
	QFont xbottom_font_Tx = ui->qwtPlot_Tx->axisTitle(QwtPlot::xBottom).font();
	QFont xbottom_axis_font_Tx = ui->qwtPlot_Tx->axisFont(QwtPlot::xBottom);
	xbottom_axis_font_Tx.setPointSize(8);
	xbottom_axis_font_Tx.setBold(false);
	ui->qwtPlot_Tx->setAxisFont(QwtPlot::xBottom, xbottom_axis_font_Tx);
	xbottom_font_Tx.setBold(false);
	xbottom_font_Tx.setPointSize(8);
	xbotom_title_Tx.setFont(xbottom_font_Tx);
	ui->qwtPlot_Tx->setAxisTitle(QwtPlot::xBottom, xbotom_title_Tx);

	
	// RFP Plot Settings
	/*ui->qwtPlot_RFP->setCanvasBackground(QBrush(QColor(Qt::white)));

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
	QString yleft_title_text_RFP = QString("NMR signal, a.u.");
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
	*/
		
	QFont tab_font_AFR = ui->tabWidgetAFR->tabBar()->font();
	tab_font_AFR.setBold(false);
	ui->tabWidgetAFR->tabBar()->setFont(tab_font_AFR);
	/*QFont tab_font_RFP = ui->tabWidgetRFP->tabBar()->font();
	tab_font_RFP.setBold(false);
	ui->tabWidgetRFP->tabBar()->setFont(tab_font_RFP);*/

	//fr_data = 0;
	pds_created_Rx = false;
	pds_created_Tx = false;
	//pds_created_RFP = false;
}

RxTxControlWizard::~RxTxControlWizard()
{
	delete ui;
}


double RxTxControlWizard::maxValue(double *_vec, int n)
{
	if (n <= 0) return 0;

	double max = _vec[0];
	for (int i = 1; i < n; i++)
	{
		if (_vec[i] > max) max = _vec[i];
	}

	return max;
}

void RxTxControlWizard::setDataSet(uint8_t data_type, QString _name, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map)
{
	removeDataSet(data_type);

	switch (data_type)
	{
	case DT_AFR1_RX:
		{
			curve_settings_Rx = new QwtCurveSettings;
			curve_settings_Rx->dataType = QwtCurveSettings::LinesAndSymbols;
			curve_settings_Rx->title = "Rx-curve";

			curve_settings_Rx->symbol->setBrush(QBrush(QColor(Qt::red)));
			curve_settings_Rx->symbol->setSize(4);
			curve_settings_Rx->symbol->setStyle(QwtSymbol::Ellipse);

			QPen symbol_pen(Qt::SolidLine);
			symbol_pen.setColor(QColor(Qt::blue));
			symbol_pen.setWidth(1);
			curve_settings_Rx->symbol->setPen(symbol_pen);

			curve_settings_Rx->pen.setStyle(Qt::SolidLine);
			curve_settings_Rx->pen.setColor(QColor(Qt::darkMagenta));
			curve_settings_Rx->pen.setWidth(1);		

			PlottedDataSet *dataset = new PlottedDataSet(_name, 0, _x, _y, _bad_map, curve_settings_Rx);
			data_Rx = dataset;
			pds_created_Rx = true;

			plotData(data_type);

			break;
		}
	case DT_AFR2_RX:
		{
			curve_settings_Tx = new QwtCurveSettings;
			curve_settings_Tx->dataType = QwtCurveSettings::LinesAndSymbols;
			curve_settings_Tx->title = "Tx-curve";

			curve_settings_Tx->symbol->setBrush(QBrush(QColor(Qt::red)));
			curve_settings_Tx->symbol->setSize(4);
			curve_settings_Tx->symbol->setStyle(QwtSymbol::Ellipse);

			QPen symbol_pen(Qt::SolidLine);
			symbol_pen.setColor(QColor(Qt::blue));
			symbol_pen.setWidth(1);
			curve_settings_Tx->symbol->setPen(symbol_pen);

			curve_settings_Tx->pen.setStyle(Qt::SolidLine);
			curve_settings_Tx->pen.setColor(QColor(Qt::darkMagenta));
			curve_settings_Tx->pen.setWidth(1);		

			PlottedDataSet *dataset = new PlottedDataSet(_name, 0, _x, _y, _bad_map, curve_settings_Tx);
			data_Tx = dataset;
			pds_created_Tx = true;

			plotData(data_type);

			break;
		}
	/*case DT_RFP:
		{
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

			plotData(data_type);

			break;
		}*/
	}	
}

PlottedDataSet* RxTxControlWizard::getDataSet(uint8_t data_type)
{
	switch (data_type)
	{
	case DT_AFR1_RX:	return data_Rx;		break;
	case DT_AFR2_RX:	return data_Tx;		break;
	default: return NULL;	
	}
}


void RxTxControlWizard::plotData(uint8_t data_type)
{	
	switch (data_type)
	{
	case DT_AFR1_RX:
		{
			int sz = data_Rx->getYData()->size();
	
			double *y_data = new double[sz];
			double *x_data = new double[sz];

			memcpy(x_data, data_Rx->getXData()->data(), sz*sizeof(double));
			memcpy(y_data, data_Rx->getYData()->data(), sz*sizeof(double));

			curve_Rx = new QwtPlotCurve(data_Rx->getName());
			curve_Rx->setRenderHint(QwtPlotItem::RenderAntialiased);

			QwtCurveSettings *cs = data_Rx->getCurveSettings();		
			/*QPen curve_pen = curve->pen();
			curve_pen.setStyle(cs->pen.style());
			curve_pen.setWidth(cs->pen.width());
			curve_pen.setColor(cs->pen.color());*/
			QColor c = QColor(Qt::darkBlue);
			c.setAlpha( 150 );
			curve_Rx->setPen(QPen(Qt::NoPen));
			curve_Rx->setBrush(c);
			//curve->setPen(curve_pen);

			QwtCurveSettings::DataType data_type = cs->dataType; 
			if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve_Rx->setStyle(QwtPlotCurve::Lines);
			else curve_Rx->setStyle(QwtPlotCurve::NoCurve);

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
			curve_Rx->setSymbol(sym);

			curve_Rx->setSamples(x_data, y_data, sz);
			curve_Rx->attach(ui->qwtPlot_Rx);

			//double max = 1.25*ui->qwtPlot->axisScaleDiv(QwtPlot::yLeft).interval().maxValue();
			double max = 1.25*maxValue(y_data, sz);
			ui->qwtPlot_Rx->setAxisScale(QwtPlot::yLeft, 0, max);		
		
			ui->qwtPlot_Rx->replot();

			delete x_data;
			delete y_data;

			break;
		}
	case DT_AFR2_RX:
		{
			int sz = data_Tx->getYData()->size();
	
			double *y_data = new double[sz];
			double *x_data = new double[sz];

			memcpy(x_data, data_Tx->getXData()->data(), sz*sizeof(double));
			memcpy(y_data, data_Tx->getYData()->data(), sz*sizeof(double));

			curve_Tx = new QwtPlotCurve(data_Tx->getName());
			curve_Tx->setRenderHint(QwtPlotItem::RenderAntialiased);

			QwtCurveSettings *cs = data_Tx->getCurveSettings();		
			QColor c = QColor(Qt::darkBlue);
			c.setAlpha( 150 );
			curve_Tx->setPen(QPen(Qt::NoPen));
			curve_Tx->setBrush(c);
			
			QwtCurveSettings::DataType data_type = cs->dataType; 
			if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve_Tx->setStyle(QwtPlotCurve::Lines);
			else curve_Tx->setStyle(QwtPlotCurve::NoCurve);

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
			curve_Tx->setSymbol(sym);

			curve_Tx->setSamples(x_data, y_data, sz);
			curve_Tx->attach(ui->qwtPlot_Tx);

			//double max = 1.25*ui->qwtPlot->axisScaleDiv(QwtPlot::yLeft).interval().maxValue();
			double max = 1.25*maxValue(y_data, sz);
			ui->qwtPlot_Tx->setAxisScale(QwtPlot::yLeft, 0, max);		
		
			ui->qwtPlot_Tx->replot();

			delete x_data;
			delete y_data;

			break;
		}
	/*case DT_RFP:
		{
			int sz = data_RFP->getYData()->size();
	
			double *y_data = new double[sz];
			double *x_data = new double[sz];

			memcpy(x_data, data_RFP->getXData()->data(), sz*sizeof(double));
			memcpy(y_data, data_RFP->getYData()->data(), sz*sizeof(double));

			curve_RFP = new QwtPlotCurve(data_RFP->getName());
			curve_RFP->setRenderHint(QwtPlotItem::RenderAntialiased);

			QwtCurveSettings *cs = data_RFP->getCurveSettings();		
			//QColor c = QColor(Qt::darkBlue);
			//c.setAlpha( 150 );
			//rfp_curve->setPen(QPen(Qt::NoPen));
			//rfp_curve->setBrush(c);			

			QwtCurveSettings::DataType data_type = cs->dataType; 
			if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve_RFP->setStyle(QwtPlotCurve::Lines);
			else curve_RFP->setStyle(QwtPlotCurve::NoCurve);

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
			curve_RFP->setSymbol(sym);

			curve_RFP->setSamples(x_data, y_data, sz);
			curve_RFP->attach(ui->qwtPlot_RFP);

			double max = 1.25*maxValue(y_data, sz);
			ui->qwtPlot_RFP->setAxisScale(QwtPlot::yLeft, 0, max);		
		
			ui->qwtPlot_RFP->replot();

			delete x_data;
			delete y_data;

			break;
		}*/
	}
	
}

void RxTxControlWizard::removeDataSet(uint8_t data_type)
{
	if (data_type == DT_AFR1_RX)
	{
		if (pds_created_Rx) 
		{
			curve_Rx->detach();
			ui->qwtPlot_Rx->replot();

			delete data_Rx;		
			delete curve_Rx;

			pds_created_Rx = false;
		}
	}
	else if (data_type == DT_AFR2_RX)
	{
		if (pds_created_Tx) 
		{
			curve_Tx->detach();
			ui->qwtPlot_Tx->replot();

			delete data_Tx;		
			delete curve_Tx;

			pds_created_Tx = false;
		}
	}
	/*else if (data_type == DT_RFP)
	{
		if (pds_created_RFP)
		{
			curve_RFP->detach();
			ui->qwtPlot_RFP->replot();

			delete data_RFP;		
			delete curve_RFP;

			pds_created_RFP = false;
		}
	}	*/
}


void RxTxControlWizard::changeLocation(Qt::DockWidgetArea area)
{
	if (area == Qt::RightDockWidgetArea || area == Qt::LeftDockWidgetArea)
	{
		/*ui->gridLayout_3->removeWidget(ui->tabWidgetAFR);
		ui->gridLayout_3->removeWidget(ui->tabWidgetRFP);

		ui->gridLayout_3->addWidget(ui->tabWidgetAFR, 0, 0, 1, 1);
		ui->gridLayout_3->addWidget(ui->tabWidgetRFP, 1, 0, 1, 1);*/

		ui->tabWidgetAFR->setTabPosition(QTabWidget::South);		
	}
	else if (area == Qt::BottomDockWidgetArea || area == Qt::NoDockWidgetArea)
	{
		/*ui->gridLayout_2->removeWidget(ui->tabWidgetAFR);
		ui->gridLayout_2->removeWidget(ui->tabWidgetRFP);

		ui->gridLayout_2->addWidget(ui->tabWidgetAFR, 0, 0, 1, 1);
		ui->gridLayout_2->addWidget(ui->tabWidgetRFP, 0, 1, 1, 1);*/

		ui->tabWidgetAFR->setTabPosition(QTabWidget::West);		
	}
}