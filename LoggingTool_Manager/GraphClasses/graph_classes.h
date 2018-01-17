#ifndef GRAPH_CLASSES_H
#define GRAPH_CLASSES_H

#include <QFrame>
#include <QGroupBox>
#include <QSettings>
#include <QToolButton>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QWidgetList>
#include <QScrollArea>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include <QToolBox>
#include <QTreeWidget>
#include <QSpinBox>
#include <QSplitter>

#include <QStyleFactory>

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_panner.h"
#include "qwt_counter.h"
#include "qwt_symbol.h"
#include "qwt_scale_draw.h"

#include "../io_general.h"
#include "plotted_objects.h"

#include "../Communication/message_class.h"
#include "../Common/experiment_settings.h"

#include "ui_sdsp_widget.h"


#define DEFAULT_CURVES		10		// количество наборов свойств кривых 


class QwtCurveSettings;
class PlottedDataSet;


class PlotZoomer: public QwtPlotZoomer
{	
public:
	PlotZoomer(int xAxis, int yAxis, /*QwtPlotCanvas*/ QWidget *canvas);

protected:
	virtual bool accept(QPolygon &) const;
	virtual bool end(bool ok);
};

class InversePlotZoomer: public QwtPlotZoomer
{	
public:
	InversePlotZoomer(int xAxis, int yAxis, /*QwtPlotCanvas*/ QWidget *canvas);

protected:
	virtual bool accept(QPolygon &) const;
	virtual bool end(bool ok);
};


class PlotPanner: public QwtPlotPanner
{

public:
	explicit PlotPanner(QWidget* parent) : QwtPlotPanner(parent) { }

	virtual bool eventFilter( QObject * object, QEvent * event)
	{
		if ( object == NULL || object != parentWidget() )
			return false;

		switch ( event->type() )
		{
		case QEvent::MouseButtonPress:
			{
				widgetMousePressEvent( static_cast<QMouseEvent *>( event ) );
				break;
			}
		case QEvent::MouseMove:
			{
				QMouseEvent * evr = static_cast<QMouseEvent *>( event );
				widgetMouseMoveEvent( evr );
				widgetMouseReleaseEvent( evr  );
				setMouseButton(evr->button(), evr->modifiers());
				widgetMousePressEvent( evr);
				break;
			}
		case QEvent::MouseButtonRelease:
			{
				QMouseEvent * evr = static_cast<QMouseEvent *>( event );
				widgetMouseReleaseEvent( static_cast<QMouseEvent *>( event ) );
				break;
				grab();
			}
		case QEvent::KeyPress:
			{
				widgetKeyPressEvent( static_cast<QKeyEvent *>( event ) );
				break;
			}
		case QEvent::KeyRelease:
			{
				widgetKeyReleaseEvent( static_cast<QKeyEvent *>( event ) );
				break;
			}
		case QEvent::Paint:
			{
				if ( isVisible() )
					return true;
				break;
			}
		default:;
		}

		return false;
	}
};


class TimeScaleDraw: public QwtScaleDraw
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
};

class TimeScaleDrawSec: public QwtScaleDraw
{
public:
	TimeScaleDrawSec( const QDateTime &base ): baseTime( base ) { }
	virtual QwtText label( double v ) const
	{
		QDateTime upTime = baseTime.addSecs( static_cast<int>( v ) );
		return upTime.time().toString("h:mm:ss");
	}

private:
	QDateTime baseTime;		
};


struct PlotCurve
{
	PlottedDataSet *ds;		// указатель на набор данных, по которым построена кривая curve
	QwtPlotCurve *curve;	
};


//#define	WIN_POINTS		512

struct WinFunc
{	
	QString name;
	int applied_name;
	QString description;
	float dx;
	int applied_dx;
	float sigma;
	int applied_sigma;
};




class DataPlot : public QWidget
{
	Q_OBJECT

public:
	DataPlot(QString &objectName, QSettings *settings, QWidget *parent = 0);
	~DataPlot();	

	QwtPlot *getPlot() { return qwtPlot; }
	QwtCurveSettings *getDefaultCurveSettings(int index); 
	QList<QwtCurveSettings*> &getDefaultCurveSettingsList() { return default_curve_settings_list; }
	void setAutoRescale(QwtPlot::Axis axis, bool flag);	
	
private:
	void setConnections();
	void loadPlotSettings();

	//void plotData(QVector<double> *_y_data, QVector<uint8_t> *_bad_map);
	void plotData(PlottedDataSet *ds);	
	
	QSettings *app_settings;

