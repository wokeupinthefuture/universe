#include "shaders.hpp"
#include "platform.hpp"
#include "renderer.hpp"

static ShaderVariable* getVariableByName(ShaderVariable* variables, const char* name)
{
    auto var = find(variables,
        MAX_SHADER_VARIABLES,
        [name](ShaderVariable& var)
        {
            if (!var.name)
                return false;
            return strncmp(var.name, name, 256) == 0;
        });
    ENSURE(var != nullptr);
    return var;
}

void setShaderVariableInt(DrawCommand& command, const char* variableName, int value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.i = value;
    var->value = newValue;
}

void setShaderVariableFloat(DrawCommand& command, const char* variableName, float value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.f = value;
    var->value = newValue;
}

void setShaderVariableVec2(DrawCommand& command, const char* variableName, vec2 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.v2 = value;
    var->value = newValue;
}

void setShaderVariableVec3(DrawCommand& command, const char* variableName, vec3 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.v3 = value;
    var->value = newValue;
}

void setShaderVariableVec4(DrawCommand& command, const char* variableName, vec4 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.v4 = value;
    var->value = newValue;
}

void setShaderVariableMat4(DrawCommand& command, const char* variableName, mat4 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.m4 = value;
    var->value = newValue;
}
float getShaderVariableFloat(DrawCommand& command, const char* variableName)
{
    auto var = getVariableByName(command.variables, variableName);
    return var->value.f;
}

i32 getShaderVariableInt(DrawCommand& command, const char* variableName)
{
    auto var = getVariableByName(command.variables, variableName);
    return var->value.i;
}

vec2 getShaderVariableVec2(DrawCommand& command, const char* variableName)
{
    auto var = getVariableByName(command.variables, variableName);
    return var->value.v2;
}

vec3 getShaderVariableVec3(DrawCommand& command, const char* variableName)
{
    auto var = getVariableByName(command.variables, variableName);
    return var->value.v3;
}

vec4 getShaderVariableVec4(DrawCommand& command, const char* variableName)
{
    auto var = getVariableByName(command.variables, variableName);
    return var->value.v4;
}

mat4 getShaderVariableMat4(DrawCommand& command, const char* variableName)
{
    auto var = getVariableByName(command.variables, variableName);
    return var->value.m4;
}
