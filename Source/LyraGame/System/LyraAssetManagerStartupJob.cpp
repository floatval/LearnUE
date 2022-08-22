// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAssetManagerStartupJob.h"
#include "LyraLogChannels.h"

TSharedPtr<FStreamableHandle> FLyraAssetManagerStartupJob::DoJob() const
{
	const double JobStartTime = FPlatformTime::Seconds();

	TSharedPtr<FStreamableHandle> Handle;
	UE_LOG(LogLyra, Display, TEXT("Startup job \"%s\" starting"), *JobName);
	JobFunc(*this, Handle);

	// 目前项目相关的代码逻辑中，没有对 Handle 进行处理，Handle 一直为 nullptr
	if (Handle.IsValid())
	{
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateRaw(this, &FLyraAssetManagerStartupJob::UpdateSubstepProgressFromStreamable));
		Handle->WaitUntilComplete(0.0f, false);
		// @todo 作用是啥？为啥要绑定两次？
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate());
	}

	UE_LOG(LogLyra, Display, TEXT("Startup job \"%s\" took %.2f seconds to complete"), *JobName, FPlatformTime::Seconds() - JobStartTime);

	return Handle;
}
