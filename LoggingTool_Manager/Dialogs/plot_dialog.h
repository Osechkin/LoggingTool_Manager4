#ifndef PLOT_DIALOG_H
#define PLOT_DIALOG_H

#include <QtGui>
#include "qwt_plot.h"
#include "qwt_plot_grid.h"

#include "ui_plot_dialog.h"


class PlotDialog : public QDialog, public Ui::PlotDialog
{
    Q_OBJECT

public:
    explicit PlotDialog(QwtPlot *_plot, QwtPlotGrid *_grid, QWidget *parent = 0);
    ~PlotDialog();

    QwtText getTitle() { return qwtPlot->title(); }
    QColor getBackgroundColor() { return qwtPlot->canvasBackground().color(); }
    bool enabledGridMajorX() { return grid->xEnabled(); }
    bool enabledGridMinorX() { return grid->xMinEnabled(); }
    bool enabledGridMajorY() { return grid->yEnabled(); }
    bool enabledGridMinorY() { return grid->yMinEnabled(); }
    QPen getGridMajorLine() { return grid->majorPen(); }
    QPen getGridMinorLine() { return grid->minorPen(); }

    QString getActiveGrid() { return active_grid; }

private:
    void initTitle();
    void initBackground();
    void initGrid(QString grid_type);
    void setActiveGrid(QString grid_type);

    void setConnections();

    QwtPlot *qwtPlot;
    QwtPlotGrid *grid;
    QList<QListWidgetItem*> grid_list;
    QString active_grid;

private slots:
    void setTitleFont(QFont font);
    void setTitleFontSize(QString str);
    void setTitleText();
    void setBoldTitle(bool flag);
    void setItalicTitle(bool flag);
    void setUnderlinedTitle(bool flag);
	void setTitleColor();
    void setBackgroundColor();
    void setGridEnabled(bool flag);
    void setGridLineType(QString str);
    void setGridLineWidth(double val);
    void setGridLineColor();	

    void selectListWidgetItem(QListWidgetItem *item);
};

#endif // PLOT_DIALOG_H
