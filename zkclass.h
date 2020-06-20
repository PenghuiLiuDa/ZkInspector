#pragma   once
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "connect.h"
#include "zookeeper.h"
#include "zookeeper_log.h"

using namespace std;

namespace ZkUtil
{
	//common
	const int kThisEpollTimeMs = 10000;
	const int kNoneEvent = 0;
	const int kReadEvent = POLLIN | POLLPRI;
	const int kWriteEvent = POLLOUT;

	const int kMaxRetryDelay = 10 * 60;   //��λ: ��
	const int kInitRetryDelay = 5;      //��λ: ��

	//zookeeper client related
	const int32_t kInvalidDataVersion = -1;
	const int kMaxNodeValueLength = 64 * 2048;
	const int kMaxPathLength = 512;

	//�����Ļص�ԭ��///////////////////////////////////////////////////////////////////////////////////////////
	enum ZkErrorCode
	{
		kZKSucceed = 0, // �����ɹ�,���� ������
		kZKNotExist,  // �ڵ㲻����, ���� ��֧��㲻����    1
		kZKError,     // ����ʧ��      2
		kZKDeleted,   // �ڵ�ɾ��    3
		kZKExisted,   // �ڵ��Ѵ���     4
		kZKNotEmpty,   // �ڵ㺬���ӽڵ�   5
		kZKLostConnection   //��zookeeper server�Ͽ�����   6
	};
};

class ZkClient:public CONNECT
{
private:
	string host_;
	const clientid_t* clientId_;
	//zhandle_t* zhandle_;
	volatile int sessionTimeout_;    //session���ֵĳ�ʱʱ��
public:
    ZkClient();
	~ZkClient();
	zhandle_t* zhandle_;
	bool haschanged_me=false;   //���ڱ�ʶ�ǲ����Լ��Ŀͻ������������˱仯����ʼΪfalse

	//������õĽӿ�
	bool init(const std::string& host, int timeout, const clientid_t* clientId = NULL, void* context = NULL);   //����zookeeper
	void get_node(const std::string& path, struct Stat& stat, string &content, struct ACL_vector& acl);         //��ȡ�ڵ���Ϣ          
	void create_node(const std::string& path, const std::string& value);                                        //�����ڵ�         
	ZkUtil::ZkErrorCode set_node(const std::string& path, const std::string& value, int32_t version = -1);      //���ýڵ���Ϣ
	void delete_node(const std::string& path, int32_t version = -1);                                             //ɾ���ڵ�
	void close();                                                                                                //�Ͽ�����

	//�ڲ����õĽӿ�
	void dump_stat(const struct Stat* stat);
	ZkUtil::ZkErrorCode isExist(const std::string& path);
	ZkUtil::ZkErrorCode create(const std::string& path, const std::string& value);
	ZkUtil::ZkErrorCode createIfNeedCreateParents(const std::string& path, const std::string& value);
	ZkUtil::ZkErrorCode createPersistentDirNode(const std::string& path);
	bool createPersistentDir(const std::string& path);
	ZkUtil::ZkErrorCode deleteNode(const std::string& path, int32_t version = -1);
	ZkUtil::ZkErrorCode deleteRecursive(const std::string& path, int32_t version = -1);
	ZkUtil::ZkErrorCode getChildren(const std::string& path,vector<string> &childnodes);

	//ע���Ǿ�̬��������Ȼ��������ָ��thisָ�룬�����������˻ص������ˣ�����������һ����
	static void zktest_watcher_g(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);    
};



	
	


