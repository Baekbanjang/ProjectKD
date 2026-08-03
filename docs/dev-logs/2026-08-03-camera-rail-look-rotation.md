# 2026-08-03 — 카메라 레일 2단계 완료: 조준 산출을 랙 전 이상 위치로 + 마우스 감도 노출

## 왜 했나

스플라인 돌리 카메라(2단계) 구현 후 **좌우 옆걸음에서만 카메라가 미세하게 스윙**하는 증상이 남아 있었다. 앞뒤 달리기는 멀쩡했다.

조사 결과 이 함수에서 잡은 버그 3개가 **전부 같은 뿌리**였다.

| 시점 | 버그 | 그때 한 조치 |
|---|---|---|
| 07-31 | 짐벌락 (정수리에서 튐) | Yaw 제거 → **오진** → `Size2D()` 가드 |
| 08-02 | 어깨 오프셋 회전 기준 어긋남 | `ArmRight = GetTargetRotation()` |
| 08-03 | **랙이 회전으로 샘** | 본 작업 |

뿌리 = **`UpdateLookRotation`이 월드에서 "실측" 위치로 조준한다**는 설계.

## 원인 — 부모의 출력을 읽었다

Tick 4단계 중 ③이 ②의 **결과**를 읽는 것이 문제였다.

```
① ApplyRailPosition()      마우스 피치 → 레일 진행도 → TargetArmLength | SocketOffset
② Super::TickComponent()   부모가 랙·충돌 먹여서 RelativeSocketLocation 산출
③ UpdateLookRotation()     ← RelativeSocketLocation을 읽는다  ★
④ UpdateChildTransforms()  카메라에 (회전, 위치) 전달
```

랙은 **캐릭터가 화면에서 미끄러져 보이게** 하려고 카메라를 일부러 뒤처지게 만든다.

> 뒤처짐 = 속도 ÷ LagSpeed = 800 ÷ 19 = **42cm**
> 스윙 각 = atan(42 ÷ 382) = **6.3°**

그 뒤처진 자리에서 캐릭터를 겨누면 결과가 뒤집힌다.

- 캐릭터는 화면 중앙에 **고정** — 랙이 무효화
- 대신 **배경 전체가 6.3° 흔들림** — 랙이 "위치 부드러움"에서 "화면 흔들림"으로 변환

앞뒤 이동은 뒤처짐이 팔 길이 방향(X)이라 거리만 변하고 각도가 안 생겨 **증상이 안 보인다.**

## 구현 — `UpdateLookRotation` 교체 (8단계 → 5단계)

각도는 ①이 정한 값에서만 뽑고, 랙은 위치에만 남긴다.

```cpp
void UKDSpringArmComponent::UpdateLookRotation()
{
    if (!DollySpline) { return; }

    // 스프링암 로컬 기준 - 랙 | 충돌이 없을 때의 카메라 자리
    const FVector IdealCam(-TargetArmLength, SocketOffset.Y, SocketOffset.Z);

    // 조준점도 같은 어깨 오프셋만큼 옆으로 - 좌우 성분 상쇄로 요 계산 소멸
    const FVector Aim(0.f, SocketOffset.Y, LookAtHeightOffset);

    const FVector Dir = Aim - IdealCam;   // Y 성분 0 - 순수 상하 기울기

    // 요는 마우스에서 직접 - 랙을 거치지 않아 흔들리지 않음
    const FRotator ArmRot = GetTargetRotation();
    const FRotator WorldLook(FMath::RadiansToDegrees(FMath::Atan2(Dir.Z, Dir.X)), ArmRot.Yaw, 0.f);

    // 월드 회전 -> 컴포넌트 상대 회전
    RelativeLookRotation = GetComponentQuat().Inverse() * WorldLook.Quaternion();
}
```

### 함께 사라진 것

