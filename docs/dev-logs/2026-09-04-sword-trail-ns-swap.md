# 2026-09-04 — 검 궤적: 리본 → 소켓 단발 방식 전환 + 총 발사체 재고 실측

연출 폴리싱 1일차. **검 궤적 27건을 `WeaponTrail`(리본) 에서 `PlayNiagaraEffect`(소켓 단발) 로 갈아탔다.** 총 발사체는 조사만 하고 미착수.

승환 지시(09-04) = 폴리싱을 앞으로 뺀다 → `1 연출 폴리싱 → 2 무기 IK 스냅 → 3 Vertex Shake`.

---

## 1. 조사 — `WeaponTrail` 노티는 검 전용이 아니었다

### ★ 76건이 두 갈래로 갈린다

`SB_Style_GameProject/Animation` 아래 몽타주 **62개** 전수 스캔:

```
KDAnimNotifyState_WeaponTrail   76건 / 몽타주 32개

NS                          무기태그   소켓            개수
NS_SlashTrail_Basic_Loop    Sword    Sword_Bottom     40   <- 검 궤적
NS_SB_Hit_Fire_Once         Gun      Muzzle           36   <- 총 머즐 플래시
```

🔴 **노티 클래스 이름만 보고 "검 궤적"이라 판단하면 총 머즐까지 같이 바꾼다.** 가르는 유일한 기준은 `WeaponMeshComponentTag`(`KDAnimNotifyState_WeaponTrail.h:33`). 이 클래스는 이름과 달리 **"소켓에 나이아가라를 붙이는 범용 통로"** 로 쓰이고 있다.

📌 총 머즐이 이 통로를 쓰는 건 의도된 것이다 — 머즐 전용 에셋이 없어 `NS_Hit` 계열을 전용한 기록이 있다(메모리 `reference_gun_asset_sources`).

---

## 2. 1차 시도 — NS 만 갈아끼움 (40건)

승환 선택 = Skill_04 가 쓰던 `NS_SB_Slash_Trail_01`.

```
NS_SlashTrail_Basic_Loop  ->  NS_SB_Slash_Trail_01
WeaponMeshComponentTag == "Sword" 인 40건 · 몽타주 29개
```

**여기서 멈추지 않고 방식 자체를 바꿨다** — 승환 제안: *"WeaponTrail 에 나이아가라를 None 으로 가고 socket 방식으로 트랙 하나 만들어서"*. Skill_04 가 쓰는 `AnimNotify_PlayNiagaraEffect` 방식이다.

---

## 3. ★★ 소켓이 어느 메시 것이냐가 방식을 결정한다

전환 전에 실측했다. **이게 이번 작업의 핵심 사실이다.**

```
소켓             검 StaticMesh   총 StaticMesh   캐릭터 SKM_Manny_GunSword
Sword_Bottom          O               x          x
Muzzle                x               O          x
Sword_FXSocket        x               x          O   -> bone = hand_r
soc_fx_root           x               x          O   -> bone = root
```

🔴 **`AnimNotify_PlayNiagaraEffect` 는 캐릭터 스켈레탈 메시에만 붙는다.** `Sword_Bottom`·`Muzzle` 은 무기 StaticMesh 소켓이라 **엔진 노티로는 손댈 수 없다.**

★ **그래서 `UKDAnimNotifyState_WeaponTrail` 이 존재한다.** 무기 메시를 `WeaponMeshComponentTag` 로 직접 찾아가려고 만든 클래스다. 09-02 의 *"`Muzzle` 이 `WeaponTrail` 클래스인 이유 = GC 가 총 메시를 못 가리킨다"*(`GameplayCueNotifyTypes.cpp:337` 이 캐릭터 메시만 본다) 와 **같은 뿌리**다.

📌 전환이 가능했던 이유 = `Sword_FXSocket` 이 **검 메시가 아니라 캐릭터 `hand_r` 본**에 달려 있어서다. 검 메시 소켓밖에 없었으면 이 방식 자체가 불가능했다.

