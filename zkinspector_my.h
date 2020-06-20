#pragma once
#include <QtWidgets/QMainWindow>
#include <QtNetwork/QtNetwork>
#include <QTreeWidgetItem>
#include <QMenu>
#include "ui_zkinspector_my.h"
#include <string>
#include <algorithm>
#include <vector>
#include <qDebug>
#include "zkclass.h"
#include "Dialog.h"
#include "DelDia.h"
#include "ConnectDia.h"
#include "Submit_dialog.h"
#include "googlesuggest.h"
#include "connect.h"
#include "network_detect.h"
#include "network_detect2.h"
#include "network_detect3.h"
#include "network_detect4.h"

using namespace std;

class zkinspector_my : public QMainWindow,public CONNECT
{
	Q_OBJECT

public:
	zkinspector_my(QWidget *parent = Q_NULLPTR);
	QJsonArray array;    //数组格式存放目前连接的zk信息，一旦改变需要同步到json文件中
	QString path = QDir::currentPath() + "/last_con_info.json";    //文件的路径
	QTreeWidgetItem* right_click_item;   //选中的右击节点
	QMenu* popMenu;

private:
	Ui::zkinspector_myClass ui;

	//支持多个连接
	vector<ZkClient*> con_zks;
	vector<QString> con_strings;
	vector<QString> con_names;
	vector<QString> con_times;

	//弹出选择对话框
	Dialog* dialog;
	DelDia* deldia;
	Submit_dialog* subdia;
	ConnectDia* condia;

	//QList<QTreeWidgetItem*> search_result;
	GSuggestCompletion* gs;

	network_detect* detect_thread;   //运行前检测网络状态的线程
	network_detect2* detect_thread2;    //运行时检测网络是否断开的线程
	network_detect3* detect_thread3;    //运行时检测网络是否在断开后重连的线程
	network_detect4* detect_thread4;    //当连接前检测到网络处于离线状态时检测网络是否恢复的线程

private slots:
	void add_node_action1();   //点击确认
	void add_node_action2();   //点击取消
	void add_node();
	void delete_node_action1();  //点击确认
	void delete_node_action2();    //点击取消
	void delete_node();
	void showdatainfo(QTreeWidgetItem* , int);
	//void connect_and_show_node();
	void add_data(QTreeWidgetItem* item);   //点击展开时添加子节点
	void submit_ch();
	void submit_ch_action1();     //点击确认
	void submit_ch_action2();     //点击取消
	void disconnect_action();   //断开连接
	void refresh_action();      //刷新连接
	void export_action();
	void import_action(); 
	void connect();
	void connect_action1();
	void connect_action2();
	void show_url(QTreeWidgetItem* item);
	void clear_all();
	void urldecode(string& encd, char decd[]);
	void init_connect(int index,QString str);   //有新连接时的操作
	void init_connect_pre();      //刚打开界面时如果json文件中有连接执行这个
	void write_to_json();
	void read_from_json();
	
	void update_network_state(int state); //连接时网络在线/不在线的措施
	void update_network_state2(int state); //运行时网络断线时的措施
	void update_network_state3(int state); //运行时网络断线又重连的措施
	void update_network_state4(int state); //连接时网络不好但是检测到网络变好的措施

	void sltShowPopMenu(const QPoint&);   //右击的槽函数
	void refresh_item_action();
	void disconnect_item_action();
};


