cbuffer Variables : register(b0)
{
    float4x4 v_world;
    float4x4 v_mvp;
    float4 v_objectColor;
    float4 v_lightColor;
    float3 v_lightDirection;
    float3 v_lightPosition;
    float v_time;
    int v_lightType;
}
