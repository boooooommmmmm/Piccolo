#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture_sampler;

layout(location = 0) out highp vec4 out_color;

void main()
{
    highp ivec2 lut_tex_size = textureSize(color_grading_lut_texture_sampler, 0);
    highp float _COLORS      = float(lut_tex_size.y);

    highp vec4 color       = subpassLoad(in_color).rgba;
    
    // texture(color_grading_lut_texture_sampler, uv)

    //x: 1/width; y: 1/height; z:width - 1; width should always equls to height * height
    highp vec3 scale_offset = vec3( 1.0f / float(lut_tex_size.x), 
                                    1.0f / float(lut_tex_size.y), 
                                    32.0f - 1.0f);

    highp float shift_x = floor(color.b * scale_offset.z);
    highp float delta_shift_x = (color.b * scale_offset.z) - shift_x;
    highp float x = color.r * scale_offset.x * scale_offset.z + scale_offset.y * shift_x;
    highp float y = color.g * scale_offset.y * scale_offset.z;
    highp vec2 uv = vec2(x, y);

    uv += vec2(scale_offset.x * 0.5f, scale_offset.y * 0.5f); //half pixcel offset to better suit linear sampler

    highp vec3 lut_color1 = texture(color_grading_lut_texture_sampler, uv).rgb;
    highp vec3 lut_color2 = texture(color_grading_lut_texture_sampler, uv + vec2(scale_offset.y, 0)).rgb;

    //lerp between current LUT block and next block to aviod color crack.
    highp vec3 lut_final_color = lut_color1 * (1.0f - delta_shift_x) + lut_color2 * delta_shift_x;

    out_color.rgb = lut_final_color;
    
    //test: temporal remove color grading
    //out_color = color;
}
