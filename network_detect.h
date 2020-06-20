#
#ifndef NETWORK_DETECT_H
#define NETWORK_DETECT_H

#include <QProcess>
#include <QThread>


class network_detect : public QThread
{
	Q_OBJECT            //一定要加这个
public:
	network_detect();
	virtual void run();
	void stop();
signals:
	void send_network_connect_state(int state);
private:
	bool flagRunning;   //线程运行标志
	QProcess* network_process;
};
#endif // NETWORK_DETECT_H
