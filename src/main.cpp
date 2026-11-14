#include "game.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
void enableVirtualTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
}
#else
void enableVirtualTerminal() {}
#endif

int main() {
    enableVirtualTerminal();

    CrosswordGame game;
    if (!game.initialize("data/dictionary.txt")) {
        std::cerr << "Failed to initialize game with dictionary!\n";
        return 1;
    }

    game.run();

    return 0;
}
