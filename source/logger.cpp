#include <strings.h>
#include <stdarg.h>

#include "logger.h"

FILE * m_file = nullptr;
bool m_enabled = false;

void Logger_Initialize()
{
    #if __VITA__
    m_file = freopen("ux0:/pico.log", "w", stderr);
    #else
    m_file = freopen("pico.log", "w", stderr);
    #endif
    m_enabled = true;
}

void Logger_LogOutput(const char * func, size_t line, const char * format, ...)
{
    if (!m_enabled || !m_file)
        return;

    va_list args;
    va_start(args, format);

    fprintf(m_file, "%s:%zu:\n", func, line);
    vfprintf(m_file, format, args);
    fprintf(m_file, "\n\n");

    fflush(m_file);
}

void Logger_Write(const char * format, ...)
{
    if (!m_enabled || !m_file)
        return;
        
    va_list args;
    va_start(args, format);

    vfprintf(m_file, format, args);

    fflush(m_file);
}

void Logger_WriteUnformatted(const char * message)
{
    if (!m_enabled || !m_file)
        return;
        
    fprintf(m_file, "%s", message);

    fflush(m_file);
}

void Logger_Exit()
{
    if (!m_enabled || !m_file)
        return;

    fclose(m_file);
}
