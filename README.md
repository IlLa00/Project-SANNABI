# Project SANNABI    
컴포넌트 기반 아키텍처를 활용해 개발한 산나비 모작 WinAPI포트폴리오 입니다.     
사용언어 : C++      
사용 툴 : VS 2022    
형상 관리 : GitHub Desktop     
제작 기간 : 25.07.30 ~ 25.08.27 (1달)       
제작 일지 : https://www.notion.so/Project-SANABI-234894b38b1080baa682d16e91d4a5b1?source=copy_link    
# 영상
https://github.com/user-attachments/assets/f8c4c865-938c-41e9-b4fe-372d6d66b446
# 들어가기 앞서,
산나비라는 게임은 Unity Engine으로 만든 2D 플랫포머 로프액션 어드벤처 장르의 게임입니다, 원작의 스타일리쉬한 조작감을 살리는 것과 구조 설계에 최대한 집중했습니다.
# 기술 설명    
## 컴포넌트 기반 게임 엔진 아키텍처 (Component-Based Architecture)
모든 게임 오브젝트를 Actor로 데이터만 가지게 하고, 특정 기능을 담당하는 Component를 가지게 하여 재사용성과 결합도를 낮추어 유연한 객체 관리를 위함입니다.
- Actor라는 Base클래스를 만들어 공용으로 사용할 데이터들을 넣고, Actor를 상속받는 Player와 Enemy, Bullet 등등은 충돌을 담당하는 CollisionComponent, 물리를 담당하는 PhysicsComponent, 애니메이션 렌더링을 담당하는 SpriteRenderComponent 등등을 보유합니다.    
## 사슬팔과 물리 시스템
원작게임의 핵심 기술인 사슬팔(Grappling)시스템은 플레이어의 이동성을 극대화하고 역동적인 게임플레이를 제공합니다, 이 시스템은 단순한 이동이 아닌 물리 기반의 움직임으로 다양한 전략적 움직임을 가능하게 합니다.   
- GrapllingComponent로 사슬팔의 발사와 부착을 담당하고, PhysicsComponent로 사슬팔에 따른 상태에 따라 물리 움직임을 다르게 합니다.
- GrapplingHookProjectile 발사체 객체의 잦은 생성 및 삭제로 인해 **오브젝트 풀 패턴**을 사용하여 런타임 성능 저하를 방지하였습니다.
- GrapplingHookProjectile 발사체 객체가 충돌이 일어났을때, **CollisionComponent에 바인됭된 람다 함수**로 콜리전 채널에 따른 함수를 호출하게 하여 유연하고 확장 가능한 이벤트 기반 시스템을 구현했습니다.
- PhysicsComponent는 기본적인 이동부터 점프나 대쉬, 피격 등 모든 물리적 상호작용을 담당합니다.
- PhysicsComponent는 EPhysicsState에 따라 고유의 물리 업데이트 로직을 적용합니다, 이는 복잡한 움직임 패턴을 효과적으로 관리하고, 상태 전환에 따른 버그 발생을 줄이는데 핵심입니다.
- 사슬팔 부착 시, **접선 벡터와 각속도, 각가속도의 개념**을 도입한 **진자 운동 시뮬레이션**을 실행합니다.
<details>
  <summary>PhysicsComponent::UpdateGrapplePhysics() 코드</summary>
    
    void PhysicsComponent::UpdateGrapplePhysics(float deltaTime)
    {
    	    if (!owner) return;
    
    	Player* player = dynamic_cast<Player*>(owner);
    	if (!player) return;
    
    	player->GetArmRenderComponent()->SetPosition(player->GetPosition() + Vector(0, -20));
    
    	Vector currentProjectilePos = curPrjoectile->GetPosition();
    	Vector projectileMovement = currentProjectilePos - lastProjectilePosition;
    
    	Vector currentPos = owner->GetPosition() + projectileMovement;
    	owner->SetPosition(currentPos);
    
    	lastProjectilePosition = currentProjectilePos;
    
    	Vector toHookCurrent = currentPos - currentProjectilePos;
    	float currentDistance = toHookCurrent.Length(); // 0일때 예외처리
    
    	GrapplingComponent* grappleComponent = owner->GetComponent<GrapplingComponent>();
    	if (!grappleComponent) return;
    
    	float maxChainLength = grappleComponent->GetMaxChainLength();
    	
    	// 최대길이보다 길면 진자운동말고 당겨오기
    	if (currentDistance > maxChainLength)
    	{
    		grappleLength = maxChainLength;
    
    		float excessDistance = currentDistance - maxChainLength;
    		Vector pullDirection = (curPrjoectile->GetPosition() - currentPos).Normalized();
    
    		float pullStrength = 800.0f; 
    		Vector pullForce = pullDirection * pullStrength * excessDistance;
    
    		Vector currentVelocity = owner->GetVelocity();
    		currentVelocity += pullForce * deltaTime;
    
    		float dampingFactor = 0.98f;
    		currentVelocity *= dampingFactor;
    
    		owner->SetVelocity(currentVelocity);
    		
    		Vector newPosition = currentPos + currentVelocity * deltaTime;
    		owner->SetPosition(newPosition);
    
    		return;
    	}
    
    	grappleLength = currentDistance;
    
    	currentAngle = atan2f(toHookCurrent.y, toHookCurrent.x);
    
    	// 진자운동
    	float angularAccel = (gravity / grappleLength) * cosf(currentAngle) * swingGravityMult;
    
    	swingInputForce -= 50.0f;
    	swingInputForce = clamp(swingInputForce, 0.f, 250.f);
    
    	Vector direction = owner->GetDirection();
    	if (direction.x != 0)
    	{
    		angularAccel += -direction.x * (swingInputForce / grappleLength);
    
    		if ((direction.x > 0 && angularVelocity > 0) ||
    			(direction.x < 0 && angularVelocity < 0))
    		{
    			angularAccel *= timingBonusMult;
    		}
    	}
    
    	// 진자 운동 관성 추가 
    	if (abs(angularVelocity) > 0.1f)
    	{
    		float swingDirection = (angularVelocity > 0) ? 1.0f : -1.0f;
    		float momentumBoost = 0.2f; 
    		angularAccel += swingDirection * momentumBoost * (swingInputForce / grappleLength);
    	}
    
    	// 각속도 업데이트
    	angularVelocity += angularAccel * deltaTime;
    	angularVelocity *= swingDamping;  // 감쇠 
    
    	// 충돌 예상 
    	float nextAngle = currentAngle + angularVelocity * deltaTime;
    
    	// 새 위치 계산 (원형 운동)
    	Vector newPos;
    	newPos.x = curPrjoectile->GetPosition().x + grappleLength * cosf(currentAngle);
    	newPos.y = curPrjoectile->GetPosition().y + grappleLength * sinf(currentAngle);
    
    	bool willCollide = false;
    
    	if (bOverlapLeftWall || bOverlapRightWall || bOnGround || bOverlapCeiling)
    	{
    		willCollide = true;
    		angularVelocity = 0; // 충돌 방향으로의 회전 중지
    	}
    
    	if (!willCollide) // 충돌이 없으면
    	{
    		currentAngle += angularVelocity * deltaTime;
    
    		Vector newPos;
    		newPos.x = curPrjoectile->GetPosition().x + grappleLength * cosf(currentAngle);
    		newPos.y = curPrjoectile->GetPosition().y + grappleLength * sinf(currentAngle);
    
    		owner->SetPosition(newPos);
    	}
    
    	// 속도 계산, 해제 시 자연스러운 이동을 위함
    	float tangentialSpeed = angularVelocity * grappleLength;
    
    	Vector tangent(-sinf(currentAngle), cosf(currentAngle));
    	Vector velocity = tangent * tangentialSpeed;
    
    	owner->SetVelocity(velocity);
    }
