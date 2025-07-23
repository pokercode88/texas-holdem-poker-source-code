#include <sstream>
#include "OuterFactoryImp.h"
#include "LogComm.h"
#include "DBAgentServer.h"

//
using namespace wbl;

//数据操作日志开关
bool openLog = false;

/**
 *
*/
OuterFactoryImp::OuterFactoryImp() : _pFileConf(NULL), _pDBConf(NULL), _pFileRedis(NULL), _pFileRelative(NULL)
{
    createAllObject();
}

/**
 *
*/
OuterFactoryImp::~OuterFactoryImp()
{
    deleteAllObject();
}

/**
 *
*/
void OuterFactoryImp::deleteAllObject()
{
    if (_pFileConf)
    {
        delete _pFileConf;
        _pFileConf = NULL;
    }

    if (_pDBConf)
    {
        delete _pDBConf;
        _pDBConf = NULL;
    }

    if (_pFileRedis)
    {
        delete _pFileRedis;
        _pFileRedis = NULL;
    }

    if (_pFileRelative)
    {
        delete _pFileRelative;
        _pFileRelative = NULL;
    }
}

void OuterFactoryImp::createAllObject()
{
    __TRY__

    //删除
    deleteAllObject();

    //tars代理Factory,访问其他tars接口时使用
    _pProxyFactory = new OuterProxyFactory();
    LOG_DEBUG << "init proxy factory succ." << endl;

    //加载配置
    load();

    __CATCH__
}

void OuterFactoryImp::load()
{
    //拉取远程配置
    g_app.addConfig(ServerConfig::ServerName + ".conf");
    g_app.addConfig("DB.conf");
    g_app.addConfig("Redis.conf");
    g_app.addConfig("Relative.conf");

    //本地配置文件
    _pFileConf = new tars::TC_Config();
    _pFileConf->parseFile(ServerConfig::BasePath + ServerConfig::ServerName + ".conf");
    LOG_DEBUG << "init config file succ, base path: " << ServerConfig::BasePath + ServerConfig::ServerName + ".conf" << endl;

    //DB群组配置
    _pDBConf = new tars::TC_Config();
    _pDBConf->parseFile(ServerConfig::BasePath + "DB.conf");
    LOG_DEBUG << "init db config file succ, base path: " << ServerConfig::BasePath + "DB.conf" << endl;

    //REDIS群组配置
    _pFileRedis = new tars::TC_Config();
    _pFileRedis->parseFile(ServerConfig::BasePath + "Redis.conf");
    LOG_DEBUG << "init config file succ:" << ServerConfig::BasePath + "redis.conf" << endl;

    //REDIS与DB映射配置
    _pFileRelative = new tars::TC_Config();
    _pFileRelative->parseFile(ServerConfig::BasePath + "Relative.conf");
    LOG_DEBUG << "init config file succ:" << ServerConfig::BasePath + "relative.conf" << endl;

    //读取所有配置
    readAllConfig();
}

// 读取所有配置
void OuterFactoryImp::readAllConfig()
{
    WriteLocker lock(m_rwlock);

    //读取MYSQL配置
    readDBConfig();

    //读取REDIS配置
    readRedisConfig();

    // 锁的数量
    readLockNum();
    printLockNum();
}

//读取MYSQL配置
void OuterFactoryImp::readDBConfig()
{
    // 读取数据类型配置
    readDBDataTypeConfig();
    printDBDataTypeConfig();

    // 分片因子类型
    readDBSectionFactorTypeConfig();
    printDBSectionFactorTypeConfig();

    // 读取分片因子配置
    readDBSectionFactorConfig();
    printDBSectionFactorConfig();
    printDBSectionFactorRealConfig();

    // 读取数据表配置
    readTableConfig();
    printTableConfig();

    // 读取数据库映射配置
    readDBMapConfig();
    printDBMapConfig();

    // 读取群组配置
    readDBClusterConfig();
    printDBClusterConfig();

    // 读取数据库群组配置
    readDBGroupConfig();
    printDBGroupConfig();
}

//读取REDIS配置
void OuterFactoryImp::readRedisConfig()
{
    // 读取数据类型配置
    readRedisDataTypeConfig();
    printRedisDataTypeConfig();

    // 分片因子类型
    readRedisSectionFactorTypeConfig();
    printRedisSectionFactorTypeConfig();

    // 读取分片因子配置
    readRedisSectionFactorConfig();
    printRedisSectionFactorConfig();
    printRedisSectionFactorRealConfig();

    // 读取群组配置
    readRedisClusterConfig();
    printRedisClusterConfig();

    // 读取数据库群组配置
    readRedisGroupConfig();
    printRedisGroupConfig();

    // 读取cache结构配置
    readRedisDescConfig();
    printRedisDescConfig();

    // 读取数据库表配置
    readTableDescConfig();
    printTableDescConfig();
}

// 读取数据类型配置
void OuterFactoryImp::readDBDataTypeConfig()
{
    const vector<string> &vecDomainKey = (*_pDBConf).getDomainKey("/Main/data_type");
    for (auto it = vecDomainKey.begin(); it != vecDomainKey.end(); ++it)
    {
        int value = TC_Common::strto<int>((*_pDBConf).get("/Main/data_type<" + *it + ">", "0"));
        dbDataType.mapData[value] = *it;
    }
}

