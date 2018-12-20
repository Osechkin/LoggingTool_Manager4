#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QtGui>
#include <QLayout>

#include "ui_about_dialog.h"


#define APP_VERSION		"0.7.41"
#define APP_DATE		"July 07, 2018"


class IaFrame : public QFrame
{
public:
	IaFrame(QWidget * parent) : QFrame(parent, /*Qt::Window |*/Qt::FramelessWindowHint)
	{
		setAttribute(Qt::WA_TranslucentBackground);

		pixmap.load(":pictures/images/ia3.png");
		resize(pixmap.size());
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		QPainter p(this);
		p.drawPixmap(0, 0, width(), height(), pixmap);
	}

private:
	QPixmap pixmap;
};


class AboutDialog : public QDialog, public Ui::AboutDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget *parent = 0)
	{
		this->setupUi(this);
		this->setParent(parent);

		IaFrame *frame = new IaFrame(this);
		frame->setObjectName(QStringLiteral("frame"));
		frame->setMinimumSize(QSize(121, 200));
		frame->setMaximumSize(QSize(121, 200));
		frame->setFrameShape(QFrame::NoFrame);	
		gridLayout->addWidget(frame, 0, 1, 1, 1);
		gridLayout->setHorizontalSpacing(0);

		this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		this->layout()->setSizeConstraint(QLayout::SetFixedSize);
		//setFixedSize( sizeHint() );

		lblTitle->setStyleSheet("QLabel { background-color : white; color : darkmagenta; }");
		lblTitle->setText(QString("<font size=5><b>Logging Tool Manager v.%1</b></font><br><font size=5>( %2 )</font>").arg(APP_VERSION).arg(APP_DATE));

		lblCitation->setStyleSheet("QLabel { background-color : white; color : darkblue; }");	
	}
};

#endif // ABOUT_DIALOG_H