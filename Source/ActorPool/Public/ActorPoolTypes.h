// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
//#include "ActorPoolTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ActorPoolLog, Log, All);


#define AP_LOG(l,...) UE_LOG(ActorPoolLog, Log, TEXT(l),  ##__VA_ARGS__)
#define AP_WARNING(l,...) UE_LOG(ActorPoolLog, Warning, TEXT(l),  ##__VA_ARGS__)
#define AP_ERROR(l,...) UE_LOG(ActorPoolLog, Error, TEXT(l),  ##__VA_ARGS__)
