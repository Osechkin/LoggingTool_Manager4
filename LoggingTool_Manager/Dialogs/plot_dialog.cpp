#include <QColorDialog>

#include "qwt_plot.h"

#include "plot_dialog.h"


PlotDialog::PlotDialog(QwtPlot *_plot, QwtPlotGrid *_grid, QWidget *parent) : QDialog(parent)
{
    this->setupUi(this);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);   
    this->setParent(parent);

    this->setWindowTitle(tr("Plot Settings Dialog"));
    this->setModal(true);
    setWindowFlags(Qt::Dialog|Qt::WindowTitleHint); // в заголовке окна остается только кнопка закрытия окна

    tedTitle->setLineWrapMode(QTextEdit::NoWrap);
    tedTitle->setAlignment(Qt::AlignLeft);

    QStringList sizes;
    sizes << "4" << "5" << "6" << "8" << "9" << "10" << "11" << "12" << "14" << "16"
          << "18" << "20" << "22" << "24" << "28" << "32" << "36" << "42" << "48";
    cboxSize->addItems(sizes);

    cboxLineType->addItems(QStringList() << tr("SolidLine") << tr("DashLine") << tr("DotLine") << tr("DashDotLine") << tr("DashDotDotLine"));

    QPalette p_dblue = gbxTitle->palette();
    QPalette p_black = cboxSize->palette();
    p_dblue.setColor(QPalette::Text, QColor(Qt::darkBlue));
    p_black.setColor(QPalette::Text, QColor(Qt::black));
    gbxTitle->setPalette(p_dblue);
    gbxBack->setPalette(p_dblue);
    gbxGrid->setPalette(p_dblue);
    font_cboxTitle->setPalette(p_black);
    cboxSize->setPalette(p_black);
    gboxMajor->setPalette(p_dblue);
    cboxLineType->setPalette(p_black);
    dsboxLineWidth->setPalette(p_black);

    QStringList item_list;
    item_list << tr("Major Grid") << tr("Minor Grid");
    QStringList icon_list;
    icon_list << "grid_major.png" << "grid_minor.png";
    listWidgetGrids->setIconSize(QSize(40, 40));
    listWidgetGrids->setFlow(QListView::TopToBottom);

    int i = 0;
    foreach (QString str, item_list)
    {
        QListWidgetItem *pitem = new QListWidgetItem(str, listWidgetGrids, QListWidgetItem::UserType);
        pitem->setIcon(QIcon(":/images/" + icon_list[i++]));
        pitem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        grid_list.append(pitem);
    }
    listWidgetGrids->setCurrentItem(grid_list.first());

    tbtTitleColor->setIcon(QIcon(":/images/text_color.png"));
    tbtBackColor->setIcon(QIcon(":/images/color.png"));
    tbtLineColor->setIcon(QIcon(":/images/color.png"));

    qwtPlot = new QwtPlot;    
    qwtPlot->setTitle(_plot->title());
    qwtPlot->setCanvasBackground(_plot->canvasBackground());

    grid = new QwtPlotGrid;
    grid->enableX(_grid->xEnabled());
    grid->enableXMin(_grid->xMinEnabled());
    grid->enableY(_grid->yEnabled());
    grid->enableYMin(_grid->yMinEnabled());

    grid->setMajorPen(_grid->majorPen());
    grid->setMinorPen(_grid->minorPen());

    active_grid = "major";
    initTitle();
    initBackground();
    initGrid(active_grid);

    setConnections();
}

PlotDialog::~PlotDialog()
{
	delete grid;
	delete qwtPlot;
}

void PlotDialog::setConnections()
{
    connect(tedTitle, SIGNAL(textChanged()), this, SLOT(setTitleText()));
    connect(font_cboxTitle, SIGNAL(currentFontChanged(QFont)), this, SLOT(setTitleFont(QFont)));
    connect(cboxSize, SIGNAL(currentIndexChanged(QString)), this, SLOT(setTitleFontSize(QString)));
    connect(tbtBoldTitle, SIGNAL(clicked(bool)), this, SLOT(setBoldTitle(bool)));
    connect(tbtItalicTitle, SIGNAL(clicked(bool)), this, SLOT(setItalicTitle(bool)));
    connect(tbtUnderlinedTitle, SIGNAL(clicked(bool)), this, SLOT(setUnderlinedTitle(bool)));
	connect(tbtTitleColor, SIGNAL(clicked()), this, SLOT(setTitleColor()));

    connect(tbtBackColor, SIGNAL(clicked()), this, SLOT(setBackgroundColor()));

    connect(chbxShowHorizontal, SIGNAL(toggled(bool)), this, SLOT(setGridEnabled(bool)));
    connect(chbxShowVertical, SIGNAL(toggled(bool)), this, SLOT(setGridEnabled(bool)));
    connect(cboxLineType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setGridLineType(QString)));
    connect(dsboxLineWidth, SIGNAL(valueChanged(double)), this, SLOT(setGridLineWidth(double)));
	connect(tbtLineColor, SIGNAL(clicked()), this, SLOT(setGridLineColor()));

	connect(listWidgetGrids, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectListWidgetItem(QListWidgetItem*)));
}

