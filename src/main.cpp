#include <iostream>
#include <application.hpp>
#include <exception>

int main()
{
    try
    {
        Application app;
        app.Run();
    }
    catch (const std::exception &e)
    {
        std::cout << "[Exception]: " << e.what() << std::endl;
    }
    return 0;
}