---
title: "Editor Phase 3-2: Widget Designer"
project: Voradorix
tags:
  - voradorix
  - editor
  - widget-designer
  - spec
status: planned
---

# Editor Phase 3-2: Widget Designer

> **목표**: 위젯 트리(WidgetBase 기반 UI) 구조를 시각적으로 편집하는 디자이너 구현  
> **의존성**: Phase 2 (Asset Browser) 완료, C++ WidgetBase 계층 안정화  
> **상태**: 기획 단계 (상세는 Phase 2 이후 확정)

---

## 1. 개요

Widget Designer는 게임 화면을 구성하는 위젯(버튼, 텍스트 라벨, 박스 등)의 위치/크기/색상을
시각적으로 편집하고, 그 결과를 JSON 데이터로 저장하여 엔진이 로드할 수 있게 합니다.

이것은 Unity의 Canvas/UI 시스템이나 Unreal의 Widget Blueprint와 유사한 개념입니다.

---

## 2. 핵심 기능 (초안)

### 2.1 캔버스 기반 편집
- 위젯을 캔버스 위에 자유롭게 배치 (드래그 앤 드롭)
- 위치/크기 실시간 조절 (핸들)
- 계층 구조 (Parent-Child) 트리 표시

### 2.2 속성 편집 (Property Panel)
- 위치 (X, Y)
- 크기 (Width, Height)
- 색상 (Background, Border)
- 텍스트 내용 및 폰트
- 텍스처 (등록된 Texture 에셋에서 선택)
- Z-Order

### 2.3 프리팹 개념
- 위젯 디자인 결과를 JSON 파일로 저장 (`Assets/Widgets/`)
- 엔진이 이 JSON을 로드하여 위젯 트리 구성
- 같은 디자인을 여러 Scene에서 재사용 가능

---

## 3. 위젯 JSON 스키마 (초안)

```json
{
  "widgetType": "Button",
  "name": "NewGameButton",
  "position": { "x": 100, "y": 500 },
  "size": { "width": 300, "height": 60 },
  "color": { "r": 255, "g": 255, "b": 255, "a": 255 },
  "text": {
    "content": "새 게임",
    "fontGuid": "c3d4e5f6-a7b8-9012-cdef-123456789012",
    "fontSize": 24,
    "color": { "r": 0, "g": 0, "b": 0, "a": 255 }
  },
  "textureGuid": null,
  "children": []
}
```

---

## 4. 고려사항

- 위젯 디자이너는 Phase 4 이후 상세 설계
- 위젯 JSON을 엔진에서 로드하는 C++ 파서도 함께 개발 필요
- 현재 WidgetBase 계층의 속성(위젯 트리, 이벤트, 렌더)과 정합성 유지가 중요
