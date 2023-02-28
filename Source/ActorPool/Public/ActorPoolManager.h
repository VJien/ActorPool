// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorPoolInterface.h"
#include "UObject/Object.h"
#include "ActorPoolManager.generated.h"


USTRUCT()
struct FActorPoolData
{
	GENERATED_BODY()
	FActorPoolData()
	{}
	FActorPoolData(TScriptInterface<IPoolActorInterface> InActor, bool bInIsClean) :Actor(InActor), bIsClean(bInIsClean)
	{
	}
	FActorPoolData(TScriptInterface<IPoolActorInterface> InActor):Actor(InActor), bIsClean(true)
	{
	}
	
public:
	bool operator==(const FActorPoolData& Other) const
	{
		return Actor == Other.Actor;
	}
	bool operator!=(const FActorPoolData& Other) const
	{
		return Actor != Other.Actor;
	}
	
	UPROPERTY()
	TScriptInterface<IPoolActorInterface> Actor = nullptr;
	UPROPERTY()
	bool bIsClean = true;
};



class IPoolActorInterface;
UCLASS()
class ACTORPOOL_API UActorPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	void OnActorDestroyed(AActor* DestroyedActor);
	void InternalInitOrResetPoolActor(AActor* Actor);
	void InternalPreUsePoolActor(AActor* Actor);
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	
public:
	UFUNCTION(BlueprintCallable, Category = "ActorPool")
	bool AddActorToPool(TScriptInterface<IPoolActorInterface> Actor, int32 PoolNumber = 10);
	UFUNCTION(BlueprintCallable, Category = "ActorPool")
	bool AddActorToPoolByClass(TSubclassOf<AActor> ActorClass, int32 PoolNumber = 10);
	UFUNCTION(BlueprintCallable, Category = "ActorPool")
	TScriptInterface<IPoolActorInterface> GetActorFromPool(TSubclassOf<AActor> ActorClass);
	UFUNCTION(BlueprintCallable, Category = "ActorPool")
	bool ReturnActorToPool(TScriptInterface<IPoolActorInterface> Actor);
	UFUNCTION(BlueprintCallable, Category = "ActorPool")
	void ResetAllPoolActorsByClass(TSubclassOf<AActor> ActorClass);
	UFUNCTION(BlueprintCallable, Category = "ActorPool")
	void PrintPoolDebugString();

protected:
	TMap<TSubclassOf<AActor>, TArray<FActorPoolData>> ActorPool;
	
};
