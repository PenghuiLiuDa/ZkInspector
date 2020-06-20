#include "zkinspector_my.h"
#include <QMessageBox>
#include <QDebug>
#include <QTreeWidget>
#include <string>
#include <algorithm>
#include <vector>
#include "Dialog.h"
#include "DelDia.h"

using namespace std;
#define ctime_r(tctime, buffer) ctime_s (buffer, 40, tctime)

//ZkClient zk;

zkinspector_my::zkinspector_my(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(showdatainfo(QTreeWidgetItem*, int)), Qt::UniqueConnection);   //单击节点时显示节点的数据，元数据，权限信息到相应地方
	connect(ui.connect_btn, SIGNAL(clicked()), this, SLOT(connect_and_show_node()), Qt::UniqueConnection);   //按钮事件 connect按钮，来完成zk的连接工作与目录树的加载工作
	connect(ui.add_btn, SIGNAL(clicked()), this, SLOT(add_node()), Qt::UniqueConnection);               //按钮事件，相应增加节点
	connect(ui.del_btn, SIGNAL(clicked()), this, SLOT(delete_node()), Qt::UniqueConnection);               //按钮事件，删除相应节点
	connect(ui.submit_ch_btn, SIGNAL(clicked()), this, SLOT(submit_ch()), Qt::UniqueConnection);               //按钮事件，提交修改
	connect(ui.disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_action()), Qt::UniqueConnection);    //按钮事件，关闭连接
	connect(ui.refresh_btn, SIGNAL(clicked()), this, SLOT(refresh_action()), Qt::UniqueConnection);    //按钮事件，刷新
}

void zkinspector_my::refresh_action() //refresh按钮
{
	//先清楚ui.tree_widget的所有东西
	ui.tree_widget->clear();

	//调用connect方法重新拉取数据
	connect_and_show_node();
}

void zkinspector_my::disconnect_action() //disconnect按钮
{
	zk.close();
	//删除掉显示在tree_widget中的所有项目
	ui.tree_widget->clear();
}

void zkinspector_my::connect_and_show_node() //connect按钮
{
	string ip_addr = ui.lineEdit_2->text().toStdString();
	int time_out = ui.lineEdit_3->text().toInt();
	//qDebug() << "i want to know if i have something wrong" << endl;
	zk.init(ip_addr, time_out);
	ui.tree_widget->show();
	string path = "/";
	vector<string> childnodes;
	zk.getChildren(path, childnodes);

	ui.tree_widget->setColumnCount(1);
	QList<QTreeWidgetItem*> items;
	for (int i = 0; i < childnodes.size(); ++i)
		items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromStdString(childnodes[i]))));
	ui.tree_widget->addTopLevelItems(items);
	for (int i = 0; i < items.size(); i++)
	{
		string path_new = path + childnodes[i];
		QTreeWidgetItem* cur = items[i];
		show_data(path_new, cur);
	}	
}

void zkinspector_my::show_data(string& path, QTreeWidgetItem* cur)
{
	vector<string> childnodes;
	zk.getChildren(path, childnodes);
	if (childnodes.empty())
		return;
	for (int i = 0; i < childnodes.size(); i++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(cur,QStringList(QString::fromStdString(childnodes[i])));
		QTreeWidgetItem* item_new = item;
		string path_new=path+"/"+ childnodes[i];
		//qDebug() << QString::fromStdString(path_new);
		show_data(path_new, item_new);		
	}	
}

