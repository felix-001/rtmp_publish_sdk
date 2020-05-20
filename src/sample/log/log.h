/**
* @file log.h
* @author rigensen
* @brief 
* @date 二  5/19 12:01:23 2020
*/

#ifndef _LOG_H

#define xCONSOLE
#ifdef CONSOLE
    #define LOGI(fmt, args...) printf("%s:%d(%s) $ "fmt"", __FILENAME__, __LINE__, __FUNCTION__, ##args)
    #define LOGE(fmt, args...) printf("\e[0;31m%s:%d(%s)$ "fmt"\e[0m", __FILENAME__, __LINE__, __FUNCTION__, ##args)
#else
    #define LOGI(args...) log_out(__FILENAME__, __FUNCTION__, __LINE__, args)
    #define LOGE LOGI
#endif

extern int log_init(char *mqtt_host, char *port,  char *user, char *passwd);
extern void log_out(char *file, const char *function, int line, char *fmt, ...);

#define _LOG_H
#endif
