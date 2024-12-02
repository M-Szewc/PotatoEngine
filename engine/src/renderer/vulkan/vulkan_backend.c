#include "vulkan_backend.h"

#include "vulkan_types.inl"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_render_pass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_frame_buffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/pe_string.h"
#include "core/pe_memory.h"
#include "core/application.h"

#include "containers/darray.h"

#include "platform/platform.h"

// Static vulkan context
static vulkan_context context;
static u32 cached_frame_buffer_width = 0;
static u32 cached_frame_buffer_height = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

i32 find_memory_index(u32 type_filter, u32 property_flags);

void create_command_buffers(renderer_backend* backend);
void regenerate_frame_buffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_render_pass* render_pass);
b8 recreate_swapchain(renderer_backend* backend);

b8 vulkan_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    
    // Function pointers
    context.find_memory_index = find_memory_index;

    // TODO: custom allocator
    context.allocator = 0;

    
    application_get_frame_buffer_size(&cached_frame_buffer_width, &cached_frame_buffer_height);
    context.frame_buffer_width = (cached_frame_buffer_width != 0) ? cached_frame_buffer_width : 800;
    context.frame_buffer_height = (cached_frame_buffer_height != 0) ? cached_frame_buffer_height : 600;
    cached_frame_buffer_width = 0;
    cached_frame_buffer_height = 0;

    // Setup Vulkan instance
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Potato Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    // Obtain a list of required extensions
    const char** required_extensions = darray_create(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);   // Generic surface extension
    platform_get_required_extension_names(&required_extensions);        // Platform-specifin extension
    #if defined(_DEBUG)
        darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug utillities
    
        PE_DEBUG("Required extensions");
        u32 length = darray_length(required_extensions);
        for (u32 i = 0; i < length; ++i) {
            PE_DEBUG(required_extensions[i]);
        } 
    #endif

    create_info.enabledExtensionCount = darray_length(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    // Validation layers
    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

    // If validation should be done, get a list of the required validation layer names
    // and make sure they exist. Validation layers should only be enabled on non-release builds.
    #if defined(_DEBUG)
        PE_INFO("Validation layers enabled. Enumerating...");

        // The list of validation layers required
        required_validation_layer_names = darray_create(const char*);
        darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
        required_validation_layer_count = darray_length(required_validation_layer_names);
        
        // Obtain a list of available validation layers
        u32 available_layer_count = 0;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
        VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

        // Verify all required layers are available
        for (u32 i = 0; i < required_validation_layer_count; ++i) {
            PE_INFO("Searching for layer: %s...", required_validation_layer_names[i]);
            b8 found = false;
            for (u32 j = 0; j < available_layer_count; ++j) {
                if (strings_equal(required_validation_layer_names[i], available_layers[j].layerName)) {
                    found = true;
                    PE_INFO("Found");
                    break;
                }
            }

            if (!found) {
                PE_FATAL("Required validation layer is missing: %s", required_validation_layer_names[i]);
            }
        }
        PE_INFO("All required validation layers are present");
    #endif

    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    PE_INFO("Vulkan Instance created.");

    // Debugger
    #if defined(_DEBUG)
        PE_DEBUG("Creating Vulkan debugger...");
        u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debug_create_info.messageSeverity = log_severity;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        debug_create_info.pfnUserCallback = vk_debug_callback;

        PFN_vkCreateDebugUtilsMessengerEXT func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
        PE_ASSERT_MSG(func, "Failed to create debug messenger!");
        VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
        PE_DEBUG("Vulkan debugger created.");
    #endif

    // Surface creation
    PE_DEBUG("Creating Vulkan surface...");
    if (!platform_create_vulkan_surface(plat_state, &context)){
        PE_ERROR("Failed to create platform surface!");
        return false;
    }

    // Device creation
    if (!vulkan_device_create(&context)){
        PE_ERROR("Failed to create device!");
        return false;
    }

    // Swapchain creation
    vulkan_swapchain_create(
        &context,
        context.frame_buffer_width,
        context.frame_buffer_height,
        &context.swapchain
    );

    // Render pass
    vulkan_render_pass_create(
        &context,
        &context.main_render_pass,
        0, 0, context.frame_buffer_width, context.frame_buffer_height,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0
    );

    // Swapchain frame buffers
    context.swapchain.frame_buffers = darray_reserve(vulkan_frame_buffer, context.swapchain.image_count);
    regenerate_frame_buffers(backend, &context.swapchain, &context.main_render_pass);

    // Create command buffers
    create_command_buffers(backend);

    // Create sync objects
    context.image_available_semaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.queue_complete_semaphores = darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
    context.in_flight_fences = darray_reserve(vulkan_fence, context.swapchain.max_frames_in_flight);

    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.image_available_semaphores[i]);
        vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.queue_complete_semaphores[i]);

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Actual fences are not owned
    // by the list.
    context.images_in_flight = darray_reserve(vulkan_fence, context.swapchain.image_count);
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_flight[i] = 0;
    }
    PE_INFO("Vulkan sync objects created");



    PE_INFO("Vulkan renderer initialized successfully.");
    return true;

}

