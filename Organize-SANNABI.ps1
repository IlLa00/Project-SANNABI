# ============================================================
# Project-SANNABI 폴더 구조 정리 스크립트
# ============================================================
# 사용법:
#   1. 이 스크립트를 Project-SANNABI 폴더에 복사
#   2. PowerShell에서 실행: .\Organize-SANNABI.ps1
# ============================================================

param(
    [string]$ProjectPath = "."
)

# 프로젝트 경로로 이동
Set-Location $ProjectPath

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Project-SANNABI 폴더 구조 정리 시작" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ============================================================
# 1. 폴더 생성
# ============================================================
$folders = @(
    "Source/Core",
    "Source/Managers",
    "Source/Components",
    "Source/Actors",
    "Source/Projectiles",
    "Source/Scenes",
    "Source/Animation",
    "Source/TileMap",
    "Source/Editor",
    "Source/UI",
    "Source/Effects",
    "Source/Resources",
    "Source/External",
    "Assets/Fonts",
    "Assets/Levels",
    "Assets/Sounds",
    "Assets/Sprites",
    "Assets/TileMaps",
    "Assets/Lobby"
)

Write-Host "[1/4] 폴더 생성 중..." -ForegroundColor Yellow
foreach ($folder in $folders) {
    if (!(Test-Path $folder)) {
        New-Item -ItemType Directory -Path $folder -Force | Out-Null
        Write-Host "  + $folder" -ForegroundColor Green
    }
}
Write-Host ""

# ============================================================
# 2. 파일 이동 매핑 정의
# ============================================================
$fileMappings = @{
    # Core - 엔진 핵심
    "Source/Core" = @(
        "Engine.cpp", "Engine.h",
        "Singleton.cpp", "Singleton.h",
        "Defines.cpp", "Defines.h",
        "pch.cpp", "pch.h",
        "framework.h",
        "targetver.h"
    )

    # Managers - 싱글톤 매니저
    "Source/Managers" = @(
        "CameraManager.cpp", "CameraManager.h",
        "CollisionManager.cpp", "CollisionManager.h",
        "InputManager.cpp", "InputManager.h",
        "ResourceManager.cpp", "ResourceManager.h",
        "SceneManager.cpp", "SceneManager.h",
        "SoundManager.cpp", "SoundManager.h",
        "TimerManager.cpp", "TimerManager.h",
        "VFXManager.cpp", "VFXManager.h"
    )

    # Components - 컴포넌트 시스템
    "Source/Components" = @(
        "Component.cpp", "Component.h",
        "CollisionComponent.cpp", "CollisionComponent.h",
        "PhysicsComponent.cpp", "PhysicsComponent.h",
        "GrapplingComponent.cpp", "GrapplingComponent.h",
        "SpriteRenderComponent.cpp", "SpriteRenderComponent.h"
    )

    # Actors - 게임 오브젝트
    "Source/Actors" = @(
        "Actor.cpp", "Actor.h",
        "Player.cpp", "Player.h",
        "PlayerController.cpp", "PlayerController.h",
        "Enemy.cpp", "Enemy.h",
        "Mari.cpp", "Mari.h",
        "Turret.cpp", "Turret.h",
        "Platform.cpp", "Platform.h",
        "TestGround.cpp", "TestGround.h"
    )

    # Projectiles - 투사체 & 오브젝트 풀
    "Source/Projectiles" = @(
        "Bullet.cpp", "Bullet.h",
        "BulletPool.cpp", "BulletPool.h",
        "GrapplingHookProjectile.cpp", "GrapplingHookProjectile.h",
        "GrapplingHookProjectilePool.cpp", "GrapplingHookProjectilePool.h"
    )

    # Scenes - 씬
    "Source/Scenes" = @(
        "Scene.cpp", "Scene.h",
        "GameScene.cpp", "GameScene.h",
        "LobbyScene.cpp", "LobbyScene.h",
        "EditorScene.cpp", "EditorScene.h"
    )

    # Animation - 애니메이션
    "Source/Animation" = @(
        "Animator.cpp", "Animator.h",
        "SpriteAnimation.cpp", "SpriteAnimation.h"
    )

    # TileMap - 타일맵 시스템
    "Source/TileMap" = @(
        "TileMap.cpp", "TileMap.h",
        "TileCollisionAdapter.cpp", "TileCollisionAdapter.h"
    )

    # Editor - 에디터 도구
    "Source/Editor" = @(
        "BuildingEditor.cpp", "BuildingEditor.h",
        "TileEditor.cpp", "TileEditor.h"
    )

    # UI
    "Source/UI" = @(
        "Button.cpp", "Button.h"
    )

    # Effects - 이펙트
    "Source/Effects" = @(
        "VFX.cpp", "VFX.h"
    )

    # Resources - 리소스 관리
    "Source/Resources" = @(
        "TextureResource.cpp", "TextureResource.h"
    )

    # External - 외부 라이브러리
    "Source/External" = @(
        "json_fwd.hpp"
    )
}

