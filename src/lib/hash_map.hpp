#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

inline uint32_t hashFunction(const void* data, size_t sizeOfData, uint32_t hash = 2166136261U)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < sizeOfData; ++i)
        hash = (hash ^ bytes[i]) * 16777619U;
    return hash;
}

#define TRIVIAL_TEMPLATE_KV(k, v)     \
    template <typename K, typename V> \
        requires std::is_trivial_v<k> && std::is_trivial_v<v>

static constexpr float LOAD_FACTOR = 0.7f;
static constexpr size_t INITIAL_CAPACITY = 256;

TRIVIAL_TEMPLATE_KV(K, V)
struct HashMap
{
    struct Entry
    {
        K key;
        V value;
        uint32_t hash;
    };

    Entry* entries{};
    size_t size{};
    size_t capacity{};

    Entry* begin() { return entries; }
    const Entry* begin() const { return entries; }
    Entry* end() { return entries + size; }
    const Entry* end() const { return entries + size; }
};

TRIVIAL_TEMPLATE_KV(K, V)
void mapAlloc(HashMap<K, V>& map, size_t capacity = INITIAL_CAPACITY)
{
    capacity = capacity == 0 ? 1 : capacity;
    map.entries = (typename HashMap<K, V>::Entry*)std::calloc(capacity, sizeof(HashMap<K, V>::Entry));
    map.size = 0;
    map.capacity = capacity;
}

TRIVIAL_TEMPLATE_KV(K, V)
void mapInsert(HashMap<K, V>& map, K key, V&& value)
{
    if (!map.entries)
        mapAlloc(map);

    const auto hash = hashFunction((void*)&key, sizeof(K));
    map.entries[hash & map.capacity - 1] = {key, value, hash};
}

TRIVIAL_TEMPLATE_KV(K, V)
V* mapAt(HashMap<K, V>& map, K key)
{
    if (!map.entries || map.size == 0)
        return nullptr;

    const auto hash = hashFunction((void*)&key, sizeof(K));
    return &map.entries[hash & map.capacity - 1].value;
}

TRIVIAL_TEMPLATE_KV(K, V)
void mapErase(HashMap<K, V>& map, K key)
{
    if (!map.entries)
        return;

    const auto hash = hashFunction((void*)&key, sizeof(K));
    map.entries[hash & map.capacity - 1] = {};
    map.size--;
}

TRIVIAL_TEMPLATE_KV(K, V)
void mapClear(HashMap<K, V>& map)
{
    if (map.entries)
        memset(map.entries, 0, map.capacity * sizeof(HashMap<K, V>::Entry));
    map.size = 0;
}

TRIVIAL_TEMPLATE_KV(K, V)
void mapFree(HashMap<K, V>& map)
{
    if (!map.entries)
        return;

    free(map.entries);
    map.entries = nullptr;
    map.size = 0;
    map.capacity = 0;
}
