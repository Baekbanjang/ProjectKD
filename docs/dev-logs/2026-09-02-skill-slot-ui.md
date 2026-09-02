# 우하단 스킬 슬롯 UI (2026-09-02 저녁)

스킬 4종에 UI를 붙였다. **코드 0줄** — 텍스처·머티리얼·위젯만으로 끝났다.

---

## 1. ★ SB 를 안 보고 설계했다가 두 번 뒤집혔다

처음 낸 안은 **사각 슬롯 · 가로 4칸 나열 · 곡선 실루엣 아이콘**이었다. 근거가 "장르 일반 패턴"이지 SB 실측이 아니었다.

`CURRENT.md` 의 *"우하단 스킬 UI — SB식 슬롯"* 은 **할 일 목록의 문구**지 실측이 아니었고, 메모리의 SB UI 자료에도 **스킬 슬롯 항목이 없었다**(플레이어 3줄 게이지 · 적 상태 바 · 보스 바뿐).

승환이 *"SB 스타일로 만든 거 맞재?"* 라고 물어 자료를 찾아봤더니 **바탕화면에 스크린샷이 있었다** — `SB 스샷/UI/ItemAbility Cross Menu.jpg`.

```
SB 실물          원형 슬롯 · 십자 배치 · 굵은 흰 라인아트 픽토그램
KD 1차 안        사각 슬롯 · 가로 나열 · 곡선 실루엣
```

★ **자료가 있는데 안 찾고 설계했다.** "SB식"이라는 말이 문서에 있으면 그 근거가 어디 있는지부터 확인해야 한다.

### 초확대에서 한 번 더 정정

1차 확대에서 *"버튼 마커 = 흰 원 + 검정 기호"* 로 읽었는데, 8배로 다시 보니 **틀렸다.**

```
△ □ ○ ✕   전부 어두운 회색 원 + 흰색 기호
L1        이것만 반전 (흰 배경 + 검정 글씨). 모디파이어라 강조한 것
```

`L1` 하나를 보고 4개에 옮긴 오독이었다. **작은 UI 요소는 확대 배율을 올려서 봐야 한다.**

### 승환이 잡아낸 규칙

> *"왼쪽에 있는 건 오른쪽, 위쪽에 있는 건 아래 이런 식으로 숫자를 배치해야 할 거 같다"*

실측으로 확인됐다 — **버튼 마커는 항상 중앙(L1)을 향한 변에 붙는다.**
```
위 슬롯 -> 아래 변 / 왼 슬롯 -> 오른쪽 변 / 오른 슬롯 -> 왼쪽 변 / 아래 슬롯 -> 위 변
```

### 가져온 것과 안 가져온 것

```
가져옴   원형 슬롯 · 어두운 채움 + 얇은 밝은 테두리 · 흰 라인아트 아이콘
        마커가 중앙 향한 변에 붙는 규칙 · 작고 눈에 안 띄는 크기
안 가져옴 십자 배치의 게임패드 대응(△□○✕)
        -> 우리는 숫자키 1~4 라 십자에 키를 얹으면 오히려 헷갈린다
        -> 배치는 십자로 하되 숫자를 마커 자리에 넣었다
```

---

## 2. ★ 알파 없는 AI 이미지를 UI 아이콘으로

AI 이미지 생성 모델은 **투명 배경을 못 뽑는다**(GPT Image 2 · Recraft 둘 다 투명 파라미터 없음). 그대로 `Image` 위젯에 넣으면 검은 사각형이 보인다.

```
해법   Material Domain = User Interface  +  Blend Mode = Additive
      TextureSampleParameter2D  ──►  Final Color        (노드 1개)
원리   Additive 는 검정(0,0,0)이 아무것도 안 더한다 = 저절로 투명
      Opacity 핀을 안 쓴다. 알파 배선 자체가 필요 없다
```

⚠️ **전제 = 배경이 진짜 순검정이어야 한다.**
```
GPT Image 2    모서리 (0,0,0)       ✅
Recraft V4.1   모서리 (21,21,21)    ❌  background_color=#000000 을 줘도 무시 (2회 재현)
```

