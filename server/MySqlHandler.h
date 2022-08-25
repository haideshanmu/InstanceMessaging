#ifndef MYSQL_HANDLER_H
#define MYSQL_HANDLER_H

#include "../server/CMysql.h"
#include <boost/noncopyable.hpp>
#include <mutex>
#include <queue>
#include <butil/logging.h>
#include <condition_variable>

class MysqlPool : public boost::noncopyable
{
public:
    static MysqlPool *GetInstance()
    {
        if (mysqlPool == nullptr)
        {
            std::lock_guard<std::mutex> lock_(pool_mutex_);
            if (mysqlPool == nullptr)
            {
                mysqlPool = new MysqlPool;
            }
        }
        return mysqlPool;
    }

    CMysql *pop();

    void push(CMysql *mysql);

private:
    MysqlPool();

private:
    static MysqlPool *mysqlPool;
    static std::mutex pool_mutex_;

    std::queue<CMysql *> mysql_conn_que;
    std::mutex mysql_que_mutex_;
    std::condition_variable mysql_que_condition_;

    int capacity_;
    int size_;

    std::string host;
    std::string user;
    std::string passwd;
    std::string database;
    int port;
};

class MysqlHanlder
{
public:
    MysqlHanlder()
    {
        mysql = MysqlPool::GetInstance()->pop();
    }

    ~MysqlHanlder()
    {
        MysqlPool::GetInstance()->push(mysql);
        mysql = nullptr;
    }

    CMysql *get()
    {
        return mysql;
    }

private:
    CMysql *mysql;
};

#endif