// Copyright DCode. All Rights Reserved.
#pragma once

// Project Headers
#include "Core/Vector.h"
#include "Ui/BoxWidget.h"
#include "Ui/TextLabel.h"

class CVrdxButton;

class CVrdxSaveLoadWindow: public CVrdxBoxWidget
{
public:
	CVrdxSaveLoadWindow(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);

	virtual void OnPostCreate() override;
	virtual void OnPreDestroy() override;

	void ShowSaveLoadWindow(const bool bSaveMode);
	TVrdxMulticastDelegate<int32_t>& GetRequestSaveGame() { return RequestSaveGame; }
	TVrdxMulticastDelegate<int32_t>& GetRequestLoadGame() { return RequestLoadGame; }
	TVrdxMulticastDelegate<>& GetRequestBackToMain() { return RequestBackToMain; }

	bool IsSaveMode() const { return bSaveMode; }
private:
	TVrdxSharedPtr<CVrdxTextLabel> TitleLabel;
	TVrdxSharedPtr<CVrdxButton> BackToMain;
	TVrdxVector<TVrdxSharedPtr<CVrdxButton>> ButtonSlots;

	TVrdxMulticastDelegate<> RequestBackToMain;
	TVrdxMulticastDelegate<int32_t> RequestSaveGame;
	TVrdxMulticastDelegate<int32_t> RequestLoadGame;

	bool bSaveMode = false;
};
