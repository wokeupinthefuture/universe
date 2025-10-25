#include "string.hpp"
#include "memory.hpp"
#include <cassert>

static constexpr auto s_isEqual = [](char a, char b) { return a == b; };
static constexpr auto s_isNotEqual = [](char a, char b) { return a != b; };

static String s_strFind(
    bool (*comparisonFunc)(char, char), String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    String result{};
    if (haystack.length == 0 || haystack.data == nullptr || needle.length == 0 || needle.data == nullptr)
        return result;

    if (haystack.length < needle.length)
        return result;

    size_t haystackTempIdx = 0;
    size_t needleFound = 0;
    for (size_t haystackIdx = 0; haystackIdx < haystack.length; ++haystackIdx)
    {
        haystackTempIdx = haystackIdx;
        for (size_t needleIdx = 0; needleIdx < needle.length && haystackTempIdx < haystack.length;)
        {
            const auto haystackChar = haystack[haystackTempIdx];
            if (haystackChar == '\0')
            {
                haystackIdx = haystack.length;
            }
            else
            {
                if (const auto needleCharFound = comparisonFunc(needle[needleIdx], haystackChar); needleCharFound)
                {
                    needleFound++;

                    if (needleFound < needle.length)
                    {
                        haystackTempIdx++;
                        needleIdx++;
                    }
                    else
                    {
                        haystackIdx = haystack.length;
                        needleIdx = needle.length;
                    }
                }
                else
                {
                    needleFound = 0;
                    needleIdx = needle.length;
                }
            }
        }
    }

    if (needleFound != needle.length)
        return result;

    result.length = (returnOnlyOccurence ? needle.length : (haystack.length - haystackTempIdx)) - resultOffset;
    result.data = haystack.data + haystackTempIdx + resultOffset;
    return result;
}

static String s_strFindReverse(
    bool (*comparisonFunc)(char, char), String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    String result{};
    if (haystack.length == 0 || haystack.data == nullptr || needle.length == 0 || needle.data == nullptr)
        return result;

    if (haystack.length < needle.length)
        return result;

    ptrdiff_t haystackTempIdx = 0;
    size_t needleFound = 0;
    for (ptrdiff_t haystackIdx = haystack.length - 1; haystackIdx >= 0; --haystackIdx)
    {
        haystackTempIdx = haystackIdx;
        for (ptrdiff_t needleIdx = needle.length - 1; needleIdx >= 0 && haystackTempIdx >= 0;)
        {
            const auto haystackChar = haystack[haystackTempIdx];
            if (haystackChar == '\0')
            {
                haystackIdx = -1;
            }
            else
            {
                if (const auto needleCharFound = comparisonFunc(needle[needleIdx], haystackChar); needleCharFound)
                {
                    needleFound++;

                    if (needleFound < needle.length)
                    {
                        haystackTempIdx--;
                        needleIdx--;
                    }
                    else
                    {
                        haystackIdx = -1;
                        needleIdx = -1;
                    }
                }
                else
                {
                    needleFound = 0;
                    needleIdx = -1;
                }
            }
        }
    }

    if (needleFound != needle.length)
        return result;

    result.length = (returnOnlyOccurence ? needle.length : (haystack.length - haystackTempIdx)) - resultOffset;
    result.data = haystack.data + haystackTempIdx + resultOffset;
    return result;
}

String strFind(String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    return s_strFind(s_isEqual, haystack, needle, returnOnlyOccurence, resultOffset);
}

String strFindNot(String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    return s_strFind(s_isNotEqual, haystack, needle, returnOnlyOccurence, resultOffset);
}

String strFindReverse(String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    return s_strFindReverse(s_isEqual, haystack, needle, returnOnlyOccurence, resultOffset);
}

String strFindNotReverse(String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    return s_strFindReverse(s_isNotEqual, haystack, needle, returnOnlyOccurence, resultOffset);
}

String strClone(const char* src, Arena& memory)
{
    if (!src)
        return {};

    String cloned{};
    cloned.length = strlen(src);
    cloned.data = arenaAlloc<char>(memory, cloned.length + 1);

    memcpy(cloned.data, src, cloned.length);
    cloned.data[cloned.length] = '\0';

    return cloned;
}

String strClone(String const& other, Arena& memory)
{
    if (!other)
        return {};

    String cloned{};
    cloned.length = other.length;
    cloned.data = arenaAlloc<char>(memory, cloned.length + 1);

    memcpy(cloned.data, other.data, cloned.length);
    cloned.data[cloned.length] = '\0';

    return cloned;
}

String strAppend(String const& s1, String const& s2, Arena& memory)
{
    String result{};
    result.data = arenaAlloc<char>(memory, s1.length + s2.length + 1);
    result.length = s1.length + s2.length;

    memcpy(result.data, s1.data, s1.length);
    memcpy(result.data + s1.length, s2.data, s2.length);
    result.data[result.length] = '\0';

    return result;
}

String strFindUntil(String src, String substr, bool inclusive)
{
    String result{};

    const auto found = strFind(src, substr);
    if (!found)
        return result;

    result.data = src.data;
    result.length = ((uintptr_t)found.data - (uintptr_t)src.data) / sizeof(char);
    result.length += (int)inclusive;

    return result;
}

String strCopy(String const& src, String& dst)
{
    String result{};
    assert(dst.length >= src.length);
    assert(src && dst);
    memcpy(dst.data, src.data, src.length);
    return result;
}

String strCopy(String const& src, char* dst, size_t dstLength)
{
    String result{};
    assert(dstLength >= src.length);
    assert(src && dst != nullptr && dstLength != 0);
    memcpy(dst, src.data, src.length);
    return result;
}

HeapArray<String> strSplit(String src, String delim, Arena& memory, bool delimInclusive)
{
    HeapArray<String> result{};

    if (!src || !delim)
        return result;

    size_t count = 0;
    auto temp = src;
    for (size_t charsRead = 0; charsRead < src.length;)
    {
        auto str = strFindUntil(temp, delim, delimInclusive);
        if (!str)
            break;
        count++;

        temp.data += str.length + (int)!delimInclusive;
        charsRead += str.length + (int)!delimInclusive;
    }

    if (count == 0)
        return result;

    result.data = arenaAlloc<String>(memory, count);
    result.size = count;

    temp = src;
    for (size_t i = 0; i < count; ++i)
    {
        auto str = strFindUntil(temp, delim, delimInclusive);
        result[i] = strClone(str, memory);
        temp.data += str.length + (int)!delimInclusive;
    }

    return result;
}
