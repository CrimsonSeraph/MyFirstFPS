#include "SettingUI/USettingFunction.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"

void USettingFunction::ApplySettingByIndex(ESettingType Setting, int32 Index,
                                           const TArray<FIntPoint> &ResList) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return;

  if (Setting == ESettingType::Resolution && !ResList.IsValidIndex(Index))
    return;

  switch (Setting) {
  // 显示设置
  case ESettingType::ScreenMode:
    // 0: Fullscreen, 1: WindowedFullscreen, 2: Windowed
    GSettings->SetFullscreenMode(static_cast<EWindowMode::Type>(Index));
    break;

  case ESettingType::Resolution:
    if (ResList.IsValidIndex(Index))
      GSettings->SetScreenResolution(ResList[Index]);
    break;

  case ESettingType::FrameRate: {
    // 预设帧率列表
    // 30FPS, 60FPS, 120FPS, 144FPS, 0FPS（无限制）
    const float FPSValues[] = {30.f, 60.f, 120.f, 144.f, 0.f};
    const int32 ArraySize = sizeof(FPSValues) / sizeof(FPSValues[0]);
    if (Index >= 0 && Index < ArraySize)
      GSettings->SetFrameRateLimit(FPSValues[Index]);
    break;
  }

  case ESettingType::VerticalSynchronization:
    GSettings->SetVSyncEnabled(Index > 0);
    break;

  case ESettingType::ViewDistanceQuality:
    GSettings->SetViewDistanceQuality(Index);
    break;

  // 画质设置
  case ESettingType::ResolutionScaler:
    // 不处理浮点类设置
    break;

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

  default:
    break;
  }

  GSettings->ApplySettings(false);
}

int32 USettingFunction::GetCurrentSettingIndex(
    ESettingType Setting, const TArray<FIntPoint> &ResList) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return 0;

  if (Setting == ESettingType::Resolution && !ResList.IsValidIndex(0))
    return 0;

  switch (Setting) {
  case ESettingType::ScreenMode:
    return static_cast<int32>(GSettings->GetFullscreenMode());

  case ESettingType::Resolution: {
    int32 FoundIdx = ResList.Find(GSettings->GetScreenResolution());
    return (FoundIdx != INDEX_NONE) ? FoundIdx : 0;
  }

  case ESettingType::FrameRate: {
    const float FPSValues[] = {30.f, 60.f, 120.f, 144.f, 0.f};
    const int32 ArraySize = sizeof(FPSValues) / sizeof(FPSValues[0]);
    float CurrentFPS = GSettings->GetFrameRateLimit();
    for (int32 i = 0; i < ArraySize; ++i) {
      if (FMath::IsNearlyEqual(FPSValues[i], CurrentFPS, 0.01f))
        return i;
    }
    return 0; // 默认
  }

  case ESettingType::VerticalSynchronization:
    return GSettings->IsVSyncEnabled() ? 1 : 0;

  case ESettingType::ViewDistanceQuality:
    return GSettings->GetViewDistanceQuality();

  case ESettingType::ResolutionScaler:
    // 不处理浮点类设置
    return 0;

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

void USettingFunction::ApplySettingByFloat(ESettingType SettingType,
                                           float Value) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return;

  switch (SettingType) {
  case ESettingType::ResolutionScaler:
    float ClampedValue = FMath::Clamp(Value, 10.0f, 100.0f);
    float NormalizedValue = ClampedValue / 100.0f;
    GSettings->SetResolutionScaleNormalized(NormalizedValue);
    break;
  }
}

float USettingFunction::GetCurrentSettingFloat(ESettingType SettingType) {
  UGameUserSettings *GSettings = UGameUserSettings::GetGameUserSettings();
  if (!GSettings)
    return 0;

  float CurrentNormalized = 0.0f;
  switch (SettingType) {
  case ESettingType::ResolutionScaler:
    CurrentNormalized = GSettings->GetResolutionScaleNormalized();
    return FMath::RoundToFloat(CurrentNormalized * 100.0f);
  default:
    return 0;
  }
}

void USettingFunction::ApplyAntiAliasingMethodInternal(int32 MethodIndex) {
  if (MethodIndex < 0 || MethodIndex >= AAMapSize) {
    return;
  }

  IConsoleVariable *CVarAA =
      IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"));
  if (!CVarAA) {
    return;
  }

  CVarAA->Set(AAMethodCVarMap[MethodIndex], ECVF_SetByGameSetting);
}

int32 USettingFunction::GetAntiAliasingMethodIndexInternal() {
  IConsoleVariable *CVarAA =
      IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"));
  if (!CVarAA) {
    // 引擎初始化失败，默认返回索引 0 (TSR)
    return 0;
  }

  int32 CurrentValue = CVarAA->GetInt();

  for (int32 i = 0; i < AAMapSize; ++i) {
    if (CurrentValue == AAMethodCVarMap[i]) {
      return i;
    }
  }

  // 如果当前值不在表中，默认返回 0
  return 0;
}
