#ifndef AXIS_DIALOG_H
#define AXIS_DIALOG_H

#include <QtGui>
#include "qwt_plot.h"

#include "ui_axis_dialog.h"


class AxisDialog : public QDialog, public Ui::AxisDialog
{
    Q_OBJECT

public:
    explicit AxisDialog(QwtPlot *_plot, QWidget *parent = 0);
    ~AxisDialog();

    QwtText getAxisTitle(QwtPlot::Axis axis);
    QColor getAxisColor(QwtPlot::Axis axis);
    int getAxisThickness(QwtPlot::Axis axis);
    bool axisVisible(QwtPlot::Axis axis);
    QFont getAxisLabelFont(QwtPlot::Axis axis);
    QColor getAxisLabelColor(QwtPlot::Axis axis);
    bool axisAutoRescale(QwtPlot::Axis axis);
    QPair<double, double> getAxisScaleRange(QwtPlot::Axis axis);
    double getAxisScaleStep(QwtPlot::Axis axis);
    QwtScaleEngine *getAxisScaleType(QwtPlot::Axis axis);
    int getAxisTicks(QwtPlot::Axis axis);
    bool areLinkedXAxes() { return linked_X; }
    bool areLinkedYAxes() { return linked_Y; }

	void hideRangeSettings(bool flag);

    typedef struct
    {
        double min_value;
        double max_value;
        double step;
    } AxisRange;

    enum AxisScale { Linear_Scale, Log10_Scale };

    QwtPlot::Axis getActiveAxis();

private:   
    void initTitle(QwtPlot::Axis axis);
    void initAxisWidget(QwtPlot::Axis axis);
    void initAxisLabel(QwtPlot::Axis axis);
    void initAxisRange(QwtPlot::Axis axis);
    void setAxisActive(QwtPlot::Axis axis);

    void setConnections();


    QwtPlot *qwtPlot;

    QwtScaleWidget *axis_widget_yleft;
    QwtScaleWidget *axis_widget_yright;
    QwtScaleWidget *axis_widget_xbottom;
    QwtScaleWidget *axis_widget_xtop;

    QwtText title_yleft;
    QwtText title_yright;
    QwtText title_xbottom;
    QwtText title_xtop;

    bool visible_yleft;
    bool visible_yright;
    bool visible_xbottom;
    bool visible_xtop;

    QFont label_font_yleft;
    QFont label_font_yright;
    QFont label_font_xbottom;
    QFont label_font_xtop;

    AxisRange range_yleft;
    AxisRange range_yright;
    AxisRange range_xbottom;
    AxisRange range_xtop;

    bool auto_rescale_yleft;
    bool auto_rescale_yright;
    bool auto_rescale_xbottom;
    bool auto_rescale_xtop;

    AxisScale scale_yleft;
    AxisScale scale_yright;
    AxisScale scale_xbottom;
    AxisScale scale_xtop;

    int ticks_yleft;
    int ticks_yright;
    int ticks_xbottom;
    int ticks_xtop;

    QList<QListWidgetItem*> axes_list;

    bool linked_X;
    bool linked_Y;


public slots:
    void setLinkedXAxes(bool flag);
    void setLinkedYAxes(bool flag);

private slots:   
    void setTitleFont(QFont font);
    void setTitleFontSize(QString sz);
    void setTitleColor();
    void setAxisColor();
    void setLabelColor();
    void setTitleText();
    void setBoldTitle(bool flag);
    void setItalicTitle(bool flag);
    void setUnderlinedTitle(bool flag);
    void setAxisThickness(int value);
    void setAxisVisible(bool flag);
    void setLabelFont(QFont font);
    void setLabelFontSize(QString sz);
    void setLabelFontBold(bool flag);
    void setLabelFontItalic(bool flag);
    void setAxisAutoRescale(bool flag);
    void setRange(QString str);
    void setAxisScale(QString str);
    void setAxisTicks(int value);
    void selectListWidgetItem(QListWidgetItem *item);
};

#endif // AXIS_DIALOG_H
