#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct logger_system_state {
    b8 initialized;
} logger_system_state;

static logger_system_state* state_ptr;

b8 initialize_logging(u64* memory_requirement, void* state){
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;
    state_ptr->initialized = true;

    // TODO: Remove this
    PE_FATAL("A test message: %f", 3.14f);
    PE_ERROR("A test message: %f", 3.14f);
    PE_WARN("A test message: %f", 3.14f);
    PE_INFO("A test message: %f", 3.14f);
    PE_DEBUG("A test message: %f", 3.14f);
    PE_TRACE("A test message: %f", 3.14f);

    // TODO: create log file.
    return true;
}

void shutdown_logging(void* state){
    // TODO: cleanup logging.write queued entries.
    state_ptr = 0;
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
