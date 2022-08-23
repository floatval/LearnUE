// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LyraActorUtilities.generated.h"

UENUM()
enum class EBlueprintExposedNetMode : uint8
{
	/** Standalone: a game without networking, with one or more local players. Still considered a server because it has all server functionality. */
	/** Standalone : 不需要网络连接，有一个或多个本地玩家，同样被认为是一个服务器，因为这个模式有所有的服务器功能 */
	Standalone,

	/** Dedicated server: server with no local players. */
	/** Dedicated server : 服务器，没有本地玩家 */
	DedicatedServer,

	/** Listen server: a server that also has a local player who is hosting the game, available to other players on the network. */
	/** Listen server : 有一个本地玩家，并且是主机，可以被其他玩家访问  -  p2p 联机游戏*/
	ListenServer,

	/**
	 * Network client: client connected to a remote server.
	 * Network client: 连接到远程服务器的客户端
	 * Note that every mode less than this value is a kind of server, so checking NetMode < NM_Client is always some variety of server.
	 * 注意：每个模式小于这个值都是一个服务器，所以检查 NetMode < NM_Client 总是一个服务器。
	 */
	Client
};


UCLASS()
class ULyraActorUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the network mode (dedicated server, client, standalone, etc...) for an actor or component.
	 * 为一个 actor 或则一个组件获取网络模式（服务器，客户端，独立游戏，等等）
	 */
	UFUNCTION(BlueprintCallable, Category="Lyra", meta=(WorldContext="WorldContextObject", ExpandEnumAsExecs=ReturnValue))
	static EBlueprintExposedNetMode SwitchOnNetMode(const UObject* WorldContextObject);
};
