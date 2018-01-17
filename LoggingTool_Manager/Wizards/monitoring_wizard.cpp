#include "../io_general.h"
#include "monitoring_wizard.h"


MonitoringWidget::MonitoringWidget(QSettings *app_settings, QWidget *parent) : QWidget(parent), ui(new Ui::MonitoringWidget)
{
	ui->setupUi(this);
	this->setObjectName("tabMonitoring");

	ui->qwtPlotTUT->setCanvasBackground(QBrush(QColor(Qt::white)));
	ui->qwtPlotDUT->setCanvasBackground(QBrush(QColor(Qt::white)));
	ui->qwtPlotPUT->setCanvasBackground(QBrush(QColor(Qt::white)));

	ToolFrame *frame = new ToolFrame(this);
	frame->setObjectName(QStringLiteral("frame"));
	frame->setMinimumSize(QSize(300, 628));
	frame->setMaximumSize(QSize(300, 628));
	frame->setFrameShape(QFrame::NoFrame);		
	ui->gridLayout_8->addWidget(frame, 0, 1, 1, 1);
	ui->gridLayout_8->setHorizontalSpacing(0);
		
	base_date_time = QDateTime::currentDateTime();

	initMonitoringObjects();

	double min_zoom_DU = (temperatureData[0]->min_zoom < temperatureData[1]->min_zoom ? temperatureData[0]->min_zoom : temperatureData[1]->min_zoom);
	double max_zoom_DU = (temperatureData[0]->max_zoom > temperatureData[1]->max_zoom ? temperatureData[0]->max_zoom : temperatureData[1]->max_zoom);
	double min_zoom_TU = (temperatureData[2]->min_zoom < temperatureData[3]->min_zoom ? temperatureData[2]->min_zoom : temperatureData[3]->min_zoom);
	double max_zoom_TU = (temperatureData[2]->max_zoom > temperatureData[3]->max_zoom ? temperatureData[2]->max_zoom : temperatureData[3]->max_zoom);
	double min_zoom_PU = (temperatureData[4]->min_zoom < temperatureData[5]->min_zoom ? temperatureData[4]->min_zoom : temperatureData[5]->min_zoom);
	double max_zoom_PU = (temperatureData[4]->max_zoom > temperatureData[5]->max_zoom ? temperatureData[4]->max_zoom : temperatureData[5]->max_zoom);

	ui->qwtPlotDUT->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(base_date_time));		
	ui->qwtPlotDUT->setAxisScale(QwtPlot::yLeft, min_zoom_DU, max_zoom_DU);
	ui->qwtPlotDUT->canvas()->setCursor(Qt::ArrowCursor);
	ui->qwtPlotDUT->replot();

	ui->qwtPlotTUT->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(base_date_time));	
	ui->qwtPlotTUT->setAxisScale(QwtPlot::yLeft, min_zoom_TU, max_zoom_TU);
	ui->qwtPlotTUT->canvas()->setCursor(Qt::ArrowCursor);
	ui->qwtPlotTUT->replot();

	ui->qwtPlotPUT->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(base_date_time));	
	ui->qwtPlotPUT->setAxisScale(QwtPlot::yLeft, min_zoom_PU, max_zoom_PU);
	ui->qwtPlotPUT->canvas()->setCursor(Qt::ArrowCursor);
	ui->qwtPlotPUT->replot();

	ui->sbxTimeDU->setValue(TIME_RANGE);
	ui->sbxTimeTU->setValue(TIME_RANGE);
	ui->sbxTimePU->setValue(TIME_RANGE);
	ui->chboxTimeDU->setChecked(true);
	ui->chboxTimeTU->setChecked(true);
	ui->chboxTimePU->setChecked(true);

	QwtPlotGrid *grid_DU = new QwtPlotGrid;
	grid_DU->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_DU->enableX(true);	
	grid_DU->enableY(true);	
	grid_DU->attach(ui->qwtPlotDUT);	

	QwtPlotGrid *grid_TU = new QwtPlotGrid;
	grid_TU->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_TU->enableX(true);	
	grid_TU->enableY(true);	
	grid_TU->attach(ui->qwtPlotTUT);	

	QwtPlotGrid *grid_PU = new QwtPlotGrid;
	grid_PU->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid_PU->enableX(true);	
	grid_PU->enableY(true);	
	grid_PU->attach(ui->qwtPlotPUT);	

	QwtPlotPanner *pannerDU = new QwtPlotPanner(ui->qwtPlotDUT->canvas());
	pannerDU->setAxisEnabled(QwtPlot::yRight, false);
	pannerDU->setMouseButton(Qt::RightButton);

	QwtPlotPanner *pannerTU = new QwtPlotPanner(ui->qwtPlotTUT->canvas());
	pannerTU->setAxisEnabled(QwtPlot::yRight, false);
	pannerTU->setMouseButton(Qt::RightButton);

	QwtPlotPanner *pannerPU = new QwtPlotPanner(ui->qwtPlotPUT->canvas());
	pannerPU->setAxisEnabled(QwtPlot::yRight, false);
	pannerPU->setMouseButton(Qt::RightButton);

	/*QwtPlotPicker *pickerDU = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlotDUT->canvas());	
	pickerDU->setRubberBandPen(QColor(Qt::green));
	pickerDU->setRubberBand(QwtPicker::CrossRubberBand);
	pickerDU->setTrackerPen(QColor(Qt::darkMagenta));

	QwtPlotPicker *pickerTU = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlotTUT->canvas());	
	pickerTU->setRubberBandPen(QColor(Qt::green));
	pickerTU->setRubberBand(QwtPicker::CrossRubberBand);
	pickerTU->setTrackerPen(QColor(Qt::darkMagenta));

	QwtPlotPicker *pickerPU = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlotPUT->canvas());	
	pickerPU->setRubberBandPen(QColor(Qt::green));
	pickerPU->setRubberBand(QwtPicker::CrossRubberBand);
	pickerPU->setTrackerPen(QColor(Qt::darkMagenta));*/

	QwtText yleft_title;
	QString yleft_title_text = QString("Temperature, <sup>0</sup>C");
	yleft_title.setText(yleft_title_text);
	yleft_title.setColor(QColor(Qt::darkBlue));
	QFont yleft_font = ui->qwtPlotDUT->axisTitle(QwtPlot::yLeft).font();
	QFont yleft_axis_font = ui->qwtPlotDUT->axisFont(QwtPlot::yLeft);
	yleft_axis_font.setPointSize(9);
	yleft_axis_font.setBold(false);
	yleft_font.setPointSize(9);
	yleft_font.setBold(false);
	yleft_title.setFont(yleft_font);

	ui->qwtPlotDUT->setAxisFont(QwtPlot::yLeft, yleft_axis_font);	
	ui->qwtPlotDUT->setAxisTitle(QwtPlot::yLeft, yleft_title);
	ui->qwtPlotDUT->setAxisFont(QwtPlot::xBottom, yleft_axis_font);
		
	ui->qwtPlotTUT->setAxisFont(QwtPlot::yLeft, yleft_axis_font);
	ui->qwtPlotTUT->setAxisTitle(QwtPlot::yLeft, yleft_title);
	ui->qwtPlotTUT->setAxisFont(QwtPlot::xBottom, yleft_axis_font);

	ui->qwtPlotPUT->setAxisFont(QwtPlot::yLeft, yleft_axis_font);
	ui->qwtPlotPUT->setAxisTitle(QwtPlot::yLeft, yleft_title);
	ui->qwtPlotPUT->setAxisFont(QwtPlot::xBottom, yleft_axis_font);


	ui->lblDUT1->setStyleSheet("QLabel { color : red; }");
	ui->lblDUT2->setStyleSheet("QLabel { color : blue; }");

	ui->lblTUT1->setStyleSheet("QLabel { color : crimson; }");
	ui->lblTUT2->setStyleSheet("QLabel { color : blue; }");

	ui->lblPUT1->setStyleSheet("QLabel { color : crimson; }");
	ui->lblPUT2->setStyleSheet("QLabel { color : blue; }");

	//ui->lbl_DUT1->setStyleSheet("QLabel { background-color : palegreen; }");	
	//ui->lbl_DUT2->setStyleSheet("QLabel { background-color : palegreen; }");

	//ui->lbl_TUT1->setStyleSheet("QLabel { background-color : palegreen; }");
	//ui->lbl_TUT2->setStyleSheet("QLabel { background-color : palegreen; }");

	//ui->lbl_PUT1->setStyleSheet("QLabel { background-color : palegreen; }");
	//ui->lbl_PUT2->setStyleSheet("QLabel { background-color : palegreen; }");

	ui->lbl_DUT1->setStyleSheet("QLabel { background-color : lightgray; color : black; }");
	ui->lbl_DUT2->setStyleSheet("QLabel { background-color : lightgray; color : black; }");

	ui->lbl_TUT1->setStyleSheet("QLabel { background-color : lightgray; color : black; }");
	ui->lbl_TUT2->setStyleSheet("QLabel { background-color : lightgray; color : black; }");

	ui->lbl_PUT1->setStyleSheet("QLabel { background-color : lightgray; color : black; }");
	ui->lbl_PUT2->setStyleSheet("QLabel { background-color : lightgray; color : black; }");

	
	ui->tbtZoomDU->setIcon(QIcon(":/images/zoom.png"));
	ui->tbtZoomInDU->setIcon(QIcon(":/images/zoom_in.png"));
	ui->tbtZoomOutDU->setIcon(QIcon(":/images/zoom_out.png"));

	ui->tbtZoomTU->setIcon(QIcon(":/images/zoom.png"));
	ui->tbtZoomInTU->setIcon(QIcon(":/images/zoom_in.png"));
	ui->tbtZoomOutTU->setIcon(QIcon(":/images/zoom_out.png"));

	ui->tbtZoomPU->setIcon(QIcon(":/images/zoom.png"));
	ui->tbtZoomInPU->setIcon(QIcon(":/images/zoom_in.png"));
	ui->tbtZoomOutPU->setIcon(QIcon(":/images/zoom_out.png"));			


	QStringList headlist;
	ui->treeWidget->setColumnCount(2);
	headlist << tr("List of sensors") << tr("Values");
	ui->treeWidget->setHeaderLabels(headlist);	
	ui->treeWidget->setColumnWidth(0,350);
	ui->treeWidget->header()->setFont(QFont("Arial", 10, 0, false));
	addSensorsToList();
	
	setConnections();
}

