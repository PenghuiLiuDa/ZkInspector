#include "zkclass.h"
#include <qDebug>


#define ctime_r(tctime, buffer) ctime_s (buffer, 40, tctime)


//��Ĺ��캯��
ZkClient::ZkClient() :
	host_(""),
	clientId_(NULL),
	zhandle_(NULL),
	sessionTimeout_(50000)
{
}

//��������
ZkClient::~ZkClient()
{
	if (clientId_)
	{
		delete clientId_;
		clientId_ = NULL;
	}
	if (zhandle_)
	{
		zookeeper_close(zhandle_);
	}
}

//չʾ�ڵ���Ϣ
void ZkClient::dump_stat(const struct Stat* stat)
{
	char tctimes[40];
	char tmtimes[40];
	time_t tctime;
	time_t tmtime;

	if (!stat) {
		//fprintf(stderr, "null\n");
		qDebug() << "wrong while showing information!" << endl;
		return;
	}
	tctime = (stat->ctime) / 1000;
	tmtime = (stat->mtime) / 1000;

	ctime_r(&tmtime, tmtimes);
	ctime_r(&tctime, tctimes);

	qDebug() << "ACL Version=" << stat->version;
	qDebug() << "Creation Time=" << tctimes;
	qDebug() << "Children Version=" << stat->cversion;
	qDebug() << "Creation ID=" << stat->czxid;
	qDebug() << "Data Length=" << stat->dataLength;
	qDebug() << "Ephemeral owner=" << stat->ephemeralOwner;
	qDebug() << "Last Modified Time=" << tmtimes;
	qDebug() << "Modified ID=" << stat->mzxid;
	qDebug() << "Number of Children=" << stat->numChildren;
	qDebug() << "Node ID=" << stat->pzxid;
	qDebug() << "ACL Version=" << stat->aversion;
}

//ȫ�ֻص�������������Ҫ��Ϊ�˼������״̬
void ZkClient::zktest_watcher_g(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
	if (type == ZOO_SESSION_EVENT && ((state == ZOO_EXPIRED_SESSION_STATE)||(state==ZOO_CONNECTING_STATE)))
	{
		qDebug() << "The Connection has lost!!!" << endl;
		connected = false;
	}	
}

//����zk
bool ZkClient::init(const std::string& host, int timeout, const clientid_t* clientId, void* context)
{
	sessionTimeout_ = timeout;
	host_ = host + ":2181";
	if (clientId == NULL)
	{
		clientId_ = NULL;
	}

	zhandle_ = zookeeper_init(host_.c_str(),zktest_watcher_g, sessionTimeout_, clientId_, context, 0);
	if (zhandle_ == NULL)
	{
		//fprintf(stderr, "Error when connecting to zookeeper servers...\n");
		qDebug() << "Error when connecting to zookeeper servers...";
		return false;
	}
	qDebug() << "connect successfully!" << endl;
	return true;
};

//Ҳ���Բο���ʱ�Ľ������

//��ȡ�ڵ���Ϣ
void ZkClient::get_node(const std::string& path,struct Stat& stat,string &content,struct ACL_vector &acl)
{
	//struct Stat stat;
	char buffer[ZkUtil::kMaxNodeValueLength];
	memset(buffer, '\0', sizeof(buffer));
	int buffer_len = sizeof(buffer);
	int rc = zoo_get(zhandle_, path.c_str(), 0, buffer, &buffer_len, &stat);
	content = buffer;
	//qDebug() << "the content of node :" << buffer;
	//printf("%s\n", buffer);

	//qDebug() << "the matadata of node:";
	//dump_stat((const struct Stat*) & stat);

	//qDebug() << "the ownership of node:";
	//struct ACL_vector acl;
	//struct Stat stat2;
	int flag = zoo_get_acl(zhandle_, path.c_str(), &acl, &stat);
	/*if (flag == ZOK)
	{
		qDebug() << "---------------- - the ACL of " << path.c_str() << ":------------";
		//printf("-----------------the ACL of %s:\n------------", path.c_str());
		qDebug() << acl.count << acl.data->perms << endl;
		//printf("%d\n", acl.count);
		//printf("%d\n", acl.data->perms);
		if (acl.data->perms == ZOO_PERM_ALL)
			qDebug() << "visiting ownership:" << "Read, Write, Create, Delete, Admin" << endl;
		else if (acl.data->perms == ZOO_PERM_READ)
			qDebug() << "visiting ownership:" << "Read" << endl;
		else if (acl.data->perms == ZOO_PERM_WRITE)
			qDebug() << "visiting ownership:" << "Write" << endl;
		else if (acl.data->perms == ZOO_PERM_CREATE)
			qDebug() << "visiting ownership:" << "Create" << endl;
		else if (acl.data->perms == ZOO_PERM_DELETE)
			qDebug() << "visiting ownership:" << "Delete" << endl;
		else
			qDebug() << "visiting ownership:" << "Admin" << endl;
		qDebug() << acl.data->id.scheme << acl.data->id.id;
		//printf("%s\n", acl.data->id.scheme);
		//printf("%s\n", acl.data->id.id);
	}*/
}

