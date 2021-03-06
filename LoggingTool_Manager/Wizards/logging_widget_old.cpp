#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"

//#include "../Dialogs/depth_scale_dialog.h"

#include "main_window.h"

#include "logging_widget.h"


PaintFrame_forLogging::PaintFrame_forLogging(QWidget *parent)
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

PaintFrame_forLogging::PaintFrame_forLogging(QwtCurveSettings *c_settings, QWidget *parent)
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


void PaintFrame_forLogging::paintEvent(QPaintEvent *event)
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

void PaintFrame_forLogging::drawDataSymbol(QPainter *painter, QwtSymbol::Style symType)
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


LoggingWidget::LoggingWidget(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	
	ui.tbtSearchData->setIcon(QIcon(":/images/find_signal.png"));
	ui.tbtClearAll->setIcon(QIcon(":/images/Eraser.png"));
	ui.tbtScaleIn_1->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_1->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_2->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_2->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_3->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_3->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_4->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_4->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_5->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_5->setIcon(QIcon(":/images/zoom_out.png"));
	ui.tbtPlus->setIcon(QIcon(":/images/plus.png"));
	ui.tbtMinus->setIcon(QIcon(":/images/minus.png"));

	ui.tbtSearchData->setToolTip(tr("Search data and rescale to show all"));
	ui.tbtScaleIn_1->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_1->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_2->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_2->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_3->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_3->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_4->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_4->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_5->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_5->setToolTip(tr("Scale data out"));	

	QPalette p = ui.frameTools->palette(); 
	p.setColor(QPalette::Background, QColor(Qt::magenta).lighter(195));
	ui.frameTools->setAutoFillBackground(true);
	ui.frameTools->setPalette(p);
	ui.frameTools->show();

	ui.qwtPlot1->enableAxis(QwtPlot::yLeft, true);
	ui.qwtPlot2->enableAxis(QwtPlot::yLeft, false);
	ui.qwtPlot3->enableAxis(QwtPlot::yLeft, false);
	ui.qwtPlot4->enableAxis(QwtPlot::yLeft, false);
	ui.qwtPlot5->enableAxis(QwtPlot::yLeft, false);

	ui.qwtPlot1->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot2->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot3->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot4->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot5->enableAxis(QwtPlot::xTop, false);

	ui.qwtPlot5->enableAxis(QwtPlot::yRight, true);

	ui.qwtPlot1->setMinimumWidth(230);
	ui.qwtPlot5->setMinimumWidth(230);
	
	QwtText title_depth;
	QString title_depth_text = QString("Depth, m");
	title_depth.setText(title_depth_text);
	title_depth.setColor(QColor(Qt::darkBlue));
	QFont font_depth = ui.qwtPlot1->axisTitle(QwtPlot::yLeft).font();
	QFont axis_font = ui.qwtPlot1->axisFont(QwtPlot::yLeft);
	axis_font.setPointSize(9);
	axis_font.setBold(false);
	font_depth.setPointSize(10);
	font_depth.setBold(false);
	title_depth.setFont(font_depth);

	QwtText title_empty;
	QString title_empty_text = QString(" ");
	title_empty.setText(title_empty_text);
	title_empty.setColor(QColor(Qt::darkBlue));
	QFont font_empty = ui.qwtPlot1->axisTitle(QwtPlot::xBottom).font();
	font_empty.setPointSize(10);
	font_empty.setBold(false);
	title_empty.setFont(font_empty);
	
	ui.qwtPlot1->setAxisFont(QwtPlot::yLeft, axis_font);	
	ui.qwtPlot1->setAxisTitle(QwtPlot::yLeft, title_depth);
	ui.qwtPlot1->setAxisFont(QwtPlot::xTop, axis_font);	
	ui.qwtPlot1->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot1->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot2->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot2->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot2->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot3->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot3->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot3->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot4->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot4->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot4->setAxisTitle(QwtPlot::xBottom, title_empty);	
	ui.qwtPlot5->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot5->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot5->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot5->setAxisFont(QwtPlot::yRight, axis_font);

	qwtplot_list.append(ui.qwtPlot1);
	qwtplot_list.append(ui.qwtPlot2);
	qwtplot_list.append(ui.qwtPlot3);
	qwtplot_list.append(ui.qwtPlot4);
	qwtplot_list.append(ui.qwtPlot5);

	QStringList items;
	items << tr("No Data");
	items << tr("NMR Integral (Probe#1)") << tr("NMR Integral (Probe#2)") << tr("NMR Integral (Probe#3)");
	items << tr("NMR Bins (Probe#1)") << tr("NMR Bins (Probe#2)") << tr("NMR Bins (Probe#3)");
	items << tr("Gamma Logging");
	items << tr("Wave Dielectric Logging (Phase difference)") << tr("Wave Dielectric Logging (Amplitude ratio)");
	items << tr("Maximum of AFR (Probe#1)") << tr("Maximum of AFR (Probe#2)") << tr("Maximum of AFR (Probe#3)");
	ui.cboxLog1->addItems(items); 
	ui.cboxLog2->addItems(items);
	ui.cboxLog3->addItems(items);
	ui.cboxLog4->addItems(items);
	ui.cboxLog5->addItems(items);

	LoggingData *dcont1 = new LoggingData(LoggingData::DataType::NMRIntegral_Probe1);
	LoggingData *dcont2 = new LoggingData(LoggingData::DataType::NMRIntegral_Probe2);
	LoggingData *dcont3 = new LoggingData(LoggingData::DataType::NMRIntegral_Probe3);
	LoggingData *dcont4 = new LoggingData(LoggingData::DataType::NMRBins_Probe1);
	LoggingData *dcont5 = new LoggingData(LoggingData::DataType::NMRBins_Probe2);
	LoggingData *dcont6 = new LoggingData(LoggingData::DataType::NMRBins_Probe3);
	LoggingData *dcont7 = new LoggingData(LoggingData::DataType::Gamma);
	LoggingData *dcont8 = new LoggingData(LoggingData::DataType::WaveDielectric_PhaseDiff);
	LoggingData *dcont9 = new LoggingData(LoggingData::DataType::WaveDielectric_AmplRatio);
	LoggingData *dcont10 = new LoggingData(LoggingData::DataType::AFR_Probe1);
	LoggingData *dcont11 = new LoggingData(LoggingData::DataType::AFR_Probe2);
	LoggingData *dcont12 = new LoggingData(LoggingData::DataType::AFR_Probe3);
	LoggingData *dcont13 = new LoggingData(LoggingData::DataType::NoType);
	data_containers << dcont1 << dcont2 << dcont3 << dcont4 << dcont5 << dcont6 << dcont7 << dcont8 << dcont9 << dcont10 << dcont11 << dcont12 << dcont13;

	LoggingPlot *logging_plot1 = new LoggingPlot(dcont4, ui.qwtPlot1, ui.frame_1, 0);
	logging_plot1->setDataType(LoggingData::NMRBins_Probe1, data_containers.at(3));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::NMRBins_Probe1, 0));
	ui.cboxLog1->setCurrentIndex(4);
	setAxisPlotTitle(ui.qwtPlot1, QwtPlot::xBottom, tr("Integral value, a.u."));
		
	LoggingPlot *logging_plot2 = new LoggingPlot(dcont7, ui.qwtPlot2, ui.frame_2, 1);
	logging_plot2->setDataType(LoggingData::Gamma, data_containers.at(6));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::Gamma, 1));	
	ui.cboxLog2->setCurrentIndex(7);
	setAxisPlotTitle(ui.qwtPlot2, QwtPlot::xBottom, tr("Counts per second, 1/sec"));

	LoggingPlot *logging_plot3 = new LoggingPlot(dcont8, ui.qwtPlot3, ui.frame_3, 2);
	logging_plot3->setDataType(LoggingData::WaveDielectric_PhaseDiff, data_containers.at(7));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::WaveDielectric_PhaseDiff, 2));	
	ui.cboxLog3->setCurrentIndex(8);
	setAxisPlotTitle(ui.qwtPlot3, QwtPlot::xBottom, tr("Phase difference, degree"));

	LoggingPlot *logging_plot4 = new LoggingPlot(dcont9, ui.qwtPlot4, ui.frame_4, 3);
	logging_plot4->setDataType(LoggingData::WaveDielectric_AmplRatio, data_containers.at(8));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::WaveDielectric_AmplRatio, 3));
	ui.cboxLog4->setCurrentIndex(9);
	setAxisPlotTitle(ui.qwtPlot4, QwtPlot::xBottom, tr("Amplitude ratio, a.u."));

	LoggingPlot *logging_plot5 = new LoggingPlot(dcont13, ui.qwtPlot5, ui.frame_5, 4);
	logging_plot5->setDataType(LoggingData::NoType, data_containers.at(9));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::NoType, 4));
	ui.cboxLog5->setCurrentIndex(0);
	setAxisPlotTitle(ui.qwtPlot5, QwtPlot::xBottom, " ");	

	logging_plot_list.append(logging_plot1);
	logging_plot_list.append(logging_plot2);
	logging_plot_list.append(logging_plot3);
	logging_plot_list.append(logging_plot4);
	logging_plot_list.append(logging_plot5);
	
	ui.frame_1->setMaximumHeight(32);
	ui.frame_1->setMinimumHeight(32);
	ui.frame_2->setMaximumHeight(32);
	ui.frame_2->setMinimumHeight(32);
	ui.frame_3->setMaximumHeight(32);
	ui.frame_3->setMinimumHeight(32);
	ui.frame_4->setMaximumHeight(32);
	ui.frame_4->setMinimumHeight(32);
	ui.frame_5->setMaximumHeight(32);
	ui.frame_5->setMinimumHeight(32);

	double up_depth = ui.qwtPlot1->axisScaleDiv(QwtPlot::yLeft).upperBound();
	double down_depth = ui.qwtPlot1->axisScaleDiv(QwtPlot::yLeft).lowerBound();
	ui.cntDepthFrom->setValue(int(up_depth));
	ui.cntDepthTo->setValue(int(down_depth));
		
	setConnections();
}

