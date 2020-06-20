#include "network_detect.h"

network_detect::network_detect()
{
	flagRunning = true;
}

void network_detect::run()
{
	QString network_cmd = "ping www.baidu.com -n 2 -w 500";
	QString result;
	network_process = new QProcess();    //不要加this

	network_process->start(network_cmd);   //调用ping 指令
	network_process->waitForFinished();    //等待指令执行完毕
	result = network_process->readAll();   //获取指令执行结果
	if (result.contains(QString("TTL=")))   //若包含TTL=字符串则认为网络在线
	{
		emit send_network_connect_state(1);  //在线
	}
	else
	{
		emit send_network_connect_state(0); //离线
	}
	//sleep(1);  //加sleep降低CPU占用率
}

void network_detect::stop()
{
	flagRunning = false;
}
