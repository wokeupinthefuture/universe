cbuffer Variables : register(b0)
{
    float4x4 world;
    float4x4 mvp;
    float4 objectColor;
    float4 lightColor;
    float3 lightDirection;
    float3 lightPosition;
    float time;
    int lightType;
}

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct PSInput
{
    float4 clipPos : SV_POSITION;
    float3 worldPos : WPOSITION;
    float3 worldNormal : NORMAL;
};
