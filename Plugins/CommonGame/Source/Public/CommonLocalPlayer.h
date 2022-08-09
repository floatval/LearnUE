// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"

#include "CommonLocalPlayer.generated.h"

class UPrimaryGameLayout;

UCLASS(config=Engine, transient)
class COMMONGAME_API UCommonLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UCommonLocalPlayer();

	/** Called when the local player is assigned a player controller */
	/** 当本地玩家被赋值给一个玩家控制器的时候调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	/** Called when the local player is assigned a player state */
	/** 当本地玩家被赋值给一个玩家状态的时候调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UCommonLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate OnPlayerStateSet;

	/** Called when the local player is assigned a player pawn */
	/** 当本地玩家被赋值给一个玩家角色的时候调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UCommonLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);
	FDelegateHandle CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate);
	FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);

public:
	virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

	UPrimaryGameLayout* GetRootUILayout() const;

private:
	bool bIsPlayerViewEnabled = true;
};