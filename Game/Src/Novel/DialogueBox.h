// Copyright DCode. All Rights Reserved.
#pragma once

// Third-party Library
#include <SFML/Graphics/RectangleShape.hpp>

namespace sf { class Text; }

// Project Headers
#include "Core/String.h"
#include "Ui/WidgetBase.h"

class CVrdxDialogueBox : public CVrdxWidgetBase
{
public:
	CVrdxDialogueBox(const TVrdxWeakPtr<CVrdxWidgetBase> ParentWidget, const sf::RectangleShape& InShape);
	virtual ~CVrdxDialogueBox() = default;

	// CVrdxBaseWidget 인터페이스
	virtual void Update(const float DeltaTick) override;
	virtual void Draw(sf::RenderWindow& Window) const override;

	virtual bool OnMouseLeftButtonPressed(const sf::Vector2f& LocalPosition) override;
	virtual bool OnKeyboardPressed(const sf::Keyboard::Scancode ScanCode) override;

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
	bool bWaiting;
	TVrdxSharedPtr<sf::Text>           SpeakerText;     // 발화자명 (Font 참조 필요)
	TVrdxSharedPtr<sf::Text>           LineText;        // 대사 내용 (Font 참조 필요)

	// 타이핑 상태
	FVrdxString        CurrentTextString;  // 원본 문자열
	size_t             VisibleCount; // 현재까지 표시된 글자 수
	float              TypeTimer;    // 타이핑 누적 시간
	float              TypeInterval; // 글자 간 시간 간격 (초)

	// 데이터
	FVrdxString        SpeakerName;  // 현재 발화자
};
