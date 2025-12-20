#pragma once
#include <iostream>
#include <string>

std::string Input(const std::string &prompt_)
{
    std::string str = "";
    std::cout << prompt_.c_str();
    std::getline(std::cin, str);
    return str;
}