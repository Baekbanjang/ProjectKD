---
title: 도술 GPT 이미지 생성 프롬프트
tags:
  - ProjectKD
  - 기획
  - 도술
  - GPT
  - prompt
created: 2026-05-14
---

# 📷 도술 GPT 이미지 생성 프롬프트

> 화부술·풍보·정승 스턴 컨셉 이미지 생성용 영문 프롬프트.
> 결과 저장 경로: `notes/ProjectKD/기획/1_도술/자료/S1_화부술.png` 등
> 본문: [[기획/1_도술/1_도술]]

---

## 🎨 공통 컨텍스트 (3종 도술 공유)

### 캐릭터 (필수 — 원화 첨부)
- **🎨 GPT 호출 시 매번 캐릭터 원화 첨부**. 원화 = 외형의 단일 진실 (single source of truth).
- 영문 프롬프트에 캐릭터 외형이 명시되어 있어도 **원화 우선 적용**.
- 원화 없이 호출하면 외형 매번 달라짐 → 일관성 깨짐.

### 정승 원화 (S3 전용 — 필수)
- **🪨 S3 정승 스턴 호출 시 정승(장승) 원화 추가 첨부**.
- 정승 모양·재질·표정·비율 모두 원화 기준. 영문 묘사보다 원화 우선.
- 캐릭터 원화 + 정승 원화 = **두 장 같이 첨부** (S3에서).

### 환경
- 조선 산골 도적촌. 황혼 골든아워 (S2는 새벽/흐림). 흙길·이끼낀 돌계단·소나무·짚지붕.

### 컬러 팔레트
- 먹 `#1A1410` · 황토 `#C07D3A` · 이끼 `#4A6741` · 솔잎 `#2D4A2A` · 황금 `#E8C547` · 한지 흰 `#F0EAD6`
- ❌ 핏빛 빨강 / 네온 / 채도 높은 컬러 금지

### 톤
- 묵직·진지한 동양 무협. Stellar Blade 시네마틱 + 검은 신화 환경 디테일 + Sekiro 색감.

### 📷 카메라 정책 (4 컷 공통 — 필수)
- **3인칭 어깨너머 (Third-person Over-the-Shoulder)** 게임 카메라.
- 카메라 위치: 길동 **뒤 2~3m, 어깨 높이**. 길동의 **등·뒷모습**이 화면 앞쪽 좌측 1/3에 위치.
- 카메라가 길동 너머로 적/액션을 향함. **길동 얼굴이 카메라 정면을 보지 않음**.
- ❌ 정면 컷·측면 컷·시네마틱 자유 카메라 금지. 인게임 플레이 시점 통일.

---

## 🎴 S1. 화부술 (火符術)

### 🇰🇷 한국어 요약 (이해용)
- **장면**: 길동이 적 발 밑에 황금 부적 다발 3~5장 흩뿌림 → 동시에 작은 불기둥들이 솟구침
- **포즈**: 길동 무표정, 한 손은 봉 잡고 다른 손이 부적 던지는 동작 (정지 컷)
- **배경**: 도적촌 마당, 적 1~3명이 불기둥에 휘말림
- **분위기**: 황혼 + 불 빛 + 흙먼지
- **시각**: 황금 부적이 공중에 흩뿌려진 채 빛남, 불기둥은 황토색+황금색 (붉은 핏빛 X)

### 🇬🇧 영문 프롬프트

