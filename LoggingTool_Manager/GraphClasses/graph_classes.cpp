#include <QMenu>
#include <QStyleFactory>
#include <QMessageBox>

#include "qwt_scale_widget.h"
#include "qwt_symbol.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_curve.h"

#include "graph_classes.h"

#include "../Dialogs/axis_dialog.h"
#include "../Dialogs/plot_dialog.h"
#include "../Dialogs/curve_dialog.h"
#include "../Dialogs/mark_dialog.h"

#include "../Common/app_settings.h"

#include "../tools_general.h"



PlotZoomer::PlotZoomer(int xAxis, int yAxis, /*QwtPlotCanvas*/ QWidget *canvas) : QwtPlotZoomer(xAxis, yAxis, canvas)
{
	setTrackerMode(QwtPicker::AlwaysOff);
	setRubberBand(QwtPicker::RectRubberBand);

	QVector<MousePattern> mouse_patterns;
	mouse_patterns << MousePattern(Qt::LeftButton);
	for (int i = 1; i < QwtEventPattern::MousePatternCount; ++i) mouse_patterns << MousePattern();
	setMousePattern(mouse_patterns);
}

bool PlotZoomer::end(bool ok)
{
	ok = QwtPlotPicker::end(ok);
	if (!ok) return false;

	QwtPlot *plot = QwtPlotZoomer::plot();
	if ( !plot ) return false;

	const QPolygon &polygon = selection();
	if ( polygon.count() < 2 ) return false;

	QRect rect = QRect(polygon[0], polygon[int(polygon.count() - 1)]);

	if (rect.width() >= 0)
	{
		QRectF zoomRect = invTransform(rect).normalized();
		const QSizeF minSize = minZoomSize();
		if (minSize.isValid())
		{
			const QPointF center = zoomRect.center();
			zoomRect.setSize(zoomRect.size().expandedTo(minZoomSize()));
			zoomRect.moveCenter(center);
		}		
		zoom( zoomRect );		
	} 
	else 
	{
		zoom(0);		
	}
	
	return true;
}

bool PlotZoomer::accept(QPolygon &polygon) const
{
	if (polygon.count() < 2) return false;

	QRect rect = QRect(polygon[0], polygon[int(polygon.count()) - 1]);
	rect = rect.normalized();

	const int minSize = 2;
	if (rect.width() < minSize && rect.height() < minSize) return false;

	const int minZoomSize = 11;
	const QPoint center = rect.center();
	rect.setSize(rect.size().expandedTo(QSize(minZoomSize, minZoomSize)));
	rect.moveCenter(center);
	polygon[0] = polygon[0];						// rect.topLeft();
	polygon[1] = polygon[int(polygon.count()) - 1];	//rect.bottomRight();
	polygon.resize(2);

	return true;
}


InversePlotZoomer::InversePlotZoomer(int xAxis, int yAxis, /*QwtPlotCanvas*/ QWidget *canvas) : QwtPlotZoomer(xAxis, yAxis, canvas)
{
	setTrackerMode(QwtPicker::AlwaysOff);
	setRubberBand(QwtPicker::RectRubberBand);

	QVector<MousePattern> mouse_patterns;
	mouse_patterns << MousePattern(Qt::LeftButton);
	for (int i = 1; i < QwtEventPattern::MousePatternCount; ++i) mouse_patterns << MousePattern();
	setMousePattern(mouse_patterns);
}

bool InversePlotZoomer::end(bool ok)
{
	ok = QwtPlotPicker::end(ok);
	if (!ok) return false;

	QwtPlot *plot = QwtPlotZoomer::plot();
	if ( !plot ) return false;

	const QPolygon &polygon = selection();
	if ( polygon.count() < 2 ) return false;

	QRect rect = QRect(polygon[0], polygon[int(polygon.count() - 1)]);

	if (rect.width() >= 0)
	{
		QRectF zoomRect = invTransform(rect).normalized();
		const QSizeF minSize = minZoomSize();
		if (minSize.isValid())
		{
			const QPointF center = zoomRect.center();
			zoomRect.setSize(zoomRect.size().expandedTo(minZoomSize()));
			zoomRect.moveCenter(center);
		}		
		zoom( zoomRect );			
	} 
	else 
	{
		zoom(0);		
	}

	return true;
}

bool InversePlotZoomer::accept(QPolygon &polygon) const
{
	if (polygon.count() < 2) return false;

	QRect rect = QRect(polygon[0], polygon[int(polygon.count()) - 1]);
	rect = rect.normalized();

	const int minSize = 2;
	if (rect.width() < minSize && rect.height() < minSize) return false;

	const int minZoomSize = 11;
	const QPoint center = rect.center();
	rect.setSize(rect.size().expandedTo(QSize(minZoomSize, minZoomSize)));
	rect.moveCenter(center);
	polygon[0] = polygon[0];						// rect.topLeft();
	polygon[1] = polygon[int(polygon.count()) - 1];	//rect.bottomRight();
	polygon.resize(2);

	return true;
}


DataPlot::DataPlot(QString &objectName, QSettings *settings, QWidget *parent)
{
	setParent(parent);
	this->setObjectName(objectName);
	app_settings = settings;

	QGridLayout *grid_layout = new QGridLayout(this);
	grid_layout->setContentsMargins(1,3,1,1);
	grid_layout->setVerticalSpacing(1);
	
	frame = new QFrame(this);
	QGridLayout *grlout_frame = new QGridLayout(frame);
	grlout_frame->setContentsMargins(1,1,1,1);
	frame->setFrameShape(QFrame::StyledPanel);
	frame->setFrameShadow(QFrame::Plain);
	frame->setContentsMargins(1,1,1,1);
	
	qwtPlot = new QwtPlot(frame);	
	qwtPlot->setContentsMargins(1,1,1,1);
	//qwtPlot->setCanvasBackground(QColor(Qt::white));
	grlout_frame->addWidget(qwtPlot, 0, 0, 1, 1);
	
	grid = new QwtPlotGrid;
	grid->setMajorPen(QPen(QBrush(QColor(Qt::gray)),1,Qt::DashLine));
	grid->setMinorPen(QPen(QBrush(QColor(Qt::gray)),0.5,Qt::DotLine));
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(true);
	grid->attach(qwtPlot);	

	zoomer = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, qwtPlot->canvas());
	zoomer->setEnabled(false);
	const QColor c(Qt::magenta);
	zoomer->setRubberBandPen(c);
	zoomer->setTrackerPen(c);
	
	panner = new QwtPlotPanner(qwtPlot->canvas());
	panner->setAxisEnabled(QwtPlot::yRight, false);
	panner->setMouseButton(Qt::RightButton);	

	picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, qwtPlot->canvas());	
	picker->setRubberBandPen(QColor(Qt::green));
	picker->setRubberBand(QwtPicker::CrossRubberBand);
	picker->setTrackerPen(QColor(Qt::darkMagenta));
	
	
	frame_toolbar = new QFrame(this);
	frame_toolbar->setFrameShape(QFrame::StyledPanel);
	frame_toolbar->setFrameShadow(QFrame::Plain);	
	QSizePolicy toolbar_sizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	toolbar_sizePolicy.setHorizontalStretch(0);
	toolbar_sizePolicy.setVerticalStretch(0);
	toolbar_sizePolicy.setHeightForWidth(frame_toolbar->sizePolicy().hasHeightForWidth());
	frame_toolbar->setSizePolicy(toolbar_sizePolicy);
	//frame_toolbar->setFrameShape(QFrame::WinPanel);
	//frame_toolbar->setFrameShadow(QFrame::Raised);
	//QVBoxLayout *verticalLayout = new QVBoxLayout(frame_toolbar);
	QHBoxLayout *horizontalLayout = new QHBoxLayout(frame_toolbar);
	horizontalLayout->setSpacing(2);
	horizontalLayout->setContentsMargins(1, 1, 1, 1);
		
	a_auto_resizeH = new QAction(QIcon(":/images/scale_horizontal.png"), tr("Horizontal"), this);	
	a_auto_resizeH->setCheckable(true);
	a_auto_resizeH->setChecked(false);
	
	a_auto_resizeV = new QAction(QIcon(":/images/scale_vertical.png"), tr("Vertical"), this);
	a_auto_resizeV->setCheckable(true);
	a_auto_resizeV->setChecked(false);

	QMenu *button_menu = new QMenu(this);
	//button_menu->setStyle(new MyQActionStyle("Fusion"));
	button_menu->addAction(a_auto_resizeH);
	button_menu->addAction(a_auto_resizeV);
	
	tbtAutoResize = new QToolButton(frame_toolbar);
	tbtAutoResize->setIconSize(QSize(24,24));		// 36,36
	tbtAutoResize->setIcon(QIcon(":/images/scale_both_24.png"));	
	tbtAutoResize->setAutoRaise(true);
	tbtAutoResize->setCheckable(true);
	tbtAutoResize->setChecked(false);
	tbtAutoResize->setPopupMode(QToolButton::MenuButtonPopup);
	tbtAutoResize->setMenu(button_menu);

	tbtFindSignal = new QToolButton(frame_toolbar);
	tbtFindSignal->setAutoRaise(true);
	tbtFindSignal->setIconSize(QSize(24,24));		// 36, 36
	//tbtFindSignal->setIcon(QIcon(":/images/find_signal_blue.png"));
	tbtFindSignal->setIcon(QIcon(":/images/axis_left_minmax_24.png"));
	tbtFindSignal->setCheckable(false);	
	tbtFindSignal->setToolTip(tr("Rescale to Show All"));

	tbtAxisYZeroDown = new QToolButton(frame_toolbar);
	tbtAxisYZeroDown->setAutoRaise(true);
	tbtAxisYZeroDown->setIconSize(QSize(24,24));	// 36, 36
	tbtAxisYZeroDown->setIcon(QIcon(":/images/axis_left_downzero_24.png"));
	tbtAxisYZeroDown->setCheckable(false);	
	tbtAxisYZeroDown->setToolTip(tr("Rescale to Show All"));

	tbtAxisYZeroCenter = new QToolButton(frame_toolbar);
	tbtAxisYZeroCenter->setAutoRaise(true);
	tbtAxisYZeroCenter->setIconSize(QSize(24,24));	// 36, 36
	tbtAxisYZeroCenter->setIcon(QIcon(":/images/axis_left_centerzero_24.png"));
	tbtAxisYZeroCenter->setCheckable(false);	
	tbtAxisYZeroCenter->setToolTip(tr("Rescale to Show All"));

	tbtAxesSettings = new QToolButton(frame_toolbar);
	tbtAxesSettings->setAutoRaise(true);
	tbtAxesSettings->setIconSize(QSize(24,24));		// 32, 32
	tbtAxesSettings->setIcon(QIcon(":/images/axis_settings_24.png"));
	tbtAxesSettings->setToolTip(tr("Set Axis Settings"));

	tbtPlotSettings = new QToolButton(frame_toolbar);
	tbtPlotSettings->setAutoRaise(true);
	tbtPlotSettings->setIconSize(QSize(24,24));		// 32, 32
	tbtPlotSettings->setIcon(QIcon(":/images/plot_settings_24.png"));
	tbtPlotSettings->setToolTip(tr("Set Plot Settings"));

	tbtCurveSettings = new QToolButton(frame_toolbar);
	tbtCurveSettings->setAutoRaise(true);
	tbtCurveSettings->setIconSize(QSize(24,24));	// 36, 36
	tbtCurveSettings->setIcon(QIcon(":/images/curve_settings_24.png"));
	tbtCurveSettings->setToolTip(tr("Set Curve Settings"));

	tbtSaveSettings = new QToolButton(frame_toolbar);
	tbtSaveSettings->setAutoRaise(true);
	tbtSaveSettings->setIconSize(QSize(24,24));		// 32, 32
	tbtSaveSettings->setIcon(QIcon(":/images/save.png"));
	tbtSaveSettings->setToolTip(tr("Save All Settings"));

	//QSpacerItem *verticalSpacer = new QSpacerItem(20, 401, QSizePolicy::Minimum, QSizePolicy::Expanding);
	QSpacerItem *horizontalSpacer = new QSpacerItem(400, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);	
	horizontalLayout->addWidget(tbtAutoResize);
	horizontalLayout->addWidget(tbtFindSignal);
	horizontalLayout->addWidget(tbtAxisYZeroCenter);
	horizontalLayout->addWidget(tbtAxisYZeroDown);
	horizontalLayout->addWidget(tbtAxesSettings);
	horizontalLayout->addWidget(tbtPlotSettings);
	horizontalLayout->addWidget(tbtCurveSettings);
	horizontalLayout->addWidget(tbtSaveSettings);	
	horizontalLayout->addItem(horizontalSpacer);
	
	grid_layout->addWidget(frame_toolbar, 0, 0, 1, 1);	
	grid_layout->addWidget(frame, 1, 0, 1, 1);
	grid_layout->setHorizontalSpacing(5);	
	// **************************************************

	QList<QColor> colors;
	colors	<< QColor(Qt::red) 
			<< QColor(Qt::blue) 
			<< QColor(Qt::green) 
			<< QColor(Qt::magenta) 
			<< QColor(Qt::yellow) 
			<< QColor(Qt::darkRed) 
			<< QColor(Qt::darkBlue) 
			<< QColor(Qt::darkGreen)
			<< QColor(Qt::darkMagenta) 
			<< QColor(Qt::darkYellow);

	for (int i = 0; i < DEFAULT_CURVES; i++)
	{
		QwtCurveSettings *default_curve_settings = new QwtCurveSettings;
		QColor color = QColor(Qt::red);
		if (i < colors.count()) color = colors[i];
		QBrush symbol_brush(color);
		symbol_brush.setStyle(Qt::SolidPattern);
		default_curve_settings->symbol->setStyle(QwtSymbol::Ellipse);
		default_curve_settings->symbol->setBrush(symbol_brush);
		default_curve_settings->symbol->setSize(QSize(6,6));
		QPen symbol_pen(Qt::black);
		symbol_pen.setWidth(1);
		default_curve_settings->symbol->setPen(symbol_pen);	
		default_curve_settings->pen.setColor(color);
		default_curve_settings->pen.setStyle(Qt::SolidLine);
		default_curve_settings->pen.setWidth(1);
		default_curve_settings->dataType = QwtCurveSettings::LinesAndSymbols;

		default_curve_settings_list.append(default_curve_settings);
	}	

	//show_bad_data = false;

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, true);
	
	loadPlotSettings();
	setConnections();
}

DataPlot::~DataPlot()
{	
	//delete default_curve_settings;
	qDeleteAll(default_curve_settings_list);
}

QwtCurveSettings *DataPlot::getDefaultCurveSettings(int index)
{
	QwtCurveSettings *settings = NULL;
	if (index < default_curve_settings_list.count()) settings = default_curve_settings_list[index];

	return settings;
}

void DataPlot::setConnections()
{
	//connect(tbtCurveSignal, SIGNAL(clicked()), this, SLOT(showCurveDialog()));
	connect(tbtAxesSettings, SIGNAL(clicked()), this, SLOT(showAxisDialog()));
	connect(tbtPlotSettings, SIGNAL(clicked()), this, SLOT(showPlotDialog()));
	connect(tbtCurveSettings, SIGNAL(clicked()), this, SLOT(showCurveDialog()));
	connect(tbtFindSignal, SIGNAL(clicked()), this, SLOT(rescaleToShowAll()));
	//connect(tbtAutoResizeV, SIGNAL(clicked(bool)), this, SLOT(setAutoRescale(bool)));
	//connect(tbtAutoResizeH, SIGNAL(clicked(bool)), this, SLOT(setAutoRescale(bool)));
	connect(tbtAxisYZeroCenter, SIGNAL(clicked()), this, SLOT(rescaleToZeroCenter()));
	connect(tbtAxisYZeroDown, SIGNAL(clicked()), this, SLOT(rescaleToZeroDown()));
	connect(tbtAutoResize, SIGNAL(clicked(bool)), this, SLOT(setAutoRescale(bool)));	
	connect(tbtSaveSettings, SIGNAL(clicked()), this, SLOT(savePlotSettings()));

	connect(a_auto_resizeH, SIGNAL(triggered(bool)), this, SLOT(setAutoRescale(bool)));
	connect(a_auto_resizeV, SIGNAL(triggered(bool)), this, SLOT(setAutoRescale(bool)));
}

void DataPlot::plotDataSet(PlottedDataSet* ds)
{
	QString plot_name = this->objectName();
	if (ds->getTargetPlotName() == plot_name)
	{
		//plotData(ds, _show_bad_data);
		plotData(ds);
	}
}

void DataPlot::plotData(PlottedDataSet *ds)
{
	int sz = ds->getYData()->size();
	//if (sz != ds->getBadMap()->size()) return;

	QVector<double> *y_data = new QVector<double>(sz);
	QVector<double> *x_data = new QVector<double>(sz);
	
	//double *x_data = new double[sz];

	//show_bad_data = _show_bad_data;
	int data_count = 0;
	for (int i = 0; i < sz; i++)
	{
		if (ds->getBadMap()->at(i) == DATA_OK && !isNAN(ds->getYData()->at(i)) && ds->getYData()->at(i) != NAN)
		{
			x_data->data()[data_count] = ds->getXData()->at(i);
			y_data->data()[data_count] = ds->getYData()->at(i);
			data_count++;
		}
	}
	x_data->resize(data_count);
	y_data->resize(data_count);

	/*if (!show_bad_data)	// отображение только корректных данных
	{		
		for (int i = 0; i < sz; i++)
		{			
			if (ds->getBadMap()->at(i) == DATA_OK)
			{
				x_data[data_count] = ds->getXData()->at(i);
				y_data[data_count] = ds->getYData()->at(i);
				data_count++;
			}		
		}
	}	
	else	*/			// отображение всех данных (некорректных тоже)
//	{		
//		data_count = sz;
//		memcpy(x_data, ds->getXData()->data(), data_count*sizeof(double));
//		memcpy(y_data, ds->getYData()->data(), data_count*sizeof(double));
//	}

	QwtPlotCurve *curve = new QwtPlotCurve(ds->getName());
	curve->setRenderHint(QwtPlotItem::RenderAntialiased);	

	QwtCurveSettings *cs = ds->getCurveSettings();
	bool isheld = ds->isHeldOn();
	PlotCurve plot_curve;
	plot_curve.curve = curve;
	plot_curve.ds = ds;
	
	QPen curve_pen = curve->pen();
	curve_pen.setStyle(cs->pen.style());
	curve_pen.setWidth(cs->pen.width());
	curve_pen.setColor(cs->pen.color());
	curve->setPen(curve_pen);

	curve->setZ(cs->z);
	
	QwtCurveSettings::DataType data_type = cs->dataType; 
	if (data_type == CurveSettings::Lines || data_type == CurveSettings::LinesAndSymbols) curve->setStyle(QwtPlotCurve::Lines);
	else curve->setStyle(QwtPlotCurve::NoCurve);

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
	curve->setSymbol(sym);

	//curve_list.append(curve);
	curve_list.append(plot_curve);

	curve->setSamples(x_data->data(), y_data->data(), data_count);
	curve->attach(qwtPlot);

	qwtPlot->replot();	

	if (qwtPlot->axisAutoScale(QwtPlot::yLeft)) zoomer->setZoomBase(true);
	if (qwtPlot->axisAutoScale(QwtPlot::xBottom)) zoomer->setZoomBase(true); 
	
	delete x_data;
	delete y_data;
}

void DataPlot::removeData(QString &ds_name)
{
	for (int i = 0; i < curve_list.count(); i++)
	{		
		if (curve_list[i].curve->title() == ds_name)
		{			
			PlotCurve plot_curve = curve_list.takeAt(i);
			QwtPlotCurve *curve = plot_curve.curve;
			
			curve->detach();	
			qwtPlot->replot();

			delete curve;	
		}
	}
}

void DataPlot::showAxisDialog()
{
	AxisDialog axis_dlg(qwtPlot, this);
	if (axis_dlg.exec())
	{
		for (int i = 0; i < 4; i++)
		{
			qwtPlot->enableAxis(QwtPlot::Axis(i), axis_dlg.axisVisible(QwtPlot::Axis(i)));
			qwtPlot->setAxisTitle(QwtPlot::Axis(i), axis_dlg.getAxisTitle(QwtPlot::Axis(i)));

			QPalette p_axis = qwtPlot->axisWidget(QwtPlot::Axis(i))->palette();
			p_axis.setColor(QPalette::Foreground, axis_dlg.getAxisColor(QwtPlot::Axis(i)));
			p_axis.setColor(QPalette::Text, axis_dlg.getAxisLabelColor(QwtPlot::Axis(i)));
			qwtPlot->axisWidget(QwtPlot::Axis(i))->setPalette(p_axis);

			qwtPlot->setAxisFont(QwtPlot::Axis(i), axis_dlg.getAxisLabelFont(QwtPlot::Axis(i)));
			qwtPlot->axisWidget(QwtPlot::Axis(i))->scaleDraw()->setPenWidth(axis_dlg.getAxisThickness(QwtPlot::Axis(i)));

			bool auto_scale = axis_dlg.axisAutoRescale(QwtPlot::Axis(i));
			qwtPlot->setAxisAutoScale(QwtPlot::Axis(i), auto_scale);
			if (!auto_scale)
			{
				QPair<double,double> from_to = axis_dlg.getAxisScaleRange(QwtPlot::Axis(i));
				double step = axis_dlg.getAxisScaleStep(QwtPlot::Axis(i));
				qwtPlot->setAxisScale(QwtPlot::Axis(i), from_to.first, from_to.second, step);				
			}

			//if ((QwtPlot::Axis)i == QwtPlot::yLeft || (QwtPlot::Axis)i == QwtPlot::yRight) 
			if ((QwtPlot::Axis)i == QwtPlot::yLeft) 
			{
				//tbtAutoResizeV->setChecked(auto_scale);
				tbtAutoResize->setChecked(auto_scale);
				a_auto_resizeV->setChecked(auto_scale);
			}
			else if ((QwtPlot::Axis)i == QwtPlot::xBottom || (QwtPlot::Axis)i == QwtPlot::xTop) 
			{
				//tbtAutoResizeH->setChecked(auto_scale);
				tbtAutoResize->setChecked(auto_scale);
				a_auto_resizeH->setChecked(auto_scale);
			}

			QwtScaleEngine *scale_engine = axis_dlg.getAxisScaleType(QwtPlot::Axis(i));
			qwtPlot->setAxisScaleEngine(QwtPlot::Axis(i), scale_engine);

			qwtPlot->setAxisMaxMinor(QwtPlot::Axis(i), axis_dlg.getAxisTicks(QwtPlot::Axis(i)));
		}

		qwtPlot->replot();
	}	
}

void DataPlot::showPlotDialog()
{
	PlotDialog plot_dlg(qwtPlot, grid, this);
	if (plot_dlg.exec())
	{
		qwtPlot->setCanvasBackground(QBrush(plot_dlg.getBackgroundColor()));
		qwtPlot->setTitle(plot_dlg.getTitle());

		grid->enableX(plot_dlg.enabledGridMajorX());
		grid->enableY(plot_dlg.enabledGridMajorY());
		grid->enableXMin(plot_dlg.enabledGridMinorX());
		grid->enableYMin(plot_dlg.enabledGridMinorY());
		grid->setMajorPen(plot_dlg.getGridMajorLine());
		grid->setMinorPen(plot_dlg.getGridMinorLine());

		qwtPlot->replot();
	}
}


