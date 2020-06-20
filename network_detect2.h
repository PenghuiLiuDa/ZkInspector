#ifndef NETWORK_DETECT2_H
#define NETWORK_DETECT2_H

#include <QThread>
#include "connect.h"


class network_detect2 : public QThread,public CONNECT
{
	Q_OBJECT            //一定要加这个
public:
	network_detect2();
	virtual void run();
	void stop();
signals:
	void send_network_connect_state2(int state);
private:
	bool flagRunning;   //线程运行标志
};
#endif // NETWORK_DETECT_H