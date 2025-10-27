Texture2D diffuseTexture : register(t0);
SamplerState texSampler : register(s0);

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 clipPos : SV_POSITION;
    float3 worldPos : WPOSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD;
};
