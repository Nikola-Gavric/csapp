
#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include "csapp.h"


#ifdef __cplusplus
extern "C" {
#endif

#define MENU_STARS  "**********************************"
    
#define LOG_FILENAME    "fileSenderServer.log"
#define LOG_SEM_NAME    "serv_sem_mutex"
 
    
extern FILE *LogFptr;
extern sem_t *gLogMutex;

#define printServerLog(...) {\
            P(gLogMutex);\
            fprintf(LogFptr, __VA_ARGS__);\
            printf(__VA_ARGS__);\
            V(gLogMutex);\
           }

#define MAX_FRMT_TIME   32


typedef struct serverTimeData{
    time_t brutTime;
    struct tm *timeInfo;
    char frmtBuf[MAX_FRMT_TIME];
} serverTimeData;


/**
 * To display the log file content
 */
void display_log_content();

/**
 * Init the login mechanism
 * @param isClearLogs if the logs need to be cleared.
 */
void init_logger(unsigned char isClearLogs);

/**
 * Build and show the menu
 */
void logger_menu();



#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */

