// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/LyraGameFeaturePolicy.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystemSettings.h"
#include "AbilitySystem/LyraGameplayCueManager.h"
#include "GameplayCueSet.h"

ULyraGameFeaturePolicy::ULyraGameFeaturePolicy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ULyraGameFeaturePolicy& ULyraGameFeaturePolicy::Get()
{
	return UGameFeaturesSubsystem::Get().GetPolicy<ULyraGameFeaturePolicy>();
}

void ULyraGameFeaturePolicy::InitGameFeatureManager()
{
	Observers.Add(NewObject<ULyraGameFeature_HotfixManager>());
	Observers.Add(NewObject<ULyraGameFeature_AddGameplayCuePaths>());

	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		Subsystem.AddObserver(Observer);
	}

	Super::InitGameFeatureManager();
}

void ULyraGameFeaturePolicy::ShutdownGameFeatureManager()
{
	Super::ShutdownGameFeatureManager();

	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		Subsystem.RemoveObserver(Observer);
	}
	Observers.Empty();
}

TArray<FPrimaryAssetId> ULyraGameFeaturePolicy::GetPreloadAssetListForGameFeature(const UGameFeatureData* GameFeatureToLoad) const
{
	return Super::GetPreloadAssetListForGameFeature(GameFeatureToLoad);
}

const TArray<FName> ULyraGameFeaturePolicy::GetPreloadBundleStateForGameFeature() const
{
	return Super::GetPreloadBundleStateForGameFeature();
}

void ULyraGameFeaturePolicy::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
	// Editor will load both, this can cause hitching as the bundles are set to not preload in editor
	// 编辑器两个都会加载，这个可能会导致一些便利？，bundles 被设置为在编辑器里面不进行预加载
	bLoadClientData = !IsRunningDedicatedServer();
	bLoadServerData = !IsRunningClientOnly();
}

bool ULyraGameFeaturePolicy::IsPluginAllowed(const FString& PluginURL) const
{
	return Super::IsPluginAllowed(PluginURL);
}

//////////////////////////////////////////////////////////////////////
//

#include "Hotfix/LyraHotfixManager.h"

void ULyraGameFeature_HotfixManager::OnGameFeatureLoading(const UGameFeatureData* GameFeatureData)
{
	if (ULyraHotfixManager* HotfixManager = Cast<ULyraHotfixManager>(UOnlineHotfixManager::Get(nullptr)))
	{
		HotfixManager->RequestPatchAssetsFromIniFiles();
	}
}

//////////////////////////////////////////////////////////////////////
//

#include "GameFeatureAction_AddGameplayCuePath.h"
#include "GameplayCueManager.h"
#include "AbilitySystemGlobals.h"

void ULyraGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ULyraGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering);
	
	const FString PluginRootPath = TEXT("/") + PluginName;
	for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		const auto AddGameplayCuePathAction = Cast<UGameFeatureAction_AddGameplayCuePath>(Action);
		if(AddGameplayCuePathAction == nullptr)
		{
			continue;
		}
		
		//if (const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<UGameFeatureAction_AddGameplayCuePath>(Action))
		auto GCM = ULyraGameplayCueManager::Get();
		if(GCM == nullptr)
		{
			continue;
		}
		
		const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCuePathAction->GetDirectoryPathsToAdd();
	//	if (ULyraGameplayCueManager* GCM = ULyraGameplayCueManager::Get())
		const UGameplayCueSet* RuntimeGameplayCueSet = GCM->GetRuntimeCueSet();
		const int32 PreInitializeNumCues = RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0;

		for (const auto& [Path] : DirsToAdd)
		{
			FString MutablePath = Path;
			UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
			GCM->AddGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);	
		}
		
		// Rebuild the runtime library with these new paths
		// 使用新的路径重新编译新的运行时库
		if (!DirsToAdd.IsEmpty())
		{
			GCM->InitializeRuntimeObjectLibrary();	
		}

		const int32 PostInitializeNumCues = RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0;
		if (PreInitializeNumCues != PostInitializeNumCues)
		{
			GCM->RefreshGameplayCuePrimaryAsset();
		}
	}
}

void ULyraGameFeature_AddGameplayCuePaths::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName)
{
	const FString PluginRootPath = TEXT("/") + PluginName;
	for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		if (const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<UGameFeatureAction_AddGameplayCuePath>(GameFeatureData))
		{
			const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGFA->GetDirectoryPathsToAdd();
			
			if (UGameplayCueManager* GCM = UAbilitySystemGlobals::Get().GetGameplayCueManager())
			{
				int32 NumRemoved = 0;
				for (const FDirectoryPath& Directory : DirsToAdd)
				{
					FString MutablePath = Directory.Path;
					UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
					NumRemoved += GCM->RemoveGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
				}

				ensure(NumRemoved == DirsToAdd.Num());
				
				// Rebuild the runtime library only if there is a need to
				if (NumRemoved > 0)
				{
					GCM->InitializeRuntimeObjectLibrary();	
				}			
			}
	}
	}
}