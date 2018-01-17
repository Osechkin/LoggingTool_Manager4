#ifndef PROCESSING_SETTINGS_DIALOG_H
#define PROCESSING_SETTINGS_DIALOG_H

#include <QtGui>
#include <QLayout>

#include "../Common/data_containers.h"

#include "ui_processing_settings_dialog_exp1.h"


class PicFrame : public QFrame
{
public:
	PicFrame(QWidget * parent);

	virtual void paintEvent(QPaintEvent *e);

private:
	QPixmap pixmap;
};


class ProcessingSettingsDialog : public QDialog, Ui::ProcessingSettingsDialog
{
	Q_OBJECT

public:
	explicit ProcessingSettingsDialog(const ProcessingRelax &proc_relax, QWidget *parent = 0);
		
	int getSmoothingNumber() { return smoothing_number; }
	int getExtrapolationNumber() { return extrapolation_number; }
	double getAlpha() { return alpha; }
	int getI() { return I; }
	int getK() { return K; }
	bool isSmoothed() { return is_smoothed; }
	bool isCentered() { return is_centered; }
	bool isExtrapolated() { return is_extrapolated; }
	bool isQualityControlled() { return is_quality_controlled; }
	bool isPorosityOn() { return chboxPorosity->isChecked(); }

	double getT2_from() { return T2_from; }
	double getT2_to() { return T2_to; }
	double getT2_min() { return T2_min; }
	double getT2_max() { return T2_max; }
	double getT2_cutoff() { return T2_cutoff; }
	double getT2_cutoff_clay() { return T2_cutoff_clay; }
	int getT2_points() { return T2_points; }
	int getIterations() { return iters; }
	QColor getMCBWcolor() { return MCBWcolor; }
	QColor getMBVIcolor() { return MBVIcolor; }
	QColor getMFFIcolor() { return MFFIcolor; }
		
private slots:
	void setCentered(bool flag);
	void setSmoothing(bool flag);
	void setExtrapolated(bool flag);
	void selectSmoothingNumber(bool flag);
	void selectExtrapolationNumber(bool flag);
	void setAlpha(double value);	
	void setI(int value);
	void setK(int value);
	void setQualityControlled(bool flag);

	void setT2from(double value);
	void setT2to(double value);
	void setT2min(double value);
	void setT2max(double value);
	void setT2cutoff(double value);
	void setT2cutoff_clay(double value);
	void setT2points(int value);
	void setIterations(int value);
	void setMCBWcolor();
	void setMBVIcolor();
	void setMFFIcolor();

private:	
	bool is_centered;
	bool is_smoothed;
	bool is_extrapolated;
	bool is_quality_controlled;
	int smoothing_number;
	int extrapolation_number;
	double alpha;
	int I;
	int K;

	double T2_from;
	double T2_to;
	double T2_cutoff;
	double T2_cutoff_clay;
	double T2_min;				// Т2, с которого начинается интегрирование спектра при вычислении MCBW
	double T2_max;				// T2, до которого ведется интегрирование спектра при вычислении MFFI
	int T2_points;
	int iters;
	QColor MCBWcolor;
	QColor MBVIcolor;
	QColor MFFIcolor;
};

#endif // PROCESSING_SETTINGS_DIALOG_H