#include "network_detect4.h"
#include <qDebug>

network_detect4::network_detect4()
{
	flagRunning = true;
}

void network_detect4::run()
{
	QString network_cmd = "ping www.baidu.com -n 2 -w 500";
	QString result;
	network_process = new QProcess();    //��Ҫ��this
	while (flagRunning)
	{
		network_process->start(network_cmd);   //����ping ָ��
		network_process->waitForFinished();    //�ȴ�ָ��ִ�����
		result = network_process->readAll();   //��ȡָ��ִ�н��
		if (result.contains(QString("TTL=")))   //������TTL=�ַ�������Ϊ��������
		{
			netstateisOK = true;   //���ñ���Ϊ����
			emit send_network_connect_state4(1);  //�����������ߵ��ź�
			break;
		}
		//sleep(1);   //��sleep����cpuռ���ʡ�
	}
}

void network_detect4::stop()
{
	flagRunning = false;
}