void DataPlot::showCurveDialog()
{		
	CurveDialog dlg(curve_list);
	if (dlg.exec())
	{		
		QList<QwtCurveSettings*> *curve_settings = dlg.getCurveSettingsList();
		for (int i = 0; i < curve_settings->size(); i++)
		{
			QwtCurveSettings *settings = curve_settings->at(i);

			for (int j = 0; j < curve_list.count(); j++)
			{
				QwtPlotCurve *curve = curve_list[j].curve;
				PlottedDataSet *ds = curve_list[j].ds;
				QString curve_title = curve->title().text();

				if (curve_title == settings->title)
				{					
					QwtCurveSettings::DataType cdt = settings->dataType;
					QPen cp = settings->pen;

					QwtSymbol::Style ss = settings->symbol->style();
					QPen sp = settings->symbol->pen();
					QBrush sb = settings->symbol->brush();
					QSize sz = settings->symbol->size();

					curve->setPen(cp);
					QwtSymbol *_symbol = new QwtSymbol(ss, sb, sp, sz);
					curve->setSymbol(_symbol);

					PaintFrame *p_frame = ds->getPaintFrame();
					if (p_frame)
					{
						p_frame->setLineSettings(cp.width(), cp.color());
						p_frame->setLineStyle(cp.style());
						p_frame->setSymbolSettings(ss, sz.width(), sp.width(), sb.color(), sp.color());
						p_frame->setDataType(cdt);

						p_frame->repaint();
					}					
				}
			}				
		}
		qwtPlot->replot();
	}	
}

void DataPlot::rescaleToZeroCenter()
{
	tbtAutoResize->setChecked(false);

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, true);	

	qwtPlot->replot();
	
	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, false);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, false);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, false);

	double y_min = fabs(qwtPlot->axisScaleDiv(QwtPlot::yLeft).interval().minValue());
	double y_max = fabs(qwtPlot->axisScaleDiv(QwtPlot::yLeft).interval().maxValue());
	double limit = y_max;
	if (limit < y_min) limit = y_min;
	
	qwtPlot->setAxisScale(QwtPlot::yLeft, -limit, limit);
	qwtPlot->replot();

	zoomer->setZoomBase(true);
	zoomer->setEnabled(true);
}

void DataPlot::rescaleToZeroDown()
{
	tbtAutoResize->setChecked(false);

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, true);	

	qwtPlot->replot();

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, false);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, false);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, false);
		
	double y_max = fabs(qwtPlot->axisScaleDiv(QwtPlot::yLeft).interval().maxValue());
	if (y_max <= 0) return;

	qwtPlot->setAxisScale(QwtPlot::yLeft, 0, y_max);
	qwtPlot->replot();

	zoomer->setZoomBase(true);
	zoomer->setEnabled(true);
}

void DataPlot::rescaleToShowAll()
{
	tbtAutoResize->setChecked(false);

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, true);	

	qwtPlot->replot();
	zoomer->setZoomBase(true);

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft, false);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom, false);
	qwtPlot->setAxisAutoScale(QwtPlot::xTop, false);

	zoomer->setEnabled(true);
}

void DataPlot::setAutoRescale(QwtPlot::Axis axis, bool flag)
{	
	if (axis == QwtPlot::yLeft) 
	{
		qwtPlot->setAxisAutoScale(axis, flag);		
		a_auto_resizeV->setChecked(flag);
		zoomer->setEnabled(!flag);
	}
	else if (axis == QwtPlot::xTop || axis == QwtPlot::xBottom) 
	{
		qwtPlot->setAxisAutoScale(QwtPlot::xBottom, flag);
		qwtPlot->setAxisAutoScale(QwtPlot::xTop, flag);		
		a_auto_resizeH->setChecked(flag);
		zoomer->setEnabled(!flag);
	}	

	if (a_auto_resizeH->isChecked() || a_auto_resizeV->isChecked()) 
	{
		tbtAutoResize->setChecked(true);
		zoomer->setEnabled(false);
	}
	else 
	{
		tbtAutoResize->setChecked(false);
		zoomer->setEnabled(true);
	}
}

void DataPlot::setAutoRescale(bool flag)
{	
	if (QToolButton *tbt = (QToolButton*)sender())
	{
		if (tbt == tbtAutoResize)
		{
			a_auto_resizeH->setChecked(flag);
			a_auto_resizeV->setChecked(flag);

			qwtPlot->setAxisAutoScale(QwtPlot::yLeft, flag);
			qwtPlot->setAxisAutoScale(QwtPlot::xBottom, flag);
			qwtPlot->setAxisAutoScale(QwtPlot::xTop, flag);	

			if (tbtAutoResize->isChecked()) zoomer->setEnabled(false);
			else zoomer->setEnabled(true);

			return;
		}		
	}
	
	if (QAction *a = (QAction*)sender())
	{
		if (flag) tbtAutoResize->setChecked(true);

		if (a == a_auto_resizeH)
		{			
			qwtPlot->setAxisAutoScale(QwtPlot::yLeft, flag);
		}
		else if (a == a_auto_resizeV)
		{			
			qwtPlot->setAxisAutoScale(QwtPlot::xBottom, flag);
			qwtPlot->setAxisAutoScale(QwtPlot::xTop, flag);
		}

		if (a_auto_resizeV->isChecked() || a_auto_resizeH->isChecked()) 
		{
			tbtAutoResize->setChecked(true);
			zoomer->setEnabled(false);
		}
		else 
		{
			tbtAutoResize->setChecked(false);
			zoomer->setEnabled(true);
		}
	}
}


void DataPlot::loadPlotSettings()
{
	bool ok;

	// Load background settings
	QVariant v_bk_color = app_settings->value(this->objectName()+"/BackgroundColor");
	if (v_bk_color != QVariant()) qwtPlot->setCanvasBackground(QBrush(QVarToQColor(v_bk_color)));

	// Load Plot Title settings
	QwtText title;
	QString title_text = app_settings->value(this->objectName()+"/Title").toString();
	if (title_text != "") title.setText(title_text);
	QVariant v_title_color = app_settings->value(this->objectName()+"/TitleColor");
	if (v_title_color != QVariant()) title.setColor(QVarToQColor(v_title_color));
	QVariant v_title_font = app_settings->value(this->objectName()+"/TitleFont");
	if (v_title_font != QVariant()) title.setFont(QVarToQFont(v_title_font));	
	qwtPlot->setTitle(title);

	// Load Grid settings
	QVariant v_majgrid_henabled = app_settings->value("_" + this->objectName()+"_MajorGrid"+"/HorizontalEnabled");
	if (v_majgrid_henabled != QVariant()) grid->enableX(v_majgrid_henabled.toBool());
	QVariant v_majgrid_venabled = app_settings->value("_" + this->objectName()+"_MajorGrid"+"/VerticalEnabled");
	if (v_majgrid_venabled != QVariant()) grid->enableY(v_majgrid_venabled.toBool());
	QVariant v_mingrid_henabled = app_settings->value("_" + this->objectName()+"_MinorGrid"+"/HorizontalEnabled");
	if (v_mingrid_henabled != QVariant()) grid->enableXMin(v_mingrid_henabled.toBool());
	QVariant v_mingrid_venabled = app_settings->value("_" + this->objectName()+"_MinorGrid"+"/VerticalEnabled");
	if (v_mingrid_venabled != QVariant()) grid->enableYMin(v_mingrid_venabled.toBool());
	QPen majpen = grid->majorPen();
	QVariant v_majgrid_type = app_settings->value("_" + this->objectName()+"_MajorGrid"+"/LineType");
	if (v_majgrid_type != QVariant()) majpen.setStyle(QVarToQtPenStyle(v_majgrid_type));
	QVariant v_majpen_width = app_settings->value("_" + this->objectName()+"_MajorGrid"+"/LineWidth");
	if (v_majpen_width != QVariant()) majpen.setWidth(v_majpen_width.toInt());
	QVariant v_majpen_color = app_settings->value("_" + this->objectName()+"_MajorGrid"+"/LineColor");
	if (v_majpen_color != QVariant()) majpen.setColor(QVarToQColor(v_majpen_color));
	grid->setMajorPen(majpen);
	QPen minpen = grid->minorPen();
	QVariant v_mingrid_type = app_settings->value("_" + this->objectName()+"_MinorGrid"+"/LineType");
	if (v_mingrid_type != QVariant()) minpen.setStyle(QVarToQtPenStyle(v_mingrid_type));
	QVariant v_minpen_width = app_settings->value("_" + this->objectName()+"_MinorGrid"+"/LineWidth");
	if (v_minpen_width != QVariant()) minpen.setWidth(v_minpen_width.toInt());
	QVariant v_minpen_color = app_settings->value("_" + this->objectName()+"_MinorGrid"+"/LineColor");
	if (v_minpen_color != QVariant()) minpen.setColor(QVarToQColor(v_minpen_color));
	grid->setMinorPen(minpen);

	// Load Axis Settings
	QVariant v_yleft_enabled = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/Enabled");
	if (v_yleft_enabled != QVariant()) qwtPlot->enableAxis(QwtPlot::yLeft, v_yleft_enabled.toBool());
	QwtText yleft_title;
	QString yleft_title_text = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/Title").toString();
	if (yleft_title_text != "") yleft_title.setText(yleft_title_text);
	QVariant v_yleft_color = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/TitleColor");
	if (v_yleft_color != QVariant()) yleft_title.setColor(QVarToQColor(v_yleft_color));
	QVariant v_yleft_font = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/TitleFont");
	if (v_yleft_font != QVariant()) yleft_title.setFont(QVarToQFont(v_yleft_font));	
	qwtPlot->setAxisTitle(QwtPlot::yLeft, yleft_title);
	QPair<double, double> yleft_range(0,1000);
	double yleft_from = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/From").toDouble(&ok);
	if (ok) yleft_range.first = yleft_from;
	double yleft_to = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/To").toDouble(&ok);
	if (ok) yleft_range.second = yleft_to;
	double yleft_step = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/Step").toDouble(&ok);
	if (ok) qwtPlot->setAxisScale(QwtPlot::yLeft, yleft_range.first, yleft_range.second, yleft_step);	
	else qwtPlot->setAxisScale(QwtPlot::yLeft, yleft_range.first, yleft_range.second);		
	QVariant v_yleft_autorescale = app_settings->value("_" + this->objectName()+"_AxisLeftY"+"/AutoRescale");
	if (v_yleft_autorescale != QVariant()) qwtPlot->setAxisAutoScale(QwtPlot::yLeft, v_yleft_autorescale.toBool());

	QVariant v_xbottom_enabled = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/Enabled");
	if (v_xbottom_enabled != QVariant()) qwtPlot->enableAxis(QwtPlot::xBottom, v_xbottom_enabled.toBool());
	QwtText xbottom_title;
	QString xbottom_title_text = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/Title").toString();
	if (xbottom_title_text != "") xbottom_title.setText(xbottom_title_text);
	QVariant v_xbottom_color = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/TitleColor");
	if (v_xbottom_color != QVariant()) xbottom_title.setColor(QVarToQColor(v_xbottom_color));
	QVariant v_xbottom_font = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/TitleFont");
	if (v_xbottom_font != QVariant()) xbottom_title.setFont(QVarToQFont(v_xbottom_font));	
	qwtPlot->setAxisTitle(QwtPlot::xBottom, xbottom_title);
	QPair<double, double> xbottom_range(0,1000);
	double xbottom_from = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/From").toDouble(&ok);
	if (ok) xbottom_range.first = xbottom_from;
	double xbottom_to = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/To").toDouble(&ok);
	if (ok) xbottom_range.second = xbottom_to;
	double xbottom_step = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/Step").toDouble(&ok);
	if (ok) 
	{
		qwtPlot->setAxisScale(QwtPlot::xBottom, xbottom_range.first, xbottom_range.second, xbottom_step);	
		qwtPlot->setAxisScale(QwtPlot::xTop, xbottom_range.first, xbottom_range.second, xbottom_step);	
	}
	else 
	{
		qwtPlot->setAxisScale(QwtPlot::xBottom, xbottom_range.first, xbottom_range.second);	
		qwtPlot->setAxisScale(QwtPlot::xTop, xbottom_range.first, xbottom_range.second);
	}
	QVariant v_xbottom_autorescale = app_settings->value("_" + this->objectName()+"_AxisBottomX"+"/AutoRescale");
	if (v_xbottom_autorescale != QVariant()) qwtPlot->setAxisAutoScale(QwtPlot::xBottom, v_xbottom_autorescale.toBool());
		
	QVariant v_yright_enabled = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/Enabled");
	if (v_yright_enabled != QVariant()) qwtPlot->enableAxis(QwtPlot::yRight, v_yright_enabled.toBool());
	QwtText yright_title;
	QString yright_title_text = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/Title").toString();
	if (yright_title_text != "") yright_title.setText(yright_title_text);
	QVariant v_yright_color = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/TitleColor");
	if (v_yright_color != QVariant()) yright_title.setColor(QVarToQColor(v_yright_color));
	QVariant v_yright_font = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/TitleFont");
	if (v_yright_font != QVariant()) yright_title.setFont(QVarToQFont(v_yright_font));	
	qwtPlot->setAxisTitle(QwtPlot::yRight, yright_title);
	QPair<double, double> yright_range(0,1000);
	double yright_from = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/From").toDouble(&ok);
	if (ok) yright_range.first = yright_from;
	double yright_to = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/To").toDouble(&ok);
	if (ok) yright_range.second = yright_to;
	double yright_step = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/Step").toDouble(&ok);
	if (ok) qwtPlot->setAxisScale(QwtPlot::yRight, yright_range.first, yright_range.second, yright_step);	
	else qwtPlot->setAxisScale(QwtPlot::yRight, yright_range.first, yright_range.second);		
	QVariant v_yright_autorescale = app_settings->value("_" + this->objectName()+"_AxisRightY"+"/AutoRescale");
	if (v_yright_autorescale != QVariant()) qwtPlot->setAxisAutoScale(QwtPlot::yRight, v_yright_autorescale.toBool());

	QVariant v_xtop_enabled = app_settings->value("_" + this->objectName()+"_AxisTopX"+"/Enabled");
	if (v_xtop_enabled != QVariant()) qwtPlot->enableAxis(QwtPlot::xTop, v_xtop_enabled.toBool());
	QwtText xtop_title;
	QString xtop_title_text = app_settings->value("_" + this->objectName()+"_AxisTopX"+"/Title").toString();
	if (xtop_title_text != "") xtop_title.setText(xtop_title_text);
	QVariant v_xtop_color = app_settings->value("_" + this->objectName()+"_AxisTopX"+"/TitleColor");
	if (v_xtop_color != QVariant()) xtop_title.setColor(QVarToQColor(v_xtop_color));
	QVariant v_xtop_font = app_settings->value("_" + this->objectName()+"_AxisTopX"+"/TitleFont");
	if (v_xtop_font != QVariant()) xtop_title.setFont(QVarToQFont(v_xtop_font));	
	qwtPlot->setAxisTitle(QwtPlot::xTop, xtop_title);

	qwtPlot->replot();
}

void DataPlot::savePlotSettings()
{
	app_settings->setValue(this->objectName()+"/BackgroundColor", QColorToQVar(qwtPlot->canvasBackground().color()));

	app_settings->setValue(this->objectName()+"/Title", qwtPlot->title().text());
	app_settings->setValue(this->objectName()+"/TitleColor", QColorToQVar(qwtPlot->title().color()));
	app_settings->setValue(this->objectName()+"/TitleFont", QFontToQVar(qwtPlot->title().font()));
	
	app_settings->setValue("_" + this->objectName()+"_MajorGrid"+"/HorizontalEnabled", grid->xEnabled());
	app_settings->setValue("_" + this->objectName()+"_MajorGrid"+"/VerticalEnabled", grid->yEnabled());
	app_settings->setValue("_" + this->objectName()+"_MajorGrid"+"/LineType", QtPenStyleToQVar(grid->majorPen()));
	app_settings->setValue("_" + this->objectName()+"_MajorGrid"+"/LineWidth", grid->majorPen().width());
	app_settings->setValue("_" + this->objectName()+"_MajorGrid"+"/LineColor", QColorToQVar(grid->majorPen().color()));
	app_settings->setValue("_" + this->objectName()+"_MinorGrid"+"/HorizontalEnabled", grid->xMinEnabled());
	app_settings->setValue("_" + this->objectName()+"_MinorGrid"+"/VerticalEnabled", grid->yMinEnabled());	
	app_settings->setValue("_" + this->objectName()+"_MinorGrid"+"/LineType", QtPenStyleToQVar(grid->minorPen()));
	app_settings->setValue("_" + this->objectName()+"_MinorGrid"+"/LineWidth", grid->minorPen().width());
	app_settings->setValue("_" + this->objectName()+"_MinorGrid"+"/LineColor", QColorToQVar(grid->minorPen().color()));

	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Enabled", qwtPlot->axisEnabled(QwtPlot::yLeft));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Title", qwtPlot->axisTitle(QwtPlot::yLeft).text());	
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/TitleColor", QColorToQVar(qwtPlot->axisTitle(QwtPlot::yLeft).color()));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/TitleFont", QFontToQVar(qwtPlot->axisTitle(QwtPlot::yLeft).font()));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Width", qwtPlot->axisWidget(QwtPlot::yLeft)->scaleDraw()->penWidth());
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Color", QColorToQVar(qwtPlot->axisWidget(QwtPlot::yLeft)->palette().color(QPalette::Foreground)));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/LabelColor", QColorToQVar(qwtPlot->axisWidget(QwtPlot::yLeft)->palette().color(QPalette::Text)));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/LabelFont", QFontToQVar(qwtPlot->axisFont(QwtPlot::yLeft)));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/AutoRescale", qwtPlot->axisAutoScale(QwtPlot::yLeft));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/From", qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound());
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/To", qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound());
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Step", qwtPlot->axisStepSize(QwtPlot::yLeft));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Ticks", qwtPlot->axisMaxMinor(QwtPlot::yLeft));
	app_settings->setValue("_" + this->objectName()+"_AxisLeftY"+"/Scale", ScaleToQVar(qwtPlot->axisScaleEngine(QwtPlot::yLeft)));

	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Enabled", qwtPlot->axisEnabled(QwtPlot::xBottom));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Title", qwtPlot->axisTitle(QwtPlot::xBottom).text());	
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/TitleColor", QColorToQVar(qwtPlot->axisTitle(QwtPlot::xBottom).color()));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/TitleFont", QFontToQVar(qwtPlot->axisTitle(QwtPlot::xBottom).font()));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Width", qwtPlot->axisWidget(QwtPlot::xBottom)->scaleDraw()->penWidth());
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Color", QColorToQVar(qwtPlot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Foreground)));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/LabelColor", QColorToQVar(qwtPlot->axisWidget(QwtPlot::xBottom)->palette().color(QPalette::Text)));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/LabelFont", QFontToQVar(qwtPlot->axisFont(QwtPlot::xBottom)));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/AutoRescale", qwtPlot->axisAutoScale(QwtPlot::xBottom));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/From", qwtPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound());
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/To", qwtPlot->axisScaleDiv(QwtPlot::xBottom).upperBound());
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Step", qwtPlot->axisStepSize(QwtPlot::xBottom));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Ticks", qwtPlot->axisMaxMinor(QwtPlot::xBottom));
	app_settings->setValue("_" + this->objectName()+"_AxisBottomX"+"/Scale", ScaleToQVar(qwtPlot->axisScaleEngine(QwtPlot::xBottom)));

	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Enabled", qwtPlot->axisEnabled(QwtPlot::yRight));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Title", qwtPlot->axisTitle(QwtPlot::yRight).text());	
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/TitleColor", QColorToQVar(qwtPlot->axisTitle(QwtPlot::yRight).color()));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/TitleFont", QFontToQVar(qwtPlot->axisTitle(QwtPlot::yRight).font()));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Width", qwtPlot->axisWidget(QwtPlot::yRight)->scaleDraw()->penWidth());
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Color", QColorToQVar(qwtPlot->axisWidget(QwtPlot::yRight)->palette().color(QPalette::Foreground)));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/LabelColor", QColorToQVar(qwtPlot->axisWidget(QwtPlot::yRight)->palette().color(QPalette::Text)));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/LabelFont", QFontToQVar(qwtPlot->axisFont(QwtPlot::yRight)));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/AutoRescale", qwtPlot->axisAutoScale(QwtPlot::yRight));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/From", qwtPlot->axisScaleDiv(QwtPlot::yRight).lowerBound());
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/To", qwtPlot->axisScaleDiv(QwtPlot::yRight).upperBound());
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Step", qwtPlot->axisStepSize(QwtPlot::yRight));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Ticks", qwtPlot->axisMaxMinor(QwtPlot::yRight));
	app_settings->setValue("_" + this->objectName()+"_AxisRightY"+"/Scale", ScaleToQVar(qwtPlot->axisScaleEngine(QwtPlot::yRight)));

	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Enabled", qwtPlot->axisEnabled(QwtPlot::xTop));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Title", qwtPlot->axisTitle(QwtPlot::xTop).text());	
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/TitleColor", QColorToQVar(qwtPlot->axisTitle(QwtPlot::xTop).color()));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/TitleFont", QFontToQVar(qwtPlot->axisTitle(QwtPlot::xTop).font()));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Width", qwtPlot->axisWidget(QwtPlot::xTop)->scaleDraw()->penWidth());
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Color", QColorToQVar(qwtPlot->axisWidget(QwtPlot::xTop)->palette().color(QPalette::Foreground)));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/LabelColor", QColorToQVar(qwtPlot->axisWidget(QwtPlot::xTop)->palette().color(QPalette::Text)));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/LabelFont", QFontToQVar(qwtPlot->axisFont(QwtPlot::xTop)));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/AutoRescale", qwtPlot->axisAutoScale(QwtPlot::xTop));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/From", qwtPlot->axisScaleDiv(QwtPlot::xTop).lowerBound());
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/To", qwtPlot->axisScaleDiv(QwtPlot::xTop).upperBound());
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Step", qwtPlot->axisStepSize(QwtPlot::xTop));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Ticks", qwtPlot->axisMaxMinor(QwtPlot::xTop));
	app_settings->setValue("_" + this->objectName()+"_AxisTopX"+"/Scale", ScaleToQVar(qwtPlot->axisScaleEngine(QwtPlot::xTop)));

	app_settings->sync();
}