//��ȡ�ӽڵ�
ZkUtil::ZkErrorCode ZkClient::getChildren(const std::string& path, std::vector<std::string>& childNodes)
{
	//if (isConnected() == false) { return  ZkUtil::kZKLostConnection; }

	int isWatch = 0;
	struct String_vector strings = { 0, NULL };
	int rc = zoo_get_children(zhandle_, path.c_str(), isWatch, &strings);
	//LOG_DEBUG << "[ZkClient::GetChildren] zoo_get_children path:" << path << ", rc:" << rc << ", session Handle:" << handle_;
	if (rc == ZOK)
	{
		for (int i = 0; i < strings.count; ++i)
		{
			childNodes.push_back(strings.data[i]);
		}
		deallocate_String_vector(&strings);
		return ZkUtil::kZKSucceed;
	}
	else if (rc == ZNONODE)
	{
		return ZkUtil::kZKNotExist;
	}
	return ZkUtil::kZKError;
}

//�жϽڵ��Ƿ����
ZkUtil::ZkErrorCode ZkClient::isExist(const std::string& path)
{
	//if (isConnected() == false) { return  ZkUtil::kZKLostConnection; }

	int isWatch = 0;
	int rc = zoo_exists(zhandle_, path.c_str(), isWatch, NULL);
	//LOG_DEBUG << "[ZkClient::IsExist] zoo_exists path:" << path << ", rc:" << rc << ", session Handle:" << handle_;
	if (rc == ZOK)
	{
		return ZkUtil::kZKSucceed;
	}
	else if (rc == ZNONODE)
	{
		return ZkUtil::kZKNotExist;
	}
	return ZkUtil::kZKError;
}

//�����ڵ�  
ZkUtil::ZkErrorCode ZkClient::create(const std::string& path, const std::string& value)
{
	//if (isConnected() == false) { return  ZkUtil::kZKLostConnection;}

	int flags = 0;
	/*if (isTemp == true)
	{
		flags |= ZOO_EPHEMERAL;
	}
	if (isSequence == true)
	{
		flags |= ZOO_SEQUENCE;
	}*/

	char buffer[ZkUtil::kMaxPathLength] = { 0 };
	int buffer_len = sizeof(buffer);
	int rc = zoo_create(zhandle_, path.c_str(), value.c_str(), value.size(), &ZOO_OPEN_ACL_UNSAFE,
		flags, buffer, buffer_len);
	/* LOG_DEBUG << "[ZkClient::Create] zoo_create path:" << path << ", value:" << value
			   << ", isTemp:" << isTemp << ", isSeq:" << isSequence
			   << ", rc:" << rc << ", session Handle:" << handle_;*/
	if (rc == ZOK)
	{
		qDebug() << "node created successfully!";
		return ZkUtil::kZKSucceed;
	}
	else if (rc == ZNONODE)
	{
		return ZkUtil::kZKNotExist;
	}
	else if (rc == ZNODEEXISTS)
	{
		qDebug() << "the node has existed!";
		return ZkUtil::kZKExisted;
	}
	return ZkUtil::kZKError;
}

