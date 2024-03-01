#ifndef CONNECTION_H
#define CONNECTION_H
//实现mysql数据库的增删改查
#include<mysql/mysql.h>
#include<string>
#include<ctime>
#include<iostream>
#define LOG(str) \
    std::cout<<__FILE__<<":"<<__LINE__<<" "<< \
    __TIMESTAMP__<<":"<<str<<std::endl;

class Connection
{
public:
    //初始化数据库操作
    Connection();
    //释放数据库资源
    ~Connection();
    //连接数据库
    bool connect(std::string ip,unsigned short port,std::string user,std::string password,
    std::string dbname);
    //更新操作
    bool update(std::string sql);
    //查询操作
    MYSQL_RES* query(std::string sql);

    //刷新一下连接的起始的空闲时间
    void refreshAliveTime(){aliveTime_=clock();}
    //返回存活的时间
    clock_t getAliveTime() const {return clock()-aliveTime_;}
private:
    MYSQL *conn_;//表示和mysql server的一条连接
    clock_t aliveTime_;//记录进入空闲状态后的存货时间
};
#endif