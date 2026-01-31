## TL;DR
- 브라우저가 HTML을 화면에 그리는 과정을 이해하기 위해 C++로 작은 브라우저 엔진을 만들었습니다.
- HTML/CSS 파싱, 레이아웃 계산, 렌더링까지 핵심 구조를 직접 구현했습니다.
- 상용 브라우저가 아닌, 학습과 이해를 목표로 한 프로젝트입니다.

# 소개

안녕하세요. 컴퓨터공학과 진학을 앞둔 고3 학생입니다.

저는 기술의 표면이 아닌 내부를 이해하고 싶어 합니다. 이전에 HTML, CSS, JavaScript로 여러 웹사이트를 만들어봤지만, 정작 이 코드를 실행하는 브라우저가 어떻게 작동하는지는 몰랐습니다.

"브라우저는 어떻게 HTML을 화면에 그릴까?"

이 질문에 답하기 위해 약 8주간 직접 브라우저 엔진을 만들어봤습니다.

C++는 이번에 처음 사용했고, 수많은 버그와 씨름했지만, 결국 HTML을 파싱하고 CSS를 적용하며 이미지까지 렌더링하는 작은 브라우저를 완성했습니다. 완벽하진 않지만, 브라우저의 핵심 원리를 이해하는 데는 충분했습니다.

## 목차

