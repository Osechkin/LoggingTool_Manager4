#include <QDoubleValidator>
#include <QLabel>
#include <QStyleFactory>

#include "settings_tree.h"


CTextEdit::CTextEdit(QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    //QRegExp regExp("^[eE0-9,.+-]{0,}$");
    //setValidator(new QRegExpValidator(regExp,this));
    setDataType(String_Data);

    frame_height = 20;
    frame_width = 70;
	
    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->setFont(QFont("Arial", 10));

    setFrame(false);
	
    activetext_Color = QColor(Qt::darkMagenta);
    readonly_Color = QColor(Qt::black);
    background_Color = QColor(Qt::white);

    setReadOnly(false);

    repaintAll();
}

CTextEdit::CTextEdit(QString _text, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = 20;
    frame_width = 70;

    setDataType(String_Data);

    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->setFont(QFont("Arial", 10));

    setFrame(false);

    activetext_Color = QColor(Qt::darkMagenta);
    readonly_Color = QColor(Qt::black);
    background_Color = QColor(Qt::white);

    setText(_text);
    setReadOnly(false);

    repaintAll();
}

CTextEdit::CTextEdit(QString _text, QString _name, QWidget *parent)
{
    this->setParent(parent);
    this->setObjectName(_name);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = 20;
    frame_width = 70;

    setDataType(String_Data);

    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->setFont(QFont("Arial", 10));

    setFrame(false);

    activetext_Color = QColor(Qt::darkMagenta);
    readonly_Color = QColor(Qt::black);
    background_Color = QColor(Qt::white);

    setText(_text);
    setReadOnly(false);

    repaintAll();
}

CTextEdit::CTextEdit(QString _text, QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = _size.height();
    frame_width = _size.width();

    setDataType(String_Data);

    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->setFont(QFont("Arial", 10));

    setFrame(false);

    activetext_Color = QColor(Qt::darkMagenta);
    readonly_Color = QColor(Qt::black);
    background_Color = QColor(Qt::white);

    setText(_text);
    setReadOnly(false);

    repaintAll();
}

CTextEdit::CTextEdit(QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = _size.height();
    frame_width = _size.width();

    setDataType(String_Data);

    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->setFont(QFont("Arial", 10));

    setFrame(false);

    activetext_Color = QColor(Qt::darkMagenta);
    readonly_Color = QColor(Qt::black);
    background_Color = QColor(Qt::white);

    setReadOnly(false);

    repaintAll();
}

CTextEdit::CTextEdit(CSettings _settings, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_width = _settings.frame_width;
    frame_height = _settings.frame_height;

    setDataType(String_Data);

    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->setFont(QFont("Arial", 10));

    setFrame(false);

    activetext_Color = _settings.active_text_color;
    readonly_Color = _settings.text_color;
    background_Color = _settings.background_color;
    alter_background_Color = _settings.alter_background_color;

    setReadOnly(_settings.read_only);

    repaintAll();
}

QSize CTextEdit::sizeHint() const
{
    return QSize(frame_width, frame_height);
}

void CTextEdit::setReadOnly(bool ok)
{
    QLineEdit::setReadOnly(ok);
    read_only = ok;

    repaintAll();
}

void CTextEdit::setBackgroundColor(QColor _color)
{
    background_Color = _color;
    repaintAll();
}

void CTextEdit::setAlterBackgroundColor(QColor _color)
{
    alter_background_Color = _color;
    repaintAll();
}

void CTextEdit::setReadOnlyColor(QColor _color)
{
    readonly_Color = _color;
    repaintAll();
}

void CTextEdit::setActiveTextColor(QColor _color)
{
    activetext_Color = _color;
    repaintAll();
}

void CTextEdit::repaintAll()
{
    QPalette palette;
    palette.setColor(QPalette::Text, (read_only ? readonly_Color : activetext_Color) );
    palette.setColor(QPalette::Base, background_Color);
    palette.setColor(QPalette::AlternateBase, alter_background_Color);
    setPalette(palette);

    setText(this->text());
}

void CTextEdit::setDataType(CDataType _type)
{
    data_type = _type;

    switch (_type)
    {
    case String_Data:
    {
        QRegExp regExp("^[0-9,a-z,A-Z,à-ÿ,À-ß,!,@,#,$,%,^,&,*,(,),_,+,=,/,.]{0,}$");
        setValidator(new QRegExpValidator(regExp,this));
        break;
    }

    case Double_Data:
        this->setValidator(new QDoubleValidator());
        break;

    case Int_Data:
        this->setValidator(new QIntValidator());
        break;

    default:
        break;
    }
}