/*
LoggingWidget::LoggingWidget(QVector<ToolChannel*> channels, QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	ui.tbtSearchData->setIcon(QIcon(":/images/find_signal.png"));
	ui.tbtClearAll->setIcon(QIcon(":/images/Eraser.png"));
	ui.tbtScaleIn_1->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_1->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_2->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_2->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_3->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_3->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_4->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_4->setIcon(QIcon(":/images/zoom_out.png"));	
	ui.tbtScaleIn_5->setIcon(QIcon(":/images/zoom_in.png"));
	ui.tbtScaleOut_5->setIcon(QIcon(":/images/zoom_out.png"));
	ui.tbtPlus->setIcon(QIcon(":/images/plus.png"));
	ui.tbtMinus->setIcon(QIcon(":/images/minus.png"));

	ui.tbtSearchData->setToolTip(tr("Search data and rescale to show all"));
	ui.tbtScaleIn_1->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_1->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_2->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_2->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_3->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_3->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_4->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_4->setToolTip(tr("Scale data out"));	
	ui.tbtScaleIn_5->setToolTip(tr("Scale data in"));
	ui.tbtScaleOut_5->setToolTip(tr("Scale data out"));	
	
	QPalette p = ui.frameTools->palette(); 
	p.setColor(QPalette::Background, QColor(Qt::magenta).lighter(195));
	ui.frameTools->setAutoFillBackground(true);
	ui.frameTools->setPalette(p);
	ui.frameTools->show();

	ui.qwtPlot1->enableAxis(QwtPlot::yLeft, true);
	ui.qwtPlot2->enableAxis(QwtPlot::yLeft, false);
	ui.qwtPlot3->enableAxis(QwtPlot::yLeft, false);
	ui.qwtPlot4->enableAxis(QwtPlot::yLeft, false);
	ui.qwtPlot5->enableAxis(QwtPlot::yLeft, false);

	ui.qwtPlot1->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot2->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot3->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot4->enableAxis(QwtPlot::xTop, false);
	ui.qwtPlot5->enableAxis(QwtPlot::xTop, false);

	ui.qwtPlot5->enableAxis(QwtPlot::yRight, true);

	ui.qwtPlot1->setMinimumWidth(230);
	ui.qwtPlot5->setMinimumWidth(230);
	
	QwtText title_depth;
	QString title_depth_text = QString("Depth, m");
	title_depth.setText(title_depth_text);
	title_depth.setColor(QColor(Qt::darkBlue));
	QFont font_depth = ui.qwtPlot1->axisTitle(QwtPlot::yLeft).font();
	QFont axis_font = ui.qwtPlot1->axisFont(QwtPlot::yLeft);
	axis_font.setPointSize(9);
	axis_font.setBold(false);
	font_depth.setPointSize(10);
	font_depth.setBold(false);
	title_depth.setFont(font_depth);

	QwtText title_empty;
	QString title_empty_text = QString(" ");
	title_empty.setText(title_empty_text);
	title_empty.setColor(QColor(Qt::darkBlue));
	QFont font_empty = ui.qwtPlot1->axisTitle(QwtPlot::xBottom).font();
	font_empty.setPointSize(10);
	font_empty.setBold(false);
	title_empty.setFont(font_empty);
	
	ui.qwtPlot1->setAxisFont(QwtPlot::yLeft, axis_font);	
	ui.qwtPlot1->setAxisTitle(QwtPlot::yLeft, title_depth);
	ui.qwtPlot1->setAxisFont(QwtPlot::xTop, axis_font);	
	ui.qwtPlot1->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot1->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot2->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot2->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot2->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot3->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot3->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot3->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot4->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot4->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot4->setAxisTitle(QwtPlot::xBottom, title_empty);	
	ui.qwtPlot5->setAxisFont(QwtPlot::xTop, axis_font);
	ui.qwtPlot5->setAxisFont(QwtPlot::xBottom, axis_font);
	ui.qwtPlot5->setAxisTitle(QwtPlot::xBottom, title_empty);
	ui.qwtPlot5->setAxisFont(QwtPlot::yRight, axis_font);

	qwtplot_list.append(ui.qwtPlot1);
	qwtplot_list.append(ui.qwtPlot2);
	qwtplot_list.append(ui.qwtPlot3);
	qwtplot_list.append(ui.qwtPlot4);
	qwtplot_list.append(ui.qwtPlot5);

	QStringList items;
	items << tr("No Data");
	for (int i = 0; i < channels.count(); i++)
	{
		bool show_on = isVisibleChannel(channels[i]);
		if (show_on) items << channels[i]->name;
	}
	ui.cboxLog1->addItems(items); 
	ui.cboxLog2->addItems(items);
	ui.cboxLog3->addItems(items);
	ui.cboxLog4->addItems(items);
	ui.cboxLog5->addItems(items);


	//bool _ok;
	//QMap<int, int> probe_counter;
	//for (int i = 0; i < channels.count(); i++)
	//{
	//	if (getDataTypeId(channels[i]->data_type, &_ok) == NMR_CHANNEL) probe_counter[NMR_CHANNEL]++;
	//	else if (getDataTypeId(channels[i]->data_type, &_ok) == SDSP_CHANNEL) probe_counter[SDSP_CHANNEL]++;
	//	else if (getDataTypeId(channels[i]->data_type, &_ok) == GK_CHANNEL) probe_counter[GK_CHANNEL]++;
	//	else if (getDataTypeId(channels[i]->data_type, &_ok) == AFR_CHANNEL) probe_counter[AFR_CHANNEL]++;
	//}
	
	LoggingData *dcont1 = new LoggingData(LoggingData::DataType::NMRIntegral_Probe1, 1);
	LoggingData *dcont2 = new LoggingData(LoggingData::DataType::NMRIntegral_Probe2, 2);
	LoggingData *dcont3 = new LoggingData(LoggingData::DataType::NMRIntegral_Probe3, 3);
	LoggingData *dcont4 = new LoggingData(LoggingData::DataType::NMRBins_Probe1, 1);
	LoggingData *dcont5 = new LoggingData(LoggingData::DataType::NMRBins_Probe2, 2);
	LoggingData *dcont6 = new LoggingData(LoggingData::DataType::NMRBins_Probe3, 3);
	LoggingData *dcont7 = new LoggingData(LoggingData::DataType::Gamma, 1);
	LoggingData *dcont8 = new LoggingData(LoggingData::DataType::WaveDielectric_PhaseDiff, 1);
	LoggingData *dcont9 = new LoggingData(LoggingData::DataType::WaveDielectric_AmplRatio, 1);
	LoggingData *dcont10 = new LoggingData(LoggingData::DataType::AFR_Probe1, 1);
	LoggingData *dcont11 = new LoggingData(LoggingData::DataType::AFR_Probe2, 2);
	LoggingData *dcont12 = new LoggingData(LoggingData::DataType::AFR_Probe3, 3);
	LoggingData *dcont13 = new LoggingData(LoggingData::DataType::NoType, 1);
	data_containers << dcont1 << dcont2 << dcont3 << dcont4 << dcont5 << dcont6 << dcont7 << dcont8 << dcont9 << dcont10 << dcont11 << dcont12 << dcont13;

	LoggingPlot *logging_plot1 = new LoggingPlot(dcont4, ui.qwtPlot1, ui.frame_1, 0);
	logging_plot1->setDataType(LoggingData::NMRBins_Probe1, data_containers.at(3));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::NMRBins_Probe1, 0));
	ui.cboxLog1->setCurrentIndex(4);
	setAxisPlotTitle(ui.qwtPlot1, QwtPlot::xBottom, tr("Integral value, a.u."));
		
	LoggingPlot *logging_plot2 = new LoggingPlot(dcont7, ui.qwtPlot2, ui.frame_2, 1);
	logging_plot2->setDataType(LoggingData::Gamma, data_containers.at(6));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::Gamma, 1));	
	ui.cboxLog2->setCurrentIndex(7);
	setAxisPlotTitle(ui.qwtPlot2, QwtPlot::xBottom, tr("Counts per second, 1/sec"));

	LoggingPlot *logging_plot3 = new LoggingPlot(dcont8, ui.qwtPlot3, ui.frame_3, 2);
	logging_plot3->setDataType(LoggingData::WaveDielectric_PhaseDiff, data_containers.at(7));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::WaveDielectric_PhaseDiff, 2));	
	ui.cboxLog3->setCurrentIndex(8);
	setAxisPlotTitle(ui.qwtPlot3, QwtPlot::xBottom, tr("Phase difference, degree"));

	LoggingPlot *logging_plot4 = new LoggingPlot(dcont9, ui.qwtPlot4, ui.frame_4, 3);
	logging_plot4->setDataType(LoggingData::WaveDielectric_AmplRatio, data_containers.at(8));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::WaveDielectric_AmplRatio, 3));
	ui.cboxLog4->setCurrentIndex(9);
	setAxisPlotTitle(ui.qwtPlot4, QwtPlot::xBottom, tr("Amplitude ratio, a.u."));

	LoggingPlot *logging_plot5 = new LoggingPlot(dcont13, ui.qwtPlot5, ui.frame_5, 4);
	logging_plot5->setDataType(LoggingData::NoType, data_containers.at(9));
	plot_map.append(QPair<LoggingData::DataType, int>(LoggingData::NoType, 4));
	ui.cboxLog5->setCurrentIndex(0);
	setAxisPlotTitle(ui.qwtPlot5, QwtPlot::xBottom, " ");	

	logging_plot_list.append(logging_plot1);
	logging_plot_list.append(logging_plot2);
	logging_plot_list.append(logging_plot3);
	logging_plot_list.append(logging_plot4);
	logging_plot_list.append(logging_plot5);
	
	ui.frame_1->setMaximumHeight(32);
	ui.frame_1->setMinimumHeight(32);
	ui.frame_2->setMaximumHeight(32);
	ui.frame_2->setMinimumHeight(32);
	ui.frame_3->setMaximumHeight(32);
	ui.frame_3->setMinimumHeight(32);
	ui.frame_4->setMaximumHeight(32);
	ui.frame_4->setMinimumHeight(32);
	ui.frame_5->setMaximumHeight(32);
	ui.frame_5->setMinimumHeight(32);

	double up_depth = ui.qwtPlot1->axisScaleDiv(QwtPlot::yLeft).upperBound();
	double down_depth = ui.qwtPlot1->axisScaleDiv(QwtPlot::yLeft).lowerBound();
	ui.cntDepthFrom->setValue(int(up_depth));
	ui.cntDepthTo->setValue(int(down_depth));
		
	setConnections();
}
*/

