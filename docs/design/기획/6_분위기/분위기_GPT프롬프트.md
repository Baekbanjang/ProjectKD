---
title: 분위기 GPT 이미지 생성 프롬프트
tags:
  - ProjectKD
  - 기획
  - 분위기
  - GPT
  - prompt
created: 2026-05-14
---

# 📷 분위기 GPT 이미지 생성 프롬프트

> 병맛 시그니처 모먼트 컨셉 이미지 생성용 영문 프롬프트.
> 도술 프롬프트 (`도술_GPT프롬프트.md`) 9-Section 구조 차용.
> 결과 저장 경로: `notes/ProjectKD/기획/6_분위기/자료/M_공중QTE_셀카.png`
> 본문: [[기획/6_분위기/6_분위기]]

---

## 🎨 공통 컨텍스트 (도술 프롬프트와 동일 정책)

### 캐릭터 (필수 — 원화 첨부)
- 🎨 **GPT 호출 시 매번 캐릭터 원화 첨부**. 원화 = 외형의 single source of truth.
- 영문 프롬프트에 외형 명시되어 있어도 **원화 우선 적용**.

### 환경
- 조선 산골 도적촌. **정오 맑음 (12~14시)**, 강한 직사광, 짙은 그림자, 황토색 강조 (6_분위기 환경 톤 결정).

### 컬러 팔레트
- 먹 `#1A1410` · 황토 `#C07D3A` · 이끼 `#4A6741` · 솔잎 `#2D4A2A` · 황금 `#E8C547` · 한지 흰 `#F0EAD6`
- ❌ 네온 / 채도 높은 컬러 / 사이버 글로우 금지

### 톤
- 묵직 동양 무협 베이스 + **메타 코믹 한 모먼트**.
- ❌ DMC·Bayonetta식 모션 과장 (액션 자체 묵직 유지 — 묵직 베이스 약화 금지).

---

## 📱 M1. 공중 QTE 마무리 셀카 (병맛 시그니처)

### 🇰🇷 한국어 요약 (이해용)
- **장면**: 공중 QTE 6단계(지면 내리꽂기) **직후 슬로우모 정지 모먼트** — 적은 이미 머리부터 땅에 박혀 다리만 위로 / 길동은 **여전히 공중 ~3~4m 부양 상태**. 길동이 공중에서 **몸을 비틀어** 자기 + 아래 박힌 적이 모두 스마트폰 셀카 프레임에 들어가도록 자세 잡음.
- **포즈**: 공중 부양 / 한 손엔 봉(밸런스용으로 옆 펼침), 다른 손엔 **현대 스마트폰** 셀카 모드로 얼굴 옆 들어 올림. 시선은 폰 화면, **무표정에 가까운 미세 미소** (deadpan).
- **카메라 시점**: **ZZZ 궁극기 컷어웨이 / 명조 에이메스 스킬 컷어웨이** 패턴 — 표준 게임플레이 어깨너머 ❌ / **시네마틱 와이드 3D 컷어웨이** — 약간 측면 + 약간 아래에서 위로 (길동의 공중 부양감 강조), 한 프레임 안에 **공중 길동** + **아래 크레이터 박힌 적** 모두 잡음.
- **배경**: 도적촌 두목 앞마당, 정오 직사광, 흙 크레이터, 부서진 잔해, 흙먼지·낙엽 슬로우.
- **분위기**: 액션 임팩트 직후 시네마틱 정지 컷 + 메타 코믹. 조선 무협 + **스마트폰** = 시대 충돌 병맛 (전체 농담의 핵심).
- **❌ 절대 금지**: 장승(Jangseung totem) / 다른 적·잡몹 / 별 이펙트 / 황금 사운드 웨이브 — S3 정승 스턴 reference 오염 방지.

### 🇬🇧 영문 프롬프트

