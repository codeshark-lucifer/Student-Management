#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <cstdarg>
#include <string>


enum class LogLevel { INFO, WARNING, ERROR };

class Debug
{
public:
    static void Logf(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...)
    {
        // Get current time
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream timeStream;
        timeStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S"); // e.g. 2025-11-25 14:35:22

        // Format message
        va_list args;
        va_start(args, fmt);
        std::vector<char> buffer(1024);
        vsnprintf(buffer.data(), buffer.size(), fmt, args);
        va_end(args);

        // Set color
        std::string colorCode;
        switch(level)
        {
            case LogLevel::INFO:    colorCode = "\033[0;37m"; break;
            case LogLevel::WARNING: colorCode = "\033[0;33m"; break;
            case LogLevel::ERROR:   colorCode = "\033[0;31m"; break;
        }

        // Level string
        std::string levelStr;
        switch(level)
        {
            case LogLevel::INFO:    levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR:   levelStr = "ERROR"; break;
        }

        // Output log
        std::cout << colorCode
                  << "[" << timeStream.str() << "] "
                  << "[" << levelStr << "] "
                  << file << ":\n" << line << " (" << func << "): "
                  << buffer.data()
                  << "\033[0m" << std::endl;
    }
};


#define LOG_INFO(fmt, ...) Debug::Logf(LogLevel::INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Debug::Logf(LogLevel::WARNING, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Debug::Logf(LogLevel::ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