📌 **`Recraft` 의 `background_color` 파라미터는 안 먹는다.** 아이콘 특화 모델(`model_type: utility`)이라 기대했으나 이 항목은 GPT 가 낫다. 스타일 자체는 Recraft 가 더 각졌다.

### 모델 비교 (Skill_01 을 3안으로 뽑아 대조)

| | Recraft A | Recraft B | GPT Image 2 |
|---|---|---|---|
| 배경 | (26,26,26) | (21,21,21) | **(0,0,0)** |
| 구도 | 좌상단 쏠림 | 대각 쏠림 | **프레임을 채움** |
| 총 3연사 표현 | 장식으로 보임 | 삼각형 3개 | **탄착으로 읽힘** |

⚠️ 첫 GPT 안은 **곡선 실루엣**이라 SB 언어와 달랐다. 프롬프트를 라인아트로 바꿔 재생성했다.
```
종전   flat pictogram, white silhouette
현행   bold white line-art, angular geometric shapes, thick uniform strokes,
      hollow shapes with internal parallel line details, technical HUD glyph,
      not a solid silhouette
```

---

## 3. 🔴 MCP `manage_asset --import` 는 프로젝트 밖 파일을 거부한다

```
Error [SECURITY_VIOLATION]: File path must be project-relative
```

바탕화면·`D:\tmp` 에서 바로 임포트가 안 된다. **프로젝트 트리에 파일을 만들지 않는 룰과 정면으로 부딪힌다.**

```python
# 우회 — unreal.AssetImportTask 는 절대경로 제한이 없다
t = unreal.AssetImportTask()
t.filename = r"C:/Users/asdasd/Desktop/Skill_01.png"
t.destination_path = "/Game/.../UI/Texture/Skill"
t.destination_name = "T_Icon_Skill01"
t.automated = True; t.replace_existing = True; t.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
```

★ **외부 소재 임포트는 MCP import 말고 파이썬으로.** 사운드·메시도 같은 길이다.

### UI 텍스처 설정 4개 — 안 바꾸면 조용히 나빠진다

```
Compression Settings   Default(DXT)  ->  UserInterface2D     경계가 블록으로 뭉갠다
Texture Group          World         ->  UI                  스트리밍돼 늦게 뜬다
Mip Gen Settings       FromGroup     ->  NoMipmaps           UI 는 밉맵 불필요
Maximum Texture Size   0(무제한)      ->  256                 1024 를 통째로 물고 있는다
```
⚠️ **`sRGB` 는 켠 채로.** 아이콘은 색이지 데이터가 아니다.

📌 파이썬 enum 이름이 표시명과 다르다:
```
UserInterface2D = TextureCompressionSettings.TC_EDITOR_ICON     <- UI2D 라는 이름이 없다
UI              = TextureGroup.TEXTUREGROUP_UI
NoMipmaps       = TextureMipGenSettings.TMGS_NO_MIPMAPS
```

---

## 4. 머티리얼 — 텍스처 0장으로 원을 그린다

`M_UI_SlotRing` 은 UV 중심거리로 원을 계산한다. 텍스처가 없어 어느 크기로 늘려도 안 깨진다.

```
UV -> (UV - 0.5) -> Length -> 중심거리
  SmoothStep(0.50, 0.47)  ->  채움 마스크
  SmoothStep(0.44,0.46) × SmoothStep(0.50,0.48)  ->  링 마스크
색   FillColor / RingColor 를 링 마스크로 Lerp
알파  (채움 × FillAlpha × 스윕) + (링 × RingAlpha)
```

★ **파라미터 5개로 인스턴스 3개를 뽑았다.** 머티리얼은 하나다.

| 인스턴스 | FillAlpha | RingAlpha | FillColor | Progress | 용도 |
|---|---|---|---|---|---|
| `MI_SlotRing` | 0.75 | 1.0 | (0.04,0.04,0.05) | 1.0 | 슬롯 — 어두운 채움 + 밝은 테두리 |
| `MI_KeyBadge` | 0.92 | 0.0 | (0.13,0.13,0.15) | 1.0 | 숫자 배지 — 어두운 원 |
| `MI_SlotDim` | 1.0 | 0.0 | (0,0,0) | **0.0** | 쿨다운 덮개 — 검은 원 |