```
A cinematic in-game screenshot from a Korean martial arts action RPG demo
"Black Gildong" (흑길동). 16:9 aspect ratio, photo-realistic 3D render
quality. This is a SIGNATURE COMEDIC BEAT — a brief animation sequence
that interrupts intense action with a meta-humor moment (structurally
similar to short character-focused skill cutaways in modern action RPGs
like Wuthering Waves character close-ups, but adapted to a deadpan
Joseon-era setting).

— CAMERA (CRITICAL) —
STANDARD GAMEPLAY THIRD-PERSON OVER-THE-SHOULDER game camera —
EXACTLY MATCHING the attached reference screenshot of the "지면 내리꽂기"
finisher slow-motion frame. This is the SAME in-game player POV as the
slam-down finisher moment, just one frame later (now the selfie beat).

⚠️ MATCH REFERENCE SCREENSHOT: camera position, framing, distance, angle,
and HUD layout MUST match the attached reference image as closely as
possible. Camera is positioned BEHIND Gildong's shoulder, slightly
elevated, looking PAST his upper back / shoulder DOWN toward the crater
+ impaled enemy in the mid-to-lower ground.

Gildong's BACK / shoulder / back of head/hat occupies the LEFT 1/3 of
the frame in the foreground (large in frame, just like the reference).
The crater + impaled enemy + radial dust pattern fill the MID-GROUND
ahead and below. NO wide cinematic cutaway, NO 3/4 side-front angle,
NO low up-angle, NO floating cinematic camera — this is the
STANDARD GAMEPLAY POV continuing from the slam-down screenshot.

— SUBJECT (Gildong — AERIAL SELFIE POSE) —
The protagonist (Gildong) — match his appearance EXACTLY to the attached
character reference artwork. The reference is the SINGLE SOURCE OF TRUTH
for face, hair, body type, outfit, and weapon design. Do NOT invent new
features.

⚠️ AERIAL POSITION: Gildong is SUSPENDED IN MID-AIR (slow-motion
aftermath of his "지면 내리꽂기 / aerial slam-down" finisher — he is
still floating, mid-descent, hasn't landed yet — same vertical position
as the reference slam-down frame). NOT standing on the ground.

⚠️ POSE CHANGE FROM REFERENCE: In the reference slam-down frame, Gildong
is gripping his staff with both hands, slamming downward. In THIS frame
(one moment later), he has shifted into a SELFIE POSE:
- LEFT hand still loosely holds the long wooden staff (봉) — staff
  trailing behind him at an angle, still in the air.
- RIGHT hand is raised UP and slightly OUT to the side at head height,
  holding a MODERN BLACK SMARTPHONE in SELFIE orientation (phone screen
  angled toward Gildong's face, back of the phone partially visible to
  the over-the-shoulder camera).
- Torso is slightly twisted/rotated so his shoulder line angles to fit
  himself + the impaled enemy below into the selfie frame.
- ⚠️ HEAD / FACE TURNED TO PLAYER CAMERA (4th-wall break):
  Gildong's HEAD is turned BACK toward the over-the-shoulder PLAYER
  CAMERA — looking DIRECTLY AT THE VIEWER, NOT at the phone screen.
  His face is partially visible to the player POV (3/4 back angle of
  face — we see one cheek, the side of his jaw, one eye glancing
  knowingly at the camera). This is a deliberate META 4TH-WALL BREAK —
  Gildong acknowledges the player while still posing for the in-world
  selfie. The phone selfie pose continues mechanically (right arm
  raised, phone in selfie position) but his eye-contact is with the
  player, not the phone.

⚠️ HAND ANATOMY (CRITICAL — common AI failure point):
Both hands MUST be anatomically correct. Five fingers per hand. Natural
finger placement gripping the smartphone (thumb on one side, four
fingers on the other side wrapping the back of the phone — typical
selfie grip). NO extra fingers, NO missing fingers, NO distorted /
melted / fused fingers, NO unnatural wrist angles. Same care for the
left hand gripping the staff.

Robe, sash ribbons, and hair are mid-flutter from the suspended motion.

— KEY PROP: THE SMARTPHONE (anachronism — entire point of the gag) —
A clearly MODERN, CONTEMPORARY black smartphone — generic flagship
design with a black glass front, slim black bezel, glass back. No brand
markings, no logo. Held vertically in one hand in SELFIE mode.

The phone's SCREEN is angled so that the cinematic camera can partially
see its content. The screen shows a LIVE SELFIE PREVIEW — a tiny
version of Gildong's face in the upper portion, and BELOW him in the
selfie frame, a tiny visible portion of the impaled enemy + crater on
the ground far below. A small white camera UI overlay on the screen
(round shutter button at the bottom, small flash icon at top). Subtle
selfie-mode horizontal mirroring on the live preview.

⚠️ CRITICAL: This MUST be a recognizable CONTEMPORARY smartphone. Do
NOT substitute a bronze mirror, hand mirror, scroll, talisman, jade
disc, paper fan, or any period-appropriate Joseon object. The visual
contradiction between the Joseon-era warrior and the MODERN SMARTPHONE
IS the entire joke.

— THE DEFEATED ENEMY (in the crater BELOW Gildong) —
A SINGLE bandit enemy is HEAD-FIRST EMBEDDED in the dirt at the center
of the crater DIRECTLY BELOW the floating Gildong — only his lower
torso, legs, and feet protrude straight up, limp and slightly splayed
in a comedic posture. Dust still drifts off him. His weapon (a crude
axe or sword) lies broken to one side. NO blood, NO gore, NO wounds —
the impact is comedic-stylized slapstick, not violent.

⚠️ EXACTLY ONE defeated enemy. NO crowd of standing bandits, NO other
enemies in the frame. Only the single impaled bandit in the crater.

— ENVIRONMENT —
Joseon-era mountain bandit village courtyard at MIDDAY (12~14시) under
strong direct overhead sunlight. Sharp shadows, warm ocher-and-earth
ground tones. Weathered wooden palisade walls, straw-thatched rooftops,
mossy stone walls flank the background. Distant pine trees on the far
ridge. Loose earth, broken pottery shards, splintered wooden fragments
scattered around the crater rim. Dust, falling leaves, and pine needles
drift in slow motion through the shafts of sunlight piercing the haze.
The crater itself is roughly 2~3 meters across with radial cracks and
embedded debris.

⚠️ ABSOLUTELY NO Korean Jangseung totem poles (장승 / wooden totems
with carved faces) anywhere in the frame. NO ceremonial wooden statues.
NO summoned spirits. The environment is ONLY natural courtyard
buildings + ground + distant trees.

— ACTION & VFX —
SLOW-MOTION FROZEN AFTERMATH — dust and debris are mid-suspension, hanging
in the air, particles slowly drifting. A subtle radial dust shockwave still
expanding outward from the crater. Faint heat-haze shimmer above the crater
from dissipating impact energy. Soft motion blur ONLY on Gildong's robe
edges and trailing hair (he is still settling from the aerial slam).

❌ ABSOLUTELY NO golden energy beams, NO fire, NO neon, NO lightning,
NO yellow stun-stars around any character, NO sound-wave ripples, NO
glowing auras. Only dust, broken wood, dirt clods, and sunlight god-rays.

— EXPRESSION & STANCE (Gildong) —
Gildong's body is dynamically suspended — torso twisted, one leg slightly
raised, the other extended, robe billowing from the aerial freeze. He
looks confident and completely deadpan. His face is turned BACK toward
the player camera (4th-wall break — see SUBJECT section). His expression
is a SUBTLE SMUG HALF-SMILE with EYES MEETING THE PLAYER directly —
barely perceptible smirk, almost imperceptible amusement. NOT a wide
grin, NOT a wink, NOT exaggerated celebration. He looks at the player
with a quiet "yeah, I know" energy while continuing the selfie pose.
The deliberate DEADPAN 4TH-WALL EYE CONTACT is the joke.

— COLOR PALETTE —
Ink black (#1A1410), ocher (#C07D3A), moss green (#4A6741), pine dark
(#2D4A2A), accent gold (#E8C547) only on subtle outfit details and the
small HUD elements, hanji-paper white (#F0EAD6). The smartphone is
plain matte black / glass — no glowing screen beyond a soft natural
display luminance. ABSOLUTELY NO neon, NO saturated cartoon colors.

— UI / HUD (MATCH the attached reference screenshot exactly) —
HUD layout MUST match the attached "지면 내리꽂기" reference screenshot
exactly — same layout, same positions, same elements. Standard gameplay
HUD remains visible (this is the standard gameplay POV continuing into
a brief gag pose, NOT a cinematic cutaway).

Match reference HUD elements:
- TOP-LEFT: Gildong portrait (circular frame with hat) + three horizontal
  bars (HP red / Stamina yellow / Doul blue) with current/max numbers.
  Doul depleted (~30% — just used 동에번쩍 R).
- BOTTOM-LEFT: small inventory slots "4" (술병 potion icon) and "5"
  (약초 herb icon) with stack count numerals.
- BOTTOM-CENTER: thin boss HP bar with "산적 두목" label, slightly
  depleted (boss-stagger moment from finisher). The "Stagger Recovery
  2.0s" indicator from the reference may still appear.
- BOTTOM-RIGHT: three circular doulsul slots labeled [1] [2] [3] with
  ability icons (matching reference), and Shift run icon to the right.

❌ NO HP numbers floating mid-screen.
❌ NO minimap.
❌ NO additional UI elements not present in the reference screenshot.

— STYLE REFERENCE —
Cinematic framing of Stellar Blade signature character moments,
environmental texture density of Black Myth: Wukong, color discipline
of Sekiro: Shadows Die Twice, restrained Korean wuxia overlay. The
MOMENT itself (a brief character-focused beat that breaks combat flow)
is structurally similar to skill-activation cutaway shots in modern
action RPGs (e.g., Wuthering Waves character skill close-ups) — a short
visual flourish, not a full cinematic. Photo-realistic 3D, NOT anime,
NOT illustration.

— TONE (CRITICAL) —
A serious Joseon-era wuxia setting INTERRUPTED by an anachronistic
smartphone selfie — the entire image is built on this single visual
contradiction. Gildong's stoic, deadpan demeanor + the absurdity of the
modern phone = the comedy. The action environment (dust, crater,
impaled enemy) remains visually heavy and serious; the smartphone alone
carries the comedic weight. Do NOT exaggerate Gildong's pose or
expression — the dry, understated delivery IS the joke.

— TECHNICAL —
16:9, 1920x1080 minimum, HDR-graded, light film grain. Gildong on the
right rule-of-thirds vertical line. Crater + impaled enemy occupy the
left two-thirds in the mid-ground. Background palisade walls in soft
focus. Depth-of-field with Gildong tack-sharp, smartphone screen
readable but small, crater slightly soft. Suspended dust particles
catching the sunlight. Calm, deadpan, ironically heroic atmosphere.
```

