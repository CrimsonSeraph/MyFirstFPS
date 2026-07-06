#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "USettingFunction.generated.h"

UENUM(BlueprintType)
enum class ESettingType : uint8 {
  // 显示设置
  ScreenMode,
  Resolution,
  FrameRate,
  VerticalSynchronization,

  ViewDistanceQuality,

  // 画质设置
  ResolutionScaler,
  AntiAliasing,
  AntiAliasingQuality,
  TextureQuality,
  ShadowQuality,

  EffectsQuality,
  PostProcessingQuality,
  GlobalIlluminationQuality,
  ReflectionQuality,
  FoliageQuality,
  ShadingQuality,
};

UCLASS()
class MYFIRSTFPS_API USettingFunction : public UBlueprintFunctionLibrary {
  GENERATED_BODY()
public:
  /// @brief Apply the setting by index.
  /// @details This function applies the setting based on the provided index and
  /// setting type.
  /// @param SettingType
  /// @param Index
  /// @param ResList
  UFUNCTION(BlueprintCallable, Category = "Apply Settings by Index")
  static void ApplySettingByIndex(ESettingType SettingType, int32 Index,
                                  const TArray<FIntPoint> &ResList);

  /// @brief Get the current setting index based on the provided setting type
  /// and resolution list.
  /// @details This function retrieves the current setting index for the
  /// specified setting type and resolution list. It returns the index of the
  /// current setting in the provided list.
  /// @param SettingType
  /// @param ResList
  /// @return
  UFUNCTION(BlueprintCallable, Category = "Get Current Settings Index")
  static int32 GetCurrentSettingIndex(ESettingType SettingType,
                                      const TArray<FIntPoint> &ResList);
  /// @brief  Apply the setting by float value.
  /// @details This function applies the setting based on the provided float.
  /// @param SettingType
  /// @param Value
  UFUNCTION(BlueprintCallable, Category = "Apply Settings by Float")
  static void ApplySettingByFloat(ESettingType SettingType, float Value);

  /// @brief Get the current setting float value based on the provided setting
  /// type.
  /// @details This function retrieves the current setting float value for the
  /// specified setting type.
  /// @param SettingType
  /// @return
  UFUNCTION(BlueprintCallable, Category = "Get Current Settings Float")
  static float GetCurrentSettingFloat(ESettingType SettingType);

private:
  static void ApplyAntiAliasingMethodInternal(int32 MethodIndex);
  static int32 GetAntiAliasingMethodIndexInternal();

  // 映射表：索引 0,1,2 对应引擎 CVar 数值
  // UE5: 4=TSR, 2=TAA, 1=FXAA  (如果是 UE4，请改为 2=TAA, 1=FXAA, 0=关闭)
  static inline const int32 AAMethodCVarMap[] = {4, 2, 1};
  static inline const int32 AAMapSize = 3; // 数组大小
};
