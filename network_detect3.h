#ifndef NETWORK_DETECT3_H
#define NETWORK_DETECT3_H

#include <QThread>
#include <QProcess>
#include "connect.h"


class network_detect3 : public QThread, public CONNECT
{
	Q_OBJECT            //一定要加这个
public:
	network_detect3();
	virtual void run();
	void stop();
signals:
	void send_network_connect_state3(int state);
private:
	bool flagRunning;   //线程运行标志
	QProcess* network_process;
};
#endif  // NETWORK_DETECT_H
