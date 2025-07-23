#include <sstream>
#include <util/tc_common.h>
#include "DBOperator.h"
#include "globe.h"
#include "LogComm.h"
#include "DBAgentServer.h"

//
using namespace wbl;

//一次拉取最大记录数
#define MAX_RECORDS_COUNT   100

/**
 *
*/
CDBOperator::CDBOperator()
{

}

/**
 *
*/
CDBOperator::~CDBOperator()
{

}

//初始化
int CDBOperator::init(const string &dbhost, const string &dbuser, const string &dbpass, const string &dbname, const string &charset, const string &dbport)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        map<string, string> mpParam;
        mpParam["dbhost"]  = dbhost;
        mpParam["dbuser"]  = dbuser;
        mpParam["dbpass"]  = dbpass;
        mpParam["dbname"]  = dbname;
        mpParam["charset"] = charset;
        mpParam["dbport"]  = dbport;

        TC_DBConf dbConf;
        dbConf.loadFromMap(mpParam);

        //保存db配置
        m_dbConf = dbConf;

        //初始化数据库连接
        m_mysqlObj.init(dbConf);
    }
    catch (exception &e)
    {
        iRet = -1;
        ROLLLOG_ERROR << "Catch exception: " << e.what() << endl;
    }
    catch (...)
    {
        iRet = -2;
        ROLLLOG_ERROR << "Catch unknown exception." << endl;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}