```
A cinematic in-game screenshot from a Korean martial arts action RPG demo
"Black Gildong" (흑길동). 16:9 aspect ratio, photo-realistic 3D render quality.

— CAMERA (CRITICAL) —
Third-person OVER-THE-SHOULDER game camera, positioned 2~3 meters BEHIND
Gildong at roughly shoulder height. We see Gildong from BEHIND — his back,
shoulders, and the back of his head/hat are in the FOREGROUND on the left
third of the frame. Camera looks PAST him toward the enemy in the
mid-ground. This is the IN-GAME PLAYER CAMERA, NOT a cinematic front-on
shot. NEVER show Gildong's face directly to the camera. NO front-angle,
NO side-profile-only shot.

— SUBJECT —
The protagonist (Gildong) — match his appearance EXACTLY to the attached
character reference artwork. The reference image is the SINGLE SOURCE OF
TRUTH for his face, hairstyle, body type, outfit, weapon design, and
overall visual style. Do NOT invent new features. Keep him fully
consistent with the reference.

In this scene, viewed from behind, he holds his long wooden martial-arts
staff (봉) in his left hand. His right hand is extended forward past his
body in a casting gesture, mid-throw, with golden paper talismans (부적) —
three to five rectangular slips inscribed with faint calligraphic marks —
flying out from his palm toward the enemies ahead.

— ACTION & VFX —
Multiple small fire pillars erupt from the ground around two or three bandit
enemies in the mid-ground, ~6 meters ahead. The talismans are mid-air,
glowing with soft golden light. The fire pillars are ocher-and-gold (NEVER
blood-red, NEVER pure orange neon) with dust and embers swirling. Embers
arc outward in slow-motion. Light heat haze distortion around the pillars.

— ENVIRONMENT —
Joseon-era mountain bandit village courtyard at golden-hour dusk. Weathered
wooden palisade walls, straw-thatched rooftops, mossy stone stairs, distant
pine trees. Soft volumetric sunset light from camera-left. Atmospheric dust
particles and fallen pine needles.

— EXPRESSION & STANCE —
Gildong's face is calm, stoic, unreadable — not surprised, not angry, just
focused. His body is slightly turned, staff grounded. The enemies in the
mid-ground are caught in the eruption — one staggering back, another
shielding his face.

— COLOR PALETTE —
Ink black (#1A1410), ocher (#C07D3A), moss green (#4A6741), pine dark
(#2D4A2A), accent gold (#E8C547), hanji-paper white (#F0EAD6). ABSOLUTELY
NO bright red blood, NO neon, NO saturated cartoon colors.

— UI / HUD (subtle, lower 20% of screen) —
Bottom-left: three slim horizontal bars (HP ocher / Stamina moss / Doul pine).
The Doul bar is partially depleted (about 50%), indicating the skill was
just cast. Bottom-right: three small circular doulsul slots, the leftmost
("1") is darkened with a thin gold outline cooldown ring filling clockwise
(Black Myth: Wukong cooldown style). Small white numerals 1·2·3 on slot
corners. No HP numbers, no minimap, no portrait.

— STYLE REFERENCE —
Cinematic framing of Stellar Blade, environmental texture density of Black
Myth: Wukong, color discipline of Sekiro: Shadows Die Twice, with a
restrained Korean wuxia overlay. Photo-realistic, NOT anime, NOT illustration.

— TECHNICAL —
16:9, 1920x1080 minimum, HDR-graded, light film grain, subject in
rule-of-thirds left position, slight motion blur on the talismans only.
```

---

## 🌬 S2. 풍보 (風步) — 2장 컷 (5단계 시퀀스 중 ③·④)

> 풍보 시퀀스 ⑤단계 중 **③ 공중 정지 + ④ 돌진** 두 모먼트를 1장씩 그림.
> 저장: `자료/S2_풍보_정지.png` + `자료/S2_풍보_돌진.png`

---

### 🎬 S2-A. 풍보 — 공중 정지 컷 (단계 ③ 시그니처)

#### 🇰🇷 한국어 요약
- **장면**: 길동이 3~4m 점프 후 **공중 정점에서 1프레임 정지**. 시간이 멈춘 듯한 시그니처 모먼트.
- **포즈**: 봉을 한 손에 쥔 채 공중에 떠 있음. 한 발은 살짝 앞으로, 한 발은 뒤. 머리·시선은 락온된 적 쪽.
- **배경**: 도적촌 골목, 적 1명이 멀리 보임 (락온 대상). 시간 멈춘 듯 낙엽도 정지.
- **시각**: 발밑 황금 도력 회오리 (아직 응축 상태), 옷자락 펄럭이려다 멈춘 듯, 카메라 약 줌인, 슬로우모.

#### 🇬🇧 영문 프롬프트

