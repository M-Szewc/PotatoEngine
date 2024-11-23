#include <core/logger.h>

// TODO: Test
#include <platform/platform.h>

int main(void){
    PE_FATAL("A test message: %f", 3.14f);
    PE_ERROR("A test message: %f", 3.14f);
    PE_WARN("A test message: %f", 3.14f);
    PE_INFO("A test message: %f", 3.14f);
    PE_DEBUG("A test message: %f", 3.14f);
    PE_TRACE("A test message: %f", 3.14f);

    platform_state state;
    if(platform_startup(&state, "Sandbox test", 100, 100, 1280, 720)) {
        while(TRUE){
            platform_pump_messages(&state);
        }
    }
    platform_shutdown(&state);

    return 0;
}