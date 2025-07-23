#ifndef _OUTER_FACTORY_IMP_H_
#define _OUTER_FACTORY_IMP_H_

#include <string>
#include "globe.h"
#include "OuterFactory.h"
#include "DBAgentServantImp.h"
#include "DBDefine.h"
#include "DataProxyProto.h"
#include "DBAgentProto.h"
#include "servant/Application.h"

//wbl
#include <wbl/regex_util.h>
#include <wbl/stream_util.h>

//
using namespace tars;
using namespace dbagent;
using namespace dataproxy;

/**
* keyname结构
*/
typedef struct _TKeyName
{
    int redis_type;    //cache数据类型
    int busi_type;     //业务类型
    string sbusi_type; //业务类型
    string key;        //主键
} KeyName;

/**
 * 外部工具接口对象工厂
 */
class OuterFactoryImp : public OuterFactory
{
public:
    /**
     *
    */
    OuterFactoryImp();

    /**
     *
    */
    virtual ~OuterFactoryImp();

private:
    /**
     *
    */
    void createAllObject();

    /**
     *
    */
    void deleteAllObject();

public:
    //加载配置
    void load();

public:
    // 获取代理

public:
    //框架中用到的outer接口(不能修改):
    const OuterProxyFactoryPtr &getProxyFactory() const
    {
        return _pProxyFactory;
    }
    //服务配置文件
    const tars::TC_Config &getConfig() const
    {
        return *_pFileConf;
    }

public:
    //读取所有配置
    void readAllConfig();
    //读取MYSQL配置
    void readDBConfig();
    //读取REDIS配置
    void readRedisConfig();

public: //MYSQL
    //读取数据类型配置
    void readDBDataTypeConfig();
    //打印数据类型配置
    void printDBDataTypeConfig();
    //获取数据类型配置
    const DataType &getDBDataTypeConfig();
    //分片因子类型
    void readDBSectionFactorTypeConfig();
    //打印分片因子类型
    void printDBSectionFactorTypeConfig();
    //获取分片因子类型
    const SectionFactorType &getDBSectionFactorTypeConfig();
    //读取分片因子配置
    void readDBSectionFactorConfig();
    //打印分片因子配置
    void printDBSectionFactorConfig();
    //打印分片因子真实配置
    void printDBSectionFactorRealConfig();
    //获取分片因子配置
    const SectionFactor &getDBSectionFactorConfig();
    //读取数据表配置
    void readTableConfig();
    //打印数据表配置
    void printTableConfig();
    //获取数据表配置
    const Table &getTableConfig();
    //获取表格索引
    int getTableIndex(int keyIndex, const string &table_name, string &indexStr);
    //读取数据库映射配置
    void readDBMapConfig();
    //打印数据库映射配置
    void printDBMapConfig();
    //获取数据库映射配置
    const DBMap &getDBMapConfig();
    //读取群组配置
    void readDBClusterConfig();
    //打印群组配置
    void printDBClusterConfig();
    //获取群组配置
    const DBCluster &getDBClusterConfig();
    //取分片配置
    int getDBCluster(int keyIndex, const string &table_name, string &groupKey);
    //读取数据库群组配置
    void readDBGroupConfig();
    //打印数据库群组配置
    void printDBGroupConfig();
    //获取数据库群组配置
    const DBGroup &getDBGroupConfig();

public: //REDIS
    //加载数据类型配置
    void readRedisDataTypeConfig();
    //打印数据类型配置
    void printRedisDataTypeConfig();
    //获取数据类型配置
    const DataType &getRedisDataTypeConfig();
    //加载分片因子类型
    void readRedisSectionFactorTypeConfig();
    //打印分片因子类型
    void printRedisSectionFactorTypeConfig();
    //获取分片因子类型
    const SectionFactorType &getRedisSectionFactorTypeConfig();
    //加载分片因子配置
    void readRedisSectionFactorConfig();
    //打印分片因子配置
    void printRedisSectionFactorConfig();
    //打印分片因子真实配置
    void printRedisSectionFactorRealConfig();
    //获取分片因子配置
    const SectionFactor &getRedisSectionFactorConfig();
    //加载群组配置
    void readRedisClusterConfig();
    //打印群组配置
    void printRedisClusterConfig();
    //获取群组配置
    const RedisCluster &getRedisClusterConfig();
    //获取分片配置
    int getRedisCluster(const TClusterInfo &clusterInfo, string &groupKey);
    //加载群组配置
    void readRedisGroupConfig();
    //打印群组配置
    void printRedisGroupConfig();
    //猎取群组配置
    const RedisGroup &getRedisGroupConfig();
    //加载cache结构配置
    void readRedisDescConfig();
    //打印cache结构配置
    void printRedisDescConfig();
    //获取cache结构配置
    const RedisDesc &getRedisDescConfig();
    //加载数据库表配置
    void readTableDescConfig();
    //打印数据库表配置
    void printTableDescConfig();
    //获取数据库表配置
    const TableDesc &getTableDescConfig();

public:
    //读取锁数量配置
    void readLockNum();
    //打印锁数量配置
    void printLockNum();
    //取锁数量配置
    int getLockNum();

public:
    //拆分字符串成整形
    int splitInt(string szSrc, vector<int> &vecInt);
    //格式化时间
    string timeFormat(const string &sFormat, time_t timeCluster);
    //域名解析
    void getIp(char *domain, char *ip);
    //域名解析
    string getIp(const string &domain);
    //解析keyname
    int parsekeyName(const string &szSrc, KeyName &keyname);
    //只是操作cache
    bool checkCacheOnly(const KeyName &keyname);
    //拆分字符串
    vector<string> split(const string &str, const string &pattern);

private:
    //服务配置
    tars::TC_Config *_pFileConf;
    //DB群组配置
    tars::TC_Config *_pDBConf;
    //redis群组配置
    tars::TC_Config *_pFileRedis;
    //cache与DB对应表配置
    tars::TC_Config *_pFileRelative;

private:
    //代理工厂管理
    OuterProxyFactoryPtr _pProxyFactory;
    //读写锁，防止数据读写不一致
    wbl::ReadWriteLocker m_rwlock;
    //互斥锁数量
    int lockNum;

private: //MYSQL
    //数据类型
    DataType dbDataType;
    //分片因子类型
    SectionFactorType dbSectionFactorType;
    //分片因子
    SectionFactor dbSectionFactor;
    //DB群组
    DBCluster dbCluster;
    //REDIS与MYSQL映射
    DBMap dbMap;
    //数据库配置
    DBGroup dbGroup;
    //数据表
    Table table;

private: //REDIS
    //数据类型
    DataType redisDataType;
    //分片因子类型
    SectionFactorType redisSectionFactorType;
    //分片因子
    SectionFactor redisSectionFactor;
    //群组
    RedisCluster redisCluster;
    //cache群组配置
    RedisGroup redisGroup;
    //cache结构配置
    RedisDesc redisDesc;
    //数据库表配置
    TableDesc tableDesc;
};

//
typedef TC_AutoPtr<OuterFactoryImp> OuterFactoryImpPtr;

#endif