```
A cinematic in-game screenshot from a Korean martial arts action RPG demo
"Black Gildong". 16:9 aspect ratio, photo-realistic 3D render. Captures
a SINGLE FROZEN MOMENT — like a time-stop bullet-time frame.

— CAMERA (CRITICAL) —
Third-person OVER-THE-SHOULDER game camera, positioned 2~3 meters BEHIND
Gildong at roughly shoulder height (or slightly lower for low-angle
drama). We see Gildong from BEHIND — his back, shoulders, and the back
of his head are in the FOREGROUND on the left third of the frame. Camera
looks PAST him toward the locked-on enemy in the mid-ground. This is the
IN-GAME PLAYER CAMERA, NOT a cinematic front-on shot. NEVER show
Gildong's face directly to the camera. NO front-angle, NO side-profile-only.

— SUBJECT —
The protagonist (Gildong) — match his appearance EXACTLY to the attached
character reference artwork. The reference image is the SINGLE SOURCE OF
TRUTH for his face, hairstyle, body type, outfit, weapon design, and
overall visual style. Do NOT invent new features. Keep him fully
consistent with the reference.

In this scene, viewed from behind, Gildong is FROZEN MID-AIR at the peak
of a 3~4 meter vertical jump, completely suspended like time has stopped.
He holds his long wooden staff (봉) in one hand at his side. Body upright
but slightly coiled — one leg slightly forward, the other trailing. His
head is turned slightly toward the locked-on enemy ahead — we see the
back of his head and a sliver of jawline, NOT his full face. Robe and hair
fabric are mid-flutter but unnaturally still, as if paused in time.

— ACTION & VFX —
A condensed swirling golden Doul (도력) vortex at his feet, made of
slow-spiraling dust, dry pine needles, and golden spirit motes — particles
are visible but appear nearly frozen in space (long exposure / time-stop
look). NO motion blur on the body. NO afterimage. NO forward thrust yet —
this is the calm BEFORE the dash. Air shimmers with faint heat-like
distortion to convey time-stop.

— ENVIRONMENT —
Joseon-era mountain bandit village courtyard or narrow stone path.
Weathered wooden palisades, mossy boulders, distant pine trees. A single
bandit enemy stands 8~10 meters away in the mid-ground, weapon drawn,
mid-stride — also frozen in time. Falling pine needles, dust motes, and
loose hanji paper scraps hang suspended in the air around him.
Atmospheric mist or overcast dusk lighting — cooler, moodier than
golden-hour, with subtle volumetric god-rays.

— EXPRESSION & STANCE —
Gildong's expression is utterly calm, eyes laser-focused on the target.
A faint subtle confidence. NOT scared, NOT angry — a master picking his
landing spot.

— COLOR PALETTE —
Ink black (#1A1410), ocher (#C07D3A), moss green (#4A6741), pine dark
(#2D4A2A), accent gold (#E8C547) ONLY on the spirit motes and a faint
gold lock-on reticle around the distant enemy, hanji-paper white
(#F0EAD6), mist gray (#9CA8A0). NO red, NO neon.

— UI / HUD (subtle) —
Bottom-left: three slim horizontal bars (HP / Stamina / Doul). Doul is
about 75%. Bottom-right: three circular doulsul slots, the middle one
("2") is darkened with a thin gold cooldown ring just starting to fill.
Small numerals 1·2·3 on slot corners. A FAINT GOLD LOCK-ON RETICLE
hovers around the distant enemy (small thin gold square or ring).
No HP numbers, no minimap, no portrait.

— STYLE REFERENCE —
Cinematic time-stop framing reminiscent of Black Myth: Wukong's
Immobilize spell moments or Stellar Blade bullet-time shots. Environmental
density of Black Myth: Wukong, color discipline of Sekiro. Photo-realistic
3D, NOT anime.

— TECHNICAL —
16:9, 1920x1080+, HDR-graded desaturated mood. Slight low-angle to
emphasize Gildong's vertical suspension. Strong depth-of-field with
Gildong tack-sharp, background slightly soft. ABSOLUTELY NO motion blur
on the subject — this is a frozen instant. Suspended particles in
mid-air. Heroic, calm, predatory.
```

