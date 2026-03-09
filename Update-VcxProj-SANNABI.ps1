# ============================================================
# Project-SANNABI .vcxproj 파일 경로 수정 스크립트
# ============================================================
# 폴더 정리 후 실행하세요!
# ============================================================

param(
    [string]$ProjectPath = "."
)

Set-Location $ProjectPath

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Visual Studio 프로젝트 파일 경로 수정" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$vcxprojFile = "SANNABI.vcxproj"
$filtersFile = "SANNABI.vcxproj.filters"

if (!(Test-Path $vcxprojFile)) {
    Write-Host "오류: $vcxprojFile 파일을 찾을 수 없습니다!" -ForegroundColor Red
    exit 1
}

# 파일 경로 매핑
$pathMap = @{
    # Core
    "Engine.cpp" = "Source\Core\Engine.cpp"
    "Engine.h" = "Source\Core\Engine.h"
    "Singleton.cpp" = "Source\Core\Singleton.cpp"
    "Singleton.h" = "Source\Core\Singleton.h"
    "Defines.cpp" = "Source\Core\Defines.cpp"
    "Defines.h" = "Source\Core\Defines.h"
    "pch.cpp" = "Source\Core\pch.cpp"
    "pch.h" = "Source\Core\pch.h"
    "framework.h" = "Source\Core\framework.h"
    "targetver.h" = "Source\Core\targetver.h"

    # Managers
    "CameraManager.cpp" = "Source\Managers\CameraManager.cpp"
    "CameraManager.h" = "Source\Managers\CameraManager.h"
    "CollisionManager.cpp" = "Source\Managers\CollisionManager.cpp"
    "CollisionManager.h" = "Source\Managers\CollisionManager.h"
    "InputManager.cpp" = "Source\Managers\InputManager.cpp"
    "InputManager.h" = "Source\Managers\InputManager.h"
    "ResourceManager.cpp" = "Source\Managers\ResourceManager.cpp"
    "ResourceManager.h" = "Source\Managers\ResourceManager.h"
    "SceneManager.cpp" = "Source\Managers\SceneManager.cpp"
    "SceneManager.h" = "Source\Managers\SceneManager.h"
    "SoundManager.cpp" = "Source\Managers\SoundManager.cpp"
    "SoundManager.h" = "Source\Managers\SoundManager.h"
    "TimerManager.cpp" = "Source\Managers\TimerManager.cpp"
    "TimerManager.h" = "Source\Managers\TimerManager.h"
    "VFXManager.cpp" = "Source\Managers\VFXManager.cpp"
    "VFXManager.h" = "Source\Managers\VFXManager.h"

    # Components
    "Component.cpp" = "Source\Components\Component.cpp"
    "Component.h" = "Source\Components\Component.h"
    "CollisionComponent.cpp" = "Source\Components\CollisionComponent.cpp"
    "CollisionComponent.h" = "Source\Components\CollisionComponent.h"
    "PhysicsComponent.cpp" = "Source\Components\PhysicsComponent.cpp"
    "PhysicsComponent.h" = "Source\Components\PhysicsComponent.h"
    "GrapplingComponent.cpp" = "Source\Components\GrapplingComponent.cpp"
    "GrapplingComponent.h" = "Source\Components\GrapplingComponent.h"
    "SpriteRenderComponent.cpp" = "Source\Components\SpriteRenderComponent.cpp"
    "SpriteRenderComponent.h" = "Source\Components\SpriteRenderComponent.h"

    # Actors
    "Actor.cpp" = "Source\Actors\Actor.cpp"
    "Actor.h" = "Source\Actors\Actor.h"
    "Player.cpp" = "Source\Actors\Player.cpp"
    "Player.h" = "Source\Actors\Player.h"
    "PlayerController.cpp" = "Source\Actors\PlayerController.cpp"
    "PlayerController.h" = "Source\Actors\PlayerController.h"
    "Enemy.cpp" = "Source\Actors\Enemy.cpp"
    "Enemy.h" = "Source\Actors\Enemy.h"
    "Mari.cpp" = "Source\Actors\Mari.cpp"
    "Mari.h" = "Source\Actors\Mari.h"
    "Turret.cpp" = "Source\Actors\Turret.cpp"
    "Turret.h" = "Source\Actors\Turret.h"
    "Platform.cpp" = "Source\Actors\Platform.cpp"
    "Platform.h" = "Source\Actors\Platform.h"
    "TestGround.cpp" = "Source\Actors\TestGround.cpp"
    "TestGround.h" = "Source\Actors\TestGround.h"

    # Projectiles
    "Bullet.cpp" = "Source\Projectiles\Bullet.cpp"
    "Bullet.h" = "Source\Projectiles\Bullet.h"
    "BulletPool.cpp" = "Source\Projectiles\BulletPool.cpp"
    "BulletPool.h" = "Source\Projectiles\BulletPool.h"
    "GrapplingHookProjectile.cpp" = "Source\Projectiles\GrapplingHookProjectile.cpp"
    "GrapplingHookProjectile.h" = "Source\Projectiles\GrapplingHookProjectile.h"
    "GrapplingHookProjectilePool.cpp" = "Source\Projectiles\GrapplingHookProjectilePool.cpp"
    "GrapplingHookProjectilePool.h" = "Source\Projectiles\GrapplingHookProjectilePool.h"

    # Scenes
    "Scene.cpp" = "Source\Scenes\Scene.cpp"
    "Scene.h" = "Source\Scenes\Scene.h"
    "GameScene.cpp" = "Source\Scenes\GameScene.cpp"
    "GameScene.h" = "Source\Scenes\GameScene.h"
    "LobbyScene.cpp" = "Source\Scenes\LobbyScene.cpp"
    "LobbyScene.h" = "Source\Scenes\LobbyScene.h"
    "EditorScene.cpp" = "Source\Scenes\EditorScene.cpp"
    "EditorScene.h" = "Source\Scenes\EditorScene.h"

    # Animation
    "Animator.cpp" = "Source\Animation\Animator.cpp"
    "Animator.h" = "Source\Animation\Animator.h"
    "SpriteAnimation.cpp" = "Source\Animation\SpriteAnimation.cpp"
    "SpriteAnimation.h" = "Source\Animation\SpriteAnimation.h"

    # TileMap
    "TileMap.cpp" = "Source\TileMap\TileMap.cpp"
    "TileMap.h" = "Source\TileMap\TileMap.h"
    "TileCollisionAdapter.cpp" = "Source\TileMap\TileCollisionAdapter.cpp"
    "TileCollisionAdapter.h" = "Source\TileMap\TileCollisionAdapter.h"

    # Editor
    "BuildingEditor.cpp" = "Source\Editor\BuildingEditor.cpp"
    "BuildingEditor.h" = "Source\Editor\BuildingEditor.h"
    "TileEditor.cpp" = "Source\Editor\TileEditor.cpp"
    "TileEditor.h" = "Source\Editor\TileEditor.h"

    # UI
    "Button.cpp" = "Source\UI\Button.cpp"
    "Button.h" = "Source\UI\Button.h"

    # Effects
    "VFX.cpp" = "Source\Effects\VFX.cpp"
    "VFX.h" = "Source\Effects\VFX.h"

    # Resources
    "TextureResource.cpp" = "Source\Resources\TextureResource.cpp"
    "TextureResource.h" = "Source\Resources\TextureResource.h"

    # External
    "json_fwd.hpp" = "Source\External\json_fwd.hpp"
}

