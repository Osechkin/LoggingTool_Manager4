#include <QtGui>
#include <QColorDialog>

#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"

#include "axis_dialog.h"


AxisDialog::AxisDialog(QwtPlot *_plot, QWidget *parent) : QDialog(parent)
{
    this->setupUi(this);
	this->setParent(parent);

    this->setWindowTitle(tr("Axis Settings Wizard"));
    this->setModal(true);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);  	
    setWindowFlags(Qt::Dialog|Qt::WindowTitleHint); // в заголовке окна остается только кнопка закрытия окна

    tedTitle->setLineWrapMode(QTextEdit::NoWrap);
    tedTitle->setAlignment(Qt::AlignCenter);

    QStringList sizes;
    sizes << "4" << "5" << "6" << "8" << "9" << "10" << "11" << "12" << "14" << "16"
          << "18" << "20" << "22" << "24" << "28" << "32" << "36" << "42" << "48";
    cboxAxisSize->addItems(sizes);
    cboxLabelSize->addItems(sizes);

    QStringList scales;
    scales << tr("Linear") << tr("Logarithmic (Log10)");
    cboxScale->addItems(scales);

    ledFrom->setValidator(new QDoubleValidator(this));
    ledTo->setValidator(new QDoubleValidator(this));
    ledStep->setValidator(new QDoubleValidator(this));

    QPalette p_dblue = gbxAxisWidget->palette();
    QPalette p_black = cboxAxisSize->palette();
    p_dblue.setColor(QPalette::Text, QColor(Qt::darkBlue));
    p_black.setColor(QPalette::Text, QColor(Qt::black));
    gbxAxisWidget->setPalette(p_dblue);
    gbxLabels->setPalette(p_dblue);
    gboxAxis->setPalette(p_dblue);
    gboxCaption->setPalette(p_dblue);
    gboxRange->setPalette(p_dblue);
    cboxAxisSize->setPalette(p_black);
    cboxLabelSize->setPalette(p_black);
    cboxScale->setPalette(p_black);
    fontboxAxisTitle->setPalette(p_black);
    fontboxAxisLabel->setPalette(p_black);
    ledFrom->setPalette(p_black);
    ledTo->setPalette(p_black);
    ledStep->setPalette(p_black);
    sbxAxisThickness->setPalette(p_black);

    QStringList item_list;
    item_list << tr("Left") << tr("Bottom") << tr("Right") << tr("Top");
    QStringList icon_list;
    icon_list << "axis_left.png" << "axis_bottom.png" << "axis_right.png" << "axis_top.png" ;
    listWidget->setIconSize(QSize(40, 40));
    listWidget->setFlow(QListView::TopToBottom);

    int i = 0;
    foreach (QString str, item_list)
    {
        QListWidgetItem *pitem = new QListWidgetItem(str, listWidget, QListWidgetItem::UserType);
        pitem->setIcon(QIcon(":/images/" + icon_list[i++]));
        pitem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        axes_list.append(pitem);
    }
    listWidget->setCurrentItem(axes_list.first());

	tbtBoldTitle->setIcon(QIcon(":/images/text_bold.png"));
	tbtItalicTitle->setIcon(QIcon(":/images/text_italic.png"));
	tbtUnderlinedTitle->setIcon(QIcon(":/images/text_underline.png"));
	tbtColorTitle->setIcon(QIcon(":/images/text_color.png"));
	tbtAxisColor->setIcon(QIcon(":/images/color.png"));
	tbtBoldLabel->setIcon(QIcon(":/images/text_bold.png"));
	tbtItalicLabel->setIcon(QIcon(":/images/text_italic.png"));
	tbtLabelColor->setIcon(QIcon(":/images/text_color.png"));


    qwtPlot = new QwtPlot;
    title_yleft = qwtPlot->axisTitle(QwtPlot::yLeft);
    title_yright = qwtPlot->axisTitle(QwtPlot::yRight);
    title_xbottom = qwtPlot->axisTitle(QwtPlot::xBottom);
    title_xtop = qwtPlot->axisTitle(QwtPlot::xTop);

    QFont font = _plot->axisTitle(QwtPlot::yLeft).font();
    QColor color = _plot->axisTitle(QwtPlot::yLeft).color();
    QString str = _plot->axisTitle(QwtPlot::yLeft).text();    
    title_yleft.setFont(font);
    title_yleft.setColor(color);
    title_yleft.setText(str);
    font = _plot->axisTitle(QwtPlot::yRight).font();
    color = _plot->axisTitle(QwtPlot::yRight).color();
    str = _plot->axisTitle(QwtPlot::yRight).text();
    title_yright.setFont(font);
    title_yright.setColor(color);
    title_yright.setText(str);
    font = _plot->axisTitle(QwtPlot::xBottom).font();
    color = _plot->axisTitle(QwtPlot::xBottom).color();
    str = _plot->axisTitle(QwtPlot::xBottom).text();
    title_xbottom.setFont(font);
    title_xbottom.setColor(color);
    title_xbottom.setText(str);
    font = _plot->axisTitle(QwtPlot::xTop).font();
    color = _plot->axisTitle(QwtPlot::xTop).color();
    str = _plot->axisTitle(QwtPlot::xTop).text();
    title_xtop.setFont(font);
    title_xtop.setColor(color);
    title_xtop.setText(str);

    axis_widget_yleft = qwtPlot->axisWidget(QwtPlot::yLeft);
    axis_widget_yright = qwtPlot->axisWidget(QwtPlot::yRight);
    axis_widget_xbottom = qwtPlot->axisWidget(QwtPlot::xBottom);
    axis_widget_xtop = qwtPlot->axisWidget(QwtPlot::xTop);
    axis_widget_yleft->setPalette(_plot->axisWidget(QwtPlot::yLeft)->palette());
    axis_widget_yright->setPalette(_plot->axisWidget(QwtPlot::yRight)->palette());
    axis_widget_xbottom->setPalette(_plot->axisWidget(QwtPlot::xBottom)->palette());
    axis_widget_xtop->setPalette(_plot->axisWidget(QwtPlot::xTop)->palette());
    int pts = _plot->axisWidget(QwtPlot::yLeft)->scaleDraw()->penWidth();
    axis_widget_yleft->scaleDraw()->setPenWidth(_plot->axisWidget(QwtPlot::yLeft)->scaleDraw()->penWidth());
    axis_widget_yright->scaleDraw()->setPenWidth(_plot->axisWidget(QwtPlot::yRight)->scaleDraw()->penWidth());
    axis_widget_xbottom->scaleDraw()->setPenWidth(_plot->axisWidget(QwtPlot::xBottom)->scaleDraw()->penWidth());
    axis_widget_xtop->scaleDraw()->setPenWidth(_plot->axisWidget(QwtPlot::xTop)->scaleDraw()->penWidth());

    visible_yleft = _plot->axisEnabled(QwtPlot::yLeft);
    visible_yright = _plot->axisEnabled(QwtPlot::yRight);
    visible_xbottom = _plot->axisEnabled(QwtPlot::xBottom);
    visible_xtop = _plot->axisEnabled(QwtPlot::xTop);

    label_font_yleft = _plot->axisFont(QwtPlot::yLeft);
    label_font_yright = _plot->axisFont(QwtPlot::yRight);
    label_font_xbottom = _plot->axisFont(QwtPlot::xBottom);
    label_font_xtop = _plot->axisFont(QwtPlot::xTop);

    range_yleft.min_value = _plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    range_yleft.max_value = _plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
    range_yleft.step = _plot->axisStepSize(QwtPlot::yLeft);
    range_yright.min_value = _plot->axisScaleDiv(QwtPlot::yRight).lowerBound();
    range_yright.max_value = _plot->axisScaleDiv(QwtPlot::yRight).upperBound();
    range_yright.step = _plot->axisStepSize(QwtPlot::yRight);
    range_xbottom.min_value = _plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    range_xbottom.max_value = _plot->axisScaleDiv(QwtPlot::xBottom).upperBound();
    range_xbottom.step = _plot->axisStepSize(QwtPlot::xBottom);
    range_xtop.min_value = _plot->axisScaleDiv(QwtPlot::xTop).lowerBound();
    range_xtop.max_value = _plot->axisScaleDiv(QwtPlot::xTop).upperBound();
    range_xtop.step = _plot->axisStepSize(QwtPlot::xTop);

    auto_rescale_yleft = _plot->axisAutoScale(QwtPlot::yLeft);
    auto_rescale_yright = _plot->axisAutoScale(QwtPlot::yRight);
    auto_rescale_xbottom = _plot->axisAutoScale(QwtPlot::xBottom);
    auto_rescale_xtop = _plot->axisAutoScale(QwtPlot::xTop);

    scale_yleft = (dynamic_cast<QwtLogScaleEngine*>(_plot->axisScaleEngine(QwtPlot::yLeft))) == 0 ? Linear_Scale : Log10_Scale;
    scale_yright = (dynamic_cast<QwtLogScaleEngine*>(_plot->axisScaleEngine(QwtPlot::yRight))) ? Log10_Scale : Linear_Scale;
    scale_xbottom = (dynamic_cast<QwtLogScaleEngine*>(_plot->axisScaleEngine(QwtPlot::xBottom))) ? Log10_Scale : Linear_Scale;
    scale_xtop = (dynamic_cast<QwtLogScaleEngine*>(_plot->axisScaleEngine(QwtPlot::xTop))) ? Log10_Scale : Linear_Scale;

    ticks_yleft = _plot->axisMaxMinor(QwtPlot::yLeft);
    ticks_yright = _plot->axisMaxMinor(QwtPlot::yRight);
    ticks_xbottom = _plot->axisMaxMinor(QwtPlot::xBottom);
    ticks_xtop = _plot->axisMaxMinor(QwtPlot::xTop);

	chbxLinkedX->setChecked(true);
	setLinkedXAxes(true);
	chbxLinkedX->setDisabled(true);
	//chbxLinkedY->setChecked(true);
	//setLinkedYAxes(true);	
	chbxLinkedY->setChecked(false);
	setLinkedYAxes(false);
	chbxLinkedY->setDisabled(true);

    setAxisActive(QwtPlot::yLeft);	

    setConnections();
}

