# DaerimD3D12PhysicsEngine
# 영상
https://github.com/user-attachments/assets/0dd9a327-22c1-4db6-9243-89e9853470e1


https://github.com/user-attachments/assets/f9bd73a5-053a-4edb-bad3-0ba8e6f9acf8
# 프로젝트 개요
이 프로젝트는 홍정모 그래픽스 새싹 코스 Part 4에서 제공된 Direct3D 11 기반 예제 코드를 Direct3D 12로 마이그레이션한 프로젝트입니다. 추가로 
# D3D12과 Bullet3 Physics 엔진을 사용해서 만든 게임 엔진
# D3D12로 벡엔드 변경
- 홍정모 그래픽스 새싹 코스 Part 4에서 제공된 Direct3D 11 기반 예제 코드를 Direct3D 12로 마이그레이션하였습니다.
- 중첩 렌더링 지원
  - 기존 구현에서는 GPU가 프레임을 렌더링할 때 CPU가 대기 상태에 놓이는 비효율적인 구조였습니다.
  - 새 구현에서는 프레임별 리소스를 분리하여 다중 버퍼링(Multi-buffering) 방식으로 전환했습니다.
  - CPU는 현재 GPU 작업과 병렬로 다음 프레임을 준비할 수 있어 작업 효율성이 향상되었습니다.
# 비동기 리소스 로
- Mesh와 Animation 데이터를 비동기적으로 로드:
    - MeshLoadHelper 클래스가 Mesh 로드를 담당.
    - AnimHelper 클래스가 Animation 로드를 담당.
- Thread Pool을 활용
- Command List Pool 개발
- gpu commandList Pool 개발
    - 리소스 로딩 스레드가 GPU 메모리 업로드 시, Resource CommandList Pool에서 명령 리스트(Command List)를 가져와 GPU 요청 처리.
# 메모리 최적화
- 동일한 애니메이션 데이터를 사용하는 여러 객체는 CPU 메모리에서 데이터를 공유
- 각 객체는 GPU 메모리에서 개별적인 애니메이션 정보(예: Bone Weight, Local To World Matrix)를 복사하여 사용.
# Bullet3 물리 엔진 연동
- physX를 사용하지 않은 이유는 내부 코드를 볼 수 없다고 들어 모든 코드를 들여댜볼 수 있는 bullet3를 사용하였다.
- 현재 bullet3로 콜리전 체크해서 FireBall에 경우 Collision 발생시 FireBall를 제거하는 식으로 사용하였다.
# Actor State 시스템(ActorState.h)
- 애니메이션 연동
  - 액터의 상태(State)에 따라 자동으로 애니메이션을 실행하도록 구현.
  - State와 Animation을 연결하여 상태 전환 시 애니메이션이 동기화되도록 설계.
- 기능 구현
  - 이동 로직 및 프로젝트 충돌 연출(예: Projectile 충돌 시 객체가 날아가는 효과 구현)
# 애니메이션 시스템
- 상체와 하체 애니메이션 분리:
  - 상체와 하체를 개별적으로 제어할 수 있는 애니메이션 분리 기능 추가.
- 루트 모션 애니메이션:
  - 루트 모션 기반 애니메이션에서 이동한 Transform 값을 애니메이션 종료 시점에 **월드 좌표(World Transform)**에 합산하여 처리.
# 앞으로 계획
- 디퍼드 렌더링으로 교체
- 나무, 풀 등 추가
- 액터에 대해 각 업데이트를 병렬처리로 수정하고, 각 액터마다 Tick을 Tick_Concurrency, Tick_GameThread 두개로 분리할 예정이다.
- 물리에 관심이 있다보니, 좀 더 물리 관련 로직들을 짜고 싶다..
- 물리 관련 로직도 따로 스레드 분리 필요
# D3D12로 백엔드 수정 후 미사용된 내용
# Render Thread 분리
- 분리 이유
  - CPU가 GPU를 기다리는 시간을 최소화하여 작업 효율성을 높이기 위해 Render Thread를 분리.
  - D3D11의 Device와 Context는 멀티스레드 접근을 지원하지 않으므로, 비동기 로딩 및 데이터 업데이트 과정에서 Deferred Context 사용에 제약이 있었음.
- Deferred Context의 한계:
  - Deferred Context는 동시 처리를 지원하지 않고, 명령을 쌓아둔 후 Main Context에서 실행해야 하는 구조.
  - GPU 리소스 업로드와 버퍼 업데이트 과정에서 메인 스레드에 의존해야 하는 비효율성이 존재.
- 분리된 역할
  - Render Thread: GPU 메모리 업로드, 버퍼 생성 등 GPU 리소스와 관련된 모든 작업을 처리.
  - Main Thread : 최종 렌더링 명령(Render())을 GPU에 제출.
  - 로딩 스레드(5개) : 비동기적으로 리소스를 로드하며, Render Thread와 협력하여 GPU 메모리로 데이터를 업로드.

https://daerimustudypage.notion.site/0cc33bcad32c433f894a4fecc12b1fb1

