#ifndef _DB_OPERATOR_MANAGER_H_
#define _DB_OPERATOR_MANAGER_H_

#include <string>
#include <map>
#include <vector>

//
#include <util/tc_config.h>
#include <util/tc_mysql.h>
#include <util/tc_singleton.h>
#include <util/tc_autoptr.h>
#include <wbl/pthread_util.h>

//数据库操作对象
#include "DBOperator.h"

//
using namespace std;
using namespace tars;

/**
* DB节点类型
*/
enum Eum_Db_Type
{
    E_DB_TYPE_MASTER = 0,  //主库
    E_DB_TYPE_SLAVE  = 1,  //备库
    E_DB_TYPE_COUNT  = 2,  //一组DB，一主一备，2个
};

/**
* 读写类型
*/
enum Eum_Db_Read_Write_Type
{
    E_DB_READ_TYPE      = 0, //读类型
    E_DB_WRITE_TYPE     = 1, //写类型
    E_DB_READ_WRITE_ALL = 2, //读写一体
};

/**
*
* DB操作对象管理
**/
class CDBOperatorManager : public TC_HandleBase
{
public:
    /**
     *
    */
    CDBOperatorManager();

    /**
     *
    */
    virtual ~CDBOperatorManager();

public:
    //初始化
    int init();
    //初始化
    int init(Eum_Db_Read_Write_Type type);
    //初始化db操作对象
    int initDBOperator();
    //初始化db操作对象
    int initDBOperator(Eum_Db_Read_Write_Type type);
    //取db操作对象
    const CDBOperatorPtr getDBOperatorByKey(const string key, Eum_Db_Type type);
    //取db操作对象
    const CDBOperatorPtr getReadWriteDBOprator(const string key, Eum_Db_Read_Write_Type type);

private:
    //数据库连接对象
    map<string, vector<CDBOperatorPtr> > mapDBOperator;
    //数据库连接对象
    map<string, map<int, CDBOperatorPtr> > mapDBReadWrite;
};

/////
typedef TC_AutoPtr<CDBOperatorManager> CDBOperatorManagerPtr;

#endif


