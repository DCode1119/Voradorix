// Copyright DCode. All Rights Reserved.

#include "Novel/ScriptEngine.h"

// C++ Standard Library
#include <fstream>
#include <memory>
#include <string>

// Project Headers
#include "Novel/NovelScene.h"

void CVrdxScriptEngine::SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene)
{
	WeakNovelScene = NovelScene;
}

bool CVrdxScriptEngine::LoadScript(const FVrdxString& InScriptPath)
{
	Reset();

	ScriptPath = InScriptPath;

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
	if (auto NovelScene = WeakNovelScene.lock())
	{
		NovelScene->Reset();
	}
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

void CVrdxScriptEngine::JumpToLine(const int32_t TargetLine)
{
	auto NovelScene = WeakNovelScene.lock();
	if (!(NovelScene && TargetLine < ScriptLines.Num()))
	{
		return;
	}

	CurrentScriptLine = TargetLine;

	if (const auto CurrentLine = ScriptLines[CurrentScriptLine++])
	{
		//Assume as loaded line indicates pending input.
		CurrentLine->Dispatch(NovelScene);
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
