#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "gbuffer.h"


struct DirectionalLight
{
    highp vec3 direction;
    lowp float _padding_direction;
    highp vec3 color;
    lowp float _padding_color;
};

struct PointLight
{
    highp vec3  position;
    highp float radius;
    highp vec3  intensity;
    lowp float  _padding_intensity;
};

layout(set = 0, binding = 2) readonly buffer _mesh_per_frame
{
    highp mat4       proj_view_matrix;
    highp vec3       camera_position;
    lowp float       _padding_camera_position;
    highp vec3       ambient_light;
    lowp float       _padding_ambient_light;
    highp uint       point_light_num;
    uint             _padding_point_light_num_1;
    uint             _padding_point_light_num_2;
    uint             _padding_point_light_num_3;
    PointLight       scene_point_lights[m_max_point_light_count];
    DirectionalLight scene_directional_light;
    highp mat4       directional_light_proj_view;

    highp vec4       time;
    highp vec4       screen_resolution;
    highp vec4       editor_screen_resolution;
};


layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 0) uniform sampler2D input_texture_sampler;
layout(set = 0, binding = 1) uniform sampler2D color_grading_infinite_tsukuyomi_texture_sampler;

layout(location = 0) in highp vec2 in_texcoord;
layout(location = 0) out highp vec4 out_color;

highp vec2 MappingUV2Viewport(highp vec2 full_screen_uv);
highp vec2 MappingUV2ViewportInSquare(highp vec2 full_screen_uv);
highp vec2 MappingUV2InfiniteTsukuyomi(highp vec2 uv_in_viewport, highp vec2 sun_pos, highp float sun_center_range);
//bool CheckIsUVOutsideOfViewport(highp vec2 full_screen_uv);
highp vec2 ClampUV(highp vec2 uv);
highp float FloatLerp(highp float v1, highp float v2, highp float x);
highp float Max(highp float x, highp float y);
highp float Min(highp float x, highp float y);

#include "mesh_lighting.h"

void main()
{
    //Constant settings
    lowp float sun_center_range = 0.1f;
    lowp float sun_halo_range = 0.2f;
    lowp vec3 sun_color = vec3(1.0f, 0.1f, 0.0f);
    highp float radial_blur_step = 0.001f;
    lowp int raidal_blur_sample_count = 64;

    //color of current framebuffer
    highp vec3 color       = subpassLoad(in_color).rgb;

    //temporal values
    lowp vec2 sun_pos = vec2(0.2f, 0.2f);
    lowp vec2 sun_pos_in_square = MappingUV2ViewportInSquare(sun_pos);

    highp vec2 uv_in_viewport = MappingUV2Viewport(in_texcoord.xy);
    highp vec2 uv_in_square = MappingUV2ViewportInSquare(in_texcoord.xy);

    highp vec2 delta_pos = sun_pos_in_square - uv_in_square;
    highp float distance_sqr = delta_pos.x * delta_pos.x + delta_pos.y * delta_pos.y;
    highp vec2 direction = normalize(uv_in_square - sun_pos_in_square);

    //initialize default color
    highp vec4 final_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    final_color.rgb = color;
    final_color.a = 1.0f;

    highp float sun_center_range_sqr = sun_center_range * sun_center_range;
    highp float sun_halo_range_sqr = sun_halo_range * sun_halo_range;
    if(distance_sqr < sun_center_range_sqr)
    {
        highp vec2 uv_for_sun_texture = MappingUV2InfiniteTsukuyomi(uv_in_viewport, sun_pos, sun_center_range);
        final_color.rgb = texture(color_grading_infinite_tsukuyomi_texture_sampler, uv_for_sun_texture).rgb;
    }
    else if(distance_sqr < sun_halo_range_sqr)
    {
        //final_color.rgb = sun_color + color;
        //final_color.a = FloatLerp(0.0f, 1.0f, (distance_sqr - sun_center_range_sqr)/(sun_halo_range_sqr - sun_center_range_sqr));

        lowp float sun_color_add = FloatLerp(0.0f, 1.0f, (distance_sqr - sun_center_range_sqr)/(sun_halo_range_sqr - sun_center_range_sqr));
        final_color.rgb = color + sun_color * sun_color_add;
        final_color.a = 1.0f;
    }
    else
    {
        //cal angle
        highp float step = radial_blur_step;
        highp float angle = dot(direction, vec2(1.0f, 0.0f));
        highp float step_noise = cos(angle * 100.0f + (time.y - 0.5f) * 20.0f);

        step = step * step_noise * (distance_sqr - sun_halo_range_sqr) * 1.0f;

        //blur
        highp int weight = raidal_blur_sample_count + 1; //current pixcel color weight is high.
        final_color.rgb = final_color.rgb * float(weight);
        for(lowp int i = 1; i <= raidal_blur_sample_count; i++)
        {
            highp vec2 sample_uv = uv_in_viewport + direction * float(i) * step;
            sample_uv = ClampUV(sample_uv);
            highp vec3 res = texture(input_texture_sampler, sample_uv).rgb;
            final_color.rgb += res * float(i);
            weight += i;
        }

        final_color.rgb /= float(weight);
        
        //final_color.rgb = texture(input_texture_sampler, uv_in_viewport).rgb;
    }

    //test
    //final_color.rgb = color;
    out_color = final_color;
}