void vulkan_renderer_backend_shutdown(renderer_backend* backend) {
    // wait for all processes to end
    vkDeviceWaitIdle(context.device.logical_device);
    
    // Destroy in the opposite order of creation

    // Sync objects
    for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
        if (context.image_available_semaphores[i]) {
            vkDestroySemaphore(
                context.device.logical_device,
                context.image_available_semaphores[i],
                context.allocator
            );
            context.image_available_semaphores[i] = 0;
        }
        if (context.queue_complete_semaphores[i]) {
            vkDestroySemaphore(
                context.device.logical_device,
                context.queue_complete_semaphores[i],
                context.allocator
            );
            context.queue_complete_semaphores[i] = 0;
        }
        vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
    }
    darray_destroy(context.image_available_semaphores);
    context.image_available_semaphores = 0;

    darray_destroy(context.queue_complete_semaphores);
    context.queue_complete_semaphores = 0;

    darray_destroy(context.in_flight_fences);
    context.in_flight_fences = 0;

    darray_destroy(context.images_in_flight);
    context.images_in_flight = 0;


    // Command buffers
    for (u32 i = 0; i < context.swapchain.image_count; i++) {
        if (context.graphics_command_buffers[i].handle) {
            vulkan_command_buffer_free(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]
            );
            context.graphics_command_buffers[i].handle = 0;
        }
    }
    darray_destroy(context.graphics_command_buffers);
    context.graphics_command_buffers = 0;

    // Frame buffers
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_frame_buffer_destroy(&context, &context.swapchain.frame_buffers[i]);
    }

    // Render pass
    vulkan_render_pass_destroy(&context, &context.main_render_pass);

    // Swapchain
    vulkan_swapchain_destroy(&context, &context.swapchain);

    PE_DEBUG("Destroying Vulkan device...");
    vulkan_device_destroy(&context);

    PE_DEBUG("Destroying Vulkan surface...");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

    #if defined(_DEBUG)
        PE_DEBUG("Destroying Vulkan debugger...");
        if (context.debug_messenger) {
            PFN_vkDestroyDebugUtilsMessengerEXT func =
                (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
            func(context.instance, context.debug_messenger, context.allocator);
        }
    #endif

    PE_DEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_begin_on_resized(renderer_backend* backend, u16 width, u16 height) {
    // Update the "frame buffer size generation", a counter which indicates when the
    // frame buffer size has been updates
    cached_frame_buffer_width = width;
    cached_frame_buffer_height = height;
    context.frame_buffer_size_generation++;

    PE_INFO("Vulkan renderer backend resized w:%i h: %i gen:%llu", width, height, context.frame_buffer_size_generation);
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time){
    vulkan_device* device = &context.device;

    // Check if recreating swap chain and boot out
    if (context.recreating_swapchain) {
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(result)) {
            PE_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: '%s'", vulkan_result_string(result, true));
            return false;
        }
        PE_INFO("Recreating swapchain, booting.");
        return false;
    }

    // Check if the frame buffer has been resized
    if (context.frame_buffer_size_generation != context.frame_buffer_size_last_generation) {
        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(result)) {
            PE_ERROR("vulkan_renderer_backend_begin_frame (2) failed: '%s'", vulkan_result_string(result, true));
            return false;
        }

        // If the swapchain recreation failed (because, for example the window was minimized),
        // boot out before unsetting the flag
        if (!recreate_swapchain(backend)) {
            return false;
        }

        PE_INFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on
    if (!vulkan_fence_wait(
        &context,
        &context.in_flight_fences[context.current_frame],
        U64MAX
    )) {
        PE_WARN("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
    /// The same semaphore will later be waited on by the queue submission to ensure this image is available
    if (!vulkan_swapchain_acquire_next_image_index(
        &context,
        &context.swapchain,
        U64MAX,
        context.image_available_semaphores[context.current_frame],
        0,
        &context.image_index
    )) {
        return false;
    }

    // Begin recording commands
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];
    vulkan_command_buffer_reset(command_buffer);
    vulkan_command_buffer_begin(command_buffer, false, false, false);

    // Dynamic state
    VkViewport viewport;
    viewport.x= 0.0f;
    viewport.y = (f32)context.frame_buffer_height;
    viewport.width = (f32)context.frame_buffer_width;
    viewport.height = -(f32)context.frame_buffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.frame_buffer_width;
    scissor.extent.height = context.frame_buffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_render_pass.w = context.frame_buffer_width;
    context.main_render_pass.h = context.frame_buffer_height;

    // Begin the render pass
    vulkan_render_pass_begin(
        command_buffer,
        &context.main_render_pass,
        context.swapchain.frame_buffers[context.image_index].handle
    );

    return true;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time){
    vulkan_command_buffer* command_buffer = &context.graphics_command_buffers[context.image_index];

    // End render pass
    vulkan_render_pass_end(command_buffer, &context.main_render_pass);

    vulkan_command_buffer_end(command_buffer);

    // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
    if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE) { // was frame
        vulkan_fence_wait(
            &context,
            context.images_in_flight[context.image_index],
            U64MAX
        );
    }

    // Mark the image fence as in-use by this frame
    context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

    // Reset the fence for use on the next frame
    vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

    // Submit the queue and wait for the operation to complete
    // Begin queue submission
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    
    // Command buffer(s) to be executed
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    // The semaphore(s) to be signaled when the queue is complete
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queue_complete_semaphores[context.current_frame];

    // Wait semaphore ensures that the operation cannot begin until the image is available
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.image_available_semaphores[context.current_frame];

    // Each semaphore waits on the corresponding pipeline stage to complete, 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphics_queue,
        1,
        &submit_info,
        context.in_flight_fences[context.current_frame].handle
    );
    if (result != VK_SUCCESS) {
        PE_ERROR("vkQueueSubmit failed with result: '%s'", vulkan_result_string(result, true));
        return false;
    }

    // End queue submission
    vulkan_command_buffer_update_submitted(command_buffer);
    
    

    // Give the image back to the swap chain
    vulkan_swapchain_present(
        &context,
        &context.swapchain,
        context.device.graphics_queue,
        context.device.present_queue,
        context.queue_complete_semaphores[context.current_frame],
        context.image_index
    );

    return true;
}


VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            PE_ERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            PE_WARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            PE_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            PE_TRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        // Check each memory type to see if its bit is set to 1
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }

    PE_WARN("Unable to find suitable memory type!");
    return -1;
}

void create_command_buffers(renderer_backend* backend) {
    if (!context.graphics_command_buffers) {
        context.graphics_command_buffers = darray_reserve(vulkan_command_buffer, context.swapchain.image_count);
        for (u32 i = 0; i < context.swapchain.image_count; ++i) {
            pe_zero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        if (context.graphics_command_buffers[i].handle) {
            vulkan_command_buffer_free(
                &context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]
            );
        }
        pe_zero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
        vulkan_command_buffer_allocate(
            &context,
            context.device.graphics_command_pool,
            true,
            &context.graphics_command_buffers[i]
        );
    }

    PE_INFO("Vulkan command buffers created.");
}


void regenerate_frame_buffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_render_pass* render_pass) {
    for (u32 i = 0; i < swapchain->image_count; ++i) {
        // TODO: make this dynamic based on the currently configured attachments
        u32 attachment_count = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depth_attachment.view
        };

        vulkan_frame_buffer_create(
            &context,
            render_pass,
            context.frame_buffer_width,
            context.frame_buffer_height,
            attachment_count,
            attachments,
            &swapchain->frame_buffers[i]
        );
    }
}

b8 recreate_swapchain(renderer_backend* backend) {
    // If already being recreated, do not try again
    if (context.recreating_swapchain) {
        PE_DEBUG("recreate_swapchain called when already recreating. Booting.");
        return false;
    }

    // Detect if the window is too small to be drawn to
    if (context.frame_buffer_width == 0 || context.frame_buffer_height == 0) {
        PE_DEBUG("recreate_swapchain called when window is < 1 in a dimension. Booting");
        return false;
    }

    // Mark as recreating if the dimensions are valid
    context.recreating_swapchain = true;

    // Wait for any operations to complete
    vkDeviceWaitIdle(context.device.logical_device);

    // Clear these just in case
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_flight[i] = 0;
    }

    // Requery support
    vulkan_device_query_swapchain_support(
        context.device.physical_device,
        context.surface,
        &context.device.swapchain_support
    );
    vulkan_device_detect_depth_format(&context.device);

    // Recreate swapchain
    vulkan_swapchain_recreate(
        &context,
        cached_frame_buffer_width,
        cached_frame_buffer_height,
        &context.swapchain
    );

    // Sync the framebuffer size with the cached sizes
    context.frame_buffer_width = cached_frame_buffer_width;
    context.frame_buffer_height = cached_frame_buffer_height;
    context.main_render_pass.w = context.frame_buffer_width;
    context.main_render_pass.h = context.frame_buffer_height;
    cached_frame_buffer_width = 0;
    cached_frame_buffer_height = 0;

    // Update frame buffer size generation
    context.frame_buffer_size_last_generation = context.frame_buffer_size_generation;

    // Cleanup swapchain
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
    }

    // Frame buffers
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_frame_buffer_destroy(&context, &context.swapchain.frame_buffers[i]);
    }

    context.main_render_pass.x = 0;
    context.main_render_pass.y = 0;
    context.main_render_pass.w = context.frame_buffer_width;
    context.main_render_pass.h = context.frame_buffer_height;

    regenerate_frame_buffers(backend, &context.swapchain, &context.main_render_pass);

    create_command_buffers(backend);

    // Clear the recreating flag
    context.recreating_swapchain = false;

    return true;
}