AxisDialog::~AxisDialog()
{	
    qDeleteAll(axes_list);
    delete qwtPlot;
}

void AxisDialog::setConnections()
{
    connect(tbtAxisColor, SIGNAL(clicked()), this, SLOT(setAxisColor()));
    connect(tbtLabelColor, SIGNAL(clicked()), this, SLOT(setLabelColor()));
    connect(tbtColorTitle, SIGNAL(clicked()), this, SLOT(setTitleColor()));
    connect(fontboxAxisTitle, SIGNAL(currentFontChanged(QFont)), this, SLOT(setTitleFont(QFont)));
    connect(cboxAxisSize, SIGNAL(currentIndexChanged(QString)), this, SLOT(setTitleFontSize(QString)));
    connect(tedTitle, SIGNAL(textChanged()), this, SLOT(setTitleText()));
    connect(tbtBoldTitle, SIGNAL(clicked(bool)), this, SLOT(setBoldTitle(bool)));
    connect(tbtItalicTitle, SIGNAL(clicked(bool)), this, SLOT(setItalicTitle(bool)));
    connect(tbtUnderlinedTitle, SIGNAL(clicked(bool)), this, SLOT(setUnderlinedTitle(bool)));
    connect(sbxAxisThickness, SIGNAL(valueChanged(int)), this, SLOT(setAxisThickness(int)));
    connect(chbxShowAxis, SIGNAL(toggled(bool)), this, SLOT(setAxisVisible(bool)));
    connect(fontboxAxisLabel, SIGNAL(currentFontChanged(QFont)), this, SLOT(setLabelFont(QFont)));
    connect(cboxLabelSize, SIGNAL(currentIndexChanged(QString)), this, SLOT(setLabelFontSize(QString)));
    connect(tbtBoldLabel, SIGNAL(toggled(bool)), this, SLOT(setLabelFontBold(bool)));
    connect(tbtItalicLabel, SIGNAL(toggled(bool)), this, SLOT(setLabelFontItalic(bool)));
    connect(chbxAutoRescale, SIGNAL(clicked(bool)), this, SLOT(setAxisAutoRescale(bool)));
    connect(ledFrom, SIGNAL(textChanged(QString)), this, SLOT(setRange(QString)));
    connect(ledTo, SIGNAL(textChanged(QString)), this, SLOT(setRange(QString)));
    connect(ledStep, SIGNAL(textChanged(QString)), this, SLOT(setRange(QString)));
    connect(cboxScale, SIGNAL(currentIndexChanged(QString)), this, SLOT(setAxisScale(QString)));
    connect(sbxTicks, SIGNAL(valueChanged(int)), this, SLOT(setAxisTicks(int)));
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectListWidgetItem(QListWidgetItem*)));
    connect(chbxLinkedX, SIGNAL(toggled(bool)), this, SLOT(setLinkedXAxes(bool)));
    connect(chbxLinkedY, SIGNAL(toggled(bool)), this, SLOT(setLinkedYAxes(bool)));
}