---

## 🔫 M2. 절벽 글록 시퀀스 (영상용 3장 세트)

> **용도**: Kling AI 시네마틱 컷씬 영상 (10초)의 시작·중간·끝 프레임.
> **컨셉**: 절벽 위 보스 처치 후 → 글록 17 reveal → 연기 부는 클래식 액션 영화 클리셰.
> **톤**: 긴장 70% + 코믹 30% (시대 충돌 = 코믹 핵심).
> **저장**: `자료/M2_절벽_대치.png` / `자료/M3_절벽_글록.png` / `자료/M4_절벽_연기후.png`

### 🎨 공통 컨텍스트 (3장 모두 동일 적용)

#### 캐릭터 (필수 — 원화 첨부)
- 🎨 **GPT 호출 시 매번 길동 캐릭터 원화 첨부**. 원화 = 외형의 single source of truth.
- 영문 프롬프트 외형 명시되어 있어도 **원화 우선 적용**.
- 적 캐릭터는 텍스트 묘사 (조선 도적: 거친 외형, 검은·황토 의상, 단검 또는 단도)

#### 환경 (3장 일관 유지)
- **절벽 위, 일몰 직전 (golden hour)**, 강한 측면 역광, 황토·주황·먹빛 산 실루엣
- 멀리 산맥 능선, 드라마틱한 구름, 바람 부는 분위기
- 한국 산수화 무협 정통

