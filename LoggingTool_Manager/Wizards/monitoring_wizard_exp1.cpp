#include "../io_general.h"
#include "monitoring_wizard_exp1.h"


MonitoringWidget::MonitoringWidget(QSettings *app_settings, QWidget *parent) : QWidget(parent), ui(new Ui::MonitoringWidgetExp1)
{
	ui->setupUi(this);
	this->setObjectName("tabMonitoring");

	base_date_time = QDateTime::currentDateTime();
		
	QPalette p = ui->frameToolBar->palette(); 
	p.setColor(QPalette::Background, QColor(Qt::magenta).lighter(195));
	ui->frameToolBar->setAutoFillBackground(true);
	ui->frameToolBar->setPalette(p);
	ui->frameToolBar->show();

	ui->tbtLRDec->setIcon(QIcon(":/images/left-right_dec.png"));
	ui->tbtLRInc->setIcon(QIcon(":/images/left-right_inc.png"));
	ui->tbtUDDec->setIcon(QIcon(":/images/up-down_dec.png"));
	ui->tbtUDInc->setIcon(QIcon(":/images/up-down_inc.png"));

	ui->tbtClearAll->setIcon(QIcon(":/images/Eraser.png"));
	ui->tbtFindAll->setIcon(QIcon(":/images/find_signal.png"));
	ui->tbtAdd->setIcon(QIcon(":/images/Add2.png"));
	ui->tbtClear->setIcon(QIcon(":/images/Delete.png"));
	//ui->tbtSettings->setIcon(QIcon(":/images/settings.png"));

	ui->tbtLRDec->setToolTip(tr("Decrease limits on the horizontal scale"));
	ui->tbtLRInc->setToolTip(tr("Increase limits on the horizontal scale"));
	ui->tbtUDDec->setToolTip(tr("Decrease limits on the depth scale"));
	ui->tbtUDInc->setToolTip(tr("Increase limits on the depth scale"));
	ui->tbtAdd->setToolTip(tr("Add one more Monitoring Panel"));
	ui->tbtClear->setToolTip(tr("Remove current Monitoring Panel"));
	//ui->tbtSettings->setToolTip(tr("Edit settings"));
	ui->tbtClearAll->setToolTip(tr("Remove all Monitoring Panels"));
	ui->tbtFindAll->setToolTip(tr("Rescale to show All"));

	depth_from = 0;
	depth_to = 100.0;
	ui->cntDepthFrom->setValue(depth_from);
	ui->cntDepthRange->setValue(depth_to - depth_from);

	//addMonitoringBoard();
	
	color_list << QColor(Qt::red) << QColor(Qt::blue) << QColor(Qt::magenta) << QColor(Qt::green);
	color_list << QColor(Qt::darkYellow) << QColor(Qt::darkRed) << QColor(Qt::darkBlue) << QColor(Qt::darkGreen);
	color_list << QColor(Qt::darkMagenta) << QColor(Qt::darkCyan);

	setConnections();
	
	addPanel();
	addPanel();
}

MonitoringWidget::~MonitoringWidget()
{	
	qDeleteAll(plot_frames.begin(), plot_frames.end());

	delete ui;
}


void MonitoringWidget::setConnections()
{
	connect(ui->tbtAdd, SIGNAL(clicked()), this, SLOT(addPanel()));
	connect(ui->tbtClear, SIGNAL(clicked()), this, SLOT(clearPanel()));
	connect(ui->tbtClearAll, SIGNAL(clicked()), this, SLOT(clearAllPanels()));
	connect(ui->cntDepthFrom, SIGNAL(valueChanged(double)), this, SLOT(changeDepthFrom(double)));
	connect(ui->cntDepthRange, SIGNAL(valueChanged(double)), this, SLOT(changeDepthRange(double)));

}

