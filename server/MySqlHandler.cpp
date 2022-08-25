#include "../server/MySqlHandler.h"
#include <cassert>

MysqlPool *MysqlPool::mysqlPool = nullptr;
std::mutex MysqlPool::pool_mutex_;

MysqlPool::MysqlPool() : size_(0)
{

    // readfromfile();从配置文件中初始化数据库的一些参数

    host = "127.0.0.1";
    user = "root";
    passwd = "duoguangpu123";
    database = "user_service";
    port = 3306;
    capacity_ = 8;

    while (size_ < capacity_)
    {
        CMysql *mysql = new CMysql(host, user, passwd, database, port);

        assert(mysql != nullptr);

        if (!mysql->isValid())
        {
            LOG(INFO) << "Failed To connected to the mysql server";
            break;
        }
        mysql_conn_que.push(mysql);
        ++size_;
    }
}

CMysql *MysqlPool::pop()
{
    std::unique_lock<std::mutex> lock_(mysql_que_mutex_);

    while (size_ == 0)
    {
        mysql_que_condition_.wait(lock_);
    }
    CMysql *mysql = mysql_conn_que.front();
    mysql_conn_que.pop();
    --size_;
    return mysql;
}

void MysqlPool::push(CMysql *mysql)
{
    std::unique_lock<std::mutex> lock_(mysql_que_mutex_);
    mysql_conn_que.push(mysql);
    ++size_;
    mysql_que_condition_.notify_one();
}
