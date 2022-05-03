#include "runtime/function/render/include/render/vulkan_manager/vulkan_common.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_mesh.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_misc.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_passes.h"
#include "runtime/function/render/include/render/vulkan_manager/vulkan_util.h"

//#include <post_process_vert.h>
#include <blur_vert.h>
#include <blur_frag.h>

#include<windows.h>

namespace Pilot
{
    void PBlurPass::initialize(VkRenderPass render_pass, VkImageView input_attachment)
    {
        _framebuffer.render_pass = render_pass;
        setupDescriptorSetLayout();
        setupPipelines();
        setupDescriptorSet();
        updateAfterFramebufferRecreate(input_attachment);
    }

    void PBlurPass::setupDescriptorSetLayout()
    {
        _descriptor_infos.resize(1);

        VkDescriptorSetLayoutBinding post_process_global_layout_bindings[3] = {};

        VkDescriptorSetLayoutBinding& post_process_global_layout_input_attachment_binding =
            post_process_global_layout_bindings[0];
        post_process_global_layout_input_attachment_binding.binding         = 0;
        post_process_global_layout_input_attachment_binding.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        post_process_global_layout_input_attachment_binding.descriptorCount = 1;
        post_process_global_layout_input_attachment_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        //add Infinite Tsukuyomi texture
        VkDescriptorSetLayoutBinding& post_process_global_layout_infinite_tsukuyomi_binding = post_process_global_layout_bindings[1];
        post_process_global_layout_infinite_tsukuyomi_binding.binding         = 1;
        post_process_global_layout_infinite_tsukuyomi_binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        post_process_global_layout_infinite_tsukuyomi_binding.descriptorCount = 1;
        post_process_global_layout_infinite_tsukuyomi_binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        //add ubo bing
        VkDescriptorSetLayoutBinding& post_process_global_layout_storage_buffer_binding =
            post_process_global_layout_bindings[2];
        post_process_global_layout_storage_buffer_binding.binding           = 2;
        post_process_global_layout_storage_buffer_binding.descriptorType    = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        post_process_global_layout_storage_buffer_binding.descriptorCount   = 1;
        post_process_global_layout_storage_buffer_binding.stageFlags        = VK_SHADER_STAGE_FRAGMENT_BIT;
        post_process_global_layout_storage_buffer_binding.pImmutableSamplers = NULL;
        

        VkDescriptorSetLayoutCreateInfo post_process_global_layout_create_info;
        post_process_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        post_process_global_layout_create_info.pNext = NULL;
        post_process_global_layout_create_info.flags = 0;
        post_process_global_layout_create_info.bindingCount =
            sizeof(post_process_global_layout_bindings) / sizeof(post_process_global_layout_bindings[0]);
        post_process_global_layout_create_info.pBindings = post_process_global_layout_bindings;

        if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_p_vulkan_context->_device,
                                                      &post_process_global_layout_create_info,
                                                      NULL,
                                                      &_descriptor_infos[0].layout))
        {
            throw std::runtime_error("create post process global layout");
        }
    }
    void PBlurPass::setupPipelines()
    {
        _render_pipelines.resize(1);

        VkDescriptorSetLayout      descriptorset_layouts[1] = {_descriptor_infos[0].layout};
        VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
        pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts    = descriptorset_layouts;

        if (vkCreatePipelineLayout(
                m_p_vulkan_context->_device, &pipeline_layout_create_info, nullptr, &_render_pipelines[0].layout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("create post process pipeline layout");
        }

        VkShaderModule vert_shader_module =
            PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, BLUR_VERT);
        VkShaderModule frag_shader_module =
            PVulkanUtil::createShaderModule(m_p_vulkan_context->_device, BLUR_FRAG);

        VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
        vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;
        vert_pipeline_shader_stage_create_info.pName  = "main";

        VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
        frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName  = "main";

        VkPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                           frag_pipeline_shader_stage_create_info};

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
        vertex_input_state_create_info.pVertexBindingDescriptions      = NULL;
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
        vertex_input_state_create_info.pVertexAttributeDescriptions    = NULL;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
        input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state_create_info {};
        viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports    = &m_command_info._viewport;
        viewport_state_create_info.scissorCount  = 1;
        viewport_state_create_info.pScissors     = &m_command_info._scissor;

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
        rasterization_state_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth               = 1.0f;
        rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp          = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
        multisample_state_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state {};
        color_blend_attachment_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        //color_blend_attachment_state.blendEnable         = VK_FALSE;
        //color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        //color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        //color_blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;
        //color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        //color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        //color_blend_attachment_state.alphaBlendOp        = VK_BLEND_OP_ADD;
        color_blend_attachment_state.blendEnable         = VK_TRUE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.alphaBlendOp        = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
        color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable     = VK_FALSE;
        color_blend_state_create_info.logicOp           = VK_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount   = 1;
        color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info {};
        depth_stencil_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable       = VK_TRUE;
        depth_stencil_create_info.depthWriteEnable      = VK_TRUE;
        depth_stencil_create_info.depthCompareOp        = VK_COMPARE_OP_LESS;
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable     = VK_FALSE;

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
        dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates    = dynamic_states;

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shader_stages;
        pipelineInfo.pVertexInputState   = &vertex_input_state_create_info;
        pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
        pipelineInfo.pViewportState      = &viewport_state_create_info;
        pipelineInfo.pRasterizationState = &rasterization_state_create_info;
        pipelineInfo.pMultisampleState   = &multisample_state_create_info;
        pipelineInfo.pColorBlendState    = &color_blend_state_create_info;
        pipelineInfo.pDepthStencilState  = &depth_stencil_create_info;
        pipelineInfo.layout              = _render_pipelines[0].layout;
        pipelineInfo.renderPass          = _framebuffer.render_pass;
        pipelineInfo.subpass             = _main_camera_subpass_blur;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState       = &dynamic_state_create_info;

        if (vkCreateGraphicsPipelines(m_p_vulkan_context->_device,
                                      VK_NULL_HANDLE,
                                      1,
                                      &pipelineInfo,
                                      nullptr,
                                      &_render_pipelines[0].pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("create post process graphics pipeline");
        }

        vkDestroyShaderModule(m_p_vulkan_context->_device, vert_shader_module, nullptr);
        vkDestroyShaderModule(m_p_vulkan_context->_device, frag_shader_module, nullptr);
    }
    void PBlurPass::setupDescriptorSet()
    {
        VkDescriptorSetAllocateInfo post_process_global_descriptor_set_alloc_info;
        post_process_global_descriptor_set_alloc_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        post_process_global_descriptor_set_alloc_info.pNext          = NULL;
        post_process_global_descriptor_set_alloc_info.descriptorPool = m_descriptor_pool;
        post_process_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        post_process_global_descriptor_set_alloc_info.pSetLayouts        = &_descriptor_infos[0].layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_p_vulkan_context->_device,
                                                   &post_process_global_descriptor_set_alloc_info,
                                                   &_descriptor_infos[0].descriptor_set))
        {
            throw std::runtime_error("allocate post process global descriptor set");
        }
    }

    void PBlurPass::updateAfterFramebufferRecreate(VkImageView input_attachment)
    {
        VkDescriptorImageInfo post_process_per_frame_input_attachment_info = {};
        post_process_per_frame_input_attachment_info.sampler =
            PVulkanUtil::getOrCreateNearestSampler(m_p_vulkan_context->_physical_device, m_p_vulkan_context->_device);
        post_process_per_frame_input_attachment_info.imageView   = input_attachment;
        post_process_per_frame_input_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet post_process_descriptor_writes_info[3];

        VkWriteDescriptorSet& post_process_descriptor_input_attachment_write_info =
            post_process_descriptor_writes_info[0];
        post_process_descriptor_input_attachment_write_info.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        post_process_descriptor_input_attachment_write_info.pNext           = NULL;
        post_process_descriptor_input_attachment_write_info.dstSet          = _descriptor_infos[0].descriptor_set;
        post_process_descriptor_input_attachment_write_info.dstBinding      = 0;
        post_process_descriptor_input_attachment_write_info.dstArrayElement = 0;
        post_process_descriptor_input_attachment_write_info.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; 
        post_process_descriptor_input_attachment_write_info.descriptorCount = 1;
        post_process_descriptor_input_attachment_write_info.pImageInfo = &post_process_per_frame_input_attachment_info;

        //add Infinite Tsukuyomi texture
        VkDescriptorImageInfo color_grading_infinite_tsukuyomi_image_info = {};
        color_grading_infinite_tsukuyomi_image_info.sampler =
            PVulkanUtil::getOrCreateLinearSampler(m_p_vulkan_context->_physical_device, m_p_vulkan_context->_device);
        color_grading_infinite_tsukuyomi_image_info.imageView =
            m_p_global_render_resource->_blur_resource._infinite_tsukuyomi_texture_image_view;
        color_grading_infinite_tsukuyomi_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet& post_process_descriptor_infinite_tsukuyomi_write_info =
            post_process_descriptor_writes_info[1];
        post_process_descriptor_infinite_tsukuyomi_write_info.sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        post_process_descriptor_infinite_tsukuyomi_write_info.pNext  = NULL;
        post_process_descriptor_infinite_tsukuyomi_write_info.dstSet = _descriptor_infos[0].descriptor_set;
        post_process_descriptor_infinite_tsukuyomi_write_info.dstBinding = 1;
        post_process_descriptor_infinite_tsukuyomi_write_info.dstArrayElement = 0;
        post_process_descriptor_infinite_tsukuyomi_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        post_process_descriptor_infinite_tsukuyomi_write_info.descriptorCount = 1;
        post_process_descriptor_infinite_tsukuyomi_write_info.pImageInfo = &color_grading_infinite_tsukuyomi_image_info;

        //add ubo info
        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of the buffer
        mesh_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer =
            m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
               m_p_global_render_resource->_storage_buffer._max_storage_buffer_range);

        VkWriteDescriptorSet& mesh_descriptor_writes_info =
            post_process_descriptor_writes_info[2];

        mesh_descriptor_writes_info.sType                                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info.pNext                                 = NULL;
        mesh_descriptor_writes_info.dstSet                                = _descriptor_infos[0].descriptor_set;
        mesh_descriptor_writes_info.dstBinding                            = 2;
        mesh_descriptor_writes_info.dstArrayElement                       = 0;
        mesh_descriptor_writes_info.descriptorType                        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;        
        mesh_descriptor_writes_info.descriptorCount                       = 1;
        mesh_descriptor_writes_info.pBufferInfo                           = &mesh_perframe_storage_buffer_info;
        //end add ubo info

        vkUpdateDescriptorSets(m_p_vulkan_context->_device,
                               sizeof(post_process_descriptor_writes_info) /
                                   sizeof(post_process_descriptor_writes_info[0]),
                               post_process_descriptor_writes_info,
                               0,
                               NULL);
    }

    void PBlurPass::draw(MeshPerframeStorageBufferObject& m_mesh_perframe_storage_buffer_object)
    {
        if (m_render_config._enable_debug_untils_label)
        {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, "Blur", {1.0f, 1.0f, 1.0f, 1.0f}};
            m_p_vulkan_context->_vkCmdBeginDebugUtilsLabelEXT(m_command_info._current_command_buffer, &label_info);
        }

        //add extra info for blur effects
        double time                                = glfwGetTime();
        double time_decimal                        = time - floor(time);
        VkExtent2D v2                              = m_p_vulkan_context->_swapchain_extent;
        m_mesh_perframe_storage_buffer_object.time     = glm::vec4(time, time_decimal, time_decimal / 20.0f, time);
        m_mesh_perframe_storage_buffer_object.screen_resolution =
            glm::vec4(float(m_p_vulkan_context->_swapchain_extent.width), float(m_p_vulkan_context->_swapchain_extent.height), 0.0f, 0.0f); 
        m_mesh_perframe_storage_buffer_object.editor_screen_resolution = glm::vec4((m_command_info._viewport.x),
                                                                                   (m_command_info._viewport.y),
                                                                                   (m_command_info._viewport.width),
                                                                                   (m_command_info._viewport.height));

        uint32_t perframe_dynamic_offset =
            roundUp(m_p_global_render_resource->_storage_buffer
                        ._global_upload_ringbuffers_end[m_command_info._current_frame_index],
                    m_p_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);

        m_p_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[m_command_info._current_frame_index] =
            perframe_dynamic_offset + sizeof(MeshPerframeStorageBufferObject);
        assert(m_p_global_render_resource->_storage_buffer
                   ._global_upload_ringbuffers_end[m_command_info._current_frame_index] <=
               (m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_begin[m_command_info._current_frame_index] +
                m_p_global_render_resource->_storage_buffer
                    ._global_upload_ringbuffers_size[m_command_info._current_frame_index]));

        (*reinterpret_cast<MeshPerframeStorageBufferObject*>(
            reinterpret_cast<uintptr_t>(
                m_p_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
            perframe_dynamic_offset)) = m_mesh_perframe_storage_buffer_object;

        uint32_t dynamic_offsets[1] = {perframe_dynamic_offset};

        //Sven modify: use full screen view port
        VkViewport viewport = {0.0,
                               0.0,
                               static_cast<float>(m_p_vulkan_context->_swapchain_extent.width),
                               static_cast<float>(m_p_vulkan_context->_swapchain_extent.height),
                               0.0,
                               1.0};
        VkRect2D   scissor  = {
            0, 0, m_p_vulkan_context->_swapchain_extent.width, m_p_vulkan_context->_swapchain_extent.height};

        m_p_vulkan_context->_vkCmdBindPipeline(
            m_command_info._current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _render_pipelines[0].pipeline);
        //m_p_vulkan_context->_vkCmdSetViewport(m_command_info._current_command_buffer, 0, 1, &viewport);
        //m_p_vulkan_context->_vkCmdSetScissor(m_command_info._current_command_buffer, 0, 1, &scissor);
        m_p_vulkan_context->_vkCmdSetViewport(m_command_info._current_command_buffer, 0, 1, &m_command_info._viewport);
        m_p_vulkan_context->_vkCmdSetScissor(m_command_info._current_command_buffer, 0, 1, &m_command_info._scissor);

        //Sven modify: add dynamic ubo data
        //m_p_vulkan_context->_vkCmdBindDescriptorSets(m_command_info._current_command_buffer,
        //                                             VK_PIPELINE_BIND_POINT_GRAPHICS,
        //                                             _render_pipelines[0].layout,
        //                                             0,
        //                                             1,
        //                                             &_descriptor_infos[0].descriptor_set,
        //                                             0,
        //                                             NULL);
        m_p_vulkan_context->_vkCmdBindDescriptorSets(m_command_info._current_command_buffer,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     _render_pipelines[0].layout,
                                                     0,
                                                     1,
                                                     &_descriptor_infos[0].descriptor_set,
                                                     (sizeof(dynamic_offsets) / sizeof(dynamic_offsets[0])),
                                                     dynamic_offsets);

        vkCmdDraw(m_command_info._current_command_buffer, 3, 1, 0, 0);

        if (m_render_config._enable_debug_untils_label)
        {
            m_p_vulkan_context->_vkCmdEndDebugUtilsLabelEXT(m_command_info._current_command_buffer);
        }
    }

} // namespace Pilot
