#include "common/baseVideoFrag.hlsl"

float4 MainPS
(
    PSVertex input
#if (defined (ENABLE_FOVEATION_DECODE) && defined(ENABLE_SM6_MULTI_VIEW))
    , in uint ViewId : SV_ViewID
#endif
) : SV_TARGET
{
    const float3 rgb = PS_SAMPLE_VIDEO_TEXTURE(input);
    const float alpha = 1.0f - rgb.g;
    return float4(rgb, alpha);
}
