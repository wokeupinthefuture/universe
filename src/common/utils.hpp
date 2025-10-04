#pragma once

#include <utility>

template <typename F>
struct privDefer
{
    F f;
    privDefer(F f) : f(f) {}
    ~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f)
{
    return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })

#define TRIVIAL_TEMPLATE(t) \
    template <typename t>   \
        requires std::is_trivial_v<t>

#include <cstdint>
using u8 = std::uint8_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

#define Kilobytes(count) ((count) * 1024)
#define Megabytes(count) (Kilobytes(count) * 1024)
#define Gigabytes(count) (Megabytes(count) * 1024)

#define setBit(number, bit) (number) | (1 << bit)
#define clearBit(number, bit) (number) & ~(1 << bit)
#define checkBit(number, bit) ((number) >> bit) & 1

template <typename T>
void memset(T& memory, int value)
{
    std::memset((void*)&memory, value, sizeof(T));
}

#define ARR_LENGTH(arr) sizeof(arr) / sizeof(arr[0])

template <typename T, typename Predicate>
T* find(T* array, size_t arrayLength, Predicate&& pred)
{
    for (auto iter = array, end = array + arrayLength; iter != end; ++iter)
    {
        if (pred(*iter))
            return iter;
    }
    return nullptr;
}

#include <chrono>

inline float getElapsedTime()
{
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = now - start;
    return duration.count();  // Returns elapsed time in seconds
}
