#include "Novel/ScriptLine.h"

#include <regex>
#include <string>
#include <unordered_map>

#include "Novel/DialogueLine.h"
#include "Scene/NovelScene.h"

TVrdxSharedPtr<FVrdxScriptLine> FVrdxScriptLine::ParseScriptLine(const std::string& Line)
{
	if (Line.length() == 0)
	{
		return nullptr;
	}

	static const std::regex CmdRegex(R"(^\s*@([A-Za-z_][A-Za-z0-9_]*)\s*)");

	std::smatch Match;
	if (!std::regex_search(Line, Match, CmdRegex))
	{
		return nullptr;
	}

	using Factory = TVrdxSharedPtr<FVrdxScriptLine>(*)();
	static const std::unordered_map<std::string, Factory> Table =
	{
		{ "bg",       []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxBackgroundScriptLine>();    } },
		{ "show",     []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxShowCharacterScriptLine>(); } },
		{ "hide",     []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxHideCharacterScriptLine>(); } },
		{ "pose",     []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxCharacterPoseScriptLine>(); } },
		{ "wait",     []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxWaitScriptLine>();          } },
		{ "label",    []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxLabelScriptLine>();         } },
		{ "jump",     []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxJumpScriptLine>();          } },
		{ "dialogue", []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxDialogueScriptLine>();      } },
		{ "choice",   []() -> TVrdxSharedPtr<FVrdxScriptLine> { return MakeVrdxShared<FVrdxChoiceScriptLine>();        } },
	};

	auto It = Table.find(Match[1].str());
	if (It == Table.end())
	{
		return nullptr;
	}

	TVrdxSharedPtr<FVrdxScriptLine> OutScriptLine = It->second();
	OutScriptLine->RawText = Line;
	OutScriptLine->Arguments.Add(Match[1].str());

	static const std::regex ArgRegex("\"([^\"]*)\"");
	auto Begin = std::sregex_iterator(Line.begin(), Line.end(), ArgRegex);
	auto End = std::sregex_iterator();

	for (; Begin != End; ++Begin)
	{
		OutScriptLine->Arguments.Add((*Begin)[1].str());
	}

	return OutScriptLine->Construct() ? OutScriptLine : nullptr;
}

bool FVrdxScriptLine::Test()
{
	std::vector<std::string> lines =
	{
		R"(@bg "WhiteRoom")",
		R"(@show "Laura")",
		R"(@dialogue "Laura" "Hello World")",
	};

	TVrdxVector<TVrdxSharedPtr<FVrdxScriptLine>> ScriptLines;
	for (int32_t Index = 0; Index < lines.size(); ++Index)
	{
		if (TVrdxSharedPtr<FVrdxScriptLine> Line = FVrdxScriptLine::ParseScriptLine(lines[Index]))
		{
			ScriptLines.Add(Line);
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool FVrdxBackgroundScriptLine::Construct()
{
	if (Arguments.Num() < 2)
	{
		return false;
	}

	MapName = Arguments[1];
	return true;
}


bool FVrdxBackgroundScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->SetBackground(MapName);
	return true;
}

bool FVrdxShowCharacterScriptLine::Construct()
{
	if (Arguments.Num() < 2)
	{
		return false;
	}

	static const std::unordered_map<FVrdxString, EVrdxCharacterPosition> Table =
	{
		{ "Center", EVrdxCharacterPosition::Center },
		{ "Left", EVrdxCharacterPosition::Left },
		{ "Right", EVrdxCharacterPosition::Right },
	};

	CharacterName = Arguments[1];

	if (Arguments.Num() > 2)
	{
		auto FindResult = Table.find(Arguments[2]);
		if (FindResult == Table.end())
		{
			return false;
		}

		Position = FindResult->second;
	}

	if (Arguments.Num() > 3)
	{
		Pose = Arguments[3];
	}

	return true;
}

bool FVrdxShowCharacterScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->ShowCharacter(CharacterName, Position);
	if (Arguments.Num() == 4)
	{
		Scene->SetCharacterPose(CharacterName, Pose);
	}

	return true;
}

bool FVrdxHideCharacterScriptLine::Construct()
{
	if (Arguments.Num() < 2)
	{
		return false;
	}

	CharacterName = Arguments[1];

	return true;
}


bool FVrdxHideCharacterScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->HideCharacter(CharacterName);

	return true;
}

bool FVrdxCharacterPoseScriptLine::Construct()
{
	if (Arguments.Num() < 3)
	{
		return false;
	}

	CharacterName = Arguments[1];
	Pose = Arguments[2];

	return true;
}

bool FVrdxCharacterPoseScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->SetCharacterPose(CharacterName, Pose);

	return true;
}

bool FVrdxWaitScriptLine::Construct()
{
	if (Arguments.Num() < 2)
	{
		return false;
	}

	Seconds = std::stof(Arguments[1].ToUtf8());

	return true;
}

bool FVrdxWaitScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->WaitForSeconds(Seconds);
	return false;
}

bool FVrdxLabelScriptLine::Construct()
{
	if (Arguments.Num() < 2)
	{
		return false;
	}

	LabelName = Arguments[1];

	return true;
}

bool FVrdxLabelScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	//nothing to be implemented.
	return true;
}

bool FVrdxJumpScriptLine::Construct()
{
	if (Arguments.Num() < 2)
	{
		return false;
	}

	TargetLabelName = Arguments[1];

	return true;
}

bool FVrdxJumpScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->JumpToLabel(TargetLabelName);

	return true;
}

bool FVrdxDialogueScriptLine::Construct()
{
	if (Arguments.Num() < 3)
	{
		return false;
	}

	Speaker = Arguments[1];
	Dialogue = Arguments[2];

	return true;
}

bool FVrdxDialogueScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->SetDialogue({Speaker, Dialogue});
	return false;
}

bool FVrdxChoiceScriptLine::Construct()
{
	if ( Arguments.Num() < 3 || 0 == (Arguments.Num() % 2) )
	{
		return false;
	}

	for (int32_t Index = 1; Index < Arguments.Num(); Index += 2)
	{
		ChoiceOptions.Add({Arguments[Index + 0], Arguments[Index + 1]});
	}

	return true;
}

bool FVrdxChoiceScriptLine::Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene)
{
	Scene->SetChoices(ChoiceOptions);
	return false;
}
