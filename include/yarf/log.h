/* Copyright (c) 2016, Johnyao 
 * All rights reserved.
 *
 * log.h
 *
 *  Created on: 2016年5月20日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_LOG_H_
#define YARF_LOG_H_


#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/syscall.h>


#define gettid() syscall(__NR_gettid)

#define RAW_LOG(level, format, args...) {\
  const char* base_name =  strrchr(__FILE__, '/');\
  printf(level"|%ld|%s:%d:%s|" format "\n", gettid(), \
    base_name?base_name+1:__FILE__  , __LINE__, __func__, ##args);\
}\

//////////////////////////////////////////////////////////////////////////////////////
#define LIB4G_ERR_LOG(format, args...) RAW_LOG("ERROR", format, ##args);

#define LIB4G_WARN_LOG(format, args...) RAW_LOG("WARN", format, ##args);

#define LIB4G_INFO_LOG(format, args...) RAW_LOG("INFO", format, ##args);

#define LIB4G_DEBUG_LOG(format, args...) RAW_LOG("DEBUG", format, ##args);

#define LIB4G_TRACE_LOG(format, args...) RAW_LOG("TRACE", format, ##args);

//////////////////////////////////////////////////////////////////////////////////////
#define ERR_LOG(format, args...) RAW_LOG("ERROR", format, ##args);

#define WARN_LOG(format, args...) RAW_LOG("WARN", format, ##args);

#define INFO_LOG(format, args...) RAW_LOG("INFO", format, ##args);

#define DEBUG_LOG(format, args...) RAW_LOG("DEBUG", format, ##args);

#define TRACE_LOG(format, args...) RAW_LOG("TRACE", format, ##args);


#endif  // YARF_LOG_H_

