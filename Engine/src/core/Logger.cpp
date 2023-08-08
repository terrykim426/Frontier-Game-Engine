#include "pch.h"
#include "core/Logger.h"
#include <stdarg.h>
#include <iostream>

namespace FGEngine
{
	void Logger::__Log(const char* file, int line, ELevel level, const char* format, ...)
	{
        const char* levelText = [&]
        {
            switch (level)
            {
            case ELevel::Info: return "Info";
            case ELevel::Debug: return "Debug";
            case ELevel::Warning: return "Warning";
            case ELevel::Error: return "Error";
            }
            return "Unsupported";
        }();

#if LOG_WITH_FILEPATH
        printf("[%s:%d, %s] ", file, line, levelText);
#endif

        va_list args;
        va_start(args, format);

        vprintf(format, args);
        va_end(args);

        printf("\n");
	}
}