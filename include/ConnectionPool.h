#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include<thread>
#include<functional>
#include<mutex>
#include<string>
#include<iostream>
#include<queue>
#include<memory>
#include<atomic>
#include<condition_variable>
#include"Connection.h"
//实现连接池功能(单例模式)
class ConnectionPool
{
public:
    //获取该单例对象
    static ConnectionPool* getConnectionpool();
    //给外部提供接口，从连接池中获取一个可用的空闲连接
    std::shared_ptr<Connection> getConnection();
     ~ConnectionPool();//析构函数，资源回收
private:
    ConnectionPool();//构造函数私有化

    //从配置文件中加载配置项
    bool loadConfigFile();

    //运行在独立的线程中，专门负责生产新连接
    void produceConnectionTask();

    //运行在独立的线程中，专门负责回收多余的连接
    void scannerConnectionTask();

    std::string ip_;//mysql的ip地址
    unsigned short port_;//mysql的端口号 3306
    std::string  username_;//mysql的登录用户名
    std::string password_;//mysql登录密码
    std::string dbname_;//连接的数据库名称
    
    int initSize_;//连接池的初始连接量
    int maxSize_;//连接池的最大连接量
    int maxIdleTime_;//连接池最大空闲时间
    int connectionTimeout_;//连接池获取连接的超时时间

    std::atomic_int32_t connectionCnt_;//记录连接所创建的connection连接的总数量
    std::queue<Connection*> connectionQue_;//存储mysql连接的队列
    std::mutex queueMutex_;//维护连接队列的线程安全互斥锁
    std::condition_variable cv;//用于连接生产线程和连接消费线程的通信
    std::atomic_bool isExists_;//连接池是否消亡
    std::condition_variable isDel_;
};
#endif