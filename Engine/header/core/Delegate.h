#pragma once

#include "core/Core.h"
#include <functional>
#include <algorithm>

#define DECLARE_DELEGATE(_name, ...) \
    template class __declspec(dllexport) FGEngine::Delegate<__VA_ARGS__>; \
    class __declspec(dllexport) _name##Delegate : public FGEngine::Delegate<__VA_ARGS__> {};

#define AddFunction(_owner, _funcName)			__Add(std::bind_front(&_funcName, _owner));
#define RemoveFunction(_owner, _funcName)		__Remove(std::bind_front(&_funcName, _owner));
#define AddLambda(_func)						__Add(_func)

namespace FGEngine
{
template <typename... Params>
class ENGINE_API Delegate
{
public:
	void __Add(const std::function<void(Params...)>& func)
	{
		if (Contains(func)) return;
		funcVec.emplace_back(func);
	}

	void __Remove(const std::function<void(Params...)>& func)
	{
		funcVec.erase(std::remove_if(funcVec.begin(), funcVec.end(),
			[&func](const auto& delegate)
			{
				return delegate.target<void(Params...)>() == func.target<void(Params...)>();
			}),
			funcVec.end());
	}

	bool Contains(const std::function<void(Params...)>& func) const
	{
		return std::any_of(funcVec.begin(), funcVec.end(),
			[&func](const auto& delegate)
			{
				return delegate.target<void(Params...)>() == func.target<void(Params...)>();
			});
	}

	void Broadcast(Params... params)
	{
		for (auto& func : funcVec)
		{
			func(params...);
		}
	}

private:
	// TODO: should properly handle this warning someday...
#pragma warning (suppress : 4251)
	std::vector<std::function<void(Params...)>> funcVec;
};
}