void AxisDialog::setTitleFont(QFont font)
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QFont title_font = title->font();
    title_font.setFamily(font.family());
    title->setFont(title_font);
    tedTitle->setFont(title->font());
    tedTitle->setText(title->text());
}

void AxisDialog::setTitleFontSize(QString sz)
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    bool ok;
    int size = sz.toInt(&ok);
    if (!ok) return;

    QFont font = title->font();
    font.setPointSize(size);
    title->setFont(font);
    tedTitle->setFont(title->font());
    tedTitle->setText(title->text());
}

void AxisDialog::setTitleColor()
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QColor title_color = title->color();
    title_color = QColorDialog::getColor(title_color);
    if (title_color.isValid())
    {
        title->setColor(title_color);
        QPalette p_title = tedTitle->palette();
        p_title.setColor(QPalette::Text, title->color());
        tedTitle->setPalette(p_title);
    }
}

void AxisDialog::setAxisColor()
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtScaleWidget *axis_widget = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_widget = axis_widget_yleft;    break;
    case QwtPlot::yRight:   axis_widget = axis_widget_yright;   break;
    case QwtPlot::xBottom:  axis_widget = axis_widget_xbottom;  break;
    case QwtPlot::xTop:     axis_widget = axis_widget_xtop;     break;
    default: return;
    }
    if (axis_widget == 0) return;

    QColor axis_color = axis_widget->palette().color(QPalette::Foreground);
    axis_color = QColorDialog::getColor(axis_color);
    if (axis_color.isValid())
    {
        QPalette p = ledAxisColor->palette();
        p.setBrush(ledAxisColor->backgroundRole(), QBrush(axis_color));
        ledAxisColor->setPalette(p);

        QPalette p_axis = axis_widget->palette();
        p_axis.setColor(QPalette::Foreground, axis_color);
        axis_widget->setPalette(p_axis);
    }
}

