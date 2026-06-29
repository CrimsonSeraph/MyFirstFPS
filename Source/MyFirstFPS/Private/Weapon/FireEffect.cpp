#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "UObject/SoftObjectPath.h"
#include "Weapon/FireEffect.h"

// 全局缓存
static TMap<TWeakObjectPtr<AActor>, TMap<FSoftObjectPath, TWeakObjectPtr<UStaticMeshComponent>>> GStaticMeshCache;

// 处理数组
void UWeaponFunctionLibrary::ExecuteFireEffects(AActor* TargetActor, const TArray<FFireEffectConfig>& ConfigArray)
{
    ExecuteFireEffectsWithLife(TargetActor, ConfigArray, 0.0f);
}

// 处理单个配置
void UWeaponFunctionLibrary::ExecuteFireEffect(AActor* TargetActor, const FFireEffectConfig& Config)
{
    ExecuteFireEffectWithLife(TargetActor, Config, 0.0f);
}

// 处理数组并设置生命周期
void UWeaponFunctionLibrary::ExecuteFireEffectsWithLife(AActor* TargetActor, const TArray<FFireEffectConfig>& ConfigArray, float LifeSpan)
{
    if (!TargetActor) return;

    // 清理失效的 Actor 缓存
    for (auto It = GStaticMeshCache.CreateIterator(); It; ++It)
    {
        if (!It->Key.IsValid())
        {
            It.RemoveCurrent();
        }
    }

    // 遍历数组，逐个执行
    for (const FFireEffectConfig& Config : ConfigArray)
    {
        ExecuteSingleEffect(TargetActor, Config, LifeSpan);
    }
}

// 处理单个配置并设置生命周期
void UWeaponFunctionLibrary::ExecuteFireEffectWithLife(AActor* TargetActor, const FFireEffectConfig& Config, float LifeSpan)
{
    if (!TargetActor) return;

    // 清理缓存
    for (auto It = GStaticMeshCache.CreateIterator(); It; ++It)
    {
        if (!It->Key.IsValid())
        {
            It.RemoveCurrent();
        }
    }

    ExecuteSingleEffect(TargetActor, Config, LifeSpan);
}

// 内部核心实现
void UWeaponFunctionLibrary::ExecuteSingleEffect(AActor* TargetActor, const FFireEffectConfig& Config, float LifeSpan)
{
    USceneComponent* FoundComp = nullptr;

    // 查缓存（静态网格）
    if (Config.StaticMesh)
    {
        FSoftObjectPath MeshPath = Config.StaticMesh.ToSoftObjectPath();

        // 获取当前 Actor 的缓存 Map
        TMap<FSoftObjectPath, TWeakObjectPtr<UStaticMeshComponent>>* ActorCache = GStaticMeshCache.Find(TargetActor);
        if (ActorCache)
        {
            if (TWeakObjectPtr<UStaticMeshComponent>* CachedCompPtr = ActorCache->Find(MeshPath))
            {
                FoundComp = CachedCompPtr->Get();
            }
        }

        // 没命中，扫描组件并缓存
        if (!FoundComp)
        {
            TArray<UStaticMeshComponent*> SMComps;
            TargetActor->GetComponents<UStaticMeshComponent>(SMComps);
            for (UStaticMeshComponent* SMC : SMComps)
            {
                if (SMC->GetStaticMesh() == Config.StaticMesh.Get())    // 比较原始指针
                {
                    FoundComp = SMC;
                    GStaticMeshCache.FindOrAdd(TargetActor).Add(MeshPath, SMC);
                    break;
                }
            }
        }
    }

    // 若没找到，回落骨骼网格组件
    if (!FoundComp)
    {
        FoundComp = TargetActor->FindComponentByClass<USkeletalMeshComponent>();
    }

    if (!FoundComp) return;

    // 检查插槽并生成特效
    if (FoundComp->DoesSocketExist(Config.SocketName))
    {
        FTransform SpawnTransform = FoundComp->GetSocketTransform(Config.SocketName);

        if (Config.Actor)
        {
            AActor* SpawnedActor = TargetActor->GetWorld()->SpawnActor<AActor>(Config.Actor.Get(), SpawnTransform);
            if (SpawnedActor && LifeSpan > 0.0f)
            {
                SpawnedActor->SetLifeSpan(LifeSpan);
            }
        }

        if (Config.Effect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                TargetActor->GetWorld(),
                Config.Effect.Get(),
                SpawnTransform.GetLocation(),
                SpawnTransform.Rotator()
            );
        }
    }
}

void UWeaponFunctionLibrary::PreloadFireEffects(const TArray<FFireEffectConfig>& ConfigArray)
{
    TArray<FSoftObjectPath> AssetsToLoad;
    for (const FFireEffectConfig& Config : ConfigArray)
    {
        if (!Config.StaticMesh.IsNull())
            AssetsToLoad.Add(Config.StaticMesh.ToSoftObjectPath());
        if (!Config.Effect.IsNull())
            AssetsToLoad.Add(Config.Effect.ToSoftObjectPath());
        if (!Config.Actor.IsNull())
            AssetsToLoad.Add(Config.Actor.ToSoftObjectPath());
    }

    if (AssetsToLoad.Num() == 0) return;

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
    StreamableManager.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([]()
    {
        // 加载完成后的处理内容
    }));
}
