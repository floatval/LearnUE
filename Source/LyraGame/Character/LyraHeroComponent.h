// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraPawnComponent.h"
#include "Camera/LyraCameraMode.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "Input/LyraMappableConfigPair.h"
#include "LyraHeroComponent.generated.h"


class ALyraPlayerState;
class UInputComponent;
struct FInputActionValue;
class ULyraInputConfig;

/**
 * ULyraHeroComponent
 *
 *	A component used to create a player controlled pawns (characters, vehicles, etc..).
 *	一个用来创建玩家控制的 pawns 的组件
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class ULyraHeroComponent : public ULyraPawnComponent
{
	GENERATED_BODY()

public:

	ULyraHeroComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the hero component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Hero")
	static ULyraHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraHeroComponent>() : nullptr); }

	void SetAbilityCameraMode(TSubclassOf<ULyraCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

	void AddAdditionalInputConfig(const ULyraInputConfig* InputConfig);
	void RemoveAdditionalInputConfig(const ULyraInputConfig* InputConfig);

	/** True if this has completed OnPawnReadyToInitialize so is prepared for late-added features */
	/** 如果 OnPawnReadyToInitialize 已经完成，则返回 true */
	bool HasPawnInitialized() const;

	/** True if this player has sent the BindInputsNow event and is prepared for bindings */
	/** 如果玩家已经发送 BindInputsNow 事件并且准备绑定了，则返回 true */
	bool IsReadyToBindInputs() const;

	static const FName NAME_BindInputsNow;

protected:

	virtual void OnRegister() override;

	virtual bool IsPawnComponentReadyToInitialize() const override;
	void OnPawnReadyToInitialize();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_AutoRun(const FInputActionValue& InputActionValue);

	TSubclassOf<ULyraCameraMode> DetermineCameraMode() const;
	
	void OnInputConfigActivated(const FLoadedMappableConfigPair& ConfigPair);
	void OnInputConfigDeactivated(const FLoadedMappableConfigPair& ConfigPair);

protected:

	/**
	 * Input Configs that should be added to this player when initalizing the input.
	 * 当初始化输入时，应该添加到玩家的输入配置
	 * 
	 * NOTE: You should only add to this if you do not have a game feature plugin accessible to you.
	 * 注意：如果你没有一个可以访问的游戏特性插件，那么你应该只添加到这里
	 * 
	 * If you do, then use the GameFeatureAction_AddInputConfig instead.
	 * 如果你有一个可以访问的插件的话，那么使用 GameFeatureAction_AddInputConfig
	 */

	UPROPERTY(EditAnywhere)
	TArray<FMappableConfigPair> DefaultInputConfigs;
	
	// Camera mode set by an ability.
	// 技能设置的相机模式
	TSubclassOf<ULyraCameraMode> AbilityCameraMode;

	// Spec handle for the last ability to set a camera mode.
	// 最后一个设置相机模式的技能的 spec handle
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	// True when the pawn has fully finished initialization
	// 当 pawn 已经完成初始化时，返回 true
	bool bPawnHasInitialized;

	// True when player input bindings have been applyed, will never be true for non-players
	// 当玩家输入绑定已经应用的时候，返回 True，对于非玩家永远不会返回 True
	bool bReadyToBindInputs;
};
