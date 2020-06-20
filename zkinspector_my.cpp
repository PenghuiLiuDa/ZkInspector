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

	//��������
	gs = new GSuggestCompletion(ui.lineEdit, ui.tree_widget);

	QMainWindow::connect(ui.tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(showdatainfo(QTreeWidgetItem*, int)), Qt::UniqueConnection);   //�����ڵ�ʱ��ʾ�ڵ�����ݣ�Ԫ���ݣ�Ȩ����Ϣ����Ӧ�ط�
	QMainWindow::connect(ui.add_btn, SIGNAL(clicked()), this, SLOT(add_node()), Qt::UniqueConnection);               //��ť�¼�����Ӧ���ӽڵ�
	QMainWindow::connect(ui.del_btn, SIGNAL(clicked()), this, SLOT(delete_node()), Qt::UniqueConnection);               //��ť�¼���ɾ����Ӧ�ڵ�
	QMainWindow::connect(ui.submit_ch_btn, SIGNAL(clicked()),this, SLOT(submit_ch()), Qt::UniqueConnection);               //��ť�¼����ύ�޸�
	QMainWindow::connect(ui.disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_action()), Qt::UniqueConnection);    //��ť�¼����ر�����
	QMainWindow::connect(ui.refresh_btn, SIGNAL(clicked()), this, SLOT(refresh_action()), Qt::UniqueConnection);    //��ť�¼���ˢ��
	QMainWindow::connect(ui.tree_widget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(add_data(QTreeWidgetItem*)), Qt::UniqueConnection);  //����۵�����ʱ����ӽڵ�	
	QMainWindow::connect(ui.export_btn, SIGNAL(clicked()), this, SLOT(export_action()), Qt::UniqueConnection);       //��ť�¼���֧�ֵ�������
	QMainWindow::connect(ui.import_btn, SIGNAL(clicked()), this, SLOT(import_action()), Qt::UniqueConnection);        //��ť������֧�ֵ������
	QMainWindow::connect(ui.connect_btn, SIGNAL(clicked()), this, SLOT(connect()), Qt::UniqueConnection);      //��ť��������ȡ���Ӳ�����������

	//����treewidget���ڵ����ֹ���ʱ����ˮƽ����
	ui.tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tree_widget->header()->setStretchLastSection(false);

	//���ñ���
	QString headers;
	headers = "Connections";
	ui.tree_widget->setHeaderLabel(headers);

	//�Ҽ��˵�
	setContextMenuPolicy(Qt::CustomContextMenu);
	QMainWindow::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(sltShowPopMenu(const QPoint&)));

	
	//������ǰ�������״̬
	detect_thread = new network_detect();   
	QMainWindow::connect(detect_thread, SIGNAL(send_network_connect_state(int)), this, SLOT(update_network_state(int)));
	detect_thread->start(); //�����������߳�
	
	//�����Ӻ�������״̬
	detect_thread2 = new network_detect2();
	QMainWindow::connect(detect_thread2, SIGNAL(send_network_connect_state2(int)), this, SLOT(update_network_state2(int)));

	//�����Ӻ�����Ͽ�������û������
	detect_thread3 = new network_detect3();
	QMainWindow::connect(detect_thread3, SIGNAL(send_network_connect_state3(int)), this, SLOT(update_network_state3(int)));
	

	//������ǰ�������߼����û�б��
	detect_thread4 = new network_detect4();
	QMainWindow::connect(detect_thread4, SIGNAL(send_network_connect_state4(int)), this, SLOT(update_network_state4(int)));

	ui.tree_widget->setColumnCount(1);
	ui.tree_widget->show();
}