PlottedDataManager::PlottedDataManager(DataPlot *_data_plot, DataPlot *_math_plot, QSettings *_settings,  QVector<ToolChannel*> _tool_channel, QWidget *parent)
{	
	if (objectName().isEmpty()) setObjectName("IncomDataManager");
	resize(466, 671);

	app_settings = _settings;
	tool_channels = _tool_channel;

	ds_counter = 1;
	is_moving_aver = true;

	setDataPlot(_data_plot);
	setMathPlot(_math_plot);

	QGridLayout *gridLayout_2 = new QGridLayout(this);
	gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
	gridLayout_2->setContentsMargins(5, 5, 5, 5);	
	QFrame *gbxDatasets = new QFrame(this);
	gbxDatasets->setObjectName(QStringLiteral("gbxDatasets"));	
	gbxDatasets->setFrameShape(QFrame::StyledPanel);
	gbxDatasets->setFrameShadow(QFrame::Plain);
	QFont font;
	font.setPointSize(10);
	gbxDatasets->setFont(font);	
	QGridLayout *gridLayout_4 = new QGridLayout(gbxDatasets);
	gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
	gridLayout_4->setContentsMargins(5, 5, 5, 5);

	QFont font1;
	font1.setPointSize(9);

	treeWidget = new QTreeWidget(gbxDatasets);
	treeWidget->setObjectName("treeWidget");
	QFont font2;
	font2.setPointSize(10);
	treeWidget->setFont(font2);
	gridLayout_4->addWidget(treeWidget, 0, 0, 1, 1);

	QHBoxLayout *horizontalLayout_2 = new QHBoxLayout();
	horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
	tbtMark = new QToolButton(gbxDatasets);
	tbtMark->setObjectName(QStringLiteral("tbtMark"));
	tbtMark->setText(tr("Mark..."));
	tbtMark->setFont(font1);
	tbtMark->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tbtMark->setIconSize(QSize(20,20));
	tbtMark->setIcon(QIcon(":/images/check_green.png"));
	tbtMark->setAutoRaise(true);

	horizontalLayout_2->addWidget(tbtMark);

	tbtUnmarkAll = new QToolButton(gbxDatasets);
	tbtUnmarkAll->setObjectName(QStringLiteral("tbtUnmarkAll"));
	tbtUnmarkAll->setText(tr("Unmark All"));
	tbtUnmarkAll->setFont(font1);
	tbtUnmarkAll->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tbtUnmarkAll->setIconSize(QSize(20,20));
	tbtUnmarkAll->setIcon(QIcon(":/images/check_red.png"));
	tbtUnmarkAll->setAutoRaise(true);

	horizontalLayout_2->addWidget(tbtUnmarkAll);

	tbtExport = new QToolButton(gbxDatasets);
	tbtExport->setObjectName(QStringLiteral("tbtExport"));
	tbtExport->setText(tr("Export..."));
	tbtExport->setFont(font1);
	tbtExport->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tbtExport->setIconSize(QSize(20,20));
	tbtExport->setIcon(QIcon(":/images/Export.png"));
	tbtExport->setAutoRaise(true);

	horizontalLayout_2->addWidget(tbtExport);

	tbtRemove = new QToolButton(gbxDatasets);
	tbtRemove->setObjectName(QStringLiteral("tbtRemove"));
	tbtRemove->setText(tr("Remove"));
	tbtRemove->setFont(font1);
	tbtRemove->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tbtRemove->setIconSize(QSize(20,20));
	tbtRemove->setIcon(QIcon(":/images/disconnect.png"));
	tbtRemove->setAutoRaise(true);

	horizontalLayout_2->addWidget(tbtRemove);

	QSpacerItem *horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout_2->addItem(horizontalSpacer_2);

	gridLayout_4->addLayout(horizontalLayout_2, 1, 0, 1, 1);


	// *********** Moving Average groupbox ***********
	gbxAveraging = new QGroupBox(gbxDatasets);
	gbxAveraging->setObjectName(QStringLiteral("gbxAveraging"));	
	gbxAveraging->setFont(font2);
	gbxAveraging->setCheckable(false);	
	gbxAveraging->setTitle(tr("Moving Average"));
	QGridLayout *gridLayoutAver = new QGridLayout(gbxAveraging);
	gridLayoutAver->setObjectName(QStringLiteral("gridLayoutAver"));
	gridLayoutAver->setContentsMargins(5, 5, 5, 5);
	QLabel *lblAver = new QLabel(gbxAveraging);
	lblAver->setObjectName(QStringLiteral("lblAver"));
	lblAver->setText(tr("<font color=darkblue>Window Length (Datasets):</font>"));

	gridLayoutAver->addWidget(lblAver, 0, 0, 1, 1);

	sbxDataSets = new QSpinBox(gbxAveraging);
	sbxDataSets->setObjectName(QStringLiteral("sbxDataSets"));
	sbxDataSets->setMinimumSize(QSize(100, 0));
	sbxDataSets->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	sbxDataSets->setMinimum(1);
	sbxDataSets->setMaximum(64);
	sbxDataSets->setValue(ds_counter);

	gridLayoutAver->addWidget(sbxDataSets, 0, 1, 1, 1);

	QSpacerItem *horizontalSpacerAver = new QSpacerItem(121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	gridLayoutAver->addItem(horizontalSpacerAver, 0, 2, 1, 1);

	gridLayout_4->addWidget(gbxAveraging, 2, 0, 1, 1);
	// ***********************************************************

	lblChannels = new QLabel(gbxDatasets);
	lblChannels->setObjectName(QStringLiteral("labelChannels"));
	lblChannels->setText("<font color=darkblue>" + tr("Tool Channels:") + "</font>");
	lblChannels->setFont(font1);
	lblChannels->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	cboxChannels = new QComboBox(gbxDatasets);
	cboxChannels->setObjectName(QStringLiteral("cboxToolChannels"));
	
	QSizePolicy sizePolicy0(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	sizePolicy0.setHorizontalStretch(0);
	sizePolicy0.setVerticalStretch(0);
	sizePolicy0.setHeightForWidth(cboxChannels->sizePolicy().hasHeightForWidth());
	cboxChannels->setSizePolicy(sizePolicy0);

	QHBoxLayout *horizontalLayout_14 = new QHBoxLayout();
	horizontalLayout_14->setSpacing(5);
	horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
	horizontalLayout_14->addWidget(lblChannels);
	horizontalLayout_14->addWidget(cboxChannels);
	gridLayout_4->addLayout(horizontalLayout_14, 3, 0, 1, 1);

	gbxTimeWin = new QGroupBox(gbxDatasets);
	gbxTimeWin->setObjectName(QStringLiteral("gbxTimeWin"));
	gbxTimeWin->setTitle(tr("Apodizing function"));
	gbxTimeWin->setMinimumSize(QSize(0, 0));
	gbxTimeWin->setCheckable(true);
	gbxTimeWin->setChecked(true);
	QGridLayout *gridLayout_6 = new QGridLayout(gbxTimeWin);
	gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
	gridLayout_6->setContentsMargins(5, 5, 5, 5);
	QGridLayout *gridLayout = new QGridLayout();
	gridLayout->setObjectName(QStringLiteral("gridLayout"));
	QLabel *label = new QLabel(gbxTimeWin);
	label->setObjectName(QStringLiteral("label"));
	label->setText("<font color=darkblue>" + tr("Type:") + "</font>");
	label->setFont(font1);
	label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	gridLayout->addWidget(label, 0, 0, 1, 1);

	cboxTypeTime = new QComboBox(gbxTimeWin);
	cboxTypeTime->setObjectName(QStringLiteral("cboxTypeTime"));
	QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(cboxTypeTime->sizePolicy().hasHeightForWidth());
	cboxTypeTime->setSizePolicy(sizePolicy1);

	gridLayout->addWidget(cboxTypeTime, 0, 1, 1, 1);

	lblX0Time = new QLabel(gbxTimeWin);
	lblX0Time->setObjectName(QStringLiteral("lblX0Time"));
	lblX0Time->setText("<font color=darkblue>" + QString(QChar(0x394)) + "x:</font>");
	lblX0Time->setFont(font2);
	lblX0Time->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	gridLayout->addWidget(lblX0Time, 1, 0, 1, 1);

	QHBoxLayout *horizontalLayout_3 = new QHBoxLayout();
	horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
	horizontalLayout_3->setSpacing(3);
	dsbxX0Time = new QDoubleSpinBox(gbxTimeWin);
	dsbxX0Time->setObjectName(QStringLiteral("dsbxX0Time"));
	sizePolicy1.setHeightForWidth(dsbxX0Time->sizePolicy().hasHeightForWidth());
	dsbxX0Time->setSizePolicy(sizePolicy1);
	dsbxX0Time->setFont(font1);
	dsbxX0Time->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	dsbxX0Time->setMaximum(9999.99);
	dsbxX0Time->setMinimum(-9999.99);
	dsbxX0Time->setSingleStep(10);

	horizontalLayout_3->addWidget(dsbxX0Time);

	QLabel *label_3 = new QLabel(gbxTimeWin);
	label_3->setObjectName(QStringLiteral("label_3"));
	label_3->setText(tr("mks"));
	label_3->setFont(font1);

	horizontalLayout_3->addWidget(label_3);

	gridLayout->addLayout(horizontalLayout_3, 1, 1, 1, 1);

	lblSigmaTime = new QLabel(gbxTimeWin);
	lblSigmaTime->setObjectName(QStringLiteral("lblSigmaTime"));
	lblSigmaTime->setText("<font color=darkblue>" + QString(QChar(0x3C3)) + ":" + "</font>");
	lblSigmaTime->setFont(font2);
	lblSigmaTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	gridLayout->addWidget(lblSigmaTime, 2, 0, 1, 1);

	QHBoxLayout *horizontalLayout_4 = new QHBoxLayout();
	horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
	horizontalLayout_4->setSpacing(3);
	dsbxSigmaTime = new QDoubleSpinBox(gbxTimeWin);
	dsbxSigmaTime->setObjectName(QStringLiteral("dsbxSigmaTime"));
	sizePolicy1.setHeightForWidth(dsbxSigmaTime->sizePolicy().hasHeightForWidth());
	dsbxSigmaTime->setSizePolicy(sizePolicy1);
	dsbxSigmaTime->setFont(font1);
	dsbxSigmaTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	dsbxSigmaTime->setMaximum(9999.99);
	dsbxSigmaTime->setMinimum(0.1);
	dsbxSigmaTime->setSingleStep(10);

	horizontalLayout_4->addWidget(dsbxSigmaTime);

	QLabel *label_5 = new QLabel(gbxTimeWin);
	label_5->setObjectName(QStringLiteral("label_5"));
	label_5->setText(tr("mks"));
	label_5->setFont(font1);

	horizontalLayout_4->addWidget(label_5);

	gridLayout->addLayout(horizontalLayout_4, 2, 1, 1, 1);

	gridLayout_6->addLayout(gridLayout, 0, 0, 1, 1);

	QGridLayout *gridLayout_5 = new QGridLayout();
	gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
	QHBoxLayout *horizontalLayout_5 = new QHBoxLayout();
	horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
	QSpacerItem *horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout_5->addItem(horizontalSpacer_3);
	
	gridLayout_5->addLayout(horizontalLayout_5, 2, 0, 1, 1);

	lblTimeWinFormula = new QLabel(gbxTimeWin);
	lblTimeWinFormula->setObjectName(QStringLiteral("lblTimeWinFormula"));
	QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	sizePolicy2.setHorizontalStretch(0);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(lblTimeWinFormula->sizePolicy().hasHeightForWidth());
	lblTimeWinFormula->setSizePolicy(sizePolicy2);
	lblTimeWinFormula->setFont(font2);
	lblTimeWinFormula->setFrameShape(QFrame::Panel);
	lblTimeWinFormula->setFrameShadow(QFrame::Sunken);
	lblTimeWinFormula->setAlignment(Qt::AlignCenter);

	gridLayout_5->addWidget(lblTimeWinFormula, 0, 0, 1, 1);

	QLabel *label_7 = new QLabel(gbxTimeWin);
	label_7->setObjectName(QStringLiteral("label_7"));
	
	gridLayout_5->addWidget(label_7, 1, 0, 1, 1);

	gridLayout_6->addLayout(gridLayout_5, 0, 1, 1, 1);

	gridLayout_4->addWidget(gbxTimeWin, 4, 0, 1, 1);

	gbxFreqWin = new QGroupBox(gbxDatasets);
	gbxFreqWin->setObjectName(QStringLiteral("gbxFreqWin"));
	gbxFreqWin->setTitle(tr("Window function"));
	gbxFreqWin->setMinimumSize(QSize(0, 0));
	gbxFreqWin->setCheckable(true);
	gbxFreqWin->setChecked(true);
	QGridLayout *gridLayout_8 = new QGridLayout(gbxFreqWin);
	gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
	gridLayout_8->setContentsMargins(5, 5, 5, 5);
	QGridLayout *gridLayout_3 = new QGridLayout();
	gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
	QLabel *label_2 = new QLabel(gbxFreqWin);
	label_2->setObjectName(QStringLiteral("label_2"));
	label_2->setText("<font color=darkblue>" + tr("Type:") + "</font>");
	label_2->setFont(font1);
	label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	gridLayout_3->addWidget(label_2, 0, 0, 1, 1);

	cboxTypeFreq = new QComboBox(gbxFreqWin);
	cboxTypeFreq->setObjectName(QStringLiteral("cboxTypeFreq"));
	sizePolicy1.setHeightForWidth(cboxTypeFreq->sizePolicy().hasHeightForWidth());
	cboxTypeFreq->setSizePolicy(sizePolicy1);

	gridLayout_3->addWidget(cboxTypeFreq, 0, 1, 1, 1);

	lblX0Freq = new QLabel(gbxFreqWin);
	lblX0Freq->setObjectName(QStringLiteral("lblX0Freq"));
	lblX0Freq->setText("<font color=darkblue>" + QString(QChar(0x394)) + "x:</font>");
	lblX0Freq->setFont(font2);
	lblX0Freq->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	gridLayout_3->addWidget(lblX0Freq, 1, 0, 1, 1);

	QHBoxLayout *horizontalLayout_6 = new QHBoxLayout();
	horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
	horizontalLayout_6->setSpacing(3);
	dsbxX0Freq = new QDoubleSpinBox(gbxFreqWin);
	dsbxX0Freq->setObjectName(QStringLiteral("dsbxX0Freq"));
	sizePolicy1.setHeightForWidth(dsbxX0Freq->sizePolicy().hasHeightForWidth());
	dsbxX0Freq->setSizePolicy(sizePolicy1);
	dsbxX0Freq->setFont(font1);
	dsbxX0Freq->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	dsbxX0Freq->setMaximum(9999.99);
	dsbxX0Freq->setMinimum(0.1);

	horizontalLayout_6->addWidget(dsbxX0Freq);

	QLabel *label_4 = new QLabel(gbxFreqWin);
	label_4->setObjectName(QStringLiteral("label_4"));
	label_4->setText(tr("kHz"));
	label_4->setFont(font1);

	horizontalLayout_6->addWidget(label_4);

	gridLayout_3->addLayout(horizontalLayout_6, 1, 1, 1, 1);

	lblSigmaFreq = new QLabel(gbxFreqWin);
	lblSigmaFreq->setObjectName(QStringLiteral("lblSigmaFreq"));
	lblSigmaFreq->setText("<font color=darkblue>" + QString(QChar(0x3C3)) + ":" + "</font>");
	lblSigmaFreq->setFont(font2);
	lblSigmaFreq->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

	gridLayout_3->addWidget(lblSigmaFreq, 2, 0, 1, 1);

	QHBoxLayout *horizontalLayout_7 = new QHBoxLayout();
	horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
	horizontalLayout_7->setSpacing(3);
	dsbxSigmaFreq = new QDoubleSpinBox(gbxFreqWin);
	dsbxSigmaFreq->setObjectName(QStringLiteral("dsbxSigmaFreq"));
	sizePolicy1.setHeightForWidth(dsbxSigmaFreq->sizePolicy().hasHeightForWidth());
	dsbxSigmaFreq->setSizePolicy(sizePolicy1);
	dsbxSigmaFreq->setFont(font1);
	dsbxSigmaFreq->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	dsbxSigmaFreq->setMaximum(9999.99);
	dsbxSigmaFreq->setMinimum(0.1);
	dsbxSigmaFreq->setSingleStep(1);

	horizontalLayout_7->addWidget(dsbxSigmaFreq);

	QLabel *label_6 = new QLabel(gbxFreqWin);
	label_6->setObjectName(QStringLiteral("label_6"));
	label_6->setText(tr("kHz"));
	label_6->setFont(font1);

	horizontalLayout_7->addWidget(label_6);

	gridLayout_3->addLayout(horizontalLayout_7, 2, 1, 1, 1);

	gridLayout_8->addLayout(gridLayout_3, 0, 0, 1, 1);

	QGridLayout *gridLayout_7 = new QGridLayout();
	gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
	lblFreqWinFormula = new QLabel(gbxFreqWin);
	lblFreqWinFormula->setObjectName(QStringLiteral("lblFreqWinFormula"));
	sizePolicy2.setHeightForWidth(lblFreqWinFormula->sizePolicy().hasHeightForWidth());
	lblFreqWinFormula->setSizePolicy(sizePolicy2);
	lblFreqWinFormula->setFont(font2);
	lblFreqWinFormula->setFrameShape(QFrame::Panel);
	lblFreqWinFormula->setFrameShadow(QFrame::Sunken);
	lblFreqWinFormula->setAlignment(Qt::AlignCenter);

	gridLayout_7->addWidget(lblFreqWinFormula, 0, 0, 1, 1);

	QLabel *label_8 = new QLabel(gbxFreqWin);
	label_8->setObjectName(QStringLiteral("label_8"));

	gridLayout_7->addWidget(label_8, 1, 0, 1, 1);

	QHBoxLayout *horizontalLayout_12 = new QHBoxLayout();
	horizontalLayout_12->setSpacing(5);
	horizontalLayout_12->setObjectName(QStringLiteral("horizontalLayout_12"));
	QSpacerItem *horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout_12->addItem(horizontalSpacer_4);

	gridLayout_7->addLayout(horizontalLayout_12, 2, 0, 1, 1);

	gridLayout_8->addLayout(gridLayout_7, 0, 1, 1, 1);
	
	gridLayout_4->addWidget(gbxFreqWin, 5, 0, 1, 1);

	pbtApplyWin = new QPushButton(gbxTimeWin);
	pbtApplyWin->setObjectName(QStringLiteral("pbtApplyWin"));
	pbtApplyWin->setText(tr("Apply Parameters"));
	pbtApplyWin->setFont(font1);
	pbtApplyWin->setIcon(QIcon(":images/apply.png"));
	pbtApplyWin->setFlat(false);

	pbtSaveSettings = new QPushButton(gbxTimeWin);
	pbtSaveSettings->setObjectName(QStringLiteral("pbtSaveSettings"));
	pbtSaveSettings->setText(tr("Save Settings"));
	pbtSaveSettings->setFont(font1);
	pbtSaveSettings->setIcon(QIcon(":images/save.png"));
	pbtSaveSettings->setFlat(false);

	QHBoxLayout *horizontalLayout_13 = new QHBoxLayout();
	horizontalLayout_13->setSpacing(5);
	horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_12"));
	horizontalLayout_13->addWidget(pbtApplyWin);
	horizontalLayout_13->addWidget(pbtSaveSettings);
	gridLayout_4->addLayout(horizontalLayout_13, 6, 0, 1, 1);

	gridLayout_2->addWidget(gbxDatasets, 0, 0, 1, 1);
	

	QStringList headlist;
	treeWidget->setColumnCount(6);
	headlist << tr("Number") << tr("Curve") << tr("Data") << tr("Points") << tr("Plot") << tr("Hold");
	treeWidget->setHeaderLabels(headlist);
	treeWidget->setColumnWidth(0,60);
	treeWidget->setColumnWidth(1,60);	
	treeWidget->setColumnWidth(2,140);
	treeWidget->setColumnWidth(3,50);
	treeWidget->setColumnWidth(4,70);
	treeWidget->setColumnWidth(5,30);

	gbxTimeWin->setChecked(false);
	gbxFreqWin->setChecked(false);
		
	createWinFuncs();
	fillWinFuncs();	

	setConnections();

	last_shown_datasets = 1;
	//busy = false;
}

PlottedDataManager::~PlottedDataManager()
{
	if (!dataset_list.isEmpty()) qDeleteAll(dataset_list);
	
	clearCTreeWidget();
}

double PlottedDataManager::NMR_SAMPLE_FREQ()
{
	double sample_freq = (4*250000);
	ToolChannel *cur_tool_channel = getCurrentToolChannel();
	if (cur_tool_channel) sample_freq = cur_tool_channel->sample_freq*1000;		// originally sample freq in [kHz] 

	return sample_freq;
}

void PlottedDataManager::setConnections()
{
	connect(tbtRemove, SIGNAL(clicked()), this, SLOT(removeDataSet()));
	connect(tbtMark, SIGNAL(clicked()), this, SLOT(markDataSets()));
	connect(tbtUnmarkAll, SIGNAL(clicked()), this, SLOT(unmarkAll()));
	connect(cboxTypeTime, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeTimeWin(QString)));
	connect(cboxTypeFreq, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeFreqWin(QString)));
	connect(gbxTimeWin, SIGNAL(clicked(bool)), this, SLOT(setTimeWinChecked(bool)));
	connect(gbxFreqWin, SIGNAL(clicked(bool)), this, SLOT(setFreqWinChecked(bool)));
	connect(dsbxX0Time, SIGNAL(valueChanged(double)), this, SLOT(changeTimeX0(double)));
	connect(dsbxSigmaTime, SIGNAL(valueChanged(double)), this, SLOT(changeTimeSigma(double)));
	connect(dsbxX0Freq, SIGNAL(valueChanged(double)), this, SLOT(changeFreqX0(double)));
	connect(dsbxSigmaFreq, SIGNAL(valueChanged(double)), this, SLOT(changeFreqSigma(double)));
	connect(pbtApplyWin, SIGNAL(clicked()), this, SLOT(applyWinParams()));
	connect(pbtSaveSettings, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(sbxDataSets, SIGNAL(valueChanged(int)), this, SLOT(changeDataSetCount(int)));
	connect(gbxAveraging, SIGNAL(clicked(bool)), this, SLOT(setMovingAverChecked(bool)));
	connect(cboxChannels, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeToolChannel(QString)));
	//connect(pbtApplyMovingAver, SIGNAL(clicked()), this, SLOT(applyDataSetCounter()));
}

void PlottedDataManager::addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, bool _hold_on)
{
	PlottedDataSet *dataset = new PlottedDataSet(_name, _plot, _x, _y, _bad_map, _settings);
	dataset_list.append(dataset);
	dataset->setID(dataset_list.count());
	//dataset->setDefaultCurveSettings(def_index);
	dataset->holdDataSetOn(_hold_on);

	addRecord(dataset);

	emit plot_dataset(dataset);
}

void PlottedDataManager::addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, bool _hold_on, int def_index)
{
	PlottedDataSet *dataset = new PlottedDataSet(_name, _plot, _x, _y, _bad_map);
	dataset_list.append(dataset);
	dataset->setID(dataset_list.count());
	dataset->setDefaultCurveSettings(def_index);
	dataset->holdDataSetOn(_hold_on);

	addRecord(dataset);

	emit plot_dataset(dataset);
}

void PlottedDataManager::hideWinWidgets()
{
	lblChannels->setVisible(false);
	cboxChannels->setVisible(false);
	gbxTimeWin->setVisible(false);
	gbxFreqWin->setVisible(false);	
	pbtApplyWin->setVisible(false);
	pbtSaveSettings->setVisible(false);
}

void PlottedDataManager::hideMovingAverWidget()
{
	gbxAveraging->setVisible(false);
}


void PlottedDataManager::clearCTreeWidget()
{
	if (!c_items.isEmpty()) qDeleteAll(c_items);
	c_items.clear();
}

void PlottedDataManager::removeDataSet()
{
	int ds_count = dataset_list.count();
	for (int i = ds_count-1; i >= 0; --i)
	{
		PlottedDataSet *ds = dataset_list[i];			
		if (ds->isMarked() && ds->getParentDataSet() != NULL) 
		{			
			QString _name = ds->getName();
			removeDataSetByName(_name);
		}
	}

	ds_count = dataset_list.count();
	for (int i = ds_count-1; i >= 0; --i)
	{
		PlottedDataSet *ds = dataset_list[i];			
		if (ds->isMarked()) 
		{			
			QString _name = ds->getName();
			removeDataSetByName(_name);
		}
	} 
}

