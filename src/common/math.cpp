#include "math.hpp"
#include "memory.hpp"

#define NOMINMAX

vec3 getForwardVector(mat4 rotation)
{
    return {rotation[2][0], rotation[2][1], rotation[2][2]};
}

vec3 getUpVector(mat4 rotation)
{
    return {rotation[1][0], rotation[1][1], rotation[1][2]};
}

vec3 getRightVector(mat4 rotation)
{
    return {rotation[0][0], rotation[0][1], rotation[0][2]};
}

quat eulerToQuat(vec3 const& eulerDegrees)
{
    const float halfToRad = 0.5f * PI / 180.0f;

    vec3 eulerRadians = eulerDegrees * halfToRad;

    float sx = std::sin(eulerRadians.x);
    float cx = std::cos(eulerRadians.x);
    float sy = std::sin(eulerRadians.y);
    float cy = std::cos(eulerRadians.y);
    float sz = std::sin(eulerRadians.z);
    float cz = std::cos(eulerRadians.z);

    quat q;
    q.x = sx * cy * cz + cx * sy * sz;
    q.y = cx * sy * cz + sx * cy * sz;
    q.z = cx * cy * sz - sx * sy * cz;
    q.w = cx * cy * cz - sx * sy * sz;

    return q;
}

vec3 quatToEuler(quat const& quat)
{
    float x = quat.x;
    float y = quat.y;
    float z = quat.z;
    float w = quat.w;

    float roll = 0.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;

    float test = x * y + z * w;

    if (test > 0.499999f)
    {
        roll = 0.0f;
        yaw = degrees(2.0f * std::atan2(x, w));
        pitch = 90.0f;
    }
    else if (test < -0.499999f)
    {
        roll = 0.0f;
        yaw = -degrees(2.0f * std::atan2(x, w));
        pitch = -90.0f;
    }
    else
    {
        float sqx = x * x;
        float sqy = y * y;
        float sqz = z * z;
        roll = degrees(std::atan2(2.0f * x * w - 2.0f * y * z, 1.0f - 2.0f * sqx - 2.0f * sqz));
        yaw = degrees(std::atan2(2.0f * y * w - 2.0f * x * z, 1.0f - 2.0f * sqy - 2.0f * sqz));
        pitch = degrees(std::asin(2.0f * test));
    }

    return vec3(roll, yaw, pitch);
}

vec3 quatToDirection(quat const& quat, vec3 viewUp)
{
    const auto normQuat = normalize(quat);
    return normQuat * viewUp;
}

vec3 matrixExtractPosition(mat4 mat)
{
    return {mat[3][0], mat[3][1], mat[3][2]};
}

vec3 matrixExtractScale(mat4 mat)
{
    vec3 scale{};
    scale.x = glm::length(mat[0]);
    scale.y = glm::length(mat[1]);
    scale.z = glm::length(mat[2]);
    return scale;
}

mat4 transformToMatrix(vec3 position, quat rotation, vec3 _scale)
{
    const auto scaleMatrix = scale(mat4(1.f), _scale);
    const auto rotMatrix = toMat4(rotation);
    const auto posMatrix = translate(mat4(1.f), position);
    return posMatrix * rotMatrix * scaleMatrix;
}

void matrixExtractRotation(mat4 mat, vec3 scale, quat& outRot, vec3& outEuler)
{
    auto rotationMatrix = mat;
    // removing translation (glm stores matrices in column-major order!)
    rotationMatrix[3][0] = 0;
    rotationMatrix[3][1] = 0;
    rotationMatrix[3][2] = 0;
    vec3 absScale = glm::abs(scale);
    const float epsilon = 0.0001f;
    // removing scale which is in 3x3 cells
    if (absScale.x > epsilon)
        rotationMatrix[0] /= vec4{scale, 1.f};
    if (absScale.y > epsilon)
        rotationMatrix[1] /= vec4{scale, 1.f};
    if (absScale.z > epsilon)
        rotationMatrix[2] /= vec4{scale, 1.f};
    outRot = toQuat(rotationMatrix);
    outEuler = quatToEuler(outRot);
}

vec3 worldToLocal(vec3 worldPoint, mat4 parentMatrix)
{
    return glm::inverse(parentMatrix) * vec4(worldPoint, 1.f);
}

quat worldToLocal(quat worldRot, quat parentWorldRot)
{
    return glm::inverse(parentWorldRot) * worldRot;
}

float remap(float source, float sourceFrom, float sourceTo, float targetFrom, float targetTo)
{
    return targetFrom + (source - sourceFrom) * (targetTo - targetFrom) / (sourceTo - sourceFrom);
}

const char* vec3ToString(Arena& arena, vec3 v)
{
    auto string = arenaAlloc<char>(arena, 256);
    sprintf(string, "%f, %f, %f", v.x, v.y, v.z);
    return string;
}
