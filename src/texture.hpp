#pragma once

#include "common/common.hpp"

struct Texture
{
    u8* data;
    size_t size;
    String name;
    u32 width;
    u32 height;
    u32 channels;
    size_t gpuTextureId;
};