	QFrame *frame_toolbar;	
	QToolButton *tbtAutoResize;
	QToolButton *tbtFindSignal;
	//QToolButton *tbtAutoResizeH;
	QAction *a_auto_resizeH;
	QAction *a_auto_resizeV;
	QToolButton *tbtAxesSettings;
	QToolButton *tbtAxisYZeroDown;
	QToolButton *tbtAxisYZeroCenter;
	QToolButton *tbtPlotSettings;
	QToolButton *tbtCurveSettings;
	QToolButton *tbtSaveSettings;

	QToolBox *toolBoxData;
	QWidget *pageCurves;
	QToolButton *tbtAddCurve;
	QToolButton *tbtRemove;
	QListWidget *listWidgetCurves;
	QWidget *pageWinFunc;
	QToolButton *tbtSettings;
	QToolButton *tbtShowWin;
	QComboBox *cboxFuncType;
	QLabel *lblFormulaView;
	QLineEdit *ledFormulaView;
	QLineEdit *ledX0;
	QLineEdit *ledSigma;

	QFrame *frame;
	QwtPlot *qwtPlot;
	QwtPlotGrid *grid;
	QwtPlotPanner *panner;
	QwtPlotPicker *picker;
	PlotZoomer *zoomer;	
	QList<PlotCurve> curve_list;		
	QList<QwtCurveSettings*> default_curve_settings_list;

	//bool show_bad_data;

public slots:
	void plotDataSet(PlottedDataSet* ds);
	void removeData(QString &ds_name);

private slots:
	void showAxisDialog();
	void showPlotDialog();
	void showCurveDialog();
	void setAutoRescale(bool flag);
	void rescaleToShowAll();
	void rescaleToZeroCenter();
	void rescaleToZeroDown();
	void savePlotSettings();	

};


class PlottedDataManager : public QWidget
{
	Q_OBJECT

public:
	PlottedDataManager(DataPlot *_data_plot, DataPlot *_math_plot, QSettings *_settings, QVector<ToolChannel*> _tool_channel, QWidget *parent = 0);
	~PlottedDataManager();

	void addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, bool _hold_on, int def_index = 0);
	void addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, bool _hold_on);
	QList<PlottedDataSet*> *getAllDataSets() { return &dataset_list; }
	PlottedDataSet *getDataSetLast() { return dataset_list.last(); }
	PlottedDataSet *getDataSetAt(int index) { if (index < dataset_list.count()) return dataset_list[index]; else return NULL; }

	void setDataPlot(DataPlot *_plot) { data_plot = _plot; }
	DataPlot *getDataPlot() { return data_plot; }
	void setMathPlot(DataPlot *_plot) { math_plot = _plot; }
	DataPlot *getMathPlot() { return math_plot; }

	void hideWinWidgets();
	void hideMovingAverWidget();

	void setLastShownDataSets(int val) { last_shown_datasets = val; }
	int getLastShownDataSets() { return last_shown_datasets; }
	
	void removeDataSetAt(int _id);
	void removeDataSetByName(QString _name);

	void refreshDataSets();

	void addRecord(PlottedDataSet *_ds);
	void addSubRecord(PlottedDataSet *_ds, QString _parent_name);

	void refreshToolChannels(QVector<ToolChannel*> _tool_channels, int cur_channel = 0);
	ToolChannel *getCurrentToolChannel();

	//void setApplyBtnChecked(bool state) { pbtApplyWinTime->setChecked(state); }

