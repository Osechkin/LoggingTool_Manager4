#ifndef RFPULSE_CONTROL_WIZARD_H
#define RFPULSE_CONTROL_WIZARD_H


#include "GraphClasses/plotted_objects.h"
#include "GraphClasses/graph_classes.h"

#include "qwt_plot_intervalcurve.h"

#include "ui_rfpulse_control_wizard.h"


class RFPulseControlWizard : public QWidget, public Ui::RFPulseControlWizard
{
	Q_OBJECT

public:
	explicit RFPulseControlWizard(QWidget *parent = 0);
	~RFPulseControlWizard();

	void setDataSet(QString _name, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map);	
	PlottedDataSet *getDataSet();

	void removeDataSet();

	Ui::RFPulseControlWizard *getUI() { return ui; }

public slots:
	void changeLocation(Qt::DockWidgetArea area);

private:
	void plotData();
	double maxValue(double *_vec, int n);
	void insertCurveBar(const QString &title, const QVector<QwtIntervalSample>& samples, const QColor &bg_color, const QColor &curve_color);

	Ui::RFPulseControlWizard *ui;
		
	PlotZoomer *zoomer_RFP;
	QwtCurveSettings *curve_settings_RFP;
	PlottedDataSet *data_RFP;
	//QwtPlotCurve *curve_RFP1;
	//QwtPlotCurve *curve_RFP2;
	QwtPlotIntervalCurve *d_intervalCurve;
	bool pds_created_RFP;	

	double y_max;
};

#endif // RFPULSE_CONTROL_WIZARD_H