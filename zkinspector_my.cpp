#include "zkinspector_my.h"
#include <QMessageBox>
#include <QDebug>
#include <QTreeWidget>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <QIcon>
#include <cstdio>
#include <QProcess>
#include <QAction>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QFile>
#include <QFileDialog>

#include <fstream>
#include <iostream>

#include "Dialog.h"
#include "DelDia.h"

using namespace std;
#define ctime_r(tctime, buffer) ctime_s (buffer, 40, tctime)

zkinspector_my::zkinspector_my(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//搜索过滤
	gs = new GSuggestCompletion(ui.lineEdit, ui.tree_widget);

	QMainWindow::connect(ui.tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(showdatainfo(QTreeWidgetItem*, int)), Qt::UniqueConnection);   //单击节点时显示节点的数据，元数据，权限信息到相应地方
	QMainWindow::connect(ui.add_btn, SIGNAL(clicked()), this, SLOT(add_node()), Qt::UniqueConnection);               //按钮事件，相应增加节点
	QMainWindow::connect(ui.del_btn, SIGNAL(clicked()), this, SLOT(delete_node()), Qt::UniqueConnection);               //按钮事件，删除相应节点
	QMainWindow::connect(ui.submit_ch_btn, SIGNAL(clicked()),this, SLOT(submit_ch()), Qt::UniqueConnection);               //按钮事件，提交修改
	QMainWindow::connect(ui.disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_action()), Qt::UniqueConnection);    //按钮事件，关闭连接
	QMainWindow::connect(ui.refresh_btn, SIGNAL(clicked()), this, SLOT(refresh_action()), Qt::UniqueConnection);    //按钮事件，刷新
	QMainWindow::connect(ui.tree_widget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(add_data(QTreeWidgetItem*)), Qt::UniqueConnection);  //点击折叠三角时会添加节点	
	QMainWindow::connect(ui.export_btn, SIGNAL(clicked()), this, SLOT(export_action()), Qt::UniqueConnection);       //按钮事件，支持导出操作
	QMainWindow::connect(ui.import_btn, SIGNAL(clicked()), this, SLOT(import_action()), Qt::UniqueConnection);        //按钮操作，支持导入操作
	QMainWindow::connect(ui.connect_btn, SIGNAL(clicked()), this, SLOT(connect()), Qt::UniqueConnection);      //按钮操作，读取连接参数进行连接

	//设置treewidget当节点名字过长时可以水平滚动
	ui.tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tree_widget->header()->setStretchLastSection(false);

	//设置标题
	QString headers;
	headers = "Connections";
	ui.tree_widget->setHeaderLabel(headers);

	//右键菜单
	setContextMenuPolicy(Qt::CustomContextMenu);
	QMainWindow::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(sltShowPopMenu(const QPoint&)));

	
	//在连接前检测网络状态
	detect_thread = new network_detect();   
	QMainWindow::connect(detect_thread, SIGNAL(send_network_connect_state(int)), this, SLOT(update_network_state(int)));
	detect_thread->start(); //开启网络检测线程
	
	//在连接后检测网络状态
	detect_thread2 = new network_detect2();
	QMainWindow::connect(detect_thread2, SIGNAL(send_network_connect_state2(int)), this, SLOT(update_network_state2(int)));

	//在连接后网络断开后检测有没有重连
	detect_thread3 = new network_detect3();
	QMainWindow::connect(detect_thread3, SIGNAL(send_network_connect_state3(int)), this, SLOT(update_network_state3(int)));
	

	//在连接前网络离线检测有没有变好
	detect_thread4 = new network_detect4();
	QMainWindow::connect(detect_thread4, SIGNAL(send_network_connect_state4(int)), this, SLOT(update_network_state4(int)));

	ui.tree_widget->setColumnCount(1);
	ui.tree_widget->show();
}

void zkinspector_my::add_data(QTreeWidgetItem* item)   //点击折叠三角时添加节点
{
	//如果目前还没有添加过，就删除掉empty节点，并开始添加节点
	if (item->childCount() == 1 && item->child(0)->text(0) == "empty")
	{		
		//删除掉empty节点
		delete item->child(0);

		//获取当前节点的路径
		vector<QString> vt;
		QString path;
		QTreeWidgetItem* item_pre = item;
		vt.push_back(item->text(0));
		while (item->parent())  //如果父节点存在
		{
			if (item->parent()->parent())  //如果爷爷节点存在
			{
				item = item->parent();
				vt.push_back(item->text(0));
			}
			else
				break;
		}

		reverse(vt.begin(), vt.end());
		for (int i = 0; i < vt.size(); i++)
		{
			path += "/" + vt[i];
		}
		qDebug() << path;

		//确定是哪一个连接
		int index;   
		item = item->parent();  //现在item指向qtreewidget中的一级节点
		QString name = item->text(0);
		for (int i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
			{
				index = i;
				break;
			}
		}

		//获取当前节点的子节点
		vector<string> childnodes;
		con_zks[index]->getChildren(path.toStdString(), childnodes);
		
		struct Stat stat;
		string buffer;
		struct ACL_vector acl;

		for (int i = 0; i < childnodes.size(); i++)
		{
			string path_new = path.toStdString() + "/" + childnodes[i];

			con_zks[index]->get_node(path_new, stat, buffer, acl);
			QTreeWidgetItem* item_child = new QTreeWidgetItem(item_pre, QStringList(QString::fromStdString(childnodes[i])));
			if (stat.numChildren > 0)
			{
				item_child->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/dir.png")));
				QTreeWidgetItem* item_child_child = new QTreeWidgetItem(item_child, QStringList((QString)"empty"));
			}
			else
				item_child->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/file.png")));
		}
	}
}

