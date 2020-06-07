// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(BishRPG, Log, All);

#define GAME_LOG(format, ...) UE_LOG(BishRPG, Log, TEXT(format), ##__VA_ARGS__)
#define GAME_LOG_FMT(format, ...) UE_LOG(BishRPG, Log, TEXT("%s"), *FString::Format(TEXT(format), { __VA_ARGS__ }))
#define GAME_WARNING(format, ...) UE_LOG(BishRPG, Warning, TEXT(format), ##__VA_ARGS__)
#define GAME_WARNING_FMT(format, ...) UE_LOG(BishRPG, Warning, TEXT("%s"), *FString::Format(TEXT(format), { __VA_ARGS__ }))
#define GAME_ERROR(format, ...) UE_LOG(BishRPG, Error, TEXT(format), ##__VA_ARGS__)
#define GAME_FATAL(format, ...) UE_LOG(BishRPG, Fatal, TEXT(format), ##__VA_ARGS__)
#define GAME_ASSERT(exp) ensure(exp)
#define GAME_ASSERT_FMT(exp, format, ...) do { if(!(exp)) { GAME_LOG_FMT(format, ##__VA_ARGS__); } ensure(exp); } while(0)

// FString name = GETENUMSTRING("EEnumType", Value);
// GAME_LOG("%s", *name);
#define GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetNameStringByIndex((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )


#define TO_TEXT(b) ((b) ? TEXT("true") : TEXT("false"))


#if WITH_EDITOR
class TimeSpan {
public:
	TimeSpan(const FString& label)
	{
		Label = label;
		StartTime = FDateTime::Now();
	}
	~TimeSpan()
	{
		FTimespan RemainingTimespan = FDateTime::Now() - StartTime;
		double RemainingSeconds = RemainingTimespan.GetTotalMilliseconds();
		GAME_LOG("Timespan(%s) %fms", *Label, RemainingSeconds);
	}

	FString   Label;
	FDateTime StartTime;
};

#define DEBUG_SCOPE_TIME_SPAN_IMPL2(label, line) TimeSpan __timespan##line(label);
#define DEBUG_SCOPE_TIME_SPAN_IMPL1(label, line) DEBUG_SCOPE_TIME_SPAN_IMPL2(label, line)
#define DEBUG_SCOPE_TIME_SPAN(label) DEBUG_SCOPE_TIME_SPAN_IMPL1(label, __LINE__)

#else

#define DEBUG_SCOPE_TIME_SPAN(...) (void)0

#endif