private:		
	void setConnections();	
	void clearCTreeWidget();
	void createWinFuncs();
	void fillWinFuncs();

	double NMR_SAMPLE_FREQ();
		
	QSettings *app_settings;

	QToolButton *tbtProcess;	
	QSpinBox *sbxDatasets;	
	QTreeWidget *treeWidget;	
	QToolButton *tbtMark;
	QToolButton *tbtUnmarkAll;
	QToolButton *tbtExport;
	QToolButton *tbtRemove;	

	QLabel *lblChannels;
	QComboBox *cboxChannels;
	QGroupBox *gbxTimeWin;
	QComboBox *cboxTypeTime;
	QLabel *lblTimeWinFormula; 
	QLabel *lblX0Time; 
	QDoubleSpinBox *dsbxX0Time; 
	QLabel *lblSigmaTime;
	QDoubleSpinBox *dsbxSigmaTime;
	//QPushButton *pbtSettingsTime;
	//QPushButton *pbtApplyWinTime;
	
	QGroupBox *gbxFreqWin;
	QComboBox *cboxTypeFreq;
	QLabel *lblFreqWinFormula;
	QLabel *lblX0Freq;
	QDoubleSpinBox *dsbxX0Freq;
	QLabel *lblSigmaFreq;
	QDoubleSpinBox *dsbxSigmaFreq;
	//QPushButton *pbtSettingsFreq;
	//QPushButton *pbtApplyWinFreq;

	QPushButton *pbtApplyWin;
	QPushButton *pbtSaveSettings;

	QGroupBox *gbxAveraging;
	QSpinBox *sbxDataSets;
	QPushButton *pbtApplyMovingAver;
	int ds_counter;
	bool is_moving_aver;


	DataPlot *data_plot;
	DataPlot *math_plot;

	QList<PlottedDataSet*> dataset_list;
	QList<CTreeWidgetItem*> c_items;
	QList<WinFunc> win_funcs;
	WinFunc time_win;
	WinFunc freq_win;

	QVector<ToolChannel*> tool_channels;

	int last_shown_datasets;

	//bool busy;			// индикатор отправки параметров оконных функций в каротажный прибор. busy = true, если данные отправлены, но ответа еще нет 

private slots:
	void removeDataSet();
	void markDataSets();
	void unmarkAll();
	void changeTimeWin(QString str);
	void changeFreqWin(QString str);
	void changeToolChannel(QString str);
	void setTimeWinChecked(bool flag);
	void setFreqWinChecked(bool flag);
	void setMovingAverChecked(bool flag);
	void changeTimeX0(double val);
	void changeTimeSigma(double val);
	void changeFreqX0(double val);
	void changeFreqSigma(double val);
	void changeDataSetCount(int val);
	void applyWinParams();
	//void applyDataSetCounter();
	void saveSettings();
	

signals:
	void dataset_removed(QString& _name);
	void plot_dataset(PlottedDataSet *ds);
	void apply_win_func(QVector<int>&);
	void apply_moving_averaging(bool, int);
};


class MonitoringPlot : public QWidget
{
	Q_OBJECT

public:
	MonitoringPlot(QString &objectName, QSettings *settings, QWidget *parent = 0);
	~MonitoringPlot();

	typedef struct 
	{
		QString param_name;
		QString last_value;
		double max_value;
		QString units;
		bool plot_state;
		QWidgetList widgets;
		QVector<uint> xtime;
		QVector<double> ydata;
		QwtPlotCurve *curve;
	} m_Parameter;

	void setTitle(const QString &title);
	void setInterval(const double val);
	void addParameter(const QString &param_name, const QString &title, const QString &units, const double max_value);
	void setParameterValue(const QString &param_name, const double val);

	QString getTitle() const { return title; } 
	double getInterval() const { return time_range; }
	QList<m_Parameter> getParameterList() const { return parameters; }

private:
	void setConnections();
	QList<QwtPlotCurve*> getCurveList();

	template<class X> void findWidget(const QWidgetList &widget_list, X *widget)
	{
		widget = NULL;
		foreach (QWidget *cur_widget, widget_list)
		{
			if (X *w = qobject_cast<X*>(cur_widget)) 
			{
				widget = w;
				return;
			}
		}
	}

	QGroupBox *main_frame;
	QFrame *tool_frame;
	QFrame *parameters_frame;
	QToolButton *tbtPlotSettings;
	QToolButton *tbtAxisSettings;
	QToolButton *tbtCurveSettings;
	QwtCounter *cntInterval;
	QwtPlot *qwtPlot;
	QwtPlotGrid *grid;	
	QSpacerItem *spacer_last;
	QScrollArea *scrollArea;
	QWidget *scrollAreaWidgetContents;

	QString title;
	double time_range;
	QList<m_Parameter> parameters;

	QSettings *app_settings;

private slots:
	void setPlotSettings();
	void setAxisSettings();
	void setCurveSettings();
	void setParamChecked(bool);
};


class OscilloscopeWidget: public QWidget
{
	Q_OBJECT

public:
	OscilloscopeWidget(QWidget *tab, QSettings *settings, QVector<ToolChannel*> tl_channels, QWidget *parent = 0);
	~OscilloscopeWidget();

	void addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, bool _hold_on);	
	void addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, bool _hold_on, int def_index = 0);
	PlottedDataSet *getDataSetLast() { return oscdata_manager->getDataSetLast(); }	
	PlottedDataSet *getDataSetAt(int index) { if (index < oscdata_manager->getAllDataSets()->size()) return oscdata_manager->getAllDataSets()->at(index); else return NULL; }

	DataPlot *getOscDataPlot() { return oscDataPlot; }
	DataPlot *getOscMathPlot() { return oscMathPlot; }
	PlottedDataManager *getDataManager() { return oscdata_manager; }

	void refreshDataSets();

