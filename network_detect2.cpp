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
		if (connected == false)    //如果网络断开的话
		{
			emit send_network_connect_state2(0);    //发送网络断开的信号
			break;
		}
		//sleep(1);    //加sleep降低CPU占用率
	}
}

void network_detect2::stop()
{
	flagRunning = false;
}