void PlottedDataManager::removeDataSetAt(int _id)
{	
	int index = -1;
	PlottedDataSet *ds = NULL;
	for (int i = 0; i < dataset_list.count(); i++)
	{
		if (dataset_list[i]->getID() == _id) 
		{
			index = i;
			ds = dataset_list[i];
		}
	}
		
	QList<PlottedDataSet*> remove_list;
	if (index >= 0) 
	{
		dataset_list.removeAt(index);
		remove_list.append(ds);
	}
	else return;
	

	for (int i = dataset_list.count()-1; i >= 0; --i)
	{
		PlottedDataSet *_ds = dataset_list[i]->getParentDataSet();
		if (_ds == ds) 
		{
			remove_list.append(dataset_list[i]);
			dataset_list.removeAt(i);
		}
	}

	if (index >= 0) 
	{
		if (remove_list.count() > 1)
		{
			for (int i = remove_list.count()-1; i > 0; --i)
			{
				PlottedDataSet *_ds = remove_list[i];
				QString _ds_name = _ds->getName();
				CTreeWidgetItem *_ctwi = _ds->getCTreeWidgetItem();
				
				bool ret = 0;
				int j = 0;
				while (ret == 0 || j < c_items.count())
				{
					if (c_items[j] == _ctwi)
					{
						c_items.removeAt(j);
						ret = 1;
					}
					j++;
				}
													
				delete _ctwi->getQSubTreeWidgetItem();
				delete _ctwi;
				_ctwi = NULL;

				remove_list.removeAt(i);
				delete _ds;

				emit dataset_removed(_ds_name);
			}				
		}

		if (remove_list.count() == 1) 
		{
			PlottedDataSet *_ds = remove_list.first();
			QString _ds_name = _ds->getName();
			CTreeWidgetItem *_ctwi = _ds->getCTreeWidgetItem();

			bool ret = 0;
			int j = 0;
			while (ret == 0 || j < c_items.count())
			{
				if (c_items[j] == _ctwi)
				{
					c_items.removeAt(j);
					ret = 1;
				}
				j++;
			}

			delete _ctwi->getQSubTreeWidgetItem();
			delete _ctwi;
			_ctwi = NULL;

			delete _ds;
			_ds = NULL;		

			emit dataset_removed(_ds_name);
		}		
	}

	for (int i = 0; i < dataset_list.count(); i++)
	{
		PlottedDataSet *ds = dataset_list[i];
		ds->setID(i+1);
		CTreeWidgetItem *ctwi = ds->getCTreeWidgetItem();
		QWidget *w = ctwi->getCWidgets().first();
		QCheckBox *chbx = qobject_cast<QCheckBox*>(w);
		if (chbx) chbx->setText(QString("%1").arg(i+1));
	}

	treeWidget->repaint();
}

void PlottedDataManager::removeDataSetByName(QString _name)
{	
	int index = -1;
	PlottedDataSet *ds = NULL;
	for (int i = 0; i < dataset_list.count(); i++)
	{
		if (dataset_list[i]->getName() == _name) 
		{
			index = i;
			ds = dataset_list[i];
		}
	}
		
	QList<PlottedDataSet*> remove_list;
	if (index >= 0) 
	{
		dataset_list.removeAt(index);
		remove_list.append(ds);
	}
	else return;
	

	for (int i = dataset_list.count()-1; i >= 0; --i)
	{
		PlottedDataSet *_ds = dataset_list[i]->getParentDataSet();
		if (_ds == ds) 
		{
			remove_list.append(dataset_list[i]);
			dataset_list.removeAt(i);
		}
	}

	if (index >= 0) 
	{
		if (remove_list.count() > 1)
		{
			for (int i = remove_list.count()-1; i > 0; --i)
			{
				PlottedDataSet *_ds = remove_list[i];
				QString _ds_name = _ds->getName();
				CTreeWidgetItem *_ctwi = _ds->getCTreeWidgetItem();
				
				bool ret = 0;
				int j = 0;
				while (ret == 0 || j < c_items.count())
				{
					if (c_items[j] == _ctwi)
					{
						c_items.removeAt(j);
						ret = 1;
					}
					j++;
				}
									
				delete _ctwi->getQSubTreeWidgetItem();
				delete _ctwi;
				_ctwi = NULL;

				remove_list.removeAt(i);
				delete _ds;

				emit dataset_removed(_ds_name);
			}				
		}

		if (remove_list.count() == 1) 
		{
			PlottedDataSet *_ds = remove_list.first();
			QString _ds_name = _ds->getName();
			CTreeWidgetItem *_ctwi = _ds->getCTreeWidgetItem();

			bool ret = 0;
			int j = 0;
			while (ret == 0 || j < c_items.count())
			{
				if (c_items[j] == _ctwi)
				{
					c_items.removeAt(j);
					ret = 1;
				}
				j++;
			}

			delete _ctwi->getQSubTreeWidgetItem();
			delete _ctwi;
			_ctwi = NULL;

			delete _ds;
			_ds = NULL;		

			emit dataset_removed(_ds_name);
		}		
	}

	for (int i = 0; i < dataset_list.count(); i++)
	{
		PlottedDataSet *ds = dataset_list[i];
		ds->setID(i+1);
		CTreeWidgetItem *ctwi = ds->getCTreeWidgetItem();
		QWidget *w = ctwi->getCWidgets().first();
		QCheckBox *chbx = qobject_cast<QCheckBox*>(w);
		if (chbx) chbx->setText(QString("%1").arg(i+1));
	}

	treeWidget->repaint();
}

void PlottedDataManager::refreshDataSets()
{
	int cnt = 0;
	for (int i = dataset_list.count()-1; i >= 0; --i)
	{
		PlottedDataSet *ds = dataset_list[i];
		if (ds->getParentDataSet() == NULL)
		{
			if (!ds->isHeldOn())
			{
				//if (++cnt > last_shown_datasets)	// temporary removed
				{
					QString ds_name = ds->getName();
					removeDataSetByName(ds_name);
				}								
			}
		}
	}
}

void PlottedDataManager::markDataSets()
{
	MarkDialog dlg(dataset_list.count());
	if (dlg.exec())
	{
		QList<int> marked_list = dlg.markedDataSets();
		for (int i = 0; i < marked_list.count(); i++)
		{
			for (int j = 0; j < dataset_list.count(); j++)
			{
				PlottedDataSet *ds = dataset_list[j];
				if (ds->getID() == marked_list[i])
				{
					CTreeWidgetItem *ctwi = ds->getCTreeWidgetItem();

					QWidget *w = ctwi->getCWidgets().first();
					QCheckBox *chbx = qobject_cast<QCheckBox*>(w);
					if (chbx) chbx->setCheckState(Qt::Checked);
				}				
			}
		}
	}
}

void PlottedDataManager::unmarkAll()
{
	for (int j = 0; j < dataset_list.count(); j++)
	{
		PlottedDataSet *ds = dataset_list[j];
		CTreeWidgetItem *ctwi = ds->getCTreeWidgetItem();

		QWidget *w = ctwi->getCWidgets().first();
		QCheckBox *chbx = qobject_cast<QCheckBox*>(w);
		if (chbx) chbx->setCheckState(Qt::Unchecked);
	}
}

void PlottedDataManager::addRecord(PlottedDataSet *_ds)
{
	int _id = _ds->getID();
	
	QList<CSettings> item_settings_list;

	CSettings item_settings1("checkbox", _id, QString("mark"));	
	item_settings1.text_color = QColor(Qt::darkBlue);		
	item_settings1.background_color = QColor(Qt::blue).lighter(190);
	item_settings1.checkable = true;
	item_settings1.check_state = false;
	item_settings1.font = QFont("Arial", 8);

	CSettings item_settings2("widget", 0);	
	item_settings2.widget = _ds->getPaintFrame();

	CSettings item_settings3("label", _ds->getName());	
	item_settings3.text_color = QColor(Qt::darkBlue);		
	item_settings3.background_color = QColor(Qt::blue).lighter(190);
	item_settings3.font = QFont("Arial", 8);

	CSettings item_settings4("label", QString::number(_ds->getPointsCount()));	
	item_settings4.text_color = QColor(Qt::darkBlue);		
	item_settings4.background_color = QColor(Qt::blue).lighter(190);
	item_settings4.font = QFont("Arial", 8);

	CSettings item_settings5("label", _ds->getTargetPlotName());	
	item_settings5.text_color = QColor(Qt::darkBlue);		
	item_settings5.background_color = QColor(Qt::blue).lighter(190);
	item_settings5.font = QFont("Arial", 8);

	CSettings item_settings6("checkbox", "", QString("hold"));
	item_settings6.text_color = QColor(Qt::darkBlue);		
	item_settings6.background_color = QColor(Qt::blue).lighter(190);
	item_settings6.checkable = true;	
	item_settings6.check_state = _ds->isHeldOn();

	item_settings_list << item_settings1 << item_settings2 << item_settings3 << item_settings4 << item_settings5 << item_settings6;
	CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, 0, item_settings_list);	
	connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), _ds, SLOT(changeDataSetState(QObject*, QVariant&)));

	_ds->linkCTreeWidgetItem(c_item);	
	c_items.append(c_item);
	c_item->show();
	_ds->getPaintFrame()->show();
}

void PlottedDataManager::addSubRecord(PlottedDataSet *_ds, QString _parent_name)
{
	int _id = _ds->getID();
	CTreeWidgetItem *ctwi = NULL;
	PlottedDataSet *ds = NULL;
	for (int i = 0; i < dataset_list.count(); i++)
	{
		PlottedDataSet *__ds = dataset_list[i];
		if (__ds->getName() == _parent_name) 
		{
			ctwi = __ds->getCTreeWidgetItem();			
		}
	}
	if (ctwi == NULL) return;


	QList<CSettings> item_settings_list;

	CSettings item_settings1("checkbox", _id, QString("mark"));	
	item_settings1.text_color = QColor(Qt::darkBlue);		
	item_settings1.background_color = QColor(Qt::blue).lighter(190);
	item_settings1.checkable = true;
	item_settings1.check_state = false;

	CSettings item_settings2("widget", 0);	
	item_settings2.widget = _ds->getPaintFrame();

	CSettings item_settings3("label", _ds->getName());	
	item_settings3.text_color = QColor(Qt::darkBlue);		
	item_settings3.background_color = QColor(Qt::blue).lighter(190);

	CSettings item_settings4("label", QString::number(_ds->getPointsCount()));	
	item_settings4.text_color = QColor(Qt::darkBlue);		
	item_settings4.background_color = QColor(Qt::blue).lighter(190);

	CSettings item_settings5("label", _ds->getTargetPlotName());	
	item_settings5.text_color = QColor(Qt::darkBlue);		
	item_settings5.background_color = QColor(Qt::blue).lighter(190);
	
	CSettings item_settings6("label", "");	
	item_settings6.text_color = QColor(Qt::darkBlue);		
	item_settings6.background_color = QColor(Qt::blue).lighter(190);

	item_settings_list << item_settings1 << item_settings2 << item_settings3 << item_settings4 << item_settings5 << item_settings6;
	CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, ctwi->getQSubTreeWidgetItem(), item_settings_list);	
	connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), _ds, SLOT(changeDataSetState(QObject*, QVariant&)));

	_ds->linkCTreeWidgetItem(c_item);
	c_items.append(c_item);
	c_item->show();

	_ds->linkCTreeWidgetItem(c_item);
	_ds->getPaintFrame()->show();
}

void PlottedDataManager::createWinFuncs()
{	
	win_funcs.clear();

	double mks_to_Pnt = 1.0/NMR_SAMPLE_FREQ()*1000000;
	double kHz_to_Pnt = (double)(NMR_SAMPLE_FREQ()/2)/512/1000;

	double time_dx1 = 0;
	double time_sigma1 = 65.0;
	double time_dx2 = 0;
	double time_sigma2 = 25.0;
	double freq_dx1 = 241;
	double freq_sigma1 = 4.0;
	double freq_dx2 = 250;
	double freq_sigma2 = 25.0;

	bool ok;
	double val = 0;
	if (app_settings->contains("WinFuncSettings/TimeFunc1_Dx")) 
	{
		val = app_settings->value("WinFuncSettings/TimeFunc1_Dx").toDouble(&ok);
		if (ok) time_dx1 = val;
	}
	if (app_settings->contains("WinFuncSettings/TimeFunc1_Sigma")) 
	{
		val = app_settings->value("WinFuncSettings/TimeFunc1_Sigma").toDouble(&ok);
		if (ok) time_sigma1 = val;
	}
	if (app_settings->contains("WinFuncSettings/TimeFunc2_Dx")) 
	{
		val = app_settings->value("WinFuncSettings/TimeFunc2_Dx").toDouble(&ok);
		if (ok) time_dx2 = val;
	}
	if (app_settings->contains("WinFuncSettings/TimeFunc2_Sigma")) 
	{
		val = app_settings->value("WinFuncSettings/TimeFunc2_Sigma").toDouble(&ok);
		if (ok) time_sigma2 = val;
	}
	if (app_settings->contains("WinFuncSettings/FreqFunc1_Dx")) 
	{
		val = app_settings->value("WinFuncSettings/FreqFunc1_Dx").toDouble(&ok);
		if (ok) freq_dx1 = val;
	}
	if (app_settings->contains("WinFuncSettings/FreqFunc1_Sigma")) 
	{
		val = app_settings->value("WinFuncSettings/FreqFunc1_Sigma").toDouble(&ok);
		if (ok) freq_sigma1 = val;
	}
	if (app_settings->contains("WinFuncSettings/FreqFunc2_Dx")) 
	{
		val = app_settings->value("WinFuncSettings/FreqFunc2_Dx").toDouble(&ok);
		if (ok) freq_dx2 = val;
	}
	if (app_settings->contains("WinFuncSettings/FreqFunc2_Sigma")) 
	{
		val = app_settings->value("WinFuncSettings/FreqFunc2_Sigma").toDouble(&ok);
		if (ok) freq_sigma2 = val;
	}	
		
	WinFunc func1;	
	func1.name = tr("Gaussian");
	func1.applied_name = 1;
	func1.description = "<font color = darkblue>A(x) = exp[-(x-x<sub>0</sub>-" + QString(QChar(0x394)) + "x)<sup>2</sup>/2" + QString(QChar(0x3C3))+ "<sup>2</sup>], x<sub>0</sub> - center</font>";
	func1.dx = time_dx1;
	func1.applied_dx = 2.0*time_dx1/mks_to_Pnt;
	func1.sigma = time_sigma1;									// ширина в мкс
	func1.applied_sigma = 2.0*time_sigma1/mks_to_Pnt;			// ширина в точках АЦП

	WinFunc func2;	
	func2.name = tr("Bigaussian");
	func2.applied_name = 2;
	func2.description = "<font color = darkblue>A(x) = exp[-(x-x<sub>0</sub>-" + QString(QChar(0x394)) + "x)<sup>4</sup>/4" + QString(QChar(0x3C3))+ "<sup>4</sup>], x<sub>0</sub> - center</font>";
	func2.dx = time_dx2;
	func2.applied_dx = 2.0*time_dx2/mks_to_Pnt;
	func2.sigma = time_sigma2;									// ширина в мкс
	func2.applied_sigma = 2.0*time_sigma2/mks_to_Pnt;			// ширина в точках АЦП

	WinFunc func3;	
	func3.name = tr("Gaussian");
	func3.applied_name = 1;
	func3.description = "<font color = darkblue>A(x) = exp[-(x-x<sub>0</sub>-" + QString(QChar(0x394)) + "x)<sup>2</sup>/2" + QString(QChar(0x3C3))+ "<sup>2</sup>], x<sub>0</sub> - center</font>";
	func3.dx = freq_dx1;										// kHz
	func3.applied_dx = freq_dx1/kHz_to_Pnt;						// в точках
	func3.sigma = freq_sigma1;									// kHz
	func3.applied_sigma = freq_sigma1/kHz_to_Pnt;			// ширина в точках

	WinFunc func4;	
	func4.name = tr("Bigaussian");
	func4.applied_name = 2;
	func4.description = "<font color = darkblue>A(x) = exp[-(x-x<sub>0</sub>-" + QString(QChar(0x394)) + "x)<sup>4</sup>/4" + QString(QChar(0x3C3))+ "<sup>4</sup>], x<sub>0</sub> - center</font>";
	func4.dx = freq_dx2;										// kHz
	func4.applied_dx = freq_dx2/kHz_to_Pnt;						// в точках
	func4.sigma = freq_sigma2;									// кГц
	func4.applied_sigma = freq_sigma2/kHz_to_Pnt;				// ширина в точках
		
	win_funcs << func1 << func2 << func3 << func4;				// win_funcs[0] и win_funcs[1] - оконные функции во временном представлении, win_funcs[2] и win_funcs[3] - в частотном 
	time_win = win_funcs[0];
	freq_win = win_funcs[2];
}

void PlottedDataManager::fillWinFuncs()
{
	if (win_funcs.isEmpty()) return;

	QStringList funcs; funcs << win_funcs[0].name << win_funcs[1].name;
	cboxTypeTime->addItems(funcs);
	cboxTypeFreq->addItems(funcs);

	time_win = win_funcs[0];
	dsbxX0Time->setValue(time_win.dx);
	dsbxSigmaTime->setValue(time_win.sigma);
	lblTimeWinFormula->setText(time_win.description);

	freq_win = win_funcs[2];
	dsbxX0Freq->setValue(freq_win.dx);
	dsbxSigmaFreq->setValue(freq_win.sigma);
	lblFreqWinFormula->setText(freq_win.description);
}

void PlottedDataManager::changeTimeWin(QString str)
{
	if (win_funcs.isEmpty()) return;

	for (int i = 0; i < win_funcs.count()/2; i++)
	{
		if (str == win_funcs[i].name)
		{			
			time_win = win_funcs[i];
			dsbxX0Time->setValue(time_win.dx);
			dsbxSigmaTime->setValue(time_win.sigma);
			lblTimeWinFormula->setText(time_win.description);

			setTimeWinChecked(true);
		}
	}
}

void PlottedDataManager::changeFreqWin(QString str)
{
	if (win_funcs.isEmpty()) return;

	for (int i = win_funcs.count()/2; i < win_funcs.count(); i++)
	{
		if (str == win_funcs[i].name)
		{			
			freq_win = win_funcs[i];
			dsbxX0Freq->setValue(freq_win.dx);
			dsbxSigmaFreq->setValue(freq_win.sigma);
			lblFreqWinFormula->setText(freq_win.description);

			setFreqWinChecked(true);
		}
	}
}

void PlottedDataManager::changeToolChannel(QString str)
{			
	createWinFuncs();
	fillWinFuncs();
	if (gbxTimeWin->isChecked()) setTimeWinChecked(true);
	if (gbxFreqWin->isChecked()) setFreqWinChecked(true);
}

void PlottedDataManager::setTimeWinChecked(bool flag)
{
	int index = -1;
	for (int i = 0; i < win_funcs.count()/2; i++)
	{
		if (cboxTypeTime->currentText() == win_funcs[i].name) index = i;
	}
	if (index < 0) return;
		
	QwtPlot *plot = data_plot->getPlot();
	QwtPlotItemList item_list = plot->itemList();
	QwtPlotItemIterator it = item_list.begin();
	while ( it != item_list.end() )
	{
		QwtPlotItem *item = *it;
		++it; 

		if (item->title().text() == "AppodizationFunc" && item->rtti() == QwtPlotItem::Rtti_PlotCurve)
		{
			item->detach();
			delete item;
		}
	}
	
	if (flag)
	{				
		PlottedDataSet *p_ds = NULL;		
		if (getAllDataSets()->isEmpty()) return;
		for (int i = 0; i < getAllDataSets()->size(); i++)
		{			
			if (getDataSetAt(i)->getTargetPlotName() == "SignalPlot") 
			{
				p_ds = getDataSetAt(i);				
			}
		}
		if (p_ds == NULL) return;
		
		int points_count = p_ds->getPointsCount();
		
		time_win.name = win_funcs[index].name;
		time_win.applied_name = win_funcs[index].applied_name;
		time_win.description = win_funcs[index].description;
		time_win.dx = win_funcs[index].dx;
		time_win.applied_dx = win_funcs[index].applied_dx;
		time_win.sigma = win_funcs[index].sigma;
		time_win.applied_sigma = win_funcs[index].applied_sigma;
		
		QVector<double> *x_data = p_ds->getXData();
		double x0 = x_data->at(points_count/2);		
						
		QVector<double> x(points_count);
		QVector<double> y(points_count);
		for (int i = 0; i < points_count; i++)
		{			
			x[i] = x_data->at(i);
			if (time_win.name == tr("Gaussian")) y[i] = exp(-(x[i]-x0-time_win.dx)*(x[i]-x0-time_win.dx)/2/time_win.sigma/time_win.sigma);
			else if (time_win.name == tr("Bigaussian")) y[i] = exp(-pow(x[i]-x0-time_win.dx,4)/4/pow(time_win.sigma,4));
			else y[i] = 0;
		}

		QwtPlotCurve *win_curve = new QwtPlotCurve;
		win_curve->setTitle("AppodizationFunc");
		win_curve->setRenderHint(QwtPlotItem::RenderAntialiased);		
		
		QPen pen1;		
		QColor color = QColor(Qt::blue);
		color.setAlpha(100);
		pen1.setColor(color);
		pen1.setStyle(Qt::SolidLine);
		//pen1.setWidth(2);
		//win_curve->setPen(color);
		win_curve->setPen(pen1);
		win_curve->setBrush(color);

		win_curve->setSamples(x, y);		
		win_curve->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
		win_curve->attach(data_plot->getPlot());
	}

	data_plot->getPlot()->replot();
}

void PlottedDataManager::setFreqWinChecked(bool flag)
{	
	int index = -1;
	for (int i = win_funcs.count()/2; i < win_funcs.count(); i++)
	{
		if (cboxTypeFreq->currentText() == win_funcs[i].name) index = i;
	}
	if (index < 0) return;

	QwtPlot *plot = math_plot->getPlot();
	QwtPlotItemList item_list = plot->itemList();
	QwtPlotItemIterator it = item_list.begin();
	while ( it != item_list.end() )
	{
		QwtPlotItem *item = *it;
		++it; 

		if (item->title().text() == "WindowFunc" && item->rtti() == QwtPlotItem::Rtti_PlotCurve)
		{
			item->detach();
			delete item;
		}
	}

	if (flag)
	{
		PlottedDataSet *p_ds = NULL;		
		if (getAllDataSets()->isEmpty()) return;
		for (int i = 0; i < getAllDataSets()->size(); i++)
		{			
			if (getDataSetAt(i)->getTargetPlotName() == "SignalMathPlot") 
			{
				p_ds = getDataSetAt(i);
			}
		}
		if (p_ds == NULL) return;

		int points_count = p_ds->getPointsCount();

		freq_win.name = win_funcs[index].name;
		freq_win.applied_name = win_funcs[index].applied_name;
		freq_win.description = win_funcs[index].description;
		freq_win.dx = win_funcs[index].dx;
		freq_win.applied_dx = win_funcs[index].applied_dx;
		freq_win.sigma = win_funcs[index].sigma;
		freq_win.applied_sigma = win_funcs[index].applied_sigma;

		//int points_count = WIN_POINTS;
		//int ds_count = getAllDataSets()->size();
		//if (ds_count > 0) points_count = getDataSetLast()->getPointsCount();
		points_count = p_ds->getPointsCount();
				
		double x0 = 0;
		QVector<double> x(points_count);
		QVector<double> y(points_count);
		for (int i = 0; i < points_count; i++)
		{			
			//x[i] = (i-points_count/2)*(NMR_SAMPLE_FREQ/2.0)/points_count/1000;
			x[i] = i*(NMR_SAMPLE_FREQ()/2.0)/points_count/1000;
			if (freq_win.name == tr("Gaussian")) y[i] = exp(-(x[i]-x0-freq_win.dx)*(x[i]-x0-freq_win.dx)/2/freq_win.sigma/freq_win.sigma);
			else if (freq_win.name == tr("Bigaussian")) y[i] = exp(-pow(x[i]-x0-freq_win.dx,4)/4/pow(freq_win.sigma,4));
			else y[i] = 0;
		}

		QwtPlotCurve *win_curve = new QwtPlotCurve;
		win_curve->setTitle("WindowFunc");
		win_curve->setRenderHint(QwtPlotItem::RenderAntialiased);		

		QColor color = QColor(Qt::red);
		color.setAlpha(100);
		win_curve->setPen(color);
		win_curve->setBrush(color);

		win_curve->setSamples(x, y);		
		win_curve->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
		win_curve->attach(math_plot->getPlot());		
	}	

	math_plot->getPlot()->replot();
}

void PlottedDataManager::setMovingAverChecked(bool flag)
{
	is_moving_aver = flag;
	sbxDataSets->setValue(ds_counter);

	emit apply_moving_averaging(is_moving_aver, ds_counter);
}

