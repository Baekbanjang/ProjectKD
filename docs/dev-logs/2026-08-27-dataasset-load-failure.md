# 2026-08-27 — DA 5개가 로드 불능이던 것 복구 (개명 재저장 누락)

클래스 개명 68개(08-26)의 **마지막 잔재.** 재저장 누락 5개가 리다이렉트 삭제와 맞물려
**로드 자체가 실패**하고 있었다. 에러 팝업도, 로그 경고도 없었다 — 콘솔에만 한 줄 남았다.

## 어떻게 발견됐나

적 상태 바 폭을 보려고 `DA_EnemyDef_*` 의 `MaxPoise` 를 읽다가 걸렸다.
`get_assets_by_class` 가 6개를 세는데 **값을 읽을 수 있는 건 3개뿐**이었다.

```
DA_Sword_Bandit     KDEnemyDefinitionDataAsset   MaxPoise=15   <- 읽힘
DA_Axe_Elite        EnemyDefinitionDataAsset     MaxPoise=?    <- 안 읽힘
```

레지스트리가 **없는 클래스 이름**을 들고 있었다.

## 원인 — 세 가지가 겹쳤다

```
1  개명            UEnemyDefinitionDataAsset -> UKDEnemyDefinitionDataAsset  (08-26)
2  전수 재저장 312개  f408086 — 여기서 5개가 빠졌다
3  리다이렉트 삭제    19a931a — 빠진 5개를 붙잡던 유일한 다리가 사라졌다
```

3번 시점에 **1·2를 통과하지 못한 에셋은 즉시 로드 불능**이 된다.

| 에셋 | 구워져 있던 옛 클래스 |
|---|---|
| `DA_Axe_Elite` · `DA_Parry_Bandit` · `DA_Dummy_Stagger` | `EnemyDefinitionDataAsset` |
| `DA_Execution_Axe` | `ExecutionProfile` |
| `DA_Weapon_Spear` | `WeaponDataAsset` |

실제 영향 = **엘리트·패링 밴딧·경직 더미가 스탯 DA 를 못 읽는 상태.** `MaxPoise`·`MaxHealth`·
넉백·`PoiseDamageByAttack` 이 전부 코드 기본값으로 돌고 있었다. 엘리트 처형 프로파일도 같이.

## ★ 08-26 검증 3종이 왜 못 잡았나

개명 dev-log 는 **"풀패스 옛 참조 0건"** 을 근거로 리다이렉트를 지웠다. 그 검사가 이렇게 돈다.

```
에셋을 연다  ->  참조 목록을 훑는다  ->  옛 이름이 있나 센다
```

**열리지 않는 에셋은 1단계에서 조용히 빠진다.** 참조를 못 세니 옛 이름도 안 나오고,
결과는 깨끗한 0건이다. **가장 깨진 에셋이 가장 정상으로 보이는** 검사였다.

> 교훈 — 개명·리다이렉트 삭제의 관문은 참조 검사가 아니라 **전수 로드 검사**다.
> "옛 이름이 남았나"가 아니라 **"전부 열리나"** 를 먼저 묻는다.

## 검사 (재사용)

```python
ar = unreal.AssetRegistryHelpers.get_asset_registry()
f  = unreal.ARFilter(recursive_paths=True, package_paths=["/Game"])

# 1) DA·설정 계열 — 레지스트리가 든 클래스가 실재하나
for a in ar.get_assets(f):
    cp = a.asset_class_path
    if str(cp.package_name) != "/Script/Project_KD":
        continue
    if unreal.load_object(None, "/Script/Project_KD." + str(cp.asset_name)) is None:
        print("죽음:", a.package_name, cp.asset_name)

# 2) BP 계열 — 부모 클래스가 실재하나 (1번에 안 걸린다. 레지스트리 클래스가 Blueprint 라서)
for a in ar.get_assets(f):
    if str(a.asset_class_path.asset_name) not in ("Blueprint","WidgetBlueprint","AnimBlueprint"):
        continue
    tag = a.get_tag_value("ParentClass")
    if not tag or "/Script/Project_KD." not in str(tag):
        continue
    inner = str(tag).split("'")[1] if "'" in str(tag) else str(tag)
    if unreal.load_object(None, inner) is None:
        print("부모 죽음:", a.package_name, inner)
```

⚠️ **1번만 돌리면 BP 를 통째로 놓친다.** BP 의 레지스트리 클래스는 `Blueprint` 라서
`/Script/Project_KD` 필터에 안 걸린다. 부모는 `ParentClass` 태그로 따로 봐야 한다.

## 복구 절차

```
1  에디터 종료                켠 채로 하면 메모리의 깨진 상태가 덮어쓴다
2  DefaultEngine.ini          [CoreRedirects] 에 ClassRedirects 3줄 임시 복원
3  에디터 켜고 5개 열어 값 확인 -> 재저장
4  리다이렉트 3줄 삭제
5  에디터 재시작 -> 전수 재검사   ini 는 시작 시에만 읽힌다
```

3번에서 값이 비어 보이면 **저장하지 말 것.** 리다이렉트가 물리면 옛 값이 그대로 읽히는 게
정상이고, 빈 채로 저장하면 그 상태가 구워진다.

## 검증

```
값 보존        엘리트 MaxPoise 25 / MaxHealth 1000 / 넉백 200 / Poise맵 3키 — 기록과 일치
디스크 바이트   5개 전부 옛 클래스 문자열 0건, 새 이름으로 교체 확인
재시작 후 검사  에셋 17개 · BP 43개 전수 0건 · 미저장 패키지 0
시각 대조      ini 삭제 20:00:22 < 에디터 시작 20:00:56 = 다리 없는 상태로 통과
```

## 곁들여 커밋한 것

`GA_LightCombo` · `GA_HeavyCombo` · `DA_LockOnConfig_Default` 가 미커밋으로 남아 있었다.
어제 필터 구조 작업(`1229465`)의 **에셋 짝** — 코드만 푸시되고 에셋이 빠져 있었다.

```
락온      Arc · 1000 · 반각 45 · 카메라 기준 · 각도순
자동조준   Arc · 800  · 반각 90 · 캐릭터 정면 · 최단거리     MaxAutoAimTurnAngle 135
```

커밋 = Content `79a4193`(복구 5개) · `6c3ba1e`(필터 값)
