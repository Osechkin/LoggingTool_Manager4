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
	enum DataType { //NMRIntegral_Probe,			// интеграл под спадом поперечной намагниченности для ЯМР
					NMRBins_Probe1,				// бины, построенные на основе спектра времен Т2 для данных датчика ЯМР					
					NMRBins_Probe2,
					NMRBins_Probe3,
					Gamma,						// данные гамма-каротажа
					WaveDielectric,				// данные волнового диэлектрического каротажа (все данные вместе)
					WaveDielectric_PhaseDiff,	// данные волнового диэлектрического каротажа (разность фаз)
					WaveDielectric_AmplRatio,	// данные волнового диэлектрического каротажа (отношение амплитуд)
					AFR_Probe,					// данные АЧХ для ЯМР-датчика	
					SolidEcho_Probe,			// данные солид-эхо 
					NoType						// данные неизвестного типа
				};

	enum BinType {  MCBW,						// бин MCBW
					MBVI,						// бин MBVI
					MFFI,						// бин MFFI
					MPHI,						// бин MPHI
					MPHS,						// бин MPHS
					None						// данные, не относящиеся к типам, характерным для спектра времен Т2
				};

	LoggingData(DataType d_type, ToolChannel *d_channel, QString d_title, QString a_label)
	{
		logging_data = new ToolData;
		depth_data = new ToolData;		

		log_type = d_type;
		channel = d_channel;

		axis_label = a_label;
		data_title = d_title;
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

	int channel_id() { return (channel == NULL ? 0xFF : channel->channel_id); }
	
	ToolData* logging_data;
	ToolData* depth_data;
	DataType log_type;	
	QString axis_label;
	QString data_title;

private:
	ToolChannel *channel;
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

	LoggingData *getLogContainer() { return log_container; }

	QList<QwtPlotCurve*> *getQwtCurveList() { return &qwt_curve_list; }	
	QList<QVector<double>* > *getDataXList() { return xdata_list; }
	QList<QVector<double>* > *getDataYList() { return ydata_list; }

	//void setKERNParameters(bool _calibr_state, double _core_diameter, double _standard_porosity, double _standard_diameter);
	void addDataSet(DataSet *ds, ToolChannel *channel, LoggingData::DataType dt);
	void clearAll();
	void closeLoggingCurveList() { cur_index = -1; }	

private:
	void setCurveSettings(QwtPlotCurve *curve, LoggingData::BinType dt = LoggingData::None);
	void addNewLog(QString log_name, LoggingData::BinType dt = LoggingData::None);

private slots:
	void onMoved(int dx, int dy);
	void onPanned(int dx, int dy);
		
private:
	QwtPlot *qwtPlot;
	QFrame *legend_frame;
	PlotZoomer *zoomer;
	QwtPlotPicker *picker;
	QwtPlotPanner *panner;

	int qwt_plot_index;
	QList<QwtPlotCurve*> qwt_curve_list;
	QList<QVector<double>* >* xdata_list;
	QList<QVector<double>* >* ydata_list;
	LoggingData::DataType data_type;
	QString data_title;
	LoggingData *log_container;
	
	/*QVector<double> calibration_store;	
	double core_diameter;
	bool calibration_state;	
	double standard_porosity;	
	double standard_diameter;	// диаметер калибровочного образца
	*/

	QList<QHBoxLayout*> hbox_list;
	QVBoxLayout *legend_vlout;

	QList<QColor> color_list;
	int cur_index;
			
signals:
	void plot_rescaled(void*);
	void vertical_axis_panned(void*);
	void new_calibration_coef(double, ToolChannel*);
};


template <class T1, class T2, class T3>
struct PlotMap
{	
	PlotMap() : first(), second(), third() {}
	PlotMap(const T1 &t1, const T2 &t2, const T3 &t3) : first(t1), second(t2), third(t3) {}
	
