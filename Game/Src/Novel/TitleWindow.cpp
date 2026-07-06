// Copyright DCode. All Rights Reserved.

#include "Novel/TitleWindow.h"
#include "Ui/Button.h"


CVrdxTitleWindow::CVrdxTitleWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape)
	: CVrdxBoxWidget(ParentWidget, InShape)
{

}

void CVrdxTitleWindow::OnPostCreate()
{
	// SetTexture
	sf::RectangleShape ButtonShape;
	TVrdxWeakPtr<CVrdxTitleWindow> WeakThis = std::dynamic_pointer_cast<CVrdxTitleWindow>(shared_from_this());

	sf::Vector2f Size = Shape.getSize();

	ButtonShape.setPosition(sf::Vector2f(100, Size.y / 2));
	ButtonShape.setSize(sf::Vector2f(Size.x - 200, 100));
	NewGameButton = CVrdxWidgetBase::CreateWidget<CVrdxButton>(WeakThis, ButtonShape);
	NewGameButton->SetText("New game");
	NewGameButton->SetColors(sf::Color::Black, sf::Color::Green);
	NewGameButton->SetLineColors(sf::Color::White, sf::Color::Red);
	NewGameButton->SetFontColors(sf::Color::White, sf::Color::Black);
	NewGameButton->GetOnClicked().Bind(WeakThis.lock(), &CVrdxTitleWindow::OnNewGame);

	ButtonShape.setPosition(sf::Vector2f(100, Size.y / 2 + 150));
	LoadGameButton = CVrdxWidgetBase::CreateWidget<CVrdxButton>(WeakThis, ButtonShape);
	LoadGameButton->SetText("Continue game");
	LoadGameButton->SetColors(sf::Color::Black, sf::Color::Green);
	LoadGameButton->SetLineColors(sf::Color::White, sf::Color::Red);
	LoadGameButton->SetFontColors(sf::Color::White, sf::Color::Black);
	LoadGameButton->GetOnClicked().Bind(WeakThis.lock(), &CVrdxTitleWindow::OnContinueGame);

	sf::RectangleShape TextShape = Shape;;
	TextShape.setPosition({250, 30});
	TextShape.setSize({50, 300});
	TitleLabel = CVrdxWidgetBase::CreateWidget<CVrdxTextLabel>(WeakThis, TextShape);
	TitleLabel->SetFontSize(50);
	TitleLabel->SetText("VORADORIX Engine.");
	TitleLabel->SetFontColor(sf::Color::Red);
	TitleLabel->SetIgnoreEvent(true);
}

void CVrdxTitleWindow::OnPreDestroy()
{
	NewGameButton = nullptr;
	LoadGameButton = nullptr;
}

void CVrdxTitleWindow::OnNewGame()
{
	RequestNewGame.Broadcast();
}

void CVrdxTitleWindow::OnContinueGame()
{
	RequestContinueGame.Broadcast()	;
}