#### 컬러 팔레트
- 먹 `#1A1410` · 황토 `#C07D3A` · 주황 일몰 `#E89143` · 먹빛 산실루엣 `#3D332A` · 한지 흰 `#F0EAD6`
- ❌ 핏빛 (피 분출) / 네온 / 네온 사이버 글로우 / 채도 높은 만화 컬러

#### 톤 일관성
- 정적 동양 무협 영화 베이스 + 글록(modern Glock 17) = 시대 충돌 코믹
- ❌ DMC식 모션 과장 / 만화식 폭발

---

### 📸 M2-A. 시작 프레임 (Beat 1 — 승리 후 접근) ✅ 딥 인터뷰 확정

> 딥 인터뷰 결과 (모호도 21.3%, 3R) — 사용자 비전 정밀 캡처본.

#### 🇰🇷 한국어 요약
- **장면**: 절벽 위, 늦은 황금 일몰. **길동의 뒷모습 (50%)**이 화면 우측 foreground, 쓰러진 적이 mid-ground (3~5m 앞)에 옆으로 누움. 길동이 **걷는 중** (한 발 내딛는 중간), 봉을 오른손에 지팡이처럼. 적은 입 살짝 열려 있음 (다음 컷 위협 발언 호환).
- **시점**: **OTS Medium back shot** — 길동 어깨너머로 적을 향함. 길동 얼굴 X, 뒷모습만.
- **분위기**: **정적 긴장감** — 폭풍 직전 침묵, 사냥꾼이 부상당한 사냥감에 다가가는 톤.
- **저장**: `자료/M2_절벽_접근.png`

#### 🇬🇧 영문 프롬프트

