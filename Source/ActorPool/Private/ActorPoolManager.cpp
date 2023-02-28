// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorPoolManager.h"

#include "ActorPoolTypes.h"

void UActorPoolManager::OnActorDestroyed(AActor* DestroyedActor)
{
	check(DestroyedActor)
	DestroyedActor->OnDestroyed.RemoveAll(this);
	AP_LOG("<UActorPoolManager> OnActorDestroyed [%s]", *DestroyedActor->GetName());
	for (auto& Pair : ActorPool)
	{
		auto& Pool = Pair.Value;
		Pool.RemoveAll([=](const FActorPoolData& Data)
		{
			return Data.Actor.GetObject() == DestroyedActor;
		});
	}
}

void UActorPoolManager::InternalInitOrResetPoolActor(AActor* Actor)
{
	check(Actor);
	Actor->SetActorLocation(FVector(0,0,5000));
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorTickEnabled(false);
}

void UActorPoolManager::InternalPreUsePoolActor(AActor* Actor)
{
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorTickEnabled(true);
}

void UActorPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UActorPoolManager::Deinitialize()
{
	Super::Deinitialize();
}

bool UActorPoolManager::AddActorToPool(TScriptInterface<IPoolActorInterface> Actor, int32 PoolNumber)
{
	if (Actor.GetObject() == nullptr)
	{
		return false;
	}
	UClass* C = Actor.GetObject()->GetClass();
	if (C == nullptr)
	{
		return false;
	}
	return AddActorToPoolByClass(C, PoolNumber);
}

bool UActorPoolManager::AddActorToPoolByClass(TSubclassOf<AActor> ActorClass, int32 PoolNumber)
{
	if (ActorClass == nullptr)
	{
		return false;
	}
	
	FTransform SpawnTransform;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AP_LOG("<UActorPoolManager> AddActorToPool [%s]:[%d]", *ActorClass->GetName(), PoolNumber);
	if (auto P = ActorPool.Find(ActorClass))
	{
		for (int32 i=0;i<PoolNumber;i++)
		{
			const auto NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParameters);
			//NewActor->OnDestroyed.AddDynamic(this,&UActorPoolManager::OnActorDestroyed);
			P->Add(FActorPoolData(NewActor, true));
			InternalInitOrResetPoolActor(NewActor);
		}
	}
	else
	{
		TArray<FActorPoolData> NewPool;
		for (int32 i=0;i<PoolNumber;i++)
		{
			const auto NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParameters);
			NewPool.Add(FActorPoolData(NewActor, true));
			InternalInitOrResetPoolActor(NewActor);
		}
		ActorPool.Add(ActorClass, NewPool);
	}
	return true;
}

TScriptInterface<IPoolActorInterface> UActorPoolManager::GetActorFromPool(TSubclassOf<AActor> ActorClass)
{
	if (ActorClass == nullptr)
	{
		return nullptr;
	}
	
	if (TArray<FActorPoolData>* P = ActorPool.Find(ActorClass))
	{
		//清理空指针
		P->RemoveAll([](const FActorPoolData& Data)
		{
			return Data.Actor.GetObject() == nullptr;
		});
		for (auto& ActorData : *P)
		{
			
			if (ActorData.bIsClean)
			{
				ActorData.bIsClean = false;
				InternalPreUsePoolActor(Cast<AActor>(ActorData.Actor.GetObject()));
				IPoolActorInterface::Execute_OnPoolActorInitialize(ActorData.Actor.GetObject());
				return ActorData.Actor;
			}
		}
		const int32 Num = ActorPool[ActorClass].Num();
		AddActorToPoolByClass(ActorClass,10);
		FActorPoolData& LastData = ActorPool[ActorClass][Num];
		LastData.bIsClean = false;
		InternalPreUsePoolActor(Cast<AActor>(LastData.Actor.GetObject()));
		AP_LOG("<UActorPoolManager> GetActorFromPool [%s]", *ActorClass->GetName());
		IPoolActorInterface::Execute_OnPoolActorInitialize(LastData.Actor.GetObject());
		return LastData.Actor;
	}
	else
	{
		AddActorToPoolByClass(ActorClass,10);
		FActorPoolData& LastData = ActorPool[ActorClass][0];
		LastData.bIsClean = false;
		InternalPreUsePoolActor(Cast<AActor>(LastData.Actor.GetObject()));
		AP_LOG("<UActorPoolManager> GetActorFromPool [%s]", *ActorClass->GetName());
		IPoolActorInterface::Execute_OnPoolActorInitialize(LastData.Actor.GetObject());
		return LastData.Actor;
	}
}

bool UActorPoolManager::ReturnActorToPool(TScriptInterface<IPoolActorInterface> Actor)
{
	if (Actor.GetObject() == nullptr)
	{
		return false;
	}
	UClass* ActorClass = Actor.GetObject()->GetClass();
	if (ActorClass == nullptr)
	{
		return false;
	}
	if (auto P = ActorPool.Find(ActorClass))
	{
		//清理空指针
		P->RemoveAll([](const FActorPoolData& Data)
		{
			return Data.Actor.GetObject() == nullptr;
		});
		
		for(FActorPoolData& Data: *P)
		{
			if (Data.Actor == Actor && !Data.bIsClean)
			{
				//日志
				AP_LOG("<UActorPoolManager> ReturnActorToPool [%s]", *ActorClass->GetName());
				InternalInitOrResetPoolActor(Cast<AActor>(Data.Actor.GetObject()));
				IPoolActorInterface::Execute_OnPoolActorReset(Actor.GetObject());
				Data.bIsClean = true;
			}
		}
	}
	return false;
}

void UActorPoolManager::ResetAllPoolActorsByClass(TSubclassOf<AActor> ActorClass)
{
	AP_LOG("<UActorPoolManager> ResetAllPoolActorsByClass [%s]", *ActorClass->GetName());
	if (auto P = ActorPool.Find(ActorClass))
	{
		for(FActorPoolData& Data: *P)
		{
			InternalInitOrResetPoolActor(Cast<AActor>(Data.Actor.GetObject()));
			IPoolActorInterface::Execute_OnPoolActorReset(Data.Actor.GetObject());
			Data.bIsClean = true;
		}
	}
}

void UActorPoolManager::PrintPoolDebugString()
{
	AP_LOG("<UActorPoolManager> PrintPoolDebugString");
	for (auto& Pair : ActorPool)
	{
		for (const auto Data : Pair.Value)
		{
			AP_LOG("<UActorPoolManager> Class = [%s]  Actor = [%s]  Stat = [%s]", *Pair.Key->GetName(), *Data.Actor.GetObject()->GetName(), Data.bIsClean ? TEXT("Clean") : TEXT("Dirty"));
		}
	}
}
