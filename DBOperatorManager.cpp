#include <util/tc_common.h>
#include "globe.h"
#include "LogComm.h"
#include "DBAgentServer.h"
#include "DBDefine.h"
#include "DBOperatorManager.h"

//
using namespace wbl;

/**
 *
*/
CDBOperatorManager::CDBOperatorManager()
{

}

/**
 *
*/
CDBOperatorManager::~CDBOperatorManager()
{

}

//初始化
int CDBOperatorManager::init()
{
    int iRet = 0;
    FUNC_ENTRY("");

    //初始化db操作对象
    iRet = initDBOperator();

    FUNC_EXIT("", iRet);
    return iRet;
}

int CDBOperatorManager::init(Eum_Db_Read_Write_Type type)
{
    int iRet = 0;
    FUNC_ENTRY("");

    //初始化db操作对象
    iRet = initDBOperator(type);

    FUNC_EXIT("", iRet);
    return iRet;
}

//初始化db操作对象
int CDBOperatorManager::initDBOperator()
{
    int iRet = 0;

    auto &dbGroup  = g_app.getOuterFactoryPtr()->getDBGroupConfig();
    for (auto it = dbGroup.mapData.begin(); it != dbGroup.mapData.end(); it++)
    {
        //主库
        auto ptrMaster = new CDBOperator();
        if (!ptrMaster)
        {
            return -1;
        }

        ptrMaster->init(it->second.szMasterHost, it->second.user, it->second.password, it->second.dbname, it->second.charset, it->second.port);
        ROLLLOG_DEBUG << "master, Host: " << it->second.szMasterHost
                      << ", domain: " << it->second.szMasterDomain
                      << ", User: " << it->second.user
                      << ", password: " << it->second.password
                      << ", dbname: " << it->second.dbname
                      << ", charset: " << it->second.charset
                      << ", port: " << it->second.port << endl;

        //从库
        auto ptrSlave = new CDBOperator();
        if (!ptrSlave)
        {
            return -2;
        }

        ptrSlave->init(it->second.szSlaveHost, it->second.user, it->second.password, it->second.dbname, it->second.charset, it->second.port);
        ROLLLOG_DEBUG << "slave, Host: " << it->second.szSlaveHost
                      << ", domain: " << it->second.szSlaveDomain
                      << ", User: " << it->second.user
                      << ", password: " << it->second.password
                      << ", dbname: " << it->second.dbname
                      << ", charset: " << it->second.charset
                      << ", port: " << it->second.port << endl;

        vector<CDBOperatorPtr> vecDBOperator;
        vecDBOperator.push_back(ptrMaster);
        vecDBOperator.push_back(ptrSlave);
        mapDBOperator[it->first] = vecDBOperator;
    }

    return iRet;
}

//初始化db操作对象
int CDBOperatorManager::initDBOperator(Eum_Db_Read_Write_Type type)
{
    int iRet = 0;

    auto &dbGroup  = g_app.getOuterFactoryPtr()->getDBGroupConfig();
    for (auto it = dbGroup.mapData.begin(); it != dbGroup.mapData.end(); it++)
    {
        CDBOperatorPtr ptrMaster = NULL;
        CDBOperatorPtr ptrSlave = NULL;
        map<int, CDBOperatorPtr> dbOperator;

        switch(type)
        {
        case E_DB_READ_TYPE:
            //从库
            ptrSlave = new CDBOperator();
            if (!ptrSlave)
            {
                return -1;
            }

            ptrSlave->init(it->second.szSlaveHost, it->second.user, it->second.password, it->second.dbname, it->second.charset, it->second.port);
            ROLLLOG_DEBUG << "slave, Host: " << it->second.szSlaveHost
                          << ", User: " << it->second.user
                          << ", password: " << it->second.password
                          << ", dbname: " << it->second.dbname
                          << ", charset: " << it->second.charset
                          << ", port: " << it->second.port << endl;
            dbOperator[E_DB_READ_TYPE] = ptrSlave;
            break;

        case E_DB_WRITE_TYPE:
            //主库
            ptrMaster = new CDBOperator();
            if (!ptrMaster)
            {
                return -2;
            }

            ptrMaster->init(it->second.szMasterHost, it->second.user, it->second.password, it->second.dbname, it->second.charset, it->second.port);
            ROLLLOG_DEBUG << "master, Host: " << it->second.szMasterHost
                          << ", User: " << it->second.user
                          << ", password: " << it->second.password
                          << ", dbname: " << it->second.dbname
                          << ", charset: " << it->second.charset
                          << ", port: " << it->second.port << endl;
            dbOperator[E_DB_WRITE_TYPE] = ptrMaster;
            break;

        case E_DB_READ_WRITE_ALL:
            //主库
            ptrMaster = new CDBOperator();
            if (!ptrMaster)
            {
                return -3;
            }

            ptrMaster->init(it->second.szMasterHost, it->second.user, it->second.password, it->second.dbname, it->second.charset, it->second.port);
            ROLLLOG_DEBUG << "master, Host: " << it->second.szMasterHost << ", User: " << it->second.user
                          << ", password: " << it->second.password << ", dbname: " << it->second.dbname
                          << ", charset: " << it->second.charset
                          << ", port: " << it->second.port << endl;
            dbOperator[E_DB_WRITE_TYPE] = ptrMaster;

            ptrSlave = new CDBOperator();
            if (!ptrSlave)
            {
                return -4;
            }

            ptrSlave->init(it->second.szSlaveHost, it->second.user, it->second.password, it->second.dbname, it->second.charset, it->second.port);
            ROLLLOG_DEBUG << "slave, Host: " << it->second.szSlaveHost
                          << ", User: " << it->second.user
                          << ", password: " << it->second.password
                          << ", dbname: " << it->second.dbname
                          << ", charset: " << it->second.charset
                          << ", port: " << it->second.port << endl;

            dbOperator[E_DB_READ_TYPE] = ptrSlave;
            break;

        default:
            return -5;
            break;
        }

        mapDBReadWrite[it->first] = dbOperator;
    }

    return iRet;
}

//取db操作对象
const CDBOperatorPtr CDBOperatorManager::getDBOperatorByKey(const string key, Eum_Db_Type type)
{
    auto it = mapDBOperator.find(key);

    //找不到key对应的连接
    if (it == mapDBOperator.end())
    {
        return NULL;
    }

    //不是主备
    if (it->second.size() != E_DB_TYPE_COUNT)
    {
        return NULL;
    }

    //返回对象
    return it->second[type];
}

//取db操作对象
const CDBOperatorPtr CDBOperatorManager::getReadWriteDBOprator(const string key, Eum_Db_Read_Write_Type type)
{
    auto it = mapDBReadWrite.find(key);
    if (it == mapDBReadWrite.end())
    {
        return NULL;
    }

    auto itReadWrite = it->second.find(type);
    if (itReadWrite == it->second.end())
    {
        return NULL;
    }

    return itReadWrite->second;
}

