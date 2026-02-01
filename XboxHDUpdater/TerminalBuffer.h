#pragma once

#include "External.h"

class TerminalBuffer
{
public:
    static const int Cols = 40;
    static const int Rows = 30;

    static void Clear();
    static void SetCursor(int x, int y);
    static void Write(const char* text);
    static void ScrollUp();

    static const char* GetBuffer();
    static int GetCols();
    static int GetRows();
    static int GetCursorX();
    static int GetCursorY();
};
