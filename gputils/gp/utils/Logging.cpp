//
// Created by ProDigital on 7/27/18.
//

#include "Logging.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

FILE* gplogFile = NULL;

void gp_log_file_set_path(const char* path) {

    if (gplogFile) {
        fclose(gplogFile);
        gplogFile = NULL;
    }

    if ((int)strlen(path)) {
        gplogFile = fopen(path, "a");
        gp_log_file_write_header(gplogFile);
    }
    else {
        gplogFile = NULL;
    }
}

void gp_log_file_printf(char level, const char* msg, ...) {

    if (!gplogFile)
        return;

    va_list argptr;
    va_start(argptr, msg);
    time_t t = time(0);

    struct tm *now = localtime(&t);
    fprintf(gplogFile, "%02d-%02d %02d:%02d:%02d %c: ", now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, level);
    vfprintf(gplogFile, msg, argptr);
    fprintf(gplogFile, "\n");
    fflush(gplogFile);
}

void gp_log_file_write_header(FILE* file) {

}