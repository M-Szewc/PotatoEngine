#include "logger.h"
#include "asserts.h"

#include "platform/platform.h"
#include "platform/filesystem.h"

#include "core/pe_string.h"
#include "core/pe_memory.h"

#include <stdarg.h>

typedef struct logger_system_state {
    file_handle log_file_handle;
} logger_system_state;

static logger_system_state* state_ptr;

void append_to_log_file(const char* message) {
    // TODO: why is_valid state of file_handle changes?
    if (state_ptr && state_ptr->log_file_handle.handle) {
    //if (state_ptr && state_ptr->log_file_handle.is_valid) {

        // since the message already contains a '\n', just write the bytes directly.
        u64 length = string_length(message);
        u64 written = 0;
        if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written)) {
            platform_console_write_error("[ERROR]: writing to console.log.", LOG_LEVEL_ERROR);
        }
    }
}

b8 logging_system_initialize(u64* memory_requirement, void* state){
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;
    
    // Create new/wipe existing log file, then open it.
    if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
        platform_console_write_error("[ERROR]: writing to console.log.", LOG_LEVEL_ERROR);
        return false;
    }

    // TODO: Remove this
    PE_FATAL("A test message: %f", 3.14f);
    PE_ERROR("A test message: %f", 3.14f);
    PE_WARN("A test message: %f", 3.14f);
    PE_INFO("A test message: %f", 3.14f);
    PE_DEBUG("A test message: %f", 3.14f);
    PE_TRACE("A test message: %f", 3.14f);

    return true;
}

void logging_system_shutdown(void* state){
    log_output(LOG_LEVEL_INFO, "Shutting down logger...");
    // TODO: cleanup logging.write queued entries.
    filesystem_close(&state_ptr->log_file_handle);

    state_ptr = 0;
}

void log_output(log_level level, const char* message, ...){
    // TODO: These string operations are pretty slow. This needs to be moved to another thread
    // eventually along with the file writes to avoid slowing things down
    const char* level_strings[6] = {"[FATAL]:", "[ERROR]:", "[WARN]:", "[INFO]:", "[DEBUG]:", "[TRACE]:"};
    b8 is_error = level < LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit on a single log entry, but ...
    // DON'T DO THAT
    const u32 LOG_MAX_LENGTH = 32000;
    char out_message[LOG_MAX_LENGTH];
    pe_zero_memory(out_message, sizeof(out_message));

    // Format original message
    va_list arg_ptr;
    va_start(arg_ptr, message);
    string_format_v(out_message, message, arg_ptr);
    va_end(arg_ptr);

    // Prepend log level to message
    string_format(out_message, "%s%s\n", level_strings[level], out_message);

    // Print accordingly
    if (is_error) {
        platform_console_write_error(out_message, level);
    } else {
        platform_console_write(out_message, level);
    }

    // Queue a copy to be written to the log file
    append_to_log_file(out_message);
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line){
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}
