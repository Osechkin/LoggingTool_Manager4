#ifndef DEPTH_SCALE_DIALOG_H
#define DEPTH_SCALE_DIALOG_H


#include <QDialog>

#include "ui_depth_scale_dialog.h"


class DepthScaleDialog : public QDialog, public Ui::DepthScaleDialog
{
	Q_OBJECT

public:
	DepthScaleDialog(double from, double to, QWidget *parent = 0);
	
	bool getMinimumDepth(double& min_depth);
	bool getMaximumDepth(double& max_depth);

private:
	void setConnections();
};


#endif // DEPTH_SCALE_DIALOG_H