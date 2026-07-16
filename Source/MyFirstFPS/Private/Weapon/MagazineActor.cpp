#include "Weapon/MagazineActor.h"

#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

AMagazineActor::AMagazineActor() {
  PrimaryActorTick.bCanEverTick = false; // 不需要 tick

  // 创建静态网格组件并设为根
  MeshComponent =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
  RootComponent = MeshComponent;

  // 初始状态：不模拟物理，仅用于查询碰撞，并默认隐藏
  MeshComponent->SetSimulatePhysics(false);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

  MeshComponent->SetHiddenInGame(true);
  MeshComponent->SetVisibility(false);
}

void AMagazineActor::BeginPlay() { Super::BeginPlay(); }

// 激活弹匣：显示、开启查询+物理碰撞（但不模拟物理），并解除依附关系（以防之前被附着）
void AMagazineActor::Activate() {
  SetActorHiddenInGame(false);
  MeshComponent->SetVisibility(true);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  MeshComponent->SetSimulatePhysics(false);
  MeshComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
  MeshComponent->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
  DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

// 停用弹匣：隐藏、关闭碰撞，并解除附着
void AMagazineActor::Deactivate() {
  SetActorHiddenInGame(true);
  MeshComponent->SetVisibility(false);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  MeshComponent->SetSimulatePhysics(false);
  DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

// 启用物理：显示弹匣，开启完整物理，施加随机冲量（模拟丢弃飞出）
void AMagazineActor::EnablePhysics() {
  SetActorHiddenInGame(false);
  MeshComponent->SetVisibility(true);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  MeshComponent->SetSimulatePhysics(true);
  MeshComponent->AddImpulse(FMath::VRand() * 100.0f);
}

// 禁用物理：关闭模拟，恢复为仅查询碰撞（用于固定到武器）
void AMagazineActor::DisablePhysics() {
  MeshComponent->SetSimulatePhysics(false);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
