// Copyright DCode. All Rights Reserved.

#include <SFML/Graphics/RectangleShape.hpp>

#include "Novel/SaveLoadWindow.h"
#include "Ui/Button.h"

CVrdxSaveLoadWindow::CVrdxSaveLoadWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxBoxWidget(ParentWidget, InShape)
{

}

void CVrdxSaveLoadWindow::OnPostCreate()
{
	// SetTexture
	TVrdxWeakPtr<CVrdxSaveLoadWindow> WeakThis = std::dynamic_pointer_cast<CVrdxSaveLoadWindow>(shared_from_this());

	constexpr float Margin = 30.f;
	constexpr float InnerMargin = 5.f;
	const sf::Vector2f Size = Shape.getSize();

	sf::RectangleShape TextShape = Shape;
	TextShape.setPosition({250, Margin});
	TextShape.setSize({500, 30});
	TextShape.setFillColor(sf::Color::White);
	TitleLabel = CVrdxWidgetBase::CreateWidget<CVrdxTextLabel>(WeakThis, TextShape);
	TitleLabel->SetFontSize(30);
	TitleLabel->SetText("VORADORIX Engine.");
	TitleLabel->SetFontColor(sf::Color::Red);
	TitleLabel->SetIgnoreEvent(true);

	const auto& TitleShape = TitleLabel->GetShape();

	sf::RectangleShape ButtonShape;
	ButtonShape.setPosition(sf::Vector2f(TitleShape.getPosition().x + TitleShape.getSize().x + InnerMargin, TitleShape.getPosition().y));
	ButtonShape.setSize(sf::Vector2f(30, 30));
	BackToMain = CVrdxWidgetBase::CreateWidget<CVrdxButton>(WeakThis, ButtonShape);
	BackToMain->SetText("Back");
	BackToMain->SetColors(sf::Color::Black, sf::Color::Green);
	BackToMain->SetLineColors(sf::Color::White, sf::Color::Red);
	BackToMain->SetFontColors(sf::Color::White, sf::Color::Black);
	BackToMain->GetOnClicked().Add([WeakThis]()
		{
			if (auto This = WeakThis.lock())
			{
				This->RequestBackToMain.Broadcast();
			}
		});

	constexpr int32_t NumSaveLoadSlots = 10;
	const float Left = Margin;
	const float Width = Size.x - Margin * 2;

	const float SlotTop = TitleShape.getPosition().y + TitleShape.getSize().y + InnerMargin;
	const float SlotBottom = Size.y - Margin;
	const float PivotSize = (SlotBottom - SlotTop) / NumSaveLoadSlots;
	const float SlotHeight = PivotSize - InnerMargin;

	for (int32_t Index = 0; Index < NumSaveLoadSlots; ++Index)
	{
		ButtonShape.setPosition({Left, SlotTop + (Index * PivotSize)});
		ButtonShape.setSize({Width, SlotHeight});

		TVrdxSharedPtr<CVrdxButton> Button = CVrdxWidgetBase::CreateWidget<CVrdxButton>(WeakThis, ButtonShape);
		Button->SetText("New game");
		Button->SetColors(sf::Color::Black, sf::Color::Green);
		Button->SetLineColors(sf::Color::White, sf::Color::Red);
		Button->SetFontColors(sf::Color::White, sf::Color::Black);
		Button->GetOnClicked().Add([WeakThis, Index]()
			{
				if (auto This = WeakThis.lock())
				{
					if (This->IsSaveMode())
					{
						This->GetRequestSaveGame().Broadcast(Index);
					}
					else
					{
						This->GetRequestLoadGame().Broadcast(Index);
					}
				}
			});

		ButtonSlots.Add(VrdxMove(Button));
	}
}

void CVrdxSaveLoadWindow::OnPreDestroy()
{
	ButtonSlots.Clear();
	TitleLabel = nullptr;
}

void CVrdxSaveLoadWindow::ShowSaveLoadWindow(bool bSave)
{
	bSaveMode = bSave;
	TitleLabel->SetText(bSaveMode ? "Save Game" : "Load Game");

	for (int32_t Index = 0; Index < ButtonSlots.Num(); ++Index)
	{
		auto& Button = ButtonSlots[Index];

		Button->SetText(
			(bSaveMode ? "Save Slot " : "Load Slot ") + std::to_string(Index + 1));
	}

	BringToFront();
}
