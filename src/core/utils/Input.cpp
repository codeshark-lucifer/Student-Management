#include <iostream>
#include <string>

std::string Input(const char* text) {
    std::string input;
    std::cout << text;
    std::getline(std::cin, input);
    std::cout << "\n";
    return input;
}