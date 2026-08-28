# 2026-08-28 — 적 몽타주 노티 11개 복구 + 바이너리 전수 검사 도입

**적 공격 판정이 통째로 죽어 있었다.** 원인은 어제 지운 `ClassRedirects` 80줄이고,
오늘 아침 DA 5개 사건과 **같은 뿌리에서 두 번째로 터진 것**이다.

직전 = `2026-08-27-dataasset-load-failure.md`

## 어떻게 발견됐나

패링 2단계 작업 중 승환이 **"적 공격 트레이스가 안 나온다"** 고 했다.
`bDrawDebug` 는 적 근접 GA 전부 `True` 였다. 그러면 트레이스 자체가 안 도는 것이다.

```
AM_Bandit_Attack 의 노티      notify / notify_state_class 가 전부 None
바이너리에 구워진 이름         ANS_WeaponTrace · ANS_EnemyAttackWindow
                             · ANS_WeaponTrail · ANS_WindupSlow    (새 이름 0건)
엔진 노티는 멀쩡              AnimNotify_PlaySound 는 정상 로드
```

**우리 클래스만 죽었다.** 노티가 `null` 이면 트레이스도 디버그 표시도 없다.

## 원인 — 세 가지가 겹쳤다 (아침과 동일)

```
① 노티 클래스가 두 번 개명됐다
     ANS_WeaponTrace -> ANS_MeleeTrace -> KDAnimNotifyState_MeleeTrace
     ClassRedirects 가 연쇄로 이어주고 있었다
② 08-26 "전수 재저장 312개" 에 Content/Assets/Enemy/ 가 빠졌다
     AM_Bandit_Attack 의 마지막 커밋 = 662390c (MCP 도입 전)
③ 19a931a 가 ClassRedirects 80줄 삭제 -> 다리가 사라졌다
```

★ **"전수 재저장" 이 실제로는 전수가 아니었다.** `SB_Style_GameProject` 위주였고
`Content/Assets/Enemy/` 와 `Blueprints/Data/`(아침 DA 5개)는 대상에서 빠져 있었다.

## 함께 죽어 있던 것

`ANS_MeleeTrace` 만이 아니었다. 같은 몽타주의 노티가 전부 null 이었다.

```
ANS_MeleeTrace          근접 판정          <- 적 공격이 안 맞던 직접 원인
ANS_EnemyAttackWindow   퍼펙트 회피 판정창  <- 퍼펙트 회피도 같이 죽어 있었다
ANS_TelegraphWindow     적 전조
ANS_WeaponTrail         무기 트레일
ANS_WindupSlow          적 윈드업 슬로우
```

`ANS_EnemyAttackWindow` 는 `State.Combat.EnemyAttackHitWindow` 를 부여하는 노티다.
그게 없으면 `GA_Dodge` 의 퍼펙트 판정이 **영원히 안 걸린다.** 증상이 안 보고된 이유는
적 공격 자체가 안 맞아서 회피할 일이 없었기 때문으로 보인다(추정).

## ★ 왜 아침 검사가 못 잡았나 — 세 번째 사각

아침에 만든 검사는 두 갈래였다.

```
① 레지스트리 클래스가 /Script/Project_KD 인 에셋   -> DA 5개를 잡았다
② BP 의 ParentClass                              -> BP 43개 정상 확인
```

**몽타주는 둘 다에 안 걸린다.** 레지스트리 클래스가 `AnimMontage` 라 ①에 안 걸리고,
BP 가 아니라 ②에도 안 걸린다. **노티는 몽타주 안에 인스턴스로 박혀 있어 바깥에서 안 보인다.**

## ★ 그래서 만든 것 — 바이너리 전수 검사

에셋을 열지 않는다. `.uasset` 을 그냥 읽어서 옛 클래스명 문자열을 찾는다.
**에디터도 필요 없고 에셋 종류도 안 가린다.**