```
A cinematic in-game cutscene establishing shot from a Korean wuxia 
action RPG. 16:9 aspect ratio, photo-realistic 3D render with 
cinematic color grading.

— CAMERA —
MEDIUM OVER-THE-SHOULDER SHOT from BEHIND Gildong. Camera positioned 
3~4 meters behind Gildong's left shoulder at chest-to-shoulder height. 
Gildong's BACK fills approximately 50% of the frame (lower-right area). 
The camera is locked behind him, looking past his shoulder toward the 
fallen enemy in the mid-ground (3~5 meters ahead).

⚠️ NEVER show Gildong's face. ONLY his BACK, hat, dreadlocks, and 
shoulders are visible. This is a classic OTS "approach" composition.

— SUBJECT (Gildong — foreground right, BACK visible only) —
The protagonist (Gildong) — match his appearance EXACTLY to the 
attached character reference artwork. The reference is the SINGLE 
SOURCE OF TRUTH for hair (dreadlocks), body, outfit (blue robe, sat 
hat), and weapon. Do NOT invent new features.

In this scene: Gildong is in MID-STRIDE, walking slowly toward the 
fallen enemy. ONE FOOT FORWARD (caught mid-step), the other foot 
planted behind. His RIGHT HAND holds the long wooden staff (봉) 
loosely, butt of the staff just touching the ground beside him (using 
it like a walking aid). His LEFT HAND hangs naturally at his side. 
Hair and robe blowing gently in the cliff wind. Posture is calm and 
controlled — a victorious warrior approaching his fallen opponent.

The viewer sees his back from a 3/4 back angle (slight side profile 
of hat brim visible). Quiet menacing presence.

— SUBJECT (Defeated Bandit — mid-ground, 3~5m ahead) —
A defeated Korean Joseon-era bandit lies on his SIDE on the rocky 
cliff ground, 3~5 meters ahead of Gildong. Body partially curled, 
one arm extended awkwardly. His FACE is angled toward the camera 
(profile/3-quarter view) so the viewer can see his MOUTH and EYES. 
Mouth slightly parted as if about to speak. Eyes narrowed defiantly — 
still alive, still dangerous, still proud despite defeat. Bruises 
and a small streak of dirt on his cheek (NOT gore, NOT graphic). 
Black-and-ocher tattered robe. His broken weapon (a single short 
sword or dagger) lies just out of his reach. Hair disheveled.

— ENVIRONMENT —
Mountain cliff edge at LATE GOLDEN HOUR — the sun has nearly set, 
casting strong orange and ember-toned back-lighting from the horizon. 
Vast mountain ranges silhouetted in the distance with ink-painting 
quality (먹빛 산). Dramatic clouds with deep orange-purple gradient. 
Cliff edge made of jagged rocks, loose dirt, sparse pine needles. 
Strong cliff wind blowing dust, falling pine needles, and robes.

Korean Joseon-era setting. NO modern objects visible in this frame. 
NO blood pools, NO graphic gore.

— ACTION & VFX —
Wind particles (dust, falling leaves, pine needles) flowing through 
the frame from one side. Hair and robes mid-flutter. Sun god-rays 
piercing through the cliff atmosphere from behind the mountains. 
Atmospheric haze for depth perception. Slight motion blur on 
Gildong's stride-forward foot suggesting movement.

This is a QUIET, TENSE moment — no active combat, just the slow 
deliberate approach of the victor to the fallen.

— EXPRESSION & STANCE —
Gildong (back view only — face NOT visible): calm, deliberate, quietly 
menacing. Walking slowly, almost casually. The kind of approach that 
says "I have all the time in the world." Quiet dominance through 
pure body language and pace.

Bandit: defeated but defiant. Mouth parted mid-curse, eyes narrowed 
venomously toward Gildong. NOT broken — still proud, still dangerous, 
still has one final word to say.

— COLOR PALETTE —
Late sunset orange (#E89143), warm gold (#E8C547), ocher (#C07D3A), 
ink-mountain silhouette (#3D332A), warm dust amber, hanji-paper 
white (#F0EAD6) on Gildong's inner robe accents. Strong warm 
rim-light on Gildong's silhouette from the setting sun behind.

NO bright red blood, NO neon, NO saturated cartoon colors.

— UI / HUD —
NO HUD. This is a CINEMATIC CUTSCENE — gameplay HUD is hidden during 
cutscenes. Pure cinematic composition.

— STYLE REFERENCE —
Cinematic framing of "calm walk to defeated victim" scenes — think 
Last of Us boss-defeated approach moments, John Wick's measured walk 
to a target, color grading of Black Myth: Wukong sunset cutscenes, 
environmental scale of Korean historical wuxia films. Photo-realistic 
3D cutscene quality, NOT anime, NOT illustration.

— TECHNICAL —
16:9, 1920x1080 minimum, HDR-graded with strong warm sunset color 
grade. Cinematic shallow depth-of-field — Gildong's back is tack-
sharp, fallen bandit is slightly softer (creating focal pull effect 
guiding viewer's eye), distant mountain background soft. Strong rim-
light on Gildong from the sun behind the mountains. Light cinematic 
film grain.

Lower 1/4 of frame is intentionally LEFT EMPTY/CLEAN (ground/horizon 
area) for potential subtitle overlay during post-production.

— TONE —
QUIET TENSE PRELUDE. The calm before the storm. The hunter approaching 
the wounded prey. Pure tension, no comedy yet — the comedy comes 
LATER in the sequence (M2-B Glock reveal). The viewer should feel 
"this is the start of something significant" but not yet know what.
```

#### 📋 딥 인터뷰 확정 사항 (변경 시 위 영문 프롬프트도 업데이트)

