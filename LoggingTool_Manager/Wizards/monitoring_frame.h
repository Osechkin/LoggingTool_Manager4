#ifndef MONITORING_FRAME_H
#define MONITORING_FRAME_H

//#include <QTextEdit>

#include <QWidget>
#include <QMouseEvent>

//#include "monitoring_wizard_exp1.h"
#include "../io_general.h"
#include "../tools_general.h"
#include "../GraphClasses/graph_classes.h"

#include "qwt_plot_picker.h"
#include "qwt_plot_grid.h"

#include "ui_monitoring_frame.h"



class PaintFrame_forMonitoring : public PaintFrame
{
	Q_OBJECT

public:
	PaintFrame_forMonitoring(QWidget *parent = 0);
	PaintFrame_forMonitoring(QwtCurveSettings *c_settings, QWidget *parent = 0);
	//~PaintFrame_forMonitoring();

private:
	QwtCurveSettings *curve_settings;
	int frame_height;
	int frame_width;
	int symbol_size;

private:
	void drawDataSymbol(QPainter *painter, QwtSymbol::Style symType);

protected:
	void paintEvent(QPaintEvent *);
};


typedef QList<QVector<double>* >	ToolData;


/*enum MonitoringDataType { 
	Temperature,				// темпеартуры					
	Volatage,					// напряжения
	Current,					// токи
	NoType						// данные неизвестного типа
};*/

/*
struct MonitoringData
{
	enum DataType { 
		Temperature,				// темпеартуры					
		Volatage,					// напряжения
		Current,					// токи
		NoType						// данные неизвестного типа
	};


	MonitoringData(DataType d_type, ToolChannel *d_channel, QString d_title, QString a_label)
	{
		monitoring_data = new ToolData;
		depth_data = new ToolData;		

		log_type = d_type;
		channel = d_channel;

		axis_label = a_label;
		data_title = d_title;
	}
	~MonitoringData()
	{
		qDeleteAll(monitoring_data->begin(), monitoring_data->end());
		qDeleteAll(depth_data->begin(), depth_data->end());
		monitoring_data->clear();
		depth_data->clear();

		delete monitoring_data;
		delete depth_data;
	}

	int channel_id() { return (channel == NULL ? 0xFF : channel->channel_id); }

	ToolData* monitoring_data;
	ToolData* depth_data;
	DataType log_type;	
	QString axis_label;
	QString data_title;

private:
	ToolChannel *channel;
};
*/


class MonitoringFrame : public QWidget, public Ui::MonitoringFrame
{
	Q_OBJECT

public:
	explicit MonitoringFrame(ScannedQuantity _data_type, QWidget *parent = 0);
	~MonitoringFrame();

	ScannedQuantity getDataType() { return data_type; }

	Ui::MonitoringFrame *getUI() { return ui; }
	QwtPlot *plot() { return ui->qwtPlot; }
	QwtPlotPicker *picker() {return plot_picker; }
	QwtPlotGrid *grid() { return plot_grid; }
	PlotZoomer *zoomer() { return plot_zoomer; }

	QLabel *label1() { return ui->label1; }
	QLabel *label2() { return ui->label2; }
	QFrame *vline() { return ui->line; }
	QVBoxLayout *legend_lout() { return ui->vlout; }
	QFrame *getCurveFrame() { return ui->frameCurves; }
	bool isActive() { return active; }
	void setActive(bool _active) { active = _active; }
	void setMinimumHeightHeader(int val) { ui->frameCurves->setMinimumHeight(val); }
	void setMaximumHeightHeader(int val) { ui->frameCurves->setMaximumHeight(val); }

	void clearAll();
	void addCurve(QwtPlotCurve *_curve);

	void setDefaultPalette();

	int itemsCount() { return items_count; }

protected:
	void mousePressEvent ( QMouseEvent * event );  

private:
	Ui::MonitoringFrame *ui;

	QwtPlotGrid *plot_grid;
	QwtPlotPicker *plot_picker;
	PlotZoomer *plot_zoomer;

	ScannedQuantity data_type;

	int items_count;
	bool active;

private slots:
	void zoomed(const QRectF &rect);

signals:
	void activated();
	void plot_rescaled(void*);
	void vertical_axis_panned(void*);
};

#endif // MONITORING_FRAME_H