---

### 🎬 S2-B. 풍보 — 돌진 컷 (단계 ④ 액션)

#### 🇰🇷 한국어 요약
- **장면**: 공중 정지 직후, 길동이 락온된 적 쪽으로 **빠른 활공 5~8m 돌진** 중. 단계 ③ 다음 컷.
- **포즈**: 봉을 옆구리에 끼고 살짝 앞으로 기운 자세 (활공 중), 다리 뒤로 뻗음.
- **배경**: 도적촌 좁은 골목·돌계단을 가로질러 활공. 적 1명 시야 안.
- **시각**: 발밑 소용돌이 잔재, 길동 잔상 1개, 옷자락·낙엽·흙먼지 뒤로 흩날림, 강한 모션 블러.

#### 🇬🇧 영문 프롬프트

```
A cinematic in-game screenshot from a Korean martial arts action RPG demo
"Black Gildong". 16:9 aspect ratio, photo-realistic 3D render. Captures
mid-flight glide / dash.

— CAMERA (CRITICAL) —
Third-person OVER-THE-SHOULDER game camera, positioned 2~3 meters BEHIND
Gildong at roughly shoulder height. We see Gildong from BEHIND as he
glides forward — his back, shoulders, trailing robe, and the back of his
head fill the FOREGROUND on the left third of the frame. Camera follows
him from behind, looking PAST him toward the enemy ahead in the mid-ground.
This is the IN-GAME PLAYER CAMERA, NOT a cinematic side-profile or
front-angle shot. NEVER show Gildong's face directly to the camera.

— SUBJECT —
The protagonist (Gildong) — match his appearance EXACTLY to the attached
character reference artwork. The reference image is the SINGLE SOURCE OF
TRUTH for his face, hairstyle, body type, outfit, weapon design, and
overall visual style. Do NOT invent new features. Keep him fully
consistent with the reference.

In this scene, viewed from behind, he holds his long wooden staff (봉)
tucked under his right arm. He is mid-air, gliding forward FAST in a
low arc roughly 1.5 meters above a stone path, body strongly leaning
forward, legs trailing back, his robe trailing dramatically in the wind.
STRONG FORWARD MOTION away from the camera, toward the enemy ahead.

— ACTION & VFX —
A trailing swirl of dust, dry pine needles, and faint golden Doul (도력)
particles streaming behind him. A SINGLE faint afterimage silhouette
ghosts behind him — only ONE, NOT multiple clones. Strong motion blur
on his body and the trailing particles. The air shimmers with light
displacement and gold spirit motes. NO lightning, NO fire, NO neon — just
wind, dust, gold motes.

— ENVIRONMENT —
Joseon-era mountain bandit village. A narrow stone-paved path between
weathered wooden palisades and straw-thatched rooftops. Mossy boulders
flank the path. A single bandit enemy stands 5 meters ahead, weapon
drawn, just turning to face the incoming Gildong with a startled posture.
Atmospheric morning mist or overcast dusk — softer, moodier lighting,
matching S2-A's tone.

— EXPRESSION & STANCE —
Gildong's expression is calm and predatory, eyes locked on the enemy
ahead. Body language is controlled and explosive — a master closing the
gap.

— COLOR PALETTE —
Ink black (#1A1410), ocher (#C07D3A), moss green (#4A6741), pine dark
(#2D4A2A), accent gold (#E8C547) ONLY on the spirit motes, hanji-paper
white (#F0EAD6), mist gray (#9CA8A0). NO red, NO neon.

— UI / HUD (subtle) —
Bottom-left: three slim horizontal bars (HP / Stamina / Doul). Doul is
about 70% (slightly less than S2-A). Bottom-right: three circular
doulsul slots, the middle one ("2") is darkened with a thin gold
cooldown ring filling clockwise. Small numerals 1·2·3 on slot corners.
No HP numbers, no minimap, no portrait.

— STYLE REFERENCE —
Cinematic framing of Stellar Blade, environmental density of Black Myth:
Wukong, color discipline of Sekiro. Photo-realistic, NOT anime.

— TECHNICAL —
16:9, 1920x1080+, HDR-graded desaturated mood (cooler than S1's
golden-hour), light film grain, subject in rule-of-thirds center
position. STRONG motion blur on the body and trailing particles, sharper
on the face. Strong sense of forward velocity. Consistency note: this
shot pairs with S2-A (same Gildong, same environment, same lighting,
same outfit) — only the motion phase differs.
```

