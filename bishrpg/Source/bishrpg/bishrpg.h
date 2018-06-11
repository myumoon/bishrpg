// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(BishRPG, Log, All);

#define GAME_LOG(format, ...) UE_LOG(BishRPG, Log, TEXT(format), __VA_ARGS__)
#define GAME_WARNING(format, ...) UE_LOG(BishRPG, Warning, TEXT(format), __VA_ARGS__)
#define GAME_ERROR(format, ...) UE_LOG(BishRPG, Error, TEXT(format), __VA_ARGS__)
#define GAME_FATAL(format, ...) UE_LOG(BishRPG, Fatal, TEXT(format), __VA_ARGS__)

// FString name = GETENUMSTRING("EEnumType", Value);
// GAME_LOG("%s", *name);
#define GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetNameStringByIndex((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )

#define TO_TEXT(b) ((b) ? TEXT("true") : TEXT("false"))