⚠️ **대신 검이 아니라 손을 따라간다.** 검이 길거나 각이 지면 칼끝과 궤적이 어긋날 수 있다. 조정 손잡이 = `location_offset` · `rotation_offset` · 소켓을 `soc_fx_root` 로 교체.

---

## 4. 전환 실행 — 27건

각 몽타주마다:

```
1  Sword WeaponTrail 의 NiagaraSystem -> None        (노티는 남긴다)
2  트랙 "SlashFX" 신설 (주황)
3  같은 시각에 AnimNotify_PlayNiagaraEffect 추가
     template        NS_SB_Slash_Trail_01
     socket_name     Sword_FXSocket
     attached        True
     location_offset (0, 0, 50)      <- Skill_04 t=2.670 항목을 그대로 베낌
```

★ **시각은 전부 해당 `WeaponTrail` 의 시작 지점 그대로** (승환 지시). 코드로 읽어서 그 값을 그대로 썼다 — 눈대중으로 찍지 않았다.

```
콤보 21개 몽타주   24건
  01_01 0.4333 · 01_02 0.4667 · 01_03 0.5167 · 01_04 0.45
  02_01 0.4667 · 02_02 0.4    · 02_03 0.5333 · 02_04 0.4333
  03_01 0.3333 / 0.95         · 03_02 0.5667 · 03_03 0.6833 · 03_04 0.4833
  04_01 0.65   · 04_02 0.7333 · 04_03 0.6167 / 0.8833 · 04_04 0.4667
  05_01 0.45   · 05_02 0.5    · 05_03 0.7167 · 05_04 0.8167
  Air_02 0.6   · Air_04 0.4
반격 L / R          2건   0.55 · 0.2969
질주공격            1건   0.45
```

`Combo_03_01` · `Combo_04_03` 은 궤적이 2개라 각각 2건.

**건너뛴 것**
```
Combo_Air_01 · Air_03   Sword WeaponTrail 이 애초에 없다 (Gun / Muzzle 만)
Skill_01~04  13건        승환 판단 = 제외
```

🔴 **스킬을 제외한 이유** = 09-03 에 붙인 전용 나이아가라가 이미 있다(Skill_01 3개 · Skill_02 1개 · Skill_04 4개). 여기에 `Slash_Trail_01` 13개를 더 얹으면 연출이 겹쳐 뭉갠다 — **Skill_03 은 6건이라 0.9초에 6번 터진다.**

### 최종 분포

```
소켓 단발 (PlayNiagaraEffect @ Sword_FXSocket)   27건   콤보 24 · 반격 2 · 질주 1
리본 유지 (WeaponTrail)                          13건   Skill_01~04
NS=None 인 빈 WeaponTrail                        27건   껍데기만 남음 (아래 정리 대상)
총 머즐 (WeaponTrail @ Gun/Muzzle)               36건   손대지 않음
```

---

## 5. ★ 내가 틀렸던 것 · API 벽

### 오판 — `WeaponTrail` 을 통째로 "검 궤적"이라 부를 뻔했다

1차 스캔에서 노티 **127건**을 클래스 이름으로만 분류해 보고했다. `WeaponTrail` 을 검 궤적으로 뭉뚱그렸는데 **절반 가까이(36/76)가 총 머즐**이었다. 무기 태그를 안 보고 클래스만 본 탓이다.

📌 승환이 **"총은 머즐 말고 조준 모드일 때의 발사체"** 라고 짚어준 덕에 갈라 볼 이유가 생겼다. 안 짚었으면 머즐 36개까지 같이 갈아끼웠을 것이다.

### 파이썬 API 벽 4개

```
AnimMontage.Notifies              protected — get_editor_property 거부
                                  -> unreal.AnimationLibrary.get_animation_notify_events(m)
AnimNotify 객체 .export_text()    UObject 엔 없다 (struct 전용)
                                  -> 노티 시각 = FAnimNotifyEvent.export_text() 의 LinkValue
NiagaraSystem 내부                get_emitter_handles · exposed_parameters 접근 불가
                                  -> 리본/단발 판정도 유저 파라미터 이름도 PIE 눈 검증만
SkeletalMesh / StaticMesh.Sockets protected — 목록을 통째로 못 읽는다
                                  -> find_socket(name) 으로 이름을 하나씩 두드린다
```

