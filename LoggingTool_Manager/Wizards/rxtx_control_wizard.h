#ifndef RXTX_CONTROL_WIZARD_H
#define RXTX_CONTROL_WIZARD_H


#include "GraphClasses/plotted_objects.h"
#include "GraphClasses/graph_classes.h"

#include "ui_rxtx_control_wizard.h"


class RxTxControlWizard : public QWidget, public Ui::RxTxControlWizard
{
	Q_OBJECT

public:
	explicit RxTxControlWizard(QWidget *parent = 0);
	~RxTxControlWizard();

	void setDataSet(uint8_t data_type, QString _name, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map);	
	PlottedDataSet *getDataSet(uint8_t data_type);
	
	void removeDataSet(uint8_t data_type);

	Ui::RxTxControlWizard *getUI() { return ui; }

public slots:
	void changeLocation(Qt::DockWidgetArea area);

private:
	void plotData(uint8_t data_type);
	double maxValue(double *_vec, int n);

	Ui::RxTxControlWizard *ui;
		
	PlotZoomer *zoomer_Rx;
	QwtCurveSettings *curve_settings_Rx;
	PlottedDataSet *data_Rx;
	QwtPlotCurve *curve_Rx;
	bool pds_created_Rx;

	PlotZoomer *zoomer_Tx;
	QwtCurveSettings *curve_settings_Tx;
	PlottedDataSet *data_Tx;
	QwtPlotCurve *curve_Tx;
	bool pds_created_Tx;

	/*PlotZoomer *zoomer_RFP;
	QwtCurveSettings *curve_settings_RFP;
	PlottedDataSet *data_RFP;
	QwtPlotCurve *curve_RFP;
	bool pds_created_RFP;	*/
	
	//bool pds_rfp_created;

public slots:

};

#endif // RXTX_CONTROL_WIZARD_H