private:
	void setConnections();	

	DataPlot *oscDataPlot;
	DataPlot *oscMathPlot;
	PlottedDataManager *oscdata_manager;

	QSettings *app_settings;
	QVector<ToolChannel*> tool_channels;

private slots:
	void applyWinFuncParams(QVector<int> &params);	

public slots:
	//void setWinFuncSeqStatus(ConnectionState state);
	
signals:
	void apply_win_func(QVector<int> &params);
	void apply_moving_averaging(bool, int);
	//void send_msg(DeviceData*, QString&);					// отправка запроса в message_processor (в конечном итоге - в каротажный прибор)
	//void place_to_statusbar(QString&);						// размещение строки символов на строке состояния
	//void add_text(QString&);								// размещение записи в логе совершенных действий в объекте NMRToolLinker
};


class RelaxationWidget: public QWidget
{
	Q_OBJECT

public:
	RelaxationWidget(QWidget *tab, QSettings *settings, QWidget *parent = 0);
	~RelaxationWidget();

	void addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, bool _hold_on);	
	void addDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, bool _hold_on, int def_index = 0);
	PlottedDataSet *getDataSetLast() { return relax_data_manager->getDataSetLast(); }	
	PlottedDataSet *getDataSetAt(int index) { if (index < relax_data_manager->getAllDataSets()->size()) return relax_data_manager->getAllDataSets()->at(index); else return NULL; }

	DataPlot *getRelaxDataPlot() { return relaxDataPlot; }
	DataPlot *getRelaxMathPlot() { return relaxMathPlot; }
	PlottedDataManager *getDataManager() { return relax_data_manager; }

	void refreshDataSets();

private:
	void setConnections();

	DataPlot *relaxDataPlot;
	DataPlot *relaxMathPlot;
	PlottedDataManager *relax_data_manager;

	QSettings *app_settings;

signals:
	void apply_moving_averaging(bool, int);

};



#define SDSP_TIME_RANGE					300		// seconds


struct SDSPCurve_Settings
{
	SDSPCurve_Settings()
	{
		pen_width = 1;
		pen_color = QColor(Qt::red);
		sym_size = 8;
		sym_style = QwtSymbol::Ellipse;
		sym_color = QColor(Qt::red);
		sym_border_width = 1;
		sym_border_color = QColor(Qt::black);
	}

	QColor pen_color;
	int pen_width;
	QwtSymbol::Style sym_style;
	QColor sym_color;
	int sym_size;
	QColor sym_border_color;
	int sym_border_width;
};


struct SDSP_PlotSettings
{
	SDSP_PlotSettings() { }
	SDSP_PlotSettings(double _min, double _max, QPair<bool, double> _time_scale)
	{
		min_zoom = _min;
		max_zoom = _max;

		time_scale = _time_scale;
	}

	double min_zoom;
	double max_zoom;
	QPair<bool, double> time_scale;
};


class SDSP_Data
{
public:
	SDSP_Data() 
	{
		curve = NULL;
		time = NULL;
		dtime = NULL;
		data = NULL;
	}	
	~SDSP_Data()
	{
		delete curve;
		delete time;
		delete dtime;
		delete data;
	}

	QwtPlotCurve *curve;				// кривая построенных данных data
	QVector<QDateTime> *time;			// вектор времен, когда были измерены данные мониторинга data
	QVector<double> *dtime;				// вектор времен, когда были измерены данные мониторинга data
	QVector<double> *data;				// вектор измеренных данных за все время эксперимента		
};


typedef		uint8_t				triple;
#define		t_false				(0)
#define		t_unknown			(1)
#define		t_true				(2)

typedef		QPair<int, triple>	int_p;
typedef		QPair<bool, triple>	bool_p;


struct SDSP_FreqParams
{
	int_p power;
	int_p gain1;
	int_p gain2;
};

struct SDSP_Params
{
	enum ParamType { 
		DAC_Code = 1,		// DAC Code
		Freq,				// frequency (32 or 60 MHz)
		Power_32,			// Power at 32 MHz
		Gain1_32,			// Gain1 at 32 MHz
		Gain2_32,			// Gain2 at 32 MHz
		Power_60,			// Power at 60 MHz
		Gain1_60,			// Gain1 at 60 MHz
		Gain2_60,			// Gain2 at 60 MHz
		Channel_1,			// Channel 1 ON/OFF
		Channel_2,			// Channel 2
		Transmitter,		// Transmitter 
		Power_Status,		// Power status
		Repetitions,		// Repetitions
		Average				// Number of averages 
	};