| 차원 | 결정값 |
|------|------|
| 카메라 | OTS Medium back shot (길동 뒷모습 50%) |
| 거리 | 길동·적 3~5m |
| 길동 동작 | 걷는 중 (mid-stride, 한 발 내딛음) |
| 길동 봉 | 오른손 지팡이처럼, butt 땅 |
| 적 자세 | 옆으로 누움 (입 보임) |
| 일몰 | 늦은 황금 시간 (해 거의 짐) |
| 분위기 | 정적 긴장감 (폭풍 직전) |
| 화면 | 16:9, 하단 1/4 자막 영역 비움 |
| HUD | 없음 (시네마틱 컷씬) |

---

### 📸 M2-B. 중간 프레임 (Beat 4 — 글록 17 Reveal)

#### 🇰🇷 한국어 요약
- **장면**: 적이 막 쓰러진 직후. 길동 우측 1/3, 적은 좌측에 limp하게 누워 있음. 길동의 RIGHT HAND가 머리 높이로 들려 있고, 손에 **모던 블랙 글록 17 권총** — 총구에서 가는 연기 한 줄기. 길동 표정 = 차분·살짝 흡족.
- **시점**: 미디엄 shot, 길동 reveal에 포커스
- **분위기**: ⚡ 시대 충돌 코믹 핵심 — 조선시대 무사 + 글록
- **저장**: `자료/M3_절벽_글록.png`

#### 🇬🇧 영문 프롬프트

```
A cinematic in-game cutscene frame from a Korean wuxia action RPG with
a deliberate anachronistic comedic punchline. 16:9, photo-realistic 3D.

— CAMERA —
MEDIUM SHOT focused on @길동, with the fallen bandit visible in lower-
left of frame. Camera positioned at chest level, slight upward angle
on @길동 to make him heroic. Cliff backdrop visible.

— SUBJECT (Gildong) —
@길동 (match attached character reference EXACTLY) standing TALL and
unmoving on the cliff edge. Weight relaxed, shoulders confident.
His LEFT hand still holds the wooden staff loosely (butt on ground).

⚠️ KEY ELEMENT — RIGHT HAND raised to head/face level, holding a
MODERN BLACK GLOCK 17 PISTOL. The pistol is pointed slightly outward
(having just been fired). A THIN WISP OF SMOKE rises gently from the
muzzle. Gildong's face is calm, deadpan, with a barely perceptible
smug half-smile.

⚠️ ABOUT THE PISTOL — CRITICAL: This MUST be a recognizable
CONTEMPORARY MODERN GLOCK 17 — black polymer frame, modern boxy slide
profile, no hammer visible. NOT a flintlock, NOT a matchlock, NOT a
period-appropriate firearm, NOT a fantasy weapon. The visual
contradiction between Joseon-era warrior and modern Glock IS the
entire comedic punchline.

— SUBJECT (Defeated Bandit) —
The bandit lies LIMP on the rocky ground in lower-left of frame,
just having fallen backward from a headshot. A SMALL DARK SPOT on
his forehead (NOT spraying blood, NOT gore — just a small visible
mark, comedic-stylized). Eyes closed or slightly open and blank.
Body relaxed, defeated. His broken weapon scattered beside him.

⚠️ NO blood pool, NO gore, NO graphic violence. The death is
comedic-stylized like an old-school spaghetti western — just a fall
and a small mark.

— ENVIRONMENT —
Same cliff edge, sunset golden hour. Wind still blowing dust, hair,
and robes. Mountain backdrop. Drama clouds. Same continuity as
previous frame.

— ACTION & VFX —
- THIN WISP OF SMOKE rising from the Glock barrel (the visual hero
  element of this frame)
- Wind particles continuing
- Slight haze around the moment of the gunshot dissipating

NO muzzle flash explosion, NO dramatic gun light, NO speed lines.
Subtle, understated — the smoke wisp alone tells the story.

— EXPRESSION & STANCE —
@길동: calm, deadpan, slightly smug. Eyes looking forward (NOT at the
gun, NOT at the bandit). Like he just did a routine task. The casual
deadpan IS the joke.
Bandit: completely limp, fallen.

— COLOR PALETTE —
Same sunset palette. The Glock is matte black (#1A1410) standing out
against the warm sunset tones — visual contrast emphasizes the
anachronism.

— UI / HUD —
NO HUD — cinematic cutscene continues.

— STYLE REFERENCE —
Cinematic framing of action film "gun reveal" moments — think classic
westerns, John Wick cool composure, but in a Joseon-era setting.
Photo-realistic 3D, cinematic color grading.

— TECHNICAL —
16:9, 1920x1080, HDR-graded. The GLOCK + SMOKE WISP must be the
clear focal point — viewer's eye drawn immediately to it. @길동's
face secondary focus. Bandit and background tertiary. Cinematic
shallow depth-of-field.

— TONE —
This is the COMEDIC PUNCHLINE FRAME. Setup (M2-A tension) → punchline
(M2-B Glock reveal). The contrast between serious wuxia composition
+ modern Glock = the entire comedy. @길동's deadpan delivery makes
it funnier — he treats it as if it's the most natural thing.
```

