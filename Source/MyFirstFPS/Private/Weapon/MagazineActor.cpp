#include "Weapon/MagazineActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

AMagazineActor::AMagazineActor() {
  PrimaryActorTick.bCanEverTick = false;

  MeshComponent =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
  RootComponent = MeshComponent;

  // 禁用导航影响（避免角色移动组件扫描到）
  MeshComponent->SetCanEverAffectNavigation(false);
}

// 激活
void AMagazineActor::Activate() {
  SetActorHiddenInGame(false);
  MeshComponent->SetVisibility(true);

  // 禁用物理但保持附着
  DisablePhysics();
}

// 停用
void AMagazineActor::Deactivate() {
  SetActorHiddenInGame(true);
  MeshComponent->SetVisibility(false);

  // 归还池时，彻底解除附着并销毁约束
  SetCollisionDisabled();
}

// 启用物理
void AMagazineActor::EnablePhysics() {
  // 显示弹匣
  SetActorHiddenInGame(false);
  MeshComponent->SetVisibility(true);

  // 解除附着
  DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  TArray<UPrimitiveComponent *> Prims;
  GetComponents<UPrimitiveComponent>(Prims);
  for (UPrimitiveComponent *P : Prims) {
    if (!P)
      continue;

    // 设置碰撞模式
    P->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    P->SetCollisionResponseToAllChannels(ECR_Block);
    P->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    P->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);

    // 显式指定碰撞预设
    P->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    // 重力设置
    P->SetEnableGravity(true);

    // 导航和重叠
    P->SetCanEverAffectNavigation(true);
    P->SetGenerateOverlapEvents(true);
    P->SetComponentTickEnabled(true);

    // 重建物理体
    P->RecreatePhysicsState();
  }

  //  启用物理模拟
  MeshComponent->SetSimulatePhysics(true);
  MeshComponent->WakeRigidBody();

  // 全局 Actor 碰撞开启
  SetActorEnableCollision(true);

  // 施加冲量
  MeshComponent->AddImpulse(FMath::VRand() * 100.0f);
}

// 禁用物理
void AMagazineActor::DisablePhysics() {
  SetPhysicsDisabledInternal(false, false);
}

// 恢复碰撞
void AMagazineActor::SetCollisionEnabled() {
  //  恢复 Actor 总开关
  SetActorEnableCollision(true);

  // 遍历所有 Primitive 组件
  TArray<UPrimitiveComponent *> Prims;
  GetComponents<UPrimitiveComponent>(Prims);
  for (UPrimitiveComponent *P : Prims) {
    if (!P)
      continue;

    // 恢复碰撞预设
    P->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    // 恢复导航影响（如果希望弹夹影响导航则开启）
    P->SetCanEverAffectNavigation(true);

    // 允许重叠事件
    P->SetGenerateOverlapEvents(true);

    // 重新创建物理状态
    P->RecreatePhysicsState();
  }
}

// 彻底关闭碰撞
void AMagazineActor::SetCollisionDisabled() {
  SetPhysicsDisabledInternal(true, true);
}

void AMagazineActor::BeginPlay() { Super::BeginPlay(); }

// 辅助函数
void AMagazineActor::SetPhysicsDisabledInternal(bool bDetachFromActor,
                                                bool bDestroyConstraints) {
  // 解除附着
  if (bDetachFromActor) {
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
  }

  // 遍历所有 Primitive 组件统一禁用
  TArray<UPrimitiveComponent *> Prims;
  GetComponents<UPrimitiveComponent>(Prims);
  for (UPrimitiveComponent *P : Prims) {
    if (!P)
      continue;

    // 停止物理模拟并强制睡眠
    P->SetSimulatePhysics(false);
    P->PutRigidBodyToSleep();

    // 彻底禁用碰撞
    P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    P->SetCollisionResponseToAllChannels(ECR_Ignore);
    P->SetCollisionProfileName(TEXT("NoCollision"));

    // 禁用导航影响与重叠事件
    P->SetCanEverAffectNavigation(false);
    P->SetGenerateOverlapEvents(false);

    // 销毁物理状态
    P->DestroyPhysicsState();

    // 禁用组件 Tick
    P->SetComponentTickEnabled(false);
  }

  // 全局 Actor 碰撞开关
  SetActorEnableCollision(false);

  // 销毁物理约束
  if (bDestroyConstraints) {
    UPhysicsConstraintComponent *Constraint =
        FindComponentByClass<UPhysicsConstraintComponent>();
    if (Constraint) {
      Constraint->BreakConstraint();
      Constraint->DestroyComponent();
    }
  }
}
