#include "USConfig.h"
#include "UphillSlide.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/Properties/ConfigPropertyBool.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Configuration/Properties/WidgetExtension/CP_Float.h"
#include "Configuration/Properties/WidgetExtension/CP_Section.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "UphillSlide"

const FString UUSConfig::UphillAngleKey = TEXT("UphillAngle");
const FString UUSConfig::SpeedLossKey = TEXT("SpeedLoss");
const FString UUSConfig::KeepEntrySpeedKey = TEXT("KeepEntrySpeed");

namespace
{
	FConfigId MakeConfigId()
	{
		FConfigId Id;
		Id.ModReference = TEXT("UphillSlide");
		return Id;
	}

	struct FTierDefaults
	{
		float UphillAngle;
		// Shown as the section header, followed by Hint in brackets.
		FText Name;
		FText Hint;
		FText Tooltip;
	};

	// Default uphill angle and Mods menu text of each tier, indexed by EUSTier.
	TArray<FTierDefaults> GetTierDefaults()
	{
		return {
			{ 7.4f, LOCTEXT("TierNone", "No Blade Runners"), LOCTEXT("TierNoneHint", "vanilla limit 7.4°"), LOCTEXT("TierNoneTip", "When you aren't wearing Blade Runners.") },
			{ 15.f, LOCTEXT("TierMk1", "Blade Runners"), LOCTEXT("TierMk1Hint", "also other mods' Blade Runners"), LOCTEXT("TierMk1Tip", "Vanilla Blade Runners. Blade Runners added by other mods (not Mk+ Blade Runners) use these settings too.") },
			{ 20.f, LOCTEXT("TierMk2", "Mk.2 Blade Runners"), LOCTEXT("TierMkPlusHint", "only with Mk+ Blade Runners"), LOCTEXT("TierMk2Tip", "Only used if you have the optional Mk+ Blade Runners mod. Covers every Mk.2 variant.") },
			{ 27.f, LOCTEXT("TierMk3", "Mk.3 Blade Runners"), LOCTEXT("TierMkPlusHint", "only with Mk+ Blade Runners"), LOCTEXT("TierMk3Tip", "Only used if you have the optional Mk+ Blade Runners mod. Covers every Mk.3 variant.") },
			{ 35.f, LOCTEXT("TierMk4", "Mk.4 Blade Runners"), LOCTEXT("TierMkPlusHint", "only with Mk+ Blade Runners"), LOCTEXT("TierMk4Tip", "Only used if you have the optional Mk+ Blade Runners mod.") },
			{ 40.f, LOCTEXT("TierMk5", "Mk.5 Blade Runners"), LOCTEXT("TierMkPlusHint", "only with Mk+ Blade Runners"), LOCTEXT("TierMk5Tip", "Only used if you have the optional Mk+ Blade Runners mod.") },
		};
	}

	// Returns the header name of the tier whose section is stored under SectionKey, or empty text.
	FText GetTierName(const FString& SectionKey)
	{
		const TArray<FTierDefaults> Defaults = GetTierDefaults();
		for (int32 Index = 0; Index < USTierCount; ++Index)
		{
			if (UUSConfig::GetTierSectionKey(static_cast<EUSTier>(Index)) == SectionKey)
			{
				return Defaults[Index].Name;
			}
		}
		return FText::GetEmpty();
	}

	// Returns the registered live configuration's root section, or null if it isn't registered yet.
	const UConfigPropertySection* FindRoot(const UObject* WorldContext)
	{
		const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		const UConfigManager* Manager = GameInstance ? GameInstance->GetSubsystem<UConfigManager>() : nullptr;
		return Manager ? Manager->GetConfigurationRootSection(MakeConfigId()) : nullptr;
	}

	// Returns the boolean setting Key inside Section, or null.
	const UConfigPropertyBool* FindBool(const UConfigPropertySection* Section, const FString& Key)
	{
		const TObjectPtr<UConfigProperty>* Found = Section ? Section->SectionProperties.Find(Key) : nullptr;
		return Found ? Cast<UConfigPropertyBool>(Found->Get()) : nullptr;
	}

