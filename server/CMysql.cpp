/************************************************************************
 * 数据库操作类															*
 ************************************************************************/
#include "CMysql.h"

using std::string;
using std::vector;

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	CMysql
Description:构造函数
Input:      host:主机名或IP地址   username：用户名   passwd：密码
            database：数据库名   port：端口号
Output:
Return:
**********************************************************/
//////////////////////////////////////////////////////////////////////////
CMysql::CMysql(const string &host, const string &username,
               const string &passwd, const string &database,
               unsigned int port) : conn_(NULL), resptr_(NULL)
{
    ////初始化变量
    // mysql_library_init(0,NULL,NULL);
    //  打开数据库连接
    MysqlConnectWrap(host, username, passwd, database, port);
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	~CMysql
Description:析构函数
Input:
Output:
Return:
**********************************************************/
//////////////////////////////////////////////////////////////////////////
CMysql::~CMysql()
{
    // 关闭数据对象
    if (resptr_ != NULL)
    {
        mysql_free_result(resptr_);
        resptr_ = NULL;
    }

    // 关闭数据库连接
    if (conn_ != NULL)
    {
        mysql_close(conn_);
        conn_ = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlConnectWrap
Description:数据库连接函数，如果存在此数据库就连接，不存在就会新建一个数据库
Input:      host:主机名或IP地址   username:用户名    passwd:密码
            database:数据库名   port:端口号
Output:
Return:     int：0：连接成功  1：初始化数据库连接句柄失败！  -1：连接失败
**********************************************************/
//////////////////////////////////////////////////////////////////////////
int CMysql::MysqlConnectWrap(const string &host, const string &username,
                             const string &passwd, const string &database,
                             unsigned int port)
{

    //初始化变量
    mysql_library_init(0, NULL, NULL);
    // 初始化连接句柄
    conn_ = mysql_init(NULL);
    mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8");

    if (conn_ == NULL)
    {
#ifdef _DEBUG
        CErrorLog errorLog;
        errorLog.Write(__FILE__, __LINE__, "初始化数据库连接句柄失败！");
#endif
        return 1;
    }

    // 连接数据库
    conn_ = mysql_real_connect(conn_, host.c_str(), username.c_str(),
                               passwd.c_str(), database.c_str(), port, NULL, 0);

    if (conn_ == NULL)
    {
        conn_ = mysql_init(NULL);
        mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8");

        conn_ = mysql_real_connect(conn_, host.c_str(), username.c_str(),
                                   passwd.c_str(), NULL, port, NULL, 0);

        if (conn_ == NULL)
            return -1;

        std::string str_sql = std::string("create database ") + database;
        if (mysql_query(conn_, str_sql.c_str()) == 0)
        {
            mysql_close(conn_);

            conn_ = mysql_init(NULL);
            mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8");

            conn_ = mysql_real_connect(conn_, host.c_str(), username.c_str(),
                                       passwd.c_str(), database.c_str(), port, NULL, 0);
            return 0;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlFreeWrap
Description:释放已查询的数据，可以再次查询，但不释放连接
Input:
Output:
Return:
**********************************************************/
//////////////////////////////////////////////////////////////////////////
void CMysql::MysqlFreeWrap()
{
    // 关闭数据对象
    if (resptr_ != NULL)
    {
        mysql_free_result(resptr_);
        resptr_ = NULL;
    }

    // 清空数据列名称
    colname_.clear();
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlInsertWrap
Description:数据插入函数
Input:      query:执行语句
Output:
Return:     int：0：成功   -1：失败
**********************************************************/
//////////////////////////////////////////////////////////////////////////
int CMysql::MysqlInsertWrap(const std::string &query)
{
    if (isValid() && !query.empty())
    {
        if (mysql_query(conn_, query.c_str()) == 0)
            return 0;
        else
        {
            char sz_errno[16];
            sprintf(sz_errno, "%d", mysql_errno(conn_));

            std::cout << query << std::endl;

            std::cout << "Insert Failed " << mysql_error(conn_) << std::endl
                      << std::flush;
            return -1;
        }
    }
    return -1;
    // return !isValid() || query.empty() || mysql_query(conn_, query.c_str()) ? -1 : 0;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlRealQueryWrap
Description:数据插入函数
Input:      query:执行语句  nLen：数据长度
Output:
Return:     int：0：成功   -1：失败
**********************************************************/
//////////////////////////////////////////////////////////////////////////
int CMysql::MysqlRealQueryWrap(const char *query, int nLen)
{
    if (query)
        return !isValid() || mysql_real_query(conn_, query, nLen) ? -1 : 0;
    else
        return -1;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlUpdateWrap
Description:数据更改函数
Input:      query:执行语句
Output:
Return:     int：0：成功   -1：失败
**********************************************************/
//////////////////////////////////////////////////////////////////////////
int CMysql::MysqlUpdateWrap(const std::string &query)
{
    return !isValid() || query.empty() || mysql_query(conn_, query.c_str()) ? -1 : 0;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlDeleteWrap
Description:数据删除函数
Input:      query:执行语句
Output:
Return:     int：0：成功   -1：失败
**********************************************************/
//////////////////////////////////////////////////////////////////////////
int CMysql::MysqlDeleteWrap(const std::string &query)
{

    if (isValid() && !query.empty())
    {
        if (mysql_query(conn_, query.c_str()) == 0)
            return 0;
        else
        {
            char sz_errno[16];
            sprintf(sz_errno, "%d", mysql_errno(conn_));
            std::cout << "sql delete failed -- error_code: --> " << sz_errno << std::endl
                      << std::flush;
            return -1;
        }
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlQueryWrap
Description:数据查询函数
Input:      query:执行语句
Output:
Return:     int：0：成功   -1：失败
**********************************************************/
//////////////////////////////////////////////////////////////////////////
int CMysql::MysqlQueryWrap(const std::string &query)
{
    if (!isValid())
        return -1;
    // 执行语句为空
    if (query.empty())
    {
        return -1;
    }

    if (mysql_query(conn_, query.c_str()) != 0)
    {
        return -1;
    }

    if ((resptr_ = mysql_store_result(conn_)) == NULL)
    {
        return -1;
    }

    // 将查找到数据的所有列成员保存
    MYSQL_FIELD *fieldPtr = NULL;
    while ((fieldPtr = mysql_fetch_field(resptr_)) != NULL)
    {
        colname_.push_back(fieldPtr->name);
    }

    // 返回查询到的记录数
    return MysqlNumRowsWrap();
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	operator[]
Description:[]重载函数，通过下标找到指定列
Input:      idx:列下标
Output:
Return:     string：指定列的值
**********************************************************/
//////////////////////////////////////////////////////////////////////////
string CMysql::Proxy::operator[](unsigned int idx) const
{

    // 数据对象为空
    if (base_->resptr_ == NULL)
    {
#ifdef _DEBUG
        CErrorLog errorLog;
        errorLog.Write(__FILE__, __LINE__, "读取数据时，数据对象为空");
#endif
        return string("");
    }

    // 判断下标是否超出列范围
    if (idx >= base_->MysqlFieldCountWrap() || idx < 0)
    {
#ifdef _DEBUG
        CErrorLog errorLog;
        errorLog.Write(__FILE__, __LINE__, "读取数据时，下标是否超出列范围");
#endif
        return string("");
    }

    // 获取指定行
    base_->sqlrow_ = mysql_fetch_row(base_->resptr_);
    unsigned long *length = mysql_fetch_lengths(base_->resptr_);
    std::string strData(length[idx], '\0');
    memcpy((void *)strData.c_str(), base_->sqlrow_[idx], length[idx]);
    // 返回指定列
    return strData;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	operator[]
Description:[]重载函数，通过关键字找到指定列
Input:      key:列名称关键字
Output:
Return:     string：指定列的值
**********************************************************/
//////////////////////////////////////////////////////////////////////////
string CMysql::Proxy::operator[](const string &key) const
{

    // 数据对象为空
    if (base_->resptr_ == NULL)
    {
#ifdef _DEBUG
        CErrorLog errorLog;
        errorLog.Write(__FILE__, __LINE__, "读取数据时，数据对象为空");
#endif
        return string("");
    }

    // 查找给定关键字列
    unsigned int idx = 0;
    vector<string>::const_iterator iter;
    for (iter = base_->colname_.begin(); iter != base_->colname_.end(); ++iter)
    {
        if (key == *iter)
        {
            break;
        }
        ++idx;
    }

    // 未找到给定关键字的列
    if (iter == base_->colname_.end())
    {
#ifdef _DEBUG
        CErrorLog errorLog;
        errorLog.Write(__FILE__, __LINE__, "读取数据时，给定关键字不存在");
#endif
        return string("");
    }

    return (*this)[idx];
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	operator[]
Description:[]重载函数，通过下标找到指定行
Input:      idx:行下标
Output:
Return:     Proxy：指定行的对象值
**********************************************************/
//////////////////////////////////////////////////////////////////////////
const CMysql::Proxy CMysql::operator[](unsigned int idx)
{

    // 数据对象为空
    if (resptr_ == NULL)
    {
        return Proxy(this);
    }

    // 判断下标是否超出行范围
    if (idx >= MysqlNumRowsWrap() || idx < 0)
    {
#ifdef _DEBUG
        CErrorLog errorLog;
        errorLog.Write(__FILE__, __LINE__, "读取数据时，下标是否超出行范围");
#endif
        idx = 0;
    }

    // 在数据对象中偏移到指定行
    mysql_data_seek(resptr_, idx);

    return Proxy(this);
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlNumRowsWrap
Description:数据对象的行数计算函数
Input:
Output:
Return:     int：计算出的行数
**********************************************************/
//////////////////////////////////////////////////////////////////////////
unsigned int CMysql::MysqlNumRowsWrap()
{
    // 数据对象不为空
    if (resptr_ != NULL)
    {
        return (unsigned int)mysql_num_rows(resptr_);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlFieldCountWrap
Description:数据对象的列数计算函数
Input:
Output:
Return:     int：计算出的列数
**********************************************************/
//////////////////////////////////////////////////////////////////////////
unsigned int CMysql::MysqlFieldCountWrap()
{
    // 连接对象与数据对象不为空
    if (conn_ != NULL && resptr_ != NULL)
    {
        return mysql_field_count(conn_);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlFindMethod
Description:通过关键字找到指定行的下标
Input:      key:列名称关键字  table:数据库表的名称   keyValue:要查找的对应关键字的值
Output:
Return:     int：指定行的位置
**********************************************************/
//////////////////////////////////////////////////////////////////////////
unsigned int CMysql::MysqlFindMethod(const std::string &key, const std::string &table, const std::string &keyValue)
{
    std::string sql = "select * from " + table; //遍历查询一遍
    int totalRows = 0;
    if (this->MysqlQueryWrap(sql) != -1)
    {
        totalRows = this->MysqlQueryWrap(sql); //通过查询找到的表的所有行数目
    }
    int index = 0;
    while (index < totalRows)
    {
        if ((*this)[index][key] != keyValue)
        {
            ++index;
            continue;
        }
        else
            break;
    }
    return index;
}
//////////////////////////////////////////////////////////////////////////
/*********************************************************
Function:	MysqlRealEscapeString
Description:向目标字符串中插入数据
Input:      dst:插入目标字符串指针   pData:待插入字符串指针  nLen:字符串数据长度
Output:
Return:     int：插入数据长度
**********************************************************/
//////////////////////////////////////////////////////////////////////////
unsigned int CMysql::MysqlRealEscapeString(char *dst, char *pData, unsigned int nLen)
{
    return mysql_real_escape_string(conn_, dst, pData, nLen);
}
