#pragma once 

#include <QtWidgets/QApplication>
#include <QMessageBox>
#include "zkinspector_my.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	zkinspector_my w;
	w.show();
	return a.exec();
}