void PlottedDataManager::changeTimeX0(double val)
{	
	double mks_to_Pnt = 1.0/NMR_SAMPLE_FREQ()*1000000;
	
	int index = -1;
	for (int i = 0; i < win_funcs.count()/2; i++)
	{
		if (time_win.name == win_funcs[i].name) index = i;
	}
	if (index < 0) return;
		
	//int applied_dx = 2.0*val*NMR_SAMPLE_FREQ/1000000;
	int applied_dx = 2.0*val/mks_to_Pnt;
	time_win.dx = val;
	time_win.applied_dx = applied_dx;
	win_funcs[index].dx = val;
	win_funcs[index].applied_dx = applied_dx;

	setTimeWinChecked(true);
}

void PlottedDataManager::changeFreqX0(double val)
{	
	double kHz_to_Pnt = (double)(NMR_SAMPLE_FREQ()/2)/512/1000;

	int index = -1;
	for (int i = win_funcs.count()/2; i < win_funcs.count(); i++)
	{
		if (freq_win.name == win_funcs[i].name) index = i;
	}
	if (index < 0) return;

	//int applied_dx = val*(NMR_SAMPLE_FREQ/2.0)/1000/1024;
	int applied_dx = val/kHz_to_Pnt;
	freq_win.dx = val;
	freq_win.applied_dx = applied_dx;
	win_funcs[index].dx = val;
	win_funcs[index].applied_dx = applied_dx;

	setFreqWinChecked(true);
}

void PlottedDataManager::changeTimeSigma(double val)
{
	double mks_to_Pnt = 1.0/NMR_SAMPLE_FREQ()*1000000;

	int index = -1;
	for (int i = 0; i < win_funcs.count()/2; i++)
	{
		if (time_win.name == win_funcs[i].name) index = i;
	}
	if (index < 0) return;

	time_win.sigma = val;
	//time_win.applied_sigma = 2.0*val*NMR_SAMPLE_FREQ/1000000; //*val/NMR_SAMPLE_FREQ*1000000;
	time_win.applied_sigma = 2.0*val/mks_to_Pnt; 
	win_funcs[index].sigma = val;
	win_funcs[index].applied_sigma = time_win.applied_sigma;

	setTimeWinChecked(true);
}

void PlottedDataManager::changeFreqSigma(double val)
{
	double kHz_to_Pnt = (double)(NMR_SAMPLE_FREQ()/2)/512/1000;

	int index = -1;
	for (int i = win_funcs.count()/2; i < win_funcs.count(); i++)
	{
		if (freq_win.name == win_funcs[i].name) index = i;
	}
	if (index < 0) return;

	int applied_sigma = val/kHz_to_Pnt;
	freq_win.sigma = val;
	//freq_win.applied_sigma = val*(NMR_SAMPLE_FREQ/2.0)/1000/1024;
	freq_win.applied_sigma = val/kHz_to_Pnt;
	win_funcs[index].sigma = val;
	//win_funcs[index].applied_sigma = val*(NMR_SAMPLE_FREQ/2.0)/1000/1024;
	win_funcs[index].applied_sigma = val/kHz_to_Pnt;

	setFreqWinChecked(true);
}

void PlottedDataManager::changeDataSetCount(int val)
{
	ds_counter = val;

	emit apply_moving_averaging(is_moving_aver, ds_counter);
}

/*void PlottedDataManager::applyDataSetCounter()
{
	ds_counter = sbxDataSets->value();

	emit apply_moving_averaging(is_moving_aver, ds_counter);
}*/

void PlottedDataManager::applyWinParams()
{	
	QVector<int> params;
	
	int func_type = 0;	
	if (gbxTimeWin->isChecked())
	{
		if (time_win.name == tr("Gaussian")) func_type = 2;
		else if (time_win.name == tr("Bigaussian")) func_type = 3;
	}	
	
	//double _dx_t = time_win.dx*NMR_SAMPLE_FREQ/1000000.0;
	//double _sigma_t = time_win.sigma*NMR_SAMPLE_FREQ/1000000.0;
	double dx_t = time_win.applied_dx;
	double sigma_t = time_win.applied_sigma;
		
	params << func_type << (int)dx_t << (int)sigma_t;

	func_type = 0;
	if (gbxFreqWin->isChecked())
	{
		if (freq_win.name == tr("Gaussian")) func_type = 2;
		else if (freq_win.name == tr("Bigaussian")) func_type = 3;
	}		

	//double dx_f = 2.0*freq_win.dx/NMR_SAMPLE_FREQ*1000*1024;
	//double sigma_f = 2.0*freq_win.sigma/NMR_SAMPLE_FREQ*1000*1024;
	double dx_f = freq_win.applied_dx;
	double sigma_f = freq_win.applied_sigma;
		
	params << func_type << (int)dx_f << (int)sigma_f;
		
	emit apply_win_func(params);
}

void PlottedDataManager::saveSettings()
{
	app_settings->setValue("WinFuncSettings/TimeFunc1_Type", win_funcs[0].name);
	app_settings->setValue("WinFuncSettings/TimeFunc1_Dx", QVariant(win_funcs[0].dx).toDouble());
	app_settings->setValue("WinFuncSettings/TimeFunc1_Sigma", QVariant(win_funcs[0].sigma).toDouble());
	app_settings->setValue("WinFuncSettings/TimeFunc1_Units", tr("mks"));
	app_settings->setValue("WinFuncSettings/TimeFunc2_Type", win_funcs[1].name);
	app_settings->setValue("WinFuncSettings/TimeFunc2_Dx", QVariant(win_funcs[1].dx).toDouble());
	app_settings->setValue("WinFuncSettings/TimeFunc2_Sigma", QVariant(win_funcs[1].sigma).toDouble());
	app_settings->setValue("WinFuncSettings/TimeFunc2_Units", tr("mks"));
	app_settings->setValue("WinFuncSettings/FreqFunc1_Type", win_funcs[2].name);
	app_settings->setValue("WinFuncSettings/FreqFunc1_Dx", QVariant(win_funcs[2].dx).toDouble());
	app_settings->setValue("WinFuncSettings/FreqFunc1_Sigma", QVariant(win_funcs[2].sigma).toDouble());
	app_settings->setValue("WinFuncSettings/FreqFunc1_Units", tr("kHz"));
	app_settings->setValue("WinFuncSettings/FreqFunc2_Type", win_funcs[3].name);
	app_settings->setValue("WinFuncSettings/FreqFunc2_Dx", QVariant(win_funcs[3].dx).toDouble());
	app_settings->setValue("WinFuncSettings/FreqFunc2_Sigma", QVariant(win_funcs[3].sigma).toDouble());
	app_settings->setValue("WinFuncSettings/FreqFunc2_Units", tr("kHz"));

	app_settings->sync();
}

void PlottedDataManager::refreshToolChannels(QVector<ToolChannel*> _tool_channels, int cur_channel)
{
	tool_channels.clear();

	QStringList channels_list;
	for (int i = 0; i < _tool_channels.count(); i++)
	{
		ToolChannel *tool_channel = _tool_channels[i];		
		double sample_freq = tool_channel->sample_freq;
		if (sample_freq > 1) tool_channels.append(tool_channel);		
		channels_list << tool_channel->name;
	}

	cboxChannels->clear();
	cboxChannels->addItems(channels_list);
	if (cur_channel >= 0) cboxChannels->setCurrentIndex(cur_channel);
}

ToolChannel* PlottedDataManager::getCurrentToolChannel()
{
	if (tool_channels.isEmpty()) return NULL;

	QString cur_channel_name = cboxChannels->currentText();
	for (int i = 0; i < tool_channels.count(); i++)
	{
		ToolChannel *tool_channel = tool_channels[i];
		if (tool_channel->name == cur_channel_name) return tool_channel;
	}

	return NULL;
}

MonitoringPlot::MonitoringPlot(QString &objectName, QSettings *settings, QWidget *parent /* = 0 */)
{
	setParent(parent);
	this->setObjectName(objectName);

	app_settings = settings;

	// Main Frame
	main_frame = new QGroupBox(this);
	main_frame->setTitle(tr("Monitoring"));
	QFont font = main_frame->font();
	font.setPointSize(10);
	font.setBold(true);
	main_frame->setFont(font);
	font.setPointSize(9);
	font.setBold(false);
	QGridLayout *gridLayout = new QGridLayout(this);	
	QGridLayout *grout = new QGridLayout(main_frame);
	grout->setMargin(2);
	grout->setVerticalSpacing(5);
	main_frame->setLayout(grout);

	// Frame for ToolBar and Monitoring Plot
	QFrame *plot_frame = new QFrame(main_frame);
	plot_frame->setFrameShape(QFrame::Box);
	plot_frame->setFrameShadow(QFrame::Sunken);	
	QVBoxLayout *vout_plot = new QVBoxLayout(plot_frame);
	vout_plot->setMargin(2);
	vout_plot->setSpacing(2);
	
	// Tool button panel
	tool_frame = new QFrame(plot_frame);
	tool_frame->setFrameShape(QFrame::Panel);
	tool_frame->setFrameShadow(QFrame::Raised);
	tool_frame->setFont(font);
	QHBoxLayout *hout_tools = new QHBoxLayout(tool_frame);
	hout_tools->setMargin(2);
	hout_tools->setSpacing(2);
	tool_frame->setLayout(hout_tools);

	// Tool buttons on the Tool button panel
	// Plot Settings button
	tbtPlotSettings = new QToolButton(tool_frame);
	tbtPlotSettings->setAutoRaise(true);
	tbtPlotSettings->setIconSize(QSize(32,32));
	tbtPlotSettings->setIcon(QIcon(":/images/plot_settings.png"));
	hout_tools->addWidget(tbtPlotSettings);

	// Axis Settings button
	tbtAxisSettings = new QToolButton(tool_frame);
	tbtAxisSettings->setAutoRaise(true);
	tbtAxisSettings->setIconSize(QSize(32,32));
	tbtAxisSettings->setIcon(QIcon(":/images/axis_settings.png"));
	hout_tools->addWidget(tbtAxisSettings);

	// Curve Settings button
	tbtCurveSettings = new QToolButton(tool_frame);
	tbtCurveSettings->setAutoRaise(true);
	tbtCurveSettings->setIconSize(QSize(34,34));
	tbtCurveSettings->setIcon(QIcon(":/images/curve_settings.png"));
	hout_tools->addWidget(tbtCurveSettings);

	// Interval Settings widget (QwtCounter)
	QFrame *frame_interval = new QFrame(tool_frame);
	frame_interval->setFrameShape(QFrame::NoFrame);
	QHBoxLayout *hout_interval = new QHBoxLayout(frame_interval);
	hout_interval->setMargin(0);
	hout_interval->setSpacing(2);
	frame_interval->setLayout(hout_interval);
	frame_interval->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	QLabel *lblInterval = new QLabel("Time Range:", frame_interval);
	hout_interval->addWidget(lblInterval);
	cntInterval = new QwtCounter(frame_interval);
	cntInterval->setNumButtons(2);
	cntInterval->setMaximum(1000);
	cntInterval->setSingleStep(1);
	cntInterval->setStepButton1(1);
	cntInterval->setStepButton2(10);
	cntInterval->setValue(60);
	cntInterval->setMaximumWidth(150);
	hout_interval->addWidget(cntInterval);
	QLabel *lblSec = new QLabel("mins", frame_interval);
	hout_interval->addWidget(lblSec);

	hout_tools->addWidget(frame_interval);

	// Add Spacer to the Tool button panel
	QSpacerItem *tools_spacer = new QSpacerItem(445,20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hout_tools->addItem(tools_spacer);

	// Temperature Monitoring Plot
	qwtPlot = new QwtPlot(plot_frame);
	qwtPlot->setFont(font);
	qwtPlot->setMinimumSize(200, 150);
	grid = new QwtPlotGrid;
	grid->setMajorPen(QPen(QBrush(QColor(Qt::gray)),1,Qt::DashLine));
	grid->setMinorPen(QPen(QBrush(QColor(Qt::gray)),0.5,Qt::DotLine));
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(true);
	grid->attach(qwtPlot);	

	// Grid of the Measured Parameters
	parameters_frame = new QFrame(main_frame);
	parameters_frame->setFrameShape(QFrame::Box);
	parameters_frame->setFrameShadow(QFrame::Sunken);
	parameters_frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	QGridLayout *gout_params = new QGridLayout(parameters_frame);
	gout_params->setMargin(2);
	gout_params->setSpacing(5);	
	parameters_frame->setLayout(gout_params);

	QHBoxLayout *horizontalLayout_3 = new QHBoxLayout();	
	QLabel *labelParams = new QLabel(tr("List of Parameters:"), parameters_frame);
	labelParams->setObjectName(QStringLiteral("labelParams"));
	horizontalLayout_3->addWidget(labelParams);
	QSpacerItem *horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout_3->addItem(horizontalSpacer_2);

	scrollArea = new QScrollArea(parameters_frame);
	scrollArea->setObjectName(QStringLiteral("scrollArea"));
	QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Expanding);
	sizePolicy2.setHorizontalStretch(0);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
	scrollArea->setSizePolicy(sizePolicy2);
	scrollArea->setFrameShape(QFrame::Box);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollAreaWidgetContents = new QWidget();
	scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
	scrollAreaWidgetContents->setGeometry(QRect(0, 0, 344, 322));
	QGridLayout *grout_scroll = new QGridLayout(scrollAreaWidgetContents);
	spacer_last = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	grout_scroll->addItem(spacer_last, 0, 0);
	scrollArea->setWidget(scrollAreaWidgetContents);	
	
	gout_params->addLayout(horizontalLayout_3, 0, 0);
	gout_params->addWidget(scrollArea, 1, 0);
	
	vout_plot->addWidget(tool_frame);
	vout_plot->addWidget(qwtPlot);

	grout->addWidget(plot_frame, 0, 0);
	grout->addWidget(parameters_frame, 0, 1);

	gridLayout->addWidget(main_frame, 0, 0);

	setConnections();
}

MonitoringPlot::~MonitoringPlot()
{
	
}

void MonitoringPlot::setConnections()
{
	connect(tbtPlotSettings, SIGNAL(clicked()), this, SLOT(setPlotSettings()));
	connect(tbtAxisSettings, SIGNAL(clicked()), this, SLOT(setAxisSettings()));
	connect(tbtCurveSettings, SIGNAL(clicked()), this, SLOT(setCurveSettings()));
}

void MonitoringPlot::addParameter(const QString &param_name, const QString &title, const QString &units, const double max_value)
{
	QFont font;
	font.setPointSize(9);
	font.setBold(false);
	
	QFrame *frame = new QFrame(scrollAreaWidgetContents);
	QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());	
	frame->setSizePolicy(sizePolicy);
	frame->setFrameShape(QFrame::NoFrame);
	frame->setFrameShadow(QFrame::Plain);
	QHBoxLayout *horizontalLayout_4 = new QHBoxLayout(frame);
	horizontalLayout_4->setSpacing(2);
	horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
	QCheckBox *chbxPlotted = new QCheckBox(frame);	
	horizontalLayout_4->addWidget(chbxPlotted);
	QLabel *lblTitle = new QLabel(title, frame);
	lblTitle->setObjectName(QStringLiteral("lblTitle"));
	lblTitle->setFont(font);
	horizontalLayout_4->addWidget(lblTitle);	

	QFrame *frame_value = new QFrame(scrollAreaWidgetContents);
	frame_value->setObjectName(QStringLiteral("frame_value"));
	frame_value->setFrameShape(QFrame::NoFrame);
	frame_value->setFrameShadow(QFrame::Plain);
	QHBoxLayout *horizontalLayout_5 = new QHBoxLayout(frame_value);
	horizontalLayout_5->setSpacing(2);
	horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
	horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
	QLineEdit *ledValue = new QLineEdit(frame_value);
	ledValue->setObjectName(QStringLiteral("ledValue"));
	ledValue->setMinimumSize(QSize(80, 0));
	ledValue->setMaximumSize(QSize(120, 16777215));
	ledValue->setFont(font);
	ledValue->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	ledValue->setReadOnly(true);
	horizontalLayout_5->addWidget(ledValue);
	QLabel *lblUnits = new QLabel(units, frame_value);
	lblUnits->setObjectName(QStringLiteral("lblUnits"));
	horizontalLayout_5->addWidget(lblUnits);

	m_Parameter new_parameter;
	new_parameter.param_name = param_name;
	new_parameter.units = units;
	new_parameter.max_value = max_value;
	new_parameter.last_value = "";
	new_parameter.plot_state = false;
	new_parameter.widgets << chbxPlotted << lblTitle << ledValue << lblUnits; // << lblMaxValue;
	new_parameter.curve = new QwtPlotCurve(param_name);
	new_parameter.curve->attach(qwtPlot);
	parameters.append(new_parameter);

	int params_count = parameters.count();
	QGridLayout *gridLayout = qobject_cast<QGridLayout*>(scrollAreaWidgetContents->layout());
	Q_ASSERT_X(gridLayout != NULL, "addParameter()", "Cannot convert scrollAreaWidgetContents layout to QGridLayout.");
	
	gridLayout->removeItem(spacer_last);
	gridLayout->addWidget(frame, params_count-1, 0);
	gridLayout->addWidget(frame_value, params_count-1, 1);	
	gridLayout->addItem(spacer_last, params_count, 0); 	

	connect(chbxPlotted, SIGNAL(checked(bool)), this, SLOT(setParamChecked(bool)));
}

void MonitoringPlot::setParameterValue(const QString &param_name, const double val)
{
	for (int i = 0; i < parameters.count(); i++)
	{
		if (parameters[i].param_name == param_name)
		{
			m_Parameter *parameter = &parameters[i];
			parameter->last_value = QString::number(val);
			parameter->ydata.append(val);

			QDateTime ctime = QDateTime::currentDateTime();
			uint utime = ctime.toTime_t();
			parameter->xtime.append(utime);	

			QLineEdit *led = NULL;
			findWidget(parameter->widgets, led);			
			if (led != NULL)
			{
				led->setText(parameter->last_value);
				QPalette p = led->palette();
				QColor color = QColor(0x99, 0xFF, 0x00, 0xFF);	// light green
				if (val > parameter->max_value) color = QColor(Qt::red);
				p.setColor(QPalette::Foreground, color);
				led->setPalette(p);
			}			

			// Update curve on qwtPlot !
			// .....
		}
	}
}

void MonitoringPlot::setParamChecked(bool flag)
{
	QCheckBox *chbx = (QCheckBox*)sender();
	if (!chbx) return;
		
	for (int i = 0; i < parameters.count(); i++)
	{
		m_Parameter *parameter = &parameters[i];
		QCheckBox *cur_chbx = NULL;
		findWidget(parameter->widgets, cur_chbx);
		if (cur_chbx == chbx)
		{			
			parameter->plot_state = flag;
			// Add or remove parameter's QwtCurve to/from qwtPlot !
			// ...
		}
	}
	

}

void MonitoringPlot::setPlotSettings()
{
	PlotDialog plot_dlg(qwtPlot, grid, this);
	if (plot_dlg.exec())
	{
		qwtPlot->setCanvasBackground(QBrush(plot_dlg.getBackgroundColor()));
		qwtPlot->setTitle(plot_dlg.getTitle());

		grid->enableX(plot_dlg.enabledGridMajorX());
		grid->enableY(plot_dlg.enabledGridMajorY());
		grid->enableXMin(plot_dlg.enabledGridMinorX());
		grid->enableYMin(plot_dlg.enabledGridMinorY());
		grid->setMajorPen(plot_dlg.getGridMajorLine());
		grid->setMinorPen(plot_dlg.getGridMinorLine());

		qwtPlot->replot();
	}
}

void MonitoringPlot::setAxisSettings()
{
	AxisDialog axis_dlg(qwtPlot, this);
	axis_dlg.hideRangeSettings(true);	
	if (axis_dlg.exec())
	{
		for (int i = 0; i < 4; i++)
		{
			qwtPlot->enableAxis(QwtPlot::Axis(i), axis_dlg.axisVisible(QwtPlot::Axis(i)));
			qwtPlot->setAxisTitle(QwtPlot::Axis(i), axis_dlg.getAxisTitle(QwtPlot::Axis(i)));

			QPalette p_axis = qwtPlot->axisWidget(QwtPlot::Axis(i))->palette();
			p_axis.setColor(QPalette::Foreground, axis_dlg.getAxisColor(QwtPlot::Axis(i)));
			p_axis.setColor(QPalette::Text, axis_dlg.getAxisLabelColor(QwtPlot::Axis(i)));
			qwtPlot->axisWidget(QwtPlot::Axis(i))->setPalette(p_axis);

			qwtPlot->setAxisFont(QwtPlot::Axis(i), axis_dlg.getAxisLabelFont(QwtPlot::Axis(i)));
			qwtPlot->axisWidget(QwtPlot::Axis(i))->scaleDraw()->setPenWidth(axis_dlg.getAxisThickness(QwtPlot::Axis(i)));

			bool auto_scale = axis_dlg.axisAutoRescale(QwtPlot::Axis(i));
			qwtPlot->setAxisAutoScale(QwtPlot::Axis(i), auto_scale);
			if (!auto_scale)
			{
				QPair<double,double> from_to = axis_dlg.getAxisScaleRange(QwtPlot::Axis(i));
				double step = axis_dlg.getAxisScaleStep(QwtPlot::Axis(i));
				qwtPlot->setAxisScale(QwtPlot::Axis(i), from_to.first, from_to.second, step);				
			}

			QwtScaleEngine *scale_engine = axis_dlg.getAxisScaleType(QwtPlot::Axis(i));
			qwtPlot->setAxisScaleEngine(QwtPlot::Axis(i), scale_engine);

			qwtPlot->setAxisMaxMinor(QwtPlot::Axis(i), axis_dlg.getAxisTicks(QwtPlot::Axis(i)));
		}

		qwtPlot->replot();
	}	
}

QList<QwtPlotCurve*> MonitoringPlot::getCurveList()
{
	QList<QwtPlotCurve*> curve_list;
	for (int i = 0; i < parameters.count(); i++)
	{
		QwtPlotCurve *curve = parameters[i].curve;
		curve_list.append(curve);
	}

	return curve_list;
}

void MonitoringPlot::setCurveSettings()
{
	/*QList<QwtPlotCurve*> curve_list = getCurveList();
	CurveDialog dlg(&curve_list, false);
	if (dlg.exec())
	{
		QList<QwtCurveSettings*> *curve_settings = dlg.getCurveSettingsList();
		for (int i = 0; i < curve_settings->size(); i++)
		{
			QwtCurveSettings *settings = curve_settings->at(i);
			QwtPlotCurve *curve = curve_list[i];
			curve->setTitle(settings->title);
			curve->setPen(settings->pen);
			QwtSymbol *_symbol = new QwtSymbol(settings->symbol->style(), settings->symbol->brush(), settings->symbol->pen(), settings->symbol->size()); 
			curve->setSymbol(_symbol);
		}
		qwtPlot->replot();
	}	*/
}


