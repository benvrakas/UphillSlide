#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "Module/GameWorldModule.h"
#include "USModules.generated.h"

// Uphill Slide's root game instance module; registers its configuration with SML.
UCLASS()
class UPHILLSLIDE_API UUSGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()

public:
	UUSGameInstanceModule();

	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;
};

// Uphill Slide's root game world module; registers its subsystem with SML.
UCLASS()
class UPHILLSLIDE_API UUSGameWorldModule : public UGameWorldModule
{
	GENERATED_BODY()

public:
	UUSGameWorldModule();
};
