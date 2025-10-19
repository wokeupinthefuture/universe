#include "geometry.hpp"
#include "common/utils.hpp"
#include "platform.hpp"
#include "common/memory.hpp"

static Mesh generateSphere(
    float radius, u32 stacks, u32 slices, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount)
{
    Mesh mesh{};

    mesh.vertices = outVertices;
    mesh.verticesCount = verticesCount;
    mesh.indices = outIndices;
    mesh.indicesCount = indicesCount;
    mesh.isIndexed = true;

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

    return mesh;
}

static Mesh generateGrid(
    i32 gridX, i32 gridY, float spacing, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount)
{
    Mesh mesh{};

    mesh.vertices = outVertices;
    mesh.verticesCount = verticesCount;
    mesh.indices = outIndices;
    mesh.indicesCount = indicesCount;
    mesh.isIndexed = true;

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

    return mesh;
}

Mesh generateMesh(MeshType type)
{
    Mesh mesh{};

    switch (type)
    {
        case MeshType::Triangle:
        {
            static Vertex VERTICES[] = {{.pos = vec3{-0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{0.0f, 0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}}};
            static u32 INDICES[] = {0, 1, 2};

            mesh.vertices = VERTICES;
            mesh.verticesCount = ARR_LENGTH(VERTICES);
            mesh.indices = INDICES;
            mesh.indicesCount = ARR_LENGTH(INDICES);
            mesh.isIndexed = true;

            break;
        }
        case MeshType::Quad:
        {
            static Vertex VERTICES[] = {{.pos = vec3{-0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{-0.5f, 0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{0.5f, 0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}}};
            static u32 INDICES[] = {0, 1, 2, 2, 3, 0};

            mesh.vertices = VERTICES;
            mesh.verticesCount = ARR_LENGTH(VERTICES);
            mesh.indices = INDICES;
            mesh.indicesCount = ARR_LENGTH(INDICES);
            mesh.isIndexed = true;

            break;
        }
        case MeshType::Cube:
        {
            static Vertex VERTICES[] = {// Front face (z = 0.5)
                {.pos = vec3{-0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
                {.pos = vec3{0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
                {.pos = vec3{0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
                {.pos = vec3{-0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
                // Back face (z = -0.5)
                {.pos = vec3{-0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{-0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                {.pos = vec3{0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
                // Left face (x = -0.5)
                {.pos = vec3{-0.5f, -0.5f, -0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
                {.pos = vec3{-0.5f, -0.5f, 0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
                {.pos = vec3{-0.5f, 0.5f, 0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
                {.pos = vec3{-0.5f, 0.5f, -0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
                // Right face (x = 0.5)
                {.pos = vec3{0.5f, -0.5f, -0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
                {.pos = vec3{0.5f, 0.5f, -0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
                {.pos = vec3{0.5f, 0.5f, 0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
                {.pos = vec3{0.5f, -0.5f, 0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
                // Top face (y = 0.5)
                {.pos = vec3{-0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
                {.pos = vec3{-0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
                {.pos = vec3{0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
                {.pos = vec3{0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
                // Bottom face (y = -0.5)
                {.pos = vec3{-0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}},
                {.pos = vec3{0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}},
                {.pos = vec3{0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}},
                {.pos = vec3{-0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}}};

            // Indices for 6 faces (12 triangles)
            static u32 INDICES[] = {
                0,
                1,
                2,
                0,
                2,
                3,  // Front
                4,
                5,
                6,
                4,
                6,
                7,  // Back
                8,
                9,
                10,
                8,
                10,
                11,  // Left
                12,
                13,
                14,
                12,
                14,
                15,  // Right
                16,
                17,
                18,
                16,
                18,
                19,  // Top
                20,
                21,
                22,
                20,
                22,
                23  // Bottom
            };

            mesh.vertices = VERTICES;
            mesh.verticesCount = ARR_LENGTH(VERTICES);
            mesh.indices = INDICES;
            mesh.indicesCount = ARR_LENGTH(INDICES);
            mesh.isIndexed = true;

            break;
        }
        case MeshType::Sphere:
        {
            static constexpr auto SPHERE_STACKS = 16;
            static constexpr auto SPHERE_SLICES = 16;
            static constexpr auto SPHERE_RADIUS = 1.f;

            static Vertex VERTICES[(SPHERE_STACKS + 1) * (SPHERE_SLICES + 1)] = {};
            static u32 INDICES[(SPHERE_STACKS + 1) * (SPHERE_SLICES + 1) * 6] = {};

            mesh = generateSphere(
                SPHERE_RADIUS, SPHERE_STACKS, SPHERE_SLICES, VERTICES, ARR_LENGTH(VERTICES), INDICES, ARR_LENGTH(INDICES));

            break;
        }
        case MeshType::Grid:
        {
            static constexpr auto GRID_X = 100;
            static constexpr auto GRID_Y = 100;
            static constexpr auto GRID_SPACING = 5.f;

            static Vertex VERTICES[GRID_X * GRID_Y] = {};
            static u32 INDICES[GRID_X * GRID_Y * 4] = {};

            mesh = generateGrid(GRID_X, GRID_Y, GRID_SPACING, VERTICES, ARR_LENGTH(VERTICES), INDICES, ARR_LENGTH(INDICES));

            break;
        }
        default: LOGIC_ERROR();
    }

    mesh.type = type;

    return mesh;
}

Mesh loadMesh(Asset const& asset, Arena& permanentMemory, Arena& tempMemory)
{
    ENSURE(asset.type == AssetType::ObjMesh);

    auto data = arenaAlloc(tempMemory, asset.size, sizeof(u8));
    memcpy(data, asset.data, asset.size);

    size_t posCount = 0;
    size_t normalCount = 0;
    size_t uvCount = 0;
    size_t verticesCount = 0;

    auto line = strtok((char*)data, "\n");
    while (line)
    {
        char lineHeader[256]{};
        (void)sscanf(line, "%s", lineHeader);

        auto isPosLine = strcmp(lineHeader, "v") == 0;
        auto isNormalLine = strcmp(lineHeader, "vn") == 0;
        auto isUVLine = strcmp(lineHeader, "vt") == 0;
        auto isIndexLine = strcmp(lineHeader, "f") == 0;

        if (isPosLine)
            posCount++;
        else if (isNormalLine)
            normalCount++;
        else if (isUVLine)
            uvCount++;
        else if (isIndexLine)
            verticesCount++;

        line = strtok(nullptr, "\n");
    }

    memcpy(data, asset.data, asset.size);

    auto posBuffer = arenaAllocArray<vec3>(tempMemory, posCount);
    auto normalBuffer = arenaAllocArray<vec3>(tempMemory, normalCount);
    auto uvBuffer = arenaAllocArray<vec2>(tempMemory, uvCount);

    auto posIndicesBuffer = arenaAllocArray<u32>(tempMemory, verticesCount * 3);
    auto normalIndicesBuffer = arenaAllocArray<u32>(tempMemory, verticesCount * 3);
    auto uvIndicesBuffer = arenaAllocArray<u32>(tempMemory, verticesCount * 3);

    line = strtok((char*)data, "\n");
    for (size_t posIdx = 0, normalIdx = 0, uvIdx = 0, indexIdx = 0; line;)
    {
        char lineHeader[256]{};
        (void)sscanf(line, "%s", lineHeader);

        auto isPosLine = strcmp(lineHeader, "v") == 0;
        auto isNormalLine = strcmp(lineHeader, "vn") == 0;
        auto isUVLine = strcmp(lineHeader, "vt") == 0;
        auto isIndexLine = strcmp(lineHeader, "f") == 0;

        if (isPosLine)
        {
            auto& vec = posBuffer[posIdx];
            const auto matches = sscanf(line + 2, "%f %f %f", &vec.x, &vec.y, &vec.z);
            ENSURE(matches == 3);
            posIdx++;
        }
        else if (isNormalLine)
        {
            auto& vec = normalBuffer[normalIdx];
            const auto matches = sscanf(line + 2, "%f %f %f", &vec.x, &vec.y, &vec.z);
            ENSURE(matches == 3);
            normalIdx++;
        }
        else if (isUVLine)
        {
            auto& vec = uvBuffer[uvIdx];
            const auto matches = sscanf(line + 2, "%f %f", &vec.x, &vec.y);
            ENSURE(matches == 2);
            uvIdx++;
        }
        else if (isIndexLine)
        {
            u32 posIndices[3]{}, normalIndices[3]{}, uvIndices[3]{};
            const auto matches = sscanf(line + 2,
                "%u/%u/%u %u/%u/%u %u/%u/%u",
                &posIndices[0],
                &normalIndices[0],
                &uvIndices[0],
                &posIndices[1],
                &normalIndices[1],
                &uvIndices[1],
                &posIndices[2],
                &normalIndices[2],
                &uvIndices[2]);
            ENSURE(matches == 9);

            posIndicesBuffer[indexIdx * 3 + 0] = posIndices[0];
            posIndicesBuffer[indexIdx * 3 + 1] = posIndices[1];
            posIndicesBuffer[indexIdx * 3 + 2] = posIndices[2];
            normalIndicesBuffer[indexIdx * 3 + 0] = normalIndices[0];
            normalIndicesBuffer[indexIdx * 3 + 1] = normalIndices[1];
            normalIndicesBuffer[indexIdx * 3 + 2] = normalIndices[2];
            uvIndicesBuffer[indexIdx * 3 + 0] = uvIndices[0];
            uvIndicesBuffer[indexIdx * 3 + 1] = uvIndices[1];
            uvIndicesBuffer[indexIdx * 3 + 2] = uvIndices[2];

            indexIdx++;
        }

        line = strtok(nullptr, "\n");
    }

    verticesCount *= 3;

    auto vertices = arenaAllocArray<Vertex>(permanentMemory, verticesCount);

    for (size_t i = 0; i < verticesCount; ++i)
    {
        const auto posIdx = posIndicesBuffer[i];
        vertices[i].pos = posBuffer[posIdx - 1];
        const auto normalIdx = normalIndicesBuffer[i];
        vertices[i].normal = normalBuffer[normalIdx - 1];
        // vertices[i].uv = uvIndicesBuffer[uvIndicesBuffer[i - 1]];
    }

    Mesh mesh{};

    mesh.vertices = vertices;
    mesh.verticesCount = verticesCount;
    mesh.isIndexed = false;
    mesh.type = MeshType::Custom;

    return mesh;
}
