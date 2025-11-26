#ifndef APPLICATION_H
#define APPLICATION_H
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "utils/debug.h"
#include "utils/Input.h"
#include "utils/database.h"
#include "utils/utils.h"

class Application
{
public:
    Application() = default;
    Application(const std::string &title);
    bool ShouldClose();
    
    void Initialize();
    void Update();
    void ProcessCommand(const char* command);

private:
    std::string title;
    std::string loggedInUser;
    bool isRunning = true;
    Database* db;
};

#endif // APPLICATION_H