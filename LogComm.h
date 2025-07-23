#ifndef __LOGCOMM_H__
#define __LOGCOMM_H__

#include <util/tc_logger.h>

//
using namespace tars;

//统计日志开关
extern bool openLog;

//
#define ROLLLOG(level) (LOG->level() << "[" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "] ")
#define ROLLLOG_DEBUG (ROLLLOG(debug))
#define ROLLLOG_INFO (ROLLLOG(info))
#define ROLLLOG_WARN (ROLLLOG(warn))
#define ROLLLOG_ERROR (ROLLLOG(error))

#define FUNC_ENTRY(in) (ROLLLOG(debug) << ">>>> Enter " << __FUNCTION__ << "() in(" << in << ")" << endl)
#define FUNC_EXIT(out, ret) (ROLLLOG(debug) << "<<<< Exit " << __FUNCTION__ << "() out[" << out << "], ret = " << ret << endl)

#define FDLOG_ERROR (FDLOG("error") << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|")
#define FDLOG_EXCEPT (FDLOG("except") << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|")

//读数据库日志
#define FDLOG_READ_ACCESS_INFO (FDLOG("read_access_info") << "|")
#define FDLOG_READ_PROCESS_INFO (FDLOG("read_process_info") << "|")

#define FDLOG_READ_CALLBACK_INFO (FDLOG("read_callback_info") << "|")
#define FDLOG_READ_CALLBACK_EXCEPTION_INFO (FDLOG("read_callback_exception_info") << "|")

//写数据日志
#define FDLOG_WRITE_ACCESS_INFO (FDLOG("write_access_info") << "|")
#define FDLOG_WRITE_PROCESS_INFO (FDLOG("write_process_info") << "|")
#define FDLOG_WRITE_BIN_LOG (FDLOG("write_bin_log") << "|")

//取最新自增id
#define FDLOG_GET_LAST_INSERT_ID_ACCESS_INFO (FDLOG("get_last_insert_id_access_info") << "|")
#define FDLOG_GET_LAST_INSERT_ID_PROCESS_INFO (FDLOG("get_last_insert_id_process_info") << "|")

//取用户注册信息
#define FDLOG_GET_USER_REGISTER_PROCESS_INFO (FDLOG("get_user_register_process_info") << "|")

//执行sql语句日志
#define FDLOG_EXCUTE_ACCESS_INFO (FDLOG("excute_access_info") << "|")
#define FDLOG_EXCUTE_PROCESS_INFO (FDLOG("excute_process_info") << "|")

//配置信息
#define FDLOG_CONFIG_INFO (FDLOG("config_info") << "|")
#define FDLOG_WRITE_CALLBACK_INFO (FDLOG("write_callback_info") << "|")
#define FDLOG_WRITE_CALLBACK_EXCEPTION_INFO (FDLOG("write_callback_exception_info") << "|")
#define FDLOG_WRITE_BIN_LOG (FDLOG("write_bin_log") << "|")
#define FDLOG_DELETE_CACHE_LOG (FDLOG("delete_cache_log") << "|")

//
#define COST_MS 50
//
#define SQLCOST_MS 10

#endif



