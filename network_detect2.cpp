#include "network_detect2.h"
#include <qDebug>

network_detect2::network_detect2()
{
	flagRunning = true;
}

void network_detect2::run()
{
	while (flagRunning)
	{
		if (connected == false)    //�������Ͽ��Ļ�
		{
			emit send_network_connect_state2(0);    //��������Ͽ����ź�
			break;
		}
		//sleep(1);    //��sleep����CPUռ����
	}
}

void network_detect2::stop()
{
	flagRunning = false;
}