void zkinspector_my::refresh_action() //refresh按钮
{
	//如果没有连接，会弹出警告
	if(!con_strings.size())
		QMessageBox::critical(0,"Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	else
	{
		//先清楚ui.tree_widget的所有东西
		ui.tree_widget->clear();
		//重新拉取数据,只是在界面的显示修改而已
		for (int i = 0; i < con_strings.size(); i++)
		{
			QJsonObject js = array.at(i).toObject();

			//连接
			string path = "/";
			vector<string> childnodes;
			con_zks[i]->getChildren(path, childnodes);

			ui.tree_widget->setColumnCount(1);
			QTreeWidgetItem* par = new QTreeWidgetItem((QTreeWidget*)0, QStringList(js.value("Connect Name").toString()));
			ui.tree_widget->insertTopLevelItem(0, par);

			QList<QTreeWidgetItem*> items;
			for (int j = 0; j < childnodes.size(); ++j)
			{
				QTreeWidgetItem* item_new = new QTreeWidgetItem(par, QStringList(QString::fromStdString(childnodes[j])));
				items.append(item_new);
			}

			//判断有无子节点，如果有，添加一个子节点名字为empty
			struct Stat stat;
			string buffer;
			struct ACL_vector acl;

			for (int k = 0; k < childnodes.size(); k++)
			{
				string path_new = path + childnodes[k];
				con_zks[i]->get_node(path_new, stat, buffer, acl);

				if (stat.numChildren > 0)
				{
					items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/dir.png")));
					QTreeWidgetItem* item_child = new QTreeWidgetItem(items[k], QStringList((QString)"empty"));
					//item_child->setHidden(true);   
				}
				else
					items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/file.png")));
			}
		}
	}
}

void zkinspector_my::disconnect_action() //disconnect按钮
{
	//如果没有连接也会弹出警告
	if (!con_strings.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	else
	{
		for (int i = 0; i < con_strings.size(); i++)
		{
			con_zks[i]->close();
			con_zks[i]->zhandle_ = NULL;
		}
		while (con_strings.size())
		{
			con_names.pop_back();
			con_strings.pop_back();
			con_times.pop_back();
			con_zks.pop_back();
		}
		//删除掉显示在tree_widget中的所有项目
		ui.tree_widget->clear();

		//同时刷新array
		while (array.count())
		{
			array.pop_back();
		}
		write_to_json();
	}
}

void zkinspector_my::urldecode(string& encd, char decd[])
{
	int j, i;
	//char* cd = (char*)encd;
	char p[2];
	unsigned int num;
	j = 0;

	for (i = 0; i < encd.size(); i++)
	{
		memset(p, '/0', 2);
		if (encd[i] != '%')
		{
			decd[j++] = encd[i];
			continue;
		}

		p[0] = encd[++i];
		p[1] =encd[++i];

		p[0] = p[0] - 48 - ((p[0] >= 'A') ? 7 : 0) - ((p[0] >= 'a') ? 32 : 0);
		p[1] = p[1] - 48 - ((p[1] >= 'A') ? 7 : 0) - ((p[1] >= 'a') ? 32 : 0);
		decd[j++] = (unsigned char)(p[0] * 16 + p[1]);

	}
	decd[j] = '/0';
}

void zkinspector_my::show_url(QTreeWidgetItem* item)
{
	ui.raw_url->setText(item->text(0));

	//解码
	string url_raw = item->text(0).toStdString().c_str();
	char decd[600];
	memset(decd, '\0', sizeof(decd));
	urldecode(url_raw, decd);

	string url(decd);
	ui.decoded_url->setText(QString::fromStdString(url));


	string protocol;   //开头
	string addr;
	string service_name;    //问号分割
	string application;
	string category;
	string check;
	string methods;
	string dubbo;
	string generic;
	string interface1;
	string pid;
	string side;
	string timeout;
	string anyhost;
	string retries;
	string service_filter;   //service.filter
	string default_check;      //default.check
	string revision;
	string timestamp;    //结尾？

	for (int i = 0; i < url.size();)
	{
		while (url[i] != ':')
		{
			protocol += url[i];
			i++;
		}

		i += 3;
		while (url[i] != '/')
		{
			addr += url[i];
			i++;
		}

		i += 1;
		while (url[i] != '?')
		{
			service_name += url[i];
			i++;
		}
		break;
	}

	size_t index = url.find("application=");
	if (index != string::npos)
	{
		index += 12;
		while (url[index] != '&')
		{
			application += url[index];
			index += 1;
		}
	}

	index = url.find("methods=");
	if (index != string::npos)
	{
		index += 8;
		while (url[index] != '&')
		{
			methods += url[index];
			index += 1;
		}

	}

	index = url.find("category=");
	if (index != string::npos)
	{
		index += 9;
		while (url[index] != '&')
		{
			category += url[index];
			index += 1;
		}
	}

	index = url.find("check=");
	if (index != string::npos)
	{
		index += 6;
		while (url[index] != '&')
		{
			check += url[index];
			index += 1;
		}
	}

	index = url.find("dubbo=");
	if (index != string::npos)
	{
		index += 6;
		while (url[index] != '&')
		{
			dubbo += url[index];
			index += 1;
		}
	}

	index = url.find("generic=");
	if (index != string::npos)
	{
		index += 8;
		while (url[index] != '&')
		{
			generic += url[index];
			index += 1;
		}
	}

	index = url.find("interface=");
	if (index != string::npos)
	{
		index += 10;
		while (url[index] != '&')
		{
			interface1 += url[index];
			index += 1;
		}
	}

	index = url.find("pid=");
	if (index != string::npos)
	{
		index += 4;
		while (url[index] != '&')
		{
			pid += url[index];
			index += 1;
		}
	}

	index = url.find("side=");
	if (index != string::npos)
	{
		index += 5;
		while (url[index] != '&')
		{
			side += url[index];
			index += 1;
		}
	}

	index = url.find("timeout=");
	if (index != string::npos)
	{
		index += 8;
		while (url[index] != '&')
		{
			timeout += url[index];
			index += 1;
		}
	}

	index = url.find("anyhost=");
	if (index != string::npos)
	{
		index += 8;
		while (url[index] != '&')
		{
			anyhost += url[index];
			index += 1;
		}
	}

	index = url.find("retries=");
	if (index != string::npos)
	{
		index += 8;
		while (url[index] != '&')
		{
			retries += url[index];
			index += 1;
		}
	}

	index = url.find("&sevice.filter=");
	if (index != string::npos)
	{
		index += 15;
		while (url[index] != '&')
		{
			service_filter += url[index];
			index += 1;
		}
	}

	index = url.find("default.check=");
	if (index != string::npos)
	{
		index += 14;
		while (url[index] != '&')
		{
			default_check += url[index];
			index += 1;
		}
	}

	index = url.find("revision=");
	if (index != string::npos)
	{
		index += 9;
		while (url[index] != '&')
		{
			revision += url[index];
			index += 1;
		}
	}

	index = url.find("timestamp=");
	if (index != string::npos)
	{
		index += 10;
		while (index < url.size())
		{
			timestamp += url[index];
			index += 1;
		}
	}

	ui.protocol->setText(QString::fromStdString(protocol));
	ui.addr->setText(QString::fromStdString(addr));
	ui.service_name->setText(QString::fromStdString(service_name));
	ui.application->setText(QString::fromStdString(application));
	ui.category->setText(QString::fromStdString(category));
	ui.check->setText(QString::fromStdString(check));
	ui.dubbo->setText(QString::fromStdString(dubbo));
	ui.methods->setText(QString::fromStdString(methods));
	ui.generic->setText(QString::fromStdString(generic));
	ui.interface_2->setText(QString::fromStdString(interface1));
	ui.pid->setText(QString::fromStdString(pid));
	ui.side->setText(QString::fromStdString(side));
	ui.timeout->setText(QString::fromStdString(timeout));
	ui.anyhost->setText(QString::fromStdString(anyhost));
	ui.retries->setText(QString::fromStdString(retries));
	ui.service_filter->setText(QString::fromStdString(service_filter));
	ui.default_check->setText(QString::fromStdString(default_check));
	ui.revision->setText(QString::fromStdString(revision));
	ui.timestamp->setText(QString::fromStdString(timestamp));
}

void zkinspector_my::clear_all()
{
	ui.addr->clear();
	ui.anyhost->clear();
	ui.application->clear();
	ui.category->clear();
	ui.check->clear();
	ui.decoded_url->clear();
	ui.default_check->clear();
	ui.dubbo->clear();
	ui.generic->clear();
	ui.interface_2->clear();
	ui.methods->clear();
	ui.pid->clear();
	ui.protocol->clear();
	ui.raw_url->clear();
	ui.retries->clear();
	ui.revision->clear();
	ui.service_filter->clear();
	ui.side->clear();
	ui.timeout->clear();
	ui.timestamp->clear();
	ui.service_name->clear();
}

void zkinspector_my::showdatainfo(QTreeWidgetItem* item, int column)
{
	QTreeWidgetItem* item_before=item;
	QString path;
	if (item->parent())  //如果该节点存在父亲节点
	{
		if (!item->parent()->parent())  //如果该节点的爷爷节点不存在，说明它是zk中的一级节点
		{
			path = "/";
			path += item->text(0);
		}

		else     //该节点不是zk中的一级节点
		{
			vector<QString> vt;
			vt.push_back(item->text(0));
			while (item->parent()->parent())
			{
				item = item->parent();
				vt.push_back(item->text(0));
			}
			reverse(vt.begin(), vt.end());
			for (int i = 0; i < vt.size(); i++)
			{
				path += "/" + vt[i];
			}
		}

		//确定哪一个连接
		int index;
		QString name = item->parent()->text(0);
		for (int i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
			{
				index = i;
				break;
			}
		}

		string path_new = path.toStdString();
		struct Stat stat;

		string buffer;
		struct ACL_vector acl;
		char tctimes[40];
		char tmtimes[40];
		time_t tctime;
		time_t tmtime;

		con_zks[index]->get_node(path_new, stat, buffer, acl);
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

		//判断是否需要解码
		if ((stat.numChildren == 0) && (item_before->text(0).toStdString().find('%') != string::npos))
		{
			show_url(item_before);
			qDebug() << "I am showing decodede information!!!" << endl;
		}

		//如果不需要解码
		else
		{
			// 清空所有需要输入解码数据的输入框
			clear_all();
			qDebug() << "clear all!!!" << endl;
		}
	}
}

void zkinspector_my::add_node_action1()    //点击确定之后实际添加节点，然后关闭窗口
{
	string path = dialog->ui2.lineEdit->text().toStdString();
	vector<string> vt;
	int len = 0;
	bool flag = false;   //看看path是否以‘/’开头
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
	
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();
	//如果当前节点没有父亲节点
	if (!cur->parent())
	{
		//首先确定是哪一个连接
		QString name = cur->text(0);
		int index;
		for (int i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
			{
				index = i;
				break;
			}
		}

		if (!flag)  //path不是以“/”开头
		{
			path = "/" + path;
		}
		con_zks[index]->create_node(path, "");
	}

	//如果有父亲节点
	else
	{
		//首先确定是哪一个连接以及当前节点路径
		QTreeWidgetItem* pre = cur;
		vector<QString> vt;
		QString path_pre;
		vt.push_back(cur->text(0));
		while (cur->parent())  //如果父节点存在
		{
			if (cur->parent()->parent())  //如果爷爷节点存在
			{
				cur = cur->parent();
				vt.push_back(cur->text(0));
			}
			else
				break;
		}

		QString name=cur->parent()->text(0);
		int index;
		for (int i = 0; i < con_names.size(); i++)
		{
			if (con_names[i] == name)
			{
				index = i;
				break;
			}
		}

		reverse(vt.begin(), vt.end());
		for (int i = 0; i < vt.size(); i++)
		{
			path_pre += "/" + vt[i];
		}


		if (!flag)  //path不是以“/”开头
		{
			path = "/" + path;
		}
		path = path_pre.toStdString() + path;;
		con_zks[index]->create_node(path, "");
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
	//如果没有连接，会弹出警告
	if (!con_strings.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

	//如果没有选中任何节点，也弹出警告
	else if (!ui.tree_widget->currentItem())
	{
		QMessageBox::critical(0, "Critical message", "Please select one node first!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//有所选中的节点时
	else
	{
		dialog = new Dialog();
		//dialog->setModal(false);

		//实现弹出窗口时禁用主窗口
		Qt::WindowFlags flags = Qt::Dialog;
		dialog->setWindowFlags(flags);
		dialog->setWindowModality(Qt::ApplicationModal);

		dialog->setWindowTitle(tr("Create Node"));
		dialog->show();

		QMainWindow::connect(dialog->ui2.ok_btn, SIGNAL(clicked()), this, SLOT(add_node_action1()), Qt::UniqueConnection);
		QMainWindow::connect(dialog->ui2.cancel_btn, SIGNAL(clicked()), this, SLOT(add_node_action2()), Qt::UniqueConnection);
	}
}

void zkinspector_my::delete_node() //点击delete node按钮时弹出框框
{
	//如果没有连接，会弹出警告
	if (!con_strings.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

	//如果当前没有选中节点
	else if(!(ui.tree_widget->currentItem()))
	{
		QMessageBox::critical(0, "Critical message", "Have not yet selected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//如果当前节点是连接zk的节点，报错
	else if (!(ui.tree_widget->currentItem()->parent()))
	{
		QMessageBox::critical(0, "Critical message", "Can't delete connection here!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	else
	{
		deldia = new DelDia();

		//实现弹出窗口时禁用主窗口
		Qt::WindowFlags flags = Qt::Dialog;
		deldia->setWindowFlags(flags);
		deldia->setWindowModality(Qt::ApplicationModal);

		deldia->setWindowTitle(tr("Delete Node"));
		deldia->show();

		QMainWindow::connect(deldia->ui3.yes_btn, SIGNAL(clicked()), this, SLOT(delete_node_action1()), Qt::UniqueConnection);
		QMainWindow::connect(deldia->ui3.cancel_btn, SIGNAL(clicked()), this, SLOT(delete_node_action2()), Qt::UniqueConnection);
	}
}

void zkinspector_my::delete_node_action1()    //点击确认之后先执行删除操作再关闭窗口  注意！！！！先在后台删除再前端删除
{
	//在后台删除
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();
	QTreeWidgetItem* cur_pre = cur;

	//先获得当前节点的完整路径
	vector<QString> vt;
	QString path;

	vt.push_back(cur->text(0));
	while (cur->parent())  //如果父节点存在
	{
		if (cur->parent()->parent())  //如果爷爷节点存在
		{
			cur = cur->parent();
			vt.push_back(cur->text(0));
		}
		else
			break;
	}

	//确定属于哪一个连接
	QString name = cur->parent()->text(0);
	int index;
	for (int i = 0; i < con_names.size(); i++)
	{
		if (con_names[i] == name)
		{
			index = i;
			break;
		}
	}

	reverse(vt.begin(), vt.end());
	for (int i = 0; i < vt.size(); i++)
	{
		path += "/" + vt[i];
	}
	qDebug() << path;
	con_zks[index]->delete_node(path.toStdString());

	//在前台删除
	delete cur_pre;

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
	//如果没有连接，会弹出警告
	if (!con_zks.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

	//如果没有选定的节点，也发出警告
	else if (!ui.tree_widget->currentItem())
	{
		QMessageBox::critical(0, "Critical message", "Please select one node first!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//如果选中的节点没有父节点，即选中的是连接
	else if (!(ui.tree_widget->currentItem()->parent()))
	{
		QMessageBox::critical(0, "Critical message", "Can't Change Connection here!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//选中的是普通节点
	else
	{
		subdia = new Submit_dialog();

		//实现弹出窗口时禁用主窗口
		Qt::WindowFlags flags = Qt::Dialog;
		subdia->setWindowFlags(flags);
		subdia->setWindowModality(Qt::ApplicationModal);

		subdia->setWindowTitle(tr("Submit Changes"));
		subdia->show();

		QMainWindow::connect(subdia->ui4.ok_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action1()), Qt::UniqueConnection);
		QMainWindow::connect(subdia->ui4.cancel_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action2()), Qt::UniqueConnection);
	}
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
			if (cur->parent()->parent())
			{
				cur = cur->parent();
				vt.push_back(cur->text(0));
			}
			else
				break;

		}

		//获得当前节点的连接
		int index;
		QString name = cur->parent()->text(0);
		for (int i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
			{
				index = i;
				break;
			}
		}

		reverse(vt.begin(), vt.end());
		for (int i = 0; i < vt.size(); i++)
		{
			path += "/" + vt[i];
		}

		//获取修改后的值
		QString text_new = ui.textEdit->toPlainText();

		//设置节点内容
		con_zks[index]->set_node(path.toStdString(), text_new.toStdString());

		//关闭窗口
		subdia->close();

}

void zkinspector_my::submit_ch_action2()    //点击取消之后重新设置为原来的字符串，再关闭窗口
{
	
	//关闭窗口
	subdia->close();
}

void zkinspector_my::export_action()     //导出配置
{
	QString filename = QFileDialog::getSaveFileName(this, "Export Json Files", QDir::currentPath(), tr("Json File(*.json)"));  //获取需要保存成的文件名
	if (filename.isEmpty())
	{
		QMessageBox::information(this, "ErrorBox", "Please input the filename");
		return;
	}

	else
	{
		QFile* file = new QFile;
		file->setFileName(filename);
		bool ok = file->open(QIODevice::WriteOnly);

		if (ok)
		{
			QJsonDocument jsonDoc = QJsonDocument(array);
			file->write(jsonDoc.toJson());
			file->close();
			qDebug() << "write successfully!!!" << endl;
		}
		else
		{
			QMessageBox::information(this, "ErrorBox", "file fail to save");
		}
	}
}

void zkinspector_my::import_action()       //导入配置,以追加的方式，同时去重
{
	QString file = QFileDialog::getOpenFileName(this, tr("Import Josn Files"), QDir::currentPath(), tr("Allfile(*.*);;mp3file(*.mp3);;jsonfile(*.json)"));
	QFile loadFile(file);

	if (!loadFile.open(QIODevice::ReadOnly))
	{
		qDebug() << "could't open projects json";
		return;
	}

	QByteArray allData = loadFile.readAll();
	loadFile.close();
	QJsonParseError json_error;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));

	QJsonArray array2= jsonDoc.array();  //存储在新数组中

	//记录之前的连接数
	int before = con_zks.size();
	int index = before - 1;
	for (int i = 0; i < array2.count(); i++)
	{
		QJsonObject js = array2.at(i).toObject();

		//先判断是否有重名的
		QString name = js.value("Connect Name").toString();
		int m;
		for (m = 0; m < con_names.size(); m++)
		{
			if (name == con_names[m])
				break;
		}

		//如果没有重名的
		if (m == con_names.size())
		{
			//新下标
			index += 1;

			con_zks.push_back(new ZkClient());
			con_zks[index]->init(js.value("Connect String").toString().toStdString(), js.value("Session Timeout").toString().toInt());
			con_names.push_back(js.value("Connect Name").toString());
			con_strings.push_back(js.value("Connect String").toString());
			con_times.push_back(js.value("Session Timeout").toString());
			array.push_back(js);

			//连接
			string path = "/";
			vector<string> childnodes;
			con_zks[index]->getChildren(path, childnodes);

			ui.tree_widget->setColumnCount(1);
			QTreeWidgetItem* par = new QTreeWidgetItem((QTreeWidget*)0, QStringList(js.value("Connect Name").toString()));
			ui.tree_widget->insertTopLevelItem(0, par);

			QList<QTreeWidgetItem*> items;
			for (int j = 0; j < childnodes.size(); ++j)
			{
				QTreeWidgetItem* item_new = new QTreeWidgetItem(par, QStringList(QString::fromStdString(childnodes[j])));
				items.append(item_new);
			}

			//判断有无子节点，如果有，添加一个子节点名字为empty
			struct Stat stat;
			string buffer;
			struct ACL_vector acl;

			for (int k = 0; k < childnodes.size(); k++)
			{
				string path_new = path + childnodes[k];
				con_zks[index]->get_node(path_new, stat, buffer, acl);

				if (stat.numChildren > 0)
				{
					items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/dir.png")));
					QTreeWidgetItem* item_child = new QTreeWidgetItem(items[k], QStringList((QString)"empty"));
					//item_child->setHidden(true);   
				}
				else
					items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/file.png")));
			}
		}		
	}
	write_to_json();
}

void zkinspector_my::update_network_state(int state)
{
	if (state)   //表示在线
	{
		//此处应该读取之前的json文件，然后加载即可
		read_from_json();
		init_connect_pre();
		//然后开始检测连接之后的网络状态
		detect_thread2->start(); 
	}

	//表示不在线
	else
	{
		netstateisOK = false;
		//这里的提示应该悬浮几秒之后自动消失
		QMessageBox::critical(0, "Critical message", "Your network doesn't work,PLease check out your network!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
		//除了上边的警告，这里还应该去检测网络状态，当检测到网络状态良好的时候再读取之前的json文件，加载即可，然后去检测网络状态(2)
		detect_thread4->start();
	}
}

void zkinspector_my::update_network_state2(int state)
{
	if (!state)   //表示断线
	{
		qDebug() << "there is something wrong woith your network!!!" << endl;
		//此处悬浮几秒
		QMessageBox::critical(0, "Critical message", "NetWork may have something wrong!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

		//接下来执行检测网络是否重连的线程
		detect_thread3->start();
	}
}

void zkinspector_my::update_network_state3(int state)
{
	if (state)   //表示断线后又重连了。
	{

		QMessageBox::information(this, tr("Congratulations!!!"), QStringLiteral("Connected again!!!"), QMessageBox::Ok);

		//执行重连
		for (int i = 0; i < con_zks.size(); i++)
		{
			con_zks[i]->init(con_strings[i].toStdString(), con_times[i].toInt());
		}

		//接下来执行检测网络是否又断开
		detect_thread2->start();
	}
}

void zkinspector_my::update_network_state4(int state)
{
	if (state)    //说明连接前网络变好了
	{
		//悬浮几秒自动关闭
		QMessageBox::information(this, tr("Congratulations!!!"), QStringLiteral("Connected again!!!"), QMessageBox::Ok);
		read_from_json();
		init_connect_pre();
		//接下来检测网络是否断开
		detect_thread2->start();
	}
}

void zkinspector_my::init_connect(int index, QString str)
{
	string path = "/";
	vector<string> childnodes;
	con_zks[index]->getChildren(path, childnodes);

	ui.tree_widget->setColumnCount(1);
	QTreeWidgetItem* par = new QTreeWidgetItem((QTreeWidget*)0, QStringList(str));
	ui.tree_widget->insertTopLevelItem(0, par);

	QList<QTreeWidgetItem*> items;
	for (int i = 0; i < childnodes.size(); ++i)
	{
		QTreeWidgetItem* item_new = new QTreeWidgetItem(par, QStringList(QString::fromStdString(childnodes[i])));
		items.append(item_new);
	}

	//判断有无子节点，如果有，添加一个子节点名字为empty
	struct Stat stat;
	string buffer;
	struct ACL_vector acl;

	for (int i = 0; i < childnodes.size(); i++)
	{
		string path_new = path + childnodes[i];
		con_zks[index]->get_node(path_new, stat, buffer, acl);

		if (stat.numChildren > 0)
		{
			items[i]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/dir.png")));
			QTreeWidgetItem* item_child = new QTreeWidgetItem(items[i], QStringList((QString)"empty"));
			//item_child->setHidden(true);   
		}
		else
			items[i]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/file.png")));
	}

	//连接之后，把这次的连接信息计入push进array中
	QJsonObject ob;
	ob.insert("Connect String", con_strings[index]);
	ob.insert("Session Timeout", con_times[index]);
	ob.insert("Connect Name", con_names[index]);
	array.push_back(ob);

	//array有变化，需要更新
	write_to_json();

	qDebug() << "write successfully!" << endl;
}

void zkinspector_my::connect()
{
	condia = new ConnectDia();

	//实现弹出窗口时禁用主窗口
	Qt::WindowFlags flags = Qt::Dialog;
	condia->setWindowFlags(flags);
	condia->setWindowModality(Qt::ApplicationModal);

	condia->setWindowTitle(tr("Add Connections."));
	condia->show();

	QMainWindow::connect(condia->ui5.ok_btn, SIGNAL(clicked()), this, SLOT(connect_action1()), Qt::UniqueConnection);
	QMainWindow::connect(condia->ui5.cancel_btn, SIGNAL(clicked()), this, SLOT(connect_action2()), Qt::UniqueConnection);
}

void zkinspector_my::connect_action1()      //在连接页面点击确认
{
	//判断是否都有输入,如果都有输入进行接下来的操作
	if (condia->ui5.lineEdit->text().size() && condia->ui5.lineEdit_2->text().size() && condia->ui5.lineEdit_3->text().size())
	{
		//先判断名字有没有重复
		QString name = condia->ui5.lineEdit_3->text();
		int i;
		for (i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
				break;
		}

		if (i == con_names.size())   //说明名字没有重复
		{
			//再判断ip地址或域名是否有效
			QProcess* network_process;
			QString network_cmd = "ping " + condia->ui5.lineEdit->text() + " -n 2 -w 500";
			QString result;
			network_process = new QProcess();

			network_process->start(network_cmd);   //调用ping 指令
			network_process->waitForFinished();    //等待指令执行完毕
			result = network_process->readAll();   //获取指令执行结果
			if (result.contains(QString("TTL=")))   //若包含TTL=字符串则认为ip或域名有效,开始连接
			{
				//开始连接
				//首先检查ip或域名是否已经被连过
				int i = 0;
				for (; i < con_strings.size(); i++)
				{
					if (con_strings[i] == condia->ui5.lineEdit->text())
						QMessageBox::critical(0, "Connect Error", "Have Yet Connected!!!",
							QMessageBox::Ok | QMessageBox::Default,
							QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
					return;
				}

				//执行到这里说明之前没有连过此地址,开始连接操作
				int index = (0 == con_strings.size() ? 0 : (con_strings.size() - 1));
				con_zks.push_back(new ZkClient());
				con_zks[index]->init(condia->ui5.lineEdit->text().toStdString(), condia->ui5.lineEdit_2->text().toInt());

				con_strings.push_back(condia->ui5.lineEdit->text());
				con_times.push_back(condia->ui5.lineEdit_2->text());
				con_names.push_back(condia->ui5.lineEdit_3->text());

				init_connect(index, condia->ui5.lineEdit_3->text());
				condia->close();
			}

			//如果无效，先关闭窗口，再弹出ip或域名无效的提示
			else
			{
				condia->close();
				QMessageBox::critical(0, "Connect Error", "Please make sure your ip correct!!!",
					QMessageBox::Ok | QMessageBox::Default,
					QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
			}
		}
		
		else  //名字重复
		{
			QMessageBox::critical(0, "Connect Error", "Duplicated Connect Name!!!",
				QMessageBox::Ok | QMessageBox::Default,
				QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
		}

	}

	//提示没有输入但并不关闭连接窗口
	else
	{
		QMessageBox::critical(0, "Critical message", "Please make sure you have input everything required!!!",
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
	}
}

void zkinspector_my::connect_action2()     //在连接页面点击取消
{
	//关闭窗口
	condia->close();
}

void zkinspector_my::write_to_json()   //写入json文件的操作，只要对array数组有任何更新的变化都调用这个函数
{
	QString filename = path;
	QFile file(filename);
	bool ok = file.open(QIODevice::WriteOnly);

	if (ok)
	{
		QJsonDocument jsonDoc = QJsonDocument(array);
		file.write(jsonDoc.toJson());
		file.close();
		qDebug() << "write successfully!!!" << endl;
	}
	else
	{
		qDebug() << "error when open file!!" << endl;
	}
}

void zkinspector_my::read_from_json()   // 刚打开界面的时候读取。
{
	if (array.count() == 0)
	{
		QString file = path;
		QFile loadFile(file);
		if (!loadFile.open(QIODevice::ReadOnly))
		{
			qDebug() << "could't open projects json";
			return ;
		}
		QByteArray allData = loadFile.readAll();
		loadFile.close();
		QJsonParseError json_error;
		QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
		array= jsonDoc.array();
		for (int i = 0; i < array.count(); i++)
		{
			QJsonObject ob= array.at(i).toObject();
			qDebug() << ob.value("Connect String").toString() << endl;
			qDebug() << ob.value("Connect Name").toString() << endl;
			qDebug() << ob.value("Session Timeout").toString() << endl;
		}
	}
}

void zkinspector_my::init_connect_pre()
{
	//开始连接配置文件中的记录
	for (int i = 0; i < array.count(); i++)
	{
		QJsonObject js = array.at(i).toObject();
		con_zks.push_back(new ZkClient());
		con_zks[i]->init(js.value("Connect String").toString().toStdString(),js.value("Session Timeout").toString().toInt());
		con_names.push_back(js.value("Connect Name").toString());
		con_strings.push_back(js.value("Connect String").toString());
		con_times.push_back(js.value("Session Timeout").toString());

		//连接
		string path = "/";
		vector<string> childnodes;
		con_zks[i]->getChildren(path, childnodes);

		ui.tree_widget->setColumnCount(1);
		QTreeWidgetItem* par = new QTreeWidgetItem((QTreeWidget*)0, QStringList(js.value("Connect Name").toString()));
		ui.tree_widget->insertTopLevelItem(0, par);

		QList<QTreeWidgetItem*> items;
		for (int j = 0; j < childnodes.size(); ++j)
		{
			QTreeWidgetItem* item_new = new QTreeWidgetItem(par, QStringList(QString::fromStdString(childnodes[j])));
			items.append(item_new);
		}

		//判断有无子节点，如果有，添加一个子节点名字为empty
		struct Stat stat;
		string buffer;
		struct ACL_vector acl;

		for (int k = 0; k < childnodes.size(); k++)
		{
			string path_new = path + childnodes[k];
			con_zks[i]->get_node(path_new, stat, buffer, acl);

			if (stat.numChildren > 0)
			{
				items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/dir.png")));
				QTreeWidgetItem* item_child = new QTreeWidgetItem(items[k], QStringList((QString)"empty"));
				//item_child->setHidden(true);   
			}
			else
				items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/file.png")));
		}
	}
}

void zkinspector_my::sltShowPopMenu(const QPoint&)  //右击的槽函数
{
	right_click_item = ui.tree_widget->currentItem();
	if (right_click_item)
	{
		//如果选到一级节点，临时生成菜单
		if (!right_click_item->parent())
		{
			popMenu = new QMenu(ui.tree_widget);
			QAction* act1 = popMenu->addAction("refresh");
			QAction* act2 = popMenu->addAction("disconnect");

			QMainWindow::connect(act1, SIGNAL(triggered(bool)), this, SLOT(refresh_item_action()));
			QMainWindow::connect(act2, SIGNAL(triggered(bool)), this, SLOT(disconnect_item_action()));
			popMenu->exec(QCursor::pos());
		}
	}
}

void zkinspector_my::refresh_item_action()
{
	right_click_item = ui.tree_widget->currentItem();
	QString name = right_click_item->text(0);

	//先确定是哪一个连接
	int index;
	for (int i = 0; i < con_names.size(); i++)
	{
		if (name == con_names[i])
		{
			index = i;
			break;
		}
	}

	//先在tree_widget中删除该节点下的所有节点
	QList<QTreeWidgetItem*> items2;
	for (int i = 0; i < right_click_item->childCount(); i++)
	{
		items2.append(right_click_item->child(i));
	}

	for (int i = 0; i < items2.size(); i++)
	{
		delete items2[i];
	}

	//再重新拉取即可
	QTreeWidgetItem* pre = right_click_item;

	//连接
	string path = "/";
	vector<string> childnodes;
	con_zks[index]->getChildren(path, childnodes);

	QList<QTreeWidgetItem*> items;
	for (int j = 0; j < childnodes.size(); ++j)
	{
		QTreeWidgetItem* item_new = new QTreeWidgetItem(right_click_item, QStringList(QString::fromStdString(childnodes[j])));
		items.append(item_new);
	}

	//判断有无子节点，如果有，添加一个子节点名字为empty
	struct Stat stat;
	string buffer;
	struct ACL_vector acl;

	for (int k = 0; k < childnodes.size(); k++)
	{
		string path_new = path + childnodes[k];
		con_zks[index]->get_node(path_new, stat, buffer, acl);

		if (stat.numChildren > 0)
		{
			items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/dir.png")));
			QTreeWidgetItem* item_child = new QTreeWidgetItem(items[k], QStringList((QString)"empty"));
			//item_child->setHidden(true);   
		}
		else
			items[k]->setIcon(0, QIcon(QStringLiteral(":/mypics/pics/file.png")));
	}
}

void zkinspector_my::disconnect_item_action()
{
	right_click_item = ui.tree_widget->currentItem();
	QString name = right_click_item->text(0);

	//先确定是哪一个连接
	int index;
	for (int i = 0; i < con_names.size(); i++)
	{
		if (name == con_names[i])
		{
			index = i;
			break;
		}
	}

	//清除该连接
	delete right_click_item;

	//然后清楚有关zk的vector指定位置
	//zks
	vector<ZkClient*>::iterator it1 = con_zks.begin();
	int i = -1;
	for (it1; it1 != con_zks.end(); it1++)
	{
		i++;
		if (i == index)
		{
			con_zks.erase(it1);
			break;
		}
	}

	//names
	vector<QString>::iterator it2 = con_names.begin();
	i = -1;
	for (it2; it2 != con_names.end(); it2++)
	{
		i++;
		if (i == index)
		{
			con_names.erase(it2);
			break;
		}
	}

	//strings
	vector<QString>::iterator it3 = con_strings.begin();
	i = -1;
	for (it3; it3 != con_strings.end(); it3++)
	{
		i++;
		if (i == index)
		{
			con_strings.erase(it3);
			break;
		}
	}

	//times
	vector<QString>::iterator it4 = con_times.begin();
	i = -1;
	for (it4; it4 != con_times.end(); it4++)
	{
		i++;
		if (i == index)
		{
			con_times.erase(it4);
			break;
		}
	}

	//删除array中对应的元素
	QJsonObject ob=array.at(index).toObject();
	QJsonArray::iterator it5 = array.begin();
	for (; it5 != array.end(); it5++)
	{
		if (*it5 == ob)
		{
			array.erase(it5);
			break;
		}
	}

	//更新文件
	write_to_json();
}