void zkinspector_my::add_data(QTreeWidgetItem* item)   //����۵�����ʱ��ӽڵ�
{
	//���Ŀǰ��û����ӹ�����ɾ����empty�ڵ㣬����ʼ��ӽڵ�
	if (item->childCount() == 1 && item->child(0)->text(0) == "empty")
	{		
		//ɾ����empty�ڵ�
		delete item->child(0);

		//��ȡ��ǰ�ڵ��·��
		vector<QString> vt;
		QString path;
		QTreeWidgetItem* item_pre = item;
		vt.push_back(item->text(0));
		while (item->parent())  //������ڵ����
		{
			if (item->parent()->parent())  //���үү�ڵ����
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

		//ȷ������һ������
		int index;   
		item = item->parent();  //����itemָ��qtreewidget�е�һ���ڵ�
		QString name = item->text(0);
		for (int i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
			{
				index = i;
				break;
			}
		}

		//��ȡ��ǰ�ڵ���ӽڵ�
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

void zkinspector_my::refresh_action() //refresh��ť
{
	//���û�����ӣ��ᵯ������
	if(!con_strings.size())
		QMessageBox::critical(0,"Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	else
	{
		//�����ui.tree_widget�����ж���
		ui.tree_widget->clear();
		//������ȡ����,ֻ���ڽ������ʾ�޸Ķ���
		for (int i = 0; i < con_strings.size(); i++)
		{
			QJsonObject js = array.at(i).toObject();

			//����
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

			//�ж������ӽڵ㣬����У����һ���ӽڵ�����Ϊempty
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

void zkinspector_my::disconnect_action() //disconnect��ť
{
	//���û������Ҳ�ᵯ������
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
		//ɾ������ʾ��tree_widget�е�������Ŀ
		ui.tree_widget->clear();

		//ͬʱˢ��array
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

	//����
	string url_raw = item->text(0).toStdString().c_str();
	char decd[600];
	memset(decd, '\0', sizeof(decd));
	urldecode(url_raw, decd);

	string url(decd);
	ui.decoded_url->setText(QString::fromStdString(url));


	string protocol;   //��ͷ
	string addr;
	string service_name;    //�ʺŷָ�
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
	string timestamp;    //��β��

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
	if (item->parent())  //����ýڵ���ڸ��׽ڵ�
	{
		if (!item->parent()->parent())  //����ýڵ��үү�ڵ㲻���ڣ�˵������zk�е�һ���ڵ�
		{
			path = "/";
			path += item->text(0);
		}

		else     //�ýڵ㲻��zk�е�һ���ڵ�
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

		//ȷ����һ������
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

		//�ж��Ƿ���Ҫ����
		if ((stat.numChildren == 0) && (item_before->text(0).toStdString().find('%') != string::npos))
		{
			show_url(item_before);
			qDebug() << "I am showing decodede information!!!" << endl;
		}

		//�������Ҫ����
		else
		{
			// ���������Ҫ����������ݵ������
			clear_all();
			qDebug() << "clear all!!!" << endl;
		}
	}
}

void zkinspector_my::add_node_action1()    //���ȷ��֮��ʵ����ӽڵ㣬Ȼ��رմ���
{
	string path = dialog->ui2.lineEdit->text().toStdString();
	vector<string> vt;
	int len = 0;
	bool flag = false;   //����path�Ƿ��ԡ�/����ͷ
	for (int i = 0; i < path.size(); i++)
	{
		if (path[i] == '/' && i != 0)
		{
			vt.push_back(path.substr(i - len, len));
			len = 0;
		}
		else if (path[i] == '/' && i == 0)
		{
			flag = true;   //path�ԡ�/����ͷ  
		}
		else
			len++;
	}
	vt.push_back(path.substr(path.size() - len, len));
	
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();
	//�����ǰ�ڵ�û�и��׽ڵ�
	if (!cur->parent())
	{
		//����ȷ������һ������
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

		if (!flag)  //path�����ԡ�/����ͷ
		{
			path = "/" + path;
		}
		con_zks[index]->create_node(path, "");
	}

	//����и��׽ڵ�
	else
	{
		//����ȷ������һ�������Լ���ǰ�ڵ�·��
		QTreeWidgetItem* pre = cur;
		vector<QString> vt;
		QString path_pre;
		vt.push_back(cur->text(0));
		while (cur->parent())  //������ڵ����
		{
			if (cur->parent()->parent())  //���үү�ڵ����
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


		if (!flag)  //path�����ԡ�/����ͷ
		{
			path = "/" + path;
		}
		path = path_pre.toStdString() + path;;
		con_zks[index]->create_node(path, "");
	}

	//�رմ���
	dialog->close();
}

void zkinspector_my::add_node_action2()    //���ȡ��֮��رմ���
{
	//�رմ���
	dialog->close();
}

void zkinspector_my::add_node() //���add node��ťʱ�������
{
	//���û�����ӣ��ᵯ������
	if (!con_strings.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

	//���û��ѡ���κνڵ㣬Ҳ��������
	else if (!ui.tree_widget->currentItem())
	{
		QMessageBox::critical(0, "Critical message", "Please select one node first!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//����ѡ�еĽڵ�ʱ
	else
	{
		dialog = new Dialog();
		//dialog->setModal(false);

		//ʵ�ֵ�������ʱ����������
		Qt::WindowFlags flags = Qt::Dialog;
		dialog->setWindowFlags(flags);
		dialog->setWindowModality(Qt::ApplicationModal);

		dialog->setWindowTitle(tr("Create Node"));
		dialog->show();

		QMainWindow::connect(dialog->ui2.ok_btn, SIGNAL(clicked()), this, SLOT(add_node_action1()), Qt::UniqueConnection);
		QMainWindow::connect(dialog->ui2.cancel_btn, SIGNAL(clicked()), this, SLOT(add_node_action2()), Qt::UniqueConnection);
	}
}

void zkinspector_my::delete_node() //���delete node��ťʱ�������
{
	//���û�����ӣ��ᵯ������
	if (!con_strings.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

	//�����ǰû��ѡ�нڵ�
	else if(!(ui.tree_widget->currentItem()))
	{
		QMessageBox::critical(0, "Critical message", "Have not yet selected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//�����ǰ�ڵ�������zk�Ľڵ㣬����
	else if (!(ui.tree_widget->currentItem()->parent()))
	{
		QMessageBox::critical(0, "Critical message", "Can't delete connection here!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	else
	{
		deldia = new DelDia();

		//ʵ�ֵ�������ʱ����������
		Qt::WindowFlags flags = Qt::Dialog;
		deldia->setWindowFlags(flags);
		deldia->setWindowModality(Qt::ApplicationModal);

		deldia->setWindowTitle(tr("Delete Node"));
		deldia->show();

		QMainWindow::connect(deldia->ui3.yes_btn, SIGNAL(clicked()), this, SLOT(delete_node_action1()), Qt::UniqueConnection);
		QMainWindow::connect(deldia->ui3.cancel_btn, SIGNAL(clicked()), this, SLOT(delete_node_action2()), Qt::UniqueConnection);
	}
}

void zkinspector_my::delete_node_action1()    //���ȷ��֮����ִ��ɾ�������ٹرմ���  ע�⣡���������ں�̨ɾ����ǰ��ɾ��
{
	//�ں�̨ɾ��
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();
	QTreeWidgetItem* cur_pre = cur;

	//�Ȼ�õ�ǰ�ڵ������·��
	vector<QString> vt;
	QString path;

	vt.push_back(cur->text(0));
	while (cur->parent())  //������ڵ����
	{
		if (cur->parent()->parent())  //���үү�ڵ����
		{
			cur = cur->parent();
			vt.push_back(cur->text(0));
		}
		else
			break;
	}

	//ȷ��������һ������
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

	//��ǰ̨ɾ��
	delete cur_pre;

	//�رմ���
	deldia->close();
}

void zkinspector_my::delete_node_action2()    //���ȡ��֮��رմ���
{
	//�رմ���
	deldia->close();
}

void zkinspector_my::submit_ch()   //���submit_ch_btn�����´���
{
	//���û�����ӣ��ᵯ������
	if (!con_zks.size())
		QMessageBox::critical(0, "Critical message", "Have not yet connected!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

	//���û��ѡ���Ľڵ㣬Ҳ��������
	else if (!ui.tree_widget->currentItem())
	{
		QMessageBox::critical(0, "Critical message", "Please select one node first!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//���ѡ�еĽڵ�û�и��ڵ㣬��ѡ�е�������
	else if (!(ui.tree_widget->currentItem()->parent()))
	{
		QMessageBox::critical(0, "Critical message", "Can't Change Connection here!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
	}

	//ѡ�е�����ͨ�ڵ�
	else
	{
		subdia = new Submit_dialog();

		//ʵ�ֵ�������ʱ����������
		Qt::WindowFlags flags = Qt::Dialog;
		subdia->setWindowFlags(flags);
		subdia->setWindowModality(Qt::ApplicationModal);

		subdia->setWindowTitle(tr("Submit Changes"));
		subdia->show();

		QMainWindow::connect(subdia->ui4.ok_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action1()), Qt::UniqueConnection);
		QMainWindow::connect(subdia->ui4.cancel_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action2()), Qt::UniqueConnection);
	}
}

void zkinspector_my::submit_ch_action1()    //���ȷ�Ϻ�ִ���޸Ĳ��������رմ���
{
	//��õ�ǰѡ�е�item
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();

		//�Ȼ�õ�ǰ�ڵ������·��
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

		//��õ�ǰ�ڵ������
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

		//��ȡ�޸ĺ��ֵ
		QString text_new = ui.textEdit->toPlainText();

		//���ýڵ�����
		con_zks[index]->set_node(path.toStdString(), text_new.toStdString());

		//�رմ���
		subdia->close();

}

void zkinspector_my::submit_ch_action2()    //���ȡ��֮����������Ϊԭ�����ַ������ٹرմ���
{
	
	//�رմ���
	subdia->close();
}

void zkinspector_my::export_action()     //��������
{
	QString filename = QFileDialog::getSaveFileName(this, "Export Json Files", QDir::currentPath(), tr("Json File(*.json)"));  //��ȡ��Ҫ����ɵ��ļ���
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

void zkinspector_my::import_action()       //��������,��׷�ӵķ�ʽ��ͬʱȥ��
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

	QJsonArray array2= jsonDoc.array();  //�洢����������

	//��¼֮ǰ��������
	int before = con_zks.size();
	int index = before - 1;
	for (int i = 0; i < array2.count(); i++)
	{
		QJsonObject js = array2.at(i).toObject();

		//���ж��Ƿ���������
		QString name = js.value("Connect Name").toString();
		int m;
		for (m = 0; m < con_names.size(); m++)
		{
			if (name == con_names[m])
				break;
		}

		//���û��������
		if (m == con_names.size())
		{
			//���±�
			index += 1;

			con_zks.push_back(new ZkClient());
			con_zks[index]->init(js.value("Connect String").toString().toStdString(), js.value("Session Timeout").toString().toInt());
			con_names.push_back(js.value("Connect Name").toString());
			con_strings.push_back(js.value("Connect String").toString());
			con_times.push_back(js.value("Session Timeout").toString());
			array.push_back(js);

			//����
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

			//�ж������ӽڵ㣬����У����һ���ӽڵ�����Ϊempty
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
	if (state)   //��ʾ����
	{
		//�˴�Ӧ�ö�ȡ֮ǰ��json�ļ���Ȼ����ؼ���
		read_from_json();
		init_connect_pre();
		//Ȼ��ʼ�������֮�������״̬
		detect_thread2->start(); 
	}

	//��ʾ������
	else
	{
		netstateisOK = false;
		//�������ʾӦ����������֮���Զ���ʧ
		QMessageBox::critical(0, "Critical message", "Your network doesn't work,PLease check out your network!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);
		//�����ϱߵľ��棬���ﻹӦ��ȥ�������״̬������⵽����״̬���õ�ʱ���ٶ�ȡ֮ǰ��json�ļ������ؼ��ɣ�Ȼ��ȥ�������״̬(2)
		detect_thread4->start();
	}
}

void zkinspector_my::update_network_state2(int state)
{
	if (!state)   //��ʾ����
	{
		qDebug() << "there is something wrong woith your network!!!" << endl;
		//�˴���������
		QMessageBox::critical(0, "Critical message", "NetWork may have something wrong!!!",
			QMessageBox::Ok | QMessageBox::Default,
			/*QMessageBox::Cancel | QMessageBox::Escape*/0, 0);

		//������ִ�м�������Ƿ��������߳�
		detect_thread3->start();
	}
}

void zkinspector_my::update_network_state3(int state)
{
	if (state)   //��ʾ���ߺ��������ˡ�
	{

		QMessageBox::information(this, tr("Congratulations!!!"), QStringLiteral("Connected again!!!"), QMessageBox::Ok);

		//ִ������
		for (int i = 0; i < con_zks.size(); i++)
		{
			con_zks[i]->init(con_strings[i].toStdString(), con_times[i].toInt());
		}

		//������ִ�м�������Ƿ��ֶϿ�
		detect_thread2->start();
	}
}

void zkinspector_my::update_network_state4(int state)
{
	if (state)    //˵������ǰ��������
	{
		//���������Զ��ر�
		QMessageBox::information(this, tr("Congratulations!!!"), QStringLiteral("Connected again!!!"), QMessageBox::Ok);
		read_from_json();
		init_connect_pre();
		//��������������Ƿ�Ͽ�
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

	//�ж������ӽڵ㣬����У����һ���ӽڵ�����Ϊempty
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

	//����֮�󣬰���ε�������Ϣ����push��array��
	QJsonObject ob;
	ob.insert("Connect String", con_strings[index]);
	ob.insert("Session Timeout", con_times[index]);
	ob.insert("Connect Name", con_names[index]);
	array.push_back(ob);

	//array�б仯����Ҫ����
	write_to_json();

	qDebug() << "write successfully!" << endl;
}

void zkinspector_my::connect()
{
	condia = new ConnectDia();

	//ʵ�ֵ�������ʱ����������
	Qt::WindowFlags flags = Qt::Dialog;
	condia->setWindowFlags(flags);
	condia->setWindowModality(Qt::ApplicationModal);

	condia->setWindowTitle(tr("Add Connections."));
	condia->show();

	QMainWindow::connect(condia->ui5.ok_btn, SIGNAL(clicked()), this, SLOT(connect_action1()), Qt::UniqueConnection);
	QMainWindow::connect(condia->ui5.cancel_btn, SIGNAL(clicked()), this, SLOT(connect_action2()), Qt::UniqueConnection);
}

void zkinspector_my::connect_action1()      //������ҳ����ȷ��
{
	//�ж��Ƿ�������,�������������н������Ĳ���
	if (condia->ui5.lineEdit->text().size() && condia->ui5.lineEdit_2->text().size() && condia->ui5.lineEdit_3->text().size())
	{
		//���ж�������û���ظ�
		QString name = condia->ui5.lineEdit_3->text();
		int i;
		for (i = 0; i < con_names.size(); i++)
		{
			if (name == con_names[i])
				break;
		}

		if (i == con_names.size())   //˵������û���ظ�
		{
			//���ж�ip��ַ�������Ƿ���Ч
			QProcess* network_process;
			QString network_cmd = "ping " + condia->ui5.lineEdit->text() + " -n 2 -w 500";
			QString result;
			network_process = new QProcess();

			network_process->start(network_cmd);   //����ping ָ��
			network_process->waitForFinished();    //�ȴ�ָ��ִ�����
			result = network_process->readAll();   //��ȡָ��ִ�н��
			if (result.contains(QString("TTL=")))   //������TTL=�ַ�������Ϊip��������Ч,��ʼ����
			{
				//��ʼ����
				//���ȼ��ip�������Ƿ��Ѿ�������
				int i = 0;
				for (; i < con_strings.size(); i++)
				{
					if (con_strings[i] == condia->ui5.lineEdit->text())
						QMessageBox::critical(0, "Connect Error", "Have Yet Connected!!!",
							QMessageBox::Ok | QMessageBox::Default,
							QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
					return;
				}

				//ִ�е�����˵��֮ǰû�������˵�ַ,��ʼ���Ӳ���
				int index = (0 == con_strings.size() ? 0 : (con_strings.size() - 1));
				con_zks.push_back(new ZkClient());
				con_zks[index]->init(condia->ui5.lineEdit->text().toStdString(), condia->ui5.lineEdit_2->text().toInt());

				con_strings.push_back(condia->ui5.lineEdit->text());
				con_times.push_back(condia->ui5.lineEdit_2->text());
				con_names.push_back(condia->ui5.lineEdit_3->text());

				init_connect(index, condia->ui5.lineEdit_3->text());
				condia->close();
			}

			//�����Ч���ȹرմ��ڣ��ٵ���ip��������Ч����ʾ
			else
			{
				condia->close();
				QMessageBox::critical(0, "Connect Error", "Please make sure your ip correct!!!",
					QMessageBox::Ok | QMessageBox::Default,
					QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
			}
		}
		
		else  //�����ظ�
		{
			QMessageBox::critical(0, "Connect Error", "Duplicated Connect Name!!!",
				QMessageBox::Ok | QMessageBox::Default,
				QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
		}

	}

	//��ʾû�����뵫�����ر����Ӵ���
	else
	{
		QMessageBox::critical(0, "Critical message", "Please make sure you have input everything required!!!",
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape/*0*/, 0);
	}
}

void zkinspector_my::connect_action2()     //������ҳ����ȡ��
{
	//�رմ���
	condia->close();
}

void zkinspector_my::write_to_json()   //д��json�ļ��Ĳ�����ֻҪ��array�������κθ��µı仯�������������
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

void zkinspector_my::read_from_json()   // �մ򿪽����ʱ���ȡ��
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
	//��ʼ���������ļ��еļ�¼
	for (int i = 0; i < array.count(); i++)
	{
		QJsonObject js = array.at(i).toObject();
		con_zks.push_back(new ZkClient());
		con_zks[i]->init(js.value("Connect String").toString().toStdString(),js.value("Session Timeout").toString().toInt());
		con_names.push_back(js.value("Connect Name").toString());
		con_strings.push_back(js.value("Connect String").toString());
		con_times.push_back(js.value("Session Timeout").toString());

		//����
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

		//�ж������ӽڵ㣬����У����һ���ӽڵ�����Ϊempty
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

void zkinspector_my::sltShowPopMenu(const QPoint&)  //�һ��Ĳۺ���
{
	right_click_item = ui.tree_widget->currentItem();
	if (right_click_item)
	{
		//���ѡ��һ���ڵ㣬��ʱ���ɲ˵�
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

	//��ȷ������һ������
	int index;
	for (int i = 0; i < con_names.size(); i++)
	{
		if (name == con_names[i])
		{
			index = i;
			break;
		}
	}

	//����tree_widget��ɾ���ýڵ��µ����нڵ�
	QList<QTreeWidgetItem*> items2;
	for (int i = 0; i < right_click_item->childCount(); i++)
	{
		items2.append(right_click_item->child(i));
	}

	for (int i = 0; i < items2.size(); i++)
	{
		delete items2[i];
	}

	//��������ȡ����
	QTreeWidgetItem* pre = right_click_item;

	//����
	string path = "/";
	vector<string> childnodes;
	con_zks[index]->getChildren(path, childnodes);

	QList<QTreeWidgetItem*> items;
	for (int j = 0; j < childnodes.size(); ++j)
	{
		QTreeWidgetItem* item_new = new QTreeWidgetItem(right_click_item, QStringList(QString::fromStdString(childnodes[j])));
		items.append(item_new);
	}

	//�ж������ӽڵ㣬����У����һ���ӽڵ�����Ϊempty
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

	//��ȷ������һ������
	int index;
	for (int i = 0; i < con_names.size(); i++)
	{
		if (name == con_names[i])
		{
			index = i;
			break;
		}
	}

	//���������
	delete right_click_item;

	//Ȼ������й�zk��vectorָ��λ��
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

	//ɾ��array�ж�Ӧ��Ԫ��
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

	//�����ļ�
	write_to_json();
}

