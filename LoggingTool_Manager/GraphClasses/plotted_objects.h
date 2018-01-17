#ifndef PLOTTED_OBJECTS_H
#define PLOTTED_OBJECTS_H

#include <QtGui>
#include <QWidget>
#include <QTreeWidget>

#include "qwt_symbol.h"

#include "../Common/settings_tree.h"
#include "graph_classes.h"
#include "../Common/data_containers.h"

#include <boost\shared_ptr.hpp>
#include <boost\make_shared.hpp>


class DataPlot;


static bool isNAN(double val) { return !(val == val); }


struct QwtCurveSettings
{
	enum DataType { Lines, Symbols, LinesAndSymbols, None };

	QwtCurveSettings() 
	{ 
		symbol = new QwtSymbol;
		title = "";
		dataType = LinesAndSymbols;
		z = 0;
	}
	~QwtCurveSettings() { delete symbol; }	

	QString title;
	QwtSymbol *symbol;
	QPen pen;
	DataType dataType;
	int z;
};

class CurveSettings
{
public:
	enum SymbolStyle { Circle, Rect, Diamond, DTriangle, UTriangle, LTriangle, RTriangle, Cross, XCross };
	enum DataType { Lines, Symbols, LinesAndSymbols, None };

	DataType dataType; 
	int symSize;
	int penWidth;
	SymbolStyle symStyle; 
	int symEdge;
	Qt::PenStyle penStyle; 
	QColor symColor;
	QColor penColor;
	QColor symEdgeColor;
};


class DefaultCurveSettings : public CurveSettings
{
public:
	DefaultCurveSettings()
	{
		dataType = CurveSettings::LinesAndSymbols;
		penStyle = Qt::SolidLine;
		penColor = QColor(Qt::red);
		penWidth = 1;
		symStyle = CurveSettings::Circle;
		symSize = 8;
		symColor = QColor(Qt::blue);
		symEdge = 1;
		symEdgeColor = QColor(Qt::red);
	}
};



class PaintFrame : public QWidget
{
	Q_OBJECT

public:
	PaintFrame(QWidget *parent = 0);
	~PaintFrame();

	void setLineSettings(int penWidth, QColor penColor); 
	void setLineStyle(Qt::PenStyle style);
	//void setSymbolSettings(CurveSettings::SymbolStyle symStyle, int symSize, int symEdge, QColor symColor, QColor symEdgeColor);
	void setSymbolSettings(QwtSymbol::Style symStyle, int symSize, int symEdge, QColor symColor, QColor symEdgeColor);
	void setDataType(QwtCurveSettings::DataType _dataType);
		
	QwtCurveSettings::DataType getDataType() { return curve_settings->dataType; }
	Qt::PenStyle getLineStyle() { return curve_settings->pen.style(); }
	QColor getLineColor() { return curve_settings->pen.color(); }
	int getLineWidth() { return curve_settings->pen.width(); }
	QwtSymbol::Style getSymbolStyle() { return curve_settings->symbol->style(); }
	QColor getSymbolColor() { return curve_settings->symbol->brush().color(); }
	int getSymbolSize() { return curve_settings->symbol->size().width(); }
	QColor getSymbolEdgeColor() { return curve_settings->symbol->pen().color(); }
	int getSymbolEdgeWidth() { return curve_settings->symbol->pen().width(); }

	//Qt::PenStyle getLineStyle(int style);    

private:
	/*CurveSettings::DataType dataType; 
	int symSize;
	int penWidth;
	CurveSettings::SymbolStyle symStyle; 
	int symEdge;
	Qt::PenStyle penStyle; 
	QColor symColor;
	QColor penColor;
	QColor symEdgeColor;*/

	QwtCurveSettings *curve_settings;

	//void drawDataSymbol(QPainter *painter, CurveSettings::SymbolStyle symType);
	void drawDataSymbol(QPainter *painter, QwtSymbol::Style symType);

protected:
	void paintEvent(QPaintEvent *);
};


class PlottedDataSet : public QObject
{
	Q_OBJECT

public:
	PlottedDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QObject *parent = 0);
	PlottedDataSet(QString _name, DataPlot *_plot, QVector<double> *_x, QVector<double> *_y, QVector<uint8_t> *_bad_map, QwtCurveSettings *_settings, QObject *parent = 0);
	~PlottedDataSet();

	bool isMarked() { return mark; }
	void markDataSet(bool flag) { mark = flag; }
	int getID() { return id; }
	void setID(int _id) { id = _id; }
	QString getName() { return name; }
	void setName(QString _name) { name = _name; }
	PaintFrame *getPaintFrame() { return pframe; }
	int getPointsCount() { return points_count; }
	bool isHeldOn() { return hold_on; }
	void holdDataSetOn(bool flag) { hold_on = flag; }	
	QString getTargetPlotName() { return data_plot_name; }
	void setTypeId(int _type_id) { type_id = _type_id; }
	int getTypeId() { return type_id; }

	void linkCTreeWidgetItem(CTreeWidgetItem *_c_twi) { c_twi = _c_twi; }
	CTreeWidgetItem *getCTreeWidgetItem() { return c_twi; }
	void setParentDataSet(PlottedDataSet *_ds) { parent_dataset = _ds; }
	PlottedDataSet *getParentDataSet() { return parent_dataset; }

	void setBasedDataSet(DataSet *ds) { data_set = ds; }
	DataSet *getBasedDataSet() { return data_set; }

	QVector<double> *getXData() { return x; }
	QVector<double> *getYData() { return y; }
	QVector<uint8_t> *getBadMap() { return bad_map; }

	QwtCurveSettings *getCurveSettings() { return curve_settings; }	
	void setDefaultCurveSettings(QwtCurveSettings *_def_curve_settings, int index);
	void setDefaultCurveSettings(int index);
	void setCurveSettings(QwtCurveSettings *_curve_settings) { curve_settings = _curve_settings; }

private:
	int generateID();

	bool mark;
	int id;
	int type_id;			// идентификатор типа кривой для наследования цвета и других свойств между данными
	QString name;
	PaintFrame *pframe;
	int points_count;
	bool hold_on;			// идентификатор постоянного объекта (существует пока hold_on = true)
	QString data_plot_name;
	DataPlot *data_plot;

	QwtCurveSettings *curve_settings;		

	QVector<double> *x;
	QVector<double> *y;
	QVector<uint8_t> *bad_map;

	CTreeWidgetItem *c_twi;
	PlottedDataSet *parent_dataset;
	DataSet *data_set;		// контейнер данных, на основе которого создан данный PlottedDataSet

private slots:
	void changeDataSetState(QObject *obj, QVariant &value);
};



#endif // PLOTTED_OBJECTS_H