void MonitoringWidget::addPanel()
{	
	QList<ScannedQuantity> data_types;
	data_types << ScannedQuantity::Temperature << ScannedQuantity::Voltage << ScannedQuantity::Current;
	ScannedQuantity data_type = ScannedQuantity::NoType;
	for (int i = 0; i < plot_frames.count(); i++)
	{
		MonitoringFrame *plot_frame = plot_frames[i];
		if (data_types.contains(plot_frame->getDataType())) data_types.removeOne(plot_frame->getDataType());
	}
	if (!data_types.isEmpty()) data_type = data_types.first();

	MonitoringFrame *plot_frame = new MonitoringFrame(data_type, ui->splitter);
	plot_frame->plot()->setAxisScale(QwtPlot::yLeft, depth_to, depth_from);
	plot_frame->plot()->setAxisScale(QwtPlot::yRight, depth_to, depth_from);
	plot_frame->plot()->setAxisScale(QwtPlot::xBottom, 0, 100, 20);
	plot_frame->plot()->setAxisScale(QwtPlot::xTop, 0, 100, 20);
	plot_frame->zoomer()->setZoomBase(true);
	ui->splitter->addWidget(plot_frame);
	plot_frames.append(plot_frame);

	QwtText title_depth;
	QString title_depth_text = QString("Depth, m");
	title_depth.setText(title_depth_text);
	title_depth.setColor(QColor(Qt::darkBlue));
	QFont font_depth = plot_frame->plot()->axisTitle(QwtPlot::yLeft).font();
	QFont axis_font = plot_frame->plot()->axisFont(QwtPlot::yLeft);
	axis_font.setPointSize(9);
	axis_font.setBold(false);
	font_depth.setPointSize(10);
	font_depth.setBold(false);
	title_depth.setFont(font_depth);

	plot_frame->plot()->enableAxis(QwtPlot::xTop, false);
	plot_frame->plot()->enableAxis(QwtPlot::xBottom, true);
	if (plot_frames.count() == 1)
	{
		plot_frame->plot()->enableAxis(QwtPlot::yLeft, true);
		plot_frame->plot()->enableAxis(QwtPlot::yRight, true);

		plot_frame->vline()->setVisible(false);

		plot_frame->plot()->setAxisTitle(QwtPlot::yLeft, title_depth);
	}
	else if (plot_frames.count() == 2)
	{
		MonitoringFrame *_plot_frame1 = plot_frames.first();
		MonitoringFrame *_plot_frame2 = plot_frame;

		_plot_frame1->plot()->enableAxis(QwtPlot::yLeft, true);
		_plot_frame1->plot()->enableAxis(QwtPlot::yRight, false);
		_plot_frame2->plot()->enableAxis(QwtPlot::yLeft, false);
		_plot_frame2->plot()->enableAxis(QwtPlot::yRight, true);

		_plot_frame1->plot()->setAxisTitle(QwtPlot::yLeft, title_depth);
		_plot_frame1->label2()->setVisible(false);
		_plot_frame2->label1()->setVisible(false);

		_plot_frame1->vline()->setVisible(true);
		_plot_frame2->vline()->setVisible(false);
	}
	else for (int i = 0; i < plot_frames.count(); i++)
	{
		MonitoringFrame *_plot_frame = plot_frames.at(i);
		if (i == 0)
		{
			_plot_frame->plot()->enableAxis(QwtPlot::yLeft, true);
			_plot_frame->plot()->enableAxis(QwtPlot::yRight, false);

			_plot_frame->label2()->setVisible(false);

			_plot_frame->vline()->setVisible(true);

			_plot_frame->plot()->setAxisTitle(QwtPlot::yLeft, title_depth);
		}
		else if (i == plot_frames.count()-1)
		{
			_plot_frame->plot()->enableAxis(QwtPlot::yLeft, false);
			_plot_frame->plot()->enableAxis(QwtPlot::yRight, true);	

			_plot_frame->label1()->setVisible(false);

			_plot_frame->vline()->setVisible(false);
		}
		else
		{
			_plot_frame->plot()->enableAxis(QwtPlot::yLeft, false);
			_plot_frame->plot()->enableAxis(QwtPlot::yRight, false);	

			_plot_frame->label1()->setVisible(false);
			_plot_frame->label2()->setVisible(false);

			_plot_frame->vline()->setVisible(true);
		}
	}		

	connect(plot_frame, SIGNAL(activated()), this, SLOT(panelActivated()));
	connect(plot_frame, SIGNAL(plot_rescaled(void*)), this, SLOT(rescaleAllPlots(void*)));
	//active_frame = NULL;
}

void MonitoringWidget::clearPanel()
{	
	for (int i = 0; i < plot_frames.count(); i++)
	{
		if (plot_frames.at(i) == active_frame)
		{
			MonitoringFrame *frame = plot_frames.takeAt(i);
			delete frame;
			active_frame = NULL;
			break;
		}
	}
}

void MonitoringWidget::clearAllPanels()
{
	if (plot_frames.isEmpty()) return;
	qDeleteAll(plot_frames.begin(), plot_frames.end());
	plot_frames.clear();
	active_frame = NULL;
}

