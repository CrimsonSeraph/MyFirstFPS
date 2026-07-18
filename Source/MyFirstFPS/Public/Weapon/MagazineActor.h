#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MagazineActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class MYFIRSTFPS_API AMagazineActor : public AActor {
  GENERATED_BODY()

public:
  AMagazineActor();

  // 弹匣的静态网格体组件，作为根组件
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
            meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UStaticMeshComponent> MeshComponent;

  /// @brief 从池中取出时调用，显示弹匣并关闭物理模拟（默认置于手中或插槽中）
  UFUNCTION(BlueprintCallable, Category = "Pool")
  void Activate();

  /// @brief 归还池中时调用，隐藏弹匣并关闭碰撞及物理
  UFUNCTION(BlueprintCallable, Category = "Pool")
  void Deactivate();

  /// @brief 启用物理模拟并施加随机冲量，用于丢弃弹匣
  UFUNCTION(BlueprintCallable, Category = "Physics")
  void EnablePhysics();

  /// @brief 禁用物理模拟，关闭碰撞，用于将弹匣固定到武器插槽
  UFUNCTION(BlueprintCallable, Category = "Physics")
  void DisablePhysics();

  /// @brief 启用碰撞（彻底开启碰撞），用于弹匣被丢弃后恢复阻挡玩家
  UFUNCTION(BlueprintCallable, Category = "Collision")
  void SetCollisionEnabled();

  /// @brief 禁用碰撞（彻底关闭碰撞），用于弹匣被拾取后避免阻挡玩家
  UFUNCTION(BlueprintCallable, Category = "Collision")
  void SetCollisionDisabled();

protected:
  virtual void BeginPlay() override;

private:
  // 内部禁用物理与碰撞的核心函数
  void SetPhysicsDisabledInternal(bool bDetachFromActor,
                                  bool bDestroyConstraints);
};