void AxisDialog::setLabelColor()
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtScaleWidget *axis_widget = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_widget = axis_widget_yleft;    break;
    case QwtPlot::yRight:   axis_widget = axis_widget_yright;   break;
    case QwtPlot::xBottom:  axis_widget = axis_widget_xbottom;  break;
    case QwtPlot::xTop:     axis_widget = axis_widget_xtop;     break;
    default: return;
    }
    if (axis_widget == 0) return;

    QColor label_color = axis_widget->palette().color(QPalette::Text);
    label_color = QColorDialog::getColor(label_color);
    if (label_color.isValid())
    {
        QPalette p = ledLabelColor->palette();
        p.setBrush(ledLabelColor->backgroundRole(), QBrush(label_color));
        ledLabelColor->setPalette(p);

        QPalette p_axis = axis_widget->palette();
        p_axis.setColor(QPalette::Text, label_color);
        axis_widget->setPalette(p_axis);
    }
}

void AxisDialog::setTitleText()
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QString title_text = tedTitle->toPlainText();
    title->setText(title_text);
}

void AxisDialog::setBoldTitle(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QFont font_title = title->font();
    font_title.setBold(flag);
    title->setFont(font_title);
    tedTitle->setFont(title->font());
}

void AxisDialog::setItalicTitle(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QFont font_title = title->font();
    font_title.setItalic(flag);
    title->setFont(font_title);
    tedTitle->setFont(title->font());
}

void AxisDialog::setUnderlinedTitle(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QFont font_title = title->font();
    font_title.setUnderline(flag);
    title->setFont(font_title);
    tedTitle->setFont(title->font());
}

void AxisDialog::setAxisThickness(int value)
{
    QwtPlot::Axis axis = getActiveAxis();
    QwtScaleWidget *axis_widget = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_widget = axis_widget_yleft;    break;
    case QwtPlot::yRight:   axis_widget = axis_widget_yright;   break;
    case QwtPlot::xBottom:  axis_widget = axis_widget_xbottom;  break;
    case QwtPlot::xTop:     axis_widget = axis_widget_xtop;     break;
    default: return;
    }
    if (axis_widget == 0) return;

    axis_widget->scaleDraw()->setPenWidth(value);
}

void AxisDialog::setAxisVisible(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    switch (axis)
    {
    case QwtPlot::yLeft:    visible_yleft = flag;    break;
    case QwtPlot::xBottom:  visible_xbottom = flag;  break;
    case QwtPlot::yRight:   visible_yright = flag;   break;
    case QwtPlot::xTop:     visible_xtop = flag;     break;
    default: return;
    }
}

void AxisDialog::setLabelFont(QFont font)
{
    QwtPlot::Axis axis = getActiveAxis();
    QFont *label_font;
    switch (axis)
    {
    case QwtPlot::yLeft:    label_font = &label_font_yleft;    break;
    case QwtPlot::xBottom:  label_font = &label_font_xbottom;  break;
    case QwtPlot::yRight:   label_font = &label_font_yright;   break;
    case QwtPlot::xTop:     label_font = &label_font_xtop;     break;
    default: return;
    }

    QString font_family = font.family();
    label_font->setFamily(font_family);
}

