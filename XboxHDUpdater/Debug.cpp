#include "Debug.h"

#include <xtl.h>
#include <string>

void Debug::Print(std::string message, ...)
{
    char buffer[1024];

    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, 1024, message.c_str(), arglist);
    va_end(arglist);

    buffer[1024 - 1] = '\0';
    OutputDebugStringA(buffer);
}