```python
# 지운 리다이렉트에서 (옛 이름 -> 새 이름) 쌍을 복원
#   git show <삭제커밋> -- Config/DefaultEngine.ini | grep -E "^-\+ClassRedirects" | sed 's/^-//'
import re, glob, collections
olds = {}   # {옛이름: 새이름}  ClassRedirects 줄에서 파싱

bad = collections.defaultdict(list)
for f in glob.glob('Content/Assets/**/*.uasset', recursive=True):
    s = set(x.decode() for x in re.findall(rb'[ -~]{4,}', open(f,'rb').read()))
    for o, n in olds.items():
        if o in s and n not in s:      # 옛 이름만 있고 새 이름이 없다 = 깨짐
            bad[f].append(o)
```

⚠️ **13GB 전체는 2분을 넘긴다.** 현역 폴더만(`Content/{Assets,Blueprints,SB_Style_GameProject,Animation,Map}`)
좁히면 1,821개에 2분 안쪽이다.

> 🔴 **규칙 — `ClassRedirects` 를 지우기 전에 이 검사를 돌린다.**
> 참조 검사는 열리는 에셋만 보고, 로드 검사는 인스턴스를 못 본다. 바이너리는 둘 다 본다.

## 범위와 복구

```
전체 몽타주 824개 중 죽은 노티 보유 70개 (죽은 노티 254개)
  /Game/TrickalFanGame/Animation/Combat   58개   버터 폐기 라인 — 복구 대상 아님
  /Game/Assets/Enemy/Axe                   5개   현역
  /Game/Assets/Enemy/Bandit                6개   현역
현역 폴더 바이너리 검사 = 1,821개 중 11개
```

절차는 아침과 같다.

```
1  에디터 종료
2  ini 에 ClassRedirects 80줄 임시 복원  (git show 로 통째로 꺼냄 — 5줄만 고르지 않았다)
3  에디터 재시작 -> 노티 생존 확인 (55개 살아남)
4  강제 저장
5  ini 80줄 삭제 -> 재시작 -> 재검사
```

### ⚠️ 4번에서 걸린 함정 — `Save` 가 아무 일도 안 한다

에디터에서 몽타주를 열고 `Save` 를 눌렀는데 **`git status` 에 아무것도 안 나왔다.**

```
원인   열기만 하고 수정하지 않으면 dirty 플래그가 안 선다.
       리다이렉트로 로드된 에셋은 메모리에선 새 클래스를 물지만 "바뀐 것" 으로 표시되지 않는다
       -> Save 가 저장할 게 없다고 판단하고 넘어간다
해결   unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
```

`only_if_is_dirty=False` 가 핵심이다. **이 플래그는 예전에도 한 번 걸렸던 자리다.**

### 검증

```
저장 전   Content/Assets 274개 중 옛 이름 잔존 11개
저장 후   0개
재시작 후  현역 폴더 죽은 노티 0건 (TrickalFanGame 58개는 폐기 라인이라 제외)
빌드·PIE  정상 (승환)
```

📌 남은 2건 = `/Game/Animation/Variant_Combat/Anims/AM_ComboAttack` · `AM_ChargedAttack`.
**우리 클래스가 아니다** — 옛 이름 스캔에 안 걸렸고, 엔진 로그의 `CBP_SandboxCharacter`
컴파일 오류와 같은 Epic 샘플 잔재로 보인다. 이미 Content 정리 후보로 기록돼 있다.

## 곁가지

`TrickalFanGame/Animation/Combat` 58개도 같은 이유로 죽어 있다. 버터 폐기 라인이고
**Content 정리 후보 1,955개에 이미 포함**돼 있어 이번엔 복구하지 않았다. 삭제 시 함께 정리.

## 교훈

> **"전수" 라고 적힌 작업을 믿지 말고 범위를 숫자로 남겨라.**
> 08-26 dev-log 의 "전수 재저장 312개" 는 폴더 범위를 안 적었고, 그래서 두 번 다
> "이미 다 했다" 고 믿은 채로 다리를 지웠다. 312 라는 숫자만으로는 무엇이 빠졌는지 알 수 없다.
