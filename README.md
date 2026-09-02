# 액션 RPG (Unreal Engine 5 / C++)

RPG의 핵심 기능을 언리얼 엔진의 최신 시스템으로 직접 구현한 개인 프로젝트입니다.
Unreal Fest, NDC 등의 개발자 발표에서 다뤄진 사례를 이정표로 삼되, 각 기법이 실제로 유효한지
직접 측정하고 검증하는 데 중점을 두었습니다.

| | |
|---|---|
| **개발 기간** | 2026.04 ~ 진행 중 |
| **인원** | 개인 프로젝트 |
| **엔진 / 언어** | Unreal Engine 5.7 / C++ |

> ## 저작권 및 사용 범위 고지 / Copyright & Usage Notice
>
> 본 저장소의 코드는 **오직 교육용 및 비상업적 용도로만** 사용되어야 합니다.
>
> The code in this repository is
> **strictly for educational and non-commercial use only.**
---

## 목차

- [기술 스택](#기술-스택)
- [1. 타격 판정 비교 분석](#1-타격-판정-비교-분석)
- [2. Iris Replication System 성능 검증](#2-iris-replication-system-성능-검증)
- [3. Exclusive Task 구현](#3-exclusive-task-구현)
- [4. 액션 연출 카메라 에디터 툴](#4-액션-연출-카메라-에디터-툴)
- [5. Game Feature Plugin 기반 릴리스 관리](#5-game-feature-plugin-기반-릴리스-관리)
- [코드 안내](#코드-안내)
- [참고 자료](#참고-자료)

---

## 기술 스택

### 전투

![Sweep Trace](https://img.shields.io/badge/Sweep_Trace-1C4ACC?style=flat-square)
![Triangle Intersection](https://img.shields.io/badge/Triangle_Intersection-1C4ACC?style=flat-square)
![Animation Data](https://img.shields.io/badge/Animation_Data-1C4ACC?style=flat-square)

### 네트워크

![Iris Replication System](https://img.shields.io/badge/Iris_Replication_System-2D6A9F?style=flat-square)
![Filtering](https://img.shields.io/badge/Filtering-2D6A9F?style=flat-square)
![Prioritization](https://img.shields.io/badge/Prioritization-2D6A9F?style=flat-square)

### 멀티스레드

![Task System](https://img.shields.io/badge/Task_System-6D4C8F?style=flat-square)
![Exclusive Task](https://img.shields.io/badge/Exclusive_Task-6D4C8F?style=flat-square)

### 에디터

![FAssetEditorToolkit](https://img.shields.io/badge/FAssetEditorToolkit-4A7C59?style=flat-square)
![Gameplay Camera System](https://img.shields.io/badge/Gameplay_Camera_System-4A7C59?style=flat-square)
![Slate](https://img.shields.io/badge/Slate-4A7C59?style=flat-square)

### 데이터 기반 설계

![Game Feature Plugin](https://img.shields.io/badge/Game_Feature_Plugin-B85042?style=flat-square)
![Data Asset](https://img.shields.io/badge/Data_Asset-B85042?style=flat-square)

### 측정

![Unreal Insights](https://img.shields.io/badge/Unreal_Insights-36454F?style=flat-square)
![AWS EC2](https://img.shields.io/badge/AWS_EC2-36454F?style=flat-square)

---

## 1. 타격 판정 비교 분석

**배경** — 근접 무기의 타격 판정은 프레임 레이트에 따라 정확도 편차가 발생합니다.
낮은 FPS에서도 일관된 타격 경험을 제공하려면 어떤 판정 방식이 적합한지 직접 비교했습니다.

**구현한 세 가지 방식**

- **Sweep Trace** — 이전 프레임과 현재 프레임 사이의 이동 경로를 Sweep 연산으로 스캔
- **Triangle Intersection** — 무기 끝과 밑동 소켓의 이전·현재 위치를 삼각형 교차로 보간
- **Animation Data** — 애니메이션 시퀀스의 Bone Animation Track을 참조해 특정 시점의 트랜스폼으로 판정

**측정 결과** — Unreal Insights로 Detection Tick당 평균 비용 산출

| 방식 | 평균 비용 |
|---|---|
| Sweep Trace | 0.014ms |
| Triangle Intersection | 0.072ms |
| Animation Data | 0.134ms |

**결론** — 60FPS 기준 프레임 예산 16.66ms를 놓고 볼 때, 가장 비싼 Animation Data 방식도
전체 예산의 1% 미만입니다. 프레임 레이트에 영향받지 않는 정확도를 우선한다면
비용 차이는 감당할 만한 수준이라고 판단했습니다.

---

## 2. Iris Replication System 성능 검증

**문제 의식** — Iris는 활성화만으로도 네트워크를 상당히 최적화하지만,
어떤 오브젝트를 얼마나 자주 복제할지는 게임마다 다른 영역이라 엔진이 대신 결정해줄 수 없습니다.
특히 RPG는 캐릭터 이동과 인벤토리 등 대량의 데이터가 복제되어 서버 성능에 직접적인 영향을 미칩니다.
Iris의 filtering과 prioritization을 커스터마이징했을 때 실제로 얼마나 개선되는지 측정했습니다.

**테스트 환경**

- 인프라: AWS EC2 — 서버 1대(z1d.xlarge), 클라이언트 4대(c6i.2xlarge)
- 시나리오: 더미 클라이언트 100명 접속, 80명 이상 접속 시 게임 시작 후 5분간 트레이스 수집
- 조건 통제: 클라이언트를 시작 지점 반경 10m로 제한하고 캐릭터·카메라 충돌을 비활성화해
  네트워크 비용만 분리 측정

**결과**

| 지표 | Default | Iris + Sphere | Iris + FoV |
|---|---|---|---|
| 서버 틱 레이트 | 10.8Hz | 18.0Hz | 17.5Hz |
| 리플리케이션 평균 송신 비용 | 51.57ms (55.6%) | 24.40ms (43.9%) | 25.50ms (44.6%) |
| 오브젝트 갱신 처리량 | 47,591/s | 89,907/s | 96,067/s |
| 우선순위 평균 연산 비용 | 1.9ms (2.0%) | 82.8μs (0.15%) | 116.8μs (0.20%) |

**결론** — Iris 도입만으로 프레임당 리플리케이션 비용이 절반 이하로 감소했습니다.
Sphere와 FoV 우선순위 설정자 간 연산 비용 차이는 약 34μs로 유의미하지 않았고,
FoV의 처리량이 7.8% 높았습니다. 따라서 두 방식 중 선택은 성능이 아니라
게임의 시야 패턴을 기준으로 판단해야 한다고 결론지었습니다.

---

## 3. Exclusive Task 구현

**문제** — 여러 태스크가 같은 자원에 접근할 때 락을 사용하면, 대기하는 워커 스레드가
점유된 채로 낭비됩니다. 게임 로직에 필요한 자원이 런타임에 결정되기 때문에
자원별 큐를 미리 구성하는 방식으로는 대응할 수 없었습니다.
자원이 하나라면 언리얼의 `FPipe`로 해결되지만, 둘 이상을 동시에 배타적으로
점유해야 하는 경우가 존재했습니다.

**구현** — NDC 발표에서 제시된 Exclusive Task 개념을 참고해 직접 구현했습니다.

- 각 공유 자원이 자신을 마지막으로 사용한 태스크를 `TailTask`로 보관
- 새 태스크는 필요한 자원들의 `TailTask`를 자기 자신으로 Exchange
- Exchange로 얻은 이전 태스크들을 새 태스크의 선행 조건으로 설정

**검증** — Unreal Insights로 실행 흐름을 분석해, 의도한 태스크·자원 간 의존 관계가
실제 실행 순서와 일치함을 DAG 형태로 확인했습니다.

**한계** — 데드락이 발생할 가능성이 완전히 없는지는 아직 형식적으로 검증하지 못했습니다.

---

## 4. 액션 연출 카메라 에디터 툴

**문제** — 카메라 연출이 전투 로직에 하드코딩되어 있어, 연출을 수정할 때마다
게임 코드를 건드려야 했습니다. 시야각과 블렌드 값 같은 설정은 매번 게임을 실행해야
확인할 수 있어 반복 비용이 컸습니다.

**구현** — Gameplay Camera System을 기반으로 독립 에디터를 제작했습니다.

1. **에디터 프레임워크** — `FAssetEditorToolkit`으로 애니메이션 에디터와 같은 독립 창을 구성하고 뷰포트·디테일·타임라인 패널 배치
2. **프리뷰 뷰포트** — 프리뷰 월드에 더미 플레이어 컨트롤러와 캐릭터를 스폰하고, 매 프레임 GCS가 계산한 트랜스폼과 FoV를 뷰포트 카메라에 반영
3. **타임라인 트랙 패널** — Slate 위젯의 그리기·입력 처리를 직접 구현해 애니메이션 타임라인상의 구간을 시각적으로 편집
4. **데이터 편집 동기화** — `PostEditChangeProperty`를 활용해 디테일 패널의 변경이 즉시 프리뷰에 반영되도록 구성

**결과** — PIE에 진입해 상황을 재현할 필요 없이 에디터에서 실시간 검증이 가능해졌습니다.
연출 데이터를 에셋으로 분리해, 신규 연출 추가 시 코드 수정 없이 에셋만 변경하면 됩니다.

---

## 5. Game Feature Plugin 기반 릴리스 관리

**문제 의식** — 라이브 서비스 게임은 여러 릴리스를 동시에 개발하기 때문에,
릴리스를 브랜치로 분리하면 병합 비용과 브랜치 간 콘텐츠 이동 비용이 커집니다.
콘텐츠가 빌드에 포함되는 시점부터 런타임에 주입되는 시점까지 전 구간을
코드 수정 없이 데이터로 제어할 수 있는지 시험했습니다.

**구현**

- 캐릭터·스킬·UI 등 콘텐츠를 Game Feature Plugin으로 분리
- 용도별로 묶은 **번들**(캐릭터1 게임플레이, 캐릭터1 코스튬)과
  릴리스에 포함할 번들 목록인 **패키지**를 각각 데이터 에셋으로 정의
- 패키지 선택에 따라 해당 플러그인만 등록되도록 필터링하고,
  빌드 스크립트가 이 정보를 읽어 미포함 플러그인을 빌드에서 제외

**결과** — 콘텐츠 단위와 출시 일정을 분리해 하나의 브랜치에서 여러 릴리스를 병렬 개발할 수 있게 되었고,
재빌드 없이 에디터에서 패키지를 전환해 각 버전의 상태를 확인할 수 있습니다.

---

## 참고 자료

- NDC
- Unreal Fest
- Unreal Engine 공식 문서
