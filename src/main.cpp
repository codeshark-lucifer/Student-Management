#include "core/application.h"

int main()
{
    Application *app;
    app->Initialize();
    while (!app->ShouldClose())
    {
        app->Update();
    }

    return 0;
}