OscilloscopeWidget::OscilloscopeWidget(QWidget *tab, QSettings *settings, QVector<ToolChannel*> tl_channels, QWidget *parent)
{
	setParent(parent);
	//this->setObjectName("tabOscilloscope");
	app_settings = settings;
	tool_channels = tl_channels;

	QSplitter *osc_splitter = new QSplitter(tab);
	osc_splitter->setHandleWidth(2);
	osc_splitter->setChildrenCollapsible(false);
	osc_splitter->setOrientation(Qt::Vertical);	
	oscDataPlot = new DataPlot(QString("SignalPlot"), app_settings, osc_splitter);
	oscMathPlot = new DataPlot(QString("SignalMathPlot"), app_settings, osc_splitter);
	osc_splitter->addWidget(oscDataPlot);
	osc_splitter->addWidget(oscMathPlot);	
	oscdata_manager = new PlottedDataManager(oscDataPlot, oscMathPlot, app_settings, tool_channels, tab);	
	oscdata_manager->hideMovingAverWidget();
	oscdata_manager->setMaximumWidth(450);
	oscdata_manager->refreshToolChannels(tool_channels);
	QSplitter *osc2_splitter = new QSplitter(tab);
	osc2_splitter->setHandleWidth(2);
	osc2_splitter->setChildrenCollapsible(false);
	osc2_splitter->setOrientation(Qt::Horizontal);
	osc2_splitter->addWidget(osc_splitter);	
	osc2_splitter->addWidget(oscdata_manager);
	QGridLayout *grlout_osc = new QGridLayout(tab);
	grlout_osc->setContentsMargins(1,1,1,1);
	grlout_osc->addWidget(osc2_splitter, 0, 0, 1, 1);
	
	double NMR_SAMPLE_FREQ = (4*250000);
	ToolChannel *cur_tool_channel = oscdata_manager->getCurrentToolChannel();
	if (cur_tool_channel)
	{
		NMR_SAMPLE_FREQ = cur_tool_channel->sample_freq*1000;	// originally sample freq in [kHz]
	}
	
	double osc_time_min = 0;
	double osc_time_max = 2.0/NMR_SAMPLE_FREQ*1024*1000000;		// 1024 - число комплексных точек (которых = 2048/2), 1000000 - переводной множитель в мкс
	oscDataPlot->getPlot()->setAxisScale(QwtPlot::xBottom, osc_time_min, osc_time_max);

	double osc_freq_min = 0;
	double osc_freq_max = NMR_SAMPLE_FREQ/2.0/2/1000;			// NMR_SAMPLE_FREQ/2.0 - фактическая частота дискретизации комплексных точек, 1000 - переводной коэффициент в кГц
	oscMathPlot->getPlot()->setAxisScale(QwtPlot::xBottom, osc_freq_min, osc_freq_max);

	oscDataPlot->setAutoRescale(QwtPlot::yLeft, false);
	oscDataPlot->setAutoRescale(QwtPlot::xBottom, false);
	oscMathPlot->setAutoRescale(QwtPlot::yLeft, false);
	oscMathPlot->setAutoRescale(QwtPlot::xBottom, false);

	setConnections();
}

OscilloscopeWidget::~OscilloscopeWidget()
{
	
}

void OscilloscopeWidget::setConnections()
{
	connect(oscdata_manager, SIGNAL(plot_dataset(PlottedDataSet*)), oscDataPlot, SLOT(plotDataSet(PlottedDataSet*)));
	connect(oscdata_manager, SIGNAL(plot_dataset(PlottedDataSet*)), oscMathPlot, SLOT(plotDataSet(PlottedDataSet*)));
	connect(oscdata_manager, SIGNAL(dataset_removed(QString&)), oscDataPlot, SLOT(removeData(QString&)));
	connect(oscdata_manager, SIGNAL(dataset_removed(QString&)), oscMathPlot, SLOT(removeData(QString&)));
	connect(oscdata_manager, SIGNAL(apply_win_func(QVector<int>&)), this, SLOT(applyWinFuncParams(QVector<int>&)));
	connect(oscdata_manager, SIGNAL(apply_moving_averaging(bool, int)), this, SIGNAL(apply_moving_averaging(bool, int)));
}

void OscilloscopeWidget::addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, bool _hold_on)
{
	oscdata_manager->addDataSet(_name, _plot, _x, _y, _bad_map, _settings, _hold_on);
}

void OscilloscopeWidget::addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, bool _hold_on, int def_index)
{
	oscdata_manager->addDataSet(_name, _plot, _x, _y, _bad_map, _hold_on, def_index);
}

void OscilloscopeWidget::refreshDataSets()
{
	oscdata_manager->refreshDataSets();
}

void OscilloscopeWidget::applyWinFuncParams(QVector<int> &params)
{
	emit apply_win_func(params);
}


RelaxationWidget::RelaxationWidget(QWidget *tab, QSettings *settings, QWidget *parent)
{
	setParent(parent);
	//this->setObjectName("tabRelax");
	app_settings = settings;

	QSplitter *relax_splitter = new QSplitter(tab);
	relax_splitter->setOrientation(Qt::Vertical);	
	relaxDataPlot = new DataPlot(QString("RelaxPlot"), app_settings, relax_splitter);
	relaxMathPlot = new DataPlot(QString("RelaxMathPlot"), app_settings, relax_splitter);
	relax_splitter->addWidget(relaxDataPlot);
	relax_splitter->addWidget(relaxMathPlot);	
	relax_data_manager = new PlottedDataManager(relaxDataPlot, relaxMathPlot, app_settings, QVector<ToolChannel*>(), tab);	
	relax_data_manager->hideWinWidgets();
	relax_data_manager->setMaximumWidth(450);
	QSplitter *relax2_splitter = new QSplitter(tab);
	relax2_splitter->setOrientation(Qt::Horizontal);
	relax2_splitter->addWidget(relax_splitter);	
	relax2_splitter->addWidget(relax_data_manager);
	QGridLayout *grlout_osc = new QGridLayout(tab);
	grlout_osc->setContentsMargins(1,1,1,1);
	grlout_osc->addWidget(relax2_splitter, 0, 0, 1, 1);

	relaxDataPlot->setAutoRescale(QwtPlot::yLeft, false);
	relaxDataPlot->setAutoRescale(QwtPlot::xBottom, false);
	relaxMathPlot->setAutoRescale(QwtPlot::yLeft, false);
	relaxMathPlot->setAutoRescale(QwtPlot::xBottom, false);

	relaxMathPlot->getPlot()->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);

	//relaxMathPlot->setVisible(false);			// Temporary !

	setConnections();
}

RelaxationWidget::~RelaxationWidget()
{

}

void RelaxationWidget::setConnections()
{
	connect(relax_data_manager, SIGNAL(plot_dataset(PlottedDataSet*)), relaxDataPlot, SLOT(plotDataSet(PlottedDataSet*)));
	connect(relax_data_manager, SIGNAL(plot_dataset(PlottedDataSet*)), relaxMathPlot, SLOT(plotDataSet(PlottedDataSet*)));
	connect(relax_data_manager, SIGNAL(dataset_removed(QString&)), relaxDataPlot, SLOT(removeData(QString&)));
	connect(relax_data_manager, SIGNAL(dataset_removed(QString&)), relaxMathPlot, SLOT(removeData(QString&)));
	connect(relax_data_manager, SIGNAL(apply_moving_averaging(bool, int)), this, SIGNAL(apply_moving_averaging(bool, int)));
}

void RelaxationWidget::addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, bool _hold_on)
{
	relax_data_manager->addDataSet(_name, _plot, _x, _y, _bad_map, _settings, _hold_on);
}

void RelaxationWidget::addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, bool _hold_on, int def_index)
{
	relax_data_manager->addDataSet(_name, _plot, _x, _y, _bad_map, _hold_on, def_index);
}

/*void RelaxationWidget::addDataSet(QString _name, DataPlot *_plot, QVector<double> &_x, QVector<double> &_y, QString parent_dataset_name, int def_index)
{		
	relax_data_manager->addDataSet(_name, _plot, _x, _y, parent_dataset_name, def_index);
}*/

void RelaxationWidget::refreshDataSets()
{
	relax_data_manager->refreshDataSets();
}



SDSPWidget::SDSPWidget(QSettings *app_settings, QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
			
	//app_settings = new QSettings("SDSP.ini", QSettings::IniFormat, this);
	this->app_settings = app_settings;
	initAllSettings();

	base_date_time = QDateTime::currentDateTime();	
	
	ui.dsbxTimePhase->setValue(phase_plot_settings.time_scale.second);
	ui.dsbxTimeAmpl->setValue(ampl_plot_settings.time_scale.second);
	
	ui.qwtPlotPhase->setCanvasBackground(QBrush(QColor(Qt::white)));
	ui.qwtPlotAmplitude->setCanvasBackground(QBrush(QColor(Qt::white)));
	
	ui.qwtPlotPhase->enableAxis(QwtPlot::xTop);
	//ui.qwtPlotPhase->setAxisScaleDraw(QwtPlot::yLeft, new TimeScaleDrawSec(base_date_time.addSecs(-phase_plot_settings.time_scale.second*60)));		
	ui.qwtPlotPhase->setAxisScaleDraw(QwtPlot::yLeft, new TimeScaleDrawSec(base_date_time.addSecs(-phase_plot_settings.time_scale.second)));
	ui.qwtPlotPhase->setAxisScale(QwtPlot::xBottom, phase_plot_settings.min_zoom, phase_plot_settings.max_zoom);
	ui.qwtPlotPhase->setAxisScale(QwtPlot::xTop, phase_plot_settings.min_zoom, phase_plot_settings.max_zoom);
	//ui.qwtPlotPhase->setAxisScale(QwtPlot::yLeft, 0, phase_plot_settings.time_scale.second*60);
	ui.qwtPlotPhase->setAxisScale(QwtPlot::yLeft, 0, phase_plot_settings.time_scale.second);
	ui.qwtPlotPhase->canvas()->setCursor(Qt::ArrowCursor);
	ui.qwtPlotPhase->replot();

	ui.qwtPlotAmplitude->enableAxis(QwtPlot::xTop);
	//ui.qwtPlotAmplitude->setAxisScaleDraw(QwtPlot::yLeft, new TimeScaleDrawSec(base_date_time.addSecs(-ampl_plot_settings.time_scale.second*60)));	
	ui.qwtPlotAmplitude->setAxisScaleDraw(QwtPlot::yLeft, new TimeScaleDrawSec(base_date_time.addSecs(-ampl_plot_settings.time_scale.second)));
	ui.qwtPlotAmplitude->setAxisScale(QwtPlot::xBottom, ampl_plot_settings.min_zoom, ampl_plot_settings.max_zoom);
	ui.qwtPlotAmplitude->setAxisScale(QwtPlot::xTop, ampl_plot_settings.min_zoom, ampl_plot_settings.max_zoom);
	//ui.qwtPlotAmplitude->setAxisScale(QwtPlot::yLeft, 0, ampl_plot_settings.time_scale.second*60);
	ui.qwtPlotAmplitude->setAxisScale(QwtPlot::yLeft, 0, ampl_plot_settings.time_scale.second);
	ui.qwtPlotAmplitude->canvas()->setCursor(Qt::ArrowCursor);
	ui.qwtPlotAmplitude->replot();

	QwtPlotGrid *grid_phase = new QwtPlotGrid;
	grid_phase->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_phase->enableX(true);	
	grid_phase->enableY(true);	
	grid_phase->attach(ui.qwtPlotPhase);	

	QwtPlotGrid *grid_ampl = new QwtPlotGrid;
	grid_ampl->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_ampl->enableX(true);	
	grid_ampl->enableY(true);	
	grid_ampl->attach(ui.qwtPlotAmplitude);	

	QwtPlotPanner *panner_phase = new QwtPlotPanner(ui.qwtPlotPhase->canvas());
	panner_phase->setAxisEnabled(QwtPlot::yRight, false);
	panner_phase->setMouseButton(Qt::RightButton);

	QwtPlotPanner *panner_ampl = new QwtPlotPanner(ui.qwtPlotAmplitude->canvas());
	panner_ampl->setAxisEnabled(QwtPlot::yRight, false);
	panner_ampl->setMouseButton(Qt::RightButton);

	/*QwtPlotPicker *picker_phase = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui.qwtPlotPhase->canvas());	
	picker_phase->setRubberBandPen(QColor(Qt::green));
	picker_phase->setRubberBand(QwtPicker::CrossRubberBand);
	picker_phase->setTrackerPen(QColor(Qt::darkMagenta));

	QwtPlotPicker *picker_ampl = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui.qwtPlotAmplitude->canvas());	
	picker_ampl->setRubberBandPen(QColor(Qt::green));
	picker_ampl->setRubberBand(QwtPicker::CrossRubberBand);
	picker_ampl->setTrackerPen(QColor(Qt::darkMagenta));*/
		
	QwtText xbottom_title_phase;
	QString xbottom_title_text_phase = QString("Phase Difference, degree");
	xbottom_title_phase.setText(xbottom_title_text_phase);
	xbottom_title_phase.setColor(QColor(Qt::darkBlue));
	QFont xbottom_font_phase = ui.qwtPlotPhase->axisTitle(QwtPlot::yLeft).font();
	QFont xbottom_axis_font_phase = ui.qwtPlotPhase->axisFont(QwtPlot::yLeft);
	xbottom_axis_font_phase.setPointSize(9);
	xbottom_axis_font_phase.setBold(false);
	xbottom_font_phase.setPointSize(10);
	xbottom_font_phase.setBold(false);
	xbottom_title_phase.setFont(xbottom_font_phase);

	ui.qwtPlotPhase->setAxisFont(QwtPlot::yLeft, xbottom_axis_font_phase);	
	ui.qwtPlotPhase->setAxisTitle(QwtPlot::xTop, xbottom_title_phase);
	ui.qwtPlotPhase->setAxisFont(QwtPlot::xBottom, xbottom_axis_font_phase);
	ui.qwtPlotPhase->setAxisFont(QwtPlot::xTop, xbottom_axis_font_phase);

	QwtText xbottom_title_ampl;
	QString xbottom_title_text_ampl = QString("Amplitude Ratio");
	xbottom_title_ampl.setText(xbottom_title_text_ampl);
	xbottom_title_ampl.setColor(QColor(Qt::darkBlue));
	QFont xbottom_font_ampl = ui.qwtPlotAmplitude->axisTitle(QwtPlot::yLeft).font();
	QFont xbottom_axis_font_ampl = ui.qwtPlotAmplitude->axisFont(QwtPlot::yLeft);
	xbottom_axis_font_ampl.setPointSize(9);
	xbottom_axis_font_ampl.setBold(false);
	xbottom_font_ampl.setPointSize(10);
	xbottom_font_ampl.setBold(false);
	xbottom_title_ampl.setFont(xbottom_font_ampl);

	ui.qwtPlotAmplitude->setAxisFont(QwtPlot::yLeft, xbottom_axis_font_ampl);
	ui.qwtPlotAmplitude->setAxisTitle(QwtPlot::xTop, xbottom_title_ampl);
	ui.qwtPlotAmplitude->setAxisFont(QwtPlot::xBottom, xbottom_axis_font_ampl);
	ui.qwtPlotAmplitude->setAxisFont(QwtPlot::xTop, xbottom_axis_font_ampl);
		
	ui.pbtApply->setIcon(QIcon(":/images/apply.png"));
	//ui.pbtView->setIcon(QIcon(":/images/view_text.png"));
	//ui.pbtStart->setIcon(QIcon(":/images/play_red.png"));
	//ui.pbtSave->setIcon(QIcon(":/images/save.png"));

	ui.tbtZoomPhase->setIcon(QIcon(":/images/zoom.png"));
	ui.tbtZoomInPhase->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtZoomOutPhase->setIcon(QIcon(":/images/zoom_out.png"));

	ui.tbtZoomAmpl->setIcon(QIcon(":/images/zoom.png"));
	ui.tbtZoomInAmpl->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtZoomOutAmpl->setIcon(QIcon(":/images/zoom_out.png"));

	ui.lblPhase->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lbl_PhaseDiff->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lblRatio->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lbl_RatioAmpl->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
			
	
	ui.ledDAC->setValidator(new QIntValidator(this));
	ui.ledGain132->setValidator(new QIntValidator(this));
	ui.ledGain160->setValidator(new QIntValidator(this));
	ui.ledGain232->setValidator(new QIntValidator(this));
	ui.ledGain260->setValidator(new QIntValidator(this));
	ui.ledPower32->setValidator(new QIntValidator(this));
	ui.ledPower60->setValidator(new QIntValidator(this));
	//ui.ledRepetitions->setValidator(new QIntValidator(this));

	//ui.pbtStart->setEnabled(false);
	//ui.pbtStart->setVisible(false);

	is_enabled = false;
	is_waiting = false;
	is_started = false;
	is_running = false;

	setConnections();

	ui.slidDAC->setValue(sdsp_params.DAC.first);
	int power32 = (int)(pow(10, sdsp_params.freq32_params.power.first/32000.0) + 0.5);
	ui.slidPower32->setValue(power32);
	ui.slidGain132->setValue(sdsp_params.freq32_params.gain1.first);
	ui.slidGain232->setValue(sdsp_params.freq32_params.gain2.first);
	int power60 = (int)(pow(10, sdsp_params.freq60_params.power.first/32000.0) + 0.5);
	ui.slidPower60->setValue(power60);
	ui.slidGain160->setValue(sdsp_params.freq60_params.gain1.first);
	ui.slidGain260->setValue(sdsp_params.freq60_params.gain2.first);
	//ui.slidRepetitions->setValue(sdsp_params.repetitions.first);
	//ui.chboxMean->setChecked(sdsp_params.average.first);
	ui.chboxChannel1->setChecked(sdsp_params.channel1.first);
	ui.chboxChannel2->setChecked(sdsp_params.channel2.first);
	//ui.chboxPower->setChecked(sdsp_params.power_status.first);
	ui.gbxPower->setChecked(sdsp_params.power_status.first);
	ui.chboxTransmitter->setChecked(sdsp_params.transmitter.first);
	ui.gbxFreq32->setChecked(true);
	ui.gbxFreq60->setChecked(false);
	
	ui.ledDAC->setText(QString::number(sdsp_params.DAC.first));
	ui.ledPower32->setText(QString::number(sdsp_params.freq32_params.power.first));
	ui.ledPower60->setText(QString::number(sdsp_params.freq60_params.power.first));
	ui.ledGain132->setText(QString::number(sdsp_params.freq32_params.gain1.first));
	ui.ledGain160->setText(QString::number(sdsp_params.freq60_params.gain1.first));
	ui.ledGain232->setText(QString::number(sdsp_params.freq32_params.gain2.first));
	ui.ledGain260->setText(QString::number(sdsp_params.freq60_params.gain2.first));
	//ui.ledRepetitions->setText(QString::number(sdsp_params.repetitions.first));

	phaseData.clear();	
	SDSPCurve_Settings phase_curve_settings;
	phase_curve_settings.pen_color = QColor(Qt::darkGreen);
	phase_curve_settings.pen_width = 1;
	phase_curve_settings.sym_border_color = QColor(Qt::darkGreen);
	phase_curve_settings.sym_border_width = 1;
	phase_curve_settings.sym_color = QColor(Qt::green);
	phase_curve_settings.sym_size = 6;
	phase_curve_settings.sym_style = QwtSymbol::Ellipse;
	addSDSPCurve(phaseData, "PhaseDifference", phase_curve_settings);	
	
	amplData.clear();
	SDSPCurve_Settings ampl_curve_settings;
	ampl_curve_settings.pen_color = QColor(Qt::darkYellow);
	ampl_curve_settings.pen_width = 1;
	ampl_curve_settings.sym_border_color = QColor(Qt::darkYellow);
	ampl_curve_settings.sym_border_width = 1;
	ampl_curve_settings.sym_color = QColor(Qt::yellow);
	ampl_curve_settings.sym_size = 6;
	ampl_curve_settings.sym_style = QwtSymbol::Ellipse;
	addSDSPCurve(amplData, "AmplitudeRatio", ampl_curve_settings);		
	
	is_running = true;

	ticks = 0;
	timer.setInterval(1000);
	timer.start();	

	cmd_timer.setInterval(500);
	cmd_timer.start();	
}

SDSPWidget::~SDSPWidget()
{
	qDeleteAll(phaseData);
	qDeleteAll(amplData);

	//delete app_settings;
}

