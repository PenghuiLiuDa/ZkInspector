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

	const int kMaxRetryDelay = 10 * 60;   //单位: 秒
	const int kInitRetryDelay = 5;      //单位: 秒

	//zookeeper client related
	const int32_t kInvalidDataVersion = -1;
	const int kMaxNodeValueLength = 64 * 2048;
	const int kMaxPathLength = 512;

	//操作的回调原型///////////////////////////////////////////////////////////////////////////////////////////
	enum ZkErrorCode
	{
		kZKSucceed = 0, // 操作成功,或者 结点存在
		kZKNotExist,  // 节点不存在, 或者 分支结点不存在    1
		kZKError,     // 请求失败      2
		kZKDeleted,   // 节点删除    3
		kZKExisted,   // 节点已存在     4
		kZKNotEmpty,   // 节点含有子节点   5
		kZKLostConnection   //与zookeeper server断开连接   6
	};
};

class ZkClient:public CONNECT
{
private:
	string host_;
	const clientid_t* clientId_;
	//zhandle_t* zhandle_;
	volatile int sessionTimeout_;    //session保持的超时时间
public:
    ZkClient();
	~ZkClient();
	zhandle_t* zhandle_;
	bool haschanged_me=false;   //用于标识是不是自己的客户端主动发生了变化，初始为false

	//对象调用的接口
	bool init(const std::string& host, int timeout, const clientid_t* clientId = NULL, void* context = NULL);   //连接zookeeper
	void get_node(const std::string& path, struct Stat& stat, string &content, struct ACL_vector& acl);         //获取节点信息          
	void create_node(const std::string& path, const std::string& value);                                        //创建节点         
	ZkUtil::ZkErrorCode set_node(const std::string& path, const std::string& value, int32_t version = -1);      //设置节点信息
	void delete_node(const std::string& path, int32_t version = -1);                                             //删除节点
	void close();                                                                                                //断开连接

	//内部调用的接口
	void dump_stat(const struct Stat* stat);
	ZkUtil::ZkErrorCode isExist(const std::string& path);
	ZkUtil::ZkErrorCode create(const std::string& path, const std::string& value);
	ZkUtil::ZkErrorCode createIfNeedCreateParents(const std::string& path, const std::string& value);
	ZkUtil::ZkErrorCode createPersistentDirNode(const std::string& path);
	bool createPersistentDir(const std::string& path);
	ZkUtil::ZkErrorCode deleteNode(const std::string& path, int32_t version = -1);
	ZkUtil::ZkErrorCode deleteRecursive(const std::string& path, int32_t version = -1);
	ZkUtil::ZkErrorCode getChildren(const std::string& path,vector<string> &childnodes);

	//注意是静态函数，不然会有隐含指针this指针，那样就做不了回调函数了，参数个数不一样了
	static void zktest_watcher_g(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);    
};



	
	


