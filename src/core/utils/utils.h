#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <sstream>
#include <vector>

inline std::vector<std::string> splitCommand(const char* command) {
    std::vector<std::string> parts;
    std::istringstream iss(command);
    std::string word;
    while (iss >> word) parts.push_back(word);
    return parts;
}

#endif // UTILS_H