void CTextEdit::setFrameHeight(int _val)
{
    frame_height = _val;
}

void CTextEdit::setFrameWidth(int _val)
{
    frame_width = _val;
}

void CTextEdit::setFrameSize(QSize _size)
{
    frame_width = _size.width();
    frame_height = _size.height();
}


CLabel::CLabel(QWidget *parent)
{
    this->setParent(parent);
    this->setAutoFillBackground(true);
    this->setFrame(false);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));
    this->setText("");

    text_Color = QColor(Qt::darkMagenta);
    background_Color = QColor(Qt::white);
    alter_background_Color = QColor(Qt::lightGray);
}

CLabel::CLabel(QString _text, QWidget *parent)
{
    this->setParent(parent);
    this->setAutoFillBackground(true);
    this->setFrame(false);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));
    this->setText(_text);

    text_Color = QColor(Qt::darkMagenta);
    background_Color = QColor(Qt::white);
    alter_background_Color = QColor(Qt::lightGray);
}

CLabel::CLabel(QString _text, QString _name, QWidget *parent)
{
    this->setParent(parent);
    this->setObjectName(_name);
    this->setAutoFillBackground(true);
    this->setFrame(false);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));
    this->setText(_text);

    text_Color = QColor(Qt::darkMagenta);
    background_Color = QColor(Qt::white);
    alter_background_Color = QColor(Qt::lightGray);
}

CLabel::CLabel(QString _text, QString _name, QSize _size, QWidget *parent)
{
    this->setParent(parent);
    this->setObjectName(_name);
    this->setAutoFillBackground(true);
    this->setFrame(false);

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));
    this->setText(_text);

    text_Color = QColor(Qt::darkMagenta);
    background_Color = QColor(Qt::white);
    alter_background_Color = QColor(Qt::lightGray);
}

CLabel::CLabel(QString _text, QSize _size, QWidget *parent)
{
    this->setParent(parent);
    this->setAutoFillBackground(true);
    this->setFrame(false);

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));
    this->setText(_text);

    text_Color = QColor(Qt::darkMagenta);
    background_Color = QColor(Qt::white);
    alter_background_Color = QColor(Qt::lightGray);
}

QSize CLabel::sizeHint() const
{
    return QSize(frame_width, frame_height);
}


void CLabel::setBackgroundColor(QColor _color)
{
    background_Color = _color;
    repaintAll();
}

void CLabel::setAlterBackgroundColor(QColor _color)
{
    alter_background_Color = _color;
    repaintAll();
}

void CLabel::setTextColor(QColor _color)
{
    text_Color = _color;
    repaintAll();
}

void CLabel::repaintAll()
{
    QPalette palette;
    palette.setColor(QPalette::Text, (text_Color) );
    palette.setColor(QPalette::Base, background_Color);
    palette.setColor(QPalette::AlternateBase, alter_background_Color);
    setPalette(palette);

    setText(this->text());
}

void CLabel::setFrame(bool flag)
{
    if (flag) this->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    else this->setFrameStyle(QFrame::NoFrame);
}

void CLabel::setFrameHeight(int _val)
{
    frame_height = _val;
}

void CLabel::setFrameWidth(int _val)
{
    frame_width = _val;
}

void CLabel::setFrameSize(QSize _size)
{
    frame_width = _size.width();
    frame_height = _size.height();
}



CComboBox::CComboBox(QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));    
}

CComboBox::CComboBox(QStringList _list, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));

    this->addItems(_list);
}

CComboBox::CComboBox(QStringList _list, QString _name, QWidget *parent)
{
    this->setParent(parent);
    this->setObjectName(_name);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));

    this->addItems(_list);
}

CComboBox::CComboBox(QStringList _list, QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));

    this->addItems(_list);
}

CComboBox::CComboBox(QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));
}

QSize CComboBox::sizeHint() const
{
    return QSize(frame_width, frame_height);
}

void CComboBox::setFrameHeight(int _val)
{
    frame_height = _val;
}

void CComboBox::setFrameWidth(int _val)
{
    frame_width = _val;
}