void zkinspector_my::showdatainfo(QTreeWidgetItem* item, int column)
{
	//qDebug()<<item->childCount();
	//qDebug() << item->text(0);
	QString path;
	if (!item->parent())
	{
		path = "/";
		path += item->text(0);
	}
	else
	{
		vector<QString> vt;
		vt.push_back(item->text(0));
		while (item->parent())
		{
			item = item->parent();
			vt.push_back(item->text(0)) ;
		}
		reverse(vt.begin(), vt.end());
		for (int i = 0; i < vt.size(); i++)
		{
			path += "/" + vt[i];
		}
	}
	string path_new= path.toStdString();
	struct Stat stat;
	
	string buffer;
	struct ACL_vector acl;
	char tctimes[40];
	char tmtimes[40];
	time_t tctime;
	time_t tmtime;

	zk.get_node(path_new, stat, buffer, acl);
	tctime = (stat.ctime) / 1000;
	tmtime = (stat.mtime) / 1000;
	ctime_r(&tmtime, tmtimes);
	ctime_r(&tctime, tctimes);

	ui.textEdit->setText(QString::fromStdString(buffer));

	ui.lineEdit_4->setText(QString::number(stat.aversion));
	ui.lineEdit_5->setText(tctimes);
	ui.lineEdit_6->setText(QString::number(stat.cversion));
	ui.lineEdit_14->setText(QString::number(stat.czxid));
	ui.lineEdit_15->setText(QString::number(stat.dataLength));
	ui.lineEdit_16->setText(QString::number(stat.ephemeralOwner));
	ui.lineEdit_17->setText(tmtimes);
	ui.lineEdit_18->setText(QString::number(stat.mzxid));
	ui.lineEdit_19->setText(QString::number(stat.numChildren));
	ui.lineEdit_20->setText(QString::number(stat.pzxid));
	ui.lineEdit_21->setText(QString::number(stat.version));

	ui.lineEdit_22->setText(acl.data->id.scheme);
	ui.lineEdit_23->setText(acl.data->id.id);

	if (acl.data->perms == ZOO_PERM_ALL)
		ui.lineEdit_24->setText("Read, Write, Create, Delete, Admin");
	else if (acl.data->perms == ZOO_PERM_READ)
		ui.lineEdit_24->setText("Read");
	else if (acl.data->perms == ZOO_PERM_WRITE)
		ui.lineEdit_24->setText("Write");
	else if (acl.data->perms == ZOO_PERM_CREATE)
		ui.lineEdit_24->setText("Create");
	else if (acl.data->perms == ZOO_PERM_DELETE)
		ui.lineEdit_24->setText("Delete");
	else
		ui.lineEdit_24->setText("Admin");
}

void zkinspector_my::add_node_action1()    //点击确定之后实际添加节点，然后关闭窗口
{
	string path = dialog->ui2.lineEdit->text().toStdString();
	vector<string> vt;
	int len = 0;
	bool flag = false;
	for (int i = 0; i < path.size(); i++)
	{
		if (path[i] == '/' && i != 0)
		{
			vt.push_back(path.substr(i - len, len));
			len = 0;
		}
		else if (path[i] == '/' && i == 0)
		{
			flag = true;   //path以“/”开头  
		}
		else
			len++;
	}
	vt.push_back(path.substr(path.size() - len, len));

	//如果目前还没有选定任何节点
	if (!ui.tree_widget->currentItem())
	{
		QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString::fromStdString(vt[0])));
		ui.tree_widget->addTopLevelItem(item);
		QTreeWidgetItem* cur = item;
		for (int i = 1; i < vt.size(); i++)
		{
			QTreeWidgetItem* item_new = new QTreeWidgetItem(cur, QStringList(QString::fromStdString(vt[i])));
			cur = item_new;
		}
		if (!flag)  //path不是以“/”开头
		{
			path = "/" + path;
		}
		zk.create_node(path, "");

	}

	//说明当前在目录树中有被选中的项目
	else
	{
		QTreeWidgetItem* cur = ui.tree_widget->currentItem();
		QTreeWidgetItem* pre = cur;
		for (int i = 0; i < vt.size(); i++)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(cur, QStringList(QString::fromStdString(vt[i])));
			cur = item;
		}
		if (!flag)  //path不是以“/”开头
		{
			path = "/" + path;
		}

		//先获得当前节点的完整路径
		vector<QString> vt;
		QString path_pre;
		vt.push_back(pre->text(0));
		while (pre->parent())
		{
			pre = pre->parent();
			vt.push_back(pre->text(0));
		}
		reverse(vt.begin(), vt.end());
		for (int i = 0; i < vt.size(); i++)
		{
			path_pre += "/" + vt[i];
		}
		path=path_pre.toStdString()+path;
		zk.create_node(path, "");
	}

	//关闭窗口
	dialog->close();
}

