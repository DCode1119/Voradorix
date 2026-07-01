#include "Core/Application.h"
#include "Novel/ScriptEngine.h"

int main()
{
    if (!FVrdxScriptLine::Test())
    {
        return -1;
    }

    CVrdxApplication Application;
    Application.Run();

    return 0;
}