void CComboBox::setFrameSize(QSize _size)
{
    frame_width = _size.width();
    frame_height = _size.height();
}

void CComboBox::setBackgroundColor(QColor _color)
{
    background_Color = _color;
    repaintAll();
}

void CComboBox::setAlterBackgroundColor(QColor _color)
{
    alter_background_Color = _color;
    repaintAll();
}

void CComboBox::setTextColor(QColor _color)
{
    text_Color = _color;
    repaintAll();
}

void CComboBox::repaintAll()
{
    QPalette palette;
    palette.setColor(QPalette::Text, text_Color);
    palette.setColor(QPalette::ButtonText, text_Color);
    palette.setColor(QPalette::Button, background_Color);
    palette.setColor(QPalette::Base, background_Color);
    palette.setColor(QPalette::AlternateBase, alter_background_Color);
    setPalette(palette);
}



CDSpinBox::CDSpinBox(QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));
	this->setDecimals(1);
	this->setSingleStep(1.0);

    repaintAll();
}

CDSpinBox::CDSpinBox(double _val, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));

	this->setDecimals(1);
	this->setSingleStep(1.0);

	if (_val < this->minimum()) this->setMinimum(_val);
	if (_val > this->maximum()) this->setMaximum(_val);
    this->setValue(_val);

    repaintAll();
}

CDSpinBox::CDSpinBox(double _val, QString _name, QWidget *parent)
{
    this->setParent(parent);
    this->setObjectName(_name);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));

	this->setDecimals(1);
	this->setSingleStep(1.0);

	if (_val < this->minimum()) this->setMinimum(_val);
	if (_val > this->maximum()) this->setMaximum(_val);
    this->setValue(_val);

    repaintAll();
}

CDSpinBox::CDSpinBox(double _val, QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));

	this->setDecimals(1);
	this->setSingleStep(1.0);

	if (_val < this->minimum()) this->setMinimum(_val);
	if (_val > this->maximum()) this->setMaximum(_val);
    this->setValue(_val);

    repaintAll();
}

CDSpinBox::CDSpinBox(QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    frame_height = _size.height();
    frame_width = _size.width();

	this->setDecimals(1);
	this->setSingleStep(1.0);

    this->setFont(QFont("Arial", 10));
	this->setDecimals(1);
	this->setSingleStep(1.0);

    repaintAll();
}

QSize CDSpinBox::sizeHint() const
{
    return QSize(frame_width, frame_height);
}

void CDSpinBox::setFrameHeight(int _val)
{
    frame_height = _val;
}

void CDSpinBox::setFrameWidth(int _val)
{
    frame_width = _val;
}

void CDSpinBox::setFrameSize(QSize _size)
{
    frame_width = _size.width();
    frame_height = _size.height();
}

void CDSpinBox::setBackgroundColor(QColor _color)
{
    background_Color = _color;
    repaintAll();
}

void CDSpinBox::setAlterBackgroundColor(QColor _color)
{
    alter_background_Color = _color;
    repaintAll();
}

void CDSpinBox::setTextColor(QColor _color)
{
    text_Color = _color;
    repaintAll();
}

void CDSpinBox::repaintAll()
{
    QPalette palette;
    palette.setColor(QPalette::Text, text_Color);
    palette.setColor(QPalette::Base, background_Color);
    palette.setColor(QPalette::AlternateBase, alter_background_Color);
    setPalette(palette);
}



CSpinBox::CSpinBox(QWidget *parent)
{
	this->setParent(parent);
	//this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
	this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	frame_height = 20;
	frame_width = 70;

	this->setFont(QFont("Arial", 10));	
	this->setSingleStep(1.0);

	repaintAll();
}

CSpinBox::CSpinBox(int _val, QWidget *parent)
{
	this->setParent(parent);
	//this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
	this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	frame_height = 20;
	frame_width = 70;

	this->setFont(QFont("Arial", 10));

	this->setSingleStep(1.0);

	if (_val < this->minimum()) this->setMinimum(_val);
	if (_val > this->maximum()) this->setMaximum(_val);
	this->setValue(_val);

	repaintAll();
}

