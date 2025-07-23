#include "ThreadLockManager.h"
#include "globe.h"
#include "LogComm.h"

ThreadLockManager *ThreadLockManager::m_instance = NULL;

//初始化
int ThreadLockManager::initialize(int num)
{
    if (num < 1)
    {
        LOG_ERROR << "lock size is less than 1, set it to be 1, num: " << num << endl;
        m_lockNum = 1;
    }
    else
    {
        m_lockNum = num;
    }

    if (m_lockNum < 1)
    {
        m_lockNum = 1;
    }

    int iRet = initializeLockPool();
    if (iRet != 0)
    {
        ROLLLOG_ERROR << "initialize lock pool failed." << endl;
        return -1;
    }

    //
    iRet = initCacheInterfaceManager();
    if (iRet != 0)
    {
        ROLLLOG_ERROR << "init cache interface manager failed." << endl;
        return -2;
    }

    //初始化db管理对象
    iRet = initDBOperatorManager();
    if (iRet != 0)
    {
        ROLLLOG_ERROR << "init db operator manager failed." << endl;
        return -2;
    }

    //锁池
    if (m_lockPool.size() < 1)
    {
        terminate();
        return -3;
    }

    //REDIS连接管理对象
    if (m_vecCacheInterfaceManager.size() < 1)
    {
        terminate();
        return -4;
    }

    if (m_lockPool.size() != m_vecCacheInterfaceManager.size())
    {
        terminate();
        return -5;
    }

    //MYSQL连接管理对象
    if (m_vecDBOperatorManager.size() < 1)
    {
        terminate();
        return -6;
    }

    if (m_lockPool.size() != m_vecDBOperatorManager.size())
    {
        terminate();
        return -7;
    }

    return 0;
}

//初始化锁池
int ThreadLockManager::initializeLockPool()
{
    m_lockPool.clear();
    for (int i = 0; i < m_lockNum; i ++)
    {
        auto newLock = new TC_ThreadLock();
        if (!newLock)
        {
            ROLLLOG_ERROR << "allocate a thread lock obj failed, exit server." << endl;
            terminate();
            return -1;
        }

        m_lockPool.push_back(newLock);
    }

    return 0;
}

//初始化redis管理对象
int ThreadLockManager::initCacheInterfaceManager()
{
    m_vecCacheInterfaceManager.clear();
    for (int i = 0; i < m_lockNum; i ++)
    {
        auto newCacheInterface = new CCacheInterfaceManager();
        if (!newCacheInterface)
        {
            ROLLLOG_ERROR << "allocate cache interface manager obj failed, exit server." << endl;
            //terminate();
            return -1;
        }

        if (newCacheInterface->init(E_CACHE_WRITE_TYPE) < 0)
        {
            ROLLLOG_ERROR << "Init cache interface manager failed, exit server." << endl;
            //terminate();
            return -2;
        }

        m_vecCacheInterfaceManager.push_back(newCacheInterface);
    }

    return 0;
}

//初始化mysql管理对象
int ThreadLockManager::initDBOperatorManager()
{
    m_vecDBOperatorManager.clear();
    for (int i = 0; i < m_lockNum; i ++)
    {
        auto newDBOperator = new CDBOperatorManager();
        if (!newDBOperator)
        {
            ROLLLOG_ERROR << "allocate DBOperatorManager obj failed, exit server." << endl;
            terminate();
            return -1;
        }

        if (newDBOperator->init(E_DB_WRITE_TYPE) < 0)
        {
            ROLLLOG_ERROR << "Init DBOperatorManager failed, exit server." << endl;
            terminate();
            return -2;
        }

        m_vecDBOperatorManager.push_back(newDBOperator);
    }

    return 0;
}

//实例化对象
ThreadLockManager *ThreadLockManager::ptr()
{
    if (!m_instance)
    {
        m_instance = new ThreadLockManager();
    }

    return m_instance;
}

//获取对象
void ThreadLockManager::getUserLock(unsigned long key, TC_ThreadLock *&userLock, CCacheInterfaceManagerPtr &cacheInterface)
{
    unsigned lockSize = m_lockPool.size();
    userLock = m_lockPool[key % lockSize];
    cacheInterface = m_vecCacheInterfaceManager[key % lockSize];
    return;
}

//获取对象
void ThreadLockManager::getUserLock(unsigned long key, TC_ThreadLock *&userLock, CDBOperatorManagerPtr &dbOperator)
{
    unsigned lockSize = m_lockPool.size();
    //取lock
    userLock = m_lockPool[key % lockSize];
    //取cache
    dbOperator = m_vecDBOperatorManager[key % lockSize];
    return;
}