void SDSPWidget::setConnections()
{
	connect(ui.gbxFreq32, SIGNAL(toggled(bool)), this, SLOT(selectFreq(bool)));
	connect(ui.gbxFreq60, SIGNAL(toggled(bool)), this, SLOT(selectFreq(bool)));
	connect(ui.slidGain132, SIGNAL(valueChanged(int)), this, SLOT(setGain1(int)));
	connect(ui.slidGain232, SIGNAL(valueChanged(int)), this, SLOT(setGain2(int)));
	connect(ui.slidGain160, SIGNAL(valueChanged(int)), this, SLOT(setGain1(int)));
	connect(ui.slidGain260, SIGNAL(valueChanged(int)), this, SLOT(setGain2(int)));
	connect(ui.slidPower32, SIGNAL(valueChanged(int)), this, SLOT(setPower(int)));
	connect(ui.slidPower60, SIGNAL(valueChanged(int)), this, SLOT(setPower(int)));
	connect(ui.slidDAC, SIGNAL(valueChanged(int)), this, SLOT(setDAC(int)));
	//connect(ui.slidRepetitions, SIGNAL(valueChanged(int)), this, SLOT(setRepetitions(int)));
	connect(ui.ledDAC, SIGNAL(textEdited(QString)), this, SLOT(setDAC(QString)));
	connect(ui.ledPower32, SIGNAL(textEdited(QString)), this, SLOT(setPower(QString)));
	connect(ui.ledPower60, SIGNAL(textEdited(QString)), this, SLOT(setPower(QString)));
	connect(ui.ledGain132, SIGNAL(textEdited(QString)), this, SLOT(setGain1(QString)));
	connect(ui.ledGain160, SIGNAL(textEdited(QString)), this, SLOT(setGain1(QString)));
	connect(ui.ledGain232, SIGNAL(textEdited(QString)), this, SLOT(setGain2(QString)));
	connect(ui.ledGain260, SIGNAL(textEdited(QString)), this, SLOT(setGain2(QString)));
	connect(ui.chboxTransmitter, SIGNAL(toggled(bool)), this, SLOT(setTransmitter(bool)));
	connect(ui.chboxChannel1, SIGNAL(toggled(bool)), this, SLOT(setChannel1(bool)));
	connect(ui.chboxChannel2, SIGNAL(toggled(bool)), this, SLOT(setChannel2(bool)));
	//connect(ui.chboxPower, SIGNAL(toggled(bool)), this, SLOT(setPowerStatus(bool)));
	connect(ui.gbxPower, SIGNAL(toggled(bool)), this, SLOT(setPowerStatus(bool)));
	//connect(ui.ledRepetitions, SIGNAL(textChanged(QString)), this, SLOT(setRepetitions(QString)));

	connect(ui.tbtZoomInPhase, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(ui.tbtZoomOutPhase, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(ui.tbtZoomPhase, SIGNAL(clicked()), this, SLOT(adjustMinMax()));
	connect(ui.tbtZoomInAmpl, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(ui.tbtZoomOutAmpl, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(ui.tbtZoomAmpl, SIGNAL(clicked()), this, SLOT(adjustMinMax()));	
	connect(ui.dsbxTimePhase, SIGNAL(valueChanged(double)), this, SLOT(changeTimeRange(double)));
	connect(ui.dsbxTimeAmpl, SIGNAL(valueChanged(double)), this, SLOT(changeTimeRange(double)));
		
	connect(&timer, SIGNAL(timeout()), this, SLOT(newTick()));
	connect(&cmd_timer, SIGNAL(timeout()), this, SLOT(checkAllParamStates()));

	connect(ui.pbtApply, SIGNAL(clicked()), this, SLOT(applyParameters()));
	//connect(ui.pbtSave, SIGNAL(clicked()), this, SLOT(saveSettings()));
}

void SDSPWidget::selectFreq(bool flag)
{
	if (!flag)
	{
		if (!ui.gbxFreq32->isChecked() && !ui.gbxFreq60->isChecked()) 
		{
			QGroupBox *gbx = (QGroupBox*)sender();
			if (gbx == ui.gbxFreq32) ui.gbxFreq60->setChecked(true);
			else if (gbx == ui.gbxFreq60) ui.gbxFreq32->setChecked(true);			
		}
		return;
	}	
		
	QGroupBox *gbx = (QGroupBox*)sender();
	if (gbx == ui.gbxFreq32)
	{
		//sdsp_params.freq = 32;
		sdsp_params.set(SDSP_Params::Freq, 32);
		ui.gbxFreq60->setChecked(false);
	}
	else if (gbx == ui.gbxFreq60)
	{
		//sdsp_params.freq = 60;
		sdsp_params.set(SDSP_Params::Freq, 60);
		ui.gbxFreq32->setChecked(false);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setDAC(int val)
{
	ui.ledDAC->setText(QString::number(val));
	//sdsp_params.DAC = val;
	sdsp_params.set(SDSP_Params::DAC_Code, val);

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setPower(int val)
{
	val = int(log10(val)*32000);

	QSlider *slider = (QSlider*)sender();	
	if (slider == ui.slidPower32)
	{		
		ui.ledPower32->setText(QString::number(val));
		//sdsp_params.freq32_params.power = val;
		sdsp_params.set(SDSP_Params::Power_32, val);
	}
	else if (slider == ui.slidPower60)
	{
		ui.ledPower60->setText(QString::number(val));
		//sdsp_params.freq60_params.power = val;
		sdsp_params.set(SDSP_Params::Power_60, val);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setGain1(int val)
{
	QSlider *slider = (QSlider*)sender();
	if (slider == ui.slidGain132)
	{
		ui.ledGain132->setText(QString::number(val));
		//sdsp_params.freq32_params.gain1 = val;
		sdsp_params.set(SDSP_Params::Gain1_32, val);
	}
	else if (slider == ui.slidGain160)
	{
		ui.ledGain160->setText(QString::number(val));
		//sdsp_params.freq60_params.gain1 = val;
		sdsp_params.set(SDSP_Params::Gain1_60, val);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setGain2(int val)
{
	QSlider *slider = (QSlider*)sender();
	if (slider == ui.slidGain232)
	{
		ui.ledGain232->setText(QString::number(val));
		//sdsp_params.freq32_params.gain2 = val;
		sdsp_params.set(SDSP_Params::Gain2_32, val);
	}
	else if (slider == ui.slidGain260)
	{
		ui.ledGain260->setText(QString::number(val));
		//sdsp_params.freq60_params.gain2 = val;
		sdsp_params.set(SDSP_Params::Gain2_60, val);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setTransmitter(bool flag)
{
	//sdsp_params.transmitter = flag;
	sdsp_params.set(SDSP_Params::Transmitter, flag);

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setChannel1(bool flag)
{
	//sdsp_params.channel1 = flag;
	sdsp_params.set(SDSP_Params::Channel_1, flag);

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setChannel2(bool flag)
{
	//sdsp_params.channel2 = flag;
	sdsp_params.set(SDSP_Params::Channel_2, flag);

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setPowerStatus(bool flag)
{
	//sdsp_params.power_status = flag;
	sdsp_params.set(SDSP_Params::Power_Status, flag);

	ui.chboxTransmitter->setChecked(flag);
	ui.chboxChannel1->setChecked(flag);
	ui.chboxChannel2->setChecked(flag);

	if (flag && is_running) applyParameters();
	
	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

/*
void SDSPWidget::setRepetitions(int val)
{
	QSlider *slider = (QSlider*)sender();
	if (slider == ui.slidRepetitions)
	{
		ui.ledRepetitions->setText(QString::number(val));
		//sdsp_params.repetitions = val;
		sdsp_params.set(SDSP_Params::Repetitions, val);
		if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
	}
}
*/

void SDSPWidget::setDAC(QString text)
{
	bool ok;
	int val = text.toInt(&ok);
	if (!ok) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect DAC Code was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	if (val < 0 || val > 63) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect DAC Code was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	ui.slidDAC->setValue(val);
	//sdsp_params.DAC = val;
	sdsp_params.set(SDSP_Params::DAC_Code, val);

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setGain1(QString text)
{
	QLineEdit *led = (QLineEdit*)sender();

	bool ok;
	int val = text.toInt(&ok);
	if (!ok) 
	{		
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Gain1 value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		if (led == ui.ledGain132) 
		{ 
			ui.slidGain132->setValue(0); 
			//sdsp_params.freq32_params.gain1 = 0; 
			sdsp_params.set(SDSP_Params::Gain1_32, 0);
		}
		else if (led == ui.ledGain160) 
		{
			ui.slidGain160->setValue(0);
			//sdsp_params.freq60_params.gain1 = 0;
			sdsp_params.set(SDSP_Params::Gain1_60, 0);
		}
		return;
	}
	if (val < 0 || val > 15) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Gain1 value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		if (led == ui.ledGain132) 
		{
			ui.slidGain132->setValue(0);
			//sdsp_params.freq32_params.gain1 = 0; 
			sdsp_params.set(SDSP_Params::Gain1_32, 0);
		}
		else if (led == ui.ledGain160) 
		{
			ui.slidGain160->setValue(0);
			//sdsp_params.freq60_params.gain1 = 0; 
			sdsp_params.set(SDSP_Params::Gain1_60, 0);
		}
		return;
	}

	if (led == ui.ledGain132)
	{
		ui.slidGain132->setValue(val);
		//sdsp_params.freq32_params.gain1 = val;
		sdsp_params.set(SDSP_Params::Gain1_32, val);
	}
	else if (led == ui.ledGain160)
	{
		ui.slidGain160->setValue(val);
		//sdsp_params.freq60_params.gain1 = val;
		sdsp_params.set(SDSP_Params::Gain1_60, val);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setGain2(QString text)
{
	QLineEdit *led = (QLineEdit*)sender();

	bool ok;
	int val = text.toInt(&ok);
	if (!ok) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Gain2 value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		if (led == ui.ledGain232) 
		{
			ui.slidGain232->setValue(0);
			//sdsp_params.freq32_params.gain2 = 0; 
			sdsp_params.set(SDSP_Params::Gain2_32, 0);
		}
		else if (led == ui.ledGain260) 
		{ 
			ui.slidGain260->setValue(0); 
			//sdsp_params.freq60_params.gain2 = 0; 
			sdsp_params.set(SDSP_Params::Gain2_60, 0);
		}
		return;
	}
	if (val < 0 || val > 15) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Gain2 value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		if (led == ui.ledGain232) 
		{ 
			ui.slidGain232->setValue(0); 
			//sdsp_params.freq32_params.gain2 = 0; 
			sdsp_params.set(SDSP_Params::Gain2_32, 0);
		}
		else if (led == ui.ledGain260) 
		{ 
			ui.slidGain260->setValue(0); 
			//sdsp_params.freq60_params.gain2 = 0; 
			sdsp_params.set(SDSP_Params::Gain2_60, 0);
		}
		return;
	}
		
	if (led == ui.ledGain232)
	{
		ui.slidGain232->setValue(val);
		//sdsp_params.freq32_params.gain2 = val;
		sdsp_params.set(SDSP_Params::Gain2_32, val);
	}
	else if (led == ui.ledGain260)
	{
		ui.slidGain260->setValue(val);
		//sdsp_params.freq60_params.gain2 = val;
		sdsp_params.set(SDSP_Params::Gain2_60, val);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

void SDSPWidget::setPower(QString text)
{
	QLineEdit *led = (QLineEdit*)sender();

	bool ok;
	int val = text.toInt(&ok);	
	if (!ok) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Power value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		if (led == ui.ledPower32) 
		{ 
			ui.slidPower32->setValue(0); 
			//sdsp_params.freq32_params.power = 0; 
			sdsp_params.set(SDSP_Params::Power_32, 0);
		}
		else if (led == ui.ledPower60) 
		{ 
			ui.slidPower60->setValue(0); 
			//sdsp_params.freq60_params.power = 0; 
			sdsp_params.set(SDSP_Params::Power_60, 0);
		}
		return;
	}
	if (val < 0 || val > 64000) 	
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Power value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		if (led == ui.ledPower32) 
		{ 
			ui.slidPower32->setValue(0); 
			//sdsp_params.freq32_params.power = 0; 
			sdsp_params.set(SDSP_Params::Power_32, 0);
		}
		else if (led == ui.ledPower60) 
		{ 
			ui.slidPower60->setValue(0); 
			//sdsp_params.freq60_params.power = 0; 
			sdsp_params.set(SDSP_Params::Power_60, 0);
		}
		return;
	}
		
	val = int(pow(10, val/32000.0));
	if (led == ui.ledPower32)
	{
		ui.slidPower32->setValue(val);
		//sdsp_params.freq32_params.power = val;
		sdsp_params.set(SDSP_Params::Power_32, val);
	}
	else if (led == ui.ledPower60)
	{
		ui.slidPower60->setValue(val);
		//sdsp_params.freq60_params.power = val;
		sdsp_params.set(SDSP_Params::Power_60, val);
	}

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}

/*
void SDSPWidget::setRepetitions(QString text)
{
	bool ok;
	int val = text.toInt(&ok);
	if (!ok) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Repetitions value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		ui.slidRepetitions->setValue(1);
		//sdsp_params.repetitions = 1; 
		sdsp_params.set(SDSP_Params::Repetitions, 1);
		return;
	}
	if (val < 1 || val > 100) 
	{
		int ret = QMessageBox::warning(this, tr("Warning!"), tr("Incorrect Repetitions value was entered!"), QMessageBox::Ok, QMessageBox::Ok);
		ui.slidRepetitions->setValue(1);
		//sdsp_params.repetitions = 1; 
		sdsp_params.set(SDSP_Params::Repetitions, 1);
		return;
	}

	ui.slidRepetitions->setValue(val);
	//sdsp_params.repetitions = val;
	sdsp_params.set(SDSP_Params::Repetitions, val);

	if (!is_enabled) emit place_to_statusbar(tr("<font color=red>NMR Tool is not ready to apply SDSP parameters!</font>"));
}
*/

void SDSPWidget::addSDSPCurve(QList<SDSP_Data*> &sdsp_data_list, QString name, SDSPCurve_Settings curve_settings)
{	
	SDSP_Data *sdsp_data = new SDSP_Data;
	
	QVector<double> *x_data = new QVector<double>;
	QVector<double> *y_data = new QVector<double>;
	QVector<QDateTime> *dt_data = new QVector<QDateTime>;

	sdsp_data->dtime = x_data;
	sdsp_data->time = dt_data;
	sdsp_data->data = y_data;

	QwtPlotCurve *sdsp_curve = new QwtPlotCurve(name); 
	sdsp_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
	sdsp_curve->setStyle(QwtPlotCurve::Lines);

	QwtSymbol *sym0 = new QwtSymbol;
	QwtSymbol::Style sym_style = curve_settings.sym_style;
	sym0->setStyle(sym_style);
	sym0->setSize(curve_settings.sym_size);	
	QPen sym_pen0 = sym0->pen();
	sym_pen0.setColor(curve_settings.sym_border_color);
	sym_pen0.setWidth(curve_settings.sym_border_width);
	sym0->setPen(sym_pen0);
	QBrush sym_brush0 = sym0->brush();
	sym_brush0.setColor(curve_settings.sym_color);
	sym_brush0.setStyle(Qt::SolidPattern);
	sym0->setBrush(sym_brush0);
	sdsp_curve->setSymbol(sym0);
	QPen pen0;
	pen0.setColor(curve_settings.pen_color);
	pen0.setWidth(curve_settings.pen_width);
	sdsp_curve->setPen(pen0);

	sdsp_data->curve = sdsp_curve;

	sdsp_data_list.append(sdsp_data);
}


void SDSPWidget::zoomIn()
{
	QToolButton *tbt = (QToolButton*)sender();
	if (!tbt) return;

	QwtPlot *qwtPlot = NULL;
	if (tbt == ui.tbtZoomInPhase) qwtPlot = ui.qwtPlotPhase;
	else if (tbt == ui.tbtZoomInAmpl) qwtPlot = ui.qwtPlotAmplitude;	
	if (qwtPlot == NULL) return;

	double cur_min = qwtPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
	double cur_max = qwtPlot->axisScaleDiv(QwtPlot::xBottom).upperBound();
	double delta = cur_max - cur_min;
	delta *= 0.80;

	double new_cur_max = (cur_max+cur_min)/2 + delta/2;
	double new_cur_min = (cur_max+cur_min)/2 - delta/2;

	qwtPlot->setAxisScale(QwtPlot::xBottom, new_cur_min, new_cur_max);
	qwtPlot->setAxisScale(QwtPlot::xTop, new_cur_min, new_cur_max);
	qwtPlot->replot();	
}

void SDSPWidget::zoomOut()
{
	QToolButton *tbt = (QToolButton*)sender();
	if (!tbt) return;

	QwtPlot *qwtPlot = NULL;
	if (tbt == ui.tbtZoomOutPhase) qwtPlot = ui.qwtPlotPhase;
	else if (tbt == ui.tbtZoomOutAmpl) qwtPlot = ui.qwtPlotAmplitude;
	if (qwtPlot == NULL) return;

	double cur_min = qwtPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
	double cur_max = qwtPlot->axisScaleDiv(QwtPlot::xBottom).upperBound();
	double delta = cur_max - cur_min;
	delta *= 1.25;

	double new_cur_max = (cur_max+cur_min)/2 + delta/2;
	double new_cur_min = (cur_max+cur_min)/2 - delta/2;

	qwtPlot->setAxisScale(QwtPlot::xBottom, new_cur_min, new_cur_max);
	qwtPlot->setAxisScale(QwtPlot::xTop, new_cur_min, new_cur_max);
	qwtPlot->replot();
}


void SDSPWidget::adjustMinMax()
{
	QToolButton *tbt = (QToolButton*)sender();
	if (!tbt) return;

	QwtPlot *qwtPlot = NULL;	
	SDSP_PlotSettings *plot_settings = NULL;
	if (tbt == ui.tbtZoomPhase) 
	{
		qwtPlot = ui.qwtPlotPhase;		
		plot_settings = &phase_plot_settings;
	}
	else if (tbt == ui.tbtZoomAmpl)
	{
		qwtPlot = ui.qwtPlotAmplitude;		
		plot_settings = &ampl_plot_settings;
	}
	
	if (qwtPlot == NULL) return;
		
	double cur_min = plot_settings->min_zoom;
	double cur_max = plot_settings->max_zoom;

	qwtPlot->setAxisScale(QwtPlot::xBottom, cur_min, cur_max);
	qwtPlot->setAxisScale(QwtPlot::xTop, cur_min, cur_max);
	qwtPlot->replot();
}

void SDSPWidget::changeTimeRange(double range)
{
	QDoubleSpinBox *dsbx = qobject_cast<QDoubleSpinBox*>(sender());
	if (!dsbx) return;

	ticks = base_date_time.secsTo(QDateTime::currentDateTime());

	if (dsbx == ui.dsbxTimePhase)
	{		
		phase_plot_settings.time_scale.second = range;
		//ui.qwtPlotPhase->setAxisScale(QwtPlot::yLeft, ticks - range*60*0.8, ticks + range*60*0.2);
		ui.qwtPlotPhase->setAxisScale(QwtPlot::yLeft, ticks - range*0.7, ticks + range*0.3);
		ui.qwtPlotPhase->replot();

		ui.dsbxTimeAmpl->setValue(range);
	}
	else if (dsbx == ui.dsbxTimeAmpl)
	{		
		ampl_plot_settings.time_scale.second = range;
		//ui.qwtPlotAmplitude->setAxisScale(QwtPlot::yLeft, ticks - range*60*0.8, ticks + range*60*0.2);
		ui.qwtPlotAmplitude->setAxisScale(QwtPlot::yLeft, ticks - range*0.7, ticks + range*0.3);
		ui.qwtPlotAmplitude->replot();

		ui.dsbxTimePhase->setValue(range);
	}	
}

void SDSPWidget::newTick()
{
	if (!is_started) return;

	//ticks++;
	ticks = base_date_time.secsTo(QDateTime::currentDateTime());
		
	double time_range_phase = phase_plot_settings.time_scale.second;
	//ui.qwtPlotPhase->setAxisScale(QwtPlot::yLeft, ticks - time_range_phase*60*0.8, ticks + time_range_phase*60*0.2);
	ui.qwtPlotPhase->setAxisScale(QwtPlot::yLeft, ticks - time_range_phase*0.7, ticks + time_range_phase*0.3);
	ui.qwtPlotPhase->replot();

	double time_range_ampl = ampl_plot_settings.time_scale.second;
	//ui.qwtPlotAmplitude->setAxisScale(QwtPlot::yLeft, ticks - time_range_phase*60*0.8, ticks + time_range_ampl*60*0.2);
	ui.qwtPlotAmplitude->setAxisScale(QwtPlot::yLeft, ticks - time_range_phase*0.7, ticks + time_range_ampl*0.3);
	ui.qwtPlotAmplitude->replot();
}

void SDSPWidget::checkAllParamStates()
{
	if (!is_enabled) return;
	if (is_waiting) return;

	QVector<int> sdsp_data;
	sdsp_data.reserve(200);

	if (sdsp_params.DAC.second == t_true)
	{
		// -- установка кода ЦАП --
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.DAC.first << 2) | 3);
		sdsp_data.append(0x04);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);

		sdsp_params.DAC.second = t_unknown;
		// ------------------------
	}

	if (sdsp_params.freq.second == t_true)
	{
		// -- установка кода ЦАП -- 
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.DAC.first << 2) | 3);
		sdsp_data.append(0x04);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);

		sdsp_params.DAC.second = t_unknown;
		// ------------------------

		if (sdsp_params.freq.first == 32)
		{
			// -- установка мощности -- 
			sdsp_data.append(0x06);
			sdsp_data.append(0x04);
			sdsp_data.append(0x00);
			sdsp_data.append(0x08);
			sdsp_data.append(sdsp_params.freq32_params.power.first & 0x00FF);
			sdsp_data.append((sdsp_params.freq32_params.power.first & 0xFF00) >> 8);			
			// ------------------------

			// -- установка усилений --
			sdsp_data.append(0x06);
			sdsp_data.append(0x00);
			sdsp_data.append(0x10);
			sdsp_data.append(0x08);
			sdsp_data.append((sdsp_params.freq32_params.gain2.first << 4) | sdsp_params.freq32_params.gain1.first);
			sdsp_data.append(0x01);
			sdsp_data.append(0x06);
			sdsp_data.append(0x01);
			sdsp_data.append(0x00);
			sdsp_data.append(0x08);
			sdsp_data.append(0xFF);
			sdsp_data.append(0x00);			
			// ------------------------

			// -- установка частоты ---
			sdsp_data.append(0x06);
			sdsp_data.append(0x00);
			sdsp_data.append(0x10);
			sdsp_data.append(0x08);
			sdsp_data.append(0xFD);
			sdsp_data.append(0x04);
			sdsp_data.append(0x06);
			sdsp_data.append(0x01);
			sdsp_data.append(0x00);
			sdsp_data.append(0x08);
			sdsp_data.append(0xFF);
			sdsp_data.append(0x00);			
			// ------------------------

			sdsp_params.freq32_params.power.second = t_unknown;
			sdsp_params.freq32_params.gain1.second = t_unknown;
			sdsp_params.freq32_params.gain2.second = t_unknown;
			sdsp_params.freq.second = t_unknown;
		}
		else
		{
			// -- установка мощности -- 
			sdsp_data.append(0x06);
			sdsp_data.append(0x04);
			sdsp_data.append(0x00);
			sdsp_data.append(0x08);
			sdsp_data.append(sdsp_params.freq60_params.power.first & 0x00FF);
			sdsp_data.append((sdsp_params.freq60_params.power.first & 0xFF00) >> 8);			
			// ------------------------

			// -- установка усилений --
			sdsp_data.append(0x06);
			sdsp_data.append(0x00);
			sdsp_data.append(0x10);
			sdsp_data.append(0x08);
			sdsp_data.append((sdsp_params.freq60_params.gain2.first << 4) | sdsp_params.freq60_params.gain1.first);
			sdsp_data.append(0x01);
			sdsp_data.append(0x06);
			sdsp_data.append(0x01);
			sdsp_data.append(0x00);
			sdsp_data.append(0x08);
			sdsp_data.append(0xFF);
			sdsp_data.append(0x00);			
			// ------------------------

			// -- установка частоты ---
			sdsp_data.append(0x06);
			sdsp_data.append(0x00);
			sdsp_data.append(0x10);
			sdsp_data.append(0x08);
			sdsp_data.append(0x01);
			sdsp_data.append(0x04);
			sdsp_data.append(0x06);
			sdsp_data.append(0x01);
			sdsp_data.append(0x00);
			sdsp_data.append(0x08);
			sdsp_data.append(0xFF);
			sdsp_data.append(0x00);			
			// ------------------------

			sdsp_params.freq60_params.power.second = t_unknown;
			sdsp_params.freq60_params.gain1.second = t_unknown;
			sdsp_params.freq60_params.gain2.second = t_unknown;
			sdsp_params.freq.second = t_unknown;
		}		
	}

	if (sdsp_params.freq32_params.power.second == t_true)
	{
		// -- установка мощности -- 
		sdsp_data.append(0x06);
		sdsp_data.append(0x04);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(sdsp_params.freq32_params.power.first & 0x00FF);
		sdsp_data.append((sdsp_params.freq32_params.power.first & 0xFF00) >> 8);

		sdsp_params.freq32_params.power.second = t_unknown;
		// ------------------------		
	}

	if (sdsp_params.freq60_params.power.second == t_true)
	{
		// -- установка мощности -- 
		sdsp_data.append(0x06);
		sdsp_data.append(0x04);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(sdsp_params.freq60_params.power.first & 0x00FF);
		sdsp_data.append((sdsp_params.freq60_params.power.first & 0xFF00) >> 8);	

		sdsp_params.freq60_params.power.second = t_unknown;
		// ------------------------		
	}

	if (sdsp_params.freq32_params.gain1.second == t_true || sdsp_params.freq32_params.gain2.second == t_true)
	{
		// -- установка усилений --
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.freq32_params.gain2.first << 4) | sdsp_params.freq32_params.gain1.first);
		sdsp_data.append(0x01);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);

		sdsp_params.freq32_params.gain1.second = t_unknown;
		sdsp_params.freq32_params.gain2.second = t_unknown;
		// ------------------------
	}

	if (sdsp_params.freq60_params.gain1.second == t_true || sdsp_params.freq60_params.gain2.second == t_true)
	{
		// -- установка усилений --
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.freq60_params.gain2.first << 4) | sdsp_params.freq60_params.gain1.first);
		sdsp_data.append(0x01);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);			

		sdsp_params.freq60_params.gain1.second = t_unknown;
		sdsp_params.freq60_params.gain2.second = t_unknown;
		// ------------------------
	}

	if (sdsp_params.transmitter.second == t_true)
	{
		// -- установка передатчика
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append(sdsp_params.transmitter.first == true ? 0xFE : 0x02);
		sdsp_data.append(0x04);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);

		sdsp_params.transmitter.second = t_unknown;
		// ------------------------
	}

	if (sdsp_params.channel1.second == t_true || sdsp_params.channel2.second == t_true || sdsp_params.power_status.second == t_true)
	{ 
		// -- установка состояния питания и каналов
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.power_status.first << 2) | (sdsp_params.channel2.first << 1) | sdsp_params.channel1.first);
		sdsp_data.append(0x02);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);

		sdsp_params.power_status.second = t_unknown;
		sdsp_params.channel1.second = t_unknown;
		sdsp_params.channel2.second = t_unknown;
		// ------------------------
	}

	if (!sdsp_data.isEmpty())
	{
		emit apply_sdsp_params(sdsp_data);
	}
}

/*
void SDSPWidget::addData(QVector<double> *_x_data, QVector<double> *_y_data)
{
	bool temp_ok = true;

	base_date_time = QDateTime::currentDateTime();
	uint bs = base_date_time.toTime_t();		

	QVector<QDateTime> *dt0 = phaseData[0]->time;
	QVector<double> *dtime0 = phaseData[0]->dtime;
	QVector<double> *temp0 = phaseData[0]->data;
	int data_size = _y_data->size();
	for (int j = 0; j < data_size; j += 4)
	{
		QDateTime _dt0 = QDateTime::fromTime_t((uint)_x_data->at(j) + bs);			
		dt0->push_back(_dt0);
		dtime0->push_back(_x_data->at(j)+bs);
		int y = (int)(_y_data->at(j)) | ((int)(_y_data->at(j+1)) << 8);
		temp0->push_back(y);
	}

	QwtPlotCurve *phase_curve = phaseData[0]->curve;
	//phase_curve->setSamples(dtime0->data(), temp0->data(), temp0->size());	
	phase_curve->setSamples(temp0->data(), dtime0->data(), dtime0->size());	
	phase_curve->attach(ui.qwtPlotPhase);

	double from_time_phase = dtime0->first();
	double to_time_phase = dtime0->last();		
	double time_range_phase = phase_plot_settings.time_scale.second;
	bool floating_timescale_phase = phase_plot_settings.time_scale.first;
	if (floating_timescale_phase) 
	{
		from_time_phase = dtime0->last() - time_range_phase*0.75*60;		// 60 seconds in 1 minute
		to_time_phase = dtime0->last() + time_range_phase*0.25*60;			// 60 seconds in 1 minute		
	}						
	ui.qwtPlotPhase->setAxisScale(QwtPlot::xBottom, from_time_phase, to_time_phase);
	ui.qwtPlotPhase->setAxisScale(QwtPlot::xTop, from_time_phase, to_time_phase);	
	ui.qwtPlotPhase->replot();

	ui.lbl_PhaseDiff->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lbl_PhaseDiff->setText(QString("<b>%1 <sup>0</sup></b>").arg(QString::number(temp0->last(), 'f', 3)));	


	QVector<QDateTime> *dt1 = amplData[0]->time;
	QVector<double> *dtime1 = amplData[0]->dtime;
	QVector<double> *temp1 = amplData[0]->data;

	for (int j = 2; j < data_size; j += 4)
	{
		QDateTime _dt1 = QDateTime::fromTime_t((uint)_x_data->at(j) + bs);			
		dt1->push_back(_dt1);
		dtime1->push_back(_x_data->at(j)+bs);
		int y = (int)(_y_data->at(j)) | ((int)(_y_data->at(j+1)) < 8);
		temp1->push_back(y);
	}

	QwtPlotCurve *ampl_curve = amplData[0]->curve;
	//ampl_curve->setSamples(dtime1->data(), temp1->data(), temp1->size());
	ampl_curve->setSamples(temp1->data(), dtime1->data(), dtime1->size());
	ampl_curve->attach(ui.qwtPlotAmplitude);

	double from_time_ampl = dtime1->first();
	double to_time_ampl = dtime1->last();	
	double time_range_ampl = ampl_plot_settings.time_scale.second;
	bool floating_timescale_ampl = ampl_plot_settings.time_scale.first;
	if (floating_timescale_ampl) 
	{
		from_time_ampl = dtime1->last() - time_range_ampl*0.75*60;		// 60 seconds in 1 minute
		to_time_ampl = dtime1->last() + time_range_ampl*0.25*60;			// 60 seconds in 1 minute
	}						
	ui.qwtPlotAmplitude->setAxisScale(QwtPlot::xBottom, from_time_ampl, to_time_ampl);			
	ui.qwtPlotAmplitude->replot();

	ui.lbl_RatioAmpl->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lbl_RatioAmpl->setText(QString("<b>%1</b>").arg(QString::number(temp1->last(), 'f', 3)));		
}
*/

void SDSPWidget::addData(QVector<double> *_x_data, QVector<double> *_y_data)
{
	is_started = true;

	bool temp_ok = true;

	uint bs = QDateTime::currentDateTime().toTime_t() - base_date_time.toTime_t();	
	
	QVector<QDateTime> *dt0 = phaseData[0]->time;
	QVector<double> *dtime0 = phaseData[0]->dtime;
	QVector<double> *temp0 = phaseData[0]->data;
	int data_size = _y_data->size();
	for (int j = 0; j < data_size; j += 4)
	{
		QDateTime _dt0 = QDateTime::fromTime_t((uint)_x_data->at(j) + bs);			
		dt0->push_back(_dt0);
		dtime0->push_back(_x_data->at(j)+bs);
		int y = (int)(_y_data->at(j)) | ((int)(_y_data->at(j+1)) << 8);

		float maxV = 2.5f;	// максимальное напряжение
		int maxP = 2048;	// максимальное значение АЦП
		double V1;			// напряжение

		V1 = (y & 0x7ff) * maxV / maxP;

		//double koef = 1.0;
		//double dfi = (V1*100 + 0.5)*koef;
		double k_deg_mV = 180/1.8;		
		double dfi = V1 * k_deg_mV * Kcalibr_dfi;

		temp0->push_back(dfi);
	}

	QwtPlotCurve *phase_curve = phaseData[0]->curve;
	phase_curve->setSamples(temp0->data(), dtime0->data(), dtime0->size());	
	phase_curve->attach(ui.qwtPlotPhase);	
	ui.qwtPlotPhase->replot(); 

	ui.lbl_PhaseDiff->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lbl_PhaseDiff->setText(QString("<b>%1<sup>0</sup></b>").arg(QString::number(temp0->last(), 'f', 2)));	


	QVector<QDateTime> *dt1 = amplData[0]->time;
	QVector<double> *dtime1 = amplData[0]->dtime;
	QVector<double> *temp1 = amplData[0]->data;

	for (int j = 2; j < data_size; j += 4)
	{
		QDateTime _dt1 = QDateTime::fromTime_t((uint)_x_data->at(j) + bs);			
		dt1->push_back(_dt1);
		dtime1->push_back(_x_data->at(j)+bs);
		int y = (int)(_y_data->at(j)) | ((int)(_y_data->at(j+1)) < 8);

		float maxV = 2.5f;	// максимальное напряжение
		int maxP = 2048;	// максимальное значение АЦП
		double V2;			// напряжение

		V2 = (y & 0x7ff) * maxV / maxP;
				
		//double ramp = (V2 * 40 - 38.2);
		double k_dB_V = 60/1.8;
		double V_0dB = 0.9; // V
		double ramp = (V2 - V_0dB) * k_dB_V * Kcalibr_ramp;
		
		temp1->push_back(ramp);
	}

	QwtPlotCurve *ampl_curve = amplData[0]->curve;
	ampl_curve->setSamples(temp1->data(), dtime1->data(), dtime1->size());
	ampl_curve->attach(ui.qwtPlotAmplitude);
	
	ui.qwtPlotAmplitude->replot();

	ui.lbl_RatioAmpl->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
	ui.lbl_RatioAmpl->setText(QString("<b>%1</b>").arg(QString::number(temp1->last(), 'f', 2)));
}


void SDSPWidget::applyParameters()
{	
	if (!is_enabled) 
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Cannot apply SDSP parameters! Please, check:\n1. Connection to NMR Tool.\n2. NMR Tool sequence has been stopped."), QMessageBox::Ok);
		return;
	}

	if (is_waiting)
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Cannot apply SDSP parameters because previous parameters have not applied yet!"), QMessageBox::Ok);
		return;
	}

	QVector<int> sdsp_data;
	sdsp_data.reserve(200);

	// -- установка кода ЦАП --
	sdsp_data.append(0x06);
	sdsp_data.append(0x00);
	sdsp_data.append(0x10);
	sdsp_data.append(0x08);
	sdsp_data.append((sdsp_params.DAC.first << 2) | 3);
	sdsp_data.append(0x04);
	sdsp_data.append(0x06);
	sdsp_data.append(0x01);
	sdsp_data.append(0x00);
	sdsp_data.append(0x08);
	sdsp_data.append(0xFF);
	sdsp_data.append(0x00);

	sdsp_params.DAC.second = t_unknown;
	// ------------------------
	
	if (sdsp_params.freq.first == 32)
	{
		// -- установка мощности -- 
		sdsp_data.append(0x06);
		sdsp_data.append(0x04);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(sdsp_params.freq32_params.power.first & 0x00FF);
		sdsp_data.append((sdsp_params.freq32_params.power.first & 0xFF00) >> 8);			
		// ------------------------

		// -- установка усилений --
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.freq32_params.gain2.first << 4) | sdsp_params.freq32_params.gain1.first);
		sdsp_data.append(0x01);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);			
		// ------------------------

		// -- установка частоты ---
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFD);
		sdsp_data.append(0x04);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);			
		// ------------------------

		sdsp_params.freq32_params.power.second = t_unknown;
		sdsp_params.freq32_params.gain1.second = t_unknown;
		sdsp_params.freq32_params.gain2.second = t_unknown;
		sdsp_params.freq.second = t_unknown;
	}
	else
	{
		// -- установка мощности -- 
		sdsp_data.append(0x06);
		sdsp_data.append(0x04);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(sdsp_params.freq60_params.power.first & 0x00FF);
		sdsp_data.append((sdsp_params.freq60_params.power.first & 0xFF00) >> 8);			
		// ------------------------

		// -- установка усилений --
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append((sdsp_params.freq60_params.gain2.first << 4) | sdsp_params.freq60_params.gain1.first);
		sdsp_data.append(0x01);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);			
		// ------------------------

		// -- установка частоты ---
		sdsp_data.append(0x06);
		sdsp_data.append(0x00);
		sdsp_data.append(0x10);
		sdsp_data.append(0x08);
		sdsp_data.append(0x01);
		sdsp_data.append(0x04);
		sdsp_data.append(0x06);
		sdsp_data.append(0x01);
		sdsp_data.append(0x00);
		sdsp_data.append(0x08);
		sdsp_data.append(0xFF);
		sdsp_data.append(0x00);			
		// ------------------------

		sdsp_params.freq60_params.power.second = t_unknown;
		sdsp_params.freq60_params.gain1.second = t_unknown;
		sdsp_params.freq60_params.gain2.second = t_unknown;
		sdsp_params.freq.second = t_unknown;
	}		
	
	// -- установка передатчика
	sdsp_data.append(0x06);
	sdsp_data.append(0x00);
	sdsp_data.append(0x10);
	sdsp_data.append(0x08);
	sdsp_data.append(sdsp_params.transmitter.first == true ? 0xFE : 0x02);
	sdsp_data.append(0x04);
	sdsp_data.append(0x06);
	sdsp_data.append(0x01);
	sdsp_data.append(0x00);
	sdsp_data.append(0x08);
	sdsp_data.append(0xFF);
	sdsp_data.append(0x00);

	sdsp_params.transmitter.second = t_unknown;
	// ------------------------
	
	// -- установка состояния питания и каналов
	sdsp_data.append(0x06);
	sdsp_data.append(0x00);
	sdsp_data.append(0x10);
	sdsp_data.append(0x08);
	sdsp_data.append((sdsp_params.power_status.first << 2) | (sdsp_params.channel2.first << 1) | sdsp_params.channel1.first);
	sdsp_data.append(0x02);
	sdsp_data.append(0x06);
	sdsp_data.append(0x01);
	sdsp_data.append(0x00);
	sdsp_data.append(0x08);
	sdsp_data.append(0xFF);
	sdsp_data.append(0x00);

	sdsp_params.power_status.second = t_unknown;
	sdsp_params.channel1.second = t_unknown;
	sdsp_params.channel2.second = t_unknown;
	// ------------------------
	
	emit apply_sdsp_params(sdsp_data);
}