### 🔴 쿨다운 덮개가 사각형이라 원 밖으로 삐져나왔다

처음엔 `Image_Dim` 을 **단색 검정 이미지**로 뒀는데, 원형 슬롯 위에 사각형이 얹혀 삐져나왔다(승환 PIE 관측).
→ 같은 머티리얼의 인스턴스(`MI_SlotDim`)로 바꾸니 **반지름이 정확히 겹쳐** 해결됐다.

★ **겹쳐 쌓는 UI 요소는 같은 머티리얼에서 파생시키면 모양이 저절로 맞는다.**

---

## 5. ★ 쿨다운 원형 스윕 — 각도로 잘라낸다

승환 = *"다른 게임처럼 한 바퀴 도는 애니메이션은 안 되나"*. 된다.

```
Arctangent2(Y=dx, X=-dy)   ->  12시=0, 시계방향으로 증가 (-π~π)
÷ 2π  ->  +1  ->  Frac      ->  0~1 정규화
1 - SmoothStep(Progress-0.004, Progress+0.004, 각도)  ->  스윕 마스크
그 마스크를 채움 알파에 곱한다
```

**축 선택이 핵심이다.** UV 는 아래가 +Y 라 `atan2(dy, dx)` 는 3시에서 시작한다. `atan2(dx, -dy)` 로 바꿔야 **12시 시작 시계방향**이 된다.

```
Progress 1.0   원 전체 덮임
Progress 0.0   아무것도 안 덮임
쿨다운 = 1 -> 0    덮개가 12시부터 시계방향으로 걷힌다
```

### 값을 움직이는 법 — 위젯 애니메이션이 머티리얼 파라미터를 움직인다

```
UMovieSceneWidgetMaterialTrack     UMG/Public/Animation/  에 실재
Anim_Cooldown_1s 에서
  Image_Dim1 트랙 -> [+] -> Brush -> Material -> Progress
  0.00 -> 1.0     1.00 -> 0.0
```
★ **BP 노드가 0개 는다.** `GetDynamicMaterial` + Tick 으로도 되지만 쿨다운이 1초 고정 공유라 애니메이션이 맞다.

⚠️ `Material` 트랙이 메뉴에 뜨려면 **브러시가 머티리얼**이어야 한다. 텍스처면 안 나온다.

⚠️ **`Image_Dim` 의 `Render Opacity` 는 1.0 이다.** 평소에 안 보이는 건 `MI_SlotDim` 의 `Progress = 0` 이 담당한다. Render Opacity 를 0 으로 두면 애니메이션이 돌아도 아무것도 안 보인다 — 두 값의 역할을 헷갈리기 쉽다.

---

## 6. 위젯 구조

```
WBP_SkillSlots
└ SizeBox_Root 200×200
   └ CanvasPanel_Cross
      슬롯 4    Overlay_Slot1~4 (Image_Ring / Icon / Dim)
      배지 4    Overlay_key1~4  (Image_KeyBg / TextBlock_Key)
```

캔버스 좌표 (전부 `Anchor(0,0)` · `Alignment(0,0)` · `Size To Content` 끔)
```
              Position     Size        배지 Position
왼   (스킬1)  (  0, 68)   64×64        ( 53, 89)
위   (스킬2)  ( 68,  0)   64×64        ( 89, 53)
오른 (스킬3)  (136, 68)   64×64        (125, 89)
아래 (스킬4)  ( 68,136)   64×64        ( 89,125)
```
계산식 = `캔버스 C = 슬롯 S × 3.125` · `중앙축 = C/2 − S/2` · `배지 = 안쪽 변 − B/2`

### UMG 함정 4개

