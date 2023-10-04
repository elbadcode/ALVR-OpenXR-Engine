#version 460
#ifdef ENABLE_ARB_INCLUDE_EXT
#extension GL_ARB_shading_language_include : require
#else
// required by glslangValidator
#extension GL_GOOGLE_include_directive : require
#endif
#pragma fragment

#include "common/baseVideoFrag.glsl"
#include "common/colorFunctions.glsl"

layout(constant_id = 9) const float AlphaValue = 0.74f; //using this as a smoothing value
layout(location = 0) out vec4 FragColor;
const float threshold = 8.78f;
const vec3 key_color = vec3(0, 0, 0);


float when_gt(float x, float y) {
    return max(sign(x - y), 0.0f);
}

float when_le(float x, float y) {
    return 1.0f - when_gt(x, y);
}

float when_lt(float x, float y) {
    return max(sign(y - x), 0.0f);
}       

void main()
{
    vec4 color = vec4(SampleVideoTexture().rgb, 1.0f);
    vec3 labcolor = SRGB_TO_LAB(color.rgb);
    float deltaE = LAB_DELTA_E_CIE2000(labcolor, key_color);
    color.a = when_gt(deltaE, threshold) * AlphaValue;
    color.a += (1.f - AlphaValue) * AlphaValue;
    FragColor = vec4(color.rgb, color.a);
}
  