	// Returns the float setting Key inside Section, or null.
	const UConfigPropertyFloat* FindFloat(const UConfigPropertySection* Section, const FString& Key)
	{
		const TObjectPtr<UConfigProperty>* Found = Section ? Section->SectionProperties.Find(Key) : nullptr;
		return Found ? Cast<UConfigPropertyFloat>(Found->Get()) : nullptr;
	}
}

FString UUSConfig::GetTierSectionKey(EUSTier Tier)
{
	switch (Tier)
	{
	case EUSTier::None: return TEXT("NoBladeRunners");
	case EUSTier::Mk1: return TEXT("Mk1");
	case EUSTier::Mk2: return TEXT("Mk2");
	case EUSTier::Mk3: return TEXT("Mk3");
	case EUSTier::Mk4: return TEXT("Mk4");
	case EUSTier::Mk5: return TEXT("Mk5");
	default: return TEXT("Unknown");
	}
}

UUSConfig::UUSConfig()
{
	ConfigId = MakeConfigId();
	DisplayName = LOCTEXT("ConfigName", "Uphill Slide");
	Description = LOCTEXT("ConfigDescription", "How steep a slope you can slide up, for each Blade Runners tier.");

	RootSection = CreateDefaultSubobject<UConfigPropertySection>(TEXT("RootSection"));

	const TArray<FTierDefaults> Defaults = GetTierDefaults();
	for (int32 Index = 0; Index < USTierCount; ++Index)
	{
		const FString SectionKey = GetTierSectionKey(static_cast<EUSTier>(Index));
		const FTierDefaults& Tier = Defaults[Index];

		UConfigPropertySection* Section = CreateDefaultSubobject<UConfigPropertySection>(*(SectionKey + TEXT("Section")));
		Section->DisplayName = Tier.Hint;
		Section->Tooltip = Tier.Tooltip;

		UConfigPropertyFloat* UphillAngle = CreateDefaultSubobject<UConfigPropertyFloat>(*(SectionKey + TEXT("UphillAngle")));
		UphillAngle->DisplayName = LOCTEXT("UphillAngle", "Max uphill angle (degrees)");
		UphillAngle->Tooltip = LOCTEXT("UphillAngleTip", "The steepest slope you can keep sliding up, in degrees above flat (0 to 89). Vanilla is about 7.4. Ramps on an 8 m foundation: 1 m is 7.1, 2 m is 14.0, 4 m is 26.6. In multiplayer, the host's settings apply to everyone.");
		UphillAngle->DefaultValue = Tier.UphillAngle;
		UphillAngle->Value = Tier.UphillAngle;
		Section->SectionProperties.Add(UphillAngleKey, UphillAngle);

		UConfigPropertyFloat* SpeedLoss = CreateDefaultSubobject<UConfigPropertyFloat>(*(SectionKey + TEXT("SpeedLoss")));
		SpeedLoss->DisplayName = LOCTEXT("SpeedLoss", "Speed lost while sliding (%)");
		SpeedLoss->Tooltip = LOCTEXT("SpeedLossTip", "How quickly a slide loses speed, on flat ground and uphill, in percent of normal: 100 is vanilla, 50 keeps your speed twice as long, 0 never slows down. Steep downhill slopes still speed you up as usual. In multiplayer, the host's settings apply to everyone.");
		SpeedLoss->DefaultValue = 100.f;
		SpeedLoss->Value = 100.f;
		Section->SectionProperties.Add(SpeedLossKey, SpeedLoss);

		UConfigPropertyBool* KeepEntrySpeed = CreateDefaultSubobject<UConfigPropertyBool>(*(SectionKey + TEXT("KeepEntrySpeed")));
		KeepEntrySpeed->DisplayName = LOCTEXT("KeepEntrySpeed", "Keep your entry speed");
		KeepEntrySpeed->Tooltip = LOCTEXT("KeepEntrySpeedTip", "On: a slide keeps the speed you go into it with, for example from bhopping, instead of being pulled down to the normal slide speed. That speed then fades at the Speed lost while sliding rate, so at 0% you keep it. Off: vanilla. In multiplayer, the host's settings apply to everyone.");
		KeepEntrySpeed->DefaultValue = false;
		KeepEntrySpeed->Value = false;
		Section->SectionProperties.Add(KeepEntrySpeedKey, KeepEntrySpeed);

		RootSection->SectionProperties.Add(SectionKey, Section);
	}
}