LoggingWidget::~LoggingWidget()
{
	qDeleteAll(logging_plot_list.begin(), logging_plot_list.end());
	qDeleteAll(data_containers.begin(), data_containers.end());
}

void LoggingWidget::setConnections()
{
	connect(ui.cboxLog1, SIGNAL(currentIndexChanged(int)), this, SLOT(setDataType(int)));
	connect(ui.cboxLog2, SIGNAL(currentIndexChanged(int)), this, SLOT(setDataType(int)));
	connect(ui.cboxLog3, SIGNAL(currentIndexChanged(int)), this, SLOT(setDataType(int)));
	connect(ui.cboxLog4, SIGNAL(currentIndexChanged(int)), this, SLOT(setDataType(int)));
	connect(ui.cboxLog5, SIGNAL(currentIndexChanged(int)), this, SLOT(setDataType(int)));

	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		connect(logging_plot, SIGNAL(plot_rescaled(void*)), this, SLOT(rescaleAllDepths(void*)));
		connect(logging_plot->getInversePlotZoomer(), SIGNAL(zoomed(const QRectF&)), this, SLOT(setRezoomAll(const QRectF&)));
	}

	//connect(ui.tbtDapth, SIGNAL(clicked()), this, SLOT(setDepthScale()));
	connect(ui.tbtSearchData, SIGNAL(clicked()), this, SLOT(searchAllData()));
	connect(ui.tbtClearAll, SIGNAL(clicked()), this, SLOT(clearAllData()));

	connect(ui.cntDepthFrom, SIGNAL(valueChanged(double)), this, SLOT(setDepthFrom(double)));
	connect(ui.cntDepthTo, SIGNAL(valueChanged(double)), this, SLOT(setDepthTo(double)));

	connect(ui.tbtScaleIn_1, SIGNAL(clicked()), this, SLOT(scaleDataIn()));
	connect(ui.tbtScaleIn_2, SIGNAL(clicked()), this, SLOT(scaleDataIn()));
	connect(ui.tbtScaleIn_3, SIGNAL(clicked()), this, SLOT(scaleDataIn()));
	connect(ui.tbtScaleIn_4, SIGNAL(clicked()), this, SLOT(scaleDataIn()));
	connect(ui.tbtScaleIn_5, SIGNAL(clicked()), this, SLOT(scaleDataIn()));
	connect(ui.tbtScaleOut_1, SIGNAL(clicked()), this, SLOT(scaleDataOut()));
	connect(ui.tbtScaleOut_2, SIGNAL(clicked()), this, SLOT(scaleDataOut()));
	connect(ui.tbtScaleOut_3, SIGNAL(clicked()), this, SLOT(scaleDataOut()));
	connect(ui.tbtScaleOut_4, SIGNAL(clicked()), this, SLOT(scaleDataOut()));
	connect(ui.tbtScaleOut_5, SIGNAL(clicked()), this, SLOT(scaleDataOut()));

	connect(ui.tbtPlus, SIGNAL(clicked()), this, SLOT(scaleDepthIn()));
	connect(ui.tbtMinus, SIGNAL(clicked()), this, SLOT(scaleDepthOut()));
}

int LoggingWidget::getDataTypeId(QString str, bool *res)
{
	if (res > 0) *res = true;

	if (str == "NMR_CHANNEL") return NMR_CHANNEL;
	else if (str == "GK_CHANNEL") return GK_CHANNEL; 
	else if (str == "SDSP_CHANNEL") return SDSP_CHANNEL;
	else if (str == "AFR_CHANNEL") return AFR_CHANNEL;
	else if (str == "RF_PULSE_CHANNEL") return RF_PULSE_CHANNEL;
	else 
	{ 
		if (res > 0) *res = false; 
		return 0xFF; 
	}
}

