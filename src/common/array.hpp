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

TRIVIAL_TEMPLATE_T(T)
struct Array
{
    T* data;
    size_t size;
    size_t capacityBytes;
    size_t capacity;

    Array() = default;

    T* begin() { return data; }
    T* end() { return data + size; }
    const T* begin() const { return data; }
    const T* end() const { return data + size; }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    operator bool() const { return data != nullptr && size > 0; }
};

TRIVIAL_TEMPLATE_T(T)
void arrayClear(Array<T>& array, const char* tag = "array")
{
    logInfo("CLEARING array %s at 0x%llx, clearing %llu bytes to 0x%llx",
        tag,
        (uintptr_t)array.data,
        array.capacityBytes,
        (uintptr_t)array.data + array.capacityBytes);
    memset(array.data, 0, array.capacityBytes);
    array.size = 0;
}

TRIVIAL_TEMPLATE_T(T)
void arrayInit(Array<T>& array, size_t capacity, Arena& arena, const char* tag = "array")
{
    array.capacity = capacity;
    array.capacityBytes = array.capacity * sizeof(T);
    array.data = (T*)arenaAlloc(arena, array.capacityBytes, alignof(T));
    array.size = 0;
    logInfo("ARRAY_INIT: %s at 0x%llx", tag, array.data);
}

TRIVIAL_TEMPLATE_T(T)
T* arrayPush(Array<T>& array, T value)
{
    assert(array.size < array.capacity);
    array.data[array.size++] = value;
    return arrayLast(array);
}

TRIVIAL_TEMPLATE_T(T)
void arrayPop(Array<T>& array)
{
    if (array.size == 0)
        return;

    array.data[array.size--] = T{};
    array.size--;
}

TRIVIAL_TEMPLATE_T(T)
T* arrayFirst(Array<T> array)
{
    return array.data;
}

TRIVIAL_TEMPLATE_T(T)
T* arrayLast(Array<T> array)
{
    return array.data + array.size - 1;
}

TRIVIAL_TEMPLATE_T(T)
Array<T> arraySpan(Array<T> from, size_t begin, size_t size)
{
    Array<T> result;
    result.data = from.data + begin;
    result.size = size;
    return result;
}
