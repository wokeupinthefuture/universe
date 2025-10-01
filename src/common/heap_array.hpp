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

    const T* begin() const { return data; }
    const T* end() const { return data + size; }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
};

TRIVIAL_TEMPLATE_T(T)
void arrayClear(HeapArray<T>& array)
{
    memset(array.data, 0, MAX_HEAP_ARRAY_SIZE);
    array.size = 0;
}

TRIVIAL_TEMPLATE_T(T)
void arrayAlloc(HeapArray<T>& array, Arena& arena)
{
    array.data = arenaAllocArray<T>(arena, MAX_HEAP_ARRAY_SIZE / sizeof(T));
    arrayClear(array);
}

TRIVIAL_TEMPLATE_T(T)
T* arrayPush(HeapArray<T>& array, T value)
{
    ENSURE(array.size < (MAX_HEAP_ARRAY_SIZE / sizeof(T)));
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

TRIVIAL_TEMPLATE_T(T)
T* arrayGet(HeapArray<T> array, size_t idx)
{
    return &array.data[idx];
}