---

### 📸 M2-C. 끝 프레임 (Beat 6 — 대사 컷 / Hero Shot)

#### 🇰🇷 한국어 요약
- **장면**: 길동 단독 hero shot. 글록은 옆구리 쪽으로 내린 상태. 죽은 적은 하단 frame에 보임. 일몰 바람에 머리·도포 휘날림. 차분한 deadpan 표정. 하단 1/4은 자막 영역으로 비워둠.
- **시점**: 로우앵글 hero shot
- **분위기**: 영웅적 마무리 + 자막 들어갈 자리
- **저장**: `자료/M4_절벽_연기후.png`

#### 🇬🇧 영문 프롬프트

```
A cinematic in-game cutscene final HERO SHOT from a Korean wuxia
action RPG. 16:9, photo-realistic 3D render with dramatic cinematic
composition. This frame will have a subtitle overlay.

— CAMERA —
LOW-ANGLE HERO SHOT. Camera positioned at waist/hip level, looking
up at @길동 to make him heroic and larger-than-life. @길동 framed
in the CENTER-RIGHT of the upper two-thirds. The fallen bandit
visible in the lower-left foreground (slightly out of focus). Lower
1/4 of frame intentionally LEFT EMPTY for subtitle overlay.

— SUBJECT (Gildong) —
@길동 (match attached character reference EXACTLY) standing
gracefully on the cliff edge, body angled slightly toward camera but
mostly facing the sunset (3/4 view). Weight on one leg, contrapposto
pose. Hair and robe billowing strongly in the cliff wind, dramatic
silhouette.

His RIGHT hand holds the BLACK GLOCK 17 LOWERED to his side —
relaxed, no longer aiming. The pistol still has the faintest trace
of smoke. His LEFT hand holds the wooden staff loosely (butt on
ground). His face shows calm, contemplative, quietly satisfied
expression — looking out at the sunset horizon, not at the camera.

⚠️ The Glock 17 must remain a recognizable modern pistol, lowered
naturally at his side. Black, modern, anachronistic.

— SUBJECT (Defeated Bandit) —
The fallen bandit visible in lower-left foreground, lying limp on
rocky ground, slightly out of focus. Same continuity as previous
frame.

— ENVIRONMENT —
Same cliff edge sunset. Vast mountain ranges silhouetted in
background. Strong sunset back-lighting creating powerful rim-light
on @길동. Wind blowing intensely. Distant clouds dramatic.

— ACTION & VFX —
- Strong wind particles (dust, leaves) flowing through frame
- Hair and robes flowing dramatically
- Sun god-rays piercing the cliff atmosphere
- Faint smoke trace from Glock barrel
- Rim-light highlighting @길동's silhouette

— EXPRESSION & STANCE —
@길동: peaceful, reflective, quietly heroic. The aftermath calm of
a victorious warrior. Subtle deadpan satisfaction (knowing he just
did something absurd with the Glock). Eyes on the horizon.

— COLOR PALETTE —
Strong sunset palette — orange, warm gold, ocher with deep
silhouette tones for mountains. @길동's silhouette partially in
shadow with rim-light. Cinematic warm grade.

— UI / HUD —
NO HUD. The lower 1/4 of frame is intentionally LEFT EMPTY (clean
ground/horizon area) for adding a subtitle text overlay during
post-production editing.

— STYLE REFERENCE —
Final hero shot composition like the closing frame of a samurai film
chapter. Think Sergio Leone western showdown endings, John Woo hero
silhouettes, classic Korean historical drama final shots. Photo-
realistic 3D, cinematic color grading, dramatic backlighting.

— TECHNICAL —
16:9, 1920x1080+, HDR-graded. Cinematic depth-of-field with @길동
sharp, bandit and mountains soft. Strong sun rim-light. Lower 1/4
of frame visually empty for subtitle. Cinematic film grain.

— TONE —
Heroic resolution. The aftermath calm. Tension resolved into quiet
satisfaction. The Glock at his side reminds the viewer of the
absurdity, but @길동's stoic posture restores the wuxia gravitas.
Tension 70% + comedic undertone 30% — perfectly balanced final
moment for a player to absorb the dialogue subtitle.
```

---

### 💡 M2 시퀀스 사용 팁

#### 첨부 자료 (3장 모두 공통)
1. **길동 캐릭터 원화** (single source of truth — 외형 일관성)
2. *(M2-B·M2-C)* 글록 17 reference 이미지 (가능하면 — 무기 정확도 ↑)
3. *(선택)* 이전 M2 이미지를 다음 호출 시 첨부 → 환경·라이팅 일관성 ↑