void LoggingWidget::setAxisPlotTitle(QwtPlot *qwtPlot, QwtPlot::Axis axisIndex, QString text)
{
	QwtText title;	
	title.setText(text);
	title.setColor(QColor(Qt::darkBlue));
	QFont font = qwtPlot->axisTitle(axisIndex).font();
	font.setBold(false);
	font.setPointSize(10);	
	title.setFont(font);
	
	qwtPlot->setAxisTitle(axisIndex, title);
	qwtPlot->replot();
}

void LoggingWidget::replotLegends()
{
	int maximum_logs = 0;
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		int logs = logging_plot->getQwtCurveList()->size();
		if (logs > maximum_logs) maximum_logs = logs;
	}

	if (maximum_logs <= 1)
	{
		ui.frame_1->setMinimumHeight(32);
		ui.frame_2->setMinimumHeight(32);
		ui.frame_3->setMinimumHeight(32);
		ui.frame_4->setMinimumHeight(32);
		ui.frame_5->setMinimumHeight(32);
		ui.frame_1->setMaximumHeight(32);
		ui.frame_2->setMaximumHeight(32);
		ui.frame_3->setMaximumHeight(32);
		ui.frame_4->setMaximumHeight(32);
		ui.frame_5->setMaximumHeight(32);
	}
	else
	{
		ui.frame_1->setMinimumHeight(maximum_logs*8+32);
		ui.frame_2->setMinimumHeight(maximum_logs*8+32);
		ui.frame_3->setMinimumHeight(maximum_logs*8+32);
		ui.frame_4->setMinimumHeight(maximum_logs*8+32);
		ui.frame_5->setMinimumHeight(maximum_logs*8+32);
		ui.frame_1->setMaximumHeight(maximum_logs*8+32);
		ui.frame_2->setMaximumHeight(maximum_logs*8+32);
		ui.frame_3->setMaximumHeight(maximum_logs*8+32);
		ui.frame_4->setMaximumHeight(maximum_logs*8+32);
		ui.frame_5->setMaximumHeight(maximum_logs*8+32);
	}	
}

bool LoggingWidget::getMinMaxValues(const QVector<double> *vec, double &min_value, double &max_value)
{
	if (vec->isEmpty()) return false;

	min_value = vec->first();
	max_value = vec->first();
	for (int i = 0; i < vec->size(); i++)
	{
		if (vec->at(i) > max_value) max_value = vec->at(i);
		if (vec->at(i) < min_value) min_value = vec->at(i);
	}

	return true;
}

void LoggingWidget::searchAllData()
{
	double upper_values = 10000;
	double downer_values = -10;
	
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = logging_plot->getQwtPlot();
		if (logging_plot->getDataType() != LoggingData::NoType)
		{
			for (int j = 0; j < logging_plot->getDataXList()->size(); j++)
			{
				QVector<double> *depth_data = logging_plot->getDataXList()->at(j);
				double up_depth = 0;
				double down_depth = 10000;
				if (getMinMaxValues(depth_data, up_depth, down_depth))
				{
					if (up_depth < upper_values) upper_values = up_depth;
					if (down_depth > downer_values) downer_values = down_depth;
				}
			}			
		}
	}

	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = logging_plot->getQwtPlot();
		
		qwtPlot->setAxisScale(QwtPlot::yLeft, downer_values+10, upper_values-10);
		qwtPlot->setAxisScale(QwtPlot::yRight, downer_values+10, upper_values-10);

		qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
		qwtPlot->setAxisAutoScale(QwtPlot::xTop, true);

		qwtPlot->replot();

		ui.cntDepthFrom->setValue(int(upper_values-10));
		ui.cntDepthTo->setValue(int(downer_values+10));
	}
}

void LoggingWidget::clearAllData()
{	
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = qwtplot_list.at(i);
		logging_plot->clearAll();

		qwtPlot->replot();
	}	
	
	for (int i = 0; i < data_containers.count(); i++)
	{
		LoggingData *logging_data = data_containers.at(i);
		logging_data->log_type = LoggingData::NoType;

		ToolData *depth_data = logging_data->depth_data;
		ToolData *data = logging_data->logging_data;
		
		qDeleteAll(depth_data->begin(), depth_data->end());
		qDeleteAll(data->begin(), data->end());

		depth_data->clear();
		data->clear();
	}
}

void LoggingWidget::setDepthFrom(double from)
{
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = logging_plot->getQwtPlot();

		double down_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();		

		qwtPlot->setAxisScale(QwtPlot::yLeft, down_depth, from);
		qwtPlot->setAxisScale(QwtPlot::yRight, down_depth, from);

		qwtPlot->replot();
	}
}

void LoggingWidget::setDepthTo(double to)
{
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = logging_plot->getQwtPlot();

		double up_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();		

		qwtPlot->setAxisScale(QwtPlot::yLeft, to, up_depth);
		qwtPlot->setAxisScale(QwtPlot::yRight, to, up_depth);

		qwtPlot->replot();
	}
}

void LoggingWidget::scaleDataIn()
{
	QToolButton *tbt = (QToolButton*)sender();
	LoggingPlot *logging_plot = NULL;
	if (tbt == ui.tbtScaleIn_1) logging_plot = logging_plot_list.at(0);
	else if (tbt == ui.tbtScaleIn_2) logging_plot = logging_plot_list.at(1);
	else if (tbt == ui.tbtScaleIn_3) logging_plot = logging_plot_list.at(2);
	else if (tbt == ui.tbtScaleIn_4) logging_plot = logging_plot_list.at(3);
	else if (tbt == ui.tbtScaleIn_5) logging_plot = logging_plot_list.at(4);
	else return;

	QwtPlot *qwtPlot = logging_plot->getQwtPlot();

	double cur_min = qwtPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
	double cur_max = qwtPlot->axisScaleDiv(QwtPlot::xBottom).upperBound();
	double delta = cur_max - cur_min;
	delta *= 0.5;//0.80;

	double new_cur_max = (cur_max+cur_min)/2 + delta/2;
	double new_cur_min = (cur_max+cur_min)/2 - delta/2;
		
	qwtPlot->setAxisScale(QwtPlot::xBottom, new_cur_min, new_cur_max);
	qwtPlot->setAxisScale(QwtPlot::xTop, new_cur_min, new_cur_max);

	qwtPlot->replot();
}

void LoggingWidget::scaleDataOut()
{
	QToolButton *tbt = (QToolButton*)sender();
	LoggingPlot *logging_plot = NULL;
	if (tbt == ui.tbtScaleOut_1) logging_plot = logging_plot_list.at(0);
	else if (tbt == ui.tbtScaleOut_2) logging_plot = logging_plot_list.at(1);
	else if (tbt == ui.tbtScaleOut_3) logging_plot = logging_plot_list.at(2);
	else if (tbt == ui.tbtScaleOut_4) logging_plot = logging_plot_list.at(3);
	else if (tbt == ui.tbtScaleOut_5) logging_plot = logging_plot_list.at(4);
	else return;

	QwtPlot *qwtPlot = logging_plot->getQwtPlot();

	double cur_min = qwtPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
	double cur_max = qwtPlot->axisScaleDiv(QwtPlot::xBottom).upperBound();
	double delta = cur_max - cur_min;
	delta *= 2.0; //1.25;

	double new_cur_max = (cur_max+cur_min)/2 + delta/2;
	double new_cur_min = (cur_max+cur_min)/2 - delta/2;

	qwtPlot->setAxisScale(QwtPlot::xBottom, new_cur_min, new_cur_max);
	qwtPlot->setAxisScale(QwtPlot::xTop, new_cur_min, new_cur_max);

	qwtPlot->replot();
}

