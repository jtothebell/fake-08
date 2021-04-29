#include <strings.h>
#include <stdarg.h>
#include <string>

#include "logger.h"

#define LOGGER_ENABLED false

FILE * m_file = nullptr;
bool m_enabled = false;

void Logger_Initialize(const char* pathPrefix)
{
    #if LOGGER_ENABLED
    std::string buf(pathPrefix);
    buf.append("pico.log");
    m_file = freopen(buf.c_str(), "w", stderr);
    m_enabled = true;
    #endif
}

void Logger_LogOutput(const char * func, size_t line, const char * format, ...)
{
    #if LOGGER_ENABLED
    if (!m_enabled || !m_file)
        return;

    va_list args;
    va_start(args, format);

    fprintf(m_file, "%s:%zu:\n", func, line);
    vfprintf(m_file, format, args);
    fprintf(m_file, "\n\n");

    fflush(m_file);
    #endif
}

void Logger_Write(const char * format, ...)
{
    #if LOGGER_ENABLED
    if (!m_enabled || !m_file)
        return;
        
    va_list args;
    va_start(args, format);

    vfprintf(m_file, format, args);

    fflush(m_file);
    #endif
}

void Logger_WriteUnformatted(const char * message)
{
    #if LOGGER_ENABLED
    if (!m_enabled || !m_file)
        return;
        
    fprintf(m_file, "%s", message);

    fflush(m_file);
    #endif
}

void Logger_Exit()
{
    #if LOGGER_ENABLED
    if (!m_enabled || !m_file)
        return;

    fclose(m_file);
    #endif
}
