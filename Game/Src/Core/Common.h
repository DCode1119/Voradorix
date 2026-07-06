// Copyright DCode. All Rights Reserved.
#pragma once

// C++ Standard Library
#include <memory>
#include <type_traits>
#include <functional>
#include "Vector.h"

// ============================================================================
// Voradorix Naming Convention Types (Unreal Engine Style)
// ============================================================================
//
// C  — Regular class     (e.g. CVrdxScene, CVrdxSceneManager)
// T  — Template          (e.g. TVrdxUniquePtr<T>, TVrdxVector<T>)
// E  — Enum              (e.g. EVrdxSceneResult)
// I  — Interface         (e.g. IVrdxSerializable)
// F  — Struct            (e.g. FVrdxSaveSlot)
// b  — Boolean variable  (e.g. bIsRunning, bLoaded)
//
// Variables: PascalCase, NO m_ prefix (Unreal style)
// Methods:   PascalCase
// Files:     PascalCase with Vrdx prefix
// ============================================================================

// ----------------------------------------------------------------------------
// Template Aliases
// ----------------------------------------------------------------------------
template <typename ElementType>
using TVrdxUniquePtr = std::unique_ptr<ElementType>;

template <typename ElementType>
using TVrdxSharedPtr = std::shared_ptr<ElementType>;

template <typename ElementType>
using TVrdxWeakPtr = std::weak_ptr<ElementType>;

template <typename ElementType>
constexpr std::remove_reference_t<ElementType>&& VrdxMove(ElementType&& Value) noexcept
{
	return static_cast<std::remove_reference_t<ElementType>&&>(Value);
}

template <typename ElementType, typename... ArgTypes>
TVrdxUniquePtr<ElementType> MakeVrdxUnique(ArgTypes&&... Args)
{
	return std::make_unique<ElementType>(std::forward<ArgTypes>(Args)...);
}

template <typename ElementType, typename... ArgTypes>
TVrdxSharedPtr<ElementType> MakeVrdxShared(ArgTypes&&... Args)
{
	return std::make_shared<ElementType>(std::forward<ArgTypes>(Args)...);
}

template<typename... ParamTypes>
class TVrdxMulticastDelegate
{
public:
	using FDelegate = std::function<void(ParamTypes...)>;

	void Add(FDelegate&& Callback)
	{
		Delegates.Add(std::move(Callback));
	}

	template<typename TClass>
	void Bind(TVrdxSharedPtr<TClass> Object, void (TClass::* Method)(ParamTypes...))
	{
		TVrdxWeakPtr<TClass> WeakObject = Object;
		Add([WeakObject, Method](ParamTypes... Args)
			{
				if (auto Object = WeakObject.lock())
				{
					(Object.get()->*Method)(std::forward<ParamTypes>(Args)...);
				}
			});
	}

	void Broadcast(ParamTypes... Args)
	{
		for (auto& Delegate : Delegates)
		{
			if (Delegate)
			{
				Delegate(Args...);
			}
		}
	}

	void Clear() { Delegates.Clear(); }

private:
	TVrdxVector<FDelegate> Delegates;
};
