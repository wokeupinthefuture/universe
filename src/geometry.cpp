#include "geometry.hpp"
#include "platform.hpp"

void generateSphere(
    float radius, u32 stacks, u32 slices, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount)
{
    size_t vertexCount = 0;
    // Generate vertices
    for (unsigned int i = 0; i <= stacks; ++i)
    {
        float phi = PI * static_cast<float>(i) / stacks;  // Polar angle (0 to π)
        float sinPhi = sinf(phi);
        float cosPhi = cosf(phi);

        for (unsigned int j = 0; j <= slices; ++j)
        {
            float theta = 2.0f * PI * static_cast<float>(j) / slices;  // Azimuthal angle (0 to 2π)
            Vertex vertex{};
            vertex.pos = vec3{
                radius * sinPhi * cosf(theta),  // x
                radius * cosPhi,                // y
                radius * sinPhi * sinf(theta)   // z
            };
            ENSURE(vertexCount < verticesCount);
            outVertices[vertexCount++] = vertex;
        }
    }

    size_t indexCount = 0;
    // Generate indices
    for (unsigned int i = 0; i < stacks; ++i)
    {
        for (unsigned int j = 0; j < slices; ++j)
        {
            unsigned int first = i * (slices + 1) + j;
            unsigned int second = first + slices + 1;

            // First triangle (clockwise)
            outIndices[indexCount++] = first;
            outIndices[indexCount++] = second;
            outIndices[indexCount++] = first + 1;

            // Second triangle (clockwise)
            outIndices[indexCount++] = second;
            outIndices[indexCount++] = second + 1;
            ENSURE(indexCount < indicesCount);
            outIndices[indexCount++] = first + 1;
        }
    }
}

void generateGrid(
    i32 gridX, i32 gridY, float spacing, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount)
{
    i32 vertexCount = 0;
    for (int z = 0; z <= gridY; z++)
    {
        for (int x = 0; x < gridX; x++)
        {
            Vertex vertex{};
            vertex.pos = {x * spacing - (spacing * 0.5 * (gridX - 1)), 0.0f, z * spacing - (spacing * 0.5 * (gridY - 1))};
            outVertices[vertexCount++] = vertex;
        }
    }

    i32 indexCount = 0;
    for (int i = 0, lineBegin = 0, lineEnd = gridX - 1; i < gridY; ++i, lineBegin += gridX, lineEnd += gridX)
    {
        outIndices[indexCount++] = lineBegin;
        outIndices[indexCount++] = lineEnd;
    }

    for (int i = 0, lineBegin = 0, lineEnd = gridX * (gridY - 1) + lineBegin; i < gridX;
        ++i, ++lineBegin, lineEnd = gridX * (gridY - 1) + lineBegin)
    {
        outIndices[indexCount++] = lineBegin;
        outIndices[indexCount++] = lineEnd;
    }
}