//初始化
int CDBOperator::init(const TC_DBConf &dbConf)
{
    FUNC_ENTRY("");

    int iRet = 0;

    try
    {
        m_dbConf = dbConf;
        m_mysqlObj.init(dbConf);
    }
    catch (exception &e)
    {
        iRet = -1;
        ROLLLOG_ERROR << "Catch exception: " << e.what() << endl;
    }
    catch (...)
    {
        iRet = -2;
        ROLLLOG_ERROR << "Catch unknown exception." << endl;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//查询
int CDBOperator::Select(const dbagent::TDBReadReq &req, dbagent::TDBReadRsp &rsp)
{
    FUNC_ENTRY("");

    int iRet = 0;

    try
    {
        string strSQL = "select";
        for (auto it = req.fields.begin(); it != req.fields.end(); it++)
        {
            if (it ==  req.fields.begin())
                strSQL += " " + it->colName;
            else
                strSQL += ", " + it->colName;
        }

        //表名
        strSQL += " from " + req.tableName;
        //分表
        string indexStr;
        if (g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr) == 0)
        {
            strSQL += indexStr;
        }

        //conditon
        strSQL += " where ";
        for (auto itConditionGroup = req.conditions.begin(); itConditionGroup != req.conditions.end(); ++itConditionGroup)
        {
            if (itConditionGroup != req.conditions.begin())
            {
                strSQL += getRelationStr(req.groupRelation);
            }

            strSQL += "(";

            for (auto itCondition = itConditionGroup->condition.begin(); itCondition != itConditionGroup->condition.end(); ++itCondition)
            {
                if (itCondition != itConditionGroup->condition.begin())
                {
                    strSQL += getRelationStr(itConditionGroup->relation);
                }

                strSQL += itCondition->colName;
                strSQL += getConditionStr(itCondition->condtion);
                strSQL += getValueStr(itCondition->colType, itCondition->colValues);
            }

            strSQL += ")";
        }

        //GROUP BY
        for (auto itGroupby = req.groupbyCol.begin(); itGroupby != req.groupbyCol.end(); ++itGroupby)
        {
            if (itGroupby == req.groupbyCol.begin())
            {
                strSQL += " GROUP BY ";
                strSQL += *itGroupby + " ";
            }
            else
            {
                strSQL += ", " + *itGroupby + " ";
            }
        }

        //ORDER BY
        for (auto itOrderby = req.orderbyCol.begin(); itOrderby != req.orderbyCol.end(); ++itOrderby)
        {
            if (itOrderby == req.orderbyCol.begin())
            {
                strSQL += " ORDER BY ";
                strSQL += itOrderby->colName + " " + getSortStr(itOrderby->sort);
            }
            else
            {
                strSQL += ", " + itOrderby->colName + " " + getSortStr(itOrderby->sort);
            }
        }

#if 0
        //limit from to
        if (req.limit > 0)
        {
            if ((req.limit_from >= 0) && (req.limit >= req.limit_from))
            {
                strSQL += " limit " + I2S(req.limit_from);
                strSQL += ", " + I2S(req.limit);
            }
            else
            {
                strSQL += " limit " + I2S(req.limit);
            }
        }
#endif

        //limit from to
        if (req.limit > 0)
        {
            if ((req.limit_from > 0) /*&& (req.limit >= req.limit_from)*/)
            {
                strSQL += " limit " + I2S(req.limit_from);
                strSQL += ", " + I2S(req.limit);
            }
            else
            {
                strSQL += " limit " + I2S(req.limit);
            }
        }
        else if (req.limit == MAX_INDEX_END)
        {
            if ((req.limit_from >= 0) && (MAX_LIST_COUNT >= req.limit_from))
            {
                strSQL += " limit " + I2S(req.limit_from);
                strSQL += ", " + I2S(MAX_LIST_COUNT);
            }
            else
            {
                strSQL += " limit " + I2S(MAX_LIST_COUNT);
            }
        }

        //构建的sql
        ROLLLOG_DEBUG << "Build SQL: " << strSQL << endl;

        //查询数据
        int64_t ms = TNOWMS;
        TC_Mysql::MysqlData res = m_mysqlObj.queryRecord(strSQL);
        ROLLLOG_DEBUG << "Execute SQL: [" << strSQL << "], return " << res.size() << " records." << ", costTime: " << TNOWMS - ms << endl;

        //无数据
        if (res.size() <= 0)
        {
            ROLLLOG_DEBUG << "table " + req.tableName + " no data." << endl;
            return 0;
        }

        //数据集太大
        if (res.size() > MAX_RECORDS_COUNT)
        {
            ROLLLOG_WARN << "table " + req.tableName + " records size too large. size: " << res.size() << endl;
        }

        //数据集
        for (size_t i = 0; i < res.size(); ++i)
        {
            ostringstream os;
            TField tField;
            vector<TField> vecField;
            for (auto it = req.fields.begin(); it != req.fields.end(); it++)
            {
                tField.colName = it->colName;
                tField.colType = it->colType;
                tField.colValue = res[i][it->colName];
                vecField.push_back(tField);

                if (it ==  req.fields.begin())
                    os << it->colName << ": ";
                else
                    os << ", " << it->colName << ": ";

                os << res[i][it->colName];
            }

            rsp.records.push_back(vecField);
            ROLLLOG_DEBUG << os.str() << endl;
        }
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "select operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "select operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//取记录数
int CDBOperator::Count(const dbagent::TDBReadReq &req, dbagent::TDBReadRsp &rsp)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        //conditon
        string sCondition = " where ";
        for (auto itConditionGroup = req.conditions.begin(); itConditionGroup != req.conditions.end(); ++itConditionGroup)
        {
            if (itConditionGroup != req.conditions.begin())
            {
                sCondition += getRelationStr(req.groupRelation);
            }

            sCondition += "(";

            for (auto itCondition = itConditionGroup->condition.begin(); itCondition != itConditionGroup->condition.end(); ++itCondition)
            {
                if (itCondition != itConditionGroup->condition.begin())
                {
                    sCondition += getRelationStr(itConditionGroup->relation);
                }

                sCondition += itCondition->colName;
                sCondition += getConditionStr(itCondition->condtion);
                sCondition += getValueStr(itCondition->colType, itCondition->colValues);
            }

            sCondition += ")";
        }

        //表名
        string tableName = req.tableName;
        //分表
        string indexStr;
        int ret = g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr);
        if (ret == 0)
        {
            tableName += indexStr;
        }

        //获取Table查询结果的数目
        size_t count = m_mysqlObj.getRecordCount(tableName, sCondition);
        ROLLLOG_DEBUG << "get record count: " << count << endl;
        rsp.totalcount = count;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "get record count catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "get record count catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//插入记录
int CDBOperator::Insert(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");
    int iRet = 0;

    ROLLLOG_DEBUG<< "req: "<< printTars(req) << endl;

    try
    {
        //example
        map<string, pair<TC_Mysql::FT, string> > mpColumns;

        //各字段
        for (auto it = req.fields.begin(); it != req.fields.end(); it++)
        {
            pair<TC_Mysql::FT, string> field;
            switch (it->colType)
            {
            case STRING:
                field = make_pair(TC_Mysql::DB_STR, it->colValue);
                break;
            case INT:
                field = make_pair(TC_Mysql::DB_INT, it->colValue);
                break;
            case BIGINT:
                field = make_pair(TC_Mysql::DB_INT, it->colValue);
                break;
            default:
                break;
            }

            mpColumns[it->colName] = field;

            ROLLLOG_DEBUG<< "col name: "<< it->colName << ", value:"<< it->colValue<< endl;
        }

        //表名
        string tableName = req.tableName;
        //分表
        string indexStr;
        int ret = g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr);
        if (ret == 0)
        {
            tableName += indexStr;
        }

        size_t count = m_mysqlObj.insertRecord(tableName, mpColumns);
        ROLLLOG_DEBUG << "insert record, tableName: " << tableName << ", effect count: " << count << endl;
        rsp.effectCount = count;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "insert operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "insert operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//获取auto_increment最后插入得ID.
long CDBOperator::LastInsertID(const dbagent::TGetLastInsertIDReq &req, dbagent::TGetLastInsertIDRsp &rsp)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        map<string, pair<TC_Mysql::FT, string> > mpColumns;

        //表名
        string tableName = req.tableName;
        //分表
        string indexStr;
        int ret = g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr);
        if (ret == 0)
        {
            tableName += indexStr;
        }

        int64_t ms = TNOWMS;
        size_t count = m_mysqlObj.insertRecord(tableName, mpColumns);
        ROLLLOG_DEBUG << "insert record, effect count: " << count << "costTime: " << TNOWMS - ms << endl;
        rsp.lastID = m_mysqlObj.lastInsertID();
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "last insert id operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "last insert id operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//获取auto_increment最后插入得ID.
long CDBOperator::LastInsertID(const dbagent::TGetTableGUIDReq &req, dbagent::TGetTableGUIDRsp &rsp)
{
    int iRet = 0;
    FUNC_ENTRY("");

    try
    {
        //表名
        string tableName = req.tableName;
        //字段
        string fieldName = req.fieldName;
        //分表
        string indexStr;
        int ret = g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr);
        if (ret == 0)
        {
            tableName += indexStr;
        }

        int64_t ms = TNOWMS;
        string strSQL = "UPDATE `" + tableName + "` SET `" + fieldName + "` = LAST_INSERT_ID(`" + fieldName + "` + 1);";
        m_mysqlObj.execute(strSQL);
        rsp.lastID = m_mysqlObj.lastInsertID();
        ROLLLOG_DEBUG << "Gets the table autoincrement: tableName: " << tableName << ", lastId：" << rsp.lastID << "costTime: " << TNOWMS - ms << endl;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "last insert id operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "last insert id operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//获取绑定手机号对应的用户信息
long CDBOperator::GetBindPhone(const dbagent::TGetBindPhoneInfoReq &req, dbagent::TGetBindPhoneInfoRsp &rsp)
{
    int iRet = 0;
    FUNC_ENTRY("");

    try
    {
        //表名
        string tableName = req.tableName;
        //分表
        string indexStr;
        int ret = g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr);
        if (ret == 0)
        {
            tableName += indexStr;
        }

        //构建的sql
        string strSQL = "SELECT uid FROM `" + tableName + "` WHERE `mobile` = '" + req.mobile + "'";
        ROLLLOG_DEBUG << "Build SQL: " << strSQL << endl;

        int64_t ms = TNOWMS;
        TC_Mysql::MysqlData res = m_mysqlObj.queryRecord(strSQL);
        ROLLLOG_DEBUG << "Execute SQL: [" << strSQL << "], return " << res.size() << " records." << ", costTime: " << TNOWMS - ms << endl;

        //无数据
        if (res.size() <= 0)
        {
            ROLLLOG_INFO << "table " + req.tableName + " no data." << endl;
            return 0;
        }

        //数据集太大
        if (res.size() != 1)
        {
            ROLLLOG_ERROR << "table " + req.tableName + " records size: " << res.size() << " error!" << endl;
            return -1;
        }

        rsp.lUid = S2L(res[0]["uid"]);

        ROLLLOG_DEBUG << "tableName:" << tableName << ",req.mobile:" << req.mobile << ", rsp.lUid:" << rsp.lUid << ", costTime:" << TNOWMS - ms << endl;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "last insert id operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "last insert id operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//insert ignore插入，若存在则放弃
int CDBOperator::InsertIgnore(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        //example
        //INSERT IGNORE INTO books (name) VALUES ('MySQL Manual')

        ostringstream sColumnNames;
        ostringstream sColumnValues;

        //各字段
        for (auto it = req.fields.begin(); it != req.fields.end(); it++)
        {
            if (it == req.fields.begin())
            {
                sColumnNames << "`" << it->colName << "`";
                sColumnValues << getValueStr(it->colType, it->colValue);
            }
            else
            {
                sColumnNames << ",`" << it->colName << "`";
                sColumnValues << "," + getValueStr(it->colType, it->colValue);
            }
        }

        string tableName = req.tableName;
        string indexStr;
        if (g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr) == 0)
        {
            tableName += indexStr;
        }

        int64_t ms = TNOWMS;
        ostringstream osSql;
        osSql << "insert ignore into " << tableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
        m_mysqlObj.execute(osSql.str());
        rsp.effectCount = m_mysqlObj.getAffectedRows();
        ROLLLOG_DEBUG << "Execute SQL: [" << osSql.str() << "], effect count: " << rsp.effectCount << ", costTime: " << TNOWMS - ms << endl;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "insert ignore operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "insert ignore operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//无者插入，有者更新
int CDBOperator::InsertOnDuplicateKey(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");

    int iRet = 0;

    try
    {
        //example
        //INSERT INTO books (name) VALUES ('MySQL Manual') ON DUPLICATE KEY UPDATE a = VALUES(a)

        ostringstream sColumnNames;
        ostringstream sColumnValues;

        //各字段
        for (auto it = req.fields.begin(); it != req.fields.end(); it++)
        {
            if (it == req.fields.begin())
            {
                sColumnNames << "`" << it->colName << "`";
                sColumnValues << getValueStr(it->colType, it->colValue);
            }
            else
            {
                sColumnNames << ",`" << it->colName << "`";
                sColumnValues << "," + getValueStr(it->colType, it->colValue);
            }
        }

        string tableName = req.tableName;
        string indexStr;
        if (g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr) == 0)
        {
            tableName += indexStr;
        }

        //sql
        ostringstream osSql;
        osSql << "insert into " << tableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";

        //on duplicate key update
        string sCondition = " on duplicate key update ";
        //初始标记
        bool bflag = false;
        for (auto itConditionGroup = req.conditions.begin(); itConditionGroup != req.conditions.end(); ++itConditionGroup)
        {
            for (auto itCondition = itConditionGroup->condition.begin(); itCondition != itConditionGroup->condition.end(); ++itCondition)
            {
                if (itCondition->condtion != E_EQ)
                {
                    continue;
                }

                //初始标记
                if (bflag)
                {
                    sCondition += ", " + itCondition->colName + " = values(" + itCondition->colName + ")";
                }
                else
                {
                    sCondition += itCondition->colName + " = values(" + itCondition->colName + ")";
                }

                //初始标记
                bflag = true;
            }
        }

        //执行sql,变更数据
        int64_t ms = TNOWMS;
        m_mysqlObj.execute(osSql.str() + sCondition);
        rsp.effectCount = m_mysqlObj.getAffectedRows();
        ROLLLOG_DEBUG << "Execute SQL: [" << (osSql.str()  + sCondition) << "], effect count: " << rsp.effectCount << "costTime: " << TNOWMS - ms << endl;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "insert ignore operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "insert ignore operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//变更记录
int CDBOperator::Update(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");

    int iRet = 0;

    try
    {
        //example
        //update tb_record_login_2017 set client_ip="192.168.0.1" where uid=123456;

        map<string, pair<TC_Mysql::FT, string> > mpColumns;

        //各字段
        for (auto it = req.fields.begin();  it != req.fields.end(); it++)
        {
            pair<TC_Mysql::FT, string> field;
            switch (it->colType)
            {
            case STRING:
                field = make_pair(TC_Mysql::DB_STR, it->colValue);
                break;
            case INT:
                field = make_pair(TC_Mysql::DB_INT, it->colValue);
                break;
            case BIGINT:
                field = make_pair(TC_Mysql::DB_INT, it->colValue);
                break;
            default:
                break;
            }

            mpColumns[it->colName] = field;
        }

        //conditon
        string sCondition = " where ";
        for (auto itConditionGroup = req.conditions.begin(); itConditionGroup != req.conditions.end(); ++itConditionGroup)
        {
            if (itConditionGroup != req.conditions.begin())
            {
                sCondition += getRelationStr(req.groupRelation);
            }

            sCondition += "(";

            for (auto itCondition = itConditionGroup->condition.begin(); itCondition != itConditionGroup->condition.end(); ++itCondition)
            {
                if (itCondition != itConditionGroup->condition.begin())
                {
                    sCondition += getRelationStr(itConditionGroup->relation);
                }

                sCondition += itCondition->colName;
                sCondition += getConditionStr(itCondition->condtion);
                sCondition += getValueStr(itCondition->colType, itCondition->colValues);
            }

            sCondition += ")";
        }

        string tableName = req.tableName;
        string indexStr;
        if (g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr) == 0)
        {
            tableName += indexStr;
        }

        int64_t ms = TNOWMS;
        size_t count = m_mysqlObj.updateRecord(tableName, mpColumns, sCondition);
        ROLLLOG_DEBUG << "update record, " << "sCondition: " << sCondition << ", effect count: " << count << ", costTime: " << TNOWMS - ms << endl;

        //影响的行数
        rsp.effectCount = count;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "update operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "update operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//构建sql,变更记录
int CDBOperator::UpdateExt(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        ostringstream os;

        ROLLLOG_DEBUG<< "fields size：" << req.fields.size() << endl;

        //各字段
        for (auto it = req.fields.begin(); it != req.fields.end(); it++)
        {
            if (it == req.fields.begin())
            {
                os << "`" << it->colName << "`";
            }
            else
            {
                os << ",`" << it->colName << "`";
            }

            switch (it->colType)
            {
            case STRING:
                //os << "= '" << m_mysqlObj.escapeString(it->colValue) << "'";
                os << " = " << getValueStr(it->colType, it->colValue);
                break;

            case INT:
            case BIGINT:
                switch (it->colArithType)
                {
                case E_ADD:
                    if (TC_Common::strto<tars::Int64>(it->colValue) <= 0)
                    {
                        return -10;
                    }
                    //os << " = `" << m_mysqlObj.escapeString(it->colName) << "` + " << m_mysqlObj.escapeString(it->colValue);
                    os << " = `" << it->colName << "` + " << getValueStr(it->colType, it->colValue);
                    break;

                case E_SUB:
                    if (TC_Common::strto<tars::Int64>(it->colValue) <= 0)
                    {
                        return -10;
                    }
                    //os << " = `" << m_mysqlObj.escapeString(it->colName) << "` - " << m_mysqlObj.escapeString(it->colValue);
                    os << " = `" << it->colName << "` - " << getValueStr(it->colType, it->colValue);
                    break;

                default:
                    //os << "= " << m_mysqlObj.escapeString(it->colValue);
                    os << " = " << getValueStr(it->colType, it->colValue);
                }
                break;

            default:
                break;
            }
        }


        ROLLLOG_DEBUG<< "conditions size：" << req.conditions.size() << ", tableName: " <<  req.tableName <<  endl;
        //conditon
        string sCondition = " where ";
        for (auto itConditionGroup = req.conditions.begin(); itConditionGroup != req.conditions.end(); ++itConditionGroup)
        {
            if (itConditionGroup != req.conditions.begin())
            {
                sCondition += getRelationStr(req.groupRelation);
            }

            sCondition += "(";

            for (auto itCondition = itConditionGroup->condition.begin(); itCondition != itConditionGroup->condition.end(); ++itCondition)
            {
                if (itCondition != itConditionGroup->condition.begin())
                {
                    sCondition += getRelationStr(itConditionGroup->relation);
                }

                sCondition += itCondition->colName;
                sCondition += getConditionStr(itCondition->condtion);
                sCondition += getValueStr(itCondition->colType, itCondition->colValues);
            }

            sCondition += ")";
        }

        string tableName = req.tableName;
        string indexStr;
        if (g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr) == 0)
        {
            tableName += indexStr;
        }

        int64_t ms = TNOWMS;
        ostringstream osSql;
        osSql << "update " << tableName << " set " << os.str() << " " << sCondition;

        ROLLLOG_DEBUG<< "osSql :" << osSql.str() <<  endl;
        m_mysqlObj.execute(osSql.str());
        rsp.effectCount = m_mysqlObj.getAffectedRows();
        ROLLLOG_DEBUG << "Execute SQL: [" << osSql.str() << "], effect count: " << rsp.effectCount << ", costTime:" << TNOWMS - ms << endl;

        //size_t count = m_mysqlObj.updateRecord(tableName, mpColumns, sCondition);
        //ROLLLOG_DEBUG << "update record, " << "sCondition: " << sCondition << ", effect count: " << count << endl;
        //rsp.effectCount = count;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "update operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "update operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//替换记录
int CDBOperator::Replace(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");

    int iRet = 0;

    try
    {
        map<string, pair<TC_Mysql::FT, string> > mpColumns;

        //各字段
        for (auto it = req.fields.begin(); it != req.fields.end(); it++)
        {
            pair<TC_Mysql::FT, string> field;
            switch (it->colType)
            {
            case STRING:
                field = make_pair(TC_Mysql::DB_STR, it->colValue);
                break;
            case INT:
                field = make_pair(TC_Mysql::DB_INT, it->colValue);
                break;
            case BIGINT:
                field = make_pair(TC_Mysql::DB_INT, it->colValue);
                break;
            default:
                break;
            }

            //加入
            mpColumns[it->colName] = field;
        }

        int64_t ms = TNOWMS;
        string tableName = req.tableName;
        string indexStr;
        if (g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr) == 0)
        {
            tableName += indexStr;
        }

        size_t count = m_mysqlObj.replaceRecord(tableName, mpColumns);
        ROLLLOG_DEBUG << "replace record, effect count: " << count << ", costTime: " << TNOWMS - ms << endl;
        rsp.effectCount = count;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "replace operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "replace operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//删除记录
int CDBOperator::Delete(const dbagent::TDBWriteReq &req, dbagent::TDBWriteRsp &rsp)
{
    FUNC_ENTRY("");

    int iRet = 0;

    try
    {
        //conditon
        string sCondition = " where ";
        for (auto itConditionGroup = req.conditions.begin(); itConditionGroup != req.conditions.end(); ++itConditionGroup)
        {
            if (itConditionGroup != req.conditions.begin())
            {
                sCondition += getRelationStr(req.groupRelation);
            }

            sCondition += "(";

            for (auto itCondition = itConditionGroup->condition.begin(); itCondition != itConditionGroup->condition.end(); ++itCondition)
            {
                if (itCondition != itConditionGroup->condition.begin())
                {
                    sCondition += getRelationStr(itConditionGroup->relation);
                }

                sCondition += itCondition->colName;
                sCondition += getConditionStr(itCondition->condtion);
                sCondition += getValueStr(itCondition->colType, itCondition->colValues);
            }

            sCondition += ")";
        }

        int64_t ms = TNOWMS;
        string tableName = req.tableName;
        string indexStr;
        if (0 == g_app.getOuterFactoryPtr()->getTableIndex(req.keyIndex, req.tableName, indexStr))
        {
            tableName += indexStr;
        }

        size_t count = m_mysqlObj.deleteRecord(tableName, sCondition);
        ROLLLOG_DEBUG << "delete record, effect count: " << count << ", costTime: " << TNOWMS - ms <<  endl;
        rsp.effectCount = count;
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "delete operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "delete operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

///执行sql语句，返回结果记录集，目前主要用于用户id的生成
int CDBOperator::Excute(const dbagent::TExcuteSqlReq &req, dbagent::TExcuteSqlRsp &rsp)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        int64_t ms = TNOWMS;
        ROLLLOG_DEBUG << "sql: " << req.sql << endl;

        if(req.opType == 0)
        {
            TC_Mysql::MysqlData res = m_mysqlObj.queryRecord(req.sql);
            ROLLLOG_DEBUG << "Execute SQL: [" << req.sql << "], return " << res.size() << " records, costTime: " << TNOWMS - ms << endl;

            //无数据
            if (res.size() <= 0)
            {
                ROLLLOG_INFO << "table " + req.tableName + " no data." << endl;
                return 0;
            }

            //数据集太大
            if (res.size() > MAX_RECORDS_COUNT)
            {
                ROLLLOG_WARN << "table " + req.tableName + " records size too large. size: " << res.size() << endl;
            }

            //数据集
            for (size_t i = 0; i < res.size(); ++i)
            {
                ostringstream os;

                TField tField;
                vector<TField> vecField;
                for (auto it = req.fields.begin(); it != req.fields.end(); it++)
                {
                    //应答数据
                    tField.colName = it->colName;
                    tField.colType = it->colType;
                    tField.colValue = res[i][it->colName];
                    vecField.push_back(tField);

                    //field
                    if (it ==  req.fields.begin())
                    {
                        os << it->colName << ": ";
                    }
                    else
                    {
                        os << ", " << it->colName << ": ";
                    }

                    //value
                    os << res[i][it->colName];
                }

                //保存到应答数据
                rsp.records.push_back(vecField);
                //一条记录
                ROLLLOG_DEBUG << os.str() << endl;
            }
        }
        else
        {
            m_mysqlObj.execute(req.sql);
            rsp.effectCount = m_mysqlObj.getAffectedRows();
        }
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_ERROR << "excute operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_ERROR << "excute operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//排序
const string CDBOperator::getSortStr(Eum_Sort sort)
{
    string retStr = "";

    switch (sort)
    {
    case ASC:
        retStr = "ASC";
        break;
    case DESC:
        retStr = "DESC";
        break;
    default:
        break;
    }

    return retStr;
}

//条件关系
const string CDBOperator::getRelationStr(enum Eum_Relation relation)
{
    string retStr = "";
    switch (relation)
    {
    case AND:
        retStr = " AND ";
        break;
    case OR:
        retStr = " OR ";
        break;
    default:
        break;
    }

    return retStr;
}

//条件转换
const string CDBOperator::getConditionStr(Eum_Condition condition)
{
    string retStr = "";

    switch (condition)
    {
    case E_EQ:
        retStr = " = ";
        break;
    case E_NE:
        retStr = " != ";
        break;
    case E_GT:
        retStr = " > ";
        break;
    case E_LT:
        retStr = " < ";
        break;
    case E_GE:
        retStr = " >= ";
        break;
    case E_LE:
        retStr = " <= ";
        break;
    case E_LIKE:
        retStr = " like ";
        break;
    case E_IN:
        retStr = " in ";
        break;
    default:
        break;
    }

    return retStr;
}

//值
const string CDBOperator::getValueStr(Eum_Col_Type colType, const string &colValue)
{
    string retStr = "";

    switch (colType)
    {
    case dbagent::STRING:
        retStr = "\'" + m_mysqlObj.escapeString(colValue) + "\'";
        break;
    case dbagent::INT:
        retStr = m_mysqlObj.escapeString(colValue);
        break;
    case dbagent::BIGINT:
        retStr = m_mysqlObj.escapeString(colValue);
        break;
    default:
        break;
    }

    return retStr;
}