namespace
{
	const TCHAR* SMLPropertyPath = TEXT("/SML/Interface/UI/Menu/Mods/ConfigProperties/");

	// Loads SML's Blueprint subclass of a config property class, e.g. BP_ConfigPropertyFloat for UConfigPropertyFloat.
	UClass* LoadEditorClass(const UClass* NativeClass)
	{
		const FString Name = TEXT("BP_") + NativeClass->GetName();
		const FString Path = FString::Printf(TEXT("%s%s.%s_C"), SMLPropertyPath, *Name, *Name);
		UClass* Loaded = LoadClass<UConfigProperty>(nullptr, *Path);
		if (!Loaded || !Loaded->IsChildOf(NativeClass))
		{
			UE_LOG(LogUphillSlide, Warning, TEXT("Config: SML editor class %s not found; that setting won't be editable in the Mods menu"), *Path);
			return nullptr;
		}
		return Loaded;
	}

	// Creates an instance of EditorClass under Outer with every property value of Source.
	UConfigProperty* CloneAs(const UConfigProperty* Source, UClass* EditorClass, UObject* Outer)
	{
		UConfigProperty* Clone = NewObject<UConfigProperty>(Outer, EditorClass, NAME_None, RF_Public);
		for (TFieldIterator<FProperty> It(Source->GetClass()); It; ++It)
		{
			It->CopyCompleteValue_InContainer(Clone, Source);
		}
		return Clone;
	}

	// Sets the accepted range of Property, a number setting stored under Key, replacing SML's 0 to 1 default.
	void ApplyNumberRange(const FString& Key, UConfigProperty* Property)
	{
		UCP_Float* Float = Cast<UCP_Float>(Property);
		if (!Float)
		{
			return;
		}
		if (Key == UUSConfig::UphillAngleKey)
		{
			Float->MinValue = UUSConfig::MinUphillAngle;
			Float->MaxValue = UUSConfig::MaxUphillAngle;
		}
		else if (Key == UUSConfig::SpeedLossKey)
		{
			Float->MinValue = UUSConfig::MinSpeedLoss;
			Float->MaxValue = UUSConfig::MaxSpeedLoss;
		}
		else
		{
			UE_LOG(LogUphillSlide, Warning, TEXT("Config: no range for number setting '%s'; the Mods menu only accepts 0 to 1"), *Key);
		}
	}

	// Converts OldSection and everything under it to SML's Blueprint editor classes, laid out as a vertical list.
	// Nested sections get a collapsible header showing their tier name and display name.
	UConfigPropertySection* ConvertSectionRecursive(const UConfigPropertySection* OldSection, const FString& SectionKey, UClass* SectionEditorClass, UObject* Outer, bool bIsRoot, int32& OutConverted, int32& OutTotal)
	{
		UConfigPropertySection* NewSection = Cast<UConfigPropertySection>(CloneAs(OldSection, SectionEditorClass, Outer));
		NewSection->SectionProperties.Reset();
		if (UCP_Section* EditorSection = Cast<UCP_Section>(NewSection))
		{
			EditorSection->WidgetType = ECP_SectionWidgetType::CPS_Vertical;
			// SML shows the header as "HeaderText (DisplayName)" and shows nothing when HeaderText is empty.
			EditorSection->HasHeader = !bIsRoot;
			EditorSection->HeaderText = bIsRoot ? FText::GetEmpty() : GetTierName(SectionKey);
		}
		for (const TPair<FString, TObjectPtr<UConfigProperty>>& Pair : OldSection->SectionProperties)
		{
			UConfigProperty* Old = Pair.Value;
			if (!Old)
			{
				continue;
			}
			++OutTotal;
			if (const UConfigPropertySection* OldChildSection = Cast<UConfigPropertySection>(Old))
			{
				NewSection->SectionProperties.Add(Pair.Key, ConvertSectionRecursive(OldChildSection, Pair.Key, SectionEditorClass, NewSection, false, OutConverted, OutTotal));
				++OutConverted;
				continue;
			}
			if (UClass* EditorClass = LoadEditorClass(Old->GetClass()))
			{
				UConfigProperty* Clone = CloneAs(Old, EditorClass, NewSection);
				ApplyNumberRange(Pair.Key, Clone);
				NewSection->SectionProperties.Add(Pair.Key, Clone);
				++OutConverted;
				continue;
			}
			NewSection->SectionProperties.Add(Pair.Key, Old);
		}
		return NewSection;
	}
}

