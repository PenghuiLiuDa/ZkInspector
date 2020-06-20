#pragma once

#include <QWidget>
#include "ui_Dialog.h"

class Dialog : public QWidget
{
	Q_OBJECT

public:
	Dialog(QWidget *parent = Q_NULLPTR);
	~Dialog();

//public:
	Ui::Dialog ui2;
};
