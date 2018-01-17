#ifndef MONITORING_WIZARD_H
#define MONITORING_WIZARD_H

#include <stdint.h>

#include <QDateTime>
#include <QTime>
#include <QSettings>
#include <QFrame>

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

#include "ui_monitoring_wizard.h"


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


class Monitor_Data
{
public:
	Monitor_Data() { }
	Monitor_Data(double _min, double _max, QPair<bool, int> _time_scale) 
	{
		max_data = _max;
		min_data = _min;	

		min_zoom = min_data;
		max_zoom = max_data;

		time_scale = _time_scale;
	}
	Monitor_Data(double _min, double _max, double _zoom_min, double _zoom_max, QPair<bool, int> _time_scale) 
	{
		max_data = _max;
		min_data = _min;	

		min_zoom = _zoom_min;
		max_zoom = _zoom_max;

		time_scale = _time_scale;
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
	QVector<double> *data;				// вектор измеренных данных за все время эксперимента	

	double max_data;
	double min_data;

	double min_zoom;
	double max_zoom;

	QPair<bool, int> time_scale;
};


class MonitoringWidget : public QWidget, public Ui::MonitoringWidget
{
	Q_OBJECT

public:
	explicit MonitoringWidget(QSettings *app_settings, QWidget *parent = 0);
	~MonitoringWidget();
		
	void addData(uint8_t _comm_id, QVector<double> *_x_data, QVector<double> *_y_data);


	QList<Monitor_Data*> &getTemperatureData() { return temperatureData; }
	QList<Monitor_Data*> &getVoltageData() { return voltageData; }
	QList<Monitor_Data*> &getCurrentData() { return currentData; }
		
	QPair<QDateTime, double> lastTemperaturePoint(int index);
	QPair<QDateTime, double> lastVoltagePoint(int index);
	QPair<QDateTime, double> lastCurrentPoint(int index);

private:
	void initMonitoringObjects();
	void addMonitoringBoard(QString board_name, QList<double> minmax, QList<double> minmax_zoom = QList<double>(), QPair<bool, int> time_scale = QPair<bool, int>(false, 0));
	void addSensorsToList();
	void setConnections();
	void clearCTreeWidget();
	
	Ui::MonitoringWidget *ui;

	QSettings *app_settings;

	QDateTime base_date_time;	

	QList<Monitor_Data*> temperatureData;		// температурные данные
	QList<Monitor_Data*> voltageData;			// данные по напряжению
	QList<Monitor_Data*> currentData;			// данные по токам

	QList<CTreeWidgetItem*> c_items;
	QList<CTreeWidgetItem*> c_title_items;
	CTreeWidgetItem *c_item_selected;

private slots:
	void zoomIn();
	void zoomOut();
	void adjustMinMax();
	void setFixedTimeRange(bool);
	void changeTimeRange(int range);

signals:
	void temperature_status(unsigned char);
	
};

#endif // MONITORING_WIZARD_H