void SDSPWidget::initAllSettings()
{
	phase_plot_settings.min_zoom = 0;
	phase_plot_settings.max_zoom = 180;
	ampl_plot_settings.min_zoom = -30;
	ampl_plot_settings.max_zoom = 30;	
	phase_plot_settings.time_scale = QPair<bool,double>(false, SDSP_TIME_RANGE);
	ampl_plot_settings.time_scale = QPair<bool,double>(false, SDSP_TIME_RANGE);

	bool ok = false;
	double time_range = SDSP_TIME_RANGE;	
	if (app_settings->contains("SDSP/Time_Range")) time_range = app_settings->value("SDSP/Time_Range").toDouble(&ok);		
	else app_settings->setValue("SDSP/Time_Range", QVariant(time_range));
	if (ok) 
	{ 
		phase_plot_settings.time_scale = QPair<bool,double>(false, time_range); 
		ampl_plot_settings.time_scale = QPair<bool,double>(false, time_range); 
	}
		
	double min_phase = 0;
	double max_phase = 180;
	double min_ampl = -30;
	double max_ampl = 30;
	if (app_settings->contains("SDSP/PhaseDifference_Min")) min_phase = app_settings->value("SDSP/PhaseDifference_Min").toDouble(&ok);		
	else app_settings->setValue("SDSP/PhaseDifference_Min", QVariant(min_phase)); 
	if (ok)	phase_plot_settings.min_zoom = min_phase; 	
	if (app_settings->contains("SDSP/PhaseDifference_Max")) max_phase = app_settings->value("SDSP/PhaseDifference_Max").toDouble(&ok);		
	else app_settings->setValue("SDSP/PhaseDifference_Max", QVariant(max_phase));
	if (ok) phase_plot_settings.max_zoom = max_phase;	
	if (app_settings->contains("SDSP/AmplitudeRatio_Min")) min_ampl = app_settings->value("SDSP/AmplitudeRatio_Min").toDouble(&ok);		
	else app_settings->setValue("SDSP/AmplitudeRatio_Min", QVariant(min_ampl));
	if (ok) ampl_plot_settings.min_zoom = min_ampl;
	if (app_settings->contains("SDSP/AmplitudeRatio_Max")) max_ampl = app_settings->value("SDSP/AmplitudeRatio_Max").toDouble(&ok);		
	else app_settings->setValue("SDSP/AmplitudeRatio_Max", QVariant(max_ampl));
	if (ok) ampl_plot_settings.max_zoom = max_ampl;
	
	sdsp_params.set(SDSP_Params::Freq, 32);
	sdsp_params.set(SDSP_Params::DAC_Code, 0);
	sdsp_params.set(SDSP_Params::Power_32, 1);
	sdsp_params.set(SDSP_Params::Gain1_32, 0);
	sdsp_params.set(SDSP_Params::Gain2_32, 0);
	sdsp_params.set(SDSP_Params::Power_60, 1);
	sdsp_params.set(SDSP_Params::Gain1_60, 0);
	sdsp_params.set(SDSP_Params::Gain2_60, 0);
	sdsp_params.set(SDSP_Params::Power_Status, true);
	sdsp_params.set(SDSP_Params::Channel_1, true);
	sdsp_params.set(SDSP_Params::Channel_2, true);
	sdsp_params.set(SDSP_Params::Transmitter, true);
	sdsp_params.set(SDSP_Params::Repetitions, 1);
	sdsp_params.set(SDSP_Params::Average, false);

	int freq = 32;
	if (app_settings->contains("SDSP/Frequency")) sdsp_params.set(SDSP_Params::Freq, app_settings->value("SDSP/Frequency").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency", QVariant(freq));
	if (!ok) sdsp_params.set(SDSP_Params::Freq, freq);

	int DAC = 0;
	if (app_settings->contains("SDSP/DAC_Code")) sdsp_params.set(SDSP_Params::DAC_Code, app_settings->value("SDSP/DAC_Code").toInt(&ok));		
	else app_settings->setValue("SDSP/DAC_Code", QVariant(DAC));
	if (!ok) sdsp_params.set(SDSP_Params::DAC_Code, DAC);

	int freq32_power = 1;
	if (app_settings->contains("SDSP/Frequency32_Power")) sdsp_params.set(SDSP_Params::Power_32, app_settings->value("SDSP/Frequency32_Power").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency32_Power", QVariant(freq32_power));
	if (!ok) sdsp_params.set(SDSP_Params::Power_32, freq32_power);

	int freq32_gain1 = 0;
	if (app_settings->contains("SDSP/Frequency32_Gain1")) sdsp_params.set(SDSP_Params::Gain1_32, app_settings->value("SDSP/Frequency32_Gain1").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency32_Gain1", QVariant(freq32_gain1));
	if (!ok) sdsp_params.set(SDSP_Params::Gain1_32, freq32_gain1); 

	int freq32_gain2 = 0;
	if (app_settings->contains("SDSP/Frequency32_Gain2")) sdsp_params.set(SDSP_Params::Gain2_32, app_settings->value("SDSP/Frequency32_Gain2").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency32_Gain2", QVariant(freq32_gain2));
	if (!ok) sdsp_params.set(SDSP_Params::Gain2_32, freq32_gain2);

	int freq60_power = 1;
	if (app_settings->contains("SDSP/Frequency60_Power")) sdsp_params.set(SDSP_Params::Power_60, app_settings->value("SDSP/Frequency60_Power").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency60_Power", QVariant(freq60_power));
	if (!ok) sdsp_params.set(SDSP_Params::Power_60, freq60_power);

	int freq60_gain1 = 0;
	if (app_settings->contains("SDSP/Frequency60_Gain1")) sdsp_params.set(SDSP_Params::Gain1_60, app_settings->value("SDSP/Frequency60_Gain1").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency60_Gain1", QVariant(freq60_gain1));
	if (!ok) sdsp_params.set(SDSP_Params::Gain1_60, freq60_gain1); 

	int freq60_gain2 = 0;
	if (app_settings->contains("SDSP/Frequency60_Gain2")) sdsp_params.set(SDSP_Params::Gain2_60, app_settings->value("SDSP/Frequency60_Gain2").toInt(&ok));		
	else app_settings->setValue("SDSP/Frequency60_Gain2", QVariant(freq60_gain2));
	if (!ok) sdsp_params.set(SDSP_Params::Gain2_60, freq60_gain2);
		
	bool power_status = true;
	if (app_settings->contains("SDSP/Power_State")) sdsp_params.set(SDSP_Params::Power_Status, app_settings->value("SDSP/Power_State").toBool());		
	else app_settings->setValue("SDSP/Power_State", QVariant(power_status));

	bool channel1 = true;
	if (app_settings->contains("SDSP/Channel1")) sdsp_params.set(SDSP_Params::Channel_1, app_settings->value("SDSP/Channel1").toBool());		
	else app_settings->setValue("SDSP/Channel1", QVariant(channel1));

	bool channel2 = true;
	if (app_settings->contains("SDSP/Channel2")) sdsp_params.set(SDSP_Params::Channel_2, app_settings->value("SDSP/Channel2").toBool());		
	else app_settings->setValue("SDSP/Channel2", QVariant(channel2));

	bool transmitter = true;
	if (app_settings->contains("SDSP/Transmitter")) sdsp_params.set(SDSP_Params::Transmitter, app_settings->value("SDSP/Transmitter").toBool());		
	else app_settings->setValue("SDSP/Transmitter", QVariant(transmitter));

	int repetitions = 1;
	if (app_settings->contains("SDSP/Repetitions")) sdsp_params.set(SDSP_Params::Repetitions, app_settings->value("SDSP/Repetitions").toInt(&ok));		
	else app_settings->setValue("SDSP/Repetitions", QVariant(repetitions));
	if (!ok) sdsp_params.set(SDSP_Params::Repetitions, repetitions); 

	bool average = false;
	if (app_settings->contains("SDSP/Average")) sdsp_params.set(SDSP_Params::Average, app_settings->value("SDSP/Average").toBool());		
	else app_settings->setValue("SDSP/Average", QVariant(average));

	Kcalibr_dfi = 1.0;
	if (app_settings->contains("SDSP/Kcalibr_dFi")) Kcalibr_dfi = app_settings->value("SDSP/Kcalibr_dFi").toDouble(&ok);		
	else app_settings->setValue("SDSP/Kcalibr_dFi", QVariant(Kcalibr_dfi));
	if (!ok) Kcalibr_dfi = 1.0; 

	Kcalibr_ramp = 1.0;
	if (app_settings->contains("SDSP/Kcalibr_rAmp")) Kcalibr_ramp = app_settings->value("SDSP/Kcalibr_rAmp").toDouble(&ok);		
	else app_settings->setValue("SDSP/Kcalibr_rAmp", QVariant(Kcalibr_ramp));
	if (!ok) Kcalibr_ramp = 1.0;
}

void SDSPWidget::saveSettings()
{
	app_settings->setValue("SDSP/Time_Range", QVariant(phase_plot_settings.time_scale.second));
	app_settings->setValue("SDSP/PhaseDifference_Min", QVariant(phase_plot_settings.min_zoom)); 
	app_settings->setValue("SDSP/PhaseDifference_Max", QVariant(phase_plot_settings.max_zoom)); 
	app_settings->setValue("SDSP/AmplitudeRatio_Min", QVariant(ampl_plot_settings.min_zoom)); 
	app_settings->setValue("SDSP/AmplitudeRatio_Max", QVariant(ampl_plot_settings.max_zoom)); 
	app_settings->setValue("SDSP/Frequency", QVariant(sdsp_params.freq.first));
	app_settings->setValue("SDSP/DAC_Code", QVariant(sdsp_params.DAC.first));
	app_settings->setValue("SDSP/Frequency32_Power", QVariant(sdsp_params.freq32_params.power.first));
	app_settings->setValue("SDSP/Frequency32_Gain1", QVariant(sdsp_params.freq32_params.gain1.first));
	app_settings->setValue("SDSP/Frequency32_Gain2", QVariant(sdsp_params.freq32_params.gain2.first));
	app_settings->setValue("SDSP/Frequency60_Power", QVariant(sdsp_params.freq60_params.power.first));
	app_settings->setValue("SDSP/Frequency60_Gain1", QVariant(sdsp_params.freq60_params.gain1.first));
	app_settings->setValue("SDSP/Frequency60_Gain2", QVariant(sdsp_params.freq60_params.gain2.first));	
	app_settings->setValue("SDSP/Power_State", QVariant(sdsp_params.power_status.first));
	app_settings->setValue("SDSP/Channel1", QVariant(sdsp_params.channel1.first));
	app_settings->setValue("SDSP/Channel2", QVariant(sdsp_params.channel2.first));
	app_settings->setValue("SDSP/Transmitter", QVariant(sdsp_params.transmitter.first));
	app_settings->setValue("SDSP/Repetitions", QVariant(sdsp_params.repetitions.first));
	app_settings->setValue("SDSP/Average", QVariant(sdsp_params.average.first));
	app_settings->setValue("SDSP/Kcalibr_dFi", QVariant(Kcalibr_dfi));
	app_settings->setValue("SDSP/Kcalibr_rAmp", QVariant(Kcalibr_ramp));
}

void SDSPWidget::resetSDSPParameters()
{
	if (sdsp_params.DAC.second == t_unknown) sdsp_params.DAC.second = t_false;
	if (sdsp_params.freq.second == t_unknown) sdsp_params.freq.second = t_false;
	if (sdsp_params.freq32_params.power.second == t_unknown) sdsp_params.freq32_params.power.second = t_false;
	if (sdsp_params.freq32_params.gain1.second == t_unknown) sdsp_params.freq32_params.gain1.second = t_false;
	if (sdsp_params.freq32_params.gain2.second == t_unknown) sdsp_params.freq32_params.gain2.second = t_false;
	if (sdsp_params.freq60_params.power.second == t_unknown) sdsp_params.freq60_params.power.second = t_false;
	if (sdsp_params.freq60_params.gain1.second == t_unknown) sdsp_params.freq60_params.gain1.second = t_false;
	if (sdsp_params.freq60_params.gain2.second == t_unknown) sdsp_params.freq60_params.gain2.second = t_false;
	if (sdsp_params.transmitter.second == t_unknown) sdsp_params.transmitter.second = t_false;
	if (sdsp_params.channel1.second == t_unknown) sdsp_params.channel1.second = t_false;
	if (sdsp_params.channel2.second == t_unknown) sdsp_params.channel2.second = t_false;
	if (sdsp_params.power_status.second == t_unknown) sdsp_params.power_status.second = t_false;
}

