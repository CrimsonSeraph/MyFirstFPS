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

  // 初始状态：无碰撞，隐藏
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
  MeshComponent->SetSimulatePhysics(false);
  MeshComponent->SetHiddenInGame(true);
  MeshComponent->SetVisibility(false);

  // 禁用导航影响（避免角色移动组件扫描到）
  MeshComponent->SetCanEverAffectNavigation(false);
}

// 激活
void AMagazineActor::Activate() {
  SetActorHiddenInGame(false);
  MeshComponent->SetVisibility(true);

  // 彻底关闭碰撞（包括解除附着和销毁物理）
  SetCollisionDisabled();
}

// 停用
void AMagazineActor::Deactivate() {
  SetActorHiddenInGame(true);
  MeshComponent->SetVisibility(false);

  SetCollisionDisabled();
}

// 启用物理
void AMagazineActor::EnablePhysics() {
  SetActorHiddenInGame(false);
  MeshComponent->SetVisibility(true);

  TArray<UPrimitiveComponent *> Prims;
  GetComponents<UPrimitiveComponent>(Prims);
  for (UPrimitiveComponent *P : Prims) {
    // 启用碰撞
    P->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    // 对 Pawn 忽略
    P->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    P->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
    // 对世界保留阻挡
    P->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    P->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    P->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);

    // 只有主网格体模拟物理
    if (P == MeshComponent) {
      P->SetSimulatePhysics(true);
    } else {
      P->SetSimulatePhysics(false);
    }

    // 恢复导航影响
    P->SetCanEverAffectNavigation(true);

    // 重建物理状态
    P->RecreatePhysicsState();
  }
  SetActorEnableCollision(true);

  // 施加冲量
  MeshComponent->AddImpulse(FMath::VRand() * 100.0f);
}

// 禁用物理
void AMagazineActor::DisablePhysics() { SetCollisionDisabled(); }

// 彻底关闭碰撞
void AMagazineActor::SetCollisionDisabled() {
  // 解除附着
  DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  // 遍历所有 Primitive 组件
  TArray<UPrimitiveComponent *> Prims;
  GetComponents<UPrimitiveComponent>(Prims);
  for (UPrimitiveComponent *P : Prims) {
    if (!P)
      continue;

    // 停止物理模拟并强制睡眠
    P->SetSimulatePhysics(false);
    P->PutRigidBodyToSleep();

    // 禁用碰撞标志
    P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    P->SetCollisionResponseToAllChannels(ECR_Ignore);
    P->SetCollisionProfileName(TEXT("NoCollision"));

    // 禁用导航影响
    P->SetCanEverAffectNavigation(false);

    // 彻底销毁物理状态
    P->DestroyPhysicsState();

    //  禁止生成重叠事件
    P->SetGenerateOverlapEvents(false);
  }

  // 全局 Actor 级别禁用碰撞
  SetActorEnableCollision(false);

  // 禁用组件 Tick
  MeshComponent->SetComponentTickEnabled(false);

  // 如果有物理约束，也一并销毁
  UPhysicsConstraintComponent *Constraint =
      FindComponentByClass<UPhysicsConstraintComponent>();
  if (Constraint) {
    Constraint->BreakConstraint();
    Constraint->DestroyComponent();
  }
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

void AMagazineActor::BeginPlay() { Super::BeginPlay(); }