void LoggingWidget::scaleDepthIn()
{
	LoggingPlot *logging_plot = logging_plot_list.at(0);
	QwtPlot *qwtPlot = logging_plot->getQwtPlot();

	double down_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();	
	double up_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();	
	double delta = fabs(down_depth - up_depth);
	delta *= 0.80;

	double new_down_depth = (down_depth+up_depth)/2 + delta/2;
	double new_up_depth = (down_depth+up_depth)/2 - delta/2;

	if ((new_down_depth < -100) || (new_up_depth < -100)) return; 

	ui.cntDepthFrom->setValue(int(new_up_depth));
	ui.cntDepthTo->setValue(int(new_down_depth));

	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = logging_plot->getQwtPlot();

		double down_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();		

		qwtPlot->setAxisScale(QwtPlot::yLeft, new_down_depth, new_up_depth);
		qwtPlot->setAxisScale(QwtPlot::yRight, new_down_depth, new_up_depth);

		qwtPlot->replot();
	}	
}

void LoggingWidget::scaleDepthOut()
{
	LoggingPlot *logging_plot = logging_plot_list.at(0);
	QwtPlot *qwtPlot = logging_plot->getQwtPlot();

	double down_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();	
	double up_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();	
	double delta = fabs(down_depth - up_depth);
	delta *= 1.25;

	double new_down_depth = (down_depth+up_depth)/2 + delta/2;
	double new_up_depth = (down_depth+up_depth)/2 - delta/2;

	if ((new_down_depth < -100) || (new_up_depth < -100)) return; 

	ui.cntDepthFrom->setValue(int(new_up_depth));
	ui.cntDepthTo->setValue(int(new_down_depth));

	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwtPlot = logging_plot->getQwtPlot();

		double down_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();		

		qwtPlot->setAxisScale(QwtPlot::yLeft, new_down_depth, new_up_depth);
		qwtPlot->setAxisScale(QwtPlot::yRight, new_down_depth, new_up_depth);

		qwtPlot->replot();
	}	
}

void LoggingWidget::addDataSets(DataSets _dss)
{
	for (int i = 0; i < _dss.count(); i++)
	{
		DataSet *ds = _dss[i];
		
		LoggingData::DataType dt = LoggingData::NoType;
		uint8_t comm_id = ds->getDataCode();
		switch (comm_id)
		{
		case DT_SGN_RELAX:	dt = LoggingData::NMRIntegral_Probe1; break; 
		case DT_SGN_RELAX2:	dt = LoggingData::NMRIntegral_Probe2; break;
		case DT_SGN_RELAX3:	dt = LoggingData::NMRIntegral_Probe3; break;
		case DT_SGN_SPEC1:	dt = LoggingData::NMRBins_Probe1; break; 
		case DT_SGN_SPEC2:	dt = LoggingData::NMRBins_Probe2; break;
		case DT_SGN_SPEC3:	dt = LoggingData::NMRBins_Probe3; break;
		case DT_GAMMA:		dt = LoggingData::Gamma; break;
		case DT_DIEL:		dt = LoggingData::WaveDielectric; break;
		case DT_AFR1_RX:	dt = LoggingData::AFR_Probe1; break;
		case DT_AFR2_RX:	dt = LoggingData::AFR_Probe2; break;
		case DT_AFR3_RX:	dt = LoggingData::AFR_Probe3; break;
		default: break;
		}
		if (dt == LoggingData::NoType) continue;
		
		// ������� ������: ������ ���������������� ��������, ������� � ����� DataSet �������� ����������� ������
		if (dt == LoggingData::WaveDielectric)
		{			
			for (int k = 0; k < plot_map.count(); k++)			
			{
				if (plot_map[k].first == LoggingData::WaveDielectric_PhaseDiff) 
				{
					int qwtplot_index = plot_map[k].second;
					//QwtPlot *qwtPlot = NULL;
					for (int j = 0; j < logging_plot_list.count(); j++) 
					{
						LoggingPlot *logging_plot = logging_plot_list.at(j);
						if (logging_plot->getQwtPlotIndex() == qwtplot_index)
						{
							logging_plot->addDataSet(ds, LoggingData::WaveDielectric_PhaseDiff);
							replotLegends();
						}
					}
				}
			}
			
			for (int k = 0; k < plot_map.count(); k++)	
			{
				if (plot_map[k].first == LoggingData::WaveDielectric_AmplRatio) 
				{
					int qwtplot_index = plot_map[k].second;
					//QwtPlot *qwtPlot = NULL;
					for (int j = 0; j < logging_plot_list.count(); j++) 
					{
						LoggingPlot *logging_plot = logging_plot_list.at(j);
						if (logging_plot->getQwtPlotIndex() == qwtplot_index)
						{
							logging_plot->addDataSet(ds, LoggingData::WaveDielectric_AmplRatio);
							replotLegends();
						}
					}
				}				
			}
		}
		// ��� ������ ������
		else  
		{
			for (int k = 0; k < plot_map.count(); k++)	
			{
				if (plot_map[k].first == dt) 
				{
					int qwtplot_index = plot_map[k].second;
					//QwtPlot *qwtPlot = NULL;
					for (int j = 0; j < logging_plot_list.count(); j++) 
					{
						LoggingPlot *logging_plot = logging_plot_list.at(j);
						if (logging_plot->getQwtPlotIndex() == qwtplot_index)
						{
							logging_plot->addDataSet(ds, dt);
							replotLegends();
						}
					}
				}							
			}
		}		
	}	
	
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		logging_plot->closeLoggingCurveList();
	}
}

