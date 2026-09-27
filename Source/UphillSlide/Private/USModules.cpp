#include "USModules.h"
#include "USConfig.h"
#include "USSlideSubsystem.h"

UUSGameInstanceModule::UUSGameInstanceModule()
{
	bRootModule = true;
	ModConfigurations.Add(UUSConfig::StaticClass());
}

void UUSGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	// SML registers the configuration during initialization; its editor classes must be in place first.
	if (Phase == ELifecyclePhase::INITIALIZATION)
	{
		UUSConfig::UseSMLEditorClasses();
	}
	Super::DispatchLifecycleEvent(Phase);
}

UUSGameWorldModule::UUSGameWorldModule()
{
	bRootModule = true;
	ModSubsystems.Add(AUSSlideSubsystem::StaticClass());
}
