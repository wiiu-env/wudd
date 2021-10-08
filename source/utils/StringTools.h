/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#pragma once

#include <vector>
#include <string>
#include <wut_types.h>
#include <algorithm>
#include <cctype>

class StringTools {
public:
    static BOOL EndsWith(const std::string &a, const std::string &b);

    static const char *byte_to_binary(int32_t x);

    static std::string removeCharFromString(std::string &input, char toBeRemoved);

    static const char *fmt(const char *format, ...);

    static const wchar_t *wfmt(const char *format, ...);

    static int32_t strprintf(std::string &str, const char *format, ...);

    static std::string strfmt(const char *format, ...);

    static BOOL char2wchar_t(const char *src, wchar_t *dest);

    static int32_t strtokcmp(const char *string, const char *compare, const char *separator);

    static int32_t strextcmp(const char *string, const char *extension, char seperator);

    static const char *FullpathToFilename(const char *path) {
        if (!path)
            return path;

        const char *ptr = path;
        const char *Filename = ptr;

        while (*ptr != '\0') {
            if (ptr[0] == '/' && ptr[1] != '\0')
                Filename = ptr + 1;

            ++ptr;
        }

        return Filename;
    }

    static void RemoveDoubleSlashs(std::string &str) {
        uint32_t length = str.size();

        //! clear path of double slashes
        for (uint32_t i = 1; i < length; ++i) {
            if (str[i - 1] == '/' && str[i] == '/') {
                str.erase(i, 1);
                i--;
                length--;
            }
        }
    }

    static std::vector<std::string> stringSplit(const std::string &value, const std::string &splitter);

    // https://stackoverflow.com/a/19839371
    static bool findStringIC(const std::string &strHaystack, const std::string &strNeedle);

    // https://stackoverflow.com/a/3418285
    static bool replace(std::string &str, const std::string &from, const std::string &to);

    static bool strCompareIC(const std::string &str1, const std::string &str2);
};