void LoggingWidget::setDataType(int index)
{
	QComboBox *cbox = (QComboBox*)sender();
	if (!cbox) return;

	int logging_plot_index = -1;
	if (cbox == ui.cboxLog1) logging_plot_index = 0;
	if (cbox == ui.cboxLog2) logging_plot_index = 1;
	if (cbox == ui.cboxLog3) logging_plot_index = 2;
	if (cbox == ui.cboxLog4) logging_plot_index = 3;
	if (cbox == ui.cboxLog5) logging_plot_index = 4;
	if (logging_plot_index < 0) return;

	LoggingPlot *logging_plot = logging_plot_list.at(logging_plot_index);
	QwtPlot *qwtPlot = qwtplot_list.at(logging_plot_index);
	logging_plot->clearAll();
		
	QString txt = cbox->currentText(); //cbox->itemText(index); 
	if (txt == tr("NMR Integral (Probe#1)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Integral value, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NMRIntegral_Probe1, logging_plot_index));
		logging_plot->setDataType(LoggingData::NMRIntegral_Probe1, data_containers.at(0));
	}
	if (txt == tr("NMR Integral (Probe#2)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Integral value, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NMRIntegral_Probe2, logging_plot_index));
		logging_plot->setDataType(LoggingData::NMRIntegral_Probe2, data_containers.at(1));
	}
	if (txt == tr("NMR Integral (Probe#3)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Integral value, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NMRIntegral_Probe3, logging_plot_index));
		logging_plot->setDataType(LoggingData::NMRIntegral_Probe3, data_containers.at(2));
	}
	if (txt == tr("NMR Bins (Probe#1)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Integral value, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NMRBins_Probe1, logging_plot_index));
		logging_plot->setDataType(LoggingData::NMRBins_Probe1, data_containers.at(3));
	}
	if (txt == tr("NMR Bins (Probe#2)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Integral value, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NMRBins_Probe2, logging_plot_index));
		logging_plot->setDataType(LoggingData::NMRBins_Probe2, data_containers.at(4));
	}
	if (txt == tr("NMR Bins (Probe#3)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Integral value, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NMRBins_Probe3, logging_plot_index));
		logging_plot->setDataType(LoggingData::NMRBins_Probe3, data_containers.at(5));
	}
	else if (txt == tr("Gamma Logging")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Counts per second, 1/sec"));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::Gamma, logging_plot_index));
		logging_plot->setDataType(LoggingData::Gamma, data_containers.at(6));
	}
	else if (txt == tr("Wave Dielectric Logging (Phase difference)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Phase difference, degree"));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::WaveDielectric_PhaseDiff, logging_plot_index));
		logging_plot->setDataType(LoggingData::WaveDielectric_PhaseDiff, data_containers.at(7));
	}
	else if (txt == tr("Wave Dielectric Logging (Amplitude ratio)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Amplitude ratio, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::WaveDielectric_AmplRatio, logging_plot_index));
		logging_plot->setDataType(LoggingData::WaveDielectric_AmplRatio, data_containers.at(8));
	}
	else if (txt == tr("Maximum of AFR (Probe#1)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Maximum of AFR, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::AFR_Probe1, logging_plot_index));
		logging_plot->setDataType(LoggingData::AFR_Probe1, data_containers.at(9));
	}
	else if (txt == tr("Maximum of AFR (Probe#2)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Maximum of AFR, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::AFR_Probe2, logging_plot_index));
		logging_plot->setDataType(LoggingData::AFR_Probe2, data_containers.at(10));
	}
	else if (txt == tr("Maximum of AFR (Probe#3)")) 
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, tr("Maximum of AFR, a.u."));
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::AFR_Probe3, logging_plot_index));
		logging_plot->setDataType(LoggingData::AFR_Probe3, data_containers.at(11));
	}
	else if (txt == tr("No Data"))
	{
		setAxisPlotTitle(qwtPlot, QwtPlot::xBottom, " ");
		plot_map.replace(logging_plot_index, QPair<LoggingData::DataType, int>(LoggingData::NoType, logging_plot_index));
		logging_plot->setDataType(LoggingData::NoType, data_containers.at(12));
	}
}

void LoggingWidget::rescaleAllDepths(void *qwtplot_obj)
{
	QwtPlot *qwtPlot = (QwtPlot*)qwtplot_obj;
	double cur_min_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
	double cur_max_depth = qwtPlot->axisScaleDiv(QwtPlot::yLeft).upperBound();

	double cur_min_val = qwtPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
	double cur_max_val = qwtPlot->axisScaleDiv(QwtPlot::xBottom).upperBound();
	double step_val = qwtPlot->axisStepSize(QwtPlot::xBottom);

	int tick_count = qwtPlot->axisMaxMajor(QwtPlot::xBottom);
	step_val = (cur_max_val - cur_min_val)/(tick_count+1);
	
	//const QList<double> ticks_major = qwtPlot->axisScaleDiv(QwtPlot::xBottom).ticks(QwtScaleDiv::MajorTick);
	//const QList<double> ticks_minor = qwtPlot->axisScaleDiv(QwtPlot::xBottom).ticks(QwtScaleDiv::MinorTick);

	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwt_plot = logging_plot->getQwtPlot();
		
		qwt_plot->setAxisScale(QwtPlot::yLeft, cur_min_depth, cur_max_depth);
		qwt_plot->setAxisScale(QwtPlot::yRight, cur_min_depth, cur_max_depth);

		qwt_plot->setAxisScale(QwtPlot::xTop, cur_min_val, cur_max_val, step_val);	
		
		/*QwtScaleDiv scale_div = qwt_plot->axisScaleDiv(QwtPlot::xTop);
		scale_div.setTicks(QwtScaleDiv::MajorTick, ticks_major);
		scale_div.setTicks(QwtScaleDiv::MinorTick, ticks_minor);
		qwt_plot->setAxisScaleDiv(QwtPlot::xTop, scale_div);*/
		
		qwtPlot->replot();		

		ui.cntDepthFrom->setValue(int(cur_max_depth));
		ui.cntDepthTo->setValue(int(cur_min_depth));
	}
}


void LoggingWidget::setRezoomAll(const QRectF &rect)
{
	for (int i = 0; i < logging_plot_list.count(); i++)
	{
		LoggingPlot *logging_plot = logging_plot_list.at(i);
		QwtPlot *qwt_plot = logging_plot->getQwtPlot();

		double min_depth = rect.y();
		double max_depth = min_depth + rect.height();
		qwt_plot->setAxisScale(QwtPlot::yLeft, max_depth, min_depth);
		qwt_plot->setAxisScale(QwtPlot::yRight, max_depth, min_depth);

		qwt_plot->replot();

		ui.cntDepthFrom->setValue(int(min_depth));
		ui.cntDepthTo->setValue(int(max_depth));
	}
}

bool LoggingWidget::isVisibleChannel(ToolChannel *channel, uint8_t comm_id)
{
	bool res = false;

	QString str = channel->data_type;
	if (str == "NMR_CHANNEL")
	{
		if (comm_id == 0) return true;

		switch (comm_id)
		{
		case DT_SGN_SPEC1:
		case DT_SGN_SPEC2:
		case DT_SGN_SPEC3:	return true; 
		}
	}
	else if (str == "GK_CHANNEL") return true; 
	else if (str == "SDSP_CHANNEL") return true;
	else if (str == "AFR_CHANNEL") return true;
	else if (str == "RF_PULSE_CHANNEL") return true;
	
	return false;
}



LoggingPlot::LoggingPlot(LoggingData *log_container, QwtPlot *qwt_plot, QFrame *frame, int index, QWidget *parent) 
{ 
	qwtPlot = qwt_plot; 
	legend_frame = frame;
	qwt_plot_index = index;
	
	qwtPlot->setCanvasBackground(QBrush(QColor(Qt::white)));

	legend_vlout = new QVBoxLayout(legend_frame);
	legend_vlout->setContentsMargins(1, 1, 1, 1);
	legend_vlout->setSpacing(2);

	QwtPlotGrid *grid = new QwtPlotGrid;
	grid->setMajorPen(QPen(QBrush(QColor(Qt::gray)), 1, Qt::DashLine));	
	grid->enableX(true);	
	grid->enableY(true);	
	grid->attach(qwtPlot);	

	QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, qwtPlot->canvas());	
	picker->setRubberBandPen(QColor(Qt::green));
	picker->setRubberBand(QwtPicker::CrossRubberBand);
	picker->setTrackerPen(QColor(Qt::darkMagenta));

	QwtPlotPanner *panner = new QwtPlotPanner(qwtPlot->canvas());
	panner->setAxisEnabled(QwtPlot::yRight, false);
	panner->setMouseButton(Qt::RightButton);

	zoomer = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, qwtPlot->canvas());
	zoomer->setEnabled(true);
	const QColor c_Rx(Qt::magenta);
	zoomer->setRubberBandPen(c_Rx);
	zoomer->setTrackerPen(c_Rx);
	
	qwt_plot->setAxisScale(QwtPlot::yLeft, 1000, 0, 200);
	qwt_plot->setAxisScale(QwtPlot::yRight, 1000, 0, 200);
	
	qwt_plot->setAxisMaxMajor(QwtPlot::xBottom, 5);
	qwt_plot->setAxisMaxMajor(QwtPlot::xTop, 5);

	//qwt_plot->setAxisAutoScale(QwtPlot::xBottom, false);
	qwt_plot->setAxisAutoScale(QwtPlot::yLeft, false);

	color_list << QColor(Qt::red) << QColor(Qt::blue) << QColor(Qt::magenta) << QColor(Qt::green);
	color_list << QColor(Qt::darkYellow) << QColor(Qt::darkRed) << QColor(Qt::darkBlue) << QColor(Qt::darkGreen);
	color_list << QColor(Qt::darkMagenta) << QColor(Qt::darkCyan);

	xdata_list = log_container->depth_data;
	ydata_list = log_container->logging_data;

	closeLoggingCurveList();
}

LoggingPlot::~LoggingPlot()
{
	for (int i = 0; i < qwt_curve_list.count(); i++)
	{
		QwtPlotCurve *qwt_curve = qwt_curve_list.at(i);
		qwt_curve->detach();
		delete qwt_curve;
	}

	//qDeleteAll(xdata_list.begin(), xdata_list.end());
	//qDeleteAll(ydata_list.begin(), ydata_list.end());
	//xdata_list.clear();
	//ydata_list.clear();
}

void LoggingPlot::setDataType(LoggingData::DataType dt, LoggingData *dcont)
{
	xdata_list = dcont->depth_data;
	ydata_list = dcont->logging_data;
	data_type = dt;
}