void PlotDialog::initTitle()
{
    QwtText title = qwtPlot->title();

    QColor clr_title = title.color();
    QFont font_title = title.font();
    QString str_title = title.text();
    QPalette p_title; p_title.setColor(QPalette::Text, clr_title);

    tedTitle->setFont(font_title);
    tedTitle->setText(str_title);
    tedTitle->setPalette(p_title);

    font_cboxTitle->setCurrentFont(font_title);
    //QString font_size = QString::number(font_title.pointSize());
    cboxSize->setCurrentIndex(font_title.pointSize());
    tbtBoldTitle->setChecked(font_title.bold());
    tbtItalicTitle->setChecked(font_title.italic());
    tbtUnderlinedTitle->setChecked(font_title.underline());
}

void PlotDialog::initBackground()
{
    QColor bk_color = qwtPlot->canvasBackground().color();
    QPalette p = ledBackColor->palette();
    p.setBrush(ledBackColor->backgroundRole(), QBrush(bk_color));
    ledBackColor->setPalette(p);
}

void PlotDialog::initGrid(QString grid_type)
{
    bool x_enabled;
    bool y_enabled;
    Qt::PenStyle line_type;
    int line_width;
    QColor line_color;
    if (grid_type.toLower() == "major")
    {
        x_enabled = grid->xEnabled();
        y_enabled = grid->yEnabled();
        line_type = grid->majorPen().style();
        line_width = grid->majorPen().width();
        line_color = grid->majorPen().color();
    }
    else if (grid_type.toLower() == "minor")
    {
        x_enabled = grid->xMinEnabled();
        y_enabled = grid->yMinEnabled();
        line_type = grid->minorPen().style();
        line_width = grid->minorPen().width();
        line_color = grid->minorPen().color();
    }
    else return;

    chbxShowHorizontal->setChecked(x_enabled);
    chbxShowVertical->setChecked(y_enabled);

    switch (line_type)
    {
    case Qt::SolidLine:
        cboxLineType->setCurrentIndex(0);
        break;
    case Qt::DashLine:
        cboxLineType->setCurrentIndex(1);
        break;
    case Qt::DotLine:
        cboxLineType->setCurrentIndex(2);
        break;
    case Qt::DashDotLine:
        cboxLineType->setCurrentIndex(3);
        break;
    case Qt::DashDotDotLine:
        cboxLineType->setCurrentIndex(4);
        break;
    default:
        cboxLineType->setCurrentIndex(0);
        break;
    }

    QPalette p = ledLineColor->palette();
    p.setBrush(ledLineColor->backgroundRole(), QBrush(line_color));
    ledLineColor->setPalette(p);

    dsboxLineWidth->setValue(line_width);
}

void PlotDialog::setActiveGrid(QString grid_type)
{
    if (grid_type.toLower() == "major") active_grid = "major";
    else if (grid_type.toLower() == "minor") active_grid = "minor";
    else return;

    initTitle();
    initBackground();
    initGrid(active_grid);
}

void PlotDialog::selectListWidgetItem(QListWidgetItem *item)
{
    if (item->text() == "Major Grid") setActiveGrid("major");
    else if (item->text() == "Minor Grid") setActiveGrid("minor");
}

void PlotDialog::setTitleFont(QFont font)
{
    QwtText title = qwtPlot->title();
    QFont title_font = title.font();
    title_font.setFamily(font.family());
    title.setFont(title_font);
    qwtPlot->setTitle(title);
    tedTitle->setFont(title.font());
    tedTitle->setText(title.text());
}

void PlotDialog::setTitleFontSize(QString str)
{
    bool ok;
    int size = str.toInt(&ok);
    if (!ok) return;

    QwtText title = qwtPlot->title();
    QFont font = title.font();
    font.setPointSize(size);
    title.setFont(font);
    tedTitle->setFont(title.font());
    tedTitle->setText(title.text());
    qwtPlot->setTitle(title);
}

