#ifndef SETTINGS_TREE_H
#define SETTINGS_TREE_H

#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <boost\shared_ptr.hpp>
#include <boost\make_shared.hpp>


enum CDataType { String_Data, Double_Data, Int_Data, Bool_Data };



struct CSettings
{
	CSettings(QString _type, QVariant _value, QString obj_name = "");

	QString name; 

    QString type;
    CDataType data_type;
    QVariant value;
    QPair<double,double> min_max;
    QFont font;
    QColor text_color;
    QColor active_text_color;
    QColor background_color;
    QColor alter_background_color;
    bool read_only;
    bool checkable;
    bool check_state;
    bool title_flag;
	bool set_frame;	
    QIcon icon;
    int frame_width;
    int frame_height;
	QString hint;

	QWidget *widget;
};



class CTextEdit : public QLineEdit
{
    Q_OBJECT

public:
    CTextEdit(QWidget *parent = 0);
    CTextEdit(QString _text, QWidget *parent = 0);
    CTextEdit(QString _text, QString _name, QWidget *parent = 0);
    CTextEdit(QSize _size, QWidget *parent = 0);
    CTextEdit(QString _text, QSize _size, QWidget *parent = 0);
    CTextEdit(CSettings _settings, QWidget *parent = 0);

    void setReadOnly(bool ok);
    QSize sizeHint() const;
    void setActiveTextColor(QColor _color);
    void setReadOnlyColor(QColor _color);
    void setBackgroundColor(QColor _color);
    void setAlterBackgroundColor(QColor _color);
    void repaintAll();

    void setDataType(CDataType _type);

    void setFrameWidth(int _val);
    void setFrameHeight(int _val);
    void setFrameSize(QSize _size);

    bool isReadOnly() const { return read_only; }
    QColor getReadOnlyColor() { return readonly_Color; }
    QColor getEditedTextColor() { return activetext_Color; }
    QColor getBackgroundColor() { return background_Color; }
    QColor getAlterBackgroundColor() { return alter_background_Color; }

    int getFrameWidth() { return frame_width; }
    int getFrameHeight() { return frame_height; }
    QSize getFrameSize() { return QSize(frame_width, frame_height); }

private:    
    CDataType data_type;
    bool read_only;
    QColor activetext_Color;
    QColor readonly_Color;
    QColor background_Color;
    QColor alter_background_Color;

    int frame_width;
    int frame_height;
};



class CLabel : public QLabel
{
    Q_OBJECT

public:
    CLabel(QWidget *parent = 0);
    CLabel(QString _text, QWidget *parent = 0);
    CLabel(QString _text, QString _name, QWidget *parent = 0);
    CLabel(QString _text, QSize _size, QWidget *parent = 0);
    CLabel(QString _text, QString _name, QSize _size, QWidget *parent = 0);

    QSize sizeHint() const;
    void setTextColor(QColor _color);
    void setBackgroundColor(QColor _color);
    void setAlterBackgroundColor(QColor _color);
    void repaintAll();

    void setFrame(bool flag);
    void setFrameWidth(int _val);
    void setFrameHeight(int _val);
    void setFrameSize(QSize _size);

    QColor getTextColor() { return text_Color; }
    QColor getBackgroundColor() { return background_Color; }
    QColor getAlterBackgroundColor() { return alter_background_Color; }

    int getFrameWidth() { return frame_width; }
    int getFrameHeight() { return frame_height; }
    QSize getFrameSize() { return QSize(frame_width, frame_height); }

private:
    QColor text_Color;
    QColor background_Color;
    QColor alter_background_Color;

    int frame_width;
    int frame_height;
};


class CComboBox : public QComboBox
{
    Q_OBJECT

public:
    CComboBox(QWidget *parent = 0);
    CComboBox(QStringList _list, QWidget *parent = 0);
    CComboBox(QStringList _list, QString _name, QWidget *parent = 0);
    CComboBox(QSize _size, QWidget *parent = 0);
    CComboBox(QStringList _list, QSize _size, QWidget *parent = 0);

    QSize sizeHint() const;
    void setTextColor(QColor _color);
    void setBackgroundColor(QColor _color);
    void setAlterBackgroundColor(QColor _color);
    void repaintAll();

    void setFrameWidth(int _val);
    void setFrameHeight(int _val);
    void setFrameSize(QSize _size);

    QColor getTextColor() { return text_Color; }
    QColor getBackgroundColor() { return background_Color; }
    QColor getAlterBackgroundColor() { return alter_background_Color; }

    int getFrameWidth() { return frame_width; }
    int getFrameHeight() { return frame_height; }
    QSize getFrameSize() { return QSize(frame_width, frame_height); }

private:
    QColor text_Color;
    QColor background_Color;
    QColor alter_background_Color;

    int frame_width;
    int frame_height;
};



class CDSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    CDSpinBox(QWidget *parent = 0);
    CDSpinBox(double _val, QWidget *parent = 0);
    CDSpinBox(double _val, QString _name, QWidget *parent = 0);
    CDSpinBox(QSize _size, QWidget *parent = 0);
    CDSpinBox(double _val, QSize _size, QWidget *parent = 0);

    QSize sizeHint() const;
    void setTextColor(QColor _color);
    void setBackgroundColor(QColor _color);
    void setAlterBackgroundColor(QColor _color);
    void repaintAll();

    void setFrameWidth(int _val);
    void setFrameHeight(int _val);
    void setFrameSize(QSize _size);

    QColor getTextColor() { return text_Color; }
    QColor getBackgroundColor() { return background_Color; }
    QColor getAlterBackgroundColor() { return alter_background_Color; }

    int getFrameWidth() { return frame_width; }
    int getFrameHeight() { return frame_height; }
    QSize getFrameSize() { return QSize(frame_width, frame_height); }

private:
    int frame_width;
    int frame_height;

    QColor text_Color;
    QColor background_Color;
    QColor alter_background_Color;
};


class CSpinBox : public QSpinBox
{
	Q_OBJECT

public:
	CSpinBox(QWidget *parent = 0);
	CSpinBox(int _val, QWidget *parent = 0);
	CSpinBox(int _val, QString _name, QWidget *parent = 0);
	CSpinBox(QSize _size, QWidget *parent = 0);
	CSpinBox(int _val, QSize _size, QWidget *parent = 0);

	QSize sizeHint() const;
	void setTextColor(QColor _color);
	void setBackgroundColor(QColor _color);
	void setAlterBackgroundColor(QColor _color);
	void repaintAll();

	void setFrameWidth(int _val);
	void setFrameHeight(int _val);
	void setFrameSize(QSize _size);

	QColor getTextColor() { return text_Color; }
	QColor getBackgroundColor() { return background_Color; }
	QColor getAlterBackgroundColor() { return alter_background_Color; }

	int getFrameWidth() { return frame_width; }
	int getFrameHeight() { return frame_height; }
	QSize getFrameSize() { return QSize(frame_width, frame_height); }

private:
	int frame_width;
	int frame_height;

	QColor text_Color;
	QColor background_Color;
	QColor alter_background_Color;
};



class CCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    CCheckBox(QWidget *parent = 0);
    CCheckBox(QString _text, QWidget *parent = 0);
    CCheckBox(QString _text, QString _name, QWidget *parent = 0);
    CCheckBox(QSize _size, QWidget *parent = 0);
    CCheckBox(QString _text, QSize _size, QWidget *parent = 0);

    QSize sizeHint() const;    
    void setTextColor(QColor _color);
    void setBackgroundColor(QColor _color);
    void setAlterBackgroundColor(QColor _color);
    void repaintAll();

    void setFrameWidth(int _val);
    void setFrameHeight(int _val);
    void setFrameSize(QSize _size);

    QColor getTextColor() { return text_Color; }
    QColor getBackgroundColor() { return background_Color; }
    QColor getAlterBackgroundColor() { return alter_background_Color; }

    int getFrameWidth() { return frame_width; }
    int getFrameHeight() { return frame_height; }
    QSize getFrameSize() { return QSize(frame_width, frame_height); }

private:
    int frame_width;
    int frame_height;

    QColor text_Color;
    QColor background_Color;
    QColor alter_background_Color;
};

class CWidget : public QWidget
{
	Q_OBJECT

public:
	CWidget(QWidget *_widget, QWidget *parent = 0);	

	QSize sizeHint() const;    	
	void repaintAll() { };

	void setFrameWidth(int _val);
	void setFrameHeight(int _val);
	void setFrameSize(QSize _size);
		
	int getFrameWidth() { return frame_width; }
	int getFrameHeight() { return frame_height; }
	QSize getFrameSize() { return QSize(frame_width, frame_height); }

	QWidget *getWidget() { return widget; }

private:
	int frame_width;
	int frame_height;

	QWidget *widget;
};



class CTreeWidgetItem : public QObject
{
    Q_OBJECT

public:    
    CTreeWidgetItem(QTreeWidget *_tw, QTreeWidgetItem *_twi, QList<CSettings> _settings);
    ~CTreeWidgetItem();

    void show();
	void linkObject(void *obj) { linkedObject = obj; } 

    QList<QWidget*> getCWidgets() { return c_objects; }
    QList<CSettings> getCSettings() { return c_settings; }

    QTreeWidget *getQTreeWidget() { return tw; }
    QTreeWidgetItem *getQTreeWidgetItem() { return twi; }
    QTreeWidgetItem *getQSubTreeWidgetItem() { return sub_twi; }
	//boost::shared_ptr<QTreeWidgetItem> getQSubTreeWidgetItem() { return sub_twi; }
	void *getLinkedObject() { return linkedObject; }

private:
    void initObjects();

    QTreeWidget *tw;
    QTreeWidgetItem *twi;
    QTreeWidgetItem *sub_twi;
	//boost::shared_ptr<QTreeWidgetItem> sub_twi;


    QList<QWidget*> c_objects;
    QList<CSettings> c_settings;

	void *linkedObject;		// объект, с которым ассоциирован данный объект CTreeWidgetItem (например, для удобства отображения в linkedObject изменений в CTreeWidgetItem)

private slots:
	void valueChanged(int value);
	void valueChanged(double value);
	void valueChanged(QString value);
	void valueChanged(bool value);
	void changingFinished();

signals:
	void value_changed(QObject*, QVariant&);
	void editing_finished(QObject*);
};

#endif // SETTINGS_TREE_H