void UUSConfig::UseSMLEditorClasses()
{
	UUSConfig* Defaults = GetMutableDefault<UUSConfig>();
	UConfigPropertySection* OldRoot = Defaults->RootSection;
	UClass* SectionClass = OldRoot ? LoadEditorClass(UConfigPropertySection::StaticClass()) : nullptr;
	if (!SectionClass || OldRoot->GetClass() == SectionClass)
	{
		return;
	}

	int32 Converted = 0, Total = 0;
	Defaults->RootSection = ConvertSectionRecursive(OldRoot, FString(), SectionClass, Defaults, true, Converted, Total);
	UE_LOG(LogUphillSlide, Log, TEXT("Config: %d of %d settings/sections use SML's editor widgets"), Converted, Total);
}

bool UUSConfig::ReadTierSettings(const UObject* WorldContext, TArray<FUSTierSettings>& OutSettings)
{
	OutSettings.Reset();
	const UConfigPropertySection* Root = FindRoot(WorldContext);
	if (!Root)
	{
		UE_LOG(LogUphillSlide, Verbose, TEXT("Config: not registered yet"));
		return false;
	}

	const TArray<FTierDefaults> Defaults = GetTierDefaults();
	OutSettings.SetNum(USTierCount);
	for (int32 Index = 0; Index < USTierCount; ++Index)
	{
		const EUSTier Tier = static_cast<EUSTier>(Index);
		const TObjectPtr<UConfigProperty>* SectionProperty = Root->SectionProperties.Find(GetTierSectionKey(Tier));
		const UConfigPropertySection* Section = SectionProperty ? Cast<UConfigPropertySection>(SectionProperty->Get()) : nullptr;
		const UConfigPropertyFloat* UphillAngle = FindFloat(Section, UphillAngleKey);
		const UConfigPropertyFloat* SpeedLoss = FindFloat(Section, SpeedLossKey);
		const UConfigPropertyBool* KeepEntrySpeed = FindBool(Section, KeepEntrySpeedKey);
		if (!UphillAngle || !SpeedLoss || !KeepEntrySpeed)
		{
			UE_LOG(LogUphillSlide, Warning, TEXT("Config: settings for %s missing (section %d, angle %d, speed loss %d, keep entry speed %d); using defaults"),
				USTierName(Tier), Section != nullptr, UphillAngle != nullptr, SpeedLoss != nullptr, KeepEntrySpeed != nullptr);
		}

		FUSTierSettings& Settings = OutSettings[Index];
		Settings.UphillAngleDegrees = FMath::Clamp(UphillAngle ? UphillAngle->Value : Defaults[Index].UphillAngle, MinUphillAngle, MaxUphillAngle);
		Settings.SpeedLossPercent = FMath::Clamp(SpeedLoss ? SpeedLoss->Value : 100.f, MinSpeedLoss, MaxSpeedLoss);
		Settings.bKeepEntrySpeed = KeepEntrySpeed ? KeepEntrySpeed->Value : false;
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
