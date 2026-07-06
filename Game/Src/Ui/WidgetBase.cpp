// Copyright DCode. All Rights Reserved.

#include "Ui/WidgetBase.h"

// C++ Standard Library
#include <algorithm>
#include <ranges>

void CVrdxWidgetBase::DestroyWidget(const TVrdxWeakPtr<CVrdxWidgetBase> TargetWidget)
{
	if (auto Target = TargetWidget.lock())
	{
		auto PreservedChildren = Target->Children;
		for (auto Child : PreservedChildren)
		{
			CVrdxWidgetBase::DestroyWidget(Child);
		}

		Target->OnPreDestroy();

		if (auto Parent = Target->Parent.lock())
		{
			Parent->UnregisterChildWidget(Target);
			return;
		}

		return;
	}

	// critical error
}

CVrdxWidgetBase::CVrdxWidgetBase(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: Shape(InShape)
	, Parent(ParentWidget)
{
	CachedGeometry = MapToGlobal(sf::FloatRect({ 0,0 }, Shape.getSize()));
}

bool CVrdxWidgetBase::HandleEvent(const sf::Event& InEvent)
{
	if (!bIsVisible || bIgnoreEvent)
	{
		return false;
	}

	for (auto& Child : std::ranges::reverse_view(Children))
	{
		if (Child->HandleEvent(InEvent))
		{
			return true;
		}
	}

	if (const auto* Event = InEvent.getIf<sf::Event::KeyPressed>())
	{
		return OnKeyboardPressed(Event->scancode);
	}

	if (const auto* Event = InEvent.getIf<sf::Event::KeyReleased>())
	{
		return OnKeyboardReleased(Event->scancode);
	}
	
	if (const auto* Event = InEvent.getIf<sf::Event::MouseButtonPressed>())
	{
		// Mouse events should be contained in shape of this widget.
		sf::Vector2f LocalPosition = MapToLocal(sf::Vector2f(Event->position));
		if (!bIsCapturing && !ContainsInLocal(LocalPosition))
		{
			return false;
		}

		if (Event->button == sf::Mouse::Button::Left)
		{
			return OnMouseLeftButtonPressed(LocalPosition);
		}
		else if (Event->button == sf::Mouse::Button::Right)
		{
			return OnMouseRightButtonPressed(LocalPosition);
		}

		return false;
	}

	if (const auto* Event = InEvent.getIf<sf::Event::MouseButtonReleased>())
	{
		// Mouse events should be contained in shape of this widget.
		sf::Vector2f LocalPosition = MapToLocal(sf::Vector2f(Event->position));
		if (!bIsCapturing && !ContainsInLocal(LocalPosition))
		{
			return false;
		}

		if (Event->button == sf::Mouse::Button::Left)
		{
			return OnMouseLeftButtonReleased(LocalPosition);
		}
		else if (Event->button == sf::Mouse::Button::Right)
		{
			return OnMouseRightButtonReleased(LocalPosition);
		}
		
		return false;
	}

	if (const auto Event = InEvent.getIf<sf::Event::MouseMoved>())
	{
		// Mouse events should be contained in shape of this widget.
		sf::Vector2f LocalPosition = MapToLocal(sf::Vector2f(Event->position));
		if (!bIsCapturing && !ContainsInLocal(LocalPosition))
		{
			return false;
		}

		return OnMouseMove(LocalPosition);
	}

	return false;
}

void CVrdxWidgetBase::Update(const float DeltaTick)
{
	const auto Geometry = MapToGlobal(sf::FloatRect({ 0,0 }, Shape.getSize()));
	if (!Equals(CachedGeometry, Geometry))
	{
		OnResized();
		CachedGeometry = Geometry;
	}

	for (auto& Child : Children)
	{
		Child->Update(DeltaTick);
	}

	if (auto ParentWidget = Parent.lock())
	{
		for (const auto Sibling : ParentWidget->Children)
		{
			if (Sibling->GetVisibility() && Sibling->Hides(this))
			{
				bCanBeDrawn = false;
				return;
			}
		}
	}

	bool bDrawThis = true;
	for (const auto Child : Children)
	{
		if (Child->GetVisibility() && Child->Hides(this))
		{
			bCanBeDrawn = false;
			return;
		}
	}

	bCanBeDrawn = true;
}

void CVrdxWidgetBase::Draw(sf::RenderWindow& Window) const
{
	if (!IsDrawable())
	{
		return;
	}

	sf::RectangleShape DrawShape = Shape;
	DrawShape.setPosition(MapToGlobal({0,0}));
	Window.draw(DrawShape);

	for (auto& Child : Children)
	{
		Child->Draw(Window);
	}
}

sf::Vector2f CVrdxWidgetBase::MapToGlobal(const sf::Vector2f& Local) const
{
	sf::Vector2f Result = Local + Shape.getPosition();
	for (auto ParentWidget = Parent.lock(); ParentWidget != nullptr; ParentWidget = ParentWidget->Parent.lock())
	{
		Result += ParentWidget->Shape.getPosition();
	}
	return Result;
}

sf::FloatRect CVrdxWidgetBase::MapToGlobal(const sf::FloatRect& Local) const
{
	return sf::FloatRect(MapToGlobal(Local.position), Local.size);
}

sf::Vector2f CVrdxWidgetBase::MapToLocal(const sf::Vector2f& Global)  const
{
	sf::Vector2f Result = Global - Shape.getPosition();
	for (auto ParentWidget = Parent.lock(); ParentWidget != nullptr; ParentWidget = ParentWidget->Parent.lock())
	{
		Result -= ParentWidget->Shape.getPosition();
	}
	return Result;
}

sf::FloatRect CVrdxWidgetBase::MapToLocal(const sf::FloatRect& Global)  const
{
	return sf::FloatRect(MapToLocal(Global.position), Global.size);
}

bool CVrdxWidgetBase::ContainsInLocal(const sf::Vector2f& Local) const
{
	return sf::FloatRect({ 0,0 }, Shape.getSize()).contains(Local);
}

bool CVrdxWidgetBase::Hides(const CVrdxWidgetBase* Target) const
{
	if (Target == nullptr || Target == this)
	{
		return false;
	}

	if (Shape.getFillColor().a != 255)
	{
		return false;
	}

	const sf::FloatRect MyRect = MapToGlobal({ {0.f, 0.f}, Shape.getSize() });
	const sf::FloatRect TargetRect = Target->MapToGlobal({ {0.f, 0.f}, Target->Shape.getSize() });

	return MyRect.contains(TargetRect.position)
		&& MyRect.contains(TargetRect.position + TargetRect.size);
}

bool CVrdxWidgetBase::IsDrawable() const
{
	return bCanBeDrawn && bIsVisible;
}

void CVrdxWidgetBase::BringToFront()
{
	if (auto ParentWidget = Parent.lock())
	{
		auto& Siblings = ParentWidget->Children;
		auto ThisPointer = shared_from_this();

		if (!Siblings.Contains(ThisPointer))
		{
			// Critical error — not a child of its own parent
			return;
		}

		Siblings.Remove(ThisPointer);
		Siblings.Add(ThisPointer);
	}
}

void CVrdxWidgetBase::RegisterChildWidget(TVrdxSharedPtr<CVrdxWidgetBase> ChildWidget)
{
	if (!Children.Contains(ChildWidget))
	{
		Children.Add(ChildWidget);
	}
}

void CVrdxWidgetBase::UnregisterChildWidget(TVrdxSharedPtr<CVrdxWidgetBase> ChildWidget)
{
	if (Children.Contains(ChildWidget))
	{
		Children.Remove(ChildWidget);
		ChildWidget->Parent.reset();
	}
}

bool CVrdxWidgetBase::Equals(const sf::FloatRect& A, const sf::FloatRect& B) const
{
	constexpr float epsilon = 0.001f;
	return (std::abs(A.position.x - B.position.x) < epsilon)
		&& (std::abs(A.position.y - B.position.y) < epsilon)
		&& (std::abs(A.size.x - B.size.x) < epsilon)
		&& (std::abs(A.size.y - B.size.y) < epsilon);
}
