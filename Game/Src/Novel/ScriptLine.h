// Copyright DCode. All Rights Reserved.
#pragma once

// Project Headers
#include "Core/Common.h"
#include "Core/String.h"
#include "Core/Vector.h"
#include "Novel/CharacterManager.h"
#include "Novel/ChoiceWidget.h"

class CVrdxNovelScene;


/*
| ------------ | ------------------------------------------------ | ------------------------------------------------------- |
| `@bg`        | `CVrdxBackground::SetBackground()` 호출          | @bg "WhiteRoom"                                         |
| `@show`      | `CVrdxCharacterManager::ShowCharacter()` 호출    | @show "Laura" "Center" "Normal"                         |
| `@hide`      | `CVrdxCharacterManager::HideCharacter()` 호출    | @hide "Laura"                                           |
| `@pose`      | `CVrdxCharacterManager::SetCharacterPose()` 호출 | @pose "Laura" "Normal"                                  |
| `@wait`      | 지정 시간 동안 진행 정지                         | @wait "0.3"                                             |
| `@label`     | 현재 위치를 레이블로 저장                        | @label "Label"                                          |
| `@jump`      | 레이블 위치로 인덱스 이동                        | @jump "Label"                                           |
| `@dialogue`  | `CDialogueBox`에 스피커/텍스트 전달 후 입력 대기 | @dialogue "Laura" "Welcome to the Chamber of Creation." |
| `@choice`    | ChoiceOption들을 전달 후 입력 대기               | @choice "Repeat" "Main" "Exit" "Exit"                   |
| ------------ | ------------------------------------------------ | ------------------------------------------------------- |
*/

struct FVrdxScriptLine
{
	virtual ~FVrdxScriptLine() VRDX_DEFAULT;
	static TVrdxSharedPtr<FVrdxScriptLine> ParseScriptLine(const std::string& Line);
	static bool Test();
	virtual bool Construct() { return false; }

	//true to continue parse
	//false to wait user input
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) { return false; }

	FVrdxString RawText;
	TVrdxVector<FVrdxString> Arguments;
};

struct FVrdxBackgroundScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString MapName;
};

struct FVrdxShowCharacterScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString CharacterName;
	FVrdxString Pose = "Normal";
	EVrdxCharacterPosition Position = EVrdxCharacterPosition::Center;

};

struct FVrdxHideCharacterScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString CharacterName;
};

struct FVrdxCharacterPoseScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString CharacterName;
	FVrdxString Pose;
};

struct FVrdxWaitScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	float Seconds = 0.f;
};

struct FVrdxLabelScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString LabelName;
};

struct FVrdxJumpScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString TargetLabelName;
};

struct FVrdxDialogueScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	FVrdxString Speaker;
	FVrdxString Dialogue;
};

struct FVrdxChoiceScriptLine : FVrdxScriptLine
{
	virtual bool Construct() override;
	virtual bool Dispatch(TVrdxSharedPtr<CVrdxNovelScene> Scene) override;

	TVrdxVector<FVrdxChoiceOption> ChoiceOptions;
};
