#pragma once

#include <memory>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "Core/Common.h"
#include "Core/Vector.h"

class CVrdxWidgetBase : public std::enable_shared_from_this<CVrdxWidgetBase>
{
public:
	template<typename TVrdxWidget> requires std::derived_from<TVrdxWidget, CVrdxWidgetBase>
	static TVrdxSharedPtr<TVrdxWidget>
	CreateWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	static void DestroyWidget(const TVrdxWeakPtr<CVrdxWidgetBase> TargetWidget);

	CVrdxWidgetBase(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	virtual ~CVrdxWidgetBase() VRDX_DEFAULT;

	virtual bool HandleEvent(const sf::Event& Event);
	virtual void Update(const float DeltaTick);
	virtual void Draw(sf::RenderWindow& Window) const;

	// Overridable hooks
	virtual void OnPostCreate() {}
	virtual void OnPreDestroy() {}

	virtual void OnResized() {}

	// Will be called HandleEvent
	virtual void OnMouseMove(const sf::Vector2f& LocalPosition) {}
	virtual void OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) {}
	virtual void OnMouseLeftButtonReleased(const sf::Vector2f& LocalPosition) {}
	virtual void OnMouseRightButtonPressed(const sf::Vector2f& LocalPosition) {}
	virtual void OnMouseRightButtonReleased(const sf::Vector2f& LocalPosition) {}
	virtual void OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode) {}
	virtual void OnKeyboardReleased(const sf::Keyboard::Scancode ScanCode) {}

	sf::Vector2f MapToGlobal(const sf::Vector2f& Local) const;
	sf::FloatRect MapToGlobal(const sf::FloatRect& Local) const;
	sf::Vector2f MapToLocal(const sf::Vector2f& Global) const;
	sf::FloatRect MapToLocal(const sf::FloatRect& Global) const;

	void SetVisibility(const bool bVisible) { bIsVisible = bVisible; }
	bool GetVisibility() const { return bIsVisible; }

	bool ContainsInLocal(const sf::Vector2f& Local) const;
	bool Hides(const CVrdxWidgetBase* Target) const;
	const TVrdxVector<TVrdxSharedPtr<CVrdxWidgetBase>>& GetChildren() const { return Children; }

	bool IsDrawable() const;
	sf::RectangleShape& GetShape() { return Shape; }
	void SetIgnoreEvent(const bool bIgnore) { bIgnoreEvent = bIgnore; }

protected:
	void SetCapture(const bool bCapture) { bIsCapturing = bCapture; }

	TVrdxWeakPtr<CVrdxWidgetBase> Parent;
	TVrdxVector<TVrdxSharedPtr<CVrdxWidgetBase>> Children;

	// note: the shape's position is relative to parent.
	sf::RectangleShape Shape;

private:
	void RegisterChildWidget(TVrdxSharedPtr<CVrdxWidgetBase> ChildWidget);
	void UnregisterChildWidget(TVrdxSharedPtr<CVrdxWidgetBase> ChildWidget);
	bool Equals(const sf::FloatRect& A, const sf::FloatRect& B) const;
	
	bool bIsVisible = true;
	bool bCanBeDrawn = true;
	bool bIsCapturing = false;
	bool bIgnoreEvent = false;

	sf::FloatRect CachedGeometry;
};

template<typename TVrdxWidget> requires std::derived_from<TVrdxWidget, CVrdxWidgetBase>
static TVrdxSharedPtr<TVrdxWidget>
CVrdxWidgetBase::CreateWidget(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
{
	if (TVrdxSharedPtr<TVrdxWidget> ChildWidget = MakeVrdxShared<TVrdxWidget>(ParentWidget, InShape))
	{
		if (auto Parent = ParentWidget.lock())
		{
			Parent->RegisterChildWidget(ChildWidget);
		}

		ChildWidget->OnPostCreate();

		return ChildWidget;
	}

	// Critical error.

	return nullptr;
}
