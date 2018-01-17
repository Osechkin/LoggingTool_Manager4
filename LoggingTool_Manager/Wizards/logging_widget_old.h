#ifndef LOGGING_WIDGET_H
#define LOGGING_WIDGET_H


#include <QtWidgets/QWidget>

#include "../Common/data_containers.h"
#include "../GraphClasses/plotted_objects.h"

#include "ui_logging_widget.h"


#define MAX_LOGGING_DATA	10000


class PaintFrame_forLogging : public PaintFrame
{
	Q_OBJECT

public:
	PaintFrame_forLogging(QWidget *parent = 0);
	PaintFrame_forLogging(QwtCurveSettings *c_settings, QWidget *parent = 0);
	//~PaintFrame_forLogging();

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

struct LoggingData
{
	enum DataType { NMRIntegral_Probe1,			// интеграл под спадом поперечной намагниченности для ЯМР-датчика 1
					NMRIntegral_Probe2,			// интеграл под спадом поперечной намагниченности для ЯМР-датчика 2
					NMRIntegral_Probe3,			// интеграл под спадом поперечной намагниченности для ЯМР-датчика 3
					NMRBins_Probe1,				// бины, построенные на основе спектра времен Т2 для данных датчика 1
					NMRBins_Probe2,				// бины, построенные на основе спектра времен Т2 для данных датчика 2
					NMRBins_Probe3,				// бины, построенные на основе спектра времен Т2 для данных датчика 3
					Gamma,						// данные гамма-каротажа
					WaveDielectric,				// данные волнового диэлектрического каротажа (все данные вместе)
					WaveDielectric_PhaseDiff,	// данные волнового диэлектрического каротажа (разность фаз)
					WaveDielectric_AmplRatio,	// данные волнового диэлектрического каротажа (отношение амплитуд)
					AFR_Probe1,					// данные АЧХ для ЯМР-датчика 1
					AFR_Probe2,					// данные АЧХ для ЯМР-датчика 2
					AFR_Probe3,					// данные АЧХ для ЯМР-датчика 3
					NoType						// данные неизвестного типа
				};

	enum BinType {  MCBW,						// бин MCBW
					MBVI,						// бин MBVI
					MFFI,						// бин MFFI
					MPHI,						// бин MPHI
					MPHS,						// бин MPHS
					None						// данные, не относящиеся к типам, характерным для спектра времен Т2
				};

	LoggingData(DataType d_type/*, int ch_number*/)
	{
		logging_data = new ToolData;
		depth_data = new ToolData;		

		log_type = d_type;
		//channel_number = ch_number;
	}
	~LoggingData()
	{
		qDeleteAll(logging_data->begin(), logging_data->end());
		qDeleteAll(depth_data->begin(), depth_data->end());
		logging_data->clear();
		depth_data->clear();

		delete logging_data;
		delete depth_data;
	}

	ToolData* logging_data;
	ToolData* depth_data;
	DataType log_type;
	//int channel_number;
};


class LoggingPlot : public QObject
{
	Q_OBJECT

public:
	LoggingPlot(LoggingData *log_container, QwtPlot *qwt_plot, QFrame *frame, int index, QWidget *parent = 0);	
	~LoggingPlot();
	
	//void setDataType(LoggingPlot::DataType dt) { data_type = dt; }
	void setDataType(LoggingData::DataType dt, LoggingData *dcont);
	LoggingData::DataType getDataType() { return data_type; }

	QwtPlot *getQwtPlot() { return qwtPlot; }
	int getQwtPlotIndex() { return qwt_plot_index; }
	PlotZoomer *getInversePlotZoomer() { return zoomer; }

	QList<QwtPlotCurve*> *getQwtCurveList() { return &qwt_curve_list; }	
	QList<QVector<double>* > *getDataXList() { return xdata_list; }
	QList<QVector<double>* > *getDataYList() { return ydata_list; }

	void addDataSet(DataSet *ds, LoggingData::DataType dt);
	void clearAll();
	void closeLoggingCurveList() { cur_index = -1; }	

private:
	void setCurveSettings(QwtPlotCurve *curve, LoggingData::BinType dt = LoggingData::None);
	void addNewLog(QString log_name, LoggingData::BinType dt = LoggingData::None);
		
private:
	QwtPlot *qwtPlot;
	QFrame *legend_frame;
	PlotZoomer *zoomer;

	int qwt_plot_index;
	QList<QwtPlotCurve*> qwt_curve_list;
	QList<QVector<double>* >* xdata_list;
	QList<QVector<double>* >* ydata_list;
	LoggingData::DataType data_type;

	QList<QHBoxLayout*> hbox_list;
	QVBoxLayout *legend_vlout;

	QList<QColor> color_list;
	int cur_index;
		
signals:
	void plot_rescaled(void*);
};


class LoggingWidget : public QWidget
{
	Q_OBJECT

public:
	LoggingWidget(QWidget *parent = 0);
	//LoggingWidget(QVector<ToolChannel*> channels, QWidget *parent = 0);
	~LoggingWidget();

	void addDataSets(DataSets _dss);

	QList<LoggingPlot*> *getLoggingPlotList() { return &logging_plot_list; }
	QList<QwtPlot*> *getQwtPlotList() { return &qwtplot_list; }
	static int getDataTypeId(QString str, bool *res);

private:
	Ui::LoggingWidget ui;

	QList<LoggingPlot*> logging_plot_list;
	QList<QwtPlot*>	qwtplot_list;
	QList<QPair<LoggingData::DataType, int> > plot_map;	// first = data_type, second = qwtPlot number (see qwtplot_list)

	QList<LoggingData*> data_containers;
	
private:
	void setConnections();
	void setAxisPlotTitle(QwtPlot *qwtPlot, QwtPlot::Axis axisIndex, QString text);
	void replotLegends();	
	bool getMinMaxValues(const QVector<double> *vec, double &min_value, double &max_value);	
	bool isVisibleChannel(ToolChannel *channel, uint8_t comm_id = 0);

private slots:
	void setDataType(int index);
	void rescaleAllDepths(void *qwtplot_obj);
	void setRezoomAll(const QRectF &rect);
	//void setDepthScale();
	void searchAllData();
	void clearAllData();
	void setDepthFrom(double from);
	void setDepthTo(double to);
	void scaleDataIn();
	void scaleDataOut();
	void scaleDepthIn();
	void scaleDepthOut();
};

#endif // LOGGING_WIDGET_H
