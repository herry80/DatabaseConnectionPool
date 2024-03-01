#include"ConnectionPool.h"
#include"Connection.h"
#include<stdio.h>
int main()
{
    // Connection conn;
    // char sql[1024]={0};
    // sprintf(sql,"insert into user(name,age,sex) values('%s',%d,'%s')","zhang san",21,"male");
    // conn.connect("127.0.0.1",3306,"my","020716","chat");
    // conn.update(sql);

    // clock_t begin=clock();
    // ConnectionPool *pool=ConnectionPool::getConnectionpool();
    // for(int i=0;i<1000;++i){
    //     /*Connection conn;
    //     char sql[1024] = {0};
    //     sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 21, "male");
    //     conn.connect("127.0.0.1", 3306, "my", "020716", "chat");
    //     conn.update(sql);*/
    //     char sql[1024] = {0};
    //     sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 21, "male");
    //     auto sp=pool->getConnection();
    //     sp->update(sql);
    // }
    // clock_t end=clock();
    // std::cout<<end-begin<<std::endl;
    clock_t begin=clock();
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 21, "male");
    ConnectionPool*pool=ConnectionPool::getConnectionpool();
    for(int i=0;i<1000;++i){
        auto sp=pool->getConnection();
        sp->update(sql);
    }
    clock_t end=clock();
    std::cout<<end-begin<<std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(15));
    return 0;
}