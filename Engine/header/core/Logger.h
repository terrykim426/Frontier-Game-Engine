#pragma once

#include "core/Core.h"

#define LogInfo(format, ...) FGEngine::Logger::__Log(__FILE__, __LINE__, FGEngine::Logger::ELevel::Info, format, ##__VA_ARGS__);
#define LogDebug(format, ...) FGEngine::Logger::__Log(__FILE__, __LINE__, FGEngine::Logger::ELevel::Debug, format, ##__VA_ARGS__);
#define LogWarning(format, ...) FGEngine::Logger::__Log(__FILE__, __LINE__, FGEngine::Logger::ELevel::Warning, format, ##__VA_ARGS__);
#define LogError(format, ...) FGEngine::Logger::__Log(__FILE__, __LINE__, FGEngine::Logger::ELevel::Error, format, ##__VA_ARGS__);
#define LogAssert(format, ...) \
    LogError(format, ##__VA_ARGS__); \
    abort();

#define Ensure(condition, format, ...) if(!(condition)){LogWarning(format, ##__VA_ARGS__)};
#define Check(condition, format, ...) if(!(condition)){LogAssert(format, ##__VA_ARGS__)};
#define NoEntry(format, ...) LogAssert(format, ##__VA_ARGS__)

namespace FGEngine
{
	class ENGINE_API Logger
	{
	public:
		enum class ELevel : unsigned int
		{
			Info,
			Debug,
			Warning,
			Error,
		};

	public:
		static void __Log(const char* file, int line, ELevel level, const char* format, ...);
	};

}