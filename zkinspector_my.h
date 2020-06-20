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
	QJsonArray array;    //�����ʽ���Ŀǰ���ӵ�zk��Ϣ��һ���ı���Ҫͬ����json�ļ���
	QString path = QDir::currentPath() + "/last_con_info.json";    //�ļ���·��
	QTreeWidgetItem* right_click_item;   //ѡ�е��һ��ڵ�
	QMenu* popMenu;

private:
	Ui::zkinspector_myClass ui;

	//֧�ֶ������
	vector<ZkClient*> con_zks;
	vector<QString> con_strings;
	vector<QString> con_names;
	vector<QString> con_times;

	//����ѡ��Ի���
	Dialog* dialog;
	DelDia* deldia;
	Submit_dialog* subdia;
	ConnectDia* condia;

	//QList<QTreeWidgetItem*> search_result;
	GSuggestCompletion* gs;

	network_detect* detect_thread;   //����ǰ�������״̬���߳�
	network_detect2* detect_thread2;    //����ʱ��������Ƿ�Ͽ����߳�
	network_detect3* detect_thread3;    //����ʱ��������Ƿ��ڶϿ����������߳�
	network_detect4* detect_thread4;    //������ǰ��⵽���紦������״̬ʱ��������Ƿ�ָ����߳�

private slots:
	void add_node_action1();   //���ȷ��
	void add_node_action2();   //���ȡ��
	void add_node();
	void delete_node_action1();  //���ȷ��
	void delete_node_action2();    //���ȡ��
	void delete_node();
	void showdatainfo(QTreeWidgetItem* , int);
	//void connect_and_show_node();
	void add_data(QTreeWidgetItem* item);   //���չ��ʱ����ӽڵ�
	void submit_ch();
	void submit_ch_action1();     //���ȷ��
	void submit_ch_action2();     //���ȡ��
	void disconnect_action();   //�Ͽ�����
	void refresh_action();      //ˢ������
	void export_action();
	void import_action(); 
	void connect();
	void connect_action1();
	void connect_action2();
	void show_url(QTreeWidgetItem* item);
	void clear_all();
	void urldecode(string& encd, char decd[]);
	void init_connect(int index,QString str);   //��������ʱ�Ĳ���
	void init_connect_pre();      //�մ򿪽���ʱ���json�ļ���������ִ�����
	void write_to_json();
	void read_from_json();
	
	void update_network_state(int state); //����ʱ��������/�����ߵĴ�ʩ
	void update_network_state2(int state); //����ʱ�������ʱ�Ĵ�ʩ
	void update_network_state3(int state); //����ʱ��������������Ĵ�ʩ
	void update_network_state4(int state); //����ʱ���粻�õ��Ǽ�⵽�����õĴ�ʩ

	void sltShowPopMenu(const QPoint&);   //�һ��Ĳۺ���
	void refresh_item_action();
	void disconnect_item_action();
};


