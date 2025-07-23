#ifndef __THREAD_LOCK_MANAGER_H__
#define __THREAD_LOCK_MANAGER_H__

#include <vector>

//
#include "DBOperatorManager.h"
#include "CacheInterfaceManager.h"

//
using namespace std;
using namespace tars;

/**
*  锁池，单例
*  根据key % lockSize 取余得到对应的锁对象
*  本server里为了解决同一用户数据并发写覆盖不一致的问题
**/
class ThreadLockManager
{
private:
    ThreadLockManager() {};

public:
    virtual ~ThreadLockManager() {};

public:
    //初始化
    int initialize(int num);
    //初始化锁池
    int initializeLockPool();
    //初始化DB管理对象
    int initDBOperatorManager();
    //初始化cache管理对象
    int initCacheInterfaceManager();

public:
    //singleton
    static ThreadLockManager *ptr();

public:
    //获取对象
    void getUserLock(unsigned long key, TC_ThreadLock *&userLock, CDBOperatorManagerPtr &dbOperator);
    //获取锁对象
    void getUserLock(unsigned long key, TC_ThreadLock *&userLock, CCacheInterfaceManagerPtr &cacheInterface);

private:
    //类和对象唯一实例
    static ThreadLockManager *m_instance;

private:
    //锁数量
    int m_lockNum;
    //锁池
    vector<TC_ThreadLock *> m_lockPool;
    //数据库连接管理对象
    vector<CDBOperatorManagerPtr> m_vecDBOperatorManager;
    //缓存连接管理对象
    vector<CCacheInterfaceManagerPtr> m_vecCacheInterfaceManager;
};

#endif