# 에셋 폴더 이동 매핑
$assetMappings = @{
    "Font" = "Assets/Fonts"
    "Level" = "Assets/Levels"
    "Sound" = "Assets/Sounds"
    "Sprite" = "Assets/Sprites"
    "TileMap" = "Assets/TileMaps"
    "Lobby" = "Assets/Lobby"
}

# ============================================================
# 3. 소스 파일 이동
# ============================================================
Write-Host "[2/4] 소스 파일 이동 중..." -ForegroundColor Yellow

$movedCount = 0
$skippedCount = 0

foreach ($destination in $fileMappings.Keys) {
    foreach ($file in $fileMappings[$destination]) {
        if (Test-Path $file) {
            Move-Item -Path $file -Destination $destination -Force
            Write-Host "  $file -> $destination" -ForegroundColor Green
            $movedCount++
        } else {
            $skippedCount++
        }
    }
}

Write-Host "  이동: $movedCount 파일, 스킵: $skippedCount 파일" -ForegroundColor Cyan
Write-Host ""

# ============================================================
# 4. 에셋 폴더 이동
# ============================================================
Write-Host "[3/4] 에셋 폴더 이동 중..." -ForegroundColor Yellow

foreach ($source in $assetMappings.Keys) {
    $destination = $assetMappings[$source]
    if (Test-Path $source) {
        # 폴더 내용물 이동
        Get-ChildItem -Path $source | Move-Item -Destination $destination -Force
        # 빈 폴더 삭제
        Remove-Item -Path $source -Force -ErrorAction SilentlyContinue
        Write-Host "  $source/* -> $destination" -ForegroundColor Green
    }
}

Write-Host ""

# ============================================================
# 5. 불필요한 파일/폴더 정리
# ============================================================
Write-Host "[4/4] 불필요한 파일 정리 중..." -ForegroundColor Yellow

$cleanupItems = @(
    ".vs",           # Visual Studio 캐시
    "RCa17176",      # 빌드 아티팩트
    "SANNABI"        # 중복 폴더가 있다면
)

foreach ($item in $cleanupItems) {
    if (Test-Path $item) {
        Remove-Item -Path $item -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "  - $item 삭제됨" -ForegroundColor Red
    }
}

Write-Host ""

# ============================================================
# 6. 결과 출력
# ============================================================
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 정리 완료!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "최종 폴더 구조:" -ForegroundColor Yellow
Write-Host ""

# 트리 구조 출력
function Show-Tree {
    param([string]$Path, [string]$Indent = "")

    $items = Get-ChildItem -Path $Path -ErrorAction SilentlyContinue |
             Where-Object { $_.Name -notmatch "^\." } |
             Sort-Object { $_.PSIsContainer } -Descending

    $count = $items.Count
    $index = 0

    foreach ($item in $items) {
        $index++
        $isLast = ($index -eq $count)
        $prefix = if ($isLast) { "└── " } else { "├── " }
        $childIndent = if ($isLast) { "    " } else { "│   " }

        if ($item.PSIsContainer) {
            Write-Host "$Indent$prefix$($item.Name)/" -ForegroundColor Blue
            if ($item.Name -eq "Source" -or $item.Name -eq "Assets") {
                Show-Tree -Path $item.FullName -Indent "$Indent$childIndent"
            }
        } else {
            Write-Host "$Indent$prefix$($item.Name)" -ForegroundColor White
        }
    }
}

Show-Tree -Path "."

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 주의사항" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. Visual Studio 프로젝트 파일(.vcxproj)의 경로를 업데이트해야 합니다." -ForegroundColor Yellow
Write-Host "2. #include 경로를 새 폴더 구조에 맞게 수정해야 합니다." -ForegroundColor Yellow
Write-Host "3. 변경사항을 git에 커밋하세요:" -ForegroundColor Yellow
Write-Host "   git add -A" -ForegroundColor White
Write-Host "   git commit -m `"refactor: 폴더 구조 정리`"" -ForegroundColor White
Write-Host ""
