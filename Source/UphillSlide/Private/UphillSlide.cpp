#include "UphillSlide.h"

DEFINE_LOG_CATEGORY(LogUphillSlide);

void FUphillSlideModule::StartupModule()
{
	UE_LOG(LogUphillSlide, Log, TEXT("Uphill Slide module starting"));
}

void FUphillSlideModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FUphillSlideModule, UphillSlide)
