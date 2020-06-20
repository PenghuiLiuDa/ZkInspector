#ifndef NETWORK_DETECT4_H
#define NETWORK_DETECT4_H

#include <QProcess>
#include <QThread>
#include "connect.h"

class network_detect4 : public QThread, public CONNECT
{
	Q_OBJECT            //一定要加这个
public:
	network_detect4();
	virtual void run();
	void stop();
signals:
	void send_network_connect_state4(int state);
private:
	bool flagRunning;   //线程运行标志
	QProcess* network_process;
};
#endif // NETWORK_DETECT_H
