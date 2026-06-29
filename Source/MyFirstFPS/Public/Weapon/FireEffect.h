#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FireEffect.generated.h"

// 结构体
USTRUCT(BlueprintType)
struct FFireEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TObjectPtr<UNiagaraSystem> Effect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TSubclassOf<AActor> Actor;
};

// 蓝图函数库
UCLASS()
class UWeaponFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 直接传入数组
    UFUNCTION(BlueprintCallable, Category = "Weapon", meta = (DisplayName = "Execute Fire Effects (Array)"))
    static void ExecuteFireEffects(AActor* TargetActor, const TArray<FFireEffectConfig>& ConfigArray);

    // 接受单个配置
    UFUNCTION(BlueprintCallable, Category = "Weapon", meta = (DisplayName = "Execute Fire Effect (Single)"))
    static void ExecuteFireEffect(AActor* TargetActor, const FFireEffectConfig& Config);

private:
    // 内部核心逻辑
    static void ExecuteSingleEffect(AActor* TargetActor, const FFireEffectConfig& Config);
};
