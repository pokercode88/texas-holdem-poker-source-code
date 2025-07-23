#ifndef _DB_OPERATOR_H_
#define _DB_OPERATOR_H_

#include <string>
#include <map>

#include <util/tc_config.h>
#include <util/tc_mysql.h>
#include <util/tc_singleton.h>
#include <util/tc_autoptr.h>
#include <wbl/pthread_util.h>

//协议
#include "DBAgentProto.h"

//
using namespace std;
using namespace tars;
using namespace dbagent;

/**
*
* DB操作类，增、删、改、查
*/
class CDBOperator : public TC_HandleBase
{
public:
    /**
     *
    */
    CDBOperator();

    /**
     *
    */
    ~CDBOperator();

public:
    //
    int init(const TC_DBConf &dbConf);
    //
    int init(const string &dbhost, const string &dbuser, const string &dbpass, const string &dbname, const string &charset, const string &dbport);

public:
    //查询
    int Select(const dbagent::TDBReadReq &req, dbagent::TDBReadRsp &rsp);
    //取记录数
    int Count(const dbagent::TDBReadReq &req, dbagent::TDBReadRsp &rsp);
    //插入记录
    int Insert(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    //获取auto_increment最后插入得ID.
    long LastInsertID(const dbagent::TGetLastInsertIDReq &req, dbagent::TGetLastInsertIDRsp &rsp);
    //获取auto_increment最后插入得ID.
    long LastInsertID(const dbagent::TGetTableGUIDReq &req, dbagent::TGetTableGUIDRsp &rsp);
    //获取绑定手机号对应的用户信息
    long GetBindPhone(const dbagent::TGetBindPhoneInfoReq &req, dbagent::TGetBindPhoneInfoRsp &rsp);
    //insert ignore插入，若存在则放弃
    int InsertIgnore(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    //无者插入，有者更新
    int InsertOnDuplicateKey(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    //变更记录
    int Update(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    //构建sql,变更记录
    int UpdateExt(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    //替换记录
    int Replace(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    //删除记录
    int Delete(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp);
    ///执行sql语句，返回结果记录集，目前主要用于用户id的生成
    int Excute(const dbagent::TExcuteSqlReq &req, dbagent::TExcuteSqlRsp &rsp);

private:
    //排序
    const string getSortStr(Eum_Sort sort);
    //条件关系
    const string getRelationStr(Eum_Relation relation);
    //条件转换
    const string getConditionStr(Eum_Condition condition);
    //值
    const string getValueStr(Eum_Col_Type colType, const string &colValue);

public:
    //取db配置
    const TC_DBConf &getDBConfig()
    {
        return m_dbConf;
    }

private:
    //mysql操作对象
    TC_Mysql m_mysqlObj;

private:
    //db数据源配置
    TC_DBConf m_dbConf;
};

typedef TC_AutoPtr<CDBOperator> CDBOperatorPtr;

#endif


