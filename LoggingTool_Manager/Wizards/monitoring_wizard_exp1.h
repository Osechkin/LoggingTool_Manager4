#ifndef MONITORING_WIZARD_EXP1_H
#define MONITORING_WIZARD_EXP1_H

#include <stdint.h>

#include <QDateTime>
#include <QTime>
#include <QSettings>
#include <QFrame>
#include <QList>

#include <qwt_scale_draw.h>
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_panner.h"
#include "qwt_counter.h"
#include "qwt_symbol.h"

#include "../Common/settings_tree.h"
#include "../GraphClasses/graph_classes.h"
#include "../GraphClasses/plotted_objects.h"

#include "monitoring_frame.h"

#include "ui_monitoring_wizard_exp1.h"


#define MAX_LOGGING_DATA	10000



/*
typedef QList<QVector<double>* >	ToolData;

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


class ToolFrame : public QFrame
{
public:
	ToolFrame(QWidget * parent) : QFrame(parent, /*Qt::Window |*/Qt::FramelessWindowHint)
	{
		setAttribute(Qt::WA_TranslucentBackground);

		pixmap.load(":pictures/images/KMRK.png");
		resize(pixmap.size());
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		QPainter p(this);
		p.drawPixmap(0, 0, width(), height(), pixmap);
	}

private:
	QPixmap pixmap;
};


/*class TimeScaleDraw: public QwtScaleDraw
{
public:
	TimeScaleDraw( const QDateTime &base ): baseTime( base ) { }
	virtual QwtText label( double v ) const
	{
		QDateTime upTime = baseTime.addSecs( static_cast<int>( v ) );
		return upTime.time().toString("h:mm");
	}

private:
	QDateTime baseTime;	
};*/


#define MIN_CRITICAL_TEMP			-50
#define MAX_CRITICAL_TEMP			200

#define TIME_RANGE					20		// minuts
#define DEPTH_RANGE					100


class Monitor_Data
{
public:
	Monitor_Data() 
	{
		curve = new QwtPlotCurve();
		time = new QVector<QDateTime>();
		dtime = new QVector<double>();
		depth = new QVector<double>();
		data = new QVector<double>();
	}
	Monitor_Data(double _min, double _max, QPair<bool, int> _depth_scale) 
	{
		max_data = _max;
		min_data = _min;	

		min_zoom = min_data;
		max_zoom = max_data;

		depth_scale = _depth_scale;
	}
	Monitor_Data(double _min, double _max, double _zoom_min, double _zoom_max, QPair<bool, int> _depth_scale) 
	{
		max_data = _max;
		min_data = _min;	

		min_zoom = _zoom_min;
		max_zoom = _zoom_max;

		depth_scale = _depth_scale;
	}
	~Monitor_Data()
	{
		delete curve;
		delete time;
		delete dtime;
		delete data;
	}

	QwtPlotCurve *curve;				// кривая построенных данных data
	QVector<QDateTime> *time;			// вектор вемен, когда были измерены данные мониторинга data
	QVector<double> *dtime;				// вектор вемен, когда были измерены данные мониторинга data
	QVector<double> *depth;				// вектор глубин, при которых измерялись данные мониторинга
	QVector<double> *data;				// вектор измеренных данных за все время эксперимента	

	double max_data;
	double min_data;

	double min_zoom;
	double max_zoom;

	QPair<bool, int> depth_scale;
	QPair<bool, int> time_scale;
};


class MonitoringWidget : public QWidget, public Ui::MonitoringWidgetExp1
{
	Q_OBJECT

public:
	explicit MonitoringWidget(QSettings *app_settings, QWidget *parent = 0);
	~MonitoringWidget();
		
	void addData(uint8_t _comm_id, QString _name, QPair<bool,double> dpt, QVector<double> *_depth_data, QVector<double> *_data);


	QList<Monitor_Data*> &getTemperatureData() { return temperatureData; }
	QList<Monitor_Data*> &getVoltageData() { return voltageData; }
	QList<Monitor_Data*> &getCurrentData() { return currentData; }
		
	QPair<QDateTime, double> lastTemperaturePoint(int index);
	QPair<QDateTime, double> lastVoltagePoint(int index);
	QPair<QDateTime, double> lastCurrentPoint(int index);

	double depthFrom() { return depth_from; }
	double depthTo() { return depth_to; }
	void setDepthFrom(double _depth_from);
	void setDepthTo(double _depth_to);

private:
	void addMonitoringTemperature(int num, QString _name);
	void addMonitoringVoltage(int num, QString _name);
	void initMonitoringObjects();
	void setCurveProperties(QwtPlotCurve *_curve, ScannedQuantity _data_type, int num);
	//void refreshPlotFrameHeader(MonitoringFrame *_frame);
	void refreshPlotFrameHeaders();
	void clearAll(MonitoringFrame *_frame);
	void setConnections();
	
	Ui::MonitoringWidgetExp1 *ui;

	QSettings *app_settings;

	QDateTime base_date_time;	

	QList<Monitor_Data*> temperatureData;		// температурные данные
	QList<Monitor_Data*> voltageData;			// данные по напряжению
	QList<Monitor_Data*> currentData;			// данные по токам

	QList<CTreeWidgetItem*> c_items;
	QList<CTreeWidgetItem*> c_title_items;
	CTreeWidgetItem *c_item_selected;

	QList<MonitoringFrame*> plot_frames;
	MonitoringFrame* active_frame;

	QList<QColor> color_list;

	double depth_from;
	double depth_to;

private slots:
	void addPanel();
	void clearPanel();
	void clearAllPanels();
	void changeDepthFrom(double val);
	void changeDepthRange(double val);
	void panelActivated();
	void rescaleAllPlots(void *qwtplot_obj);

signals:
	void temperature_status(unsigned char);
	
};

#endif // MONITORING_WIZARD_EXP1_H