void MonitoringWidget::panelActivated()
{
	MonitoringFrame* _active_frame = (MonitoringFrame*)sender();
	for (int i = 0; i < plot_frames.count(); i++)
	{
		MonitoringFrame *plot_frame = plot_frames[i];
		if (plot_frame == _active_frame)
		{
			active_frame = _active_frame;			
		}
		else
		{
			// change palette to default one
			plot_frame->setActive(false);
			plot_frame->setDefaultPalette();
		}
	}
}

void MonitoringWidget::setDepthFrom(double _depth_from)
{

}

void MonitoringWidget::setDepthTo(double _depth_to)
{

}

void MonitoringWidget::changeDepthRange(double val)
{
	double cur_max_depth = ui->cntDepthFrom->value();
	double cur_min_depth = val + cur_max_depth;
	for (int i = 0; i < plot_frames.count(); i++)
	{		
		MonitoringFrame *plot_frame = plot_frames.at(i);
		QwtPlot *qwt_plot = plot_frame->plot();		

		qwt_plot->setAxisScale(QwtPlot::yLeft, cur_min_depth, cur_max_depth);
		qwt_plot->setAxisScale(QwtPlot::yRight, cur_min_depth, cur_max_depth);

		qwt_plot->replot();		
	}
}

void MonitoringWidget::changeDepthFrom(double val)
{		
	double cur_min_depth = val + ui->cntDepthRange->value();
	double cur_max_depth = val;
	for (int i = 0; i < plot_frames.count(); i++)
	{		
		MonitoringFrame *plot_frame = plot_frames.at(i);
		QwtPlot *qwt_plot = plot_frame->plot();

		qwt_plot->setAxisScale(QwtPlot::yLeft, cur_min_depth, cur_max_depth);
		qwt_plot->setAxisScale(QwtPlot::yRight, cur_min_depth, cur_max_depth);

		qwt_plot->replot();		
	}
}

void MonitoringWidget::rescaleAllPlots(void *qwtplot_obj)
{
	QwtPlot *qwtPlot = (QwtPlot*)qwtplot_obj;
	double cur_min_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
	double cur_max_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();		
	for (int i = 0; i < plot_frames.count(); i++)
	{
		MonitoringFrame *plot_frame = plot_frames.at(i);
		QwtPlot *qwt_plot = plot_frame->plot();
		
		qwt_plot->setAxisScale(QwtPlot::yLeft, cur_min_depth, cur_max_depth);
		qwt_plot->setAxisScale(QwtPlot::yRight, cur_min_depth, cur_max_depth);
		
		qwt_plot->replot();		
	}

	disconnect(ui->cntDepthFrom, SIGNAL(valueChanged(double)), this, SLOT(changeDepthFrom(double)));
	disconnect(ui->cntDepthRange, SIGNAL(valueChanged(double)), this, SLOT(changeDepthRange(double)));
	ui->cntDepthFrom->setValue(cur_max_depth);
	ui->cntDepthRange->setValue(cur_min_depth-cur_max_depth);
	connect(ui->cntDepthFrom, SIGNAL(valueChanged(double)), this, SLOT(changeDepthFrom(double)));
	connect(ui->cntDepthRange, SIGNAL(valueChanged(double)), this, SLOT(changeDepthRange(double)));
}


QPair<QDateTime, double> MonitoringWidget::lastVoltagePoint(int index) 
{ 
	/*
	QDateTime dtime = voltageData[index]->time->last();
	double vol = voltageData[index]->data->last();
	QPair<QDateTime, double> out = QPair<QDateTime, double>(dtime, vol); 

	return out;
	*/
	return QPair<QDateTime, double>();
}

QPair<QDateTime, double> MonitoringWidget::lastTemperaturePoint(int index) 
{ 
	/*
	QDateTime dtime = temperatureData[index]->time->last();
	double temp = temperatureData[index]->data->last();
	QPair<QDateTime, double> out = QPair<QDateTime, double>(dtime, temp); 

	return out;
	*/
	return QPair<QDateTime, double>();
}

QPair<QDateTime, double> MonitoringWidget::lastCurrentPoint(int index) 
{ 
	/*
	QDateTime dtime = currentData[index]->time->last();
	double cur = currentData[index]->data->last();
	QPair<QDateTime, double> out = QPair<QDateTime, double>(dtime, cur); 

	return out;
	*/
	return QPair<QDateTime, double>();
}


