#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "USettingFunction.generated.h"

UENUM(BlueprintType)
enum class ESettingType : uint8 {
  ScreenMode,              // 屏幕模式（硬编码 0=全屏,1=窗口全屏,2=窗口）
  Resolution,              // 分辨率（需外部 FIntPoint 列表）
  FrameRate,               // 帧率限制（需外部浮点列表）
  VerticalSynchronization, // 垂直同步（0/1）
  ViewDistanceQuality,     // 视距质量

  ResolutionScaler,          // 分辨率缩放（浮点值 0~100）
  AntiAliasing,              // 抗锯齿方法（索引）
  AntiAliasingQuality,       // 抗锯齿质量
  TextureQuality,            // 纹理质量
  ShadowQuality,             // 阴影质量
  EffectsQuality,            // 特效质量
  PostProcessingQuality,     // 后处理质量
  GlobalIlluminationQuality, // 全局光照质量
  ReflectionQuality,         // 反射质量
  FoliageQuality,            // 植物质量
  ShadingQuality,            // 着色质量

  other // 其他未分类设置
};

UCLASS()
class MYFIRSTFPS_API USettingFunction : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

public:
  // 无列表整数函数
  /// @brief 通过整数索引应用设置（适用于不需要外部列表的设置）
  /// @param SettingType 设置类型
  /// @param Index 索引值（对于屏幕模式：0=全屏,1=窗口全屏,2=窗口）
  UFUNCTION(BlueprintCallable, Category = "Settings|Apply")
  static void ApplySettingByIndex(ESettingType SettingType, int32 Index);

  /// @brief 获取当前设置对应的整数索引
  /// @param SettingType 设置类型
  /// @return 当前索引
  UFUNCTION(BlueprintCallable, Category = "Settings|Get")
  static int32 GetCurrentSettingIndex(ESettingType SettingType);

  // 无列表浮点函数
  /// @brief 通过浮点值应用设置（适用于自由浮点设置，如分辨率缩放）
  /// @param SettingType 设置类型（必须为浮点型设置）
  /// @param Value 浮点值（例如分辨率缩放百分比 0~100）
  UFUNCTION(BlueprintCallable, Category = "Settings|Apply")
  static void ApplySettingByFloat(ESettingType SettingType, float Value);

  /// @brief 获取当前浮点设置值
  /// @param SettingType 设置类型（必须为浮点型设置）
  /// @return 当前浮点值
  UFUNCTION(BlueprintCallable, Category = "Settings|Get")
  static float GetCurrentSettingFloat(ESettingType SettingType);

  // 带整数列表函数
  /// @brief 通过整数列表和索引应用设置（适用于帧率等可用整数列表的设置）
  /// @param SettingType 设置类型（若不需要列表，则内部转调
  /// ApplySettingByIndex）
  /// @param Index 列表索引
  /// @param IntList 整数列表（例如帧率值列表）
  UFUNCTION(BlueprintCallable, Category = "Settings|Apply|List")
  static void ApplySettingByIndexWithIntList(ESettingType SettingType,
                                             int32 Index,
                                             const TArray<int32> &IntList);

  /// @brief 获取当前设置值在整数列表中的索引
  /// @param SettingType 设置类型
  /// @param IntList 整数列表
  /// @return 当前值在列表中的索引，未找到返回 0
  UFUNCTION(BlueprintCallable, Category = "Settings|Get|List")
  static int32 GetCurrentSettingIndexWithIntList(ESettingType SettingType,
                                                 const TArray<int32> &IntList);

  // 带浮点列表函数
  /// @brief 通过浮点列表和索引应用设置（适用于帧率等可用浮点列表的设置）
  /// @param SettingType 设置类型（若不需要列表，则内部转调
  /// ApplySettingByIndex）
  /// @param Index 列表索引
  /// @param FloatList 浮点数列表（例如帧率值列表）
  UFUNCTION(BlueprintCallable, Category = "Settings|Apply|List")
  static void ApplySettingByIndexWithFloatList(ESettingType SettingType,
                                               int32 Index,
                                               const TArray<float> &FloatList);

  /// @brief 获取当前设置值在浮点列表中的索引
  /// @param SettingType 设置类型
  /// @param FloatList 浮点数列表
  /// @return 当前值在列表中的索引，未找到返回 0
  UFUNCTION(BlueprintCallable, Category = "Settings|Get|List")
  static int32
  GetCurrentSettingIndexWithFloatList(ESettingType SettingType,
                                      const TArray<float> &FloatList);

  // 带分辨率列表函数
  /// @brief 通过分辨率列表和索引应用分辨率设置
  /// @param SettingType 设置类型（若不为 Resolution，则转调无列表函数）
  /// @param Index 分辨率列表索引
  /// @param ResList 分辨率列表（FIntPoint）
  UFUNCTION(BlueprintCallable, Category = "Settings|Apply|List")
  static void
  ApplySettingByIndexWithResolutionList(ESettingType SettingType, int32 Index,
                                        const TArray<FIntPoint> &ResList);

  /// @brief 获取当前分辨率在分辨率列表中的索引
  /// @param SettingType 设置类型
  /// @param ResList 分辨率列表
  /// @return 当前分辨率索引，未找到返回 0
  UFUNCTION(BlueprintCallable, Category = "Settings|Get|List")
  static int32
  GetCurrentSettingIndexResolutionList(ESettingType SettingType,
                                       const TArray<FIntPoint> &ResList);

private:
  // 内部抗锯齿辅助
  static void ApplyAntiAliasingMethodInternal(int32 MethodIndex);
  static int32 GetAntiAliasingMethodIndexInternal();

  // 抗锯齿 CVar 映射：索引 0->TSR(4), 1->TAA(2), 2->FXAA(1) (UE5)
  static inline const int32 AAMethodCVarMap[] = {4, 2, 1};
  static inline const int32 AAMapSize = 3;
};
