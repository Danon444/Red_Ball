#include "Game.h"

#include <exception>
#ifdef _WIN32
#include <windows.h>
#else
#include <iostream>
#endif

int main()
{
    try
    {
        Game game;
        game.run();
    }
    catch (const std::exception& error)
    {
#ifdef _WIN32
        MessageBoxA(nullptr, error.what(), "Red Ball", MB_OK | MB_ICONERROR);
#else
        std::cerr << error.what() << '\n';
#endif
        return 1;
    }

    return 0;
}
