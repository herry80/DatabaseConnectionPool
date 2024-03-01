#include"Connection.h"
// 初始化数据库操作
Connection::Connection(){
    conn_=mysql_init(nullptr);
}
// 释放数据库资源
Connection::~Connection(){
    if(conn_){
        mysql_close(conn_);
    }
}
// 连接数据库
bool Connection::connect(std::string ip, unsigned short port, std::string user, std::string password,
             std::string dbname)
{
    MYSQL* p=mysql_real_connect(conn_,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    return p!=nullptr;
}
// 更新操作
bool Connection::update(std::string sql){
    if(mysql_query(conn_,sql.c_str())){
        LOG("更新失败："+sql);
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES * Connection::query(std::string sql){
    if(mysql_query(conn_,sql.c_str())){
        LOG("更新失败："+sql);
        return nullptr;
    }
    return mysql_use_result(conn_);
}