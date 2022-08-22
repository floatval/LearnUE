// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"

#include "LyraGameplayCueManager.generated.h"

/**
 * ULyraGameplayCueManager
 *
 * Game-specific manager for gameplay cues
 * 针对游戏类型的游戏 Cues 管理器
 */
UCLASS()
class ULyraGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	ULyraGameplayCueManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static ULyraGameplayCueManager* Get();

	//~UGameplayCueManager interface
	virtual void OnCreated() override;
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	virtual bool ShouldSyncLoadMissingGameplayCues() const override;
	virtual bool ShouldAsyncLoadMissingGameplayCues() const override;
	//~End of UGameplayCueManager interface

	static void DumpGameplayCues(const TArray<FString>& Args);

	// When delay loading cues, this will load the cues that must be always loaded anyway
	// 当延迟加载 cues 时，这个方法总是会加载无条件的加载 cues
	void LoadAlwaysLoadedCues();

	// Updates the bundles for the singular gameplay cue primary asset
	// 更新单一的游戏 cues 主要资产的包
	void RefreshGameplayCuePrimaryAsset();

private:
	void OnGameplayTagLoaded(const FGameplayTag& Tag);
	void HandlePostGarbageCollect();
	void ProcessLoadedTags();
	void ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject);
	void OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue);
	void RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject);
	void HandlePostLoadMap(UWorld* NewWorld);
	void UpdateDelayLoadDelegateListeners();
	bool ShouldDelayLoadGameplayCues() const;

private:
	struct FLoadedGameplayTagToProcessData
	{
		FGameplayTag Tag;
		TWeakObjectPtr<UObject> WeakOwner;

		FLoadedGameplayTagToProcessData() {}
		FLoadedGameplayTagToProcessData(const FGameplayTag& InTag, const TWeakObjectPtr<UObject>& InWeakOwner) : Tag(InTag), WeakOwner(InWeakOwner) {}
	};

private:
	// Cues that were preloaded on the client due to being referenced by content
	// Cues 在客户端中是预加载的,因为他们被内容所引用
	UPROPERTY(transient)
	TSet<UClass*> PreloadedCues;
	TMap<FObjectKey, TSet<FObjectKey>> PreloadedCueReferencers;

	// Cues that were preloaded on the client and will always be loaded (code referenced or explicitly always loaded)
	// 在客户端上预加载的 Cues，并且总是加载（代码引用或者显式总是加载）
	UPROPERTY(transient)
	TSet<UClass*> AlwaysLoadedCues;

	TArray<FLoadedGameplayTagToProcessData> LoadedGameplayTagsToProcess;
	FCriticalSection LoadedGameplayTagsToProcessCS;
	bool bProcessLoadedTagsAfterGC = false;
};
