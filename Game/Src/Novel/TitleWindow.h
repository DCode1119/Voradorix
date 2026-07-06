// Copyright DCode. All Rights Reserved.
#pragma once

// Project Headers
#include "Core/Vector.h"
#include "Ui/BoxWidget.h"
#include "Ui/TextLabel.h"

class CVrdxButton;

class CVrdxTitleWindow: public CVrdxBoxWidget
{
public:
	CVrdxTitleWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	virtual void OnPostCreate() override;
	virtual void OnPreDestroy() override;

	void OnNewGame();
	void OnContinueGame();

	TVrdxMulticastDelegate<>& GetRequestNewGame() { return RequestNewGame; }
	TVrdxMulticastDelegate<>& GetRequestContinueGame() { return RequestContinueGame; }

private:
	TVrdxSharedPtr<CVrdxTextLabel> TitleLabel;
	TVrdxSharedPtr<CVrdxButton> NewGameButton;
	TVrdxSharedPtr<CVrdxButton> LoadGameButton;

	TVrdxMulticastDelegate<> RequestNewGame;
	TVrdxMulticastDelegate<> RequestContinueGame;
};
