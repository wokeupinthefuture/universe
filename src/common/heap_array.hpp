#pragma once

#include <cstdio>
#include <type_traits>
#include "memory.hpp"

#define TRIVIAL_TEMPLATE_T(t) \
    template <typename t>     \
        requires std::is_trivial_v<t>

#define TRIVIAL_TEMPLATE_T1_T2(t1, t2)  \
    template <typename t1, typename t2> \
        requires std::is_trivial_v<t1> && std::is_trivial_v<t2>

static constexpr size_t MAX_HEAP_ARRAY_SIZE = Megabytes(10);

TRIVIAL_TEMPLATE_T(T)
struct HeapArray
{
    T* data;
    size_t size;

    HeapArray() = default;

    T* begin() { return data; }
    T* end() { return data + size; }
    const T* begin() const { return data; }
    const T* end() const { return data + size; }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
};

TRIVIAL_TEMPLATE_T(T)
void arrayClear(HeapArray<T>& array, const char* name = "array")
{
    logInfo("CLEARING array %s at 0x%llx, clearing %llu bytes to 0x%llx",
        name,
        (uintptr_t)array.data,
        MAX_HEAP_ARRAY_SIZE,
        (uintptr_t)array.data + MAX_HEAP_ARRAY_SIZE);
    memset(array.data, 0, MAX_HEAP_ARRAY_SIZE);
    array.size = 0;
}

TRIVIAL_TEMPLATE_T(T)
void arrayInit(HeapArray<T>& array, Arena& arena, const char* name = "array")
{
    array.data = (T*)arenaAlloc(arena, MAX_HEAP_ARRAY_SIZE, alignof(T));
    array.size = 0;
    logInfo("ARRAY_INIT: %s at 0x%llx", name, array.data);
}

TRIVIAL_TEMPLATE_T(T)
T* arrayPush(HeapArray<T>& array, T value)
{
    assert(array.size < MAX_HEAP_ARRAY_SIZE / sizeof(T));
    array.data[array.size++] = value;
    return arrayLast(array);
}

TRIVIAL_TEMPLATE_T(T)
void arrayPop(HeapArray<T>& array)
{
    if (array.size == 0)
        return;

    array.data[array.size--] = T{};
    array.size--;
}

TRIVIAL_TEMPLATE_T(T)
T* arrayFirst(HeapArray<T> array)
{
    return array.data;
}

TRIVIAL_TEMPLATE_T(T)
T* arrayLast(HeapArray<T> array)
{
    return array.data + array.size - 1;
}
