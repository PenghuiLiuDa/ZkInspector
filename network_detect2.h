#ifndef NETWORK_DETECT2_H
#define NETWORK_DETECT2_H

#include <QThread>
#include "connect.h"


class network_detect2 : public QThread,public CONNECT
{
	Q_OBJECT            //һ��Ҫ�����
public:
	network_detect2();
	virtual void run();
	void stop();
signals:
	void send_network_connect_state2(int state);
private:
	bool flagRunning;   //�߳����б�־
};
#endif // NETWORK_DETECT_H