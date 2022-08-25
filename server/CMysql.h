
/************************************************************************/
/*                        数据库操作类头文件                            */
/************************************************************************/

#ifndef _CMYSQL_H
#define _CMYSQL_H
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <mysql/mysql.h>
#include <string.h>

class CMysql
{
public:
    CMysql()
        : resptr_(nullptr), conn_(nullptr) {}
    //构造函数
    CMysql(const std::string &host, const std::string &username,
           const std::string &passwd, const std::string &database,
           unsigned int port);
    //析构函数
    ~CMysql();
    //插入数据
    // query:执行语句
    //返回值：成功返回0，失败返回-1
    int MysqlInsertWrap(const std::string &query);

    //插入数据
    // query:执行语句
    //返回值：成功返回0，失败返回-1
    int MysqlRealQueryWrap(const char *query, int nLen);

    //删除数据
    // query:执行语句
    //返回值：成功返回0，失败返回-1
    int MysqlDeleteWrap(const std::string &query);

    //更改数据
    // query:执行语句
    //返回值：成功返回0，失败返回-1
    int MysqlUpdateWrap(const std::string &query);

    //查询数据
    // query:执行语句
    //返回值：成功返回0，失败返回-1
    int MysqlQueryWrap(const std::string &query);

    //释放已查询的数据
    void MysqlFreeWrap();

    // 数据库连接函数
    // host:主机名或IP地址
    // username:用户名
    // passwd:密码
    // database:数据库名
    // port:端口号
    int MysqlConnectWrap(const std::string &host, const std::string &username,
                         const std::string &passwd, const std::string &database,
                         unsigned int port);

    bool isValid()
    {
        return conn_ != nullptr ? true : false;
    }

    //中间类，用于支持下标访问
    class Proxy
    {
    public:
        //构造函数
        Proxy(CMysql *base) : base_(base) {}
        //通过下标找到指定列
        // idx:列下标
        //返回值：指定列的值
        std::string operator[](unsigned int idx) const;

        //通过关键字找到指定列
        // key:列名称关键字
        //返回值：指定列的值
        std::string operator[](const std::string &key) const;

    private:
        //用于访问CMysql类的对象
        CMysql *base_;
    };

    //通过下标找到指定行
    // idx:列下标
    const Proxy operator[](unsigned int idx);

    // 计算数据对象的行数
    unsigned int MysqlNumRowsWrap();

    // 计算数据对象的列数
    unsigned int MysqlFieldCountWrap();

    //通过关键字,在某张表里查找对应关键字所在行下标
    unsigned int MysqlFindMethod(const std::string &key, const std::string &table, const std::string &keyValue);

    //向目标字符串中插入数据
    unsigned int MysqlRealEscapeString(char *dst, char *pData, unsigned int nLen);

private:
    // 数据库连接对象
    MYSQL *conn_;
    // 数据对象
    MYSQL_RES *resptr_;
    // 数据行对象
    MYSQL_ROW sqlrow_;
    // 数据列名称
    std::vector<std::string> colname_;
};

#endif
