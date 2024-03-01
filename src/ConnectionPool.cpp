#include"ConnectionPool.h"
#include<fcntl.h>
#include<stdio.h>
//获取该单例对象
ConnectionPool* ConnectionPool::getConnectionpool()
{
    static ConnectionPool pool;
    return &pool;
}

//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE *fp=fopen("/home/My/Desktop/project/DatabaseConnectionPool/src/mysql.conf","r");
    if(fp==nullptr){
        LOG("mysql.conf is not exist!");
        return false;
    }
    while(!feof(fp))
    {
        char line[1024]={0};
        fgets(line,1024,fp);
        std::string str=line;
        int idx=str.find('=',0);
        if(idx==-1){
            continue;
        }
        int endidx=str.find('\n',idx);
        std::string key=str.substr(0,idx);
        std::string value=str.substr(idx+1,endidx-idx-1);
        if(key=="ip"){
            ip_=value;
        }
        else if(key=="port"){
            port_=atoi(value.c_str());
        }
        else if(key=="username"){
            username_=value;
        }
        else if(key=="password"){
            password_=value;
        }
        else if(key=="initSize"){
            initSize_=atoi(value.c_str());
        }
        else if(key=="maxSize"){
            maxSize_=atoi(value.c_str());
        }
        else if(key=="maxIdleTime"){
            maxIdleTime_=atoi(value.c_str());
        }
        else if(key=="connectionTimeOut"){
            connectionTimeout_=atoi(value.c_str());;
        }
        else if(key=="dbname"){
            dbname_=value;
        }
    }
    return true;
}

//连接池的构造函数
ConnectionPool::ConnectionPool(){
    //加载配置项
    if(!loadConfigFile()){
        return;
    }
    //创建初始连接
    for(int i=0;i<initSize_;++i){
        Connection*p=new Connection();
        //std::shared_ptr<Connection>p=std::make_shared<Connection>();
        p->connect(ip_,port_,username_,password_,dbname_);
        p->refreshAliveTime();//刷新一下开始空闲的起始时间
        connectionQue_.push(p);
        isExists_=false;
        connectionCnt_++;
    }
    //启动一个新的线程，作为连接的生产者
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
    produce.detach();
    //再启动一个线程，扫描多余的空闲连接，超过maxIdleTime_的空闲连接，进行多余的连接回收
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask,this));
    scanner.detach();
}

ConnectionPool::~ConnectionPool()//析构函数，资源回收
{
    isExists_=true;
    std::unique_lock<std::mutex> lock(queueMutex_);
    isDel_.notify_all();
    cv.notify_all();
    isDel_.wait(lock,[&]()->bool{return connectionCnt_==initSize_;});
    // std::unique_lock<std::mutex> lock(queueMutex_);
    // cv.notify_all();
    while(!connectionQue_.empty()){
        auto p=connectionQue_.front();
        connectionQue_.pop();
        delete p;
    }
}
//运行在独立的线程中，专门负责回收多余的连接
void ConnectionPool::scannerConnectionTask()
{
    for(;;)
    {
        //通过sleep模拟定时效果
        //std::this_thread::sleep_for(std::chrono::seconds(maxIdleTime_));
        //扫描整个队列
        std::unique_lock<std::mutex> lock(queueMutex_);
        if(!isDel_.wait_for(lock,std::chrono::seconds(maxIdleTime_),[&]()->bool{return isExists_;}))
        {
            while (connectionCnt_ > initSize_) // 当前连接线程总数量大于初始数量
            {
                Connection *p = connectionQue_.front();
                // auto p=connectionQue_.front();
                if (p->getAliveTime() >= (maxIdleTime_ * 1000))
                {
                    connectionQue_.pop();
                    connectionCnt_--;
                    delete p; // 释放资源
                }
                else
                {
                    break; // 队头的都没有超过maxIdleTime,其他连接肯定没有超过
                }
            }
        }
        if(isExists_){
            isDel_.notify_all();
            return;
        }
        //isDel_.notify_all();
    }
}

//运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
    for(;;){
        std::unique_lock<std::mutex> lock(queueMutex_);
        while(!connectionQue_.empty()){
            cv.wait(lock);//队列不为空，此处生产线程进入等待状态
            //if(isExists_)return;
            if(isExists_)return;
        }
        //if(isExists_)return;
        //连接数量没有到达上限，可以继续创建
        if(connectionCnt_<maxSize_){
            Connection *p = new Connection();
            //std::shared_ptr<Connection>p=std::make_shared<Connection>();
            p->connect(ip_, port_, username_, password_, dbname_);
            p->refreshAliveTime();//刷新一下开始空闲的起始时间
            connectionQue_.push(p);
            connectionCnt_++;
        }
        //通知消费者线程
        cv.notify_all();
    }
}
//给外部提供接口，从连接池中获取一个可用的空闲连接
std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(queueMutex_);
    //如果队列是空的
    if(!cv.wait_for(lock,std::chrono::seconds(connectionTimeout_),[&]()->bool{
        return !connectionQue_.empty();}))
    {
        LOG("连接超时了。。。获取连接失败!");
        return nullptr;
    }
    // if(connectionQue_.empty())
    // {
    //     cv.wait_for(lock,std::chrono::seconds(connectionTimeout_));
    //      if(connectionQue_.empty()){
    //          Log();return nullptr;
    //      }
    // }

    //在超时时间内有空闲连接
    /*注意
        shared_ptr析构时，采用默认方式是delete，会把connection资源直接释放掉，我们希望是把connec
        资源归还给connectionQue_,需要自定义删除器
        用lambda表示就行
    */
    std::shared_ptr<Connection> sp(connectionQue_.front(),[&](Connection* pcon){
        //服务器应用线程调用，一定要考虑线程安全
        std::unique_lock<std::mutex> lock(queueMutex_);
        pcon->refreshAliveTime();//刷新一下开始空闲的起始时间
        connectionQue_.push(pcon);
    });
    connectionQue_.pop();
    cv.notify_all();//队列为空了，通知去生产
    return sp;
}