| | 왜 필요 없어졌나 |
|---|---|
| `CompTM` / `UnrotateVector` | 처음부터 로컬 좌표라 월드 왕복 불필요 |
| `OwnerActor` / `GetActorLocation()` | 스프링암 원점이 곧 캐릭터 |
| `ArmRight` (`FRotationMatrix`) | 로컬에서 `SocketOffset.Y`를 양쪽에 넣어 상쇄 |
| 짐벌락 가드 (`LocalDir.Size2D() < 1.f`) | `Dir.Y ≡ 0`이라 `Atan2(Z, X)`가 특이점에서도 안전 |
| `ToOrientationQuat` | 요를 벡터에서 안 뽑음 |

### `Aim`의 Y가 오버숄더의 정체

`Aim`은 **카메라가 겨누는 표적**(캐릭터 몸통)이다. 스프링암 로컬이라 `X = 0`이 곧 캐릭터 자리.

| `Aim.Y` | `Dir.Y` | 결과 |
|---|---|---|
| `0` (캐릭터 정중앙) | −40 | 카메라가 안쪽으로 틂 → 캐릭터가 화면 정중앙에 박힘 |
| `SocketOffset.Y` | **0** | 카메라가 정면 그대로 → 캐릭터가 화면 옆 = **오버숄더** |

부수 효과로 **좌우 성분이 없어 요를 뽑을 일이 사라져 짐벌락이 구조적으로 불가능**해진다. 어깨 오프셋 처리와 짐벌락 해결이 같은 한 줄에서 나온다.

## 마우스 감도 노출

`Handle_Look`이 축 값을 배율 없이 넘기고 있어 감도가 IMC Scalar에만 박혀 있었다. 옵션화·상하 분리가 불가능한 구조.

```cpp
// KDPlayerController.h
UPROPERTY(EditAnywhere, Category = "Input|Look")
float LookSensitivityYaw = 1.f;
UPROPERTY(EditAnywhere, Category = "Input|Look")
float LookSensitivityPitch = 0.7f;

// KDPlayerController.cpp — Handle_Look
AddYawInput(Axis.X * LookSensitivityYaw);
AddPitchInput(Axis.Y * LookSensitivityPitch);
```

**`EditDefaultsOnly`가 아니라 `EditAnywhere`인 이유** — PIE 중 월드 아웃라이너에서 인스턴스를 잡아 실시간 조절할 수 있다. 감도는 숫자를 눈으로 찾아야 하는 값이라 이게 결정적이다.

**상하를 낮게 잡은 근거** — 피치 134°(`ViewPitchMin/Max = -89/45`)가 레일 **전체**를 훑는다. 팔 길이 1.21→382→52, 높이 514→117→−83, 거기에 `CF_FovByCamDist`로 FOV까지 딸려 움직인다. 1도당 화면 변화량이 요보다 훨씬 크다.

> 최종감도 = `IMC Scalar × 코드값`. 튜닝 창구를 하나로 모으려면 IMC Scalar를 `(1,1)`로 두는 게 낫다. **Negate(상하반전)는 그대로 둘 것** — 우리 배율은 양수라 부호를 안 건드린다.

## 값

| 항목 | 값 |
|---|---|
| 레일 3점 | `(-1.21,0,514.18)` `(-382.54,40,117.58)` `(-52.91,0,-82.98)` — SB 원본 좌표 + 탄젠트 이식 |
| `PitchAtStart` / `PitchAtEnd` | −89 / 45 (`ViewPitchMin/Max`와 동일해야 함) |
| `LookAtHeightOffset` | 34 |
| `CameraLagSpeed` / `MaxDistance` / `ProbeSize` | 19 / 57 / 10 (SB 실측 일치) |
| `LookSensitivityYaw` / `Pitch` | 1.0 / 0.7 |

**레일 3점 대입 피치**

| 레일 | `TargetArmLength` | `Dir.Z` | 피치 |
|---|---|---|---|
| t=0 | 1.21 | −480 | −89.9° 거의 수직 내려다봄 |
| t=1 | 382.54 | −83.58 | **−12.3°** |
| t=2 | 52.91 | +117 | +65.7° 올려다봄 |