void MonitoringWidget::setCurveProperties(QwtPlotCurve *_curve, ScannedQuantity _data_type, int num)
{

}


void MonitoringWidget::initMonitoringObjects()
{		
	
	/*temperatureData.clear();
	addMonitoringBoard("DT_T", QList<double>() << 0 << 80 << 0 << 80, QList<double>() << -20 << 100 << -20 << 100, QPair<bool,int>(false, TIME_RANGE));	
	addMonitoringBoard("TU", QList<double>() << 0 << 80 << 0 << 80, QList<double>() << -20 << 100 << -20 << 100, QPair<bool,int>(false, TIME_RANGE));	
	addMonitoringBoard("PU", QList<double>() << 0 << 80 << 0 << 80, QList<double>() << -20 << 100 << -20 << 100, QPair<bool,int>(false, TIME_RANGE));	
	*/
}

void MonitoringWidget::addMonitoringTemperature(int num, QString _name)
{
	Monitor_Data *mon_temp = new Monitor_Data();
	QwtPlotCurve *curve = mon_temp->curve;
	QString name = _name.split("#").first();
	name += QString("#%1").arg(num+1);
	curve->setTitle(name);
	curve->setRenderHint(QwtPlotItem::RenderAntialiased);	

	curve->setStyle(QwtPlotCurve::Lines);

	int color_index = num % color_list.count();
	QColor color = color_list.at(color_index);

	QPen curve_pen = Qt::SolidLine;
	curve_pen.setWidth(1);
	curve_pen.setColor(color);
	curve->setPen(curve_pen);

	QwtSymbol *sym = new QwtSymbol;
	sym->setStyle(QwtSymbol::Ellipse);
	sym->setSize(4);
	QPen sym_pen;
	sym_pen.setColor(QColor(Qt::black));
	sym_pen.setWidth(1);
	sym->setPen(sym_pen);
	QBrush sym_brush;
	sym_brush.setColor(color);
	sym_brush.setStyle(Qt::SolidPattern);
	sym->setBrush(sym_brush);
	curve->setSymbol(sym); 

	temperatureData.append(mon_temp);
}

void MonitoringWidget::addMonitoringVoltage(int num, QString _name)
{
	Monitor_Data *mon_volt = new Monitor_Data();
	QwtPlotCurve *curve = mon_volt->curve;
	QString name = _name.split("#").first();
	name += QString("#%1").arg(num+1);
	curve->setTitle(name);
	curve->setRenderHint(QwtPlotItem::RenderAntialiased);	

	curve->setStyle(QwtPlotCurve::Lines);

	int color_index = num % color_list.count();
	QColor color = color_list.at(color_index);

	QPen curve_pen = Qt::SolidLine;
	curve_pen.setWidth(1);
	curve_pen.setColor(color);
	curve->setPen(curve_pen);

	QwtSymbol *sym = new QwtSymbol;
	sym->setStyle(QwtSymbol::Ellipse);
	sym->setSize(4);
	QPen sym_pen;
	sym_pen.setColor(QColor(Qt::black));
	sym_pen.setWidth(1);
	sym->setPen(sym_pen);
	QBrush sym_brush;
	sym_brush.setColor(color);
	sym_brush.setStyle(Qt::SolidPattern);
	sym->setBrush(sym_brush);
	curve->setSymbol(sym); 

	voltageData.append(mon_volt);
}


void MonitoringWidget::clearAll(MonitoringFrame *_frame)
{
	QwtPlot *qwtPlot = _frame->plot();


	/*for (int i = 0; i < qwt_curve_list.count(); i++)
	{
		QwtPlotCurve *curve = qwt_curve_list.at(i);
		curve->detach();
	}
	qDeleteAll(qwt_curve_list.begin(), qwt_curve_list.end());
	qwt_curve_list.clear();

	QObjectList louts = legend_vlout->children();
	for (int i = 0; i < louts.count(); i++)
	{
		QHBoxLayout *item = qobject_cast<QHBoxLayout*>(louts.at(i));		
		while (QLayoutItem *sub_item = item->takeAt(0))
		{
			QWidget *widget = sub_item->widget();
			item->removeWidget(widget);
			delete widget;			
		}
		legend_vlout->removeItem(item);
	}
	qDeleteAll(louts.begin(), louts.end());
	louts.clear();

	legend_frame->repaint();*/

	//calibration_store.clear();
	//calibr_normalize_coef = 1.0;

	//cur_index = -1;

	
	bool autoDelete = false;
	qwtPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, autoDelete);
}


