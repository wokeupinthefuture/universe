#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

#include "lib/common.hpp"

#define TRIVIAL_TEMPLATE_T(t) \
    template <typename t>     \
        requires std::is_trivial_v<t>

#define TRIVIAL_TEMPLATE_T1_T2(t1, t2)  \
    template <typename t1, typename t2> \
        requires std::is_trivial_v<t1> && std::is_trivial_v<t2>

TRIVIAL_TEMPLATE_T(T)
struct HeapArray
{
    T* data{};
    size_t size{};
    size_t capacity{};

    const T* begin() const { return data; }
    const T* end() const { return data + size; }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
};

TRIVIAL_TEMPLATE_T(T)
void arrayAlloc(HeapArray<T>& array, size_t size)
{
    const auto capacity = std::max<size_t>(size, 1);

    if (array.data != nullptr)
    {
        auto newData = (T*)std::realloc(array.data, capacity * sizeof(T));
        if (!newData)
        {
            std::fprintf(stderr, "array realloc failed in arrayAlloc");
            return;
        }
        array.data = newData;
        array.capacity = capacity;
    }
    else
    {
        array.data = (T*)std::calloc(capacity, sizeof(T));
        array.capacity = capacity;
    }

    array.size = size;
}

TRIVIAL_TEMPLATE_T(T)
void arrayFree(HeapArray<T>& array)
{
    if (!array.data)
        return;

    free(array.data);
    array.data = nullptr;
    array.size = 0;
    array.capacity = 0;
}

TRIVIAL_TEMPLATE_T(T)
void arrayPush(HeapArray<T>& array, T value)
{
    if (!array.data)
    {
        arrayAlloc(array, 0);
    }
    else if (array.size >= array.capacity)
    {
        auto newCapacity = array.capacity * 3 / 2;
        if (newCapacity <= array.capacity)
            newCapacity++;
        array.capacity = newCapacity;

        auto newData = (T*)std::realloc(array.data, array.capacity * sizeof(T));
        if (!newData)
        {
            std::fprintf(stderr, "array realloc failed in arrayPush");
            return;
        }
        array.data = newData;
    }

    array.data[array.size++] = value;
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
void arrayClear(HeapArray<T>& array)
{
    memset(array.data, 0, array.capacity * sizeof(T));
    array.size = 0;
}

TRIVIAL_TEMPLATE_T(T)
void arrayReserve(HeapArray<T>& array, size_t capacity)
{
    if (capacity > array.capacity)
    {
        auto newData = (T*)std::realloc(array.data, capacity * sizeof(T));
        if (!newData)
        {
            std::fprintf(stderr, "array realloc failed in arrayReserve");
            return;
        }
        array.data = newData;
        array.capacity = capacity;
    }
    else
    {
        std::fprintf(stderr, "array reserve cannot shrink");
    }
}

TRIVIAL_TEMPLATE_T(T)
T* arrayLast(HeapArray<T> array)
{
    return array.data + array.size - 1;
}