CSpinBox::CSpinBox(int _val, QString _name, QWidget *parent)
{
	this->setParent(parent);
	this->setObjectName(_name);
	//this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
	this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	frame_height = 20;
	frame_width = 70;

	this->setFont(QFont("Arial", 10));

	this->setSingleStep(1.0);

	if (_val < this->minimum()) this->setMinimum(_val);
	if (_val > this->maximum()) this->setMaximum(_val);
	this->setValue(_val);

	repaintAll();
}

CSpinBox::CSpinBox(int _val, QSize _size, QWidget *parent)
{
	this->setParent(parent);
	//this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
	this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	frame_height = _size.height();
	frame_width = _size.width();

	this->setFont(QFont("Arial", 10));

	this->setSingleStep(1.0);

	if (_val < this->minimum()) this->setMinimum(_val);
	if (_val > this->maximum()) this->setMaximum(_val);
	this->setValue(_val);

	repaintAll();
}

CSpinBox::CSpinBox(QSize _size, QWidget *parent)
{
	this->setParent(parent);
	//this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
	this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	frame_height = _size.height();
	frame_width = _size.width();

	this->setSingleStep(1.0);

	this->setFont(QFont("Arial", 10));
	this->setSingleStep(1.0);

	repaintAll();
}

QSize CSpinBox::sizeHint() const
{
	return QSize(frame_width, frame_height);
}

void CSpinBox::setFrameHeight(int _val)
{
	frame_height = _val;
}

void CSpinBox::setFrameWidth(int _val)
{
	frame_width = _val;
}

void CSpinBox::setFrameSize(QSize _size)
{
	frame_width = _size.width();
	frame_height = _size.height();
}

void CSpinBox::setBackgroundColor(QColor _color)
{
	background_Color = _color;
	repaintAll();
}

void CSpinBox::setAlterBackgroundColor(QColor _color)
{
	alter_background_Color = _color;
	repaintAll();
}

void CSpinBox::setTextColor(QColor _color)
{
	text_Color = _color;
	repaintAll();
}

void CSpinBox::repaintAll()
{
	QPalette palette;
	palette.setColor(QPalette::Text, text_Color);
	palette.setColor(QPalette::Base, background_Color);
	palette.setColor(QPalette::AlternateBase, alter_background_Color);
	setPalette(palette);
}



CCheckBox::CCheckBox(QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    this->setLayoutDirection(Qt::LeftToRight);

    alter_background_Color = QColor(Qt::white);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));
}

CCheckBox::CCheckBox(QString _text, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    //this->setLayoutDirection(Qt::RightToLeft);
	this->setLayoutDirection(Qt::LeftToRight);

    alter_background_Color = QColor(Qt::white);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));

    this->setText(_text);
}

CCheckBox::CCheckBox(QString _text, QString _name, QWidget *parent)
{
    this->setParent(parent);
    this->setObjectName(_name);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    //this->setLayoutDirection(Qt::RightToLeft);
	this->setLayoutDirection(Qt::LeftToRight);

    alter_background_Color = QColor(Qt::white);

    frame_height = 20;
    frame_width = 70;

    this->setFont(QFont("Arial", 10));

    this->setText(_text);
}

CCheckBox::CCheckBox(QString _text, QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    //this->setLayoutDirection(Qt::RightToLeft);
	this->setLayoutDirection(Qt::LeftToRight);

    alter_background_Color = QColor(Qt::white);

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));

    this->setText(_text);
}

CCheckBox::CCheckBox(QSize _size, QWidget *parent)
{
    this->setParent(parent);
    //this->setStyle(new QPlastiqueStyle);
	this->setStyle(QStyleFactory::create("Fusion"));
    //this->setLayoutDirection(Qt::RightToLeft);
	this->setLayoutDirection(Qt::LeftToRight);

    alter_background_Color = QColor(Qt::white);

    frame_height = _size.height();
    frame_width = _size.width();

    this->setFont(QFont("Arial", 10));
}

QSize CCheckBox::sizeHint() const
{
    return QSize(frame_width, frame_height);
}

void CCheckBox::setFrameHeight(int _val)
{
    frame_height = _val;
}

void CCheckBox::setFrameWidth(int _val)
{
    frame_width = _val;
}

void CCheckBox::setFrameSize(QSize _size)
{
    frame_width = _size.width();
    frame_height = _size.height();
}

void CCheckBox::setBackgroundColor(QColor _color)
{
    background_Color = _color;
    repaintAll();
}

