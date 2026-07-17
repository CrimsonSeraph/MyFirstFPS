#include "Weapon/ReloadComponent.h"
#include "Weapon/MagazineActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UReloadComponent::UReloadComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UReloadComponent::SetMagazineClassAndInitPool(
    TSubclassOf<AMagazineActor> NewMagazineClass) {
  MagazineClass = NewMagazineClass;
  // 清空旧池
  for (AMagazineActor *Mag : MagazinePool) {
    if (IsValid(Mag))
      Mag->Destroy();
  }
  MagazinePool.Empty();
  // 重新初始化
  InitPool();
}

// 抓取新弹匣：先从池获取，然后附着到握持插槽，并记录为 CurrentHandMag
AMagazineActor *
UReloadComponent::SpawnAndGrabMag(TSubclassOf<AMagazineActor> MagClass) {
  if (CurrentHandMag) {
    UE_LOG(LogTemp, Warning,
           TEXT("ReloadComponent: Already holding a mag, dropping it."));
    DropCurrentMag();
  }

  TSubclassOf<AMagazineActor> UseClass = MagClass ? MagClass : MagazineClass;
  if (!UseClass) {
    UE_LOG(LogTemp, Error,
           TEXT("ReloadComponent: No magazine class specified!"));
    return nullptr;
  }

  AMagazineActor *NewMag = GetMagFromPool();
  if (!IsValid(NewMag))
    return nullptr;

  AActor *Owner = GetOwner();
  if (!Owner) {
    ReturnMagToPool(NewMag);
    return nullptr;
  }

  // 查找拥有者的骨骼网格体组件，并附着到手部插槽
  USkeletalMeshComponent *SkeletalMesh =
      Owner->FindComponentByClass<USkeletalMeshComponent>();
  if (SkeletalMesh && SkeletalMesh->DoesSocketExist(HandSocketName)) {
    NewMag->AttachToComponent(
        SkeletalMesh, FAttachmentTransformRules::SnapToTargetIncludingScale,
        HandSocketName);
  } else {
    UE_LOG(LogTemp, Error, TEXT("ReloadComponent: Hand socket '%s' not found!"),
           *HandSocketName.ToString());
    ReturnMagToPool(NewMag);
    return nullptr;
  }

  CurrentHandMag = NewMag;
  return NewMag;
}

// 将手中弹匣转移到武器插槽（结束换弹过程）
void UReloadComponent::TransferMagToWeapon() {
  if (!IsValid(CurrentHandMag)) {
    UE_LOG(LogTemp, Warning,
           TEXT("ReloadComponent: No magazine in hand to transfer."));
    return;
  }

  AActor *Owner = GetOwner();
  if (!Owner)
    return;

  // 查找拥有者身上的骨骼网格体组件（排除根组件，找到武器骨骼）
  USkeletalMeshComponent *WeaponMesh = nullptr;
  TArray<USceneComponent *> Components;
  Owner->GetComponents<USceneComponent>(Components);
  for (USceneComponent *Comp : Components) {
    if (USkeletalMeshComponent *Skel = Cast<USkeletalMeshComponent>(Comp)) {
      if (Skel != Owner->GetRootComponent() &&
          Skel->DoesSocketExist(WeaponMagSocketName)) {
        WeaponMesh = Skel;
        break;
      }
    }
  }

  if (!WeaponMesh) {
    UE_LOG(LogTemp, Error,
           TEXT("ReloadComponent: Weapon socket '%s' not found!"),
           *WeaponMagSocketName.ToString());
    return;
  }

  // 附着到武器插槽，并禁用物理（固定）
  CurrentHandMag->AttachToComponent(
      WeaponMesh, FAttachmentTransformRules::SnapToTargetIncludingScale,
      WeaponMagSocketName);

  CurrentHandMag->DisablePhysics();

  // 清空手中引用
  CurrentHandMag = nullptr;
}

// 丢弃手中弹匣：启用物理，并启动定时器以便后续回收
void UReloadComponent::DropCurrentMag() {
  if (!IsValid(CurrentHandMag))
    return;

  CurrentHandMag->EnablePhysics();

  // 使用弱指针防止弹匣被销毁后回调访问野指针
  TWeakObjectPtr<AMagazineActor> WeakMag = CurrentHandMag;
  if (GetWorld()) {
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this, WeakMag]() {
          if (AMagazineActor *Mag = WeakMag.Get()) {
            OnDropTimerFinished(Mag);
          }
        },
        DropLifeTime, false);
    DropTimerHandles.Add(CurrentHandMag, TimerHandle);
  }

  CurrentHandMag = nullptr;
}

// 从池中查找已停用的弹匣，如果无可用则动态生成一个
AMagazineActor *UReloadComponent::GetMagFromPool() {
  for (AMagazineActor *Mag : MagazinePool) {
    if (IsValid(Mag) && Mag->IsHidden()) {
      Mag->Activate();
      return Mag;
    }
  }

  // 池耗尽，即时生成（并加入池中）
  UE_LOG(LogTemp, Warning,
         TEXT("ReloadComponent: Pool exhausted, spawning new magazine."));
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  AMagazineActor *NewMag = GetWorld()->SpawnActor<AMagazineActor>(
      MagazineClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
  if (IsValid(NewMag)) {
    MagazinePool.Add(NewMag);
  }
  return NewMag;
}

void UReloadComponent::BeginPlay() {
  Super::BeginPlay();

  if (!MagazineClass) {
    UE_LOG(LogTemp, Warning,
           TEXT("ReloadComponent: MagazineClass is not set!"));
  } else {
    InitPool(); // 生成初始池
  }
}

void UReloadComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  // 清理所有丢弃定时器
  for (auto &Pair : DropTimerHandles) {
    if (GetWorld() && Pair.Value.IsValid()) {
      GetWorld()->GetTimerManager().ClearTimer(Pair.Value);
    }
  }
  DropTimerHandles.Empty();

  // 销毁池中所有弹匣
  for (AMagazineActor *Mag : MagazinePool) {
    if (IsValid(Mag)) {
      Mag->Destroy();
    }
  }
  MagazinePool.Empty();

  CurrentHandMag = nullptr;

  Super::EndPlay(EndPlayReason);
}

// 生成 PoolSize 个弹匣并全部停用（隐藏）
void UReloadComponent::InitPool() {
  if (!GetWorld() || !MagazineClass)
    return;

  for (int32 i = 0; i < PoolSize; ++i) {
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AMagazineActor *NewMag = GetWorld()->SpawnActor<AMagazineActor>(
        MagazineClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (IsValid(NewMag)) {
      NewMag->Deactivate();
      MagazinePool.Add(NewMag);
    }
  }
}

// 将弹匣归还池中：停用、清理定时器、清除引用
void UReloadComponent::ReturnMagToPool(AMagazineActor *Mag) {
  if (!IsValid(Mag))
    return;

  Mag->Deactivate();

  if (CurrentHandMag == Mag) {
    CurrentHandMag = nullptr;
  }

  if (DropTimerHandles.Contains(Mag)) {
    if (GetWorld()) {
      GetWorld()->GetTimerManager().ClearTimer(DropTimerHandles[Mag]);
    }
    DropTimerHandles.Remove(Mag);
  }
}

// 定时器回调：回收弹匣到池中
void UReloadComponent::OnDropTimerFinished(AMagazineActor *Mag) {
  if (IsValid(Mag)) {
    ReturnMagToPool(Mag);
  }
}
