#pragma once

#include <cstring>

struct String
{
    char* data;
    size_t length;

    char* begin() { return data; }
    char* end() { return data + length; }
    const char* begin() const { return data; }
    const char* end() const { return data + length; }

    char& operator[](size_t index) { return data[index]; }
    const char& operator[](size_t index) const { return data[index]; }

    operator bool() const { return data != nullptr && length > 0; }

    bool operator==(const String& other) const
    {
        if (length != other.length)
            return false;
        if (!*this && !other)
            return true;
        return strncmp(data, other.data, length) == 0;
    }

    bool operator==(const char* otherStr) const
    {
        if (!*this)
            return otherStr == nullptr;
        return strncmp(data, otherStr, length) == 0;
    }
};

inline bool operator==(String& str, const char* otherStr)
{
    if (!str)
        return otherStr == nullptr;
    return strncmp(str.data, otherStr, str.length) == 0;
}

struct Arena;
String strFind(String needle, String haystack, bool returnOnlyOccurence = true, ptrdiff_t resultOffset = 0);
String strFindReverse(String needle, String haystack, bool returnOnlyOccurence = true, ptrdiff_t resultOffset = 0);
String strClone(const char* other, Arena& memory);
String strClone(String const& other, Arena& memory);
String strAppend(String const& s1, String const& s2, Arena& memory);

#define strFromNullTerm(str)                      \
    String                                        \
    {                                             \
        .data = (char*)str, .length = strlen(str) \
    }
#define strFromLiteral(str) String{.data = (char*)str, .length = sizeof(str) - 1}
