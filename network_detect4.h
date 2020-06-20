#ifndef NETWORK_DETECT4_H
#define NETWORK_DETECT4_H

#include <QProcess>
#include <QThread>
#include "connect.h"

class network_detect4 : public QThread, public CONNECT
{
	Q_OBJECT            //һ��Ҫ�����
public:
	network_detect4();
	virtual void run();
	void stop();
signals:
	void send_network_connect_state4(int state);
private:
	bool flagRunning;   //�߳����б�־
	QProcess* network_process;
};
#endif // NETWORK_DETECT_H
