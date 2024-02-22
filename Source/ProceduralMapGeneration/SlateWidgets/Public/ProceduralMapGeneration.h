// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UPluginSettings;

class FProceduralMapGenerationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void InitCBMenuExtension();
	TSharedRef<SWindow> OnSpawnProGenTab();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	void OnProGenButtonClicked();
	 void OnTabClosed(const TSharedRef<SDockTab> Tab);
	/*If we don't have one spawn one and then return it's reference*/
	static AActor* GetSceneCapActor(UClass* ObjectClass);
	UClass* SceneCapClass;

	bool ShouldRemoveSceneCapActor = true;

};
