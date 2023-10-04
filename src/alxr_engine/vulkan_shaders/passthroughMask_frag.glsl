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

/*
vec4 when_eq(vec4 x, vec4 y) {
  return 1.0 - abs(sign(x - y));
}

vec4 when_neq(vec4 x, vec4 y) {
  return abs(sign(x - y));
}

vec4 when_gt(vec4 x, vec4 y) {
  return max(sign(x - y), 0.0);
}

vec4 when_lt(vec4 x, vec4 y) {
  return max(sign(y - x), 0.0);
}

vec4 when_ge(vec4 x, vec4 y) {
  return 1.0 - when_lt(x, y);
}

vec4 when_le(vec4 x, vec4 y) {
  return 1.0 - when_gt(x, y);
}

vec4 and(vec4 a, vec4 b) {
  return a * b;
}

vec4 or(vec4 a, vec4 b) {
  return min(a + b, 1.0);
}

vec4 xor(vec4 a, vec4 b) {
  return (a + b) % 2.0;
}

vec4 not(vec4 a) {
  return 1.0 - a;
}
    
 */

const vec3 key_color = vec3(37.77, -14.95, 7.98);  //60, 92, 72
layout(location = 0) out vec4 FragColor;
const float threshold1 = 11.0f; //high confidence
const float threshold2 = 18.2f; //lower confidence 
const float smoothing = 0.007f; 

//math functions to avoid branching with conditionals

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
    //if low confidence match reduce alpha by smoothing factor
    color.a -= when_le(deltaE,threshold2) * (1.f - (deltaE * smoothing)); 
    //multiply by 0 if high confidence. Two step method works better than any single formula
    color.a *= when_gt(deltaE, threshold1);
    FragColor = vec4(color.rgb, color.a);
}
