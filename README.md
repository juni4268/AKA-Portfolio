# AKA — 보스 러시 액션 게임

![AKA cover](Docs/images/cover.png)

> Unreal Engine 5.6 · C++ · Behavior Tree AI · 1인 플레이 보스 러시

보스를 쓰러뜨릴 때마다 보스가 강해져 다시 등장하고, 클리어 타임(랩타임)을 기록하는 보스 러시 액션 게임입니다.

<!-- TODO: 플레이 영상 링크로 바꿔 주세요 -->
**▶ 플레이 영상:** [YouTube](https://youtube.com/)
**⬇ 플레이 빌드 (Windows):** [Releases](../../releases)

---

## 주요 구현 내용

### 1. Behavior Tree 기반 보스 AI
- `ABossAIController` + Behavior Tree/Blackboard로 보스 행동을 구성했습니다.
- **커스텀 Service:** [`BTService_UpdateBossState`](Source/AKA/BTService_UpdateBossState.cpp)
  - 0.1초 간격으로 거리, 페이즈, 스턴 여부, 패턴별 쿨타임을 Blackboard에 갱신합니다.
  - 공격 플래그가 몽타주 없이 남아 있으면 강제로 초기화하는 **워치독 로직**으로 AI가 멈추는 버그를 막았습니다.
- **커스텀 Task:** 근접 공격, 돌진, 점프 공격, 원거리 공격과 각 2페이즈 변형을 노드 단위로 분리했습니다.
  - [`BTTaskNode_Attack`](Source/AKA/BTTaskNode_Attack.cpp) / [`_Attack_P2`](Source/AKA/BTTaskNode_Attack_P2.cpp)
  - [`BTTaskNode_DashAttack`](Source/AKA/BTTaskNode_DashAttack.cpp)
  - [`BTTaskNode_JumpAttack`](Source/AKA/BTTaskNode_JumpAttack.cpp) / [`_JumpAttack_P2`](Source/AKA/BTTaskNode_JumpAttack_P2.cpp)
  - [`BTTaskNode_RangeAttack`](Source/AKA/BTTaskNode_RangeAttack.cpp)
- **커스텀 Decorator:** [`BTDecorator_CheckLineOfSight`](Source/AKA/BTDecorator_CheckLineOfSight.cpp)로 시야 확보 여부를 판정해 패턴 선택 조건으로 사용합니다.

### 2. 보스 패턴과 페이즈 시스템 — [`BossMonster.cpp`](Source/AKA/BossMonster.cpp)
| 패턴 | 내용 |
|---|---|
| 근접 공격 | 양손 본에 붙인 `BoxComponent`로 피격을 판정합니다 |
| 돌진 | `EDashState`(준비 → 돌진) 상태 머신으로 처리하고, AnimNotify로 준비가 끝나는 시점을 맞춥니다 |
| 점프 공격 | 장판(Indicator)이 일정 시간 플레이어를 추적하다가 멈추면 보스가 그 위치에 착지합니다 |
| 원거리 공격 | 번개 투사체를 부채꼴로 발사하며, 페이즈가 오를수록 강해집니다 |

- **3페이즈:** HP가 2/3, 1/3 아래로 떨어질 때 페이즈가 전환되고, 오버레이 머티리얼로 시각적인 변화를 줍니다.
- **보스 레벨업:** 처치할 때마다 HP, 데미지, 투사체 속도가 레벨에 비례해 늘어납니다(`LevelUpBoss`).

### 3. 기믹: 스턴 기둥 — [`StunnablePillar`](Source/AKA/StunnablePillar.cpp)
- 플레이어가 돌진하는 보스를 기둥에 부딪히게 하면 보스가 **스턴** 상태가 되어 딜 타이밍이 생깁니다.
- 기둥은 일정 횟수를 맞으면 물리 충격을 받아 쓰러집니다.

### 4. 플레이어 — [`AkaPlayer.cpp`](Source/AKA/AkaPlayer.cpp)
- Enhanced Input 기반으로 이동, 공격(번개 발사), 일시정지, 랩타임 보드를 조작합니다.
- 피격 시 보스 위치를 기준으로 넉백 방향을 계산합니다.

### 5. 게임 흐름 / UI
- [`AKATrigger`](Source/AKA/AKATrigger.cpp)는 구역 진입 시 카메라 전환, HUD 표시, 랩타임 보드 표시를 처리합니다.
- [`AKAGameInstance`](Source/AKA/AKAGameInstance.cpp)는 레벨 간 데이터 유지와 랩타임 기록 및 정렬을 담당합니다.
- [`PlayerWidget`](Source/AKA/PlayerWidget.cpp)과 [`TitleWidget`](Source/AKA/TitleWidget.cpp)은 HP, 보스 게이지, 타이틀 UI를 담당합니다.

---

## 프로젝트 구조
```
Source/AKA/
├─ BossMonster.*          보스 본체 (패턴, 페이즈, 스턴, 레벨업)
├─ BossAIController.*     AI 컨트롤러
├─ BTService_* / BTTask* / BTDecorator_*   Behavior Tree 노드
├─ AkaPlayer.*            플레이어
├─ LightningBolt.* / BossLightningBolt.*   투사체
├─ JumpAttackIndicator.*  점프 공격 장판
├─ StunnablePillar.*      스턴 기둥 기믹
├─ AKATrigger.*           구역 트리거
└─ AKAGameInstance.* / *Widget.*   게임 흐름, UI
```

## 참고
- 이 저장소에는 **코드와 설정만** 포함되어 있습니다. 아트, 사운드, 이펙트 등은 마켓플레이스(Fab) 에셋을 사용해서 라이선스상 포함하지 않았습니다. 그래서 이 저장소만으로는 에디터에서 프로젝트를 열 수 없으며, 플레이는 Releases의 빌드로 해 주세요.
- 엔진: Unreal Engine 5.6 / 플러그인: PaperZD