ZkUtil::ZkErrorCode ZkClient::createIfNeedCreateParents(const std::string& path, const std::string& value)
{
	ZkUtil::ZkErrorCode ec = create(path, value);
	/* LOG_DEBUG << "ZkClient::CreateIfNeedCreateParents Create path:" << path << ", value:" << value << ", isTemp" << isTemp
			<< ", isSeq:" <<isSequence << ", ec:" << ec << ", session Handle:" << handle_;*/
	if (ec == ZkUtil::kZKNotExist)  //��֧��㲻����
	{
		string::size_type pos = path.rfind('/');
		if (pos == string::npos)
		{
			/*LOG_ERROR << "[ZkClient::CreateIfNeedCreateParents] Can't find / character, create node failed! path:"
					<< path << ", session Handle:" << handle_;*/
			return ZkUtil::kZKError;
		}
		else
		{
			std::string parentDir = path.substr(0, pos);
			//�ݹ鴴�� ���� ��Ŀ¼���
			if (createPersistentDir(parentDir) == true)
			{
				//����Ҷ�ӽ��
				return create(path, value);
			}
			else
			{
				/*LOG_ERROR << "[ZkClient::CreateIfNeedCreateParents] create dir failed! dir:" << parentDir
							<< ", path:" << path << ", session Handle:" << handle_;*/
				return ZkUtil::kZKError;
			}
		}
	}
	else
	{
		return ec;
	}
}

ZkUtil::ZkErrorCode ZkClient::createPersistentDirNode(const std::string& path)
{
	//if (isConnected() == false) { return  ZkUtil::kZKLostConnection;}

	int flags = 0;   //��֧·���Ľ�� Ĭ���� �־��͡���˳����
	int rc = zoo_create(zhandle_, path.c_str(), NULL, -1, &ZOO_OPEN_ACL_UNSAFE,
		flags, NULL, 0);
	/*LOG_DEBUG << "[ZkClient::CreatePersistentDirNode] handle: "<< handle_ << "path:" << path << "rc:" << rc
			  << ", session Handle:" << handle_;*/
	if (rc == ZOK)
	{
		qDebug() << "node create successfylly!";
		return ZkUtil::kZKSucceed;
	}
	else if (rc == ZNONODE)
	{
		return ZkUtil::kZKNotExist;
	}
	else if (rc == ZNODEEXISTS)
	{
		return ZkUtil::kZKExisted;
	}
	return ZkUtil::kZKError;
}

//����ʽ ����Ŀ¼���
bool ZkClient::createPersistentDir(const std::string& path)
{
	//LOG_DEBUG << "[ZkClient::CreatePersistentDir] path:" << path  << ", session Handle:" << handle_;
	//�ȳ��Դ��� ���� Ŀ¼���
	ZkUtil::ZkErrorCode ec = createPersistentDirNode(path);
	if (ec == ZkUtil::kZKSucceed || ec == ZkUtil::kZKExisted)
	{
		return true;
	}
	else if (ec == ZkUtil::kZKNotExist)  //���ʧ�ܣ����ȳ��� �������� Ŀ¼��㣬Ȼ�󴴽� ����Ŀ¼���
	{
		string::size_type pos = path.rfind('/');
		if (pos == string::npos)
		{
			/*LOG_ERROR << "[ZkClient::CreatePersistentDir] Can't find / character, create dir failed! path:"
					<< path  << ", session Handle:" << handle_;*/
			return false;
		}
		else
		{
			std::string parentDir = path.substr(0, pos);
			if (createPersistentDir(parentDir) == true)  //������Ŀ¼�ɹ�
			{
				return createPersistentDir(path);
			}
			else
			{
				/*LOG_ERROR << "[ZkClient::CreatePersistentDir] create parent dir failed! dir:" << parentDir
						  << ", session Handle:" << handle_;*/
				return false;
			}
		}
	}
	else  //ZkUtil::kZKError
	{
		/*LOG_ERROR << "[ZkClient::CreatePersistentDir] CreatePersistentDirNode failed! path:" << path
				 << ", session Handle:" << handle_;*/
		return false;
	}
}

