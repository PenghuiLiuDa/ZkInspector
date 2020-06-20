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
	network_process = new QProcess();    //不要加this
	while (flagRunning)
	{
		network_process->start(network_cmd);   //调用ping 指令
		network_process->waitForFinished();    //等待指令执行完毕
		result = network_process->readAll();   //获取指令执行结果
		if (result.contains(QString("TTL=")))   //若包含TTL=字符串则认为网络在线
		{
			netstateisOK = true;   //设置变量为在线
			emit send_network_connect_state4(1);  //发送重新在线的信号
			break;
		}
		//sleep(1);   //加sleep降低cpu占有率。
	}
}

void network_detect4::stop()
{
	flagRunning = false;
}