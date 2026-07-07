#include "SettingUI/USettingFunction.h"

#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"

// 无列表整数
void USettingFunction::ApplySettingByIndex(ESettingType Setting, int32 Index) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return;

  switch (Setting) {
  // 显示设置
  case ESettingType::ScreenMode:
    // 硬编码：0=全屏, 1=窗口全屏, 2=窗口
    GSettings->SetFullscreenMode(static_cast<EWindowMode::Type>(Index));
    break;

  case ESettingType::VerticalSynchronization:
    GSettings->SetVSyncEnabled(Index > 0);
    break;

  case ESettingType::ViewDistanceQuality:
    GSettings->SetViewDistanceQuality(Index);
    break;

    // 画质设置
  case ESettingType::AntiAliasing:
    ApplyAntiAliasingMethodInternal(Index);
    break;

  case ESettingType::AntiAliasingQuality:
    GSettings->SetAntiAliasingQuality(Index);
    break;

  case ESettingType::TextureQuality:
    GSettings->SetTextureQuality(Index);
    break;

  case ESettingType::ShadowQuality:
    GSettings->SetShadowQuality(Index);
    break;

  case ESettingType::EffectsQuality:
    GSettings->SetVisualEffectQuality(Index);
    break;

  case ESettingType::PostProcessingQuality:
    GSettings->SetPostProcessingQuality(Index);
    break;

  case ESettingType::GlobalIlluminationQuality:
    GSettings->SetGlobalIlluminationQuality(Index);
    break;

  case ESettingType::ReflectionQuality:
    GSettings->SetReflectionQuality(Index);
    break;

  case ESettingType::FoliageQuality:
    GSettings->SetFoliageQuality(Index);
    break;

  case ESettingType::ShadingQuality:
    GSettings->SetShadingQuality(Index);
    break;

  // 不处理列表的设置类型
  case ESettingType::Resolution:
  case ESettingType::FrameRate:
  case ESettingType::ResolutionScaler:

  default:
    return;
  }
}

int32 USettingFunction::GetCurrentSettingIndex(ESettingType Setting) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return 0;

  switch (Setting) {
  // 显示设置
  case ESettingType::ScreenMode:
    return static_cast<int32>(GSettings->GetFullscreenMode());

  case ESettingType::VerticalSynchronization:
    return GSettings->IsVSyncEnabled() ? 1 : 0;

  case ESettingType::ViewDistanceQuality:
    return GSettings->GetViewDistanceQuality();

    // 画质设置
  case ESettingType::AntiAliasing:
    return GetAntiAliasingMethodIndexInternal();

  case ESettingType::AntiAliasingQuality:
    return GSettings->GetAntiAliasingQuality();

  case ESettingType::TextureQuality:
    return GSettings->GetTextureQuality();

  case ESettingType::ShadowQuality:
    return GSettings->GetShadowQuality();

  case ESettingType::EffectsQuality:
    return GSettings->GetVisualEffectQuality();

  case ESettingType::PostProcessingQuality:
    return GSettings->GetPostProcessingQuality();

  case ESettingType::GlobalIlluminationQuality:
    return GSettings->GetGlobalIlluminationQuality();

  case ESettingType::ReflectionQuality:
    return GSettings->GetReflectionQuality();

  case ESettingType::FoliageQuality:
    return GSettings->GetFoliageQuality();

  case ESettingType::ShadingQuality:
    return GSettings->GetShadingQuality();

  default:
    return 0;
  }
}

// 无列表浮点
void USettingFunction::ApplySettingByFloat(ESettingType SettingType,
                                           float Value) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return;

  if (SettingType == ESettingType::ResolutionScaler) {
    float Clamped = FMath::Clamp(Value, 10.0f, 100.0f);
    GSettings->SetResolutionScaleNormalized(Clamped / 100.0f);
  }
}

float USettingFunction::GetCurrentSettingFloat(ESettingType SettingType) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return 0.0f;

  if (SettingType == ESettingType::ResolutionScaler) {
    float Normalized = GSettings->GetResolutionScaleNormalized();
    return FMath::RoundToFloat(Normalized * 100.0f);
  }
  return 0.0f;
}

