#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "ReloadComponent.generated.h"

class AMagazineActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYFIRSTFPS_API UReloadComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UReloadComponent();

  // 对象池初始大小
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
  int32 PoolSize = 10;

  // 弹匣 Actor 的类（用于生成）
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
  TSubclassOf<AMagazineActor> MagazineClass;

  // 武器骨骼上用于手持弹匣的插槽名称（如 "hand_r"）
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
  FName HandSocketName = "hand_r";

  // 武器骨骼上用于安装弹匣的插槽名称（如 "mag_socket"）
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
  FName WeaponMagSocketName = "mag_socket";

  // 丢弃后等待回收的时间（秒）
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
  float DropLifeTime = 3.0f;

  /// @brief 从池中取出一个弹匣并握在手上（附着到 HandSocket）
  /// @param MagClass 可选，若为空则使用 MagazineClass
  /// @return 成功返回弹匣指针，否则 nullptr
  UFUNCTION(BlueprintCallable, Category = "Reload")
  AMagazineActor *
  SpawnAndGrabMag(TSubclassOf<AMagazineActor> MagClass = nullptr);

  /// @brief 将手中弹匣转移到武器插槽（WeaponMagSocket），并禁用物理
  UFUNCTION(BlueprintCallable, Category = "Reload")
  void TransferMagToWeapon();

  /// @brief 丢弃手中弹匣，启用物理，并在 DropLifeTime 后回收至池中
  UFUNCTION(BlueprintCallable, Category = "Reload")
  void DropCurrentMag();

  /// @brief 获取当前握持的弹匣（仅读）
  UFUNCTION(BlueprintPure, Category = "Reload")
  AMagazineActor *GetCurrentHandMag() const { return CurrentHandMag; }

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  // 初始化对象池，生成 PoolSize 个弹匣并立即停用
  void InitPool();

  // 从池中获取一个空闲弹匣，若池用尽则动态生成
  AMagazineActor *GetMagFromPool();

  // 将弹匣归还池中（停用并清理相关定时器）
  void ReturnMagToPool(AMagazineActor *Mag);

  // 定时器回调：丢弃超时后回收弹匣
  void OnDropTimerFinished(AMagazineActor *Mag);

private:
  // 当前手中握持的弹匣
  UPROPERTY()
  AMagazineActor *CurrentHandMag;

  // 对象池数组
  UPROPERTY()
  TArray<AMagazineActor *> MagazinePool;

  // 记录每个被丢弃弹匣的回收定时器句柄
  TMap<AMagazineActor *, FTimerHandle> DropTimerHandles;
};
