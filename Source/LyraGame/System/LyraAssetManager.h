// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "LyraAssetManagerStartupJob.h"
#include "LyraAssetManager.generated.h"

class ULyraGameData;
class ULyraPawnData;

struct FLyraBundles
{
	static const FName Equipped;
};


/**
 * ULyraAssetManager
 *
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	通过覆写功能和存储游戏特定的类型来实现游戏的资产管理器
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	预期绝大多数游戏都会覆写AssetManager，因为它提供了一个好的位置来处理游戏特定的加载逻辑
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
 *	通过在 DefaultEngine.ini 文件中的 'AssetManagerClassName' 设置来使用此类
 */
UCLASS(Config = Game)
class ULyraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	ULyraAssetManager();

	// Returns the AssetManager singleton object.
	// 返回资源管理器单例对象
	static  ULyraAssetManager& Get();

	// Returns the asset referenced by a TSoftObjectPtr.  This will synchronously load the asset if it's not already loaded.
	// 返回一个TSoftObjectPtr引用的资产。如果它没有加载，则将同步加载它。
	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Returns the subclass referenced by a TSoftClassPtr.  This will synchronously load the asset if it's not already loaded.
	// 返回一个TSoftClassPtr引用的子类。如果它没有加载，则将同步加载它。
	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Logs all assets currently loaded and tracked by the asset manager.
	// 记录所有当前资源管理器加载和跟踪的资产。
	static void DumpLoadedAssets();

	const ULyraGameData& GetGameData();
	const ULyraPawnData* GetDefaultPawnData() const;

protected:
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		if (const UPrimaryDataAsset* const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// Does a blocking load if needed
		// 如果需要的话采用阻塞加载
		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName()));
	}


	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);
	static bool ShouldLogAssetLoads();

	// Thread safe way of adding a loaded asset to keep in memory.
	// 添加一个加载过的资产并保存在内存中的线程安全的方法
	void AddLoadedAsset(const UObject* Asset);

	//~UAssetManager interface
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~End of UAssetManager interface

	UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

protected:

	// Global game data asset to use.
	// 使用全局游戏数据
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraGameData> LyraGameDataPath;

	// Loaded version of the game data
	UPROPERTY(Transient)
	TMap<UClass*, UPrimaryDataAsset*> GameDataMap;

	// Pawn data used when spawning player pawns if there isn't one set on the player state.
	// 当玩家状态没有设置时，用于创建玩家人物的人物数据
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraPawnData> DefaultPawnData;

private:
	// Flushes the StartupJobs array. Processes all startup work.
	// 刷新 StartupJobs 数组。处理所有启动工作。
	void DoAllStartupJobs();

	// Sets up the ability system
	// 设置技能系统
	void InitializeAbilitySystem();
	void InitializeGameplayCueManager();

	// Called periodically during loads, could be used to feed the status to a loading screen
	// 在加载期间周期性地调用，可以用于向加载界面提供状态
	void UpdateInitialGameContentLoadPercent(float GameContentPercent);

	// The list of tasks to execute on startup. Used to track startup progress.
	// 在启动时执行的任务列表。用于跟踪启动进度。
	TArray<FLyraAssetManagerStartupJob> StartupJobs;

private:
	
	// Assets loaded and tracked by the asset manager.
	// 用来加载和跟踪的资产的资源管理器
	UPROPERTY()
	TSet<const UObject*> LoadedAssets;

	// Used for a scope lock when modifying the list of load assets.
	// 当修改加载资源的列表的时候，用来当作一个范围锁的对象
	FCriticalSection LoadedAssetsCritical;
};


template<typename AssetType>
AssetType* ULyraAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

template<typename AssetType>
TSubclassOf<AssetType> ULyraAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}
