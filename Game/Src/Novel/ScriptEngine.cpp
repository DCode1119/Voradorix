#include "Novel/ScriptEngine.h"

#include <fstream>
#include <string>
#include <memory>

#include "Scene/NovelScene.h"

void CVrdxScriptEngine::SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene)
{
	WeakNovelScene = NovelScene;
}

bool CVrdxScriptEngine::LoadScript(const FVrdxString& ScriptPath)
{
	Reset();

	std::ifstream FileStream(ScriptPath.ToUtf8());
	if (!FileStream)
	{
		return false;
	}

	int32_t LineNumber = 0;
	std::string Line;
	while (std::getline(FileStream, Line))
	{
		if (TVrdxSharedPtr<FVrdxScriptLine> ParsedLine = FVrdxScriptLine::ParseScriptLine(Line))
		{
			ScriptLines.Add(ParsedLine);
			if (ParsedLine->Arguments[0] == "label")
			{
				Labels.emplace(ParsedLine->Arguments[1], ScriptLines.Num()-1);
			}
		}
	}

	return true;
}

void CVrdxScriptEngine::Reset()
{
	CurrentScriptLine = 0;
	ScriptLines.Clear();
	Labels.clear();
}

void CVrdxScriptEngine::Update(float DeltaTick)
{
	while (!IsFinished())
	{
		if (!CanAdvance() || !ParseLine())
		{
			break;
		}
	}
}

bool CVrdxScriptEngine::CanAdvance() const
{
	auto NovelScene = WeakNovelScene.lock();

	return NovelScene && NovelScene->CanAdvance();
}

bool CVrdxScriptEngine::IsFinished() const
{
	return !(CurrentScriptLine < ScriptLines.Num());
}

void CVrdxScriptEngine::JumpToLabel(const FVrdxString& TargetLabelName)
{
	if (Labels.find(TargetLabelName) != Labels.end())
	{
		CurrentScriptLine = Labels[TargetLabelName];
	}
}

bool CVrdxScriptEngine::ParseLine()
{
	auto NovelScene = WeakNovelScene.lock();
	if (!(NovelScene && CurrentScriptLine < ScriptLines.Num()))
	{
		return false;
	}

	const auto CurrentLine = ScriptLines[CurrentScriptLine++];

	return CurrentLine ? CurrentLine->Dispatch(NovelScene) : false;
}