	void set(SDSP_Params::ParamType type, int val, triple flag = t_true)
	{
		switch (type)
		{
		case DAC_Code:		DAC.first = val; DAC.second = flag; break;
		case Freq:			freq.first = val; freq.second = flag; break;
		case Power_32:		freq32_params.power.first = val; freq32_params.power.second = flag; break;
		case Gain1_32:		freq32_params.gain1.first = val; freq32_params.gain1.second = flag; break;
		case Gain2_32:		freq32_params.gain2.first = val; freq32_params.gain2.second = flag; break;
		case Power_60:		freq60_params.power.first = val; freq60_params.power.second = flag; break;
		case Gain1_60:		freq60_params.gain1.first = val; freq60_params.gain1.second = flag; break;
		case Gain2_60:		freq60_params.gain2.first = val; freq60_params.gain2.second = flag; break;		
		case Repetitions:	repetitions.first = val; repetitions.second = flag; break;		
		}
	}

	void set(SDSP_Params::ParamType type, bool val, triple flag = t_true)
	{
		switch (type)
		{		
		case Channel_1:		channel1.first = val; channel1.second = flag; break;
		case Channel_2:		channel2.first = val; channel2.second = flag; break;
		case Transmitter:	transmitter.first = val; transmitter.second = flag; break;
		case Power_Status:	power_status.first = val; power_status.second = flag; break;		
		case Average:		average.first = val; average.second = flag; break;
		}
	}


	int_p DAC;
	int_p freq;
	SDSP_FreqParams freq32_params;
	SDSP_FreqParams freq60_params;
	bool_p channel1;
	bool_p channel2;
	bool_p transmitter;
	bool_p power_status;
	int_p repetitions;
	bool_p average;
};


class SDSPWidget : public QWidget
{
	Q_OBJECT

public:
	SDSPWidget(QSettings *app_settings, QWidget *parent = 0);
	~SDSPWidget();

	Sequence *getSequence() { return &curSeq; }
	void resetSDSPParameters();
	void waitCmdResult(bool flag) { is_waiting = flag; }
	void adjustmentWasStarted(bool flag) { is_started = flag; } 
	void addData(QVector<double> *_x_data, QVector<double> *_y_data);

	void saveSettings();

public slots:
	void enableSDSP(bool flag) { is_enabled = flag; }

private:
	void setConnections();	
	void initAllSettings();
	void addSDSPCurve(QList<SDSP_Data*> &sdsp_data_list, QString name, SDSPCurve_Settings curve_settings = SDSPCurve_Settings());
	void setDataTransferEnabled(bool flag) { is_enabled = flag; }

private:
	Ui::SDSPWidget ui;

	SDSP_Params sdsp_params;
	SDSP_PlotSettings phase_plot_settings;
	SDSP_PlotSettings ampl_plot_settings;
	double Kcalibr_dfi;
	double Kcalibr_ramp;
	QList<SDSP_Data*> phaseData;
	QList<SDSP_Data*> amplData;
	QDateTime base_date_time;

	Sequence curSeq;

	QTimer timer;
	int ticks;

	QTimer cmd_timer;
	bool is_enabled;				// флаг возможности начать передачу параметров в диэлектрический прибор
	bool is_waiting;				// флаг ожидания результата отправки параметров в диэлектрический прибор
	bool is_started;				// диэлектрический прибор был запущен/остановлен (програама для его настройки)
	bool is_running;				// программа запустилась/еще запускается...

	QSettings *app_settings;

private slots:
	void selectFreq(bool flag);
	void setDAC(int val);
	void setPower(int val);
	void setGain1(int val);
	void setGain2(int val);
	void setTransmitter(bool flag);
	void setChannel1(bool flag);
	void setChannel2(bool flag);
	void setPowerStatus(bool flag);
	//void setRepetitions(int val);
	void setDAC(QString text);
	void setPower(QString text);
	void setGain1(QString text);
	void setGain2(QString text);
	//void setRepetitions(QString text);
	void zoomIn();
	void zoomOut();
	void adjustMinMax();
	void changeTimeRange(double range);
	void newTick();
	void applyParameters();
	void checkAllParamStates();

signals:
	void apply_sdsp_params(QVector<int>&);
	void place_to_statusbar(QString&);
};



#endif // GRAPH_CLASSES_H