#ifndef ERROR_H
#define ERROR_H

#include <string.h>

extern void print_error_warning_msg(
    const char* prefix,
    const char* func,
    const char* file,
    int line,
    const char* format,
    ...
);

extern void print_error(
    const char* func,
    const char* file,
    int line,
    int error_code
);


extern void print_error_and_msg(
    const char* func,
    const char* file,
    int line,
    int error_code,
    const char* format,
    ...
); 

#endif // ERROR_H
