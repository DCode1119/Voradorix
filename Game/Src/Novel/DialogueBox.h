// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

// Project Headers
#include "Core/String.h"
#include "Ui/WidgetBase.h"

class CVrdxDialogueBox : public CVrdxWidgetBase
{
public:
	CVrdxDialogueBox(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	virtual ~CVrdxDialogueBox() VRDX_DEFAULT;

	// CVrdxBaseWidget 인터페이스
	virtual void Update(const float DeltaTick) VRDX_OVERRIDE;
	virtual void Draw(sf::RenderWindow& Window) const VRDX_OVERRIDE;

	virtual void OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) VRDX_OVERRIDE;
	virtual void OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode) VRDX_OVERRIDE;

	// 대사 설정
	void SetSpeaker(const FVrdxString& Name);
	void SetLine(const FVrdxString& Text);

	// 상태
	bool IsTyping() const;      // 타이핑 진행 중?
	bool IsFinished() const;    // 현재 대사 출력 완료?
	bool IsWaiting() const;
	void FinishTyping();        // 즉시 전체 표시

	void AdvanceProcess();

private:
	void StartTyping();

	// 위젯
	sf::Font           Font;            // 폰트
	bool               bFontLoaded;     // 폰트 로드 성공 여부
	bool bWaiting;
	sf::Text           SpeakerText;     // 발화자명 (Font 참조 필요)
	sf::Text           LineText;        // 대사 내용 (Font 참조 필요)

	// 타이핑 상태
	FVrdxString        CurrentText;  // 원본 문자열
	size_t             VisibleCount; // 현재까지 표시된 글자 수
	float              TypeTimer;    // 타이핑 누적 시간
	float              TypeInterval; // 글자 간 시간 간격 (초)

	// 데이터
	FVrdxString        SpeakerName;  // 현재 발화자
};
