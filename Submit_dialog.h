#pragma once

#include <QWidget>
#include "ui_Submit_dialog.h"

class Submit_dialog : public QWidget
{
	Q_OBJECT

public:
	Submit_dialog(QWidget *parent = Q_NULLPTR);
	~Submit_dialog();
	Ui::Submit_dialog ui4;
};
