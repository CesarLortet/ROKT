#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <cctype>

// Cette fonction retire les espaces en début/fin et le ';' terminal s'il existe.
static std::string trim(const std::string &s) {
    auto start = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); }).base();
    std::string result = (start < end) ? std::string(start, end) : "";
    if (!result.empty() && result.back() == ';')
        result.pop_back();
    return result;
}

#endif // UTILS_H
