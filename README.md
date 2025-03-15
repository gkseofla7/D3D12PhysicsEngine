# DaerimD3D12PhysicsEngine
# 영상
# 이전 D3D11 벡엔드 영상

https://github.com/user-attachments/assets/0dd9a327-22c1-4db6-9243-89e9853470e1


https://github.com/user-attachments/assets/f9bd73a5-053a-4edb-bad3-0ba8e6f9acf8

# D3D12로 백엔드 수정한 이후 영상


https://github.com/user-attachments/assets/fdc8105b-ccfd-490a-adb7-9cc72e77898a


# D3D12와 Bullet3 Physics 엔진을 사용해서 만든 게임 엔진
# D3D12로 벡엔드 변경
- 홍정모 그래픽스 새싹 코스 Part 4에서 제공된 Direct3D 11 기반 예제 코드를 Direct3D 12로 마이그레이션하였습니다.
- 중첩 렌더링 지원
  - 기존 구현에서는 GPU가 프레임을 렌더링할 때 CPU가 대기 상태에 놓이는 비효율적인 구조
  - 새 구현에서는 프레임별 리소스를 분리하여 다중 버퍼링(Multi-buffering) 방식으로 전환
  - CPU는 현재 GPU 작업과 병렬로 다음 프레임을 준비할 수 있어 작업 효율성이 향상
- 최대한 CPU가 GPU 작업으로 Blocking 되지 않도록 고려
  - [https://daerimustudypage.notion.site/1b19eb57b07e80aaa541c6f85dff0ee9](https://daerimustudypage.notion.site/1b09eb57b07e8044a0cde073f58fd3ab)
![image](https://github.com/user-attachments/assets/815d2fa6-15c4-48d4-a53d-8dc72eb65a8d)
## 비동기 리소스 로딩

### ✅ Mesh 및 Animation 데이터 비동기 로딩
 **비동기 로딩을 담당하는 헬퍼 클래스**
-  `MeshLoadHelper` (`MeshLoadHelper2.h`) → **Mesh 로드 담당**
-  `AnimHelper` (`AnimHelper2.h`) → **Animation 로드 담당**
-  **Thread Pool 활용**하여 비동기 처리

###  Resource Command Pool
-  **리소스 로딩 스레드**가 **GPU 메모리 업로드 시**
- **`Resource CommandList Pool`에서 명령 리스트(Command List)**를 가져와 **GPU 요청 처리**

---

##  메모리 최적화

### 애니메이션 데이터 공유
-  **동일한 애니메이션 데이터를 사용하는 객체들은 CPU 메모리에서 공유**
-  **GPU 메모리에서는 개별적인 애니메이션 정보(예: Bone Weight, Local To World Matrix) 복사하여 사용**

### 리소스 재사용 (`s_resourceMap`)
-  **이미 로드된 리소스를 재사용**하여 **메모리 낭비 방지**

---

##  Actor State 시스템 (`ActorState.h`)

###  애니메이션 연동
-  액터의 **상태(State)에 따라 자동으로 애니메이션 실행**
-  **State와 Animation을 연결**하여 **상태 전환 시 애니메이션이 동기화**

###  기능 구현
-  **이동 로직 및 충돌 연출**  
  - 예: **Projectile 충돌 시 객체가 날아가는 효과 구현**

---

##  Bullet3 물리 엔진 연동

###  왜 `PhysX` 대신 `Bullet3`?
-  **PhysX는 내부 코드를 볼 수 없음**
-  **Bullet3는 오픈소스이므로 모든 코드를 직접 확인 가능**

###  현재 적용 사례
-  `Bullet3`를 활용한 **콜리전 체크**
-  `FireBall` 충돌 시 **Collision 감지 후 FireBall 제거**

---

## 🎭 애니메이션 시스템

### 🔗 상체 & 하체 애니메이션 분리
- **개별 제어 가능한 애니메이션 분리 기능 추가**
  - 예: **상체는 공격 모션, 하체는 이동 모션**

### 🏃 루트 모션 애니메이션
- **루트 모션 기반 이동**
- **애니메이션 종료 시 `Transform` 값을 월드 좌표(World Transform)에 합산하여 이동 처리**

---
# 앞으로 계획
- 디퍼드 렌더링으로 교체(빛 연산 없이 돌려봤을때 성능에 아직 큰 차이가 없어서 나중에 변경 예정)
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

