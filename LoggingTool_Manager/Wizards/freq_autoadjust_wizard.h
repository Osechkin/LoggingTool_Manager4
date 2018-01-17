#ifndef FREQ_AUTOADJUST_WIZARD_H
#define FREQ_AUTOADJUST_WIZARD_H


#include "GraphClasses/plotted_objects.h"
#include "GraphClasses/graph_classes.h"

#include "qwt_plot_intervalcurve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_textlabel.h"

#include "ui_freq_autoadjust_wizard.h"


class FreqAutoadjustWizard : public QWidget, public Ui::tabFreqAutoadjust
{
	Q_OBJECT

public:
	explicit FreqAutoadjustWizard(QWidget *parent = 0);
	~FreqAutoadjustWizard();

	void setDataSet(QString _name, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map);	
	PlottedDataSet *getDataSet() { return data_Freq; }

	void removeDataSet();

	Ui::tabFreqAutoadjust *getUI() { return ui; }

public slots:
	//void changeLocation(Qt::DockWidgetArea area);

private:
	void plotData();
	double maxValue(double *_vec, int n);
	//void insertCurveBar(const QString &title, const QVector<QwtIntervalSample>& samples, const QColor &bg_color, const QColor &curve_color);

	Ui::tabFreqAutoadjust *ui;

	PlotZoomer *zoomer_Freq;
	QwtCurveSettings *curve_settings_Freq;
	PlottedDataSet *data_Freq;	
	QwtPlotCurve *curve_Freq;
	//QwtPlotIntervalCurve *d_intervalCurve;
	bool pds_created_Freq;	
	QwtPlotTextLabel *qwttext_max_freq;
	QwtPlotMarker *max_line;

	double y_max;
};

#endif // FREQ_AUTOADJUST_WIZARD_H