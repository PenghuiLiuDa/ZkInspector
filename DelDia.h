#pragma once

#include <QWidget>
#include "ui_DelDia.h"

class DelDia : public QWidget
{
	Q_OBJECT

public:
	DelDia(QWidget *parent = Q_NULLPTR);
	~DelDia();
	Ui::DelDia ui3;
};