void AxisDialog::setLabelFontSize(QString sz)
{
    QwtPlot::Axis axis = getActiveAxis();
    QFont *label_font;
    switch (axis)
    {
    case QwtPlot::yLeft:    label_font = &label_font_yleft;    break;
    case QwtPlot::xBottom:  label_font = &label_font_xbottom;  break;
    case QwtPlot::yRight:   label_font = &label_font_yright;   break;
    case QwtPlot::xTop:     label_font = &label_font_xtop;     break;
    default: return;
    }

    bool ok;
    int font_size = sz.toInt(&ok);
    if (ok) label_font->setPointSize(font_size);
}

void AxisDialog::setLabelFontBold(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    switch (axis)
    {
    case QwtPlot::yLeft:    label_font_yleft.setBold(flag);    break;
    case QwtPlot::xBottom:  label_font_xbottom.setBold(flag);  break;
    case QwtPlot::yRight:   label_font_yright.setBold(flag);   break;
    case QwtPlot::xTop:     label_font_xtop.setBold(flag);     break;
    default: return;
    }
}

void AxisDialog::setLabelFontItalic(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    switch (axis)
    {
    case QwtPlot::yLeft:    label_font_yleft.setItalic(flag);    break;
    case QwtPlot::xBottom:  label_font_xbottom.setItalic(flag);  break;
    case QwtPlot::yRight:   label_font_yright.setItalic(flag);   break;
    case QwtPlot::xTop:     label_font_xtop.setItalic(flag);     break;
    default: return;
    }
}

void AxisDialog::setAxisAutoRescale(bool flag)
{
    QwtPlot::Axis axis = getActiveAxis();
    switch (axis)
    {
    case QwtPlot::yLeft:    auto_rescale_yleft = flag;    break;
    case QwtPlot::xBottom:  auto_rescale_xbottom = flag;  break;
    case QwtPlot::yRight:   auto_rescale_yright = flag;   break;
    case QwtPlot::xTop:     auto_rescale_xtop = flag;     break;
    default: return;
    }

    ledFrom->setDisabled(flag);
    ledTo->setDisabled(flag);
    ledStep->setDisabled(flag);

    if (areLinkedXAxes()) setLinkedXAxes(true);
    if (areLinkedYAxes()) setLinkedYAxes(true);
}

void AxisDialog::setRange(QString str)
{
    QwtPlot::Axis axis = getActiveAxis();
    AxisRange *axis_range;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_range = &range_yleft;    break;
    case QwtPlot::xBottom:  axis_range = &range_xbottom;  break;
    case QwtPlot::yRight:   axis_range = &range_yright;   break;
    case QwtPlot::xTop:     axis_range = &range_xtop;     break;
    default: return;
    }

    bool ok;
    double value = str.toDouble(&ok);
    if (!ok) return;

    QLineEdit *led = (QLineEdit*)sender();
    if (led == ledFrom) axis_range->min_value = value;
    else if (led == ledTo) axis_range->max_value = value;
    else if (led == ledStep) axis_range->step = value;

    if (areLinkedXAxes()) setLinkedXAxes(true);
    if (areLinkedYAxes()) setLinkedYAxes(true);
}

void AxisDialog::setAxisScale(QString str)
{
    QwtPlot::Axis axis = getActiveAxis();
    AxisScale *axis_scale;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_scale = &scale_yleft;    break;
    case QwtPlot::xBottom:  axis_scale = &scale_xbottom;  break;
    case QwtPlot::yRight:   axis_scale = &scale_yright;   break;
    case QwtPlot::xTop:     axis_scale = &scale_xtop;     break;
    default: return;
    }

    if (str == tr("Linear")) *axis_scale = Linear_Scale;
    else if (str == tr("Logarithmic (Log10)")) *axis_scale = Log10_Scale;

    if (areLinkedXAxes()) setLinkedXAxes(true);
    if (areLinkedYAxes()) setLinkedYAxes(true);
}

void AxisDialog::setAxisTicks(int value)
{
    QwtPlot::Axis axis = getActiveAxis();
    switch (axis)
    {
    case QwtPlot::yLeft:    ticks_yleft = value;    break;
    case QwtPlot::xBottom:  ticks_xbottom = value;  break;
    case QwtPlot::yRight:   ticks_yright = value;   break;
    case QwtPlot::xTop:     ticks_xtop = value;     break;
    default: return;
    }

    sbxTicks->setValue(value);

    if (areLinkedXAxes()) setLinkedXAxes(true);
    if (areLinkedYAxes()) setLinkedYAxes(true);
}


