/***************************************************************************
 * 
 * Copyright (c) 2007 feimat.com, Inc. All Rights Reserved
 * $Id: dep_conf.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_conf_dep.h
 * @author feimat(com@feimat.com)
 * @date 2007/12/20 11:51:26
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __UL_CONF_DEP_H_
#define  __UL_CONF_DEP_H_

typedef struct Ul_confitem Ul_confitem;
typedef struct Ul_confdata Ul_confdata;
typedef struct Ul_confdata ul_confdata_t;
typedef struct arg_option arg_option;
typedef struct base_arg base_arg;
typedef struct muti_option muti_option;
typedef struct Ul_argu_stru Ul_argu_stru;

/**
 * 隐藏readnum系列的原因是，配置文件居然从conf.n读取，很奇怪的行为
 */

/**
 * 直接从配置文件中获取一个配置项的值, 值为int 类型
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，0没有找到
 */
int ul_readnum_oneint(const char *work_path, const char *fname, char *vname, int *pvalue);


/**
 * 直接从配置文件中获取一个配置项的值, 值为unsigned int 类型
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，0没有找到
 */
int ul_readnum_oneuint(const char *work_path, const char *fname, char *vname, unsigned int *pvalue);


/**
 * 直接从配置文件中获取一个配置项的值, 值为long long类型
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，0没有找到
 */
int ul_readnum_oneint64(const char *work_path, const char *fname, char *vname, long long *pvalue);

/**
 * 直接从配置文件中获取一个配置项的值, 值为unsigned long long类型
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，0没有找到
 */
int ul_readnum_oneuint64(const char *work_path, const char *fname, char *vname,
						 unsigned long long *pvalue);

/**
 * 在配置信息结构中读取配置项的值，值为char*类型
 * 因为没有判断字符串的长度，所以不安全
 *
 * @param pd_conf 结构指针
 * @param conf_name 配置项名
 * @param[out] conf_value 配置项的值
 * @return 1成功，0没有找到
 */
int ul_getconfstr(ul_confdata_t * pd_conf, char *conf_name, char *conf_value);

/**
 * 直接从配置文件中获取一个配置项的值, 值为int 类型. 如果没有相应的配置项, 会打印日志
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，-1没有找到
 */
int ul_kreadnum_oneint(const char *work_path, const char *fname, char *vname, int *pvalue);


/**
 * 直接从配置文件中获取一个配置项的值, 值为float 类型. 如果没有相应的配置项, 会打印日志
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，-1没有找到
 */
int ul_kreadnum_onefloat(const char *work_path, const char *fname, char *vname, float *pvalue);


/** 
 * 初始化配置文件,如果文件以及存在,会将其清空
 *
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @return >=0 成功 <0 失败
 */
int ul_writenum_init(const char *work_path, const char *fname);

/**
 * 将一个配置项写到配置文件中, 值为int类型
 *
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param value 配置项的值
 * @return >=0 成功 <0 失败
 */
int ul_writenum_oneint(const char *work_path, const char *fname, const char *vname, int value);

/**
 * 直接从配置文件中获取一个配置项的值, 值为u_int 类型. 如果没有相应的配置项, 会打印日志
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，-1没有找到
 */
int ul_kreadnum_oneuint(const char *work_path, const char *fname, char *vname, u_int *pvalue);

/**
 * 将一个配置项写到配置文件中, 值为u_int类型
 *
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param value 配置项的值
 * @return >=0 成功 <0 失败
 */
int ul_writenum_oneuint(const char *work_path, const char *fname, const char *vname, u_int value);

/**
 * 直接从配置文件中获取一个配置项的值, 值为int64类型. 如果没有相应的配置项, 会打印日志
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，-1没有找到
 */
int ul_kreadnum_oneint64(const char *work_path, const char *fname, char *vname, long long *pvalue);

/**
 * 将一个配置项写到配置文件中, 值为int64类型
 *
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param value 配置项的值
 * @return >=0 成功 <0 失败
 */
int ul_writenum_oneint64(const char *work_path, const char *fname, const char *vname, long long value);


/**
 * 直接从配置文件中获取一个配置项的值, 值为unsigned long long类型. 如果没有相应的配置项, 会打印日志
 * 
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param[out] pvalue 配置项的值
 * @return 1成功，-1没有找到
 */
int ul_kreadnum_oneuint64(const char *work_path, const char *fname, char *vname, unsigned long long *pvalue);

/**
 * 将一个配置项写到配置文件中, 值为unsigned long long类型
 *
 * @param work_path 配置文件路径
 * @param fname 配置文件名
 * @param vname 配置项名
 * @param value 配置项的值
 * @return >=0 成功 <0 失败
 */
int ul_writenum_oneuint64(const char *work_path, const char *fname, const char *vname, unsigned long long value);














#endif  //__UL_CONF_DEP_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
