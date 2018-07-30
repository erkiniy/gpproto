//
// Created by ProDigital on 7/30/18.
//

#include "StringUtils.h"
using namespace gpproto;

std::vector<std::string> StringUtils::Split(const std::string &text, char delimeter) {
    size_t start = 0;
    size_t end = 0;

    auto tokens = std::vector<std::string>();
    while((end = text.find(delimeter, start)) != std::string::npos)
    {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }

    tokens.push_back(text.substr(start));

    return tokens;
}

std::string StringUtils::Replace(std::string string, const std::string &from, const std::string &to) {
    size_t pos = 0;

    while((pos = string.find(from, pos)) != std::string::npos)
    {
        string.replace(pos, from.length(), to);
        pos += to.length();
    }

    return string;
}


bool StringUtils::StartsWith(const std::string &fullString, std::string const& prefix)
{
    if (fullString.length() >= prefix.length())
        return (0 == fullString.compare(0, prefix.length(), prefix));

    return false;
}

bool StringUtils::EndsWith(const std::string &fullString, std::string const& suffix)
{
    if (fullString.length() >= suffix.length())
        return (0 == fullString.compare(fullString.length() - suffix.length(), suffix.length(), suffix));

    return false;
}