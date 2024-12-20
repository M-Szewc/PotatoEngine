#pragma once

#include "defines.h"
#include "core/asserts.h"

#include "renderer/renderer_types.inl"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                  \
    {                                   \
        PE_ASSERT(expr == VK_SUCCESS);  \
    }

typedef struct vulkan_buffer {
    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;


typedef struct vulkan_device{
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;

    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    VkQueue graphics_queue;     
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkCommandPool graphics_command_pool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format;
} vulkan_device;

typedef struct vulkan_image{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef enum vulkan_render_pass_state {
    NOT_ALLOCATED,
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED
} vulkan_render_pass_state;

typedef struct vulkan_render_pass{
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;

    f32 depth;
    u32 stencil;

    vulkan_render_pass_state state;
} vulkan_render_pass;

typedef struct vulkan_frame_buffer{
    VkFramebuffer handle;
    u32 attachment_count;
    VkImageView* attachments;
    vulkan_render_pass* render_pass;
} vulkan_frame_buffer;

typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    VkImage* images;
    VkImageView* views;

    vulkan_image depth_attachment;

    // darray of framebuffers used for on-screen rendering
    vulkan_frame_buffer* frame_buffers;
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;

    // Command buffer state
    vulkan_command_buffer_state state;
} vulkan_command_buffer;

typedef struct vulkan_fence {
    VkFence handle;
    b8 is_signaled;
} vulkan_fence;

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
} vulkan_pipeline;  

#define OBJECT_SHADER_STAGE_COUNT 2

typedef struct vulkan_object_shader {
    // vertex, fragment
    vulkan_shader_stage stages[OBJECT_SHADER_STAGE_COUNT];

    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;

    // One edscriptor set per frame - max 3 for triple buffering.
    VkDescriptorSet global_descriptor_sets[3];
    b8 descriptor_updated[3];

    // Global uniform object
    global_uniform_object global_ubo;

    // Global uniform buffer
    vulkan_buffer global_uniform_buffer;
    
    vulkan_pipeline pipeline;

} vulkan_object_shader;

typedef struct vulkan_context {

    // The frame buffer's current dimensions
    u32 frame_buffer_width;
    u32 frame_buffer_height;

    // Current generation of frame buffer size. If it does not match frame_buffer_size_last_generation,
    // a new one should be generated
    u64 frame_buffer_size_generation;

    // The generation of the frame buffer, when it was last created. Set to frame_buffer_size_generation
    // when updated.
    u64 frame_buffer_size_last_generation;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    vulkan_device device;

    vulkan_swapchain swapchain;
    vulkan_render_pass main_render_pass;

    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;

    u64 geometry_vertex_offset;
    u64 geometry_index_offset;

    // darray
    vulkan_command_buffer* graphics_command_buffers;

    // darray
    VkSemaphore* image_available_semaphores;

    // darray
    VkSemaphore* queue_complete_semaphores;
    
    u32 in_flight_fence_count;
    // darray
    vulkan_fence* in_flight_fences;

    // Holds pointers to fences which exist and are owned elsewhere
    vulkan_fence** images_in_flight;

    u32 image_index;
    u32 current_frame;

    b8 recreating_swapchain;

    vulkan_object_shader object_shader;

    i32 (*find_memory_index)(u32 type_filter, u32 proprtty_flags);

} vulkan_context;