void AxisDialog::setLinkedXAxes(bool flag)
{
    linked_X = flag;
    QwtPlot::Axis axis = getActiveAxis();

    if (linked_X)
    {
        if (axis == QwtPlot::xTop)
        {
            auto_rescale_xbottom = auto_rescale_xtop;
            range_xbottom.min_value = range_xtop.min_value;
            range_xbottom.max_value = range_xtop.max_value;
            range_xbottom.step = range_xtop.step;
            scale_xbottom = scale_xtop;
            ticks_xbottom = ticks_xtop;
        }
        else
        {
            auto_rescale_xtop = auto_rescale_xbottom;
            range_xtop.min_value = range_xbottom.min_value;
            range_xtop.max_value = range_xbottom.max_value;
            range_xtop.step = range_xbottom.step;
            scale_xtop = scale_xbottom;
            ticks_xtop = ticks_xbottom;
        }
    }
}

void AxisDialog::setLinkedYAxes(bool flag)
{
    linked_Y = flag;
    QwtPlot::Axis axis = getActiveAxis();

    if (linked_Y)
    {
        if (axis == QwtPlot::yRight)
        {
            auto_rescale_yleft = auto_rescale_yright;
            range_yleft.min_value = range_yright.min_value;
            range_yleft.max_value = range_yright.max_value;
            range_yleft.step = range_yright.step;
            scale_yleft = scale_yright;
            ticks_yleft = ticks_yright;
        }
        else
        {
            auto_rescale_yright = auto_rescale_yleft;
            range_yright.min_value = range_yleft.min_value;
            range_yright.max_value = range_yleft.max_value;
            range_yright.step = range_yleft.step;
            scale_yright = scale_yleft;
            ticks_yright = ticks_yleft;
        }
    }
}


void AxisDialog::initTitle(QwtPlot::Axis axis)
{
    QwtText *title;
    switch (axis)
    {
    case QwtPlot::yLeft:    title = &title_yleft;    break;
    case QwtPlot::xBottom:  title = &title_xbottom;  break;
    case QwtPlot::yRight:   title = &title_yright;   break;
    case QwtPlot::xTop:     title = &title_xtop;     break;
    default: return;
    }

    QColor clr_title = title->color();
    QFont font_title = title->font();
    QString str_title = title->text();
    QPalette p_title; p_title.setColor(QPalette::Text, clr_title);
    tedTitle->setText(str_title);
    tedTitle->setFont(font_title);
    tedTitle->setPalette(p_title);

    fontboxAxisTitle->setCurrentFont(title->font());
    QString font_size = QString::number(title->font().pointSize());
    cboxAxisSize->setCurrentIndex(cboxAxisSize->findText(font_size));
    tbtBoldTitle->setChecked(font_title.bold());
    tbtItalicTitle->setChecked(font_title.italic());
    tbtUnderlinedTitle->setChecked(font_title.underline());
}

void AxisDialog::initAxisWidget(QwtPlot::Axis axis)
{
    QwtScaleWidget *axis_widget;
    bool visible_axis;
    switch (axis)
    {
    case QwtPlot::yLeft:
        axis_widget = axis_widget_yleft;
        visible_axis = visible_yleft;
        break;
    case QwtPlot::xBottom:
        axis_widget = axis_widget_xbottom;
        visible_axis = visible_xbottom;
        break;
    case QwtPlot::yRight:
        axis_widget = axis_widget_yright;
        visible_axis = visible_yright;
        break;
    case QwtPlot::xTop:
        axis_widget = axis_widget_xtop;
        visible_axis = visible_xtop;
        break;
    default: return;
    }

    chbxShowAxis->setChecked(visible_axis);

    QPalette p_axis = axis_widget->palette();
    QColor axis_color = p_axis.color(QPalette::Foreground);
    p_axis.setBrush(ledAxisColor->backgroundRole(), QBrush(axis_color));
    ledAxisColor->setPalette(p_axis);

    int axis_width = axis_widget->scaleDraw()->penWidth();
    sbxAxisThickness->setValue(axis_width);
}

void AxisDialog::initAxisLabel(QwtPlot::Axis axis)
{
    QwtScaleWidget *axis_widget;
    QFont label_font;
    switch (axis)
    {
    case QwtPlot::yLeft:
        label_font = label_font_yleft;
        axis_widget = axis_widget_yleft;
        break;
    case QwtPlot::xBottom:
        label_font = label_font_xbottom;
        axis_widget = axis_widget_xbottom;
        break;
    case QwtPlot::yRight:
        label_font = label_font_yright;
        axis_widget = axis_widget_yright;
        break;
    case QwtPlot::xTop:
        label_font = label_font_xtop;
        axis_widget = axis_widget_xtop;
        break;
    default: return;
    }

    fontboxAxisLabel->setCurrentFont(label_font);

    QString font_size = QString::number(label_font.pointSize());
    cboxLabelSize->setCurrentIndex(cboxLabelSize->findText(font_size));
    tbtBoldLabel->setChecked(label_font.bold());
    tbtItalicLabel->setChecked(label_font.italic());

    QPalette p_label = axis_widget->palette();
    QColor label_color = p_label.color(QPalette::Text);
    p_label.setBrush(ledLabelColor->backgroundRole(), QBrush(label_color));
    ledLabelColor->setPalette(p_label);
}