void zkinspector_my::add_node_action2()    //点击取消之后关闭窗口
{
	//关闭窗口
	dialog->close();
}

void zkinspector_my::add_node() //点击add node按钮时弹出框框
{
	dialog = new Dialog();
	//dialog->setModal(false);
	dialog->setWindowTitle(tr("Create Node"));
	dialog->show();
	
	connect(dialog->ui2.ok_btn, SIGNAL(clicked()), this, SLOT(add_node_action1()), Qt::UniqueConnection);
	connect(dialog->ui2.cancel_btn, SIGNAL(clicked()), this, SLOT(add_node_action2()), Qt::UniqueConnection);
}

void zkinspector_my::delete_node() //点击delete node按钮时弹出框框
{
	deldia = new DelDia();
	deldia->setWindowTitle(tr("Delete Node"));
	deldia->show();
	
	connect(deldia->ui3.yes_btn, SIGNAL(clicked()), this, SLOT(delete_node_action1()), Qt::UniqueConnection);
	connect(deldia->ui3.cancel_btn, SIGNAL(clicked()), this, SLOT(delete_node_action2()), Qt::UniqueConnection);
}

void zkinspector_my::delete_node_action1()    //点击确认之后先执行删除操作再关闭窗口
{
	//在窗口删除
	QList<QTreeWidgetItem*> s = ui.tree_widget->selectedItems();
	// 无情删除
	for (auto item : s) {
		delete item;
	}

	//在后台删除
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();

	//先获得当前节点的完整路径
	vector<QString> vt;
	QString path;
	vt.push_back(cur->text(0));
	while (cur->parent())
	{
		cur = cur->parent();
		vt.push_back(cur->text(0));
	}
	reverse(vt.begin(), vt.end());
	for (int i = 0; i < vt.size(); i++)
	{
		path += "/" + vt[i];
	}
	zk.delete_node(path.toStdString());

	//关闭窗口
	deldia->close();
}
void zkinspector_my::delete_node_action2()    //点击取消之后关闭窗口
{
	//关闭窗口
	deldia->close();
}

void zkinspector_my::submit_ch()   //点击submit_ch_btn弹出新窗口
{
	subdia = new Submit_dialog();
	subdia->setWindowTitle(tr("Submit Changes"));
	subdia->show();

	connect(subdia->ui4.ok_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action1()), Qt::UniqueConnection);
	connect(subdia->ui4.cancel_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action2()), Qt::UniqueConnection);
}

void zkinspector_my::submit_ch_action1()    //点击确认后执行修改操作，并关闭窗口
{
	//获得当前选中的item
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();

	//先获得当前节点的完整路径
	vector<QString> vt;
	QString path;
	vt.push_back(cur->text(0));
	while (cur->parent())
	{
		cur = cur->parent();
		vt.push_back(cur->text(0));
	}
	reverse(vt.begin(), vt.end());
	for (int i = 0; i < vt.size(); i++)
	{
		path += "/" + vt[i];
	}

	//获取修改后的值
	QString text_new = ui.textEdit->toPlainText();

	//设置节点内容
	zk.set_node(path.toStdString(), text_new.toStdString());

	//关闭窗口
	subdia->close();
}

void zkinspector_my::submit_ch_action2()    //点击取消之后重新设置为原来的字符串，再关闭窗口
{
	
	//关闭窗口
	subdia->close();
}











