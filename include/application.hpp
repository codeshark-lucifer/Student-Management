#pragma once
#include <memory>
#include <string>

#include <database.hpp>

class Application
{
public:
    Application();
    ~Application();

    void Run();

private:
    bool running = false;
    bool authenticated = false;
    std::shared_ptr<Database> db;

    void PrintResult(const QueryResult& result);
};