void AxisDialog::initAxisRange(QwtPlot::Axis axis)
{
    AxisRange axis_range;
    bool axis_auto_rescale;
    AxisScale axis_scale;
    int axis_ticks;
    switch (axis)
    {
    case QwtPlot::yLeft:
        axis_range = range_yleft;
        axis_auto_rescale = auto_rescale_yleft;
        axis_scale = scale_yleft;
        axis_ticks = ticks_yleft;
        break;
    case QwtPlot::xBottom:
        axis_range = range_xbottom;
        axis_auto_rescale = auto_rescale_xbottom;
        axis_scale = scale_xbottom;
        axis_ticks = ticks_xbottom;
        break;
    case QwtPlot::yRight:
        axis_range = range_yright;
        axis_auto_rescale = auto_rescale_yright;
        axis_scale = scale_yright;
        axis_ticks = ticks_yright;
        break;
    case QwtPlot::xTop:
        axis_range = range_xtop;
        axis_auto_rescale = auto_rescale_xtop;
        axis_scale = scale_xtop;
        axis_ticks = ticks_xtop;
        break;
    default: return;
    }

    chbxAutoRescale->setChecked(axis_auto_rescale);
    if (axis_auto_rescale)
    {
        ledFrom->setText(QString::number(axis_range.min_value));
        ledTo->setText(QString::number(axis_range.max_value));
        ledStep->setText("");
    }
    else
    {
        ledFrom->setText(QString::number(axis_range.min_value));
        ledTo->setText(QString::number(axis_range.max_value));
        ledStep->setText(QString::number(axis_range.step));
    }
    ledFrom->setEnabled(!axis_auto_rescale);
    ledTo->setEnabled(!axis_auto_rescale);
    ledStep->setEnabled(!axis_auto_rescale);

    int index_lin = cboxScale->findText(tr("Linear"));
    int index_log10 = cboxScale->findText(tr("Logarithmic (Log10)"));
    if (index_lin < 0 || index_log10 < 0) return;
    switch (axis_scale)
    {
    case Linear_Scale: cboxScale->setCurrentIndex(index_lin); break;
    case Log10_Scale: cboxScale->setCurrentIndex(index_log10); break;
    }

    sbxTicks->setValue(axis_ticks);
}

QwtPlot::Axis AxisDialog::getActiveAxis()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (item->text() == tr("Left")) return QwtPlot::yLeft;
    else if (item->text() == tr("Right")) return QwtPlot::yRight;
    else if (item->text() == tr("Bottom")) return QwtPlot::xBottom;
    else if (item->text() == tr("Top")) return QwtPlot::xTop;
    else return QwtPlot::axisCnt;
}

void AxisDialog::selectListWidgetItem(QListWidgetItem *item)
{
    if (item->text() == "Top") setAxisActive(QwtPlot::xTop);
    else if (item->text() == "Left") setAxisActive(QwtPlot::yLeft);
    else if (item->text() == "Right") setAxisActive(QwtPlot::yRight);
    else if (item->text() == "Bottom") setAxisActive(QwtPlot::xBottom);
}

void AxisDialog::setAxisActive(QwtPlot::Axis axis)
{
    initTitle(axis);
    initAxisWidget(axis);
    initAxisLabel(axis);
    initAxisRange(axis);
}

QwtText AxisDialog::getAxisTitle(QwtPlot::Axis axis)
{
    switch (axis)
    {
    case QwtPlot::yLeft:    return title_yleft;
    case QwtPlot::xBottom:  return title_xbottom;
    case QwtPlot::yRight:   return title_yright;
    case QwtPlot::xTop:     return title_xtop;
    default: return QwtText();
    }
}

QColor AxisDialog::getAxisColor(QwtPlot::Axis axis)
{
    QwtScaleWidget *axis_widget = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_widget = axis_widget_yleft;    break;
    case QwtPlot::yRight:   axis_widget = axis_widget_yright;   break;
    case QwtPlot::xBottom:  axis_widget = axis_widget_xbottom;  break;
    case QwtPlot::xTop:     axis_widget = axis_widget_xtop;     break;
    default: return QColor();
    }
    if (axis_widget == 0) return QColor();

    return axis_widget->palette().color(QPalette::Foreground);
}

