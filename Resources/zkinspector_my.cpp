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
	connect(ui.tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(showdatainfo(QTreeWidgetItem*, int)), Qt::UniqueConnection);   //�����ڵ�ʱ��ʾ�ڵ�����ݣ�Ԫ���ݣ�Ȩ����Ϣ����Ӧ�ط�
	connect(ui.connect_btn, SIGNAL(clicked()), this, SLOT(connect_and_show_node()), Qt::UniqueConnection);   //��ť�¼� connect��ť�������zk�����ӹ�����Ŀ¼���ļ��ع���
	connect(ui.add_btn, SIGNAL(clicked()), this, SLOT(add_node()), Qt::UniqueConnection);               //��ť�¼�����Ӧ���ӽڵ�
	connect(ui.del_btn, SIGNAL(clicked()), this, SLOT(delete_node()), Qt::UniqueConnection);               //��ť�¼���ɾ����Ӧ�ڵ�
	connect(ui.submit_ch_btn, SIGNAL(clicked()), this, SLOT(submit_ch()), Qt::UniqueConnection);               //��ť�¼����ύ�޸�
	connect(ui.disconnect_btn, SIGNAL(clicked()), this, SLOT(disconnect_action()), Qt::UniqueConnection);    //��ť�¼����ر�����
	connect(ui.refresh_btn, SIGNAL(clicked()), this, SLOT(refresh_action()), Qt::UniqueConnection);    //��ť�¼���ˢ��
}

void zkinspector_my::refresh_action() //refresh��ť
{
	//�����ui.tree_widget�����ж���
	ui.tree_widget->clear();

	//����connect����������ȡ����
	connect_and_show_node();
}

void zkinspector_my::disconnect_action() //disconnect��ť
{
	zk.close();
	//ɾ������ʾ��tree_widget�е�������Ŀ
	ui.tree_widget->clear();
}

void zkinspector_my::connect_and_show_node() //connect��ť
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

void zkinspector_my::add_node_action1()    //���ȷ��֮��ʵ����ӽڵ㣬Ȼ��رմ���
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
			flag = true;   //path�ԡ�/����ͷ  
		}
		else
			len++;
	}
	vt.push_back(path.substr(path.size() - len, len));

	//���Ŀǰ��û��ѡ���κνڵ�
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
		if (!flag)  //path�����ԡ�/����ͷ
		{
			path = "/" + path;
		}
		zk.create_node(path, "");

	}

	//˵����ǰ��Ŀ¼�����б�ѡ�е���Ŀ
	else
	{
		QTreeWidgetItem* cur = ui.tree_widget->currentItem();
		QTreeWidgetItem* pre = cur;
		for (int i = 0; i < vt.size(); i++)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(cur, QStringList(QString::fromStdString(vt[i])));
			cur = item;
		}
		if (!flag)  //path�����ԡ�/����ͷ
		{
			path = "/" + path;
		}

		//�Ȼ�õ�ǰ�ڵ������·��
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
	dialog = new Dialog();
	//dialog->setModal(false);
	dialog->setWindowTitle(tr("Create Node"));
	dialog->show();
	
	connect(dialog->ui2.ok_btn, SIGNAL(clicked()), this, SLOT(add_node_action1()), Qt::UniqueConnection);
	connect(dialog->ui2.cancel_btn, SIGNAL(clicked()), this, SLOT(add_node_action2()), Qt::UniqueConnection);
}

void zkinspector_my::delete_node() //���delete node��ťʱ�������
{
	deldia = new DelDia();
	deldia->setWindowTitle(tr("Delete Node"));
	deldia->show();
	
	connect(deldia->ui3.yes_btn, SIGNAL(clicked()), this, SLOT(delete_node_action1()), Qt::UniqueConnection);
	connect(deldia->ui3.cancel_btn, SIGNAL(clicked()), this, SLOT(delete_node_action2()), Qt::UniqueConnection);
}

void zkinspector_my::delete_node_action1()    //���ȷ��֮����ִ��ɾ�������ٹرմ���
{
	//�ڴ���ɾ��
	QList<QTreeWidgetItem*> s = ui.tree_widget->selectedItems();
	// ����ɾ��
	for (auto item : s) {
		delete item;
	}

	//�ں�̨ɾ��
	QTreeWidgetItem* cur = ui.tree_widget->currentItem();

	//�Ȼ�õ�ǰ�ڵ������·��
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
	subdia = new Submit_dialog();
	subdia->setWindowTitle(tr("Submit Changes"));
	subdia->show();

	connect(subdia->ui4.ok_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action1()), Qt::UniqueConnection);
	connect(subdia->ui4.cancel_btn, SIGNAL(clicked()), this, SLOT(submit_ch_action2()), Qt::UniqueConnection);
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
		cur = cur->parent();
		vt.push_back(cur->text(0));
	}
	reverse(vt.begin(), vt.end());
	for (int i = 0; i < vt.size(); i++)
	{
		path += "/" + vt[i];
	}

	//��ȡ�޸ĺ��ֵ
	QString text_new = ui.textEdit->toPlainText();

	//���ýڵ�����
	zk.set_node(path.toStdString(), text_new.toStdString());

	//�رմ���
	subdia->close();
}

void zkinspector_my::submit_ch_action2()    //���ȡ��֮����������Ϊԭ�����ַ������ٹرմ���
{
	
	//�رմ���
	subdia->close();
}