	template <typename TT1, typename TT2, typename TT3>
	PlotMap(const PlotMap<TT1, TT2, TT3> &p) : first(p.first), second(p.second), third(p.third) {}
	
	template <typename TT1, typename TT2, typename TT3>
	PlotMap &operator=(const PlotMap<TT1, TT2, TT3> &p) 
	{ 
		first = p.first; 
		second = p.second; 
		third = p.third; 
		return *this; 
	}

#ifdef Q_COMPILER_RVALUE_REFS	
	template <typename TT1, typename TT2, typename TT3>
	PlotMap(PlotMap<TT1, TT2, TT3> &&p) : first(std::move(p.first)), second(std::move(p.second)), third(std::move(p.third)) {}
	
	template <typename TT1, typename TT2, typename TT3>
	PlotMap &operator=(PlotMap<TT1, TT2, TT3> &&p)
	{ 
		first = std::move(p.first); 
		second = std::move(p.second); 
		third = std::move(p.third); 
		return *this; 
	}
#endif

	T1 first;
	T2 second;
	T3 third;
};


class LoggingWidget : public QWidget
{
	Q_OBJECT

public:
	//LoggingWidget(QWidget *parent = 0);
	LoggingWidget(QVector<ToolChannel*> channels, QWidget *parent = 0);
	~LoggingWidget();

	void resetLoggingPlots(QVector<ToolChannel*> channels);
	void addDataSets(DataSets _dss);

	QList<LoggingPlot*> *getLoggingPlotList() { return &logging_plot_list; }
	QList<QwtPlot*> *getQwtPlotList() { return &qwtplot_list; }
	static int getDataTypeId(QString str, bool *res);
	bool isCalibrationON() { return calibration_state; }

private:
	Ui::LoggingWidget ui;

	QList<LoggingPlot*> logging_plot_list;
	QList<QwtPlot*>	qwtplot_list;
	//QList<QPair<LoggingData::DataType, int> > plot_map;	// first = data_type, second = qwtPlot number (see qwtplot_list)
	QList<PlotMap<LoggingData::DataType, int, int> > plot_map;	// first = data_type, second = channel_id, third = qwtPlot number (see qwtplot_list), 

	QList<LoggingData*> data_containers;
	QVector<ToolChannel*> tool_channels;

	bool calibration_state;		// откалиброваны/не откалиброваны результаты ЯМР измерений на калибровочный образец
	double core_diameter;
	double standard_porosity;
	double standard_core_diameter;
	
private:
	void setConnections();
	void setAxisPlotTitle(QwtPlot *qwtPlot, QwtPlot::Axis axisIndex, QString text);
	void replotLegends();	
	bool getMinMaxValues(const QVector<double> *vec, double &min_value, double &max_value);	
	bool isVisibleChannel(ToolChannel *channel, uint8_t comm_id = 0);
	bool calcCalibrationCoeff(DataSet *ds, ToolChannel *channel, double &calibr_coef);

private slots:
	void setDataType(int index);
	void rescaleAllPlots(void *qwtplot_obj);
	void rescaleAllDepths(void *qwtplot_obj);
	void setRezoomAll(const QRectF &rect);
	//void setDepthScale();
	void searchAllData();	
	void setDepthFrom(double from);
	void setDepthRange(double range);
	void scaleDataIn();
	void scaleDataOut();
	void scaleDepthIn();
	void scaleDepthOut();
	void calibrateNMRData(double calibr_coef, ToolChannel *channel);

public slots:
	void startCalibration();
	void finishCalibration();
	void setCoreDiameter(double val) { core_diameter = val; }
	void setStandardPorosity(double val) { standard_porosity = val; }
	void setStandardCoreDiameter(double val) { standard_core_diameter = val; }
	void clearAllData();

signals:
	void new_calibration_coef_toCfg(double, ToolChannel*);
	void new_calibration_coef(double, ToolChannel*);

};

#endif // LOGGING_WIDGET_H
