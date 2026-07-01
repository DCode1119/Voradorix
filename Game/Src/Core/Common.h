#pragma once

#include <memory>
#include <type_traits>

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

// ----------------------------------------------------------------------------
// C++ Syntax Macros
// ----------------------------------------------------------------------------
#define VRDX_DEFAULT        = default
#define VRDX_PURE_VIRTUAL   = 0
#define VRDX_NO_COPY        = delete
#define VRDX_NO_DISCARD     [[nodiscard]]
#define VRDX_OVERRIDE       override
#define VRDX_FINAL          final
