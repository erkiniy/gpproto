//
// Created by ProDigital on 7/30/18.
//

#ifndef GPPROTO_STRINGUTILS_H
#define GPPROTO_STRINGUTILS_H

#include <string>
#include <vector>

namespace gpproto {
    class StringUtils {
    public:
        static std::vector<std::string> Split(const std::string &text, char delimeter);

        static std::string Replace(std::string string, const std::string& from, const std::string& to);

        static bool StartsWith(const std::string &fullString, std::string const& prefix);

        static bool EndsWith(const std::string &fullString, std::string const& suffix);

    };
}

#endif //GPPROTO_STRINGUTILS_H
