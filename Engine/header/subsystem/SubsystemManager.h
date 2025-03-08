#pragma once

#include "EngineSubsystem.h"

#include <unordered_map>
#include <typeindex>
#include <memory>

namespace FGEngine
{
class SubsystemManager
{
public:
	static SubsystemManager& Get()
	{
		static SubsystemManager instance;
		return instance;
	}

private:
	SubsystemManager() = default;

public:
	~SubsystemManager()
	{
		engineSubsystems.clear();
	}

	template<typename TSubSystem, typename... Args>
	TSubSystem* RegisterSubsystem(Args&&... args)
	{
		static_assert(std::is_base_of<EngineSubsystem, TSubSystem>::value, "TSubSystem must derive from EngineSubsystem");

		std::type_index idx = typeid(TSubSystem);
		if (!engineSubsystems.contains(idx))
		{
			engineSubsystems[idx] = std::make_unique<TSubSystem>(std::forward<Args>(args)...);
		}
		return static_cast<TSubSystem*>(engineSubsystems[idx].get());
	}

	template<typename TSubSystem>
	void UnregisterSubsystem()
	{
		static_assert(std::is_base_of<EngineSubsystem, TSubSystem>::value, "TSubSystem must derive from EngineSubsystem");

		std::type_index idx = typeid(TSubSystem);
		std::erase_if(engineSubsystems,
			[&idx](const auto& p)
			{
				return p.first == idx;
			});
	}

	template<typename TSubSystem>
	TSubSystem* GetSubsystem()
	{
		static_assert(std::is_base_of<EngineSubsystem, TSubSystem>::value, "TSubSystem must derive from EngineSubsystem");

		std::type_index idx = typeid(TSubSystem);
		if (engineSubsystems.contains(idx))
		{
			return static_cast<TSubSystem*>(engineSubsystems[idx].get());
		}

		return nullptr;
	}
private:
	std::unordered_map<std::type_index, std::unique_ptr<EngineSubsystem>> engineSubsystems;
};
}