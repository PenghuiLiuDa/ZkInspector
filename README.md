# ZkInspector

## Introduction

对官方ZooKeeper查看界面ZooInspector的改进。项目在Windows平台下VS的QT中开发，采用C++语言。

功能点如下：

1. zk节点的获取，展示，增加，删除以及刷新
2. 连接记录的导入导出（json格式）
3. 节点的搜索功能
4. zk断网后联网的自动重连
5. 部分节点的URL转码的功能
6. 同时支持多个zk连接

## Environment

1. OS: Windows10
2. VS:  Visual Studio 2019
3. ZooKeeper: Zookeeper-3.5.6
4. Qt: Qt5.9.1

## Model Division

#### ZkClient模块

- [ ] 认识，了解并学习zookeeper的相关概念，API，回调函数，同步调用异步调用
- [ ] 下载zookeeper工具，尝试在windows下调用zookeeper c API
- [ ] 封装zookeeper C API成ZkClient类

#### 用户界面模块

- [ ] 学习QT中树形目录控件treewidget来完成节点的加载与展示
- [ ] 结合封装好的ZkClient类来完成zk的连接，关闭，刷新，增删节点等基本功能
- [ ] 完成部分节点的URL转码功能
- [ ] 连接记录的导入导出以及连接的记忆功能。导入导出是以json格式存储进json文件来实现的。json文件作为资源文件。连接记忆功能：另一个独立的json文件，记录上次退出时的所有zk连接信息，打开界面时自动读取这个文件并连接所有zk
- [ ] 同时支持多个zk连接 搞一组vector即可。

#### 搜索过滤模块

- [ ] 利用QT中treewidget控件中的匹配函数来实现查找功能
- [ ] 学习QT中google  suggest example来对搜索过滤功能进行优化

#### 网络监控模块

- [ ] 打开程序时监测网络是否没有连接，并在网络未连接时提示用户
- [ ] 在断网时打开程序后会一直监测网络状态直到网络在线并提示用户
- [ ] 在程序运行中若捕捉网络断开信息会提示用户
- [ ] 程序运行中网络断开又重连时提示用户，并快速执行zk的重连恢复到网络断开前的状态

静态变量

netstateisOK  监测初始化网络状态  初始化为true

connected   监测运行时网络状态   初始化为true

线程

network_detect1         

network_detect2

network_detect3

network_detect4

![1592538513629](C:\Users\hui\AppData\Roaming\Typora\typora-user-images\1592538513629.png)

## 项目的UML类图

![1592538552829](C:\Users\hui\AppData\Roaming\Typora\typora-user-images\1592538552829.png)



## 项目界面示意图

![1592538669240](C:\Users\hui\AppData\Roaming\Typora\typora-user-images\1592538669240.png)



## 软件下载安装和项目注意事项

ZooKeeper工具下载以及windows下伪集群搭建：https://blog.csdn.net/qq_40123329/article/details/102727071

ZooKeeper程序运行的配置/编译选项：https://blog.csdn.net/qq_40123329/article/details/102763916

在VS中使用QT：https://blog.csdn.net/yzy_1996/article/details/81939610