# vcxproj 파일 수정
Write-Host "1. $vcxprojFile 수정 중..." -ForegroundColor Yellow

$content = Get-Content -Path $vcxprojFile -Raw
$replacements = 0

foreach ($old in $pathMap.Keys) {
    $new = $pathMap[$old]

    # <ClCompile Include="파일명" /> 패턴
    $pattern1 = "Include=`"$old`""
    $replace1 = "Include=`"$new`""

    if ($content -match [regex]::Escape($pattern1)) {
        $content = $content -replace [regex]::Escape($pattern1), $replace1
        $replacements++
    }
}

Set-Content -Path $vcxprojFile -Value $content -NoNewline
Write-Host "  $replacements개 경로 수정됨" -ForegroundColor Green

# vcxproj.filters 파일 수정 (존재하는 경우)
if (Test-Path $filtersFile) {
    Write-Host ""
    Write-Host "2. $filtersFile 수정 중..." -ForegroundColor Yellow

    $filterContent = Get-Content -Path $filtersFile -Raw
    $filterReplacements = 0

    foreach ($old in $pathMap.Keys) {
        $new = $pathMap[$old]
        $pattern = "Include=`"$old`""
        $replace = "Include=`"$new`""

        if ($filterContent -match [regex]::Escape($pattern)) {
            $filterContent = $filterContent -replace [regex]::Escape($pattern), $replace
            $filterReplacements++
        }
    }

    Set-Content -Path $filtersFile -Value $filterContent -NoNewline
    Write-Host "  $filterReplacements개 경로 수정됨" -ForegroundColor Green
}

# Include 디렉토리 추가 안내
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 완료!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "추가 설정 필요:" -ForegroundColor Yellow
Write-Host ""
Write-Host "Visual Studio에서 프로젝트 속성 -> C/C++ -> 일반 -> 추가 포함 디렉터리에" -ForegroundColor White
Write-Host "다음을 추가하세요:" -ForegroundColor White
Write-Host ""
Write-Host "  $(Get-Location)\Source" -ForegroundColor Cyan
Write-Host ""
Write-Host "또는 .vcxproj 파일의 <ItemDefinitionGroup>에 다음을 추가:" -ForegroundColor White
Write-Host ""
Write-Host '  <AdditionalIncludeDirectories>$(ProjectDir)Source;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>' -ForegroundColor Cyan
Write-Host ""
