//
// Created by ProDigital on 7/27/18.
//

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <cmath>

#include "gp/utils/Logging.h"

FILE* gplogFile = NULL;

void gp_log_file_set_path(const char* path) {

    if (gplogFile) {
        fclose(gplogFile);
        gplogFile = NULL;
    }

    if ((int)strlen(path)) {
        gplogFile = fopen(path, "a");
        gp_log_file_write_header(gplogFile);
        printf("Setting log file with path %s\n", path);
    }
    else {
        gplogFile = NULL;
    }
}

void gp_log_file_write_header(FILE* file) {

}

void gp_log_file_printf(char level, const char* msg, ...) {

    if (!gplogFile)
        return;

    va_list argptr;
    va_start(argptr, msg);
    time_t t = time(nullptr);

    struct tm *now = localtime(&t);
    fprintf(gplogFile, "%02d-%02d %02d:%02d:%02d %c: ", now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, level);
    vfprintf(gplogFile, msg, argptr);
    fprintf(gplogFile, "\n");
    fflush(gplogFile);
}

void gp_log_printf(char level, const char* msg, ...) {

    va_list argptr;
    va_start(argptr, msg);
    time_t t = time(nullptr);

    long ms;
    time_t s;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    ms = round(spec.tv_nsec / 1.0e6);

    if (ms > 999)
        ms = 0;

    struct tm *now = localtime(&t);
    printf("gpproto: ");
    printf("%02d-%02d %02d:%02d:%02d:%03d %c: ", now->tm_mday, now->tm_mon + 1, now->tm_hour, now->tm_min, now->tm_sec, (int)ms, level);
    vprintf(msg, argptr);
    printf("\n");
}