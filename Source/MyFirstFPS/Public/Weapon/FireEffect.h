#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPath.h"
#include "FireEffect.generated.h"

// 结构体
USTRUCT(BlueprintType)
struct FFireEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TSoftObjectPtr<UNiagaraSystem> Effect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TSoftClassPtr<AActor> Actor;
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

    // 直接传入数组并设置生命周期
    UFUNCTION(BlueprintCallable, Category = "Weapon", meta = (DisplayName = "Execute Fire Effect (Array with LifeSpan)"))
    static void ExecuteFireEffectsWithLife(AActor* TargetActor, const TArray<FFireEffectConfig>& ConfigArray, float LifeSpan = 0.0f);

    // 接受单个配置并设置生命周期
    UFUNCTION(BlueprintCallable, Category = "Weapon", meta = (DisplayName = "Execute Fire Effect (Single with LifeSpan)"))
    static void ExecuteFireEffectWithLife(AActor* TargetActor, const FFireEffectConfig& Config, float LifeSpan = 0.0f);

    // 异步预加载函数
    UFUNCTION(BlueprintCallable, Category = "Weapon|Preload")
    static void PreloadFireEffects(const TArray<FFireEffectConfig>& ConfigArray);

private:
    // 内部核心逻辑
    static void ExecuteSingleEffect(AActor* TargetActor, const FFireEffectConfig& Config, float LifeSpan);
};
