#ifndef HEADERPARSER_H
#define HEADERPARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

class HeaderParser {
public:
    // Parses raw HTTP headers string into vector of key-value pairs
    static std::vector<std::pair<std::string, std::string>> parseHeaders(const std::string& headers);
    static bool iequals(const std::string& a, const std::string& b);

private:
    static bool ichar_equals(char a, char b);
};

#endif // HEADERPARSER_H

// IMPLEMENTATION:

bool HeaderParser::ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool HeaderParser::iequals(const std::string& a, const std::string& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), ichar_equals);
}

std::vector<std::pair<std::string, std::string>> HeaderParser::parseHeaders(const std::string& headers) {
    std::vector<std::pair<std::string, std::string>> result;
    std::istringstream stream(headers);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);

        // Trim helpers
        auto ltrim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
        };

        auto rtrim = [](std::string& s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        };

        ltrim(key);
        rtrim(key);
        ltrim(value);
        rtrim(value);

        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });

        result.emplace_back(key, value);
    }

    return result;
}