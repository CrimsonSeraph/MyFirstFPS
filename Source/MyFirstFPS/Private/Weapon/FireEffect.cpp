#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/FireEffect.h"

// 全局缓存
static TMap<TWeakObjectPtr<AActor>, TMap<UStaticMesh*, TWeakObjectPtr<UStaticMeshComponent>>> GStaticMeshCache;

// 处理数组
void UWeaponFunctionLibrary::ExecuteFireEffects(AActor* TargetActor, const TArray<FFireEffectConfig>& ConfigArray)
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
        ExecuteSingleEffect(TargetActor, Config);
    }
}

// 处理单个配置
void UWeaponFunctionLibrary::ExecuteFireEffect(AActor* TargetActor, const FFireEffectConfig& Config)
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

    ExecuteSingleEffect(TargetActor, Config);
}

// 内部核心实现
void UWeaponFunctionLibrary::ExecuteSingleEffect(AActor* TargetActor, const FFireEffectConfig& Config)
{
    USceneComponent* FoundComp = nullptr;

    // 查缓存
    if (Config.StaticMesh)
    {
        // 尝试找当前 Actor 的缓存
        TMap<UStaticMesh*, TWeakObjectPtr<UStaticMeshComponent>>* ActorCache = GStaticMeshCache.Find(TargetActor);
        if (ActorCache)
        {
            if (TWeakObjectPtr<UStaticMeshComponent>* CachedCompPtr = ActorCache->Find(Config.StaticMesh))
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
                if (SMC->GetStaticMesh() == Config.StaticMesh)
                {
                    FoundComp = SMC;
                    GStaticMeshCache.FindOrAdd(TargetActor).Add(Config.StaticMesh, SMC);
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
            TargetActor->GetWorld()->SpawnActor<AActor>(Config.Actor, SpawnTransform);
        }

        if (Config.Effect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                TargetActor->GetWorld(),
                Config.Effect,
                SpawnTransform.GetLocation(),
                SpawnTransform.Rotator()
            );
        }
    }
}
