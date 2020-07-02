// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h" 
#include "Modules/ModuleManager.h" 

DECLARE_LOG_CATEGORY_EXTERN(BishRPGEd, Log, All);

#define EDITOR_LOG(format, ...) UE_LOG(BishRPGEd, Log, TEXT(format), ##__VA_ARGS__)
#define EDITOR_LOG_FMT(format, ...) UE_LOG(BishRPGEd, Log, TEXT("%s"), *FString::Format(TEXT(format), { __VA_ARGS__ }))
#define EDITOR_WARNING(format, ...) UE_LOG(BishRPGEd, Warning, TEXT(format), ##__VA_ARGS__)
#define EDITOR_WARNING_FMT(format, ...) UE_LOG(BishRPGEd, Warning, TEXT("%s"), *FString::Format(TEXT(format), { __VA_ARGS__ }))
#define EDITOR_ERROR(format, ...) UE_LOG(BishRPGEd, Error, TEXT(format), ##__VA_ARGS__)
#define EDITOR_FATAL(format, ...) UE_LOG(BishRPGEd, Fatal, TEXT(format), ##__VA_ARGS__)
#define EDITOR_ASSERT(exp) ensure(exp)
#define EDITOR_ASSERT_FMT(exp, format, ...) do { if(!(exp)) { GAME_LOG_FMT(format, ##__VA_ARGS__); } ensure(exp); } while(0)

// FString name = GETENUMSTRING("EEnumType", Value);
// GAME_LOG("%s", *name);
#define EDITOR_GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetNameStringByIndex((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )

#define EDITOR_TO_TEXT(b) ((b) ? TEXT("true") : TEXT("false"))

class IBishrpgEd : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IBishrpgEd& Get()
	{
		return FModuleManager::LoadModuleChecked< IBishrpgEd >("bishrpgEd");
	}
	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("bishrpgEd");
	}
};