int AxisDialog::getAxisThickness(QwtPlot::Axis axis)
{
    QwtScaleWidget *axis_widget = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_widget = axis_widget_yleft;    break;
    case QwtPlot::yRight:   axis_widget = axis_widget_yright;   break;
    case QwtPlot::xBottom:  axis_widget = axis_widget_xbottom;  break;
    case QwtPlot::xTop:     axis_widget = axis_widget_xtop;     break;
    default: return 0;
    }
    if (axis_widget == 0) return 0;

    return axis_widget->scaleDraw()->penWidth();
}

bool AxisDialog::axisVisible(QwtPlot::Axis axis)
{
    switch (axis)
    {
    case QwtPlot::yLeft:    return visible_yleft;
    case QwtPlot::xBottom:  return visible_xbottom;
    case QwtPlot::yRight:   return visible_yright;
    case QwtPlot::xTop:     return visible_xtop;
    }

    return true;
}

QFont AxisDialog::getAxisLabelFont(QwtPlot::Axis axis)
{
    switch (axis)
    {
    case QwtPlot::yLeft:    return label_font_yleft;
    case QwtPlot::xBottom:  return label_font_xbottom;
    case QwtPlot::yRight:   return label_font_yright;
    case QwtPlot::xTop:     return label_font_xtop;
    }

    return QFont();
}

QColor AxisDialog::getAxisLabelColor(QwtPlot::Axis axis)
{
    QwtScaleWidget *axis_widget = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_widget = axis_widget_yleft;    break;
    case QwtPlot::yRight:   axis_widget = axis_widget_yright;   break;
    case QwtPlot::xBottom:  axis_widget = axis_widget_xbottom;  break;
    case QwtPlot::xTop:     axis_widget = axis_widget_xtop;     break;
    default: return QColor();
    }
    if (axis_widget == 0) return QColor();

    return axis_widget->palette().color(QPalette::Text);
}

bool AxisDialog::axisAutoRescale(QwtPlot::Axis axis)
{
    switch (axis)
    {
    case QwtPlot::yLeft:    return auto_rescale_yleft;
    case QwtPlot::xBottom:  return auto_rescale_xbottom;
    case QwtPlot::yRight:   return auto_rescale_yright;
    case QwtPlot::xTop:     return auto_rescale_xtop;
    }

    return 0;
}

QPair<double, double> AxisDialog::getAxisScaleRange(QwtPlot::Axis axis)
{
    AxisRange *axis_range;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_range = &range_yleft;    break;
    case QwtPlot::xBottom:  axis_range = &range_xbottom;  break;
    case QwtPlot::yRight:   axis_range = &range_yright;   break;
    case QwtPlot::xTop:     axis_range = &range_xtop;     break;
    default: return QPair<double,double>(0,0);
    }

    double from = axis_range->min_value;
    double to = axis_range->max_value;

    return QPair<double,double>(from, to);
}

double AxisDialog::getAxisScaleStep(QwtPlot::Axis axis)
{
    AxisRange *axis_range;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_range = &range_yleft;    break;
    case QwtPlot::xBottom:  axis_range = &range_xbottom;  break;
    case QwtPlot::yRight:   axis_range = &range_yright;   break;
    case QwtPlot::xTop:     axis_range = &range_xtop;     break;
    default: return 0;
    }

    return axis_range->step;
}

QwtScaleEngine* AxisDialog::getAxisScaleType(QwtPlot::Axis axis)
{
    AxisScale *axis_scale = 0;
    switch (axis)
    {
    case QwtPlot::yLeft:    axis_scale = &scale_yleft;    break;
    case QwtPlot::xBottom:  axis_scale = &scale_xbottom;  break;
    case QwtPlot::yRight:   axis_scale = &scale_yright;   break;
    case QwtPlot::xTop:     axis_scale = &scale_xtop;     break;
    }

    if (axis_scale == 0) return 0;

    if (*axis_scale == Linear_Scale) return new QwtLinearScaleEngine;
    else if (*axis_scale == Log10_Scale) return new QwtLogScaleEngine;
	else return NULL;
}

int AxisDialog::getAxisTicks(QwtPlot::Axis axis)
{
    switch (axis)
    {
    case QwtPlot::yLeft:    return ticks_yleft;
    case QwtPlot::xBottom:  return ticks_xbottom;
    case QwtPlot::yRight:   return ticks_yright;
    case QwtPlot::xTop:     return ticks_xtop;
    }

    return 0;
}

void AxisDialog::hideRangeSettings(bool flag)
{
	gboxRange->setVisible(!flag);	
}