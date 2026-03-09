# ============================================================
# Project-SANNABI #include 경로 자동 수정 스크립트
# ============================================================
# 폴더 정리 후 실행하세요!
# ============================================================

param(
    [string]$ProjectPath = "."
)

Set-Location $ProjectPath

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " #include 경로 자동 수정" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# include 경로 매핑 (기존 파일명 -> 새 경로)
$includeMap = @{
    # Core
    '"Engine.h"' = '"Core/Engine.h"'
    '"Singleton.h"' = '"Core/Singleton.h"'
    '"Defines.h"' = '"Core/Defines.h"'
    '"pch.h"' = '"Core/pch.h"'
    '"framework.h"' = '"Core/framework.h"'
    '"targetver.h"' = '"Core/targetver.h"'

    # Managers
    '"CameraManager.h"' = '"Managers/CameraManager.h"'
    '"CollisionManager.h"' = '"Managers/CollisionManager.h"'
    '"InputManager.h"' = '"Managers/InputManager.h"'
    '"ResourceManager.h"' = '"Managers/ResourceManager.h"'
    '"SceneManager.h"' = '"Managers/SceneManager.h"'
    '"SoundManager.h"' = '"Managers/SoundManager.h"'
    '"TimerManager.h"' = '"Managers/TimerManager.h"'
    '"VFXManager.h"' = '"Managers/VFXManager.h"'

    # Components
    '"Component.h"' = '"Components/Component.h"'
    '"CollisionComponent.h"' = '"Components/CollisionComponent.h"'
    '"PhysicsComponent.h"' = '"Components/PhysicsComponent.h"'
    '"GrapplingComponent.h"' = '"Components/GrapplingComponent.h"'
    '"SpriteRenderComponent.h"' = '"Components/SpriteRenderComponent.h"'

    # Actors
    '"Actor.h"' = '"Actors/Actor.h"'
    '"Player.h"' = '"Actors/Player.h"'
    '"PlayerController.h"' = '"Actors/PlayerController.h"'
    '"Enemy.h"' = '"Actors/Enemy.h"'
    '"Mari.h"' = '"Actors/Mari.h"'
    '"Turret.h"' = '"Actors/Turret.h"'
    '"Platform.h"' = '"Actors/Platform.h"'
    '"TestGround.h"' = '"Actors/TestGround.h"'

    # Projectiles
    '"Bullet.h"' = '"Projectiles/Bullet.h"'
    '"BulletPool.h"' = '"Projectiles/BulletPool.h"'
    '"GrapplingHookProjectile.h"' = '"Projectiles/GrapplingHookProjectile.h"'
    '"GrapplingHookProjectilePool.h"' = '"Projectiles/GrapplingHookProjectilePool.h"'

    # Scenes
    '"Scene.h"' = '"Scenes/Scene.h"'
    '"GameScene.h"' = '"Scenes/GameScene.h"'
    '"LobbyScene.h"' = '"Scenes/LobbyScene.h"'
    '"EditorScene.h"' = '"Scenes/EditorScene.h"'

    # Animation
    '"Animator.h"' = '"Animation/Animator.h"'
    '"SpriteAnimation.h"' = '"Animation/SpriteAnimation.h"'

    # TileMap
    '"TileMap.h"' = '"TileMap/TileMap.h"'
    '"TileCollisionAdapter.h"' = '"TileMap/TileCollisionAdapter.h"'

    # Editor
    '"BuildingEditor.h"' = '"Editor/BuildingEditor.h"'
    '"TileEditor.h"' = '"Editor/TileEditor.h"'

    # UI
    '"Button.h"' = '"UI/Button.h"'

    # Effects
    '"VFX.h"' = '"Effects/VFX.h"'

    # Resources
    '"TextureResource.h"' = '"Resources/TextureResource.h"'

    # External
    '"json_fwd.hpp"' = '"External/json_fwd.hpp"'
}

# 모든 .cpp, .h 파일 찾기
$sourceFiles = Get-ChildItem -Path "Source" -Include "*.cpp", "*.h" -Recurse
$rootFiles = Get-ChildItem -Path "." -Include "*.cpp", "*.h" -File

$allFiles = @()
$allFiles += $sourceFiles
$allFiles += $rootFiles

Write-Host "총 $($allFiles.Count)개 파일 검사 중..." -ForegroundColor Yellow
Write-Host ""

$totalReplacements = 0

foreach ($file in $allFiles) {
    $content = Get-Content -Path $file.FullName -Raw -ErrorAction SilentlyContinue
    if (!$content) { continue }

    $originalContent = $content
    $fileReplacements = 0

    foreach ($old in $includeMap.Keys) {
        $new = $includeMap[$old]
        if ($content -match [regex]::Escape($old)) {
            $content = $content -replace [regex]::Escape($old), $new
            $fileReplacements++
        }
    }

    if ($fileReplacements -gt 0) {
        Set-Content -Path $file.FullName -Value $content -NoNewline
        Write-Host "  $($file.Name): $fileReplacements개 수정" -ForegroundColor Green
        $totalReplacements += $fileReplacements
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 완료! 총 $totalReplacements개 #include 경로 수정됨" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