void PlotDialog::setTitleText()
{
    QwtText title = qwtPlot->title();

    QString title_text = tedTitle->toPlainText();
    title.setText(title_text);

    qwtPlot->setTitle(title);
}

void PlotDialog::setBoldTitle(bool flag)
{
    QwtText title = qwtPlot->title();

    QFont font_title = title.font();
    font_title.setBold(flag);
    title.setFont(font_title);

    tedTitle->setFont(title.font());
    tedTitle->setText(title.text());
    qwtPlot->setTitle(title);
}

void PlotDialog::setItalicTitle(bool flag)
{
    QwtText title = qwtPlot->title();

    QFont font_title = title.font();
    font_title.setItalic(flag);
    title.setFont(font_title);

    tedTitle->setFont(title.font());
    tedTitle->setText(title.text());
    qwtPlot->setTitle(title);
}

void PlotDialog::setUnderlinedTitle(bool flag)
{
    QwtText title = qwtPlot->title();

    QFont font_title = title.font();
    font_title.setUnderline(flag);
    title.setFont(font_title);

    tedTitle->setFont(title.font());
    tedTitle->setText(title.text());
    qwtPlot->setTitle(title);
}

void PlotDialog::setTitleColor()
{
	QColor title_color = qwtPlot->title().color();
	title_color = QColorDialog::getColor(title_color);
	if (title_color.isValid())
	{
		qwtPlot->title().setColor(title_color);
		QPalette p_title = tedTitle->palette();
		p_title.setColor(QPalette::Text, title_color);
		tedTitle->setPalette(p_title);
	}
}

void PlotDialog::setBackgroundColor()
{
    QColor bk_color = qwtPlot->canvasBackground().color();
    bk_color = QColorDialog::getColor(bk_color);
    if (bk_color.isValid())
    {
        QPalette p = ledBackColor->palette();
        p.setBrush(ledBackColor->backgroundRole(), QBrush(bk_color));
        ledBackColor->setPalette(p);

        qwtPlot->setCanvasBackground(QBrush(bk_color));
    }
}

void PlotDialog::setGridEnabled(bool flag)
{
    QCheckBox *chbx = (QCheckBox*)sender();
    if (!chbx) return;

    if (chbx == chbxShowHorizontal)
    {
        if (active_grid == "major") grid->enableX(flag);
        else if (active_grid == "minor") grid->enableXMin(flag);
    }
    else if (chbx == chbxShowVertical)
    {
        if (active_grid == "major") grid->enableY(flag);
        else if (active_grid == "minor") grid->enableYMin(flag);
    }
}

void PlotDialog::setGridLineType(QString str)
{
    QPen pen;
    if (active_grid == "major") pen = grid->majorPen();
    else if (active_grid == "minor") pen = grid->minorPen();
    else return;

    if (str == "SolidLine") pen.setStyle(Qt::SolidLine);
    else if (str == "DashLine") pen.setStyle(Qt::DashLine);
    else if (str == "DotLine") pen.setStyle(Qt::DotLine);
    else if (str == "DashDotLine") pen.setStyle(Qt::DashDotLine);
    else if (str == "DashDotDotLine") pen.setStyle(Qt::DashDotDotLine);

    if (active_grid == "major") grid->setMajorPen(pen);
    else if (active_grid == "minor") grid->setMinorPen(pen);
}

void PlotDialog::setGridLineWidth(double val)
{
    QPen pen;
    if (active_grid == "major") pen = grid->majorPen();
    else if (active_grid == "minor") pen = grid->minorPen();
    else return;

    pen.setWidth(val);

    if (active_grid == "major") grid->setMajorPen(pen);
    else if (active_grid == "minor") grid->setMinorPen(pen);
}

void PlotDialog::setGridLineColor()
{
    QPen pen;
    if (active_grid == "major") pen = grid->majorPen();
    else if (active_grid == "minor") pen = grid->minorPen();
    else return;

    QColor line_color = pen.color();
    line_color = QColorDialog::getColor(line_color);
    if (line_color.isValid())
    {
        QPalette p = ledLineColor->palette();
        p.setBrush(ledLineColor->backgroundRole(), QBrush(line_color));
        ledLineColor->setPalette(p);

        pen.setColor(line_color);

        if (active_grid == "major") grid->setMajorPen(pen);
        else if (active_grid == "minor") grid->setMinorPen(pen);
    }
}
