// Fill out your copyright notice in the Description page of Project Settings.

#include "bishrpgEd.h"
#include "Modules/ModuleManager.h"

//IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, bishrpgEd, "bishrpgEd" );

DEFINE_LOG_CATEGORY(BishRPGEd);


class FBishrpgEd : public IBishrpgEd
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
IMPLEMENT_MODULE(FBishrpgEd, bishrpgEd)

void FBishrpgEd::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.) 
}
void FBishrpgEd::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading, 
	// we call this function before unloading the module. 
}
