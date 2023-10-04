#include "common/baseVideoFrag.hlsl"
#include "common/colorfunctions.hlsl"




static const float3 MaskKeyColor = float3(0.21, 0.61, 0.31);

static const float3 key_color = vec3(44.77, -14.95, 7.98);

float4 MainPS
(
    PSVertex input
#if (defined (ENABLE_FOVEATION_DECODE) && defined(ENABLE_SM6_MULTI_VIEW))
    , in uint ViewId : SV_ViewID
#endif
) : SV_TARGET
{
    const float3 rgb = PS_SAMPLE_VIDEO_TEXTURE(input);
    const float3 labcolor = SRGB_TO_LAB(color.rgb);
    const float alpha = all(rgb < MaskKeyColor) ? 0.3f : 1.0f;
    return float4(rgb, alpha);

   }

float4 MainPS
(
layout(location = 0) out vec4 FragColor;

float when_gt(float x, float y) { return max(sign(x - y), 0.0f); }

float when_le(float x, float y) { return 1.0f - when_gt(x, y); }

float when_lt(float x, float y) { return max(sign(y - x), 0.0f); }

void main() {
    float3 rgb = PS_SAMPLE_VIDEO_TEXTURE(input);
    float3 labcolor = SRGB_TO_LAB(color.rgb);
    float deltaE = LAB_DELTA_E_CIE2000(labcolor, key_color);
    color.a -= when_lt(deltaE, 18.2f) * (1.f - (deltaE * 0.009f));
    color.a *= when_gt(deltaE, 9.90f);
    return float4(rgb, alpha);
}