> 중간 −12.3°는 SB의 −17.1°(`atan(118/383)`)보다 덜 숙인다. `LookAtHeightOffset = 34`가 표적을 올려서다. 발을 더 보이려면 이 값을 내린다(0이면 정확히 SB와 동일).

## 검증

**빌드** — 라이더가 선점 컴파일, `Build.bat`은 `Target is up to date` / `Succeeded`. DLL(14:24:12)이 소스 3개(13:49~14:00)보다 늦어 변경 포함 확인. **에러 0 · 경고 0**

**PIE 6항목 전수 통과**

| 항목 | 결과 |
|---|---|
| ★좌우 옆걸음 | ✅ 배경 스윙 소멸 |
| 제자리 회전 / 급턴 / 정면 접근 | ✅ 회귀 없음 |
| 정수리·발밑 끝까지 | ✅ 가드 제거해도 튐 없음 |
| 감도 실시간 조절 | ✅ (실사용 튜닝은 BP 기본값으로 진행 예정) |
| 화면 구도 | ✅ 기준선 부합 |
| 벽 접근 | ✅ 거리 따라 FOV 확장 — `CF_FovByCamDist` 의도된 동작 |

## 아키텍처 메모

**일반 교훈 — 부모의 출력을 읽어 계산하면 부모가 입힌 효과가 그 계산에 섞여 들어간다.** 랙·충돌은 "위치에만 걸리는 효과"인데, 그 결과 위치로 각도를 내는 순간 회전에도 걸린 셈이 된다. 부모의 **입력**을 읽으면 안 섞인다.

> **한 함수에서 버그가 반복되면 개별 증상이 아니라 그 함수의 입력 선택을 의심하라.** 세 번을 따로 고치느라 세 세션을 썼다.

**`AKDPlayerCharacter` 601줄** (`.h` 124 + `.cpp` 477) — 549 → 601. CLAUDE.md §1-1 Pawn 500줄 한도 초과가 심해졌다.

**단, 지금 분리하지 않는다.** 실측 결과 카메라 셋업은 생성자 20줄(`51~71`)뿐이라 빼봐야 581로 한도 초과가 유지된다. `.cpp` 477줄의 실제 무게는 다른 곳이다.

| 구간 | 줄 |
|---|---|
| `TryLightAttack` ~ `TryDodge` 6개 | **171** |
| `Tick` (선입력 버퍼 소비) | **101** |
| 생성자 | 47 (그중 카메라 20) |
| 나머지 15개 함수 | 158 |

`Try*` 6개는 전부 "입력 → GA 활성화" 같은 모양이고 서로 결합이 없어 읽기 어렵지 않다. 500줄은 **God Class 경보**지 자동 규칙이 아니다(§1-4 "빠른 답이 나오면 의심"). **`Try*`가 더 늘어나면 그때 `PlayerAbilityInput` 컴포넌트로 분리**하는 것이 순서다 — 카메라가 아니라.

## 남은 것

- **벽 충돌 시 조준 보정 부재** — 카메라가 앞으로 당겨져도 각도가 그대로라 캐릭터가 화면 아래로 밀린다. 현재는 거슬리지 않는 수준. 필요해지면 `IdealCam.X`를 실제 압축 거리로 대체(랙은 계속 제외)
- **IMC_Default의 `IA_Look` Scalar 미확인** — 코드 초기값의 실효 배율에 영향

## 관련

- 볼트 `notes/언리얼/카메라/06_스플라인_돌리_3인칭시점.md` §7-D — 흐름 대조(옛 8단계/새 5단계)
- 볼트 `notes/언리얼/트러블슈팅/카메라-랙이회전으로샘_실측위치조준.md` — 오류사례 전문
- 볼트 `notes/코드구조/02_플레이어.md` §6-B — 구현 현황
- 3부작 전부 볼트 `notes/언리얼/트러블슈팅/카메라-*.md` — 짐벌락 / 어깨 오프셋 / 랙 스윙. 앞 두 건의 해법 코드는 **본 작업으로 대체되어 현재 코드베이스에 없다**(각 문서 머리에 표기)
