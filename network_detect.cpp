#include "network_detect.h"

network_detect::network_detect()
{
	flagRunning = true;
}

void network_detect::run()
{
	QString network_cmd = "ping www.baidu.com -n 2 -w 500";
	QString result;
	network_process = new QProcess();    //��Ҫ��this

	network_process->start(network_cmd);   //����ping ָ��
	network_process->waitForFinished();    //�ȴ�ָ��ִ�����
	result = network_process->readAll();   //��ȡָ��ִ�н��
	if (result.contains(QString("TTL=")))   //������TTL=�ַ�������Ϊ��������
	{
		emit send_network_connect_state(1);  //����
	}
	else
	{
		emit send_network_connect_state(0); //����
	}
	//sleep(1);  //��sleep����CPUռ����
}

void network_detect::stop()
{
	flagRunning = false;
}