void LoggingPlot::addDataSet(DataSet *ds, LoggingData::DataType dt)
{	
	//static double dpt = ds->getDepth().second;
	QPair<bool,double> depth = ds->getDepth(); //QPair<bool, double>(true, dpt);
	
	if (ds->getYData()->isEmpty()) return;
	
	data_type = dt;

	ProcessingRelax processing_relax;
	QVector<ToolChannel*> current_tool_channels;
	foreach (QWidget *win, QApplication::topLevelWidgets())
	{
		if (MainWindow *mainWin = qobject_cast<MainWindow *>(win)) 
		{
			processing_relax = mainWin->getProcessingRelax();
			current_tool_channels = mainWin->getCurrentToolChannels();
		}
	}

	double S = 0;
	switch (dt)
	{
	case LoggingData::NMRBins_Probe1:
	case LoggingData::NMRBins_Probe2:
	case LoggingData::NMRBins_Probe3:
		{
			//static int dataset_probe1_counter = 1;
						
			int cur_log = cur_index+1;

			/*ProcessingRelax processing_relax;
			QVector<ToolChannel*> current_tool_info;
			foreach (QWidget *win, QApplication::topLevelWidgets())
			{
				if (MainWindow *mainWin = qobject_cast<MainWindow *>(win)) 
				{
					processing_relax = mainWin->getProcessingRelax();
					current_tool_info = mainWin->getCurrentToolChannels();
				}
			}*/
			double T2_min = processing_relax.T2_min;			
			double T2_cutoff_clay = processing_relax.T2_cutoff_clay;
			double T2_cutoff = processing_relax.T2_cutoff;
			double T2_max = processing_relax.T2_max;

			double mcbw = 0;
			double mbvi = 0;
			double mffi = 0;
			for (int i = 0; i < ds->getXData()->size(); i++)
			{
				double x = ds->getXData()->at(i);
				double y = ds->getYData()->at(i);

				if (T2_min <= x && x < T2_cutoff_clay) mcbw += y;
				else if (T2_cutoff_clay <= x && x < T2_cutoff) mbvi += y;
				else if (T2_cutoff <= x && x <= T2_max) mffi += y;
			}
			double mphi = mbvi + mffi;
			double mphs = mffi + mcbw + mbvi;

			if (processing_relax.porosity_on)
			{
				ToolChannel *channel = NULL;
				for (int j = 0; j < current_tool_channels.count(); j++)
				{
					if (dt == LoggingData::NMRBins_Probe1) 
					{
						if (current_tool_channels[j]->name == "NMR_CHANNEL_1") channel = current_tool_channels[j];
					}
					else if (dt == LoggingData::NMRBins_Probe2) 
					{
						if (current_tool_channels[j]->name == "NMR_CHANNEL_2") channel = current_tool_channels[j];
					}
					else if (dt == LoggingData::NMRBins_Probe3) 
					{
						if (current_tool_channels[j]->name == "NMR_CHANNEL_3") channel = current_tool_channels[j];
					}					
				}
				if (channel != NULL)
				{
					mphs = mphs * channel->normalize_coef1 * channel->normalize_coef2;
					mffi = mffi * channel->normalize_coef1 * channel->normalize_coef2;
					mphi = mphi * channel->normalize_coef1 * channel->normalize_coef2;
				}
			}

			// -------------- MPHS -------------------
			cur_index++;

			bool new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "_MPHS#" + QString::number(cur_log+1);
				addNewLog(curve_name, LoggingData::MPHS);
				new_log = true;
			}

			QVector<double> *x_mphs = xdata_list->at(cur_index);
			QVector<double> *y_mphs = ydata_list->at(cur_index);
			QwtPlotCurve *curve_mphs = qwt_curve_list.at(cur_index);
			x_mphs->push_back(depth.second);
			y_mphs->push_back(mphs);

			if (!new_log) curve_mphs->detach();
			curve_mphs->setSamples(y_mphs->data(), x_mphs->data(), y_mphs->size());
			curve_mphs->attach(qwtPlot);
			// --------------------------------------

			// -------------- MPHI -------------------
			cur_index++;

			new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "_MPHI#" + QString::number(cur_log+1);
				addNewLog(curve_name, LoggingData::MPHI);
				new_log = true;
			}

			QVector<double> *x_mphi = xdata_list->at(cur_index);
			QVector<double> *y_mphi = ydata_list->at(cur_index);
			QwtPlotCurve *curve_mphi = qwt_curve_list.at(cur_index);
			x_mphi->push_back(depth.second);
			y_mphi->push_back(mphi);

			if (!new_log) curve_mphi->detach();
			curve_mphi->setSamples(y_mphi->data(), x_mphi->data(), y_mphi->size());
			curve_mphi->attach(qwtPlot);
			// --------------------------------------
									
			// -------------- MFFI -------------------
			cur_index++;

			new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "_MFFI#" + QString::number(cur_log+1);
				addNewLog(curve_name, LoggingData::MFFI);
				new_log = true;
			}

			QVector<double> *x_mffi = xdata_list->at(cur_index);
			QVector<double> *y_mffi = ydata_list->at(cur_index);
			QwtPlotCurve *curve_mffi = qwt_curve_list.at(cur_index);
			x_mffi->push_back(depth.second);
			y_mffi->push_back(mffi);

			if (!new_log) curve_mffi->detach();
			curve_mffi->setSamples(y_mffi->data(), x_mffi->data(), y_mffi->size());
			curve_mffi->attach(qwtPlot);
			// --------------------------------------

			qwtPlot->replot();

			void *qwtplot_obj = (void*)qwtPlot;
			emit plot_rescaled(qwtplot_obj);

			break;
		}
	/*case LoggingData::NMRBins_Probe2:
		{
			static int dataset_probe2_counter = 1;
			break;
		}
	case LoggingData::NMRBins_Probe3:
		{
			static int dataset_probe3_counter = 1;
			break;
		}*/

	case LoggingData::NMRIntegral_Probe1:
	case LoggingData::NMRIntegral_Probe2:
	case LoggingData::NMRIntegral_Probe3:
		{
			cur_index++;
			
			bool new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}
			
			for (int i = 0; i < ds->getYData()->size(); i++) S += ds->getYData()->at(i);
			
			QVector<double> *x = xdata_list->at(cur_index);
			QVector<double> *y = ydata_list->at(cur_index);
			QwtPlotCurve *curve = qwt_curve_list.at(cur_index);
			x->push_back(depth.second);
			y->push_back(S);
					
			if (!new_log) curve->detach();
			curve->setSamples(y->data(), x->data(), y->size());
			curve->attach(qwtPlot);

			qwtPlot->replot();

			void *qwtplot_obj = (void*)qwtPlot;
			emit plot_rescaled(qwtplot_obj);

			break;
		}
	case LoggingData::Gamma:
		{
			cur_index++;

			bool new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}

			S = ds->getYData()->first();

			if (processing_relax.porosity_on)
			{
				ToolChannel *channel = NULL;
				for (int j = 0; j < current_tool_channels.count(); j++)
				{
					if (dt == LoggingData::Gamma) 
					{
						if (current_tool_channels[j]->name == "GK_CHANNEL") channel = current_tool_channels[j];
					}									
				}
				if (channel != NULL)
				{
					S = S * channel->normalize_coef1 * channel->normalize_coef2;					
				}
			}

			QVector<double> *x = xdata_list->at(cur_index);
			QVector<double> *y = ydata_list->at(cur_index);
			QwtPlotCurve *curve = qwt_curve_list.at(cur_index);
			x->push_back(depth.second);
			y->push_back(S);
			
			if (!new_log) curve->detach();
			curve->setSamples(y->data(), x->data(), y->size());
			curve->attach(qwtPlot);

			qwtPlot->replot();

			void *qwtplot_obj = (void*)qwtPlot;
			emit plot_rescaled(qwtplot_obj);

			break;
		}
	case LoggingData::WaveDielectric_PhaseDiff:
		{
			if (ds->getYData()->size() < 4) return;

			cur_index++;

			// Frequency 60 MHz
			bool new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}

			S = ds->getYData()->at(0);

			QVector<double> *x1 = xdata_list->at(cur_index);
			QVector<double> *y1 = ydata_list->at(cur_index);
			QwtPlotCurve *curve1 = qwt_curve_list.at(cur_index);
			x1->push_back(depth.second);
			y1->push_back(S);
						
			if (!new_log) curve1->detach();
			curve1->setSamples(y1->data(), x1->data(), y1->size());
			curve1->attach(qwtPlot);			
			// -------------------

			cur_index++;

			// Frequency 32 MHz
			new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}

			S = ds->getYData()->at(2);

			QVector<double> *x2 = xdata_list->at(cur_index);
			QVector<double> *y2 = ydata_list->at(cur_index);
			QwtPlotCurve *curve2 = qwt_curve_list.at(cur_index);
			x2->push_back(depth.second);
			y2->push_back(S);

			if (!new_log) curve2->detach();
			curve2->setSamples(y2->data(), x2->data(), y2->size());
			curve2->attach(qwtPlot);			
			// -------------------
			
			qwtPlot->replot();
												
			void *qwtplot_obj = (void*)qwtPlot;
			emit plot_rescaled(qwtplot_obj);

			break;
		}
	case LoggingData::WaveDielectric_AmplRatio:
		{
			if (ds->getYData()->size() < 4) return;
			
			cur_index++;

			// Frequency 60 MHz
			bool new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}

			S = ds->getYData()->at(1);

			QVector<double> *x1 = xdata_list->at(cur_index);
			QVector<double> *y1 = ydata_list->at(cur_index);
			QwtPlotCurve *curve1 = qwt_curve_list.at(cur_index);
			x1->push_back(depth.second);
			y1->push_back(S);
			
			if (!new_log) curve1->detach();
			curve1->setSamples(y1->data(), x1->data(), y1->size());
			curve1->attach(qwtPlot);			
			// -------------------

			cur_index++;

			// Frequency 32 MHz
			new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}

			S = ds->getYData()->at(3);

			QVector<double> *x2 = xdata_list->at(cur_index);
			QVector<double> *y2 = ydata_list->at(cur_index);
			QwtPlotCurve *curve2 = qwt_curve_list.at(cur_index);
			x2->push_back(depth.second);
			y2->push_back(S);

			if (!new_log) curve2->detach();
			curve2->setSamples(y2->data(), x2->data(), y2->size());
			curve2->attach(qwtPlot);			
			// -------------------
			
			qwtPlot->replot();

			void *qwtplot_obj = (void*)qwtPlot;
			emit plot_rescaled(qwtplot_obj);
			
			break;
		}
	case LoggingData::AFR_Probe1:
	case LoggingData::AFR_Probe2:
	case LoggingData::AFR_Probe3:
		{
			cur_index++;
			bool new_log = false;
			if (cur_index >= qwt_curve_list.count()) 
			{				
				QString curve_name = ds->getDataName() + "#" + QString::number(cur_index+1);
				addNewLog(curve_name);
				new_log = true;
			}
						
			S = ds->getYData()->first();
			for (int i = 1; i < ds->getYData()->size(); i++)
			{
				if (S < ds->getYData()->at(i)) S = ds->getYData()->at(i);
			}

			QVector<double> *x = xdata_list->at(cur_index);
			QVector<double> *y = ydata_list->at(cur_index);
			QwtPlotCurve *curve = qwt_curve_list.at(cur_index);
			x->push_back(depth.second);
			y->push_back(S);

			if (!new_log) curve->detach();
			curve->setSamples(y->data(), x->data(), y->size());
			curve->attach(qwtPlot);

			qwtPlot->replot();
			
			void *qwtplot_obj = (void*)qwtPlot;
			emit plot_rescaled(qwtplot_obj);

			break;
		}
	default: return;
	}	
}