highp vec2 MappingUV2Viewport(highp vec2 full_screen_uv)
{
    highp vec2 editor_ratio = editor_screen_resolution.zw / screen_resolution.xy;
    highp vec2 offset = editor_screen_resolution.xy / screen_resolution.xy;
    highp vec2 viewport_uv = full_screen_uv.xy * editor_ratio + offset.xy;
    
    return viewport_uv;
}

highp vec2 MappingUV2ViewportInSquare(highp vec2 full_screen_uv)
{
    highp vec2 viewport_uv = MappingUV2Viewport(full_screen_uv);

    if(editor_screen_resolution.z > editor_screen_resolution.w)//width is bigger than height
    {
        highp float ratio = editor_screen_resolution.w / editor_screen_resolution.z;
        viewport_uv.x /= ratio;
    }
    else
    {
        highp float ratio = editor_screen_resolution.z / editor_screen_resolution.w;
        viewport_uv.y /= ratio;
    }    
    
    return viewport_uv;
}

highp vec2 MappingUV2InfiniteTsukuyomi(highp vec2 uv_in_viewport, highp vec2 sun_pos, highp float sun_center_range)
{    
    //magic, fix it latter
    highp vec2 sun_pos_in_view_port = MappingUV2Viewport(sun_pos);
    lowp float scale = uv_in_viewport.x / sun_center_range;
    lowp vec2 offset = vec2(-sun_pos.x, -sun_pos.y);

    lowp vec2 final_uv = uv_in_viewport - sun_pos_in_view_port;
    final_uv = final_uv * 5.0f + 0.5f;
    return final_uv;
}

/*
bool CheckIsUVOutsideOfViewport(highp vec2 full_screen_uv)
{
    highp vec2 uv_min = MappingUV2Viewport(vec2(0.0f, 0.0f));
    highp vec2 uv_max = MappingUV2Viewport(vec2(1.0f, 1.0f));
    highp vec2 uv = MappingUV2Viewport(full_screen_uv);

    if(full_screen_uv.x < uv_min.x || full_screen_uv.y < uv_min.y 
        || full_screen_uv.x > uv_max.x || full_screen_uv.y > uv_max.y)
    {
        return true;
    }
    return false;
}
*/

highp vec2 ClampUV(highp vec2 uv)
{
    highp vec2 uv_min = MappingUV2Viewport(vec2(0.0f, 0.0f));
    highp vec2 uv_max = MappingUV2Viewport(vec2(1.0f, 1.0f));
    uv.x = Max(uv.x, uv_min.x + 0.001f);
    uv.y = Max(uv.y, uv_min.y + 0.001f);
    uv.x = Min(uv.x, uv_max.x - 0.001f);
    uv.y = Min(uv.y, uv_max.y - 0.001f);

    return uv;
}

highp float FloatLerp(highp float v1, highp float v2, highp float x)
{
    highp float lerp_value = Max(x, 0.0f);
    lerp_value = Min(x, 1.0f);

    highp float final_value = v1 * lerp_value + v2 * (1.0f - lerp_value);
    return final_value;
}

highp float Max(highp float x, highp float y)
{
    return x > y ? x : y;
}

highp float Min(highp float x, highp float y)
{
    return x < y ? x : y;
}