⚠️ 같은 protected 인데 `KDAnimNotifyState_WeaponTrail` 의 프로퍼티는 `get/set_editor_property` 가 **정상 동작**한다. `AnimMontage.Notifies` 와 갈린 이유는 **확인하지 않았다**(추정 = 엔진 클래스 쪽 별도 차단).

### 크래시 회피

09-02 에 *"24개 일괄 투입 중 에디터 크래시(파일럿만 생존)"* 기록이 있어 **몽타주 하나씩 처리하고 즉시 `save_asset`** 했다. 크래시 없이 완주.

---

## 6. 총 발사체 — `NS_Laser` → `NS_ArrowTrail_Basic`

```
GA_Shoot (조준 중 좌클릭)  ->  BP_Bullet 스폰
   KDGameplayAbility_Shoot.h:38  ProjectileClass
   BP_Bullet 참조자 = GA_Shoot 하나뿐

BP_Bullet   /Game/SB_Style_GameProject/Combat/BP_Bullet   부모 = KDProjectile
  CollisionSphere      반경 12 · 프로파일 Projectile · 루트
  ProjectileMovement   속도 4000 (Initial = Max) · 중력 0 · 수명 3초
  Bullet (StaticMesh)  🔴 StaticMesh = 비어 있음
  NS_Laser (Niagara)   /Game/Animation/MartialArtsFightGame/demo/FX/NS_Laser
                       유저 파라미터 = User.Length · User.Scale
```

🔴 **눈에 보이는 건 `NS_Laser` 하나뿐이고 그건 `MartialArtsFightGame` 데모 팩 잔재다.** 승환 결정 = 다른 NS 로 교체.

### 재고 실측 — NiagaraSystem 309개

```
93 SlashTrailElemental · 84 Vefects Shockwave · 60 SB Effect
35 GhostSamurai · 17 Free_Magic · 10 ArrowTrail

후보 1순위  ArrowTrail 10종 (Basic / Basic_02 / Fire / Fire_02 / Holy /
                            Ice / Magic / Magic_2 / Nature / Water)
  발사체 꼬리 트레일 전용 팩 = 총알과 구조가 같다
  참조자 = 데모 맵 하나뿐 = 게임에서 0사용
  적 화살(BP_Arrow · BP_Arrow_Charged)은 NS 를 아예 안 쓴다(메시만) = 겹침 X
후보 2      Free_Magic Projectile1 / 2 (+_Base) · Attack1 / 2 / Attack_Line
후보 3 ❌   SlashTrailElemental 의 BP_Slash_Projectile_Start_* 14개
           NS 의존성 0건 = 나이아가라가 아니다 (메시·머티리얼 방식으로 추정)
           ⚠️ 의존성 스캔 결과일 뿐 열어보지 않았다
```

### 교체 실행

```
BP_Bullet 의 NiagaraComponent "NS_Laser"
  Asset   NS_Laser  ->  /Game/ArrowTrail/FX/NS_ArrowTrail_Basic
compile_blueprint + save_asset · 재조회 검증 통과
```

⚠️ **컴포넌트 이름은 `NS_Laser` 그대로 뒀다.** 내용물만 바꿨다 — 개명하면 BP 내부에서 그 변수를 참조하는 노드가 끊길 수 있다(2026-07 `UE 서브오브젝트 리네임 함정` 과 같은 계열). 방식이 확정된 뒤에 개명한다.

