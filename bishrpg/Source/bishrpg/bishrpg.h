// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(BishRPG, Log, All);

#define GAME_LOG(format, ...) UE_LOG(BishRPG, Log, TEXT(format), __VA_ARGS__)
#define GAME_WARNING(format, ...) UE_LOG(BishRPG, Warning, TEXT(format), __VA_ARGS__)
#define GAME_ERROR(format, ...) UE_LOG(BishRPG, Error, TEXT(format), __VA_ARGS__)
#define GAME_FATAL(format, ...) UE_LOG(BishRPG, Fatal, TEXT(format), __VA_ARGS__)

