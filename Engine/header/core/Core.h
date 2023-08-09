#pragma once

#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif // ENGINE_EXPORTS
#else
#define ENGINE_API 
#endif // _WIN32


#include "FGEngine.h"