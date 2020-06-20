#pragma once

#include <QWidget>
#include "ui_ConnectDia.h"

class ConnectDia : public QWidget
{
	Q_OBJECT

public:
	ConnectDia(QWidget *parent = Q_NULLPTR);
	~ConnectDia();
	Ui::ConnectDia ui5;
};