</details>    

## 충돌 시스템
- AABB충돌 방식을 기반으로, Singleton구조의 CollisionManager클래스에서 모든 충돌체를 검사합니다.
- CollisionComponent는 Actor 또는 지형지물에 부착되어 있으며, **CollisionChannel**과 **OnComponentBeginOverlap, OnComponentEndOverlap**이라는 함수 포인터를 가지고 있습니다. CollisionChannel을 통해 다른 충돌체와의 충돌 여부를 효율적으로 설정할 수 있고, 함수 포인터를 활용하여 충돌 발생 시 실행될 기능을 자유롭게 정의함으로써 높은 다형성을 확보했습니다.
<details>
  <summary> 2차원배열 코드</summary>
    
    bool bIgnore[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
    bool bBlock[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
    bool bOverlap[(int)ECollisionChannel::Max][(int)ECollisionChannel::Max] = { 0 };
    
    bBlock[(int)ECollisionChannel::DeathTile][(int)ECollisionChannel::Character] = 1;
    bIgnore[(int)ECollisionChannel::Projectile][(int)ECollisionChannel::Projectile] = 1; 
</details>     

- CollisionManager에 HitResult 구조체를 만들며 반환하는 함수를 개발하며, 이 구조체를 통해 지면 감지, 벽 타기 등 다양한 상황에 맞는 로직을 실행할 수 있게합니다.
- CollisionChannel과 2차원 bool 배열을 통해 충돌 필터링 시스템을 구현하여 불필요한 충돌 연산을 줄였습니다, 언리얼 엔진의 충돌 시스템에서 영감을 받았고 매 프레임마다 모든 충돌체를 검사하기엔 효율성이 떨어질 것이라 생각하여 구현해보았습니다.
- 동적인 객체에 한해 다음 프레임 위치를 예측해 발생할 수 있는 충돌을 미리 감지하는 충돌 예측 기법을 통해 터널링 현상이나 객체 겹침 현상을 방지하였습니다, 기존에 충돌체를 보유한 객체끼리 겹치는 현상이 있어 이를 방지하고자 구현해보았습니다.
- 매 프레임마다 발생하는 콜백함수 호출로 인한 중복 이벤트 호출을 방지하기 위해, unordered_map자료구조를 사용해 정확한 상태 변화를 감지를 구현하였습니다.
<details>
  <summary>CollisionManager::Update() 코드</summary>
    
    void CollisionManager::Update()
    {
        ProcessPredictiveCollisions();
    	for (int i = 0; i < collisionComponents.size(); i++)
    	{
    		if (!collisionComponents[i]->IsActive()) continue;
    
    		for (int j = i + 1; j < collisionComponents.size(); j++)
    		{
    			if (!collisionComponents[j]->IsActive()) continue;
    
    			ECollisionChannel channel1 = collisionComponents[i]->GetCollisionChannel();
    			ECollisionChannel channel2 = collisionComponents[j]->GetCollisionChannel();
    
    			if (bIgnore[(int)channel1][(int)channel2] || !bBlock[(int)channel1][(int)channel2])
    				continue;
    
    			auto pair = make_pair(collisionComponents[i], collisionComponents[j]);
    			HitResult collisionInfo = CheckAABBCollision(collisionComponents[i], collisionComponents[j]);
    			bool wasColliding = collisionPairs[pair];
    
    			if (collisionInfo.isColliding && !wasColliding) 
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
    			else if (!collisionInfo.isColliding && wasColliding) 
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
</details>    

## 맵 에디터
다양한 레벨 디자인을 가능하게 하기 위해 커스텀 맵 에디터를 구현했습니다, 이 에디터에서는 게임 씬의 타일 및 건축물, 지형 충돌체와 적군스폰지점 등을 시각적으로 보여주며 저장/로드 기능을 지원합니다.     
- 서브윈도우 2개를 생성해 사용할 타일셋과 경로를 추적해 건축물 파일을 불러오게 합니다.     
- 메인 윈도우에 그리드를 그려 타일 배치를 위한 시각적인 가이드를 제공하고 서브 윈도우의 타일 팔레트에서 유저가 원하는 타일을 선택하고 메인 윈도우에 클릭 및 드래그로 배치할 수 있습니다.
- 맵 에디터에서 생성된 레벨 데이터를 파일 직렬화를 통해 영구적으로 저장하고 게임 씬에서 해당 데이터를 불러와 맵을 구성합니다.
- 타일뿐만 아니라 타일뿐만이 아니라 건축물과 지형 충돌체, 적군 스폰지점과 무빙 플랫폼도 에디터에서 배치할 수 있게해 확장성을 챙겼습니다.
   
https://github.com/user-attachments/assets/7e237f47-22fb-42b2-bcec-37487cec6ec7

https://github.com/user-attachments/assets/b9eae180-8472-433c-9c2b-7b059eac8ddb

- 맵 에디터를 통해 생성되고 커스텀 CollisionRect타입의 타일 충돌체는 기존 CollisionComponent 및 CollisionManager과 효과적으로 연동되기 위해, **어댑터 패턴**을 사용하여 기존 시스템을 변경하지 않고 활용할 수 있게하였습니다. 이는 시스템간의 낮은 결합도를 유지하며 기능을 확장하는데 기여했습니다.     

# 트러블 슈팅   
## 렌더링 최적화 사례    
- 타일맵 로딩 시 프레임 드롭(60FPS 이하) 문제가 발생했습니다. 성능 프로파일러를 통해 타일 렌더링 함수의 높은 CPU 점유율을 확인했고, 이를 해결하기 위해 다음과 같은 최적화 기법을 적용했습니다.
<img width="1614" height="752" alt="image (1)" src="https://github.com/user-attachments/assets/41fe108b-5e7b-49f1-a95c-872f6c32419c" />

1. 타일맵 컬링(Culling) 적용: 화면에 보이는 타일만 렌더링하도록 하여 CPU 점유율을 4% 감소시켰습니다.
2. 정적 요소 캐싱: 배경, 타일맵, 건축물과 같은 정적인 요소들은 매 프레임 Render 함수에서 그리는 대신, Init 함수에서 별도의 HDC에 한 번만 미리 렌더링하여 캐싱했습니다.
3. 최적화된 WinAPI 함수 사용: 투명 처리나 크기 조절이 필요 없는 상황에서는 성능이 느린 TransparentBlt 대신 BitBlt 함수를 사용하여 렌더링 효율을 극대화했습니다.

- 그 결과, 프레임을 안정적(평균 100프레임)으로 유지시켰습니다.