void CCheckBox::setAlterBackgroundColor(QColor _color)
{
    alter_background_Color = _color;
    repaintAll();
}

void CCheckBox::setTextColor(QColor _color)
{
    text_Color = _color;
    repaintAll();
}

void CCheckBox::repaintAll()
{
    QPalette palette;
    palette.setColor(QPalette::Foreground, text_Color);
    //palette.setColor(QPalette::Window, QColor(Qt::darkBlue));
    //palette.setColor(QPalette::Button, background_Color);
    this->setStyleSheet(QString("background-color: %1;").arg(background_Color.name()));
    palette.setColor(QPalette::AlternateBase, alter_background_Color);
    //palette.setColor(QPalette::WindowText, background_Color);
    //palette.setColor(QPalette::AlternateBase, alter_background_Color);
    setPalette(palette);
}


CWidget::CWidget(QWidget *_widget, QWidget *parent)
{
	this->setParent(parent);	
	this->setStyle(QStyleFactory::create("Fusion"));	

	widget = _widget;

	frame_height = 20;
	frame_width = 70;

	widget->show();
}

QSize CWidget::sizeHint() const
{
	return QSize(frame_width, frame_height);
}

void CWidget::setFrameHeight(int _val)
{
	frame_height = _val;
}

void CWidget::setFrameWidth(int _val)
{
	frame_width = _val;
}

void CWidget::setFrameSize(QSize _size)
{
	frame_width = _size.width();
	frame_height = _size.height();
}

/*void CWidget::repaintAll()
{
	QPalette palette;
	palette.setColor(QPalette::Foreground, text_Color);	
	this->setStyleSheet(QString("background-color: %1;").arg(background_Color.name()));
	palette.setColor(QPalette::AlternateBase, alter_background_Color);	
	setPalette(palette);
}*/


CSettings::CSettings(QString _type, QVariant _value, QString obj_name)
{
	name = obj_name;

    type = _type;
    data_type = String_Data;
    value = _value;
    min_max = QPair<double,double>(-1,-1);
    read_only = false;
	set_frame = false;
	hint = "";
	
    font = QFont("Arial", 9);
    icon = QIcon();

    text_color = QColor(Qt::black);
    active_text_color = QColor(Qt::darkMagenta);
    background_color = QColor(Qt::white);
    alter_background_color = QColor(Qt::lightGray);

    title_flag = false;
    checkable = true;
    check_state = false;

    frame_width = 70;
    frame_height = 20;

	widget = NULL;
}



CTreeWidgetItem::CTreeWidgetItem(QTreeWidget *_tw, QTreeWidgetItem *_twi, QList<CSettings> _settings)
{
    tw = _tw;
    twi = _twi;
	
    if (twi) sub_twi = new QTreeWidgetItem(twi, 0);
    else sub_twi = new QTreeWidgetItem(tw);
//	if (twi) sub_twi = boost::make_shared<QTreeWidgetItem>(twi, 0);
//	else sub_twi = boost::make_shared<QTreeWidgetItem>(tw);
	
    c_settings = _settings;
    initObjects();
}

CTreeWidgetItem::~CTreeWidgetItem()
{    	
	/*for (int i = 0; i < c_objects.count(); i++)
	{
		tw->removeItemWidget(sub_twi, i);
	}*/
	//delete sub_twi;

	//delete sub_twi;
	//sub_twi = NULL;
	
}