void OuterFactoryImp::printDBDataTypeConfig()
{
    ostringstream os;
    os << "data type, ";

    for ( auto it = dbDataType.mapData.begin(); it != dbDataType.mapData.end(); it++)
    {
        os << "key: " << it->first << ", value: " << it->second << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const DataType &OuterFactoryImp::getDBDataTypeConfig()
{
    return dbDataType;
}

//分片因子类型
void OuterFactoryImp::readDBSectionFactorTypeConfig()
{
    const vector<string> &vecDomainKey = (*_pDBConf).getDomainKey("/Main/section_factor");
    for (auto it = vecDomainKey.begin(); it != vecDomainKey.end(); ++it)
    {
        int value = TC_Common::strto<int>((*_pDBConf).get("/Main/section_factor<" + *it + ">", "0"));
        dbSectionFactorType.mapData[value] = *it;
    }
}

void OuterFactoryImp::printDBSectionFactorTypeConfig()
{
    ostringstream os;
    os << "section factor type, ";

    for (auto it = dbSectionFactorType.mapData.begin(); it != dbSectionFactorType.mapData.end(); it++)
    {
        os << "key: " << it->first << ", value: " << it->second << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

// 读取分片因子配置
void OuterFactoryImp::readDBSectionFactorConfig()
{
    const vector<string> &vecDomain = (*_pDBConf).getDomainVector("/Main/factor_section");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        string path = "/Main/factor_section/" + *it;
        string szValue = (*_pDBConf).get(path + "<value>", "0");

        SectionFactorItem item;
        splitInt(szValue, item.vecData);
        item.factor = TC_Common::strto<int>((*_pDBConf).get(path + "<factor>", "0"));
        dbSectionFactor.mapData[*it] = item;

        map<int, int> mapValue;
        int index = 0;
        for (auto itItem = item.vecData.begin(); itItem != item.vecData.end(); ++itItem)
        {
            int result = *itItem / item.factor;
            for (int i = index; i < result; i++)
            {
                mapValue[index] = *itItem;
                index++;
            }
        }

        dbSectionFactor.mapRealData[*it] = mapValue;
    }
}

const SectionFactorType &OuterFactoryImp::getDBSectionFactorTypeConfig()
{
    return dbSectionFactorType;
}

void OuterFactoryImp::printDBSectionFactorConfig()
{
    ostringstream os;
    os << "section factor, ";

    for (auto it = dbSectionFactor.mapData.begin(); it != dbSectionFactor.mapData.end(); it++)
    {
        os << "key: " << it->first << ", factor: " << it->second.factor << ", value: ";
        for (auto itValue = it->second.vecData.begin(); itValue != it->second.vecData.end(); itValue++)
        {
            os << *itValue << " ";
        }
        os << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

void OuterFactoryImp::printDBSectionFactorRealConfig()
{
    ostringstream os;
    os << "section factor, ";

    for (auto it = dbSectionFactor.mapRealData.begin(); it != dbSectionFactor.mapRealData.end(); it++)
    {
        os << "key: " << it->first << ", value: ";
        for (auto itValue = it->second.begin(); itValue != it->second.end(); itValue++)
        {
            os << "|" << itValue->first << " " << itValue->second << "|";
        }

        os << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

//
const SectionFactor &OuterFactoryImp::getDBSectionFactorConfig()
{
    return dbSectionFactor;
}

// 读取数据表配置
void OuterFactoryImp::readTableConfig()
{
    const vector<string> &vecDomain = (*_pDBConf).getDomainVector("/Main/table");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        string path = "/Main/table/" + *it;

        TableItem tableItem;
        tableItem.dataType = TC_Common::strto<int>((*_pDBConf).get(path + "<data_type>", "0"));
        tableItem.sectionFactor = TC_Common::strto<int>((*_pDBConf).get(path + "<section_factor>", "0"));
        tableItem.dbName = (*_pDBConf).get(path + "<db_name>", "");
        tableItem.tableNum = TC_Common::strto<int>((*_pDBConf).get(path + "<table_num>", "1"));
        tableItem.sTimeFormat = (*_pDBConf).get(path + "<time_format>", "");
        table.mapData[*it] = tableItem;
    }
}

const Table &OuterFactoryImp::getTableConfig()
{
    return table;
}

void OuterFactoryImp::printTableConfig()
{
    ostringstream os;
    os << "table config, ";

    auto it = table.mapData.begin();
    for (; it != table.mapData.end(); it++)
    {
        os << "key: " << it->first
           << ", dataType: " << it->second.dataType
           << ", sectionFactor: " << it->second.sectionFactor
           << ", dbName: " <<  it->second.dbName
           << ", tableNum: " << it->second.tableNum
           << ", sTimeFormat: " << it->second.sTimeFormat << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

int OuterFactoryImp::getTableIndex(int keyIndex, const string &table_name, string &indexStr)
{
    wbl::ReadLocker lock(m_rwlock);

    indexStr = "";

    //找不到配置
    auto it = table.mapData.find(table_name);
    if (it == table.mapData.end())
    {
        return -1;
    }

    //不分表
    if (it->second.tableNum <= 0)
    {
        return 1;
    }

    if (it->second.tableNum == 1)
    {
        if (it->second.sTimeFormat.length() > 0)
        {
            string sztimeFormat = timeFormat(it->second.sTimeFormat, keyIndex);
            if (sztimeFormat.length() > 0)
            {
                indexStr += "_" + sztimeFormat;
            }
            else
            {
                return 2;
            }
        }

        return 3;
    }

    //取余数
    unsigned int index = (unsigned int)keyIndex % it->second.tableNum;
    char sz[10] = {0};
    sprintf(sz, "_%02u", index);
    indexStr += sz;
    return 0;
}

// 读取数据库映射配置
void OuterFactoryImp::readDBMapConfig()
{
    const vector<string> &vecDomain = (*_pDBConf).getDomainVector("/Main/database");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        DBMapItem dbMapItem;
        string path = "/Main/database/" + *it;
        dbMapItem.dbGroup = (*_pDBConf).get(path + "<db_group>", "0");
        dbMap.mapData[*it] = dbMapItem;
    }
}

void OuterFactoryImp::printDBMapConfig()
{
    ostringstream os;
    os << "db map config, ";

    auto it = dbMap.mapData.begin();
    for (; it != dbMap.mapData.end(); it++)
    {
        os << "key: " << it->first << ", dbGroup: " << it->second.dbGroup << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const DBMap &OuterFactoryImp::getDBMapConfig()
{
    return dbMap;
}

/*
*
#群组配置
    <cluster>
        <attribute>
            <user_id>
                <LE_1000000>
                    db_userinfo = db_userinfo_1
                    db_userwealth = db_userwealth_1
                    db_mp_core = db_mp_core_1
                    db_hp_correlation = db_hp_correlation_1
                    db_bookroom = db_bookroom_1
                </LE_1000000>
            </user_id>
            <string>
                <LE_4294967295>
                    db_hp_correlation = db_hp_correlation_1
                    db_gameserver = db_gameserver_1
                </LE_4294967295>
            </string>
        </attribute>
    </cluster>
*/
// 读取群组配置
void OuterFactoryImp::readDBClusterConfig()
{
    const vector<string> &vecCluster = (*_pDBConf).getDomainVector("/Main/cluster");
    for (auto it = vecCluster.begin(); it != vecCluster.end(); it++)
    {
        string path = "/Main/cluster/" + *it;
        ROLLLOG_DEBUG << "datetype: " << *it << endl;

        map<string, map<string, map<string, string> > > mapDataType;
        const vector<string> &vecDataType = (*_pDBConf).getDomainVector(path);
        for (auto itDatatype = vecDataType.begin(); itDatatype != vecDataType.end(); itDatatype++)
        {
            string path = "/Main/cluster/" + *it + "/" + *itDatatype;
            ROLLLOG_DEBUG << "db group: " << *itDatatype << endl;

            map<string, map<string, string> > mapGroup;

            const vector<string> &vecKey = (*_pDBConf).getDomainVector(path);
            for (auto itkey = vecKey.begin();  itkey != vecKey.end();  itkey++)
            {
                ROLLLOG_DEBUG << "key: " << *itkey << endl;

                string path = "/Main/cluster/" + *it + "/" + *itDatatype + "/" + *itkey;

                map<string, string> mapValue;
                const vector<string> &vecValue = (*_pDBConf).getDomainKey(path);
                for (auto itvalue = vecValue.begin(); itvalue != vecValue.end(); itvalue++)
                {
                    ROLLLOG_DEBUG << "value: " << *itvalue << endl;
                    string szValue = (*_pDBConf).get(path + "<" + *itvalue + ">", "0");
                    mapValue[*itvalue] = szValue;
                }

                mapGroup[*itkey] = mapValue;
            }

            mapDataType[*itDatatype] = mapGroup;
        }

        dbCluster.mapData[*it] = mapDataType;
    }
}

void OuterFactoryImp::printDBClusterConfig()
{
    ostringstream os;
    os << "dbCluster config, ";

    for (auto it = dbCluster.mapData.begin(); it != dbCluster.mapData.end(); ++it)
    {
        for (auto itDatatype = it->second.begin(); itDatatype != it->second.end(); ++itDatatype)
        {
            for (auto itGroup = itDatatype->second.begin(); itGroup != itDatatype->second.end(); ++itGroup)
            {
                for (auto itItem = itGroup->second.begin(); itItem != itGroup->second.end(); ++itItem)
                {
                    os << "dbCluster item, "
                       << it->first << ", "
                       << itDatatype->first << ", "
                       << itGroup->first << ", "
                       << itItem->first << ", "
                       << itItem->second << endl;
                }
            }
        }
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

//
const DBCluster &OuterFactoryImp::getDBClusterConfig()
{
    return dbCluster;
}

// 取分片配置
int OuterFactoryImp::getDBCluster(int keyIndex, const string &table_name, string &groupKey)
{
    wbl::ReadLocker lock(m_rwlock);

    //table
    auto itTable = getTableConfig().mapData.find(table_name);
    ROLLLOG_DEBUG << "table name: " << table_name << endl;
    if (itTable == getTableConfig().mapData.end())
    {
        ROLLLOG_ERROR << "find table, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey << endl;
        return -1;
    }

    //data type
    auto itDatatype = getDBDataTypeConfig().mapData.find(itTable->second.dataType);
    ROLLLOG_DEBUG << "data type: " << itTable->second.dataType << endl;
    if (itDatatype == getDBDataTypeConfig().mapData.end())
    {
        ROLLLOG_ERROR << "find data type, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", data type : " << itTable->second.dataType << endl;
        return -2;
    }

    //cluster
    auto itCluster = getDBClusterConfig().mapData.find(itDatatype->second);
    ROLLLOG_DEBUG << "cluster type: " << itDatatype->second << endl;
    if (itCluster == getDBClusterConfig().mapData.end())
    {
        ROLLLOG_ERROR << "find cluster config, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", cluster type : " << itDatatype->second << endl;
        return -3;
    }

    //section
    auto itFactorType = getDBSectionFactorTypeConfig().mapData.find(itTable->second.sectionFactor);
    ROLLLOG_DEBUG << "section factor: " << itTable->second.sectionFactor << endl;
    if (itFactorType == getDBSectionFactorTypeConfig().mapData.end())
    {
        ROLLLOG_ERROR << "find section type, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", section factor : " << itTable->second.sectionFactor << endl;
        return -4;
    }

    //
    auto itSection = getDBSectionFactorConfig().mapData.find(itFactorType->second);
    ROLLLOG_DEBUG << "factor type: " << itFactorType->second << endl;
    if (itSection == getDBSectionFactorConfig().mapData.end())
    {
        ROLLLOG_ERROR << "find factor type, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", factor type: " << itFactorType->second << endl;
        return -5;
    }

    auto itSectionReal = getDBSectionFactorConfig().mapRealData.find(itFactorType->second);
    ROLLLOG_DEBUG << "factor type: " << itFactorType->second << endl;
    if (itSectionReal == getDBSectionFactorConfig().mapRealData.end())
    {
        ROLLLOG_ERROR << "find real data, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", factor type: " << itFactorType->second << endl;
        return -6;
    }

    int result = (unsigned int)keyIndex / (unsigned int)itSection->second.factor;
    auto itRange = itSectionReal->second.find(result);
    ROLLLOG_DEBUG << "keyindex: " << (unsigned int)keyIndex << ", factor: " << (unsigned int)itSection->second.factor << ", result: " << result << ", " << endl;
    if (itRange == itSectionReal->second.end())
    {
        ROLLLOG_ERROR << "find range, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", range: " << result << endl;
        return -7;
    }

    //cluster
    auto itClusterSection = itCluster->second.find(itFactorType->second);
    ROLLLOG_DEBUG << "factor type: " << itFactorType->second << endl;
    if (itClusterSection == itCluster->second.end())
    {
        ROLLLOG_ERROR << "find cluster, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", factor tpye: " << itFactorType->second << endl;
        return -8;
    }

    //cluster
    auto itClusterFactor = itClusterSection->second.find("LE_" + U2S((unsigned int)itRange->second));
    ROLLLOG_DEBUG << "range: " << (unsigned int)itRange->second << endl;
    if (itClusterFactor == itClusterSection->second.end())
    {
        ROLLLOG_ERROR << "find cluster, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", cluster factor: " << (unsigned int)itRange->second << endl;
        return -9;
    }

    //无配置
    if (itClusterFactor->second.size() == 0)
    {
        ROLLLOG_ERROR << "now cluster, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey << endl;
        return -10;
    }

    //数据库
    auto itDB = getDBMapConfig().mapData.find(itTable->second.dbName);
    ROLLLOG_DEBUG << "db name: " << itTable->second.dbName << endl;
    if (itDB == getDBMapConfig().mapData.end())
    {
        ROLLLOG_ERROR << "find db, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", db name: " << itTable->second.dbName << endl;
        return -11;
    }

    //
    auto itGroupKey = itClusterFactor->second.find(itDB->second.dbGroup);
    ROLLLOG_DEBUG << "db group: " << itDB->second.dbGroup << endl;
    if (itGroupKey == itClusterFactor->second.end())
    {
        ROLLLOG_ERROR << "find group, keyindex: " << keyIndex << ", table name: " << table_name << ", group key: " << groupKey
                      << ", group: " << itDB->second.dbGroup << endl;
        return -12;
    }

    //返回值
    groupKey = itGroupKey->second;

    return 0;
}

// 读取数据库群组配置
void OuterFactoryImp::readDBGroupConfig()
{
    const vector<string> &vecDomain = (*_pDBConf).getDomainVector("/Main/db_group");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        DBGroupItem dbGroupItem;
        string path = "/Main/db_group/" + *it;

        dbGroupItem.szMasterHost = (*_pDBConf).get(path + "<master_host>", "");
        dbGroupItem.szMasterDomain = (*_pDBConf).get(path + "<master_domain>", "");
        dbGroupItem.szSlaveHost = (*_pDBConf).get(path + "<slave_host>", "");
        dbGroupItem.szSlaveDomain = (*_pDBConf).get(path + "<slave_domain>", "");
        dbGroupItem.port = (*_pDBConf).get(path + "<port>", "0");
        dbGroupItem.user = (*_pDBConf).get(path + "<user>", "0");
        dbGroupItem.password = (*_pDBConf).get(path + "<password>", "");
        dbGroupItem.charset = (*_pDBConf).get(path + "<charset>", "");
        dbGroupItem.dbname = (*_pDBConf).get(path + "<dbname>", "");

        //
        string szMasterHost = "";
        if (dbGroupItem.szMasterDomain.length() > 0)
        {
            szMasterHost = getIp(dbGroupItem.szMasterDomain);
            if (szMasterHost.length() > 0)
            {
                dbGroupItem.szMasterHost = szMasterHost;
                ROLLLOG_DEBUG << "get master host by domain, szMasterDomain: " << dbGroupItem.szMasterDomain << ", szMasterHost: " << szMasterHost << endl;
            }
        }

        //
        string szSlaveHost = "";
        if (dbGroupItem.szSlaveDomain.length() > 0)
        {
            szSlaveHost = getIp(dbGroupItem.szSlaveDomain);
            if (szMasterHost.length() > 0)
            {
                dbGroupItem.szSlaveHost = szSlaveHost;
                ROLLLOG_DEBUG << "get slave host by domain, szSlaveDomain: " << dbGroupItem.szSlaveDomain << ", szSlaveHost: " << szSlaveHost << endl;
            }
        }

        dbGroup.mapData[*it] = dbGroupItem;
    }
}

void OuterFactoryImp::printDBGroupConfig()
{
    ostringstream os;
    os << "db group config, ";

    for (auto it = dbGroup.mapData.begin(); it != dbGroup.mapData.end(); it++)
    {

        os << "key: " << it->first
           << ", szMasterHost: " << it->second.szMasterHost
           << ", szMasterDomain: " << it->second.szMasterDomain
           << ", szSlaveHost: " << it->second.szSlaveHost
           << ", szSlaveDomain: " << it->second.szSlaveDomain
           << ", port: " <<  it->second.port
           << ", user: " << it->second.user
           << ", password: " << it->second.password
           << ", charset: " << it->second.charset
           << ", dbname: " << it->second.dbname << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

//
const DBGroup &OuterFactoryImp::getDBGroupConfig()
{
    return dbGroup;
}

//读取锁数量配置
void OuterFactoryImp::readLockNum()
{
    lockNum = TC_Common::strto<int>((*_pFileConf).get("/Main/lock_num<number>", "10"));
    openLog = TC_Common::strto<bool>((*_pFileConf).get("/Main/System<openLog>", "false"));
}

//打印锁数量配置
void OuterFactoryImp::printLockNum()
{
    FDLOG_CONFIG_INFO << "lock num: " << lockNum << endl;
}

//取锁数量配置
int OuterFactoryImp::getLockNum()
{
    wbl::ReadLocker lock(m_rwlock);
    return lockNum;
}

//拆分字符串成整形
int OuterFactoryImp::splitInt(string szSrc, vector<int> &vecInt)
{
    split_int(szSrc, "[ \t]*\\|[ \t]*", vecInt);
    return 0;
}

//格式化时间
string OuterFactoryImp::timeFormat(const string &sFormat, time_t timeCluster)
{
    //string sFormat("%Y-%m-%d %H:%M:%S");
    if (sFormat.length() == 0)
    {
        return "";
    }

    //time
    //time_t t = time(NULL);
    time_t t = timeCluster;
    struct tm *pTm = localtime(&t);
    if (pTm == NULL)
    {
        return "";
    }

    //format
    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);

    return string(sTimeString);
}

//域名解析
void OuterFactoryImp::getIp(char *domain, char *ip)
{
    if (domain == NULL || ip == NULL)
    {
        return;
    }

    if (strlen(domain) == 0)
    {
        return;
    }

    //
    struct hostent host = *gethostbyname(domain);
    for (int i = 0; host.h_addr_list[i]; i++)
    {
        strcpy(ip, inet_ntoa(*(struct in_addr *)host.h_addr_list[i]));

        break;
    }
}

//域名解析
string OuterFactoryImp::getIp(const string &domain)
{
    if (domain.length() == 0)
    {
        return "";
    }

    //
    struct hostent host = *gethostbyname(domain.c_str());
    for (int i = 0; host.h_addr_list[i]; i++)
    {
        string ip = inet_ntoa(*(struct in_addr *)host.h_addr_list[i]);

        return ip;
    }

    return "";
}


// 读取数据类型配置
void OuterFactoryImp::readRedisDataTypeConfig()
{
    const vector<string> &vecDomainKey = (*_pFileRedis).getDomainKey("/Main/data_type");
    for (auto it = vecDomainKey.begin(); it != vecDomainKey.end(); ++it)
    {
        int value = TC_Common::strto<int>((*_pFileRedis).get("/Main/data_type<" + *it + ">", "0"));
        redisDataType.mapData[value] = *it;
    }
}

//
void OuterFactoryImp::printRedisDataTypeConfig()
{
    ostringstream os;
    os << "data type, ";

    for (auto it = redisDataType.mapData.begin(); it != redisDataType.mapData.end(); it++)
    {
        os << "key: " << it->first << ", value: " << it->second << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

//
const DataType &OuterFactoryImp::getRedisDataTypeConfig()
{
    return redisDataType;
}

//分片因子类型
void OuterFactoryImp::readRedisSectionFactorTypeConfig()
{
    const vector<string> &vecDomainKey = (*_pFileRedis).getDomainKey("/Main/section_factor");
    for (auto it = vecDomainKey.begin(); it != vecDomainKey.end(); ++it)
    {
        int value = TC_Common::strto<int>((*_pFileRedis).get("/Main/section_factor<" + *it + ">", "0"));
        redisSectionFactorType.mapData[value] = *it;
    }
}

void OuterFactoryImp::printRedisSectionFactorTypeConfig()
{
    ostringstream os;
    os << "section factor type, ";

    for (auto it = redisSectionFactorType.mapData.begin(); it != redisSectionFactorType.mapData.end(); it++)
    {
        os << "key: " << it->first << ", value: " << it->second << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const SectionFactorType &OuterFactoryImp::getRedisSectionFactorTypeConfig()
{
    return redisSectionFactorType;
}

// 读取分片因子配置
void OuterFactoryImp::readRedisSectionFactorConfig()
{
    const vector<string> &vecDomain = (*_pFileRedis).getDomainVector("/Main/factor_section");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        string path = "/Main/factor_section/" + *it;
        string szValue = (*_pFileRedis).get(path + "<value>", "0");

        SectionFactorItem item;
        splitInt(szValue, item.vecData);
        item.factor = TC_Common::strto<int>((*_pFileRedis).get(path + "<factor>", "0"));
        redisSectionFactor.mapData[*it] = item;

        //map
        map<int, int> mapValue;
        int index = 0;
        for (auto itItem = item.vecData.begin(); itItem != item.vecData.end(); ++itItem)
        {
            int result = *itItem / item.factor;
            for (int i = index; i < result; i++)
            {
                mapValue[index] = *itItem;
                index++;
            }
        }

        redisSectionFactor.mapRealData[*it] = mapValue;
    }
}

void OuterFactoryImp::printRedisSectionFactorConfig()
{
    ostringstream os;
    os << "section factor, ";

    for (auto it = redisSectionFactor.mapData.begin(); it != redisSectionFactor.mapData.end(); it++)
    {
        os << "key: " << it->first << ", factor: " << it->second.factor << ", value: ";

        for (auto itValue = it->second.vecData.begin(); itValue != it->second.vecData.end(); itValue++)
        {
            os << *itValue << " ";
        }

        os << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

void OuterFactoryImp::printRedisSectionFactorRealConfig()
{
    ostringstream os;
    os << "section factor, ";

    for (auto it = redisSectionFactor.mapRealData.begin(); it != redisSectionFactor.mapRealData.end(); it++)
    {
        os << "key: " << it->first << ", value: ";
        for (auto itValue = it->second.begin(); itValue != it->second.end();  itValue++)
        {
            os << "|" << itValue->first << " " << itValue->second << "|";
        }
        os << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const SectionFactor &OuterFactoryImp::getRedisSectionFactorConfig()
{
    return redisSectionFactor;
}

// 读取群组配置
void OuterFactoryImp::readRedisClusterConfig()
{
    const vector<string> &vecCluster = (*_pFileRedis).getDomainVector("/Main/cluster");
    for (auto it = vecCluster.begin(); it != vecCluster.end(); it++)
    {
        string path = "/Main/cluster/" + *it;
        ROLLLOG_DEBUG << "datetype: " << *it << endl;

        map<string, map<string, vector<string> > > mapDataType;
        const vector<string> &vecDataType = (*_pFileRedis).getDomainVector(path);
        for (auto itDatatype = vecDataType.begin(); itDatatype != vecDataType.end();  itDatatype++)
        {
            string path = "/Main/cluster/" + *it + "/" + *itDatatype;
            ROLLLOG_DEBUG << "redis group: " << *itDatatype << endl;
            map<string, vector<string> > mapGroup;
            const vector<string> &vecKey = (*_pFileRedis).getDomainKey(path);
            for (auto itkey = vecKey.begin();  itkey != vecKey.end(); itkey++)
            {
                ROLLLOG_DEBUG << "key: " << *itkey << endl;
                string path = "/Main/cluster/" + *it + "/" + *itDatatype;
                string szValue = (*_pFileRedis).get(path + "<" + *itkey + ">", "0");
                vector<string> vecValue = split(szValue, "|");
                mapGroup[*itkey] = vecValue;
            }

            mapDataType[*itDatatype] = mapGroup;
        }

        redisCluster.mapData[*it] = mapDataType;
    }
}

void OuterFactoryImp::printRedisClusterConfig()
{
    ostringstream os;
    os << "redisCluster config, ";

    for (auto it = redisCluster.mapData.begin(); it != redisCluster.mapData.end(); ++it)
    {
        for (auto itDatatype = it->second.begin(); itDatatype != it->second.end(); ++itDatatype)
        {
            for (auto itGroup = itDatatype->second.begin(); itGroup != itDatatype->second.end(); ++itGroup)
            {
                for (auto itItem = itGroup->second.begin(); itItem != itGroup->second.end(); ++itItem)
                {
                    os << "redisCluster item, " << it->first << ", " << itDatatype->first << ", " << itGroup->first << ", " << *itItem << endl;
                }
            }
        }
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const RedisCluster &OuterFactoryImp::getRedisClusterConfig()
{
    return redisCluster;
}

int OuterFactoryImp::getRedisCluster(const TClusterInfo &clusterInfo, string &groupKey)
{
    wbl::ReadLocker lock(m_rwlock);

    //data type
    auto itDatatype = getRedisDataTypeConfig().mapData.find(clusterInfo.busiType);
    if (itDatatype == getRedisDataTypeConfig().mapData.end())
    {
        return -1;
    }

    //cluster
    auto itCluster = getRedisClusterConfig().mapData.find(itDatatype->second);
    if (itCluster == getRedisClusterConfig().mapData.end())
    {
        return -2;
    }

    //section
    auto itFactorType = getRedisSectionFactorTypeConfig().mapData.find(clusterInfo.frageFactorType);
    if (itFactorType == getRedisSectionFactorTypeConfig().mapData.end())
    {
        return -3;
    }

    //
    auto itSection = getRedisSectionFactorConfig().mapData.find(itFactorType->second);
    if (itSection == getRedisSectionFactorConfig().mapData.end())
    {
        return -4;
    }

    auto itSectionReal = getRedisSectionFactorConfig().mapRealData.find(itFactorType->second);
    if (itSectionReal == getRedisSectionFactorConfig().mapRealData.end())
    {
        return -5;
    }

    //ROLLLOG_DEBUG << "frageFactor: " << (unsigned)clusterInfo.frageFactor << ", factor: " << (unsigned)itSection->second.factor << endl;

    int result = (unsigned)clusterInfo.frageFactor / (unsigned)itSection->second.factor;
    auto itRange = itSectionReal->second.find(result);
    if (itRange == itSectionReal->second.end())
    {
        return -6;
    }

    //cluster
    auto itClusterSection = itCluster->second.find(itFactorType->second);
    if (itClusterSection == itCluster->second.end())
    {
        return -7;
    }

    //cluster
    //ROLLLOG_DEBUG << "group key: " << "LE_" + I2S(range) << endl;
    auto itClusterFactor = itClusterSection->second.find("LE_" + U2S((unsigned int)itRange->second));
    if (itClusterFactor == itClusterSection->second.end())
    {
        return -8;
    }

    //无配置
    if (itClusterFactor->second.size() == 0)
    {
        return -9;
    }

    //先按照分片因子取模,后续用一致性hash
    groupKey = itClusterFactor->second[(unsigned int)clusterInfo.frageFactor % itClusterFactor->second.size()];
    return 0;
}

// 读取cache群组配置
void OuterFactoryImp::readRedisGroupConfig()
{
    const vector<string> &vecDomain = (*_pFileRedis).getDomainVector("/Main/redis_group");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        string path = "/Main/redis_group/" + *it;
        string szMaster = (*_pFileRedis).get(path + "<master>", "");
        string szSlave = (*_pFileRedis).get(path + "<slave>", "");
        int masterPort = S2I((*_pFileRedis).get(path + "<master_port>", "0"));
        int slavePort = S2I((*_pFileRedis).get(path + "<slave_port>", "0"));
        string szMasterDomain = (*_pFileRedis).get(path + "<master_domain>", "");
        string szSlaveDomain = (*_pFileRedis).get(path + "<slave_domain>", "");
        string szMasterPasswd = (*_pFileRedis).get(path + "<master_password>", "");
        string szSlavePasswd = (*_pFileRedis).get(path + "<slave_password>", "");
        ROLLLOG_DEBUG << "master: " << szMaster << ", slave: " << szSlave
                      << ", masterPort: " << masterPort << ", slavePort: " << slavePort
                      << ", szMasterDomain: " << szMasterDomain << ", szSlaveDomain: " << szSlaveDomain
                      << ", master_password: " << szMasterPasswd << ", slave_password: " << szSlavePasswd << endl;

        //port
        vector<string> vecMaster = split(szMaster, ":");
        vector<string> vecSlave = split(szSlave, ":");
        ROLLLOG_DEBUG << "vecMaster: " << vecMaster.size() << ", vecSlave: " << vecSlave.size() << endl;
        if (vecMaster.size() != 2 || vecSlave.size() != 2)
        {
            return;
        }

        //
        string szMasterHost = "";
        if (szMasterDomain.length() > 0)
        {
            szMasterHost = getIp(szMasterDomain);
            ROLLLOG_DEBUG << "get master host by domain, szMasterDomain: " << szMasterDomain << ", szMasterHost: " << szMasterHost << endl;
        }

        string szSlaveHost = "";
        if (szSlaveDomain.length() > 0)
        {
            szSlaveHost = getIp(szSlaveDomain);
            ROLLLOG_DEBUG << "get slave host by domain, szSlaveDomain: " << szSlaveDomain << ", szSlaveHost: " << szSlaveHost << endl;
        }

        //cache配置
        RedisGroupItem redisGroupItem;

        //主
        if (szMasterHost.length() > 0)
        {
            redisGroupItem.szMasterHost = szMasterHost;
            redisGroupItem.szMasterPort = I2S(masterPort);
        }
        else
        {
            redisGroupItem.szMasterHost = vecMaster[0];
            redisGroupItem.szMasterPort = vecMaster[1];
        }

        // redisGroupItem.szMasterHost = vecMaster[0];
        // redisGroupItem.szMasterPort = vecMaster[1];
        redisGroupItem.szMasterPasswd = szMasterPasswd;
        redisGroupItem.szMasterDomain = szMasterDomain;

        //备
        if (szSlaveHost.length() > 0)
        {
            redisGroupItem.szSlaveHost  = szSlave;
            redisGroupItem.szSlavePort  = I2S(slavePort);
        }
        else
        {
            redisGroupItem.szSlaveHost  = vecSlave[0];
            redisGroupItem.szSlavePort  = vecSlave[1];
        }

        redisGroupItem.szSlaveHost  = vecSlave[0];
        redisGroupItem.szSlavePort  = vecSlave[1];
        redisGroupItem.szSlavePasswd = szSlavePasswd;
        redisGroupItem.szSlaveDomain = szSlaveDomain;
        redisGroup.mapData[*it] = redisGroupItem;
    }
}

void OuterFactoryImp::printRedisGroupConfig()
{
    ostringstream os;
    os << "redis group config, ";

    for (auto it = redisGroup.mapData.begin(); it != redisGroup.mapData.end(); it++)
    {
        os << "key: " << it->first
           << ", szMasterHost: " << it->second.szMasterHost
           << ", szSlaveHost: "  << it->second.szSlaveHost
           << ", szMasterPasswd: " << it->second.szMasterPasswd
           << ", szMasterPort: " <<  it->second.szMasterPort
           << ", szSlavePort: "  << it->second.szSlavePort
           << ", szSlavePasswd: " << it->second.szSlavePasswd
           << ", szMasterDomain: " << it->second.szMasterDomain
           << ", szSlaveDomain: " << it->second.szSlaveDomain
           << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const RedisGroup &OuterFactoryImp::getRedisGroupConfig()
{
    return redisGroup;
}

//读取cache结构配置
void OuterFactoryImp::readRedisDescConfig()
{
    const vector<string> &vecDomain = (*_pFileRelative).getDomainVector("/Main/redis_struct");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        string path = "/Main/redis_struct/" + *it;

        RedisDescItem redisDescItem;
        redisDescItem.dataType       = TC_Common::strto<int>((*_pFileRelative).get(path + "<data_type>", "0"));
        redisDescItem.sectionFactor  = TC_Common::strto<int>((*_pFileRelative).get(path + "<section_factor>", "0"));
        redisDescItem.ttl            = TC_Common::strto<int>((*_pFileRelative).get(path + "<ttl>", "0"));
        redisDescItem.szTableName    = (*_pFileRelative).get(path + "<table>", "");
        //redisDescItem.relative       = TC_Common::strto<int>((*_pFileRelative).get(path + "<relative>", "0"));
        redisDescItem.key_field      = (*_pFileRelative).get(path + "<key_field>", "");
        redisDescItem.key_field_type = TC_Common::strto<int>((*_pFileRelative).get(path + "<key_field_type>", "0"));
        //redisDescItem.join_fields  = split((*_pFileRelative).get(path + "<join_fields>", ""), "|");
        //redisDescItem.join_fields_type = splitInt((*_pFileRelative).get(path + "<join_fields_type>", ""), "|");
        //splitInt((*_pFileRelative).get(path + "<join_fields_type>", ""), redisDescItem.join_fields_type);

        //conditions
        //relation
        //string pathConditions = "/Main/redis_struct/" + *it + "/conditions";
        string pathConditions = path + "/conditions";
        redisDescItem.conditions.groupRelation = (Eum_Relation)TC_Common::strto<int>((*_pFileRelative).get(pathConditions + "<relation>", "0"));
        //ROLLLOG_DEBUG << "groupRelation: " << redisDescItem.conditions.groupRelation << endl;

        //condition
        const vector<string> &vecDomainCondition = (*_pFileRelative).getDomainVector(pathConditions + "/condition");
        for (auto itCondition = vecDomainCondition.begin();  itCondition != vecDomainCondition.end(); itCondition++)
        {
            string pathFields = pathConditions + "/condition/" + *itCondition;
            ConditionGroup conditionGroup;
            conditionGroup.relation = (Eum_Relation)TC_Common::strto<int>((*_pFileRelative).get(pathFields + "<relation>", "0"));
            //ROLLLOG_DEBUG << "conditionGroup.relation: " << conditionGroup.relation << endl;

            const vector<string> &vecDomainFields = (*_pFileRelative).getDomainVector(pathFields);
            for (auto itfields = vecDomainFields.begin(); itfields != vecDomainFields.end(); ++itfields)
            {
                dbagent::Condition condition;
                condition.condtion  = (Eum_Condition)TC_Common::strto<int>((*_pFileRelative).get(pathFields + "/" + *itfields + "<condition_type>", "0"));
                condition.colType   = (Eum_Col_Type)TC_Common::strto<int>((*_pFileRelative).get(pathFields + "/" + *itfields + "<col_type>", "0"));
                condition.colName   = (*_pFileRelative).get(pathFields + "/" + *itfields + "<col_name>", "0");
                condition.colValues = (*_pFileRelative).get(pathFields + "/" + *itfields + "<col_value>", "0");
                conditionGroup.condition.push_back(condition);

                ROLLLOG_DEBUG << "condition, condition: " << condition.condtion
                              << ", colType: " << condition.colType
                              << ", colName: " << condition.colName
                              << ", colValues: " << condition.colValues << endl;
            }

            redisDescItem.conditions.conditions.push_back(conditionGroup);
        }

        //groupby
        const vector<string> &vecDomainGroupby = (*_pFileRelative).getDomainKey(path + "/conditions/groupby");
        redisDescItem.conditions.groupbyCol = vecDomainGroupby;
        //ROLLLOG_DEBUG << "groupbyCol size: " << redisDescItem.conditions.groupbyCol.size() << endl;

        //orderby
        const vector<string> &vecDomainOrderby = (*_pFileRelative).getDomainVector(path + "/conditions/orderby");
        for (auto itOrderby = vecDomainOrderby.begin(); itOrderby != vecDomainOrderby.end(); ++itOrderby)
        {
            OrderBy orderby;
            orderby.sort = (Eum_Sort)TC_Common::strto<int>((*_pFileRelative).get(path + "/conditions/orderby" + "/" + *itOrderby + "<sort_type>", "0"));
            orderby.colName = (*_pFileRelative).get(path + "/conditions/orderby" + "/" + *itOrderby + "<col_name>", "0");
            redisDescItem.conditions.orderbyCol.push_back(orderby);
        }

        ROLLLOG_DEBUG << "orderbyCol size: " << redisDescItem.conditions.orderbyCol.size() << endl;
        redisDesc.mapData[*it] = redisDescItem;
    }
}

void OuterFactoryImp::printRedisDescConfig()
{
    ostringstream os;
    os << "redis describle config, ";

    auto it = redisDesc.mapData.begin();
    for (; it != redisDesc.mapData.end(); it++)
    {
        os << "key: " << it->first
           << ", dataType: "       << it->second.dataType
           << ", sectionFactor: "  << it->second.sectionFactor
           << ", ttl: "            << it->second.ttl
           << ", szTableName: "    << it->second.szTableName
           //<< ", relative: "       << it->second.relative
           << ", key_field: "      << it->second.key_field
           << ", key_field_type: " << it->second.key_field_type;
#if 0
                << ", join_fields: ";

        //联合字段
        for (auto itJoinFields = it->second.join_fields.begin();
                itJoinFields != it->second.join_fields.end();
                ++itJoinFields)
        {
            os << " ; "  << *itJoinFields;
        }

        //联合字段类型
        os << ", join_fields_type: ";
        for (auto itJoinFieldsType = it->second.join_fields_type.begin();
                itJoinFieldsType != it->second.join_fields_type.end();
                ++itJoinFieldsType)
        {
            os << " ; " << *itJoinFieldsType;
        }
#endif

        //conditions
        os << ", conditions, groupRelation: " << it->second.conditions.groupRelation;

        //条件组
        for (auto itconditionGroup = it->second.conditions.conditions.begin(); itconditionGroup != it->second.conditions.conditions.end(); ++itconditionGroup)
        {
            os << ", relation: " << itconditionGroup->relation;
            for (auto itCondition = itconditionGroup->condition.begin(); itCondition != itconditionGroup->condition.end();  ++itCondition)
            {
                os << ", condtion: " << itCondition->condtion << ", colType: " << itCondition->colType << ", colName: "
                   << itCondition->colName << ", colValues: " << itCondition->colValues;
            }
        }

        //分组
        os << ", groupby, value: ";
        for (auto itgroupby = it->second.conditions.groupbyCol.begin(); itgroupby !=  it->second.conditions.groupbyCol.end(); ++itgroupby)
        {
            os << ", " << *itgroupby;
        }

        //排序
        os << ", orderby, value: ";
        for (auto itorderby = it->second.conditions.orderbyCol.begin(); itorderby != it->second.conditions.orderbyCol.end(); ++itorderby)
        {
            os << ", sort: " << itorderby->sort << ", colName: " << itorderby->colName;
        }

        os << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const RedisDesc &OuterFactoryImp::getRedisDescConfig()
{
    wbl::ReadLocker lock(m_rwlock);
    return redisDesc;
}

//读取数据库表配置
void OuterFactoryImp::readTableDescConfig()
{
    const vector<string> &vecDomain = (*_pFileRelative).getDomainVector("/Main/tables");
    for (auto it = vecDomain.begin(); it != vecDomain.end(); it++)
    {
        string path = "/Main/tables/" + *it;
        TableDescItem tableDescItem;
        tableDescItem.dataType = TC_Common::strto<int>((*_pFileRelative).get(path + "<data_type>", "0"));
        tableDescItem.sectionFactor = TC_Common::strto<int>((*_pFileRelative).get(path + "<section_factor>", "0"));
        string szCols = (*_pFileRelative).get(path + "<cols>", "0");
        tableDescItem.vecCols = split(szCols, "|");
        tableDesc.mapData[*it] = tableDescItem;
    }
}

void OuterFactoryImp::printTableDescConfig()
{
    ostringstream os;
    os << "table describle config, ";

    for (auto it = tableDesc.mapData.begin(); it != tableDesc.mapData.end(); it++)
    {
        os << "key: " << it->first << ", dataType: " << it->second.dataType << ", sectionFactor: " << it->second.sectionFactor << ", values: ";

        //字段
        for (auto itCols = it->second.vecCols.begin(); itCols != it->second.vecCols.end(); ++itCols)
        {
            os << " ; "  << *itCols;
        }

        os << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
}

const TableDesc &OuterFactoryImp::getTableDescConfig()
{
    wbl::ReadLocker lock(m_rwlock);
    return tableDesc;
}

//解析keyname
int OuterFactoryImp::parsekeyName(const string &szSrc, KeyName &keyname)
{
    if (szSrc.length() == 0)
        return -1;

    //拆分
    vector<string> vecitems;
    size_t idx = 0;
    for (size_t i = 0; i < E_FIELD_INDEX_COUNT - 1; ++i)
    {
        size_t start_idx = idx;
        idx = szSrc.find(":", idx);
        if (idx == string::npos)
            return -2;

        size_t len = idx - start_idx;
        vecitems.push_back(szSrc.substr(start_idx, len ));
        idx += 1;
    }

    vecitems.push_back(szSrc.substr(idx));
    if (vecitems.size() != E_FIELD_INDEX_COUNT)
    {
        return -2;
    }

    //保存
    keyname.redis_type = S2I(vecitems[E_FIELD_INDEX_REDIS_TYPE]);
    keyname.busi_type = S2I(vecitems[E_FIELD_INDEX_BUSI_TYPE]);
    keyname.sbusi_type = vecitems[E_FIELD_INDEX_BUSI_TYPE];
    keyname.key = vecitems[E_FIELD_INDEX_KEY];

    //cache数据类型
    if (keyname.redis_type < E_REDIS_TYPE_HASH || keyname.redis_type > E_REDIS_TYPE_SORT_SET)
    {
        return -3;
    }

    //业务类型
    if (keyname.busi_type < 0)
    {
        return -4;
    }

    //业务类型
    if (keyname.sbusi_type.length() == 0)
    {
        return -5;
    }

    //主键
    if (keyname.key.length() == 0)
    {
        return -6;
    }

    return 0;
}

//只是操作cache
bool OuterFactoryImp::checkCacheOnly(const KeyName &keyname)
{
    const RedisDesc &redisDesc = getRedisDescConfig();
    auto it = redisDesc.mapData.find(keyname.sbusi_type);
    if (it == redisDesc.mapData.end())
        return true;

    const TableDesc &tableDesc = getTableDescConfig();
    auto ittable = tableDesc.mapData.find(it->second.szTableName);
    if (ittable == tableDesc.mapData.end())
        return true;

    return false;
}

//拆分字符串
vector<std::string> OuterFactoryImp::split(const string &str, const string &pattern)
{
    return SEPSTR(str, pattern);
}