//�����ڵ������õ��ĺ���
void ZkClient::create_node(const std::string& path, const std::string& value)
{
	if (path.find('/') == string::npos)
	{
		const string path_new = "/" + path;
		create(path_new, value);
	}
	else
	{
		if (path[0] != '/')
		{
			const string path_new = "/" + path;
			createIfNeedCreateParents(path_new, value);
		}
		createIfNeedCreateParents(path, value);
	}
}


//���ýڵ�����
ZkUtil::ZkErrorCode ZkClient::set_node(const std::string& path, const std::string& value, int32_t version)
{
	//if (isConnected() == false) { return  ZkUtil::kZKLostConnection; }

	int rc = zoo_set(zhandle_, path.c_str(), value.c_str(), value.size(), version);
	/*LOG_DEBUG << "[ZkClient::Set] zoo_set path:" << path << ", value:" << value << ", version:" << version
		<< ", rc:" << rc << ", session Handle:" << handle_;*/
	if (rc == ZOK)
	{
		qDebug() << "advise successfully!" << endl;
		return ZkUtil::kZKSucceed;
	}
	else if (rc == ZNONODE)
	{
		return ZkUtil::kZKNotExist;
	}
	return ZkUtil::kZKError;
}

//ɾ���ڵ�
ZkUtil::ZkErrorCode ZkClient::deleteNode(const std::string& path, int32_t version)
{
	//if (isConnected() == false) { return  ZkUtil::kZKLostConnection; }

	int rc = zoo_delete(zhandle_, path.c_str(), version);
	/*LOG_DEBUG << "[ZkClient::Delete] zoo_delete path:" << path << ", version:" << version << ", rc:" << rc
		<< ", session Handle:" << handle_;*/
	if (rc == ZOK)
	{
		qDebug() << "delete node successfully!";
		return ZkUtil::kZKSucceed;
	}
	else if (rc == ZNONODE)
	{
		return ZkUtil::kZKNotExist;
	}
	else if (rc == ZNOTEMPTY)   //�ýڵ����������ӽڵ�
	{
		return ZkUtil::kZKNotEmpty;
	}
	return ZkUtil::kZKError;
}

//�ݹ�ɾ���ڵ�
/*
 * return:
 *      kZKSucceed: ɾ���ɹ�
 *      kZKNotExist: ����Ѳ�����
 *      kZKError: ����ʱ���ִ���
 */
ZkUtil::ZkErrorCode ZkClient::deleteRecursive(const std::string& path, int32_t version /*= -1*/)
{
	//��ȡchild ���
	std::vector<std::string> childNodes;
	childNodes.clear();
	ZkUtil::ZkErrorCode ec = getChildren(path, childNodes);
	if (ec == ZkUtil::kZKNotExist)
	{
		return ZkUtil::kZKSucceed;
		qDebug() << "delete successfully!";
	}
	else if (ec != ZkUtil::kZKSucceed)
	{
		//LOG_ERROR << "[ZkClient::DeleteRecursive] GetChildren failed! ec:" << ec
			//<< ", path:" << path << ", version:" << version << ", session Handle:" << handle_;
		return ZkUtil::kZKError;
	}
	else  //ZkUtil::kZKSucceed
	{
		//ɾ�� child ���
		std::vector<std::string>::iterator iter = childNodes.begin();
		for (; iter != childNodes.end(); iter++)
		{
			std::string childPath = path + "/" + (*iter);
			ZkUtil::ZkErrorCode ec1 = deleteRecursive(childPath, -1);   //ɾ���ӽ�� �� �����version

			if (ec1 != ZkUtil::kZKSucceed &&
				ec1 != ZkUtil::kZKNotExist)
			{
				//LOG_ERROR << "[ZkClient::DeleteRecursive] GetChildren failed! ec:" << ec
					//<< ", path:" << path << ", version:" << version << ", session Handle:" << handle_;
				return ZkUtil::kZKError;
			}
		}

		//ɾ����֧���
		return deleteNode(path, version);
	}
}

//ɾ���ڵ������õ��ĺ���
void ZkClient::delete_node(const std::string& path, int32_t version)
{
	//����ʲô����������õݹ�ɾ��
	deleteRecursive(path, version);
}

//�Ͽ�����
void ZkClient::close()
{
	int rc = zookeeper_close(zhandle_);
}