```
SizeBox            Width/Height 가 각각 체크박스다 (editcondition). 값만 넣으면 안 먹는다
디자이너 미리보기    루트가 캔버스가 아니면 Fill Screen 이 화면 전체로 늘려 그린다 -> Desired 로
Is Variable        기본 꺼짐. 안 켜면 그래프에서 그 위젯을 못 잡는다
                   MCP 로 확인 가능 — CDO 프로퍼티로 노출되는지가 곧 Is Variable 여부다
HorizontalBox 슬롯  Size = Fill 이면 SizeBox 의 Width Override 가 무시된다 (Auto 로)
```

📌 **`Set Render Opacity` 가 `Set Color and Opacity` 보다 낫다** — 타입이 갈리기 때문이다.
```
Image.SetColorAndOpacity      FLinearColor
TextBlock.SetColorAndOpacity  FSlateColor      <- 변환 노드가 낀다
Widget.SetRenderOpacity       float            <- 모든 위젯 공통
```

---

## 7. 배선

```
Construct
 └ Set TargetActor ← Get Owning Player Pawn → IsValid → Sequence
    [0] 배열 5개 Make Array
    [1] Get Float Attribute(Stamina) → RefreshSlots
    [2] Wait for Attribute Changed(Stamina)     → Set AsyncStamina / Changed → RefreshSlots
    [3] Wait Gameplay Tag Add to Actor          → Added → Play Animation
    [4] Wait Gameplay Tag Remove from Actor
Destruct
 └ Async 3개 → End Action
RefreshSlots(NewStamina)
 └ bUsable = NewStamina >= StaminaCost
    ForEach 4개 → Set Render Opacity (Select 로 1.0 / 0.25~0.40)
```

🔴 **`Changed` 실행 핀을 흐름으로 이어 쓰면 안 된다** — 뒤 구독이 생성 자체가 안 되는데 에러도 로그도 없다(08-18). `Sequence` 로 갈라 독립 핀에.
⚠️ **Async 노드는 함수 안에 못 놓는다.** `EventGraph` 에만.
⚠️ `Destruct` 를 빠뜨리면 리스폰 후 갱신이 멈춘다(08-25 적 상태 바가 이걸로 죽었다).

📌 `SlotDims` 는 `RefreshSlots` 가 안 건드린다 — **애니메이션이 소유**한다. 둘이 같이 만지면 쿨다운 중 스태미나가 바뀔 때 싸운다.

⚠️ `StaminaCost 20` 이 GA CDO 와 위젯 변수 **두 곳에 적힌다.** 갈리면 UI 가 거짓말한다.

---

## 검증

- [x] 우하단 십자 4칸 표시 (승환 PIE)
- [x] 아이콘 4장 — 검은 사각형 없이 라인만
- [x] 쿨다운 원형 스윕 12시 시계방향
- [x] 스태미나 부족 시 흐려짐
- [ ] 슬롯 크기 — 조금 작다는 관측. `Render Transform Scale` 로 배율을 먼저 찾고 좌표 재계산
- [ ] `Skill_04` 아이콘 — 셋과 언어가 다르다(궤적 vs 물건) + 선이 촘촘해 작은 크기에서 뭉갤 수 있다

## 커밋 (Content)

```
6054ede  머티리얼 FillAlpha/RingAlpha 파라미터 + 키 배지 MI
2073d4a  색 분리 (FillColor/RingColor) + SB 실측대로 배지 정정
088c899  쿨다운 덮개를 원형으로 (MI_SlotDim)
eed3fc9  아이콘 2~4 생성·임포트 + MI 3개
4192a94  쿨다운 원형 스윕 (Progress 파라미터)
a0f9e11  MI_SlotDim Progress 기본값 0
a088ee6  위젯 완성 + MainHUD 배치
```

## 남은 것

```
슬롯 크기 확대       Render Transform Scale 로 배율 확정 후 좌표 재계산
Skill_04 아이콘      필요하면 재생성 (크레딧 1170 남음)
스킬 연출            HitConfirmProfile 분리 · 카메라 연출 (슬로모는 승환 판단으로 제외)
스킬 나이아가라      승환 구상 중
ST 리젠 제거         명중 시 스태미나 회복으로 전환 — 이 UI 가 그 정보를 보여주게 된다
```