---

## 🪨 S3. 정승 스턴

### 🇰🇷 한국어 요약
- **장면**: 길동 양옆 땅에서 거대한 정승(장승) 2기가 솟구쳐 입을 벌리고 전방에 황금 음파 포효
- **포즈**: 길동은 가운데 서서 봉을 땅에 짚고 무표정. 정승은 양옆에서 위협적
- **배경**: 도적촌 마당. 적 2~3명이 부채꼴 음파에 휘말려 경직·뒤로 밀림
- **시각**: 정승은 갈색·이끼낀 나무 조각상 (전통 한국 장승), 입에서 황금 동심원 음파 발사 + 흙먼지 폭발

### 🇬🇧 영문 프롬프트

```
A cinematic in-game screenshot from a Korean martial arts action RPG demo
"Black Gildong". 16:9 aspect ratio, photo-realistic 3D render. Dramatic
ceremonial moment.

— CAMERA (CRITICAL) —
Third-person OVER-THE-SHOULDER game camera, positioned 2~3 meters BEHIND
Gildong at roughly shoulder height. We see Gildong from BEHIND — his back,
shoulders, and the back of his head are in the FOREGROUND, slightly
LEFT-OF-CENTER (he stands roughly centered relative to the frame's
horizontal axis, but the camera angles past him on his right side).
Camera looks PAST him forward toward the enemies in the mid-ground.
This is the IN-GAME PLAYER CAMERA, NOT a cinematic front-on shot.
NEVER show Gildong's face directly to the camera. NO front-angle.

— SUBJECT —
The protagonist (Gildong) — match his appearance EXACTLY to the attached
character reference artwork. The reference image is the SINGLE SOURCE OF
TRUTH for his face, hairstyle, body type, outfit, weapon design, and
overall visual style. Do NOT invent new features. Keep him fully
consistent with the reference.

In this scene, viewed from behind, he holds his long wooden staff (봉)
planted vertically on the ground beside him, both hands gripping it.
He stands stoic, facing AWAY from the camera (toward the enemies),
completely still — the calm eye of the storm. We see his back, the back
of his head, and a sliver of his shoulder/staff in the foreground.

— DOUL SUMMONS — Korean Jangseung Totems —
Two massive Korean wooden totem poles (장승 / Jangseung) erupt from the
ground on either side of Gildong, framing him symmetrically.

⚠️ JANGSEUNG APPEARANCE — REQUIRED: Match the totems EXACTLY to the
attached Jangseung reference image. The reference is the single source
of truth for the totem's face carving, proportions, color, material,
expression, and silhouette. Follow the reference visually for all
design details. Do NOT default to Japanese Oni or Chinese Fu Dog motifs.

Both totems are mid-emergence — soil, dust, and grass tufts erupt around
their bases from breaking through the ground. They lean slightly forward,
mouths open wide toward the enemies. Moss and lichen on lower portions.

— VFX — INVISIBLE SHIMMERING SOUND WAVES —
From the open mouths of both totems, TRANSPARENT sound waves blast
forward in a wide ~110-degree cone toward the enemies, ~10 meters ahead.
The waves are NEARLY INVISIBLE — rendered as HEAT-HAZE SHIMMER and AIR
DISTORTION ripples (like the visible distortion above a hot road, or
sonar/shockwave displacement). Think: the air itself rippling and
warping in concentric waves, NOT a glowing energy beam.

❌ NO golden color, NO bright sound-ring outline, NO neon glow, NO solid
visible wave — the only thing visible is the WARPING/REFRACTION of the
background through the wave (atmospheric distortion).

✅ The waves are detectable ONLY by:
- Air distortion / heat shimmer ripple
- Concentric refraction of the background scenery (warps the trees,
  enemies, buildings behind the wave)
- Dust, dry leaves, pine needles, and small debris BLOWN FORWARD by
  the wave (these are visible particles, the wave itself is not)
- Faint subtle white/translucent ripple lines at the leading edges
  (minimal, almost invisible)

The cone shape should be perceivable purely through these distortion
artifacts. NO lightning, NO fire, NO glowing energy.

— ENEMIES —
Two or three bandit enemies in the mid-ground are caught in the cone,
staggering back, weapons dropped, hands clutching their ears, faces
contorted in shock — clearly stunned. Small yellow stun-stars or daze
particles spiral around their heads (a subtle game-y indicator).

— ENVIRONMENT —
Joseon-era mountain bandit village courtyard. Open dirt ground with stone
borders, scattered straw bundles, weathered wooden buildings flanking the
sides. Late afternoon golden light from above, warm but dust-filtered.
Atmospheric haze.

— EXPRESSION & STANCE —
Gildong's face is completely impassive — utterly calm despite the chaos.
His stoicism contrasts with the violent eruption of the totems on either
side. He has not moved.

— COLOR PALETTE —
Ink black (#1A1410), ocher (#C07D3A), moss green (#4A6741), pine dark
(#2D4A2A), accent gold (#E8C547) ONLY on subtle totem rim highlights and
the HUD elements (NOT on the sound waves — waves are TRANSPARENT/INVISIBLE),
hanji-paper white (#F0EAD6), traditional Jangseung accent colors used
SPARINGLY only on totem face details per the reference image.
NO neon, NO blood, NO saturated cartoon colors, NO golden energy waves.

— UI / HUD (subtle) —
Bottom-left: three slim horizontal bars (HP / Stamina / Doul). The Doul bar
is heavily depleted (~25%), indicating the skill was just cast (highest
cost ability). Bottom-right: three circular doulsul slots, the rightmost
("3") is darkened with a thin gold cooldown ring filling clockwise. Small
numerals 1·2·3 on slot corners. No HP numbers, no minimap, no portrait.

— STYLE REFERENCE —
Cinematic framing of Stellar Blade boss-moment shots, environmental detail
of Black Myth: Wukong, color discipline of Sekiro. Photo-realistic 3D,
NOT anime, NOT illustration. Strong symmetrical composition emphasizing
ceremonial Korean folk religion aesthetic.

— TECHNICAL —
16:9, 1920x1080+, HDR-graded, dramatic centered composition (Gildong
exactly center, totems mirrored left/right at rule-of-thirds vertical
lines), slight film grain, motion blur on the dust eruption but sharp on
the totems and Gildong. Heroic ceremonial mood.
```