void CTreeWidgetItem::initObjects()
{
    int count = c_settings.count();
    
	static int obj_number = 1;

    for (int i = 0; i < count; i++)
    {
        if (c_settings[i].type == "label")
        {
            CLabel *lbl = new CLabel(c_settings[i].value.toString());
			lbl->setObjectName(QString("label%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) lbl->setObjectName(lbl->objectName() + QString("_%1").arg(c_settings[i].name));
            lbl->setFont(c_settings[i].font);
            lbl->setTextColor(c_settings[i].text_color);
            lbl->setBackgroundColor(c_settings[i].background_color);
            lbl->setAlterBackgroundColor(c_settings[i].alter_background_color);
            sub_twi->setIcon(i,c_settings[i].icon);
            lbl->setFrameSize(QSize(c_settings[i].frame_width, c_settings[i].frame_height));
			lbl->setToolTip(c_settings[i].hint);
			lbl->setFrame(c_settings[i].set_frame);
			
            c_objects.append(lbl);

            if (c_settings[i].title_flag)
            {
                tw->setFirstItemColumnSpanned(sub_twi, true);
                return;
            }            
        }
		else if (c_settings[i].type == "spinbox")
		{
			CSpinBox *sbx = new CSpinBox(c_settings[i].value.toDouble());
			sbx->setObjectName(QString("spinbox%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) sbx->setObjectName(sbx->objectName() + QString("_%1").arg(c_settings[i].name));
			sbx->setToolTip(c_settings[i].hint);
			sbx->setMinimum(c_settings[i].min_max.first);
			sbx->setMaximum(c_settings[i].min_max.second);
			sbx->setFont(c_settings[i].font);
			sbx->setTextColor(c_settings[i].text_color);
			sbx->setBackgroundColor(c_settings[i].background_color);
			sbx->setAlterBackgroundColor(c_settings[i].alter_background_color);
			sbx->setReadOnly(c_settings[i].read_only);
			sbx->setFrameSize(QSize(c_settings[i].frame_width, c_settings[i].frame_height));
			sub_twi->setIcon(i,c_settings[i].icon);

			connect(sbx, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
			connect(sbx, SIGNAL(editingFinished()), this, SLOT(changingFinished()));

			c_objects.append(sbx);
		}
        else if (c_settings[i].type == "dspinbox")
        {
            CDSpinBox *sbx = new CDSpinBox(c_settings[i].value.toDouble());
			sbx->setObjectName(QString("dspinbox%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) sbx->setObjectName(sbx->objectName() + QString("_%1").arg(c_settings[i].name));
			sbx->setToolTip(c_settings[i].hint);
            sbx->setMinimum(c_settings[i].min_max.first);
            sbx->setMaximum(c_settings[i].min_max.second);
            sbx->setFont(c_settings[i].font);
            sbx->setTextColor(c_settings[i].text_color);
            sbx->setBackgroundColor(c_settings[i].background_color);
            sbx->setAlterBackgroundColor(c_settings[i].alter_background_color);
			sbx->setReadOnly(c_settings[i].read_only);
            sbx->setFrameSize(QSize(c_settings[i].frame_width, c_settings[i].frame_height));
			sub_twi->setIcon(i,c_settings[i].icon);

			connect(sbx, SIGNAL(valueChanged(double)), this, SLOT(valueChanged(double)));
			connect(sbx, SIGNAL(editingFinished()), this, SLOT(changingFinished()));

            c_objects.append(sbx);
        }
        else if (c_settings[i].type == "textedit")
        {
            CTextEdit *ted = new CTextEdit(c_settings[i].value.toString());
			ted->setObjectName(QString("label%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) ted->setObjectName(ted->objectName() + QString("_%1").arg(c_settings[i].name));
            ted->setFont(c_settings[i].font);
            ted->setActiveTextColor(c_settings[i].active_text_color);
            ted->setReadOnlyColor(c_settings[i].text_color);
            ted->setBackgroundColor(c_settings[i].background_color);
            ted->setAlterBackgroundColor(c_settings[i].alter_background_color);
            ted->setDataType(c_settings[i].data_type);
            sub_twi->setIcon(i,c_settings[i].icon);
            ted->setFrameSize(QSize(c_settings[i].frame_width, c_settings[i].frame_height));
			ted->setReadOnly(c_settings[i].read_only);
			ted->setFrame(c_settings[i].set_frame);
			
			connect(ted, SIGNAL(textChanged(QString)), this, SLOT(valueChanged(QString)));

            c_objects.append(ted);
        }
        else if (c_settings[i].type == "combobox")
        {
			QString str_value = c_settings[i].value.toString();
			str_value.remove('"');
			str_value.remove('\'');
            QStringList list = str_value.split("|");
            CComboBox *cbox = new CComboBox(list);
			cbox->setObjectName(QString("combobox%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) cbox->setObjectName(cbox->objectName() + QString("_%1").arg(c_settings[i].name));
            cbox->setFont(c_settings[i].font);
            cbox->setTextColor(c_settings[i].text_color);
            cbox->setBackgroundColor(c_settings[i].background_color);
            cbox->setAlterBackgroundColor(c_settings[i].alter_background_color);
            sub_twi->setIcon(i,c_settings[i].icon);
            cbox->setFrameSize(QSize(c_settings[i].frame_width, c_settings[i].frame_height));
			
			connect(cbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(valueChanged(QString)));

            c_objects.append(cbox);
        }
        else if (c_settings[i].type == "checkbox")
        {			
			//CCheckBox *chbox = new CCheckBox(c_settings[i].value.toString());
			CCheckBox *chbox = new CCheckBox("");
			QString title = c_settings[i].value.toString();
			if (title != "no_text") chbox->setText(title);
            chbox->setObjectName(QString("checkbox%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) chbox->setObjectName(chbox->objectName() + QString("_%1").arg(c_settings[i].name));
            chbox->setFont(c_settings[i].font);
            chbox->setTextColor(c_settings[i].text_color);
            chbox->setBackgroundColor(c_settings[i].background_color);
            chbox->setAlterBackgroundColor(c_settings[i].alter_background_color);
            sub_twi->setIcon(i,c_settings[i].icon);
            chbox->setFrameSize(QSize(c_settings[i].frame_width, c_settings[i].frame_height));			
			chbox->setCheckable(c_settings[i].checkable);
            if (c_settings[i].checkable) chbox->setCheckState(c_settings[i].check_state == true ? Qt::Checked : Qt::Unchecked);
			
			connect(chbox, SIGNAL(stateChanged(int)), this, SLOT(valueChanged(int)));

            c_objects.append(chbox);
        }
		else if (c_settings[i].type == "widget")
		{				
			QWidget *w = new QWidget;
			w->setObjectName(QString("label%1").arg(obj_number++));
			if (!c_settings[i].name.isEmpty()) w->setObjectName(w->objectName() + QString("_%1").arg(c_settings[i].name));
			c_settings[i].widget->setParent(w);
			
			c_objects.append(w);
		}
    }
}

void CTreeWidgetItem::valueChanged(QString value)
{
	CTextEdit *cted = qobject_cast<CTextEdit*>(sender());
	if (cted) 
	{
		QVariant v_value = QVariant(value);
		emit value_changed(qobject_cast<QObject*>(cted), v_value);
		return;
	}
	
	CComboBox *ccbox = qobject_cast<CComboBox*>(sender());
	if (ccbox) 
	{
		int index = ccbox->findText(value);
		QVariant v_value = QVariant(index);
		emit value_changed(qobject_cast<QObject*>(ccbox), v_value);
		return;
	}	
}

void CTreeWidgetItem::valueChanged(int value)
{
	CCheckBox *chbox = qobject_cast<CCheckBox*>(sender());
	if (chbox) 
	{
		QVariant v_value = QVariant(value);
		emit value_changed(qobject_cast<QObject*>(chbox), v_value);
		return;
	}
	
	CSpinBox *sbox = qobject_cast<CSpinBox*>(sender());
	if (sbox) 
	{
		QVariant v_value = QVariant(value);
		emit value_changed(qobject_cast<QObject*>(sbox), v_value);
		return;
	}

}

void CTreeWidgetItem::valueChanged(double value)
{
	CDSpinBox *dsbox = qobject_cast<CDSpinBox*>(sender());
	if (dsbox) 
	{
		QVariant v_value = QVariant(value);
		emit value_changed(qobject_cast<QObject*>(dsbox), v_value);
		return;
	}
}

void CTreeWidgetItem::changingFinished()
{
	CDSpinBox *dsbox = qobject_cast<CDSpinBox*>(sender());
	if (dsbox) 
	{		
		emit editing_finished(qobject_cast<QObject*>(dsbox));
		return;
	}
	
	CSpinBox *sbox = qobject_cast<CSpinBox*>(sender());
	if (sbox) 
	{		
		emit editing_finished(qobject_cast<QObject*>(sbox));
		return;
	}

}

void CTreeWidgetItem::valueChanged(bool value)
{
	/*CCheckBox *chbox = qobject_cast<CCheckBox*>(sender());
	if (chbox) 
	{
		QVariant v_value = QVariant(value);
		emit value_changed(qobject_cast<QObject*>(chbox), v_value);
		return;
	}*/
}

void CTreeWidgetItem::show()
{
    for (int i = 0; i < c_objects.count(); i++)
    {
        tw->setItemWidget(sub_twi, i, c_objects[i]);
    }

    if (twi) twi->setExpanded(true);
}
