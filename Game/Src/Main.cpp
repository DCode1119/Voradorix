#include "Core/Application.h"
#include "Novel/ScriptEngine.h"

int main()
{
    if (!FVrdxScriptLine::Test())
    {
        return -1;
    }

    sf::RectangleShape Panel;
    Panel.setSize({1280, 720});
    Panel.setPosition({0, 0});
    Panel.setFillColor(sf::Color::Transparent);

    if (TVrdxSharedPtr<CVrdxApplication> Application = CVrdxWidgetBase::CreateWidget<CVrdxApplication>(TVrdxWeakPtr<CVrdxWidgetBase>{}, Panel))
    {
		Application->Run();
		CVrdxWidgetBase::DestroyWidget(Application);
    }
    else
    {
        //Critical error
    }
    
    return 0;
}
