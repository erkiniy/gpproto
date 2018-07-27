//
// Created by ProDigital on 7/27/18.
//

#include "GPLogWrapperDarwin.h"
#include <stdio.h>
#include <stdarg.h>

void __gpproto_call_gplog(const char* format, ...) {
    va_list args;
    va_start(args, format);

    vprintf(format, args);
    printf("\n");

    va_end(args);
}
