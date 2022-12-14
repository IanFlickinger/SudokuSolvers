#include <windows.h>
#include "puzzle.h"

#include "display.h"

#include <iostream>

// #define DEBUG_ENABLED
// #define DEBUG_ENABLED_VERBOSE
#include "debugging.h"


namespace Display {

void showPuzzle(const Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("Display::showPuzzle(const Puzzle&)")
    for (int r = 0; r < puzzle.getSize(); r++) {
        if (r % puzzle.getSizeSqrt() == 0) {
            std::cout << '+';
            for (int k = 0; k < puzzle.getSizeSqrt(); k++) {
                for (int i=0; i < puzzle.getSizeSqrt(); i++) {
                    std::cout << "---";
                }
                std::cout << '+';
            }
            std::cout << '\n';
        }
        for (int c = 0; c < puzzle.getSize(); c++) {
            if (c % puzzle.getSizeSqrt() == 0) std::cout << "|";
            unsigned char value = static_cast<int>(puzzle.getValue(r, c)) + '0';
            if (value == '0') value = ' ';
            std::cout << ' ' << value << ' ';
        }
        std::cout << "|\n";
    }
    std::cout << '+';
    for (int k = 0; k < puzzle.getSizeSqrt(); k++) {
        for (int i=0; i < puzzle.getSizeSqrt(); i++) {
            std::cout << "---";
        }
        std::cout << '+';
    }
    std::cout << std::endl;
    DEBUG_FUNC_END()
}

void clear() { 
    char fill = ' ';
    COORD tl = {0,0};
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);   
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

}