🔴 **BP 컴포넌트 편집 경로를 찾는 데 두 번 실패했다.**
```
bp.get_editor_property("simple_construction_script")   Blueprint 에 그 프로퍼티가 없다
MCP inspect set_component_property                     actorName 요구 = 월드 액터 전용
성공 = unreal.SubobjectDataSubsystem
  k2_gather_subobject_data_for_blueprint(bp)
  -> k2_find_subobject_data_from_handle(h)
  -> SubobjectDataBlueprintFunctionLibrary.get_object(d) / get_variable_name(d)
  -> obj.set_editor_property("asset", NEW)  ->  compile_blueprint  ->  save_asset
```
★ **UE5 에서 BP 컴포넌트 템플릿을 파이썬으로 만지는 표준 통로가 `SubobjectDataSubsystem` 이다.** 다음에 BP 컴포넌트 값을 바꿀 일이 있으면 여기부터 간다.

📌 `Effect/Elemental/` 에 6원소 세트(각 6개)를 확인했으나 **원소 톤 결정은 취소**(Basic 유지, 승환).

---

## 7. 검증

승환 확인 = **"좋다잉"** (PIE). 세부 항목별 판정은 따로 받지 않았다.

```
전환 후 재스캔 (코드 실측)
  PlayNiagaraEffect  NS_SB_Slash_Trail_01 @ Sword_FXSocket   28   (27 + Skill_04 원래 1)
  WeaponTrail        None / Sword                            27
  WeaponTrail        NS_SB_Slash_Trail_01 / Sword            13   Skill_01~04
  WeaponTrail        NS_SB_Hit_Fire_Once / Gun               36   총 머즐 무변경
```

---

## 8. 남은 것 · 정리 대상

```
총알 원소 선택     Basic 이 안 맞으면 Fire / Ice / Holy / Magic / Nature / Water / Basic_02
                 ⚠️ 발사체 속도 4000 이라 꼬리가 짧으면 눈에 안 걸릴 수 있다
스킬 궤적 13건    리본 방식으로 남음. 나머지와 느낌이 다르면 그때 전환
빈 WeaponTrail    NS=None 인 껍데기 27건. 동작에 무해하나 타임라인이 지저분하다
                 ⚠️ 지우면 되돌리기가 어려워지므로 방식이 확정된 뒤에
파라미터 잔류      빈 WeaponTrail 에 Lifetime_Trail 0.12 · Trail Width 200 이 남아 있다
                 NS 가 None 이라 무동작. 껍데기와 같이 정리
Bullet 스태틱메시  BP_Bullet 의 StaticMeshComponent 가 비어 있다 — 쓸지 지울지 판단
NS_Laser 개명      컴포넌트 이름이 내용물과 안 맞는다. 방식 확정 후
```

## 커밋

```
코드     docs 3건 (dev-log 신규 · INDEX · CURRENT)
Content  30개 uasset
         몽타주 29 (콤보 21 · 반격 2 · 질주 1 · 스킬 4 · 공중 1)
         BP_Bullet 1
         되돌리기 지점 = b873162
```

⚠️ **작업 중 "54개"라고 두 번 보고했는데 틀렸다.** 같은 몽타주를 NS 교체 때 한 번, 소켓 전환 때 또 한 번 저장한 것을 **각각 세서 더했다.** git 이 세는 건 변경된 **파일 수**지 저장 횟수가 아니다.

### ★ 커밋 직전에 내 것이 아닌 변경 2건을 걸렀다

```
GA_LightCombo · GA_HeavyCombo   19:01:29 · 둘 다 18193 -> 18168 (-25 바이트 동일)
```

이 세션에서 나는 몽타주와 `BP_Bullet` 만 `save_asset` 했다. **두 GA 는 건드린 적이 없어서 커밋 전에 물었고**, 승환 답 = **"그냥 디버그만 잠시 끈 것"** → `git checkout` 으로 되돌리고 커밋에서 뺐다.

★ **이게 `wrap-up` 의 *"커밋되지 않은 변경이 있으면 그것부터 판단한다"* 가 실제로 걸러낸 첫 사례다.** 파일 수만 세고 넘어갔으면 **디버그 플래그를 끈 상태가 이력에 섞여 들어갔을 것이다.**
📌 판별에 쓴 단서 = **수정 시각 + 크기 변화가 두 파일 동일(-25)**. 바이너리라 내용은 못 읽지만 **"내가 저장한 목록에 없다"** 는 것만으로 질문할 근거가 된다.
```