void LoggingPlot::addNewLog(QString log_name, LoggingData::BinType dt)
{
	QwtPlotCurve *curve = new QwtPlotCurve(log_name);
	setCurveSettings(curve, dt);

	QVector<double> *xdata = new QVector<double>;
	QVector<double> *ydata = new QVector<double>;
	xdata->reserve(MAX_LOGGING_DATA);
	ydata->reserve(MAX_LOGGING_DATA);

	qwt_curve_list.push_back(curve);
	xdata_list->push_back(xdata);
	ydata_list->push_back(ydata);	

	QHBoxLayout *hout = new QHBoxLayout;
	hout->setSpacing(3);
	hout->setContentsMargins(0, 0, 0, 0);
		
	QwtCurveSettings c_settings;
	switch (dt)
	{
	case LoggingData::MCBW:
	case LoggingData::MBVI:
	case LoggingData::MFFI:
	case LoggingData::MPHI:
	case LoggingData::MPHS:
		{
			c_settings.dataType = QwtCurveSettings::DataType::Lines;
			QPen curve_pen = curve->pen();
			c_settings.pen = curve_pen;
			PaintFrame_forLogging *pframe = new PaintFrame_forLogging(&c_settings);	
			hout->addWidget(pframe);
			break;
		}
	default:
		{
			c_settings.dataType = QwtCurveSettings::DataType::LinesAndSymbols;
			QPen curve_pen = curve->pen();
			c_settings.pen = curve_pen;
			QwtSymbol *sym = c_settings.symbol;
			sym->setBrush(curve->symbol()->brush());
			sym->setPen(curve->symbol()->pen());
			PaintFrame_forLogging *pframe = new PaintFrame_forLogging(&c_settings);	
			hout->addWidget(pframe);
			break;
		}
	}	

	QLabel *label = new QLabel;	
	label->setText(log_name);
	QFont label_font = label->font();
	label_font.setPointSize(8);
	label->setFont(label_font);
	QPalette label_palette = label->palette();
	label_palette.setColor(QPalette::WindowText, curve->pen().color());
	label_palette.setColor(QPalette::Background, Qt::white);
	label->setAutoFillBackground(true);
	label->setPalette(label_palette);

	hout->addWidget(label);

	legend_vlout->addLayout(hout);
	legend_frame->repaint();
}

void LoggingPlot::clearAll()
{
	for (int i = 0; i < qwt_curve_list.count(); i++)
	{
		QwtPlotCurve *curve = qwt_curve_list.at(i);
		curve->detach();
	}
	qDeleteAll(qwt_curve_list.begin(), qwt_curve_list.end());
	qwt_curve_list.clear();

	/*qDeleteAll(xdata_list->begin(), xdata_list->end());
	qDeleteAll(ydata_list->begin(), ydata_list->end());
	xdata_list->clear();
	ydata_list->clear();*/
		
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

	legend_frame->repaint();

	cur_index = -1;
}

void LoggingPlot::setCurveSettings(QwtPlotCurve *curve, LoggingData::BinType dt)
{
	if (cur_index < 0) return;

	if (dt == LoggingData::None)
	{
		int color_index = cur_index % color_list.count();
		QColor color = color_list.at(color_index);

		curve->setRenderHint(QwtPlotItem::RenderAntialiased);	

		curve->setStyle(QwtPlotCurve::Lines);

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
	}
	else
	{
		ProcessingRelax processing_relax;
		foreach (QWidget *win, QApplication::topLevelWidgets())
		{
			if (MainWindow *mainWin = qobject_cast<MainWindow *>(win)) 
			{
				processing_relax = mainWin->getProcessingRelax();
			}
		}

		QColor color = QColor(Qt::red);
		switch (dt)
		{
		case LoggingData::MPHS:
		case LoggingData::MCBW: color = processing_relax.MCBWcolor; break;
		case LoggingData::MPHI:
		case LoggingData::MBVI: color = processing_relax.MBVIcolor; break;
		case LoggingData::MFFI: color = processing_relax.MFFIcolor; break;
		}
		
		curve->setRenderHint(QwtPlotItem::RenderAntialiased);	
		curve->setStyle(QwtPlotCurve::Lines);

		QColor pen_color(color);
		QPen curve_pen = QPen(Qt::SolidLine);
		curve_pen.setWidth(2);
		curve_pen.setColor(pen_color);
		curve->setPen(curve_pen);

		QColor brush_color(color);
		QBrush curve_brush(Qt::SolidPattern);
		curve_brush.setColor(color);
		curve->setOrientation(Qt::Horizontal);
		curve->setBrush(curve_brush);		
	}	
}

