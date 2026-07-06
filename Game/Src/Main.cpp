// Copyright DCode. All Rights Reserved.

// Project Headers
#include "Novel/ScriptEngine.h"
#include "Novel/NovelScene.h"
#include "Ui/Application.h"

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
			CVrdxWidgetBase::CreateWidget<CVrdxNovelScene>(RootWidget, Shape);
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
