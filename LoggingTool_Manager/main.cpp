#include <QtWidgets/QApplication>
#include <QStyleFactory>

#include "main_window.h"

#ifndef QT_NO_DEBUG
#include "vld.h"
#endif


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setStyle(QStyleFactory::create("Fusion"));

	MainWindow w;
	w.show();
	return a.exec();
}
