cbuffer BasicCB : register(b0)
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

cbuffer UnlitCB : register(b0)
{
    float4x4 ul_mvp;
    float4 ul_objectColor;
    float ul_time;
}

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 worldNormal : NORMAL;
};
