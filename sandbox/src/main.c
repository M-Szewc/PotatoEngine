#include <core/logger.h>

#include <core/asserts.h>

int main(void){
    PE_FATAL("A test message: %f", 3.14f);
    PE_ERROR("A test message: %f", 3.14f);
    PE_WARN("A test message: %f", 3.14f);
    PE_INFO("A test message: %f", 3.14f);
    PE_DEBUG("A test message: %f", 3.14f);
    PE_TRACE("A test message: %f", 3.14f);

    PE_ASSERT_MSG(1==2, "not possible");

    return 0;
}