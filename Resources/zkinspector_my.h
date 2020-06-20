#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_zkinspector_my.h"
#include "zkclass.h"
#include <string>
#include <algorithm>
#include <vector>
#include <qDebug>
#include "Dialog.h"
#include "DelDia.h"
#include "Submit_dialog.h"

using namespace std;

class zkinspector_my : public QMainWindow
{
	Q_OBJECT

public:
	zkinspector_my(QWidget *parent = Q_NULLPTR);
private:
	Ui::zkinspector_myClass ui;
	ZkClient zk;
	Dialog* dialog;
	DelDia* deldia;
	Submit_dialog* subdia;
private slots:
	//void On_pushButton_2_clicked();
	//void On_pushButton_5_clicked();
	void add_node_action1();   //���ȷ��
	void add_node_action2();   //���ȡ��
	void add_node();
	void delete_node_action1();  //���ȷ��
	void delete_node_action2();    //���ȡ��
	void delete_node();
	void showdatainfo(QTreeWidgetItem* , int);
	void connect_and_show_node();
    void show_data(string& path, QTreeWidgetItem* cur);
	void submit_ch();
	void submit_ch_action1();     //���ȷ��
	void submit_ch_action2();     //���ȡ��
	void disconnect_action();    //�Ͽ�����
	void refresh_action();        //ˢ������
};