#### 일관성 유지 팁
- 3장 모두 **같은 ChatGPT 세션**에서 연속 생성 (스타일 일관성 ↑)
- M2-A 결과 마음에 들면 → "Same setting, same lighting, same character" 한 줄 추가하여 M2-B·C 호출
- 환경 묘사 줄여서 호출 (이전 컨텍스트 활용)

#### 결과 저장 + 영상화 흐름
```
1. M2-A·B·C 3장 생성 → 자료/M2_절벽_대치.png 등 저장
2. Kling AI Omni 모드 진입
3. 3장 모두 reference 이미지로 업로드
4. 스마트 멀티 샷 ON
5. 영상 프롬프트 ([[기획/6_분위기/분위기_영상프롬프트]] 참조) 입력
6. 720p OFF 10초 생성 (60 크레딧)
```

---

## 💡 사용 팁

### 첨부 자료 (필수)
1. **캐릭터 원화** (single source of truth — 외형)
2. **🆕 "지면 내리꽂기" 슬로우모 프레임 스크린샷** (필수 — 카메라 POV·HUD·환경 매칭용)
   - 셀카 프레임은 이 reference의 "다음 프레임" 컨셉 → 동일 POV·동일 HUD 유지
   - 첨부 시 한 줄 추가: `Match the camera angle, framing, and HUD layout of the attached gameplay screenshot exactly. This selfie pose is the NEXT FRAME of that slam-down moment.`
3. *(선택)* 공중 QTE C6 지면 내리꽂기 결과 이미지 — 환경 / 적 자세 추가 참고

### 첫 결과 안 맞을 때 추가 지시
| 증상 | 추가 한 줄 |
|------|----------|
| **장승(Jangseung totem)이 등장** (S3 reference 오염) | `ABSOLUTELY NO Korean Jangseung totem poles, NO wooden carved totems, NO summoned spirits anywhere in the frame` |
| **다른 적·잡몹 다수 등장** | `EXACTLY ONE enemy — the single impaled bandit in the crater. NO standing bandits, NO crowd, NO additional enemies` |
| **별 이펙트·황금 사운드 웨이브 등장** | `NO yellow stun-stars around any character, NO golden energy waves, NO sound-wave ripples` |
| **카메라가 시네마틱 와이드 컷어웨이로 변환** (게임 POV 이탈) | `STANDARD third-person over-the-shoulder gameplay POV matching the attached slam-down screenshot, NOT a cinematic cutaway, NOT a wide cinematic shot` |
| **손 anatomy 깨짐** (손가락 추가·왜곡·녹음) | `anatomically correct hands, EXACTLY five fingers per hand, natural selfie grip on the smartphone (thumb on one side + four fingers wrapping the back), NO extra fingers, NO distorted hands` |
| **HUD가 reference와 다름** (요소 누락·추가) | `HUD layout MUST match the attached reference screenshot exactly — top-left portrait + 3 bars, bottom-left inventory 4·5, bottom-center boss HP "산적 두목", bottom-right doulsul slots [1][2][3] + Shift` |
| **길동 얼굴이 폰 화면만 봄** (4th-wall break ❌) | `Gildong's HEAD is turned BACK toward the over-the-shoulder player camera, looking DIRECTLY at the viewer with a subtle smug half-smile — a deliberate META 4TH-WALL BREAK while continuing the selfie pose mechanically` |
| **길동이 땅에 서 있음** (공중 부양 X) | `Gildong MUST be SUSPENDED IN MID-AIR ~3-4 meters above the crater, NOT standing on the ground, NOT on the crater rim` |
| 스마트폰이 시대물 객체 (거울·두루마리 등)로 변환 | `the smartphone MUST be a modern contemporary black smartphone, NOT a bronze mirror, scroll, or any period-appropriate Joseon object` |
| 길동 표정이 과장되어 윙크·웃음으로 변환 | `expression should be a subtle smug deadpan half-smile, NOT a wide smile or wink — almost imperceptible` |
| 적이 피·고어로 사실적 죽음 표현 | `the impaled enemy is comedic-stylized slapstick, NO blood, NO gore — only legs visible protruding upward` |
| 일본·중국풍 환경으로 변환 | `Korean Joseon-era setting only, NOT Japanese, NOT Chinese` |
| 카메라가 정면 컷으로 변환 | `3/4 back angle from player POV, NEVER full front-on, NEVER pure side profile` |
| 액션 모션 과장 (DMC·Bayonetta식 포즈) | `Gildong's pose is relaxed and grounded, NOT a heroic pose, NOT exaggerated` |

### 결과 저장
- `notes/ProjectKD/기획/6_분위기/자료/M_공중QTE_셀카.png`
- 옵시디언 본문 [[기획/6_분위기/6_분위기]] "추구하는 병맛" 단락 안에 `![[자료/M_공중QTE_셀카.png]]`로 임베드.

---

> 📅 최종 수정: 2026-05-14
