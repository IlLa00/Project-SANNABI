# Project SANNABI

컴포넌트 기반 아키텍처를 활용해 개발한 산나비 모작 WinAPI포트폴리오 입니다.     
사용언어 : C++      
사용 툴 : VS 2022    
형상 관리 : GitHub Desktop     
제작 기간 : 25.07.30 ~ 25.08.27 (1달)       

**[제작 일지 (Notion)](https://www.notion.so/Project-SANABI-234894b38b1080baa682d16e91d4a5b1)**

### 플레이 영상
https://github.com/user-attachments/assets/f8c4c865-938c-41e9-b4fe-372d6d66b446

---

## 담당 시스템
1. [컴포넌트 기반 엔진 아키텍처](#1-컴포넌트-기반-엔진-아키텍처)
2. [사슬팔과 물리 시스템](#2-사슬팔과-물리-시스템)
3. [충돌 시스템](#3-충돌-시스템)
4. [맵 에디터](#4-맵-에디터)

---

## 1. 컴포넌트 기반 엔진 아키텍처

### Actor - 컴포넌트 보유 기반 클래스

```cpp
class Actor
{
public:
    virtual void Init();
    virtual void Update(float deltaTime);
    virtual void Render(HDC _hdcBack);
    virtual void Destroy();
    virtual void TakeDamage();

    void AddComponent(Component* component);

    template<typename T>
    T* GetComponent()
    {
        for (Component* comp : components)
        {
            T* castedComp = dynamic_cast<T*>(comp);
            if (castedComp) return castedComp;
        }
        return nullptr;
    }

    void SetPosition(Vector newPosition) { position = newPosition; }
    void SetVelocity(Vector newVelocity) { velocity = newVelocity; }
    void SetAcceleration(Vector newAcceleration) { acceleration = newAcceleration; }

    Actor* GetOwner() { return owner; }
    void SetOwner(Actor* newOwner) { owner = newOwner; }

protected:
    Vector position;
    Vector velocity;
    Vector acceleration;
    float speed = 300.f;
    Vector direction = Vector(1, 0);
    float scale = 0.25f;
    Actor* owner = nullptr;
    unordered_set<Component*> components;
};
```

### CollisionComponent - 함수 포인터 기반 이벤트

```cpp
class CollisionComponent : public Component
{
    using FOnComponentBeginOverlap = function<void(CollisionComponent*, HitResult)>;
    using FOnComponentEndOverlap = function<void(CollisionComponent*, HitResult)>;

public:
    void Init(Actor* _owner) override;
    void Update(float deltaTime) override;
    void Render(HDC hdc) override;

    void SetCollisionChannel(ECollisionChannel channel) { collisionChannel = channel; }
    ECollisionChannel GetCollisionChannel() { return collisionChannel; }
    RECT GetBoundingBox() const;

public:
    FOnComponentBeginOverlap OnComponentBeginOverlap;
    FOnComponentEndOverlap OnComponentEndOverlap;

private:
    Vector position;
    int offsetX, offsetY, width, height;
    bool bPendingKill = false;
    ECollisionChannel collisionChannel;
};
```

---

## 2. 사슬팔과 물리 시스템

![1-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/6a7fb0a1-f707-471f-8323-f83e7286391c)
![2-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/96a59514-bb06-4be6-9377-e79d22348e0e)

### PhysicsComponent - EPhysicsState 기반 상태 분기

<details>
<summary><b>PhysicsComponent::Update - 상태 머신</b></summary>

```cpp
enum class EPhysicsState
{
    Normal, Grappling, ExtendingChain,
    RightWallClimbing, LeftWallClimbing, CeilingHang
};

void PhysicsComponent::Update(float deltaTime)
{
    Super::Update(deltaTime);
    if (!owner) return;

    switch (physicsState)
    {
    case EPhysicsState::Normal:          UpdateNormalPhysics(deltaTime);   break;
    case EPhysicsState::Grappling:       UpdateGrapplePhysics(deltaTime);  break;
    case EPhysicsState::ExtendingChain:  UpdateExtendChainPhysics(deltaTime); break;
    case EPhysicsState::CeilingHang:     UpdateCeilingPhysics(deltaTime);  break;
    case EPhysicsState::RightWallClimbing:
    case EPhysicsState::LeftWallClimbing: UpdateClimbingPhysics(deltaTime); break;
    }

    // 충돌 상태에 따른 속도 보정
    Vector velocity = owner->GetVelocity();
    if (bOnGround && !bJustReleasedGrapple) velocity.y = 0;
    if (bOverlapCeiling) { velocity.y = 0; if (bJumping) bJumping = false; }
    if (bOverlapLeftWall)  velocity.x = 0;
    if (bOverlapRightWall) velocity.x = 0;
    owner->SetVelocity(velocity);
}
```
</details>

### 진자 운동 시뮬레이션

<details>
<summary><b>PhysicsComponent::UpdateGrapplePhysics</b></summary>

```cpp
void PhysicsComponent::UpdateGrapplePhysics(float deltaTime)
{
    if (!owner) return;

    Vector currentPos = owner->GetPosition();
    Vector toHookCurrent = currentPos - grapplePoint;
    float currentDistance = toHookCurrent.Length();

    GrapplingComponent* grappleComponent = owner->GetComponent<GrapplingComponent>();
    if (!grappleComponent) return;

    float maxChainLength = grappleComponent->GetMaxChainLength();

    // 최대 길이 초과 시 스프링처럼 당기기
    if (currentDistance > maxChainLength)
    {
        grappleLength = maxChainLength;
        float excessDistance = currentDistance - maxChainLength;
        Vector pullDirection = (grapplePoint - currentPos).Normalized();

        float pullStrength = 800.0f;
        Vector pullForce = pullDirection * pullStrength * excessDistance;

        Vector currentVelocity = owner->GetVelocity();
        currentVelocity += pullForce * deltaTime;
        currentVelocity *= 0.98f;  // 감쇠

        owner->SetVelocity(currentVelocity);
        owner->SetPosition(currentPos + currentVelocity * deltaTime);
        return;
    }

    grappleLength = currentDistance;
    currentAngle = atan2f(toHookCurrent.y, toHookCurrent.x);

    // 각가속도 = (g / L) * cos(θ) — 진자 운동 기본 공식
    float angularAccel = (gravity / grappleLength) * cosf(currentAngle) * swingGravityMult;

    // 플레이어 입력에 따른 추가 가속
    swingInputForce -= 50.0f;
    swingInputForce = clamp(swingInputForce, 0.f, 250.f);

    Vector direction = owner->GetDirection();
    if (direction.x != 0)
    {
        angularAccel += -direction.x * (swingInputForce / grappleLength);

        // 입력 방향과 스윙 방향 일치 시 타이밍 보너스
        if ((direction.x > 0 && angularVelocity > 0) ||
            (direction.x < 0 && angularVelocity < 0))
            angularAccel *= timingBonusMult;
    }

    // 관성 부스트
    if (abs(angularVelocity) > 0.1f)
    {
        float swingDirection = (angularVelocity > 0) ? 1.0f : -1.0f;
        angularAccel += swingDirection * 0.2f * (swingInputForce / grappleLength);
    }

    // 각속도 업데이트 + 감쇠
    angularVelocity += angularAccel * deltaTime;
    angularVelocity *= swingDamping;

    // 충돌 시 회전 정지
    bool willCollide = (bOverlapLeftWall || bOverlapRightWall || bOnGround || bOverlapCeiling);
    if (willCollide)
        angularVelocity = 0;

    if (!willCollide)
    {
        currentAngle += angularVelocity * deltaTime;
        Vector newPos;
        newPos.x = grapplePoint.x + grappleLength * cosf(currentAngle);
        newPos.y = grapplePoint.y + grappleLength * sinf(currentAngle);
        owner->SetPosition(newPos);
    }

    // 해제 시 자연스러운 사출을 위한 접선 속도 계산
    float tangentialSpeed = angularVelocity * grappleLength;
    Vector tangent(-sinf(currentAngle), cosf(currentAngle));
    owner->SetVelocity(tangent * tangentialSpeed);
}
```
</details>

### 충돌면 법선벡터 기반 상태 전환

<details>
<summary><b>PhysicsComponent::OnGroundBeginOverlap</b></summary>

```cpp
void PhysicsComponent::OnGroundBeginOverlap(CollisionComponent* other, HitResult info)
{
    if (other && other->GetCollisionChannel() == ECollisionChannel::WorldStatic)
    {
        Vector normal = info.collisionNormal;
        Player* player = dynamic_cast<Player*>(owner);
        if (!player) return;

        if (normal.x == 0 && normal.y == -1) // 지면
        {
            SetPhysicsState(EPhysicsState::Normal);
            bOnGround = true;
            player->UpdateMovementState(EPlayerMovementState::Idle);
        }
        else if (normal.x == 0 && normal.y == 1) // 천장
        {
            SetPhysicsState(EPhysicsState::CeilingHang);
            bOverlapCeiling = true;
            player->UpdateActionState(EPlayerActionState::Ceiling);
        }
        else if (normal.x == -1 && normal.y == 0) // 오른쪽 벽
        {
            if (bOnGround) { bBlockedRight = true; return; }
            SetPhysicsState(EPhysicsState::RightWallClimbing);
            bOverlapRightWall = true;
            player->UpdateActionState(EPlayerActionState::WallGrab);
        }
        else if (normal.x == 1 && normal.y == 0) // 왼쪽 벽
        {
            if (bOnGround) { bBlockedLeft = true; return; }
            SetPhysicsState(EPhysicsState::LeftWallClimbing);
            bOverlapLeftWall = true;
            player->UpdateActionState(EPlayerActionState::WallGrab);
        }
    }
}
```
</details>

### 람다 콜백 기반 이벤트 바인딩

<details>
<summary><b>PhysicsComponent::Init - CollisionComponent 콜백 바인딩</b></summary>

```cpp
void PhysicsComponent::Init(Actor* _owner)
{
    Super::Init(_owner);

    if (owner)
    {
        CollisionComponent* collisionComp = owner->GetComponent<CollisionComponent>();
        if (collisionComp)
        {
            collisionComp->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult info)
            {
                OnGroundBeginOverlap(other, info);
            };
            collisionComp->OnComponentEndOverlap = [this](CollisionComponent* other, HitResult info)
            {
                if (other->GetCollisionChannel() == ECollisionChannel::Projectile
                    && other->GetOwner()->GetOwner() == owner)
                    return;
                OnGroundEndOverlap(other, info);
            };
        }
    }
}
```
</details>

### GrapplingComponent - 오브젝트 풀 + 람다 충돌 콜백

<details>
<summary><b>GrapplingComponent::FireGrapple</b></summary>

```cpp
void GrapplingComponent::FireGrapple(Vector direction)
{
    if (poolInstance && !curProjectile)
    {
        curProjectile = poolInstance->GetProjectile(owner->GetPosition(), direction, pullSpeed);
        if (curProjectile)
        {
            bFiring = true;
            curProjectile->SetOwner(owner);

            CollisionComponent* comp = curProjectile->GetComponent<CollisionComponent>();
            if (comp)
            {
                // 충돌 시 콜리전 채널에 따라 분기하는 람다 바인딩
                comp->OnComponentBeginOverlap = [this](CollisionComponent* other, HitResult result)
                {
                    if (!curProjectile) return;
                    if (other && other->GetOwner() == owner) return;

                    curProjectile->SetFlying(false);

                    if (other->GetCollisionChannel() == ECollisionChannel::WorldStatic)
                        OnGrappling();  // 지형 부착 → 진자 운동 시작
                    else if (other->GetCollisionChannel() == ECollisionChannel::Character)
                    {
                        // 적 부착 → 대쉬 공격
                        GrapplingHookProjectilePool::GetInstance()->ReturnProjectile(curProjectile);
                        curProjectile = nullptr;
                        bFiring = false;

                        Player* player = dynamic_cast<Player*>(owner);
                        if (player)
                        {
                            player->SetTarget(other->GetOwner());
                            player->Dash(result.collisionPoint);
                        }
                    }
                };
            }
        }
    }
}
```
</details>

### GrapplingHookProjectilePool - 오브젝트 풀 패턴

```cpp
class GrapplingHookProjectilePool : public Singleton<GrapplingHookProjectilePool>
{
public:
    void CreatePool(int size)
    {
        for (int i = 0; i < size; i++)
        {
            GrapplingHookProjectile* projectile = new GrapplingHookProjectile();
            projectile->Init();
            projectile->Deactivate();
            pool.push_back(projectile);
        }
    }

    GrapplingHookProjectile* GetProjectile(Vector postion, Vector direction, float speed)
    {
        for (auto projectile : pool)
        {
            if (!projectile->IsActive())
            {
                projectile->Activate(postion, direction, speed);
                return projectile;
            }
        }
        return nullptr;
    }

    void ReturnProjectile(GrapplingHookProjectile* projectile)
    {
        if (projectile) projectile->Deactivate();
    }

private:
    vector<GrapplingHookProjectile*> pool;
};
```

---

## 3. 충돌 시스템

### HitResult + 채널 필터링 매트릭스

```cpp
struct HitResult
{
    bool isColliding = false;
    Vector collisionNormal;  // 충돌면 법선 벡터
    Vector collisionPoint;   // 충돌 지점
};

class CollisionManager : public Singleton<CollisionManager>
{
private:
    // 채널 간 충돌 규칙 2차원 배열
    bool bIgnore[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
    bool bBlock[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max]  = { 0 };

    vector<CollisionComponent*> collisionComponents;

    // 상태 추적 → 중복 이벤트 방지
    unordered_map<pair<CollisionComponent*, CollisionComponent*>, bool, PairHash> collisionPairs;
    unordered_map<pair<CollisionComponent*, CollisionComponent*>, HitResult, PairHash> lastCollisionInfo;
};
```

### AABB 충돌 검사 + 법선 벡터 계산

<details>
<summary><b>CollisionManager::CheckAABBCollision</b></summary>

```cpp
HitResult CollisionManager::CheckAABBCollision(CollisionComponent* comp1, CollisionComponent* comp2)
{
    HitResult info;
    if (!comp1 || !comp2) return info;

    RECT rect1 = comp1->GetBoundingBox();
    RECT rect2 = comp2->GetBoundingBox();

    // AABB 비교
    if (rect1.right < rect2.left || rect1.left > rect2.right ||
        rect1.bottom < rect2.top || rect1.top > rect2.bottom)
        return info;

    info.isColliding = true;

    float overlapX = min(rect1.right, rect2.right) - max(rect1.left, rect2.left);
    float overlapY = min(rect1.bottom, rect2.bottom) - max(rect1.top, rect2.top);

    // 겹침이 작은 축 → 충돌면 판정
    if (overlapX < overlapY)
    {
        if (rect1.left < rect2.left)
        {
            info.collisionNormal = Vector(-1, 0);
            info.collisionPoint = Vector(rect1.right, (rect1.top + rect1.bottom) / 2.0f);
        }
        else
        {
            info.collisionNormal = Vector(1, 0);
            info.collisionPoint = Vector(rect1.left, (rect1.top + rect1.bottom) / 2.0f);
        }
    }
    else
    {
        if (rect1.top < rect2.top)
        {
            info.collisionNormal = Vector(0, -1);
            info.collisionPoint = Vector((rect1.left + rect1.right) / 2.0f, rect1.bottom);
        }
        else
        {
            info.collisionNormal = Vector(0, 1);
            info.collisionPoint = Vector((rect1.left + rect1.right) / 2.0f, rect1.top);
        }
    }
    return info;
}
```
</details>

### 충돌 상태 변화 감지 + 콜백 호출

<details>
<summary><b>CollisionManager::Update</b></summary>

```cpp
void CollisionManager::Update()
{
    for (int i = 0; i < collisionComponents.size(); i++)
    {
        if (!collisionComponents[i]->IsActive()) continue;

        for (int j = i + 1; j < collisionComponents.size(); j++)
        {
            if (!collisionComponents[j]->IsActive()) continue;

            ECollisionChannel channel1 = collisionComponents[i]->GetCollisionChannel();
            ECollisionChannel channel2 = collisionComponents[j]->GetCollisionChannel();

            // 채널 필터링
            if (bIgnore[(int)channel1][(int)channel2] ||
                !bBlock[(int)channel1][(int)channel2])
                continue;

            auto pair = make_pair(collisionComponents[i], collisionComponents[j]);
            HitResult collisionInfo = CheckAABBCollision(collisionComponents[i], collisionComponents[j]);
            bool wasColliding = collisionPairs[pair];

            if (collisionInfo.isColliding && !wasColliding) // 새 충돌
            {
                lastCollisionInfo[pair] = collisionInfo;

                if (collisionComponents[i]->OnComponentBeginOverlap)
                    collisionComponents[i]->OnComponentBeginOverlap(collisionComponents[j], collisionInfo);

                if (collisionComponents[j]->OnComponentBeginOverlap)
                {
                    HitResult reversedInfo = collisionInfo;
                    reversedInfo.collisionNormal.x = -collisionInfo.collisionNormal.x;
                    reversedInfo.collisionNormal.y = -collisionInfo.collisionNormal.y;
                    collisionComponents[j]->OnComponentBeginOverlap(collisionComponents[i], reversedInfo);
                }
            }
            else if (!collisionInfo.isColliding && wasColliding) // 충돌 종료
            {
                HitResult lastInfo = lastCollisionInfo[pair];

                if (collisionComponents[i]->OnComponentEndOverlap)
                    collisionComponents[i]->OnComponentEndOverlap(collisionComponents[j], lastInfo);

                if (collisionComponents[j]->OnComponentEndOverlap)
                {
                    HitResult reversedInfo = lastInfo;
                    reversedInfo.collisionNormal.x = -lastInfo.collisionNormal.x;
                    reversedInfo.collisionNormal.y = -lastInfo.collisionNormal.y;
                    collisionComponents[j]->OnComponentEndOverlap(collisionComponents[i], reversedInfo);
                }
                lastCollisionInfo.erase(pair);
            }
            collisionPairs[pair] = collisionInfo.isColliding;
        }
    }
}
```
</details>

### 채널 필터링 초기화

```cpp
void CollisionManager::Init(HWND hwnd)
{
    // Block 관계 설정
    bBlock[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::Character]   = 1;
    bBlock[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::WorldStatic] = 1;
    bBlock[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::WorldDynamic]= 1;
    bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::WorldStatic]  = 1;
    bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::WorldDynamic] = 1;
    bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::Projectile]   = 1;
    bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::Character]    = 1;
    bBlock[(int)ECollisionChannel::Character][(int)ECollisionChannel::Perception]   = 1;
    // ... (양방향 대칭)

    // Ignore 관계 설정
    bIgnore[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::Projectile]   = 1;
    bIgnore[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::WorldStatic] = 1;
    bIgnore[(int)ECollisionChannel::WorldStatic][(int)ECollisionChannel::WorldDynamic]= 1;
    bIgnore[(int)ECollisionChannel::WorldDynamic][(int)ECollisionChannel::WorldDynamic]= 1;
}
```

---

## 4. 맵 에디터

https://github.com/user-attachments/assets/7e237f47-22fb-42b2-bcec-37487cec6ec7

https://github.com/user-attachments/assets/b9eae180-8472-433c-9c2b-7b059eac8ddb

### 렌더링 최적화

> Before
<img width="1614" alt="image" src="https://github.com/user-attachments/assets/41fe108b-5e7b-49f1-a95c-872f6c32419c" />

> After: 평균 100 FPS 안정화