- [주요 기능](#주요-기능)
- [기술 스택](#기술-스택)
- [빌드 및 실행](#빌드-및-실행)
- [아키텍처](#아키텍처)
- [프로젝트 구조](#프로젝트-구조)
- [지원하는 CSS 속성](#지원하는-css-속성)
- [어려웠던 점과 해결 방법](#어려웠던-점과-해결-방법)
- [배운 점과 느낀점](#배운-점과-느낀점)

## 주요 기능

이 브라우저는 다음 기능을 지원합니다:

1. **HTML 파싱**: 토큰화, DOM 트리 생성, 자동 오류 교정
2. **CSS 렌더링**: 선택자 매칭, 스타일 상속, Cascade 적용
3. **레이아웃**: Block/Inline 레이아웃, Position 속성 지원
4. **이미지**: 로컬/네트워크/Data URL 지원, 비동기 로딩, 캐싱
5. **네비게이션**: 링크 클릭, 이벤트 버블링, 히스토리(뒤로/앞으로)

<img width="400" height="250" alt="Image" src="https://github.com/user-attachments/assets/40c3bfc2-8b28-412a-9849-b098f9ca60da" />
<img width="400" height="250" alt="Image" src="https://github.com/user-attachments/assets/f0c27381-4b47-49c3-8839-a34d8e150cda" />


## 기술 스택

| 항목 | 설명 |
|------|------|
| **언어** | C++17 |
| **GUI 프레임워크** | Qt6 (Core, Gui, Network) |
| **빌드 시스템** | CMake 3.16+ |
| **렌더링** | Qt Graphics View Framework |

## 빌드 및 실행

### 전제 조건

- macOS, Linux 또는 Windows (MSVC 지원)
- CMake 3.16 이상
- Qt6 설치 (개발 라이브러리 포함)
- C++17을 지원하는 컴파일러

**macOS에서 Qt6 설치 (Homebrew 사용)**:
```bash
brew install qt6
```

**Linux (Ubuntu/Debian)**:
```bash
sudo apt-get install qt6-base-dev cmake build-essential
```

### 빌드 방법

```bash
# 1. 프로젝트 디렉토리로 이동
cd /path/to/small_browser

# 2. 빌드 디렉토리 생성 및 진입
mkdir -p build && cd build

# 3. CMake 빌드 설정
cmake ..

# 4. 빌드 실행
make

# 빌드 완료 후 executable: ./browser
```

### 실행 방법

```bash
# 빌드 디렉토리에서
./browser

# 또는 전체 경로로
./build/browser
```

애플리케이션이 실행되면 기본 GUI 윈도우가 표시됩니다. test_html_files 디렉토리의 HTML 파일을 열어서 렌더링을 확인할 수 있습니다.

### 테스트 실행

```bash
# 빌드 디렉토리에서
cd test
ctest
```

## 아키텍처

### 브라우저 렌더링 파이프라인

실제 브라우저는 다음과 같은 5단계 파이프라인으로 HTML을 화면에 렌더링합니다:

<img width=600 height=300 src="https://webperf.tips/static/4e73c9992ce3b9177bcc80a2113b3138/906b5/BrowserRenderingPipeline01.png" />

이 프로젝트는 이 파이프라인을 다음과 같이 구현했습니다:

### 1단계: 토큰화 (Tokenization)

**목적**: HTML 문자열을 토큰으로 변환

HTML 문서의 첫 번째 단계는 텍스트를 파서가 이해할 수 있는 작은 단위들(토큰)로 분해하는 것입니다.

**핵심 구조체**:
- `TOKEN` (include/html/token.h)
  - `TOKEN_TYPE`: START_TAG, END_TAG, TEXT
  - `value`: 태그명 또는 텍스트 내용
  - `attributes`: 태그 속성 (key-value 맵)

**구현**:
  - (include/html/html_tokenizer.h, src/html/html_tokenizer.cpp)
  - 역할: HTML 문자열을 파싱하여 TOKEN 벡터로 변환
  - 주요 메서드: `tokenize()`

**예시**:
```
Input: <div class="container">Hello</div>
Output: 
  - TOKEN{START_TAG, "div", {"class": "container"}}
  - TOKEN{TEXT, "Hello"}
  - TOKEN{END_TAG, "div"}
```

### 2단계: DOM 트리 생성 (DOM Construction)

**목적**: 토큰들로부터 DOM 트리 생성

토큰들을 계층적인 트리 구조(DOM Tree)로 조직화합니다.

**핵심 클래스**:
- `NODE` (include/html/node.h, src/html/node.cpp)
  - `NODE_TYPE`: ELEMENT, TEXT
  - 속성:
    - `m_tag_name`: 요소의 태그명 (예: "div", "p")
    - `m_text`: 텍스트 노드의 내용
    - `m_children`: 자식 노드들
    - `m_attributes`: 속성 맵 (id, class, src 등)
    - `m_parent`: 부모 노드
    - `m_computed_style`: 계산된 스타일 정보
  
  - (include/html/html_parser.h, src/html/html_parser.cpp)
  - 역할: 토큰 스트림을 파싱하여 DOM 트리 구축
  - 주요 메서드: `parse()`

**메모리 구조**:
```
root NODE
├── NODE (tag: html)
│   ├── NODE (tag: head)
│   │   └── NODE (tag: title)
│   │       └── NODE (text: "My Page")
│   └── NODE (tag: body)
│       ├── NODE (tag: div, attributes: {class: "container"})
│       │   └── NODE (text: "Hello World")
│       └── NODE (tag: img, attributes: {src: "image.png"})
```

### 3단계: CSSOM 생성 및 스타일 적용 (Style Calculation)

**목적**: CSS 규칙을 파싱하고 각 노드에 스타일 적용

CSS 파일을 파싱하여 스타일 규칙을 추출하고, CSS Cascade 규칙에 따라 각 DOM 노드에 최종 스타일을 계산합니다.

**핵심 구조체/클래스**:
- `CSS_RULE` (include/css/css_rule.h)
  - `selector`: CSS 선택자 (예: ".container", "#redBtn")
  - `properties`: 스타일 속성들 (color, font-size, width 등)

- `COMPUTED_STYLE` (include/css/computed_style.h)
  - 각 노드에 최종 적용되는 스타일
  - 속성 예시:
    - `color`: 텍스트 색상
    - `font_size`: 폰트 크기
    - `font_weight`: 폰트 굵기
    - `display`: BLOCK, INLINE, NONE
    - `margin_top/bottom/left/right`: 마진값
    - `padding_top/bottom/left/right`: 패딩값
    - `background_color`: 배경색
    - `position`: Static, Relative, Absolute, Fixed

  - (include/css/css_parser.h, src/css/css_parser.cpp)
  - CSS 규칙 파싱

- `class CSSOM` (include/css/cssom.h, src/css/cssom.cpp)
  - CSS 객체 모델 관리

- `apply_style()` (include/css/apply_style.h, src/css/apply_style.cpp)
  - 함수: CSS 규칙을 DOM 노드에 적용

**스타일 계산 프로세스**:
```
1. CSS 규칙 파싱 → CSS_RULE 저장
2. 각 NODE에 대해:
   - 매칭되는 선택자 찾기
   - 우선순위(specificity) 계산(저는 생략했습니다.)
   - Cascade 규칙 적용
   - COMPUTED_STYLE 생성
```

### 4단계: 레이아웃 계산 (Layout)

**목적**: 각 요소의 위치와 크기 계산

스타일이 적용된 노드들을 기반으로, 각 요소가 화면에서 차지할 위치(x, y)와 크기(width, height)를 계산합니다.

**핵심 구조체**:
- `LAYOUT_BOX` (include/css/layout_tree.h)
  - `node`: 해당 DOM 노드
  - `style`: 적용된 COMPUTED_STYLE
  - `x, y`: 위치
  - `width, height`: 크기
  - `children`: 자식 LAYOUT_BOX들

- `LINE_STATE` (include/css/layout_tree.h)
  - 인라인 레이아웃 계산 시 현재 상태 추적
  - `current_x, current_y`: 현재 위치
  - `line_height`: 현재 줄의 높이

  - (include/css/layout_tree.h, src/css/layout_tree.cpp)
  - 역할: DOM 노드들로부터 LAYOUT_BOX 트리 생성
  - 주요 메서드:
    - `layout_box()`: 블록 레이아웃 계산
    - `layout_inline_children()`: 인라인 요소 레이아웃
    - `layout_text()`: 텍스트 노드 크기 계산
    - `layout_image()`: 이미지 요소 레이아웃

**레이아웃 알고리즘**:
```
BLOCK 요소:
  - 전체 너비 사용
  - 자식들이 수직으로 배치
  
INLINE 요소:
  - 수평으로 배치
  - 텍스트처럼 흐름
  
마진/패딩 처리:
  - total_width = margin_left + border + padding + content + padding + border + margin_right
```

### 5단계: 렌더링 (Painting/Rendering)

**목적**: 계산된 레이아웃을 화면에 그리기

LAYOUT_BOX 트리를 순회하면서 각 요소를 그래픽으로 렌더링합니다.

**핵심 클래스**:
- `class Renderer` (include/gui/renderer.h, src/gui/renderer.cpp)
  - 역할: 레이아웃 박스를 Qt Graphics에 그리기
  - 주요 메서드:
    - `paint_layout()`: 전체 레이아웃 박스 트리 렌더링
    - `draw_element_box()`: 요소의 박스(배경, 테두리) 그리기
    - `draw_text_node()`: 텍스트 렌더링
  - Qt의 `QPainter`를 사용하여 실제 화면에 그림

**렌더링 순서**:
```
1. 배경색 채우기
2. 테두리 그리기
3. 패딩 영역 처리
4. 텍스트 렌더링
5. 이미지 렌더링 (캐시된 이미지 사용)
6. 자식 요소들 순회
```

### 전체 파이프라인 흐름

```
HTML 문자열
    ↓
[HTML_TOKENIZER] - Tokenization
    ↓ TOKEN 벡터
[HTML_PARSER] - DOM Construction
    ↓ NODE 트리
[CSS_PARSER + CSSOM + apply_style()] - Style Calculation
    ↓ NODE + COMPUTED_STYLE
[LAYOUT_TREE] - Layout
    ↓ LAYOUT_BOX 트리
[RENDERER] - Painting
    ↓
화면 출력
```

### 추가 컴포넌트

- **IMAGE_CACHE_MANAGER** (include/gui/image_cache_manager.h)
  - 다운로드된 이미지를 캐시하여 중복 로드 방지
  - QPixmap 저장

- **MAIN_WINDOW** (include/gui/main_window.h, src/gui/main_window.cpp)
  - Qt 메인 윈도우
  - 사용자 인터페이스 제공

### 프로젝트 구조

```
include/
  ├── html/           # HTML 파싱 (token.h, node.h, html_tokenizer.h, html_parser.h)
  ├── css/            # CSS 파싱 및 레이아웃 (computed_style.h, css_parser.h, layout_tree.h 등)
  └── gui/            # 렌더링 및 UI (renderer.h, main_window.h 등)

src/
  ├── html/           # HTML 파싱 구현
  ├── css/            # CSS 및 레이아웃 구현
  └── gui/            # 렌더링 구현
```

## 지원하는 CSS 속성

이 브라우저는 다음의 CSS 속성들을 지원합니다. 모든 속성 파싱은 [include/css/computed_style.h](include/css/computed_style.h)의 `COMPUTED_STYLE` 구조체에서 정의되며, 실제 구현은 [src/css/computed_style.cpp](src/css/computed_style.cpp)에 있습니다.

### 글꼴 속성 (Font Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `color` | 텍스트 색상 | 색상명, hex (#RRGGBB) | black |
| `font-size` | 글씨 크기 | 숫자 + px (예: 16px) | 16px |
| `font-weight` | 글씨 굵기 | normal, bold, 100-900 | normal |
| `font-style` | 글씨 스타일 | normal, italic | normal |
| `font-family` | 글씨 폰트 | 폰트명 | Arial |
| `line-height` | 줄 높이 | 숫자 (배수) | font-size * 1.5 |

### 배경 속성 (Background Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `background-color` | 배경색 | 색상명, hex (#RRGGBB) | transparent |

### 크기 속성 (Dimension Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `width` | 요소 너비 | 숫자 + px, auto | auto (-1) |
| `height` | 요소 높이 | 숫자 + px, auto | auto (-1) |
| `box-sizing` | 박스 크기 계산 방식 | content-box, border-box | content-box |

### 여백 속성 (Margin/Padding Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `margin-top` | 위쪽 마진 | 숫자 + px | 0 |
| `margin-right` | 오른쪽 마진 | 숫자 + px | 0 |
| `margin-bottom` | 아래쪽 마진 | 숫자 + px | 0 |
| `margin-left` | 왼쪽 마진 | 숫자 + px | 0 |
| `margin` | 마진 단축형 | 1~4개 값 | 0 |
| `padding-top` | 위쪽 패딩 | 숫자 + px | 0 |
| `padding-right` | 오른쪽 패딩 | 숫자 + px | 0 |
| `padding-bottom` | 아래쪽 패딩 | 숫자 + px | 0 |
| `padding-left` | 왼쪽 패딩 | 숫자 + px | 0 |
| `padding` | 패딩 단축형 | 1~4개 값 | 0 |

**단축형 속성 사용 예시**:
- `margin: 10px;` → 모든 방향에 10px
- `margin: 10px 20px;` → 위/아래 10px, 좌/우 20px
- `margin: 10px 20px 30px;` → 위 10px, 좌/우 20px, 아래 30px
- `margin: 10px 20px 30px 40px;` → 위 10px, 우 20px, 아 30px, 좌 40px

### 테두리 속성 (Border Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `border-width` | 테두리 두께 | 숫자 + px | 0 |
| `border-color` | 테두리 색상 | 색상명, hex (#RRGGBB) | black |
| `border-style` | 테두리 스타일 | solid, dashed, dotted 등 | solid |
| `border` | 테두리 단축형 | width color style | 0 black solid |

### 레이아웃 속성 (Layout Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `display` | 표시 방식 | block, inline, none | inline |
| `position` | 위치 지정 방식 | static, relative, absolute, fixed | static |
| `top` | 위 위치 (relative, absolute, fixed) | 숫자 + px | 0 |
| `right` | 오른쪽 위치 (relative, absolute, fixed) | 숫자 + px | 0 |
| `bottom` | 아래 위치 (relative, absolute, fixed) | 숫자 + px | 0 |
| `left` | 왼쪽 위치 (relative, absolute, fixed) | 숫자 + px | 0 |

### 텍스트 속성 (Text Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `text-align` | 텍스트 정렬 | left, center, right, justify | left |
| `text-decoration` | 텍스트 장식 | none, underline, line-through, overline | none |

### 기타 속성 (Other Properties)

| 속성 | 설명 | 가능한 값 | 기본값 |
|------|------|---------|--------|
| `opacity` | 투명도 | 0 ~ 1 | 1 |
| `visibility` | 요소 가시성 | visible, hidden (true/false) | visible |

### 구현 세부사항

**속성 파싱 및 설정** ([src/css/computed_style.cpp](src/css/computed_style.cpp)):
- `init_setters()`: 모든 CSS 속성에 대한 setter 함수 등록
- `parse_color()`: 색상값 파싱 (hex, named color)
- `parse_font_size()`: 폰트 크기 파싱
- `parse_string_to_float()`: 숫자값 파싱
- `parse_display_type()`: display 값 파싱
- `parse_text_align()`: text-align 값 파싱
- `parse_box_sizing()`: box-sizing 값 파싱
- `parse_text_decoration()`: text-decoration 값 파싱
- `parse_position_type()`: position 값 파싱
- `parse_spacing_shorthand()`: margin/padding 단축형 파싱

**스타일 상속** ([src/css/computed_style.cpp](src/css/computed_style.cpp)):
- `inherit_color()`
- `inherit_font_size()`
- `inherit_font_weight()`
- `inherit_font_style()`
- `inherit_font_family()`
- `inherit_line_height()`
- `inherit_text_align()`
- `inherit_visibility()`
- `inherit_text_decoration()`

## 어려웠던 점과 해결 방법

C++를 처음 사용했고, 프로젝트의 시작부터 끝까지 예상치 못한 문제와 버그들이 연속으로 나타났습니다. 완전히 해결되지 않은 부분들도 있지만, 가장 어려웠던 3가지 과제를 통해 배운 것들을 정리했습니다.

### 1. 문자열 파싱 (HTML 토큰화, CSS 파싱)

**어려웠던 점:**

파서를 작성할 때 고려해야 할 상태(state)가 매우 많았습니다. HTML 파싱에서는 start tag, end tag, text, comment 등 다양한 경우를 모두 처리해야 했습니다. 처음에는 전혀 어떻게 시작해야 할지 몰라서 막막했습니다.

**해결 방법:**

두 가지 참고 자료를 읽으며 파서의 기본 로직을 이해했습니다:
- https://scrapingant.com/blog/c-plus-plus-parse-html
- https://kangdanne.tistory.com/331

이들 글을 통해 파싱의 기본 원리를 파악한 후, 몇 개의 간단한 파서를 직접 작성해보니 파서의 작동 방식에 익숙해졌습니다. 충분히 이해한 후에는 전체 CSS 파싱 로직을 독립적으로 작성할 수 있었습니다.

### 2. 렌더링 (Rendering)

**어려웠던 점:**

렌더링은 파싱보다도 더 많은 상태를 관리해야 했습니다. `LINE_STATE`, `LAYOUT_BOX` 등 추적해야 할 상태들이 복잡했고, 재귀적으로 구현해야 했는데 재귀 함수 작성에 익숙하지 않았습니다. 또한 박스 요소, 인라인 요소, 텍스트 등 서로 다른 타입의 요소들을 각각 처리해야 했습니다.

**해결 방법:**

Claude AI의 도움을 많이 받아서 렌더링 로직을 이해했고, 코드 기반을 검토하면서 학습했습니다. AI가 작성한 코드를 그대로 사용하지 않고, 코드를 직접 검토하고 질문을 통해 동작 원리를 이해하려고 노력했습니다. 이 과정을 거쳐서 렌더링 로직에 익숙해진 후에는 코드베이스를 수정하고 버그를 수정할 수 있었습니다.

### 3. 이미지 캐싱 및 레이아웃 재계산 (Image Caching/Reflowing)

**어려웠던 점:**

외부 서버에서 HTTP/HTTPS 이미지를 가져오는 것이 가장 복잡했습니다. 이미지를 다운로드하는 중에도 레이아웃 계산을 멈추지 않아야 했으므로 비동기 처리를 이해해야 했습니다. 이론적으로는 이미지 캐싱 및 레이아웃 재계산 시스템을 어느 정도 이해했지만, 실제로 구현하려니 고려해야 할 사항들이 매우 많았습니다.

**해결 방법:**

견고하고 효율적인 이미지 캐싱/레이아웃 재계산 시스템을 설계하는 데 3~5시간을 투자했습니다. 아키텍처를 충분히 설계한 후에는 구현이 훨씬 수월해졌습니다. 이 과정을 통해 멀티스레딩과 비동기 처리, 논블로킹 I/O의 차이를 명확히 이해할 수 있었습니다.

---

이 세 가지 과제를 극복하면서 단순히 기술을 배우는 것뿐 아니라, **문제 해결 방식과 설계의 중요성**을 몸으로 배울 수 있었습니다. 완벽하지는 않지만, 이것이 이 프로젝트의 가장 큰 수확이라고 생각합니다.

## 배운 점과 느낀점

### 기술적 학습

이 프로젝트를 통해 **실제 브라우저가 어떻게 작동하는지** 체득할 수 있었습니다. 단순히 "브라우저는 HTML을 렌더링한다"는 개념에서 벗어나, 토큰화부터 레이아웃 계산, 최종 렌더링까지 각 단계가 어떻게 상호작용하는지 이해하게 되었습니다. 이는 웹 개발을 할 때 브라우저의 동작을 예측하고 성능을 최적화하는 데 도움이 될 것입니다.

### 더 중요한 소프트웨어 엔지니어링 역량

하지만 이 프로젝트에서 배운 가장 중요한 것은 기술 자체가 아니었습니다:

**1. 체계적인 디버깅 능력**

끝없는 버그들과 마주할 때마다, 무작정 코드를 고치는 것이 아니라 가설을 세우고 검증하는 방식으로 접근하게 되었습니다. 적절한 로깅을 통해 프로그램의 상태를 추적하고, 어느 단계에서 문제가 발생하는지 체계적으로 파악했습니다. 이런 디버깅 습관은 C++뿐 아니라 앞으로 사용할 모든 언어에서 도움이 될 것입니다.

**2. 끈기와 인내심**

며칠 동안 해결되지 않는 문제들 앞에서 포기하고 싶을 때가 많았습니다. 하지만 문제를 조금씩 분해하고, 작은 진전들을 축적하면서 결국 극복할 수 있었습니다. 이는 프로그래밍뿐 아니라 모든 분야에서 성공하기 위한 필수 자질임을 깨달았습니다.

**3. 실용주의의 가치**

완벽한 소프트웨어는 존재하지 않습니다. 이 브라우저도 지원하지 않는 CSS 속성과 HTML 요소들이 많습니다, 여전히 여러 버그들도 많고요. 하지만 작동하는 불완전한 제품을 만드는 것이 이상적인 완벽함만을 꿈꾸는 것보다 낫다는 것을 배웠습니다. 이는 실제 소프트웨어 산업에서 매우 중요한 태도입니다.

**4. "왜?"라는 질문의 힘**

AI나 튜토리얼에서 코드를 받았을 때, 단순히 작동하는지만 확인하지 않고 "왜 이렇게 작동할까?", "이 부분이 정말 필요한가?", "다른 방식은 없을까?" 같은 질문을 계속 던졌습니다. 이 호기심과 깊이 있는 탐구가 표피적인 학습을 넘어 원리를 이해하게 만들었습니다.

## 마치며

이 프로젝트의 가장 큰 성과는 완성된 브라우저가 아닙니다.

**문제 해결 능력을 키운 것입니다.**

체계적인 디버깅, 포기하지 않는 끈기, "완벽보다 완성"의 실용주의, 
그리고 항상 "왜?"를 묻는 습관.

이것들은 앞으로 만날 모든 문제에서 저를 도와줄 것입니다.

---

솔직히, 숙련된 개발자분들은 더 완성도 있게 프로젝트를 끝낼 수 있겠지만,

C++를 처음 배우는 학생에게는 **거의 불가능해 보였습니다.**

그럼에도 도전한 이유는 단순했습니다:
- 너무 궁금했고
- 재밌어 보였고  
- 할 수 있을 것 같았습니다

그래서 폴더를 만들고, 에디터를 열고, 코드를 썼습니다.
```bash
mkdir mini_browser
cd mini_browser
# 시작
```

많은 어려움이 있었습니다. 
5시간 디버깅도, 3일째 안 풀리는 버그도 있었습니다.

하지만 8주 후, 작동하는 브라우저가 있습니다.

---

**혹시 지금 불가능해 보이는 것 앞에 있나요?**

시작해보세요.

완벽하지 않아도 괜찮습니다.
느려도 괜찮습니다.
막혀도 괜찮습니다.

**시작하지 않으면, 영원히 불가능합니다.**
**시작하면, 가능해집니다.**

긴 글 읽어주셔서 감사합니다. 🚀

🎉 100+ Stars! 스타가 100개를 넘었네요! 정말 예상하지 못했는데 감사합니다!:)
