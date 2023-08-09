#pragma once

#include "core/Core.h"
#include <unordered_map>
#include <functional>
#include <memory>

#define DECLARE_DELEGATE_ZERO(_name) \
	template class __declspec(dllexport) FGEngine::Delegate<>; \
	class _declspec(dllexport) _name##Delegate : public FGEngine::Delegate<>{};

#define DECLARE_DELEGATE_ONE(_name, _param1) \
	template class __declspec(dllexport) FGEngine::Delegate<_param1>; \
	class _declspec(dllexport) _name##Delegate : public FGEngine::Delegate<_param1>{};

#define DECLARE_DELEGATE_TWO(_name, _param1, _param2) \
	template class __declspec(dllexport) FGEngine::Delegate<_param1, _param2>; \
	class _declspec(dllexport) _name##Delegate : public FGEngine::Delegate<_param1, _param2>{};

#define DECLARE_DELEGATE_THREE(_name, _param1, _param2, _param3) \
	template class __declspec(dllexport) FGEngine::Delegate<_param1, _param2, _param3>; \
	class _declspec(dllexport) _name##Delegate : public FGEngine::Delegate<_param1, _param2, _param3>{};

#define DECLARE_DELEGATE_FOUR(_name, _param1, _param2, _param3, param4_) \
	template class __declspec(dllexport) FGEngine::Delegate<_param1>; \
	class _declspec(dllexport) _name##Delegate : public FGEngine::Delegate<_param1, _param2, _param3, _param4>{};


#define DELEGATE_PTR(type_, _name) std::shared_ptr<type_> _name = std::make_shared<type_>();

#define Add0(_owner, _funcName) __Add(std::bind(&_funcName, _owner));
#define Add1(_owner, _funcName) __Add(std::bind(&_funcName, _owner, std::placeholders::_1));
#define Add2(_owner, _funcName) __Add(std::bind(&_funcName, _owner, std::placeholders::_1, std::placeholders::_2));
#define Add3(_owner, _funcName) __Add(std::bind(&_funcName, _owner, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
#define Add4(_owner, _funcName) __Add(std::bind(&_funcName, _owner, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

#define AddLambda(_func) __Add(_func)

namespace FGEngine
{
	struct DelegateHandle;

	class ENGINE_API DelegateBase : public std::enable_shared_from_this<DelegateBase>
#pragma warning (suppress : 4251)
	{
	public:
		std::weak_ptr<DelegateBase> Get()
		{
			return weak_from_this();
		}

		virtual void Remove(const DelegateHandle& handle) = 0;
	};

	struct ENGINE_API DelegateHandle
	{
	public:
		DelegateHandle() = default;
		DelegateHandle(int key, std::weak_ptr<DelegateBase> delegate) :
			key(key),
			delegate(delegate)
		{
		}

		~DelegateHandle()
		{
			if (IsValid())
			{
				delegate.lock()->Remove(*this);
			}
		}

		DelegateHandle(DelegateHandle&& other) noexcept :
			key(std::move(other.key)),
			delegate(std::move(other.delegate))
		{
		}

		DelegateHandle& operator=(DelegateHandle&& other) noexcept
		{
			key = std::move(other.key);
			delegate = std::move(other.delegate);
			return *this;
		}

		inline unsigned int GetKey() const
		{
			return key;
		}

		inline std::weak_ptr<DelegateBase> GetWeakDelegate() const
		{
			return delegate;
		}

		inline bool IsValid() const
		{
			return !delegate.expired();
		}

	private:
		unsigned int key = 0;
#pragma warning (suppress : 4251)
		std::weak_ptr<DelegateBase> delegate;
	};

	template <typename... Params>
	class ENGINE_API Delegate : public DelegateBase
	{
	public:
		DelegateHandle __Add(std::function<void(Params...)> func)
		{
			if (funcMap.size() >= UINT_MAX)
			{
				LogError("Delegate space is full!");
				return DelegateHandle();
			}

			// loop until an available key is found
			while (funcMap.find(nextKeyId) != funcMap.end())
			{
				// no need to worry about overflow, since it will return to zero.
				nextKeyId++;
			}

			funcMap.emplace(nextKeyId, func);

			return DelegateHandle(nextKeyId, this->Get());
		}

		virtual void Remove(const DelegateHandle& handle) override
		{
			if (!handle.IsValid()) return;
			if (handle.GetWeakDelegate().lock().get() != this) return;

			auto it = funcMap.find(handle.GetKey());
			if (it == funcMap.end()) return;
			funcMap.erase(it);
		}

		void Broadcast(Params... params)
		{
			for (auto func : funcMap)
			{
				func.second(params...);
			}
		}

	private:
#pragma warning (suppress : 4251)
		std::unordered_map<unsigned int, std::function<void(Params...)>> funcMap;
		unsigned int nextKeyId = 0;
	};
}