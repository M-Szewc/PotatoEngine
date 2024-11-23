#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initialize_logging(){
    // TODO: create log file.
    return TRUE;
}

void shutdown_logging(){
    // TODO: cleanup logging.write queued entries.
}

void log_output(log_level level, const char* message, ...){
    const char* level_strings[6] = {"[FATAL]:", "[ERROR]:", "[WARN]:", "[INFO]:", "[DEBUG]:", "[TRACE]:"};
    b8 is_error = level < LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit o a single log entry, but...
    // DON'T DO THAT!
    const u32 msg_length = 32000;
    char out_message[msg_length];
    memset(out_message, 0, sizeof(out_message));
    i32 offset = sprintf(out_message,"%s", level_strings[level]);

    va_list arg_ptr;
    va_start(arg_ptr, message);
    offset += vsnprintf(out_message + offset, msg_length - offset, message, arg_ptr);
    va_end(arg_ptr);

    sprintf(out_message + offset, "\n");

    if (is_error) {
        platform_console_write(out_message, level);
    } else {
        platform_console_write_error(out_message, level);
    }
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line){
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}
