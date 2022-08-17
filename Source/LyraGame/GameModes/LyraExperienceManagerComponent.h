// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "GameFeaturePluginOperationResult.h"
#include "LoadingProcessInterface.h"

#include "LyraExperienceManagerComponent.generated.h"

class ULyraExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLyraExperienceLoaded, const ULyraExperienceDefinition* /*Experience*/);

enum class ELyraExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

UCLASS()
class ULyraExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	ULyraExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface

#if WITH_SERVER_CODE
	// 单机游戏的话，也是通过这个来的
	void ServerSetCurrentExperience(FPrimaryAssetId ExperienceId);
#endif

	// Ensures the delegate is called once the experience has been loaded,
	// 确保体验加载完成之后调用一次这个委托
	// before others are called.
	// 在其他委托之前调用
	// However, if the experience has already loaded, calls the delegate immediately.
	// 如果体验已经加载，立即调用委托
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// 确保体验加载完成之后调用一次这个委托
	// If the experience has already loaded, calls the delegate immediately
	// 如果体验已经加载，立即调用委托
	void CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// 确保体验加载完成之后调用一次这个委托
	// If the experience has already loaded, calls the delegate immediately
	// 如果体验已经加载，立即调用委托
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// This returns the current experience if it is fully loaded, asserting otherwise
	// 如果体验已经加载，返回当前体验，否则命中断言
	// (i.e., if you called it too soon)
	const ULyraExperienceDefinition* GetCurrentExperienceChecked() const;

	// Returns true if the experience is fully loaded
	// 如果体验已经加载，返回true
	bool IsExperienceLoaded() const;

private:
	UFUNCTION()
	void OnRep_CurrentExperience();

	// 开始加载体验
	void StartExperienceLoad();
	// 体验加载完成
	void OnExperienceLoadComplete();
	// 加载游戏特性完成
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	// 游戏体验完全加载
	void OnExperienceFullLoadCompleted();

	// 行动取消完成
	void OnActionDeactivationCompleted();
	// 所有行动取消完成
	void OnAllActionsDeactivated();

private:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	// 当前游戏体验
	const ULyraExperienceDefinition* CurrentExperience;

	// 当前体验状态
	ELyraExperienceLoadState LoadState = ELyraExperienceLoadState::Unloaded;

	// 在该体验中加载的游戏特性插件的数量
	int32 NumGameFeaturePluginsLoading = 0;
	// 游戏特性的 URL 路径 - 本地的话，为文件的相对路径
	TArray<FString> GameFeaturePluginURLs;

	// 暂停的观察者的数量
	int32 NumObservedPausers = 0;
	// 期望的暂停的观察者的数量
	int32 NumExpectedPausers = 0;

	/**
	 * Delegate called when the experience has finished loading just before others
	 * 在体验加载完成之后，第一个调用的委托
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FOnLyraExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	/** 在体验加载完成之后，调用的委托 */
	FOnLyraExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	/** 在体验加载完成之后，调用的委托 */
	FOnLyraExperienceLoaded OnExperienceLoaded_LowPriority;
};
