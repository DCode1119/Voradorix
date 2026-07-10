// Copyright DCode. All Rights Reserved.

// Project Headers
#include "Novel/ScriptEngine.h"
#include "Novel/NovelScene.h"
#include "Ui/Application.h"
#include "Novel/TitleWindow.h"
#include "Novel/SaveLoadWindow.h"

int main()
{
    if (!FVrdxScriptLine::Test())
    {
        return -1;
    }

    CVrdxApplication::VRDX_Initializer Initializer = [](TVrdxWeakPtr<CVrdxWidgetBase>& RootWidget)
        {
			sf::RectangleShape Shape(sf::Vector2f(1280, 720));
			Shape.setFillColor(sf::Color::Transparent);
			Shape.setOutlineColor(sf::Color::Transparent);
			auto NovelWindow = CVrdxWidgetBase::CreateWidget<CVrdxNovelScene>(RootWidget, Shape);

            Shape.setFillColor(sf::Color::Black);
            auto TitleWindow = CVrdxWidgetBase::CreateWidget<CVrdxTitleWindow>(RootWidget, Shape);
			auto SaveLoadWindow = CVrdxWidgetBase::CreateWidget<CVrdxSaveLoadWindow>(RootWidget, Shape);

            auto ToggleShowWindow = [NovelWindow, TitleWindow, SaveLoadWindow](TVrdxSharedPtr<CVrdxWidgetBase> Target)
                {
                    TVrdxSharedPtr<CVrdxWidgetBase> Widgets[] = {
                        NovelWindow, TitleWindow, SaveLoadWindow,
                    };

                    for (auto& Widget : Widgets)
                    {
                        Widget->SetVisibility(Widget == Target);
                    }
                };

            TitleWindow->GetRequestNewGame().Add([NovelWindow, ToggleShowWindow]() {
                    NovelWindow->ResetScriptEngine();
                    NovelWindow->BringToFront();
                    ToggleShowWindow(NovelWindow);
                });

			TitleWindow->GetRequestContinueGame().Add([SaveLoadWindow, ToggleShowWindow]()
                {
                    SaveLoadWindow->ShowSaveLoadWindow(false);
                    ToggleShowWindow(SaveLoadWindow);
                });

            SaveLoadWindow->GetRequestBackToMain().Add([TitleWindow, ToggleShowWindow]()
                {
                    TitleWindow->BringToFront();
                    ToggleShowWindow(TitleWindow);
                });
            SaveLoadWindow->GetRequestLoadGame().Add([NovelWindow, ToggleShowWindow](int32_t SlotIndex)
                {
                    const std::string Filename = "Save" + std::to_string(SlotIndex) + ".dat";

                    if (std::filesystem::exists("Saves/" + Filename))
                    {
                        NovelWindow->Load(Filename);
                        NovelWindow->BringToFront();
                        ToggleShowWindow(NovelWindow);
                    }
                    else
                    {
                        // @todo: Display error
                    }
                });

            TitleWindow->BringToFront();
            ToggleShowWindow(TitleWindow);

        };

    // Generate and initialize
    TVrdxSharedPtr<CVrdxApplication> Application = MakeVrdxShared<CVrdxApplication>(sf::Vector2f(1280, 720));
    Application->Initialize(VrdxMove(Initializer));

    // Main loop
	Application->Run();

    // Destroy and exit
	CVrdxWidgetBase::DestroyWidget(Application);
    
    return 0;
}
