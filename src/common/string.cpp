#include "string.hpp"
#include "memory.hpp"

String strFindReverse(String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    String result{};
    if (haystack.length == 0 || haystack.data == nullptr || needle.length == 0 || needle.data == nullptr)
        return result;

    ptrdiff_t haystackSearchIdx = 0;
    size_t needleFound = 0;
    for (ptrdiff_t haystackIdx = haystack.length - 1; haystackIdx >= 0 && needleFound < needle.length; --haystackIdx)
    {
        haystackSearchIdx = haystackIdx;
        for (ptrdiff_t needleIdx = needle.length - 1; haystackSearchIdx >= 0 && needleIdx >= 0;)
        {
            const auto needleCharFound = needle[needleIdx] == haystack[haystackSearchIdx];
            needleFound += needleCharFound;
            needleFound *= needleCharFound;
            needleIdx -= 1 - needle.length * !needleCharFound;  // step 1 or exit if needle chain broke
            haystackSearchIdx -= 1 * !needleCharFound;
        }
    }

    if (needleFound != needle.length)
        return result;

    result.length = returnOnlyOccurence ? needle.length : (haystack.length - haystackSearchIdx);
    result.data = haystack.data + haystackSearchIdx + resultOffset;
    return result;
}

String strFind(String haystack, String needle, bool returnOnlyOccurence, ptrdiff_t resultOffset)
{
    String result{};
    if (haystack.length == 0 || haystack.data == nullptr || needle.length == 0 || needle.data == nullptr)
        return result;

    size_t haystackSearchIdx = 0;
    size_t needleFound = 0;
    for (size_t haystackIdx = 0; haystackIdx < haystack.length && needleFound < needle.length; ++haystackIdx)
    {
        haystackSearchIdx = haystackIdx;
        for (size_t needleIdx = 0; haystackSearchIdx < haystack.length && needleIdx < needle.length;)
        {
            const auto needleCharFound = needle[needleIdx] == haystack[haystackSearchIdx];
            needleFound += needleCharFound;
            needleFound *= needleCharFound;
            needleIdx += 1 + needle.length * !needleCharFound;  // step 1 or exit if needle chain broke
            haystackSearchIdx += 1 * !needleCharFound;
        }
    }

    if (needleFound != needle.length)
        return result;

    result.length = returnOnlyOccurence ? needle.length : (haystack.length - haystackSearchIdx);
    result.data = haystack.data + haystackSearchIdx + resultOffset;
    return result;
}

String strClone(const char* src, Arena& memory)
{
    if (!src)
        return {};

    String cloned;
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

    String cloned;
    cloned.length = other.length;
    cloned.data = arenaAlloc<char>(memory, cloned.length + 1);

    memcpy(cloned.data, other.data, cloned.length);
    cloned.data[cloned.length] = '\0';

    return cloned;
}

String strAppend(String const& s1, String const& s2, Arena& memory)
{
    String result;
    result.data = arenaAlloc<char>(memory, s1.length + s2.length + 1);
    result.length = s1.length + s2.length;
    memcpy(result.data, s1.data, s1.length);
    memcpy(result.data + s1.length, s2.data, s2.length);
    result.data[result.length] = '\0';
    return result;
}
