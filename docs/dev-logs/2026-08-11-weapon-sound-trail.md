# 2026-08-11 — 검 사운드 배선 + 웨폰 트레일 파라미터 TMap화

콤보 20개가 **소리도 궤적도 없던 상태**를 닫았다. 커밋 = 소스 `d60fcc4` `61b4aeb`(push 완료) / Content `66b29e0`(로컬).

---

## 1. 사운드 — 49칸이 비어 있었다

몽타주 20개에 `AnimNotify_PlaySound`가 **49개 꽂혀 있는데 전부 `Sound=None`**이었다. 팩 제작자가 자리만 잡아둔 것.

### 트랙이 답을 갖고 있었다

처음엔 노티 위치를 기하학적으로 분석해 휘두름/타격을 구분하려 했다. 그럴 필요가 없었다 — 노티가 이미 **`Sound_Swing` / `Sound_Hit` 두 개의 이름 붙은 트랙**으로 나뉘어 있었다.

내가 세운 추론 규칙은 49개 중 43개를 맞혔다. **트랙이 정답지였고 추론은 불필요했다.**

### ★ 타격음 채널 이원화 문제

| 채널 | 발동 조건 |
|---|---|
`AnimNotify_PlaySound` | 타임라인 그 지점에서 **무조건** |
`GameplayCue` | **실제로 맞았을 때만** |

`Sound_Hit` 트랙을 채우면 **허공을 갈라도 살 때리는 소리가 난다.**

**승환 결정 = `Sound_Hit` 트랙 제거. 타격음은 DA에만.**

```
Sound_Swing 26칸  → SC_Sword_Swing    무조건 재생
Sound_Hit         → 제거
DA_HitLightAttack → SC_Sword_Hit      GCN_PlayerHitConfirm 경유, 맞췄을 때만
```

### 곁가지 — 총 공격 2개를 사운드가 찾아냈다

`Combo_03_03` `Combo_05_04`만 **`Sound_Swing` 트랙이 비어 있었다.** 총/검 본 속도비 실측에서도 이 둘이 1·2위(2.91 / 1.91)였다. 팩 제작자가 총 공격이라 칼 휘두르는 소리를 안 넣은 것.

→ 여기 들어갈 건 Swing이 아니라 **총성**이다. 발사 프레임 = `03_03` f32 / `05_04` f36.

### 🔴 남은 것 — 톤이 틀렸다

PIE 확인 결과 **소리는 정상으로 난다. 톤이 다르다.** 승환 평 = "빈 철봉 휘두르는 소리".

`Metal_Light_Whoosh` 계열이 속 빈 금속봉처럼 들린다. **고치는 비용은 싸다** — 큐 2개 내용만 갈면 된다. 몽타주 26칸과 DA는 큐를 가리키므로 재작업 없음.

---

## 2. 웨폰 트레일 — 안 보이던 원인

27개 노티가 전부 **`NS_SlashTrail_Distortion_Only_Loop`**를 물고 있었다. 이 계열은 `NMS_Color_Core` 모듈이 없는 **굴절 전용**이라 색이 없다. 배선은 처음부터 정상이었고 에셋 선택이 틀렸던 것.

→ 전 27개를 **`NS_SlashTrail_Basic_Loop`**로 통일.

```
NS               NS_SlashTrail_Basic_Loop
소켓             Sword_Bottom / 무기 태그 Sword
회전·위치 보정    0
Trail Width      200
Lifetime_Trail   0.12
```

### `Trail Width`는 한 방향이다

승환 관찰 — 값을 줄이면 **"칼끝 쪽만 줄어든다"**. 중심에서 양쪽으로 퍼지는 폭이 아니라 **소켓에서 칼끝 방향으로 뻗는 거리**다. 칼날 실측 129 → 200은 1.55배.

`Lifetime_Trail`은 꼬리 길이 노브다. **보이는 길이 = 휘두르는 속도 × Lifetime_Trail.**

### 진단 중 틀린 것 2개 (기록 보존)

1. **"NS 축이 90도 어긋났다"** — 팩 소켓의 `Roll -90`을 근거로 삼았는데, 그건 임의의 `weapon_r` 본 축에 맞춘 값이지 NS 요구사항이 아니었다. 승환이 **전부 0,0,0으로 두니 각도가 맞았다**
2. **"Ribbon FacingMode 차이"** — LightSaber만 각도가 이상한 이유로 세웠으나 승환 확인 결과 둘이 같았다

진짜 혼동 원인은 **근접 판정 디버그 선이 트레일과 겹쳐 보인 것**이었다. GA BP 5개의 `bDrawDebug`를 껐다.

---

## 3. 코드 — 하드코딩 2개를 TMap으로

`ANS_WeaponTrail`이 `SwordLength` / `TrailWidth` **두 이름을 코드에 박고** 있었다. NS를 갈아끼우면 파라미터 이름이 달라서 코드를 고쳐야 했다.

```cpp
// 나이아가라 유저 파라미터 - float / Color / Vector
TMap<FName, float>         FloatParams;
TMap<FName, FLinearColor>  ColorParams;
TMap<FName, FVector>       VectorParams;
```

```cpp
// 노티에 등록된 이름만 전달
for (const TPair<FName, float>& P : FloatParams)
{
    Trail->SetVariableFloat(P.Key, P.Value);
}
```

이제 NS마다 다른 파라미터를 **노티에서 직접 키를 적어** 조절한다. 코드 수정 불필요.

### ★ 공백 하나에 걸렸다

NS의 실제 파라미터 이름은 **`Trail Width`(공백 있음)**인데 코드는 `TrailWidth`였다. `SetVariableFloat`는 정확한 FName 조회라 **못 찾으면 조용히 아무 일도 안 한다.** 에러도 경고도 없다.

기존 노티 26개는 빌드 없이 MCP로 일괄 수정. `SwordLength`는 Basic이 안 받는 죽은 키라 제거.

---

## 4. CLAUDE.md — 주석 문체 명문화

승환 지적 — **"쓰잘데기 없이 너무 길고 이해가 안 돼."**

주석 규칙이 메모리에만 있어 새 세션마다 유실됐다. **§0에 X/O 표로 압축해 박았다**(매 세션 자동 로드되는 위치). 합격본 = `Combat/KDProjectile.h/.cpp`.

핵심 = **명사구로 끝낸다 / 이게 무엇인지만 / 결과·경고 문장은 주석에서 빼고 문서로 / 함수 본문 첫 줄에 `// 기능 : ~`**

---

## 검증

| | |
|---|---|
검 휘두름 소리 | ✅ 26칸 재생 |
타격음 | ✅ 맞췄을 때만 |
트레일 | ✅ 27개 보임 |
빌드 | ✅ 에러 0 |
사운드 톤 | 🔴 재작업 필요 |

## 남은 것

- **검 사운드 톤 교체** — 큐 2개 내용만
- **총성 2칸** (`03_03` f32 / `05_04` f36) — 에셋 조달 후
- **트레일 색** — 현재 빨강. `ColorParams`에 `Color` 넣으면 27개 일괄
- **`AM_SB_Combo_Air_01`** — `ANS_MeleeTrace`가 없어 판정 자체가 없음
- ⚠️ Content 커밋에 `Gun_and_Sword/.../Sword.uasset`이 **내용 미확인으로 딸려 들어갔다**