---

## 💡 사용 팁

### 🎨 캐릭터 원화 첨부 (필수)
1. **매 이미지 생성마다 캐릭터 원화 첨부**. 한 번만 첨부하고 다른 컷 만들면 외형 달라짐.
2. GPT 대화창에 첨부 + 위 영문 프롬프트 붙여넣기. 두 input 같이 줘야 함.
3. 첫 결과에서 캐릭터 어긋나면:
   - **"refer strictly to the attached reference image for the protagonist"** 한 줄 추가
   - 또는 **"keep the protagonist identical to the reference — face, hair, outfit, weapon"**

### 일반 팁
1. 결과는 `notes/ProjectKD/기획/1_도술/자료/` 폴더에 저장:
   - `S1_화부술.png` / `S2_풍보.png` / `S3_정승스턴.png`
2. **첫 결과 안 맞으면 추가 지시**:
   - "redder fire" 나오면 → "make the fire ocher-gold, not red"
   - 일본·중국풍 나오면 → "Korean Joseon-era, NOT Japanese, NOT Chinese"
   - HUD 어긋나면 → 이전 HUD 이미지 (`M1_StellarBlade식.png` / `M2_Wukong식.png`) 같이 첨부
3. **다중 참조 첨부 추천**:
   - 캐릭터 원화 (필수)
   - HUD 참조 (`M2_Wukong식.png` 권장)
   - → 캐릭터·환경·HUD 일관성 동시 확보

---

> 📅 최종 수정: 2026-05-14
