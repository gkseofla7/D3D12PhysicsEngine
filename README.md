# DaerimD3D11PhysicsEngine
# 영상
https://github.com/user-attachments/assets/0dd9a327-22c1-4db6-9243-89e9853470e1


https://github.com/user-attachments/assets/f9bd73a5-053a-4edb-bad3-0ba8e6f9acf8

# D3D11과 Bullet3 Physics 엔진을 사용해서 만든 게임 엔진
# Mesh, Animation등에 대한 데이터 모두 비동기 로딩
- Mesh 로딩에 경우엔 MeshLoadHelper 클래스, Animation 로딩에 경우엔 AnimHelper에서 로딩을 담당하고 있다.
- Loading에 관여하는 Thread 5개를 Thread Pool를 사용해서 로드한다. (ThreadPool.h)
# Render Thread 분리
- Render Thread를 먼저 분리한 이유는 [https://www.notion.so/daerimustudypage/eaa2eb07127a445ba0747d0073256e17](https://daerimustudypage.notion.site/eaa2eb07127a445ba0747d0073256e17) 에 정리돼있다(당연하게도 cpu가 gpu를 기다리는 시간을 최소화하기 위해서)
- 간단하게 분리하게된 이유를 설명하자면 D3D11의 Device와 Context는 멀티스레드 접근을 지원해주지 않기 때문에 Deffered Context를 활용하였다.
- Mesh, Animation등을 비동기 로딩처리하면서 텍스처 데이터를 매핑하고 중간 중간에 Buffer에 업데이트를 해야했는데 이런 부분들은 Deffered Context를 사용할 수 없게 돼있다..
  따라서 렌더링 스레드를 분리하지 않았다면 메인스레드에서 관리하고있는 Device와 Context에 접근해야되기 때문에 메인스레드에서 gpu 관련 업로드 과정들을 모두 돌릴 수 밖에 없었다.
- 따라서 Render Thread를 분리하고 Render Thread에서는 모든 gpu 메모리 업로드, 버퍼 생성들을 다루고
  마지막 gpu에 렌더링 명령은 메인스레드에서 담당하기로 했다.(메인 스레드, 렌더 스레드, 로딩 관련 스레드(5개), 더 좋은 방법이 있을듯 싶긴하다.. 뭔가 Render 과정을 Main Thread에서 돌리는건 찝찝한 )
- Render Theard로 분리하면서 수정이 많았던 부분은 Sphere Mesh나 Box Mesh는 현재 재사용하지 않고 바로 로컬 변수로 인자를 전달하도록 돼있었다. RenderThread로 분리하면서 결국 람다로 캡쳐해야되는데 로컬 변수를 캡쳐하면..
  따라서 다 R-Value로 넘겨서 소유권을 변경시켜줬다.
# 동일한 데이터를 갖고있는경우 공유, 다른 데이터만 별도로 gpu 메모리를 갖도록 함(애니메이션의 bone weight, Local To World Matrix 등)
- 여러 동일한 객체가 동일한 애니메이션을 틀어줄 때 애니메이션을  cpu 메모리에 로드한 후 각각의 객체가 해당 애니메이션 정보를 서로 다른 gpu 메모리에 복사해서 쓰고 있는 상황이다.
bullet3 물리 엔진 연동 및 컬리전, 물리 시뮬레이션 사용
- physX를 사용하지 않은 이유는 내부 코드를 볼 수 없다고 들어 모든 코드를 들여댜볼 수 있는 bullet3를 사용하였다.
- 현재 bullet3로 콜리전 체크해서 FireBall에 경우 Collision 발생시 FireBall를 제거하는 식으로 사용하였다.
# Actor State
- 액터 상태에 따라 애니메이션을 틀어주기 위해 State에 Animation을 엮어서 자동으로 애니메이션이 틀어지도록 구현을 해둔 상태다. 사실 이렇게 구현하는게 좋을지는.. 좀더 개발하면서 수정해나갈 계획이다
- ActorState.h를 확인
# 애니메이션
- 상체 하체 애니메이션 분리해서 사용가능하도록 기능 추가
# 앞으로 계획
- 나무 DaerimD3D11PhysicsEngine
- 액터에 대해 각 업데이트를 병렬처리로 수정하고, 각 액터마다 Tick을 Tick_Concurrency, Tick_GameThread 두개로 분리할 예정이다.
- 물리에 관심이 있다보니, 좀 더 물리 관련 로직들을 짜고 싶다..

https://daerimustudypage.notion.site/0cc33bcad32c433f894a4fecc12b1fb1