void MonitoringWidget::addData(uint8_t _comm_id, QString _name, QPair<bool,double> dpt, QVector<double> *_depth_data, QVector<double> *_data)
{	
	bool temp_ok = true;

	switch (_comm_id)
	{
	case DT_T:
		{
			for (int j = 0; j < plot_frames.size(); j++)
			{
				if (plot_frames[j]->getDataType() == ScannedQuantity::Temperature) 
				{					
					plot_frames[j]->clearAll();
				}				
			}

			uint bs = base_date_time.toTime_t();

			int idx = 0;
			for (int i = 0; i < _data->size(); i+=2)
			{				
				int chNum = (int)_data->at(i);			// the number of temperature channel
				double T0 = _data->at(i+1);
				if (T0 < MIN_CRITICAL_TEMP || T0 > MAX_CRITICAL_TEMP || T0 != T0) return;		// check temperature data

				bool new_log = false;
				if (idx >= temperatureData.size()) 
				{
					addMonitoringTemperature(idx, _name);
					new_log = true;
				}

				QVector<QDateTime> *dt0 = temperatureData[idx]->time;			
				dt0->push_back(QDateTime::currentDateTime());
				QVector<double> *dtime0 = temperatureData[idx]->dtime;
				dtime0->push_back(dt0->last().toTime_t()-bs);
				QVector<double> *depths = temperatureData[idx]->depth;
				depths->push_back(dpt.second);
				QVector<double> *temp0 = temperatureData[idx]->data;				
				temp0->push_back(T0);

				for (int j = 0; j < plot_frames.size(); j++)
				{
					if (plot_frames[j]->getDataType() == ScannedQuantity::Temperature) 
					{						
						QwtPlotCurve *temp0_curve = temperatureData[idx]->curve;
						temp0_curve->setSamples(temp0->data(), depths->data(), temp0->size());

						plot_frames[j]->addCurve(temp0_curve);
					}
				}		
				idx++;
			}
			refreshPlotFrameHeaders();
			break;
		}	
	case DT_U:
		{
			for (int j = 0; j < plot_frames.size(); j++)
			{
				if (plot_frames[j]->getDataType() == ScannedQuantity::Voltage) 
				{
					plot_frames[j]->clearAll();
				}
			}

			uint bs = base_date_time.toTime_t();

			for (int i = 0; i < _data->size(); i++)
			{				
				double U0 = _data->at(i);
				if (U0 < -1000 || U0 > 1000 || U0 != U0) return;		// check voltage data

				bool new_log = false;
				if (i >= voltageData.size()) 
				{
					addMonitoringVoltage(i, _name);
					new_log = true;
				}
								
				QVector<QDateTime> *dt0 = voltageData[i]->time;			
				dt0->push_back(QDateTime::currentDateTime());
				QVector<double> *dtime0 = voltageData[i]->dtime;
				dtime0->push_back(dt0->last().toTime_t()-bs);
				QVector<double> *depths = voltageData[i]->depth;
				depths->push_back(dpt.second);
				QVector<double> *volt0 = voltageData[i]->data;				
				volt0->push_back(U0);
				
				for (int j = 0; j < plot_frames.size(); j++)
				{
					if (plot_frames[j]->getDataType() == ScannedQuantity::Voltage) 
					{						
						QwtPlotCurve *volt_curve = voltageData[i]->curve;
						volt_curve->setSamples(volt0->data(), depths->data(), volt0->size());

						plot_frames[j]->addCurve(volt_curve);
					}
				}
			}
			refreshPlotFrameHeaders();
			break;
		}	
	default: break;
	}

	emit temperature_status(temp_ok ? TempOK : TempHigh);	
}

void MonitoringWidget::refreshPlotFrameHeaders()
{
	int max_items = 0;
	for (int j = 0; j < plot_frames.size(); j++)
	{
		if (plot_frames[j]->itemsCount() > max_items) max_items = plot_frames[j]->itemsCount();
	}

	for (int j = 0; j < plot_frames.size(); j++)
	{
		if (max_items <= 1)
		{
			plot_frames[j]->setMinimumHeightHeader(32);
			plot_frames[j]->setMaximumHeightHeader(32);			
		}
		else
		{
			plot_frames[j]->setMinimumHeightHeader(max_items*16+16);
			plot_frames[j]->setMaximumHeightHeader(max_items*16+16);
		}	
	} 
}