MonitoringWidget::~MonitoringWidget()
{
	for (int i = 0; i < temperatureData.count(); i++)
	{
		Monitor_Data *temp_data = temperatureData.at(i);
		delete temp_data;
	}
	for (int i = 0; i < voltageData.count(); i++)
	{
		Monitor_Data *volt_data = voltageData.at(i);
		delete volt_data;
	}
	for (int i = 0; i < currentData.count(); i++)
	{
		Monitor_Data *curr_data = currentData.at(i);
		delete curr_data;
	}

	clearCTreeWidget();
	delete ui;
}

void MonitoringWidget::clearCTreeWidget()
{
	if (!c_items.isEmpty()) qDeleteAll(c_items);
	c_items.clear();

	if (!c_title_items.isEmpty()) qDeleteAll(c_title_items);
	c_title_items.clear();
}


void MonitoringWidget::setConnections()
{
	connect(ui->tbtZoomInDU, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(ui->tbtZoomInTU, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(ui->tbtZoomInPU, SIGNAL(clicked()), this, SLOT(zoomIn()));

	connect(ui->tbtZoomOutDU, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(ui->tbtZoomOutTU, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(ui->tbtZoomOutPU, SIGNAL(clicked()), this, SLOT(zoomOut()));

	connect(ui->tbtZoomDU, SIGNAL(clicked()), this, SLOT(adjustMinMax()));
	connect(ui->tbtZoomTU, SIGNAL(clicked()), this, SLOT(adjustMinMax()));
	connect(ui->tbtZoomPU, SIGNAL(clicked()), this, SLOT(adjustMinMax()));

	connect(ui->chboxTimeDU, SIGNAL(clicked(bool)), this, SLOT(setFixedTimeRange(bool)));
	connect(ui->chboxTimeTU, SIGNAL(clicked(bool)), this, SLOT(setFixedTimeRange(bool)));
	connect(ui->chboxTimePU, SIGNAL(clicked(bool)), this, SLOT(setFixedTimeRange(bool)));

	connect(ui->sbxTimeDU, SIGNAL(valueChanged(int)), this, SLOT(changeTimeRange(int)));
	connect(ui->sbxTimeTU, SIGNAL(valueChanged(int)), this, SLOT(changeTimeRange(int)));
	connect(ui->sbxTimePU, SIGNAL(valueChanged(int)), this, SLOT(changeTimeRange(int)));
}


void MonitoringWidget::addSensorsToList()
{
	ui->treeWidget->clear();

	// Создание раздела "Temperature sensors in the Digital Unit"
	QList<CSettings> tempDU_list;
	CSettings title_DU("label", tr("Temperature sensors in the Digital Unit"));
	title_DU.title_flag = true;
	title_DU.text_color = QColor(Qt::black);
	title_DU.background_color = QColor(Qt::magenta).lighter(170);
	tempDU_list.append(title_DU);

	CTreeWidgetItem *c_title_DU = new CTreeWidgetItem(ui->treeWidget, 0, tempDU_list);
	c_title_items.append(c_title_DU);
	c_title_DU->show();
	

	// Создание раздела "Temperature sensors in Transciever Units"
	QList<CSettings> tempTU_list;
	CSettings title_TU("label", tr("Temperature sensors in the Transciever Unit"));
	title_TU.title_flag = true;
	title_TU.text_color = QColor(Qt::black);
	title_TU.background_color = QColor(Qt::magenta).lighter(170);
	tempTU_list.append(title_TU);

	CTreeWidgetItem *c_title_TU = new CTreeWidgetItem(ui->treeWidget, 0, tempTU_list);
	c_title_items.append(c_title_TU);
	c_title_TU->show();
	

	// Создание раздела "Temperature sensors in Power Unit"
	QList<CSettings> tempPU_list;
	CSettings title_PU("label", tr("Temperature sensors in the Power Unit"));
	title_PU.title_flag = true;
	title_PU.text_color = QColor(Qt::black);
	title_PU.background_color = QColor(Qt::magenta).lighter(170);
	tempPU_list.append(title_PU);

	CTreeWidgetItem *c_title_PU = new CTreeWidgetItem(ui->treeWidget, 0, tempPU_list);
	c_title_items.append(c_title_PU);
	c_title_PU->show();
	
	
	QList<CSettings> item_DU1_list;
	CSettings item_settings1("label", tr("Temperature of the Cold Storage #1 ( T<sub>CS</sub> )"));
	item_settings1.text_color = QColor(Qt::darkBlue);		
	item_settings1.background_color = QColor(Qt::magenta).lighter(195);		

	CSettings item_settings11("label", "");
	item_settings11.text_color = QColor(Qt::darkBlue);		
	item_settings11.background_color = QColor(Qt::magenta).lighter(195);

	item_DU1_list << item_settings1 << item_settings11;
	CTreeWidgetItem *c_item_DU1 = new CTreeWidgetItem(ui->treeWidget, c_title_DU->getQSubTreeWidgetItem(), item_DU1_list);
	//connect(c_item_DU1, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
	//c_item_DU1->linkObject(temperatureData[0]);
	c_items.append(c_item_DU1);
	c_item_DU1->show();


	QList<CSettings> item_DU2_list;
	CSettings item_settings2("label", tr("Temperature of the Programmator ( T<sub>CORE</sub> )"));
	item_settings2.text_color = QColor(Qt::darkBlue);		
	item_settings2.background_color = QColor(Qt::magenta).lighter(195);		

	CSettings item_settings21("label", "");
	item_settings21.text_color = QColor(Qt::darkBlue);		
	item_settings21.background_color = QColor(Qt::magenta).lighter(195);

	item_DU2_list << item_settings2 << item_settings21;
	CTreeWidgetItem *c_item_DU2 = new CTreeWidgetItem(ui->treeWidget, c_title_DU->getQSubTreeWidgetItem(), item_DU2_list);
	//connect(c_item_DU2, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
	//c_item_DU2->linkObject(temperatureData[1]);
	c_items.append(c_item_DU2);
	c_item_DU2->show();


	QList<CSettings> item_TU1_list;
	CSettings item_settings3("label", tr("Temperature of the Cold Storage #2 ( T<sub>CS</sub> )"));
	item_settings3.text_color = QColor(Qt::darkBlue);		
	item_settings3.background_color = QColor(Qt::magenta).lighter(195);		

	CSettings item_settings31("label", "");
	item_settings31.text_color = QColor(Qt::darkBlue);		
	item_settings31.background_color = QColor(Qt::magenta).lighter(195);

	item_TU1_list << item_settings3 << item_settings31;
	CTreeWidgetItem *c_item_TU1 = new CTreeWidgetItem(ui->treeWidget, c_title_TU->getQSubTreeWidgetItem(), item_TU1_list);
	//connect(c_item_DU1, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
	//c_item_TU1->linkObject(temperatureData[2]);
	c_items.append(c_item_TU1);
	c_item_TU1->show();


	QList<CSettings> item_TU2_list;
	CSettings item_settings4("label", tr("Temperature of the Downhole Sonde Housing ( T<sub>DSH</sub> )"));
	item_settings4.text_color = QColor(Qt::darkBlue);		
	item_settings4.background_color = QColor(Qt::magenta).lighter(195);		

	CSettings item_settings41("label", "");
	item_settings41.text_color = QColor(Qt::darkBlue);		
	item_settings41.background_color = QColor(Qt::magenta).lighter(195);

	item_TU2_list << item_settings4 << item_settings41;
	CTreeWidgetItem *c_item_TU2 = new CTreeWidgetItem(ui->treeWidget, c_title_TU->getQSubTreeWidgetItem(), item_TU2_list);
	//connect(c_item_DU2, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
	//c_item_TU2->linkObject(temperatureData[3]);
	c_items.append(c_item_TU2);
	c_item_TU2->show();


	QList<CSettings> item_PU1_list;
	CSettings item_settings5("label", tr("Temperature of the Cold Storage #3 ( T<sub>CS</sub> )"));
	item_settings5.text_color = QColor(Qt::darkBlue);		
	item_settings5.background_color = QColor(Qt::magenta).lighter(195);		

	CSettings item_settings51("label", "");
	item_settings51.text_color = QColor(Qt::darkBlue);		
	item_settings51.background_color = QColor(Qt::magenta).lighter(195);

	item_PU1_list << item_settings5 << item_settings51;
	CTreeWidgetItem *c_item_PU1 = new CTreeWidgetItem(ui->treeWidget, c_title_PU->getQSubTreeWidgetItem(), item_PU1_list);
	//connect(c_item_DU1, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
	//c_item_PU1->linkObject(temperatureData[4]);
	c_items.append(c_item_PU1);
	c_item_PU1->show();


	QList<CSettings> item_PU2_list;
	CSettings item_settings6("label", tr("Temperature of the Transformer ( T<sub>TR</sub> )"));
	item_settings6.text_color = QColor(Qt::darkBlue);		
	item_settings6.background_color = QColor(Qt::magenta).lighter(195);		

	CSettings item_settings61("label", "");
	item_settings61.text_color = QColor(Qt::darkBlue);		
	item_settings61.background_color = QColor(Qt::magenta).lighter(195);

	item_PU2_list << item_settings6 << item_settings61;
	CTreeWidgetItem *c_item_PU2 = new CTreeWidgetItem(ui->treeWidget, c_title_PU->getQSubTreeWidgetItem(), item_PU2_list);
	//connect(c_item_DU2, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
	//c_item_PU2->linkObject(temperatureData[5]);
	c_items.append(c_item_PU2);
	c_item_PU2->show();
}


void MonitoringWidget::zoomIn()
{
	QToolButton *tbt = (QToolButton*)sender();
	if (!tbt) return;

	QwtPlot *qwtPlot = NULL;
	if (tbt == ui->tbtZoomInDU) qwtPlot = ui->qwtPlotDUT;
	else if (tbt == ui->tbtZoomInTU) qwtPlot = ui->qwtPlotTUT;
	else if (tbt == ui->tbtZoomInPU) qwtPlot = ui->qwtPlotPUT;
	if (qwtPlot == NULL) return;

	double cur_min = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
	double cur_max = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();
	double delta = cur_max - cur_min;
	delta *= 0.80;

	cur_max = (cur_max+cur_min)/2 + delta/2;
	cur_min = (cur_max+cur_min)/2 - delta/2;

	qwtPlot->setAxisScale(QwtPlot::yLeft, cur_min, cur_max);
	qwtPlot->replot();	
}

void MonitoringWidget::zoomOut()
{
	QToolButton *tbt = (QToolButton*)sender();
	if (!tbt) return;

	QwtPlot *qwtPlot = NULL;
	if (tbt == ui->tbtZoomOutDU) qwtPlot = ui->qwtPlotDUT;
	else if (tbt == ui->tbtZoomOutTU) qwtPlot = ui->qwtPlotTUT;
	else if (tbt == ui->tbtZoomOutPU) qwtPlot = ui->qwtPlotPUT;
	if (qwtPlot == NULL) return;

	double cur_min = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
	double cur_max = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();
	double delta = cur_max - cur_min;
	delta *= 1.25;

	cur_max = (cur_max+cur_min)/2 + delta/2;
	cur_min = (cur_max+cur_min)/2 - delta/2;

	qwtPlot->setAxisScale(QwtPlot::yLeft, cur_min, cur_max);
	qwtPlot->replot();
}


void MonitoringWidget::adjustMinMax()
{
	QToolButton *tbt = (QToolButton*)sender();
	if (!tbt) return;

	QwtPlot *qwtPlot = NULL;
	Monitor_Data *monitor_data1 = NULL;
	Monitor_Data *monitor_data2 = NULL;
	if (tbt == ui->tbtZoomDU) 
	{
		qwtPlot = ui->qwtPlotDUT;
		monitor_data1 = temperatureData[0];
		monitor_data2 = temperatureData[1];
	}
	else if (tbt == ui->tbtZoomTU) 
	{
		qwtPlot = ui->qwtPlotTUT;
		monitor_data1 = temperatureData[2];
		monitor_data2 = temperatureData[3];
	}
	else if (tbt == ui->tbtZoomPU) 
	{
		qwtPlot = ui->qwtPlotPUT;
		monitor_data1 = temperatureData[4];
		monitor_data2 = temperatureData[5];
	}
	if (qwtPlot == NULL) return;
	
	double cur_min = (monitor_data1->min_zoom < monitor_data2->min_zoom ? monitor_data1->min_zoom : monitor_data2->min_zoom);
	double cur_max = (monitor_data1->max_zoom > monitor_data2->max_zoom ? monitor_data1->max_zoom : monitor_data2->max_zoom);

	qwtPlot->setAxisScale(QwtPlot::yLeft, cur_min, cur_max);
	qwtPlot->replot();
}

void MonitoringWidget::setFixedTimeRange(bool is_floating)
{
	QCheckBox *chbox = qobject_cast<QCheckBox*>(sender());
	if (!chbox) return;

	if (chbox == ui->chboxTimeDU)
	{
		temperatureData[0]->time_scale.first = is_floating;
		temperatureData[1]->time_scale.first = is_floating;
		
		ui->sbxTimeDU->setEnabled(is_floating);
		ui->lblDUminuts->setEnabled(is_floating);
	}
	else if (chbox == ui->chboxTimeTU)
	{
		temperatureData[2]->time_scale.first = is_floating;
		temperatureData[3]->time_scale.first = is_floating;

		ui->sbxTimeTU->setEnabled(is_floating);
		ui->lblTUminuts->setEnabled(is_floating);
	}
	else if (chbox == ui->chboxTimePU)
	{
		temperatureData[4]->time_scale.first = is_floating;
		temperatureData[5]->time_scale.first = is_floating;

		ui->sbxTimePU->setEnabled(is_floating);
		ui->lblPUminuts->setEnabled(is_floating);
	}
}

void MonitoringWidget::changeTimeRange(int range)
{
	QSpinBox *sbx = qobject_cast<QSpinBox*>(sender());
	if (!sbx) return;

	if (sbx == ui->sbxTimeDU)
	{
		temperatureData[0]->time_scale.second = range;
		temperatureData[1]->time_scale.second = range;
	}
	else if (sbx == ui->sbxTimeTU)
	{
		temperatureData[2]->time_scale.second = range;
		temperatureData[3]->time_scale.second = range;
	}
	else if (sbx == ui->sbxTimePU)
	{
		temperatureData[4]->time_scale.second = range;
		temperatureData[5]->time_scale.second = range;
	}
}


QPair<QDateTime, double> MonitoringWidget::lastVoltagePoint(int index) 
{ 
	QDateTime dtime = voltageData[index]->time->last();
	double vol = voltageData[index]->data->last();
	QPair<QDateTime, double> out = QPair<QDateTime, double>(dtime, vol); 

	return out;
}

QPair<QDateTime, double> MonitoringWidget::lastTemperaturePoint(int index) 
{ 
	QDateTime dtime = temperatureData[index]->time->last();
	double temp = temperatureData[index]->data->last();
	QPair<QDateTime, double> out = QPair<QDateTime, double>(dtime, temp); 

	return out;
}

QPair<QDateTime, double> MonitoringWidget::lastCurrentPoint(int index) 
{ 
	QDateTime dtime = currentData[index]->time->last();
	double cur = currentData[index]->data->last();
	QPair<QDateTime, double> out = QPair<QDateTime, double>(dtime, cur); 

	return out;
}


void MonitoringWidget::addMonitoringBoard(QString board_name, QList<double> minmax, QList<double> minmax_zoom, QPair<bool, int> time_scale)
{
	double min1 = minmax[0];
	double max1 = minmax[1];
	double min2 = minmax[2];
	double max2 = minmax[3];

	Monitor_Data *mon_T0 = 0;
	Monitor_Data *mon_T1 = 0;
	if (minmax_zoom.count() == 4)
	{
		mon_T0 = new Monitor_Data(min1, max1, minmax_zoom[0], minmax_zoom[1], time_scale);
		mon_T1 = new Monitor_Data(min2, max2, minmax_zoom[2], minmax_zoom[3], time_scale);
	}
	else
	{
		mon_T0 = new Monitor_Data(min1, max1, time_scale);
		mon_T1 = new Monitor_Data(min2, max2, time_scale);
	}
	
	QVector<double> *x_data_temp0 = new QVector<double>;
	QVector<double> *y_data_temp0 = new QVector<double>;
	QVector<QDateTime> *dt_data_temp0 = new QVector<QDateTime>;

	mon_T0->dtime = x_data_temp0;
	mon_T0->time = dt_data_temp0;
	mon_T0->data = y_data_temp0;

	QwtPlotCurve *temp0_curve = new QwtPlotCurve(board_name + "_T1"); 
	temp0_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
	temp0_curve->setStyle(QwtPlotCurve::Lines);

	QwtSymbol *sym0 = new QwtSymbol;
	QwtSymbol::Style sym_style = QwtSymbol::Ellipse;
	sym0->setStyle(sym_style);
	sym0->setSize(5);
	QPen sym_pen0 = sym0->pen();
	sym_pen0.setColor(QColor(Qt::darkRed));
	sym_pen0.setWidth(1);
	sym0->setPen(sym_pen0);
	QBrush sym_brush0 = sym0->brush();
	sym_brush0.setColor(QColor(Qt::red));
	sym_brush0.setStyle(Qt::SolidPattern);
	sym0->setBrush(sym_brush0);
	temp0_curve->setSymbol(sym0);
	QPen pen0;
	pen0.setColor(QColor(Qt::darkRed));
	pen0.setWidth(1);
	temp0_curve->setPen(pen0);

	mon_T0->curve = temp0_curve;

	temperatureData.append(mon_T0);
		

	QVector<double> *x_data_temp1 = new QVector<double>;
	QVector<double> *y_data_temp1 = new QVector<double>;
	QVector<QDateTime> *dt_data_temp1 = new QVector<QDateTime>;

	mon_T1->dtime = x_data_temp1;
	mon_T1->time = dt_data_temp1;
	mon_T1->data = y_data_temp1;

	QwtPlotCurve *temp1_curve = new QwtPlotCurve(board_name + "_T2"); 
	temp1_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
	temp1_curve->setStyle(QwtPlotCurve::Lines);

	QwtSymbol *sym1 = new QwtSymbol;	
	sym1->setStyle(sym_style);
	sym1->setSize(5);
	QPen sym_pen1 = sym1->pen();
	sym_pen1.setColor(QColor(Qt::darkBlue));
	sym_pen1.setWidth(1);
	sym1->setPen(sym_pen1);
	QBrush sym_brush1 = sym1->brush();
	sym_brush1.setColor(QColor(Qt::blue));
	sym_brush1.setStyle(Qt::SolidPattern);
	sym1->setBrush(sym_brush1);
	temp1_curve->setSymbol(sym1);	
	QPen pen1;
	pen1.setColor(QColor(Qt::darkBlue));
	pen1.setWidth(1);
	temp1_curve->setPen(pen1);

	mon_T1->curve = temp1_curve;

	temperatureData.append(mon_T1);
}


void MonitoringWidget::initMonitoringObjects()
{	
	temperatureData.clear();
	addMonitoringBoard("DU", QList<double>() << 0 << 80 << 0 << 80, QList<double>() << -20 << 100 << -20 << 100, QPair<bool,int>(false, TIME_RANGE));	
	addMonitoringBoard("TU", QList<double>() << 0 << 80 << 0 << 80, QList<double>() << -20 << 100 << -20 << 100, QPair<bool,int>(false, TIME_RANGE));	
	addMonitoringBoard("PU", QList<double>() << 0 << 80 << 0 << 80, QList<double>() << -20 << 100 << -20 << 100, QPair<bool,int>(false, TIME_RANGE));	
}

void MonitoringWidget::addData(uint8_t _comm_id, QVector<double> *_x_data, QVector<double> *_y_data)
{
	bool temp_ok = true;

	switch (_comm_id)
	{
	case DT_DU_T:
		{			
			uint bs = base_date_time.toTime_t();

			QVector<QDateTime> *dt0 = temperatureData[0]->time;			
			QDateTime _dt0 = QDateTime::fromTime_t((uint)_x_data->at(0));			
			dt0->push_back(_dt0);
			QVector<double> *dtime0 = temperatureData[0]->dtime;
			dtime0->push_back(_x_data->at(0)-bs);
			QVector<double> *temp0 = temperatureData[0]->data;
			double T0 = _y_data->at(0);
			if (T0 < MIN_CRITICAL_TEMP || T0 > MAX_CRITICAL_TEMP || T0 != T0) return;
			temp0->push_back(T0);

			QwtPlotCurve *temp0_curve = temperatureData[0]->curve;
			temp0_curve->setSamples(dtime0->data(), temp0->data(), temp0->size());						
			temp0_curve->attach(ui->qwtPlotDUT);

			CTreeWidgetItem *ctree_item0 = c_items.at(0);
			CLabel *clbl0 = qobject_cast<CLabel*>(ctree_item0->getCWidgets().last());			
			

			QVector<QDateTime> *dt1 = temperatureData[1]->time;
			QDateTime _dt1 = QDateTime::fromTime_t((uint)_x_data->at(1));			
			dt1->push_back(_dt1);
			QVector<double> *dtime1 = temperatureData[1]->dtime;
			dtime1->push_back(_x_data->at(1)-bs);
			QVector<double> *temp1 = temperatureData[1]->data;
			double T1 = _y_data->at(1);
			if (T1 < MIN_CRITICAL_TEMP || T1 > MAX_CRITICAL_TEMP || T1 != T1) return;			
			temp1->push_back(T1);
						
			QwtPlotCurve *temp1_curve = temperatureData[1]->curve;
			temp1_curve->setSamples(dtime1->data(), temp1->data(), temp1->size());
			temp1_curve->attach(ui->qwtPlotDUT);

			CTreeWidgetItem *ctree_item1 = c_items.at(1);
			CLabel *clbl1 = qobject_cast<CLabel*>(ctree_item1->getCWidgets().last());
			
			double from_time = dtime1->first();
			double to_time = dtime1->last();
			double time_range = temperatureData[1]->time_scale.second;
			bool floating_timescale = temperatureData[1]->time_scale.first;			
			if (floating_timescale) 
			{
				from_time = dtime1->last() - time_range*0.75*60;		// 60 seconds in 1 minute
				to_time = dtime1->last() + time_range*0.25*60;			// 60 seconds in 1 minute
			}						
			ui->qwtPlotDUT->setAxisScale(QwtPlot::xBottom, from_time, to_time);			
			ui->qwtPlotDUT->replot();

			if (T0 > temperatureData[0]->max_data)
			{
				ui->lbl_DUT1->setStyleSheet("QLabel { background-color : lightsalmon; color : darkRed }");
				ui->lbl_DUT1->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T0, 'f', 1)));				
				
				if (clbl0) clbl0->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T0, 'f', 1)));
				
				temp_ok = false;
			}
			else 
			{
				ui->lbl_DUT1->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
				ui->lbl_DUT1->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T0, 'f', 1)));	

				if (clbl0) clbl0->setText(QString("<font color=green>%1 <sup>0</sup>C").arg(QString::number(T0, 'f', 1)));
			}
			
			if (T1 > temperatureData[1]->max_data)
			{
				ui->lbl_DUT2->setStyleSheet("QLabel { background-color : lightsalmon; color : darkRed }");				
				ui->lbl_DUT2->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T1, 'f', 1)));		
				
				if (clbl1) clbl1->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T1, 'f', 1)));
				
				temp_ok = false;
			}
			else 
			{
				ui->lbl_DUT2->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
				ui->lbl_DUT2->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T1, 'f', 1)));

				if (clbl1) clbl1->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T1, 'f', 1)));
			}			
			break;
		}
	case DT_TU_T:
		{			
			uint bs = base_date_time.toTime_t();

			QVector<QDateTime> *dt0 = temperatureData[2]->time;			
			QDateTime _dt0 = QDateTime::fromTime_t((uint)_x_data->at(0));			
			dt0->push_back(_dt0);
			QVector<double> *dtime0 = temperatureData[2]->dtime;
			dtime0->push_back(_x_data->at(0)-bs);
			QVector<double> *temp0 = temperatureData[2]->data;
			double T0 = _y_data->at(0);
			if (T0 < MIN_CRITICAL_TEMP || T0 > MAX_CRITICAL_TEMP || T0 != T0) return;
			temp0->push_back(T0);

			QVector<QDateTime> *dt1 = temperatureData[3]->time;
			QDateTime _dt1 = QDateTime::fromTime_t((uint)_x_data->at(1));			
			dt1->push_back(_dt1);
			QVector<double> *dtime1 = temperatureData[3]->dtime;
			dtime1->push_back(_x_data->at(1)-bs);
			QVector<double> *temp1 = temperatureData[3]->data;
			double T1 = _y_data->at(1);
			if (T1 < MIN_CRITICAL_TEMP || T1 > MAX_CRITICAL_TEMP || T1 != T1) return;
			temp1->push_back(T1);
			
			//temp0->push_back(T1);		// Для TU в целях более удобного отображения данных 
			//temp1->push_back(T0);		// целесообразно переставить T0 и T1 местами

			QwtPlotCurve *temp0_curve = temperatureData[2]->curve;
			temp0_curve->setSamples(dtime0->data(), temp0->data(), temp0->size());
			temp0_curve->attach(ui->qwtPlotTUT);	

			CTreeWidgetItem *ctree_item0 = c_items.at(2);
			CLabel *clbl0 = qobject_cast<CLabel*>(ctree_item0->getCWidgets().last());


			QwtPlotCurve *temp1_curve = temperatureData[3]->curve;
			temp1_curve->setSamples(dtime1->data(), temp1->data(), temp1->size());
			temp1_curve->attach(ui->qwtPlotTUT);


			//double from_time = dtime1->last()-900;	
			//double to_time = dtime1->last()+300;	
			double from_time = dtime1->first();
			double to_time = dtime1->last();
			double time_range = temperatureData[3]->time_scale.second;
			bool floating_timescale = temperatureData[3]->time_scale.first;			
			if (floating_timescale) 
			{
				from_time = dtime1->last() - time_range*0.75*60;		// 60 seconds in 1 minute
				to_time = dtime1->last() + time_range*0.25*60;			// 60 seconds in 1 minute
			}
			ui->qwtPlotTUT->setAxisScale(QwtPlot::xBottom, from_time, to_time);			
			ui->qwtPlotTUT->replot();

			CTreeWidgetItem *ctree_item1 = c_items.at(3);
			CLabel *clbl1 = qobject_cast<CLabel*>(ctree_item1->getCWidgets().last());

						
			if (T0 > temperatureData[2]->max_data)
			{
				ui->lbl_TUT1->setStyleSheet("QLabel { background-color : lightsalmon; color : darkRed }");
				ui->lbl_TUT1->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T0, 'f', 1)));

				if (clbl0) clbl0->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T0, 'f', 1)));

				temp_ok = false;
			}
			else 
			{
				ui->lbl_TUT1->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
				ui->lbl_TUT1->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T0, 'f', 1)));

				if (clbl1) clbl1->setText(QString("<font color=green>%1 <sup>0</sup>C").arg(QString::number(T0, 'f', 1)));
			}
			
			if (T1 > temperatureData[3]->max_data)
			{
				ui->lbl_TUT2->setStyleSheet("QLabel { background-color : lightsalmon; color : darkRed }");
				ui->lbl_TUT2->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T1, 'f', 1)));

				if (clbl1) clbl1->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T1, 'f', 1)));

				temp_ok = false;
			}
			else 
			{
				ui->lbl_TUT2->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
				ui->lbl_TUT2->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T1, 'f', 1)));

				if (clbl1) clbl1->setText(QString("<font color=green>%1 <sup>0</sup>C").arg(QString::number(T1, 'f', 1)));
			}			
			break;
		}
	case DT_PU_T:
		{			
			uint bs = base_date_time.toTime_t();

			QVector<QDateTime> *dt0 = temperatureData[4]->time;			
			QDateTime _dt0 = QDateTime::fromTime_t((uint)_x_data->at(0));			
			dt0->push_back(_dt0);
			QVector<double> *dtime0 = temperatureData[4]->dtime;
			dtime0->push_back(_x_data->at(0)-bs);
			QVector<double> *temp0 = temperatureData[4]->data;
			double T0 = _y_data->at(0);
			if (T0 < MIN_CRITICAL_TEMP || T0 > MAX_CRITICAL_TEMP || T0 != T0) return;
			temp0->push_back(T0);

			QwtPlotCurve *temp0_curve = temperatureData[4]->curve;
			temp0_curve->setSamples(dtime0->data(), temp0->data(), temp0->size());
			temp0_curve->attach(ui->qwtPlotPUT);

			CTreeWidgetItem *ctree_item0 = c_items.at(4);
			CLabel *clbl0 = qobject_cast<CLabel*>(ctree_item0->getCWidgets().last());


			QVector<QDateTime> *dt1 = temperatureData[5]->time;
			QDateTime _dt1 = QDateTime::fromTime_t((uint)_x_data->at(1));			
			dt1->push_back(_dt1);
			QVector<double> *dtime1 = temperatureData[5]->dtime;
			dtime1->push_back(_x_data->at(1)-bs);
			QVector<double> *temp1 = temperatureData[5]->data;
			double T1 = _y_data->at(1);		
			if (T1 < MIN_CRITICAL_TEMP || T1 > MAX_CRITICAL_TEMP || T1 != T1) return;
			temp1->push_back(T1);

			QwtPlotCurve *temp1_curve = temperatureData[5]->curve;
			temp1_curve->setSamples(dtime1->data(), temp1->data(), temp1->size());
			temp1_curve->attach(ui->qwtPlotPUT);

			CTreeWidgetItem *ctree_item1 = c_items.at(5);
			CLabel *clbl1 = qobject_cast<CLabel*>(ctree_item1->getCWidgets().last());


			//double from_time = dtime1->last()-900;	
			//double to_time = dtime1->last()+300;	
			double from_time = dtime1->first();
			double to_time = dtime1->last();
			double time_range = temperatureData[5]->time_scale.second;
			bool floating_timescale = temperatureData[5]->time_scale.first;			
			if (floating_timescale) 
			{
				from_time = dtime1->last() - time_range*0.75*60;		// 60 seconds in 1 minute
				to_time = dtime1->last() + time_range*0.25*60;			// 60 seconds in 1 minute
			}
			ui->qwtPlotPUT->setAxisScale(QwtPlot::xBottom, from_time, to_time);			
			ui->qwtPlotPUT->replot();
			
			if (T0 > temperatureData[4]->max_data)
			{
				ui->lbl_PUT1->setStyleSheet("QLabel { background-color : lightsalmon; color : darkRed }");
				ui->lbl_PUT1->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T0, 'f', 1)));

				if (clbl0) clbl0->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T0, 'f', 1)));

				temp_ok = false;
			}
			else 
			{
				ui->lbl_PUT1->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
				ui->lbl_PUT1->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T0, 'f', 1)));

				if (clbl0) clbl0->setText(QString("<font color=green>%1 <sup>0</sup>C").arg(QString::number(T0, 'f', 1)));
			}
			
			if (T1 > temperatureData[5]->max_data)
			{
				ui->lbl_PUT2->setStyleSheet("QLabel { background-color : lightsalmon; color : darkRed }");
				ui->lbl_PUT2->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T1, 'f', 1)));

				if (clbl1) clbl1->setText(QString("<font color=red>%1 <sup>0</sup>C").arg(QString::number(T1, 'f', 1)));

				temp_ok = false;
			}
			else 
			{
				ui->lbl_PUT2->setStyleSheet("QLabel { background-color : rgb(170, 255, 0); color : black; }");
				ui->lbl_PUT2->setText(QString("<b>%1 <sup>0</sup>C</b>").arg(QString::number(T1, 'f', 1)));

				if (clbl1) clbl1->setText(QString("<font color=green>%1 <sup>0</sup>C").arg(QString::number(T1, 'f', 1)));
			}			
			break;
		}	
	default: break;
	}

	emit temperature_status(temp_ok ? TempOK : TempHigh);
}