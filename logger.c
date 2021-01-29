
#include <stdio.h>
#include "logger.h"


FILE *LogFptr = NULL;

void display_log_content() {
    char tmpCh;

    tmpCh = fgetc(LogFptr);
    while (tmpCh != EOF) {
        printf("%c", tmpCh);
        tmpCh = fgetc(LogFptr);
    }
}

void init_logger(unsigned char isClearLogs) {
    if (isClearLogs)
        LogFptr = fopen(LOG_FILENAME, "w"); // w: open for writing (creates file if it doesn't exist)
    else
        LogFptr = fopen(LOG_FILENAME, "a+"); // a+: open for reading and writing (append if file exists)

    if (LogFptr == NULL) {
        fprintf(stderr, "ERROR, could not create or open " LOG_FILENAME "\n");
        exit(1);
    }

    if (isClearLogs) {
        printf("log history cleaned up\n");
    } else {
        display_log_content();
    }
}

static int menu(void) {
    int ret, result;
    printf("\n%s%s\n", MENU_STARS, MENU_STARS);
    printf("Enter a numeric value to select an option for logs history:\n");
    printf("1) View\n");
    printf("2) Delete\n");

    printf("%s%s\n", MENU_STARS, MENU_STARS);

    printf("=> ");
    while ((result = scanf("%d", &ret)) != 1 || (ret < 1 || ret > 2)) {
        if (result != 1) {
            scanf("%*s");
        }

        printf("select a value between 1 and 2, please.\n=> ");
    }
    return ret;

}

void logger_menu() {
    int code;
    unsigned char isCleanAsked = 0, doLoop = 1;

    do {
        code = menu();

        switch (code) {
            case 1 :
                doLoop = 0;
                break;

            case 2 :
                isCleanAsked = 1;
                doLoop = 0;
                break;

            default:
                printf("Oops!\n");
                break;
        }

    } while (doLoop);

    init_logger(isCleanAsked);

    printf("Server ready to receive\n");
}
