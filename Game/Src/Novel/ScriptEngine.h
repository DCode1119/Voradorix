#pragma once

#include "ScriptLine.h"

class CVrdxNovelScene;

class CVrdxScriptEngine
{
public:
	void SetNovelScene(TVrdxSharedPtr<CVrdxNovelScene> NovelScene);
	bool LoadScript(const FVrdxString& ScriptPath);
	void Reset();

	void Update(float DeltaTick);

	bool CanAdvance() const;
	bool IsFinished() const;
	void JumpToLabel(const FVrdxString& TargetLabelName);

	FVrdxString GetScriptName() const { return ScriptPath; }
	int32_t GetCurrentScriptLine() const { return CurrentScriptLine; }
	void JumpToLine(const int32_t TargetLine);

private:
	bool ParseLine();

	FVrdxString ScriptPath;

	int32_t CurrentScriptLine;
	TVrdxVector<TVrdxSharedPtr<FVrdxScriptLine>> ScriptLines;
	TVrdxWeakPtr<CVrdxNovelScene> WeakNovelScene;
	std::unordered_map<FVrdxString, int32_t> Labels;
};