// 带整数列表
void USettingFunction::ApplySettingByIndexWithIntList(
    ESettingType Setting, int32 Index, const TArray<int32> &IntList) {
  if (!IntList.IsValidIndex(Index))
    return;
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return;

  switch (Setting) {
  case ESettingType::FrameRate:
    GSettings->SetFrameRateLimit(static_cast<float>(IntList[Index]));
    break;
  default:
    // 不需要列表的设置，转调无列表整数函数
    ApplySettingByIndex(Setting, Index);
    return; // 避免重复 ApplySettings
  }
}

int32 USettingFunction::GetCurrentSettingIndexWithIntList(
    ESettingType Setting, const TArray<int32> &IntList) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return 0;

  switch (Setting) {
  case ESettingType::FrameRate: {
    float CurrentFPS = GSettings->GetFrameRateLimit();
    for (int32 i = 0; i < IntList.Num(); ++i) {
      if (FMath::IsNearlyEqual(static_cast<float>(IntList[i]), CurrentFPS,
                               0.01f))
        return i;
    }
    return 0;
  }
  default:
    return GetCurrentSettingIndex(Setting);
  }
}

// 带浮点列表
void USettingFunction::ApplySettingByIndexWithFloatList(
    ESettingType Setting, int32 Index, const TArray<float> &FloatList) {
  if (!FloatList.IsValidIndex(Index))
    return;
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return;

  switch (Setting) {
  case ESettingType::FrameRate:
    GSettings->SetFrameRateLimit(FloatList[Index]);
    break;
  default:
    ApplySettingByIndex(Setting, Index);
    return;
  }
}

int32 USettingFunction::GetCurrentSettingIndexWithFloatList(
    ESettingType Setting, const TArray<float> &FloatList) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return 0;

  switch (Setting) {
  case ESettingType::FrameRate: {
    float CurrentFPS = GSettings->GetFrameRateLimit();
    for (int32 i = 0; i < FloatList.Num(); ++i) {
      if (FMath::IsNearlyEqual(FloatList[i], CurrentFPS, 0.01f))
        return i;
    }
    return 0;
  }
  default:
    return GetCurrentSettingIndex(Setting);
  }
}

// 带分辨率列表
void USettingFunction::ApplySettingByIndexWithResolutionList(
    ESettingType Setting, int32 Index, const TArray<FIntPoint> &ResList) {
  if (Setting == ESettingType::Resolution) {
    if (!ResList.IsValidIndex(Index))
      return;

    UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
    if (!GSettings)
      return;

    GSettings->SetScreenResolution(ResList[Index]);
  } else {
    ApplySettingByIndex(Setting, Index);
  }
}

int32 USettingFunction::GetCurrentSettingIndexResolutionList(
    ESettingType Setting, const TArray<FIntPoint> &ResList) {
  if (Setting == ESettingType::Resolution) {
    UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
    if (!GSettings)
      return 0;

    int32 Found = ResList.Find(GSettings->GetScreenResolution());
    return (Found != INDEX_NONE) ? Found : 0;
  } else {
    return GetCurrentSettingIndex(Setting);
  }
}

// 内部抗锯齿辅助
void USettingFunction::ApplyAntiAliasingMethodInternal(int32 MethodIndex) {
  if (MethodIndex < 0 || MethodIndex >= AAMapSize)
    return;

  IConsoleVariable *CVar =
      IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"));
  if (!CVar)
    return;

  CVar->Set(AAMethodCVarMap[MethodIndex], ECVF_SetByGameSetting);
}

int32 USettingFunction::GetAntiAliasingMethodIndexInternal() {
  IConsoleVariable *CVar =
      IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"));
  if (!CVar)
    return 0;

  int32 Current = CVar->GetInt();
  for (int32 i = 0; i < AAMapSize; ++i) {
    if (Current == AAMethodCVarMap[i])
      return i;
  }

  // 如果当前值不在表中，默认返回 0
  return 0;
}
