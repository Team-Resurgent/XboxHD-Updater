#include "TerminalBuffer.h"
#include "Drawing.h"

namespace
{
    char s_buffer[TerminalBuffer::Rows][TerminalBuffer::Cols];
    int s_cursor_x = 0;
    int s_cursor_y = 0;
}

void TerminalBuffer::Clear()
{
    for (int row = 0; row < Rows; row++)
    {
        for (int col = 0; col < Cols; col++)
        {
            s_buffer[row][col] = ' ';
        }
    }
    s_cursor_x = 0;
    s_cursor_y = 0;
}

void TerminalBuffer::SetCursor(int x, int y)
{
    s_cursor_x = (x >= 0 && x < Cols) ? x : 0;
    s_cursor_y = (y >= 0 && y < Rows) ? y : 0;
}

void TerminalBuffer::Write(const char* text)
{
    if (!text)
    {
        return;
    }

    for (const char* p = text; *p; p++)
    {
        if (*p == '\n')
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= Rows)
            {
                ScrollUp();
                s_cursor_y = Rows - 1;
            }
            continue;
        }

        if (s_cursor_x >= Cols)
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= Rows)
            {
                ScrollUp();
                s_cursor_y = Rows - 1;
            }
        }

        if (s_cursor_y >= 0 && s_cursor_y < Rows && s_cursor_x >= 0 && s_cursor_x < Cols)
        {
            s_buffer[s_cursor_y][s_cursor_x] = *p;
        }
        s_cursor_x++;
    }

    Drawing::DrawTerminal(GetBuffer(), 0xff00ff00);
}

void TerminalBuffer::ScrollUp()
{
    for (int row = 0; row < Rows - 1; row++)
    {
        for (int col = 0; col < Cols; col++)
        {
            s_buffer[row][col] = s_buffer[row + 1][col];
        }
    }
    for (int col = 0; col < Cols; col++)
    {
        s_buffer[Rows - 1][col] = ' ';
    }
}

const char* TerminalBuffer::GetBuffer()
{
    return &s_buffer[0][0];
}

int TerminalBuffer::GetCols()
{
    return Cols;
}

int TerminalBuffer::GetRows()
{
    return Rows;
}

int TerminalBuffer::GetCursorX()
{
    return s_cursor_x;
}

int TerminalBuffer::GetCursorY()
{
    return s_cursor_y;
}
