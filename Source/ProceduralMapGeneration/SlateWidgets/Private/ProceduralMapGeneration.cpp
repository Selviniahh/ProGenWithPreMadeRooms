// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralMapGeneration/SlateWidgets/Public/ProceduralMapGeneration.h"
#include "ContentBrowserModule.h"
#include "Async/Async.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/GlobalInputListener.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/ProGenWidget.h"
#define LOCTEXT_NAMESPACE "FProceduralMapGenerationModule"

class SProGenWidget;
struct FStreamableManager;

void FProceduralMapGenerationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// RegisterWindow();
	InitCBMenuExtension();
	// FSlateApplication::Get().RegisterInputPreProcessor(TSharedPtr<IInputProcessor>(MakeShareable<new FMyInputProcessor()>));
	TSharedPtr<IInputProcessor> InputProcessor = MakeShared<FMyInputProcessor>();
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);

	FMyInputProcessor::FillMap();
}

void FProceduralMapGenerationModule::ShutdownModule()
{
}

//Bind your own custom delegate to content browser module. 
void FProceduralMapGenerationModule::InitCBMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//Retrieve all the custom menu extenders from the content browser module
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//Bind your own custom delegate to content browser module. 
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FProceduralMapGenerationModule::CustomCBMenuExtender));
}

//Will be invoked when right clicked to content browser. From upper function, we already bound to content browser. This function won't create the menu itself.
//Instead it will just specify where the entity should be placed. After this function returned, the bound delegate will be invoked right away and that bound function will
//Specify exactly what the entity should be.
TSharedRef<FExtender> FProceduralMapGenerationModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender = MakeShared<FExtender>();

	MenuExtender->AddMenuExtension(FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(),
	                               FMenuExtensionDelegate::CreateRaw(this, &FProceduralMapGenerationModule::AddCBMenuEntry));
	return MenuExtender;
}

//Will be invoked right away after upper function returned. This function will create the actual menu itself.
void FProceduralMapGenerationModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Make very cool Procedural Map label")),
		FText::FromString(TEXT("Make very cool Procedural Map for tool tip")),
		FSlateIcon(), FExecuteAction::CreateRaw(this, &FProceduralMapGenerationModule::OnProGenButtonClicked)
	);
}

//Will be invoked when the option itself is clicked. This function will just invoke the given name tab. But before trying to invoke, registering that tab is required. Below the function
void FProceduralMapGenerationModule::OnProGenButtonClicked()
{
	OnSpawnProGenTab();
}


//Register the window tab with given TabId. The bound delegate itself will be finally actual content of the window. 

TSharedRef<SWindow> FProceduralMapGenerationModule::OnSpawnProGenTab()
{
	const UPluginSettings* PluginSettings = GetDefault<UPluginSettings>();
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	TArray<FSoftObjectPath> AssetsPathToLoad = {
		PluginSettings->SceneCapActor.ToSoftObjectPath(),
		PluginSettings->SceneCapMaterial.ToSoftObjectPath(),
	};


	//Set plugin settings properties
	StreamableManager.RequestAsyncLoad(AssetsPathToLoad, [this, PluginSettings]()
	{
		if (UMaterialInterface* Material = Cast<UMaterialInterface>(PluginSettings->SceneCapMaterial.Get()))
		{
			// Asynchronously load the actor
			if (UClass* SceneCapClass = Cast<UClass>(PluginSettings->SceneCapActor.Get()))
			{
				AActor* SceneCapActor = GetSceneCapActor(SceneCapClass);

				TSharedRef<SProGenWidget> ProGenWidget = SNew(SProGenWidget)
				.Material(Material)
				.SceneCapActor(SceneCapActor);

				// Make sure to run this on the game thread, as we are modifying the UI
				Async(EAsyncExecution::TaskGraphMainThread, [ProGenWidget]()
				{
					auto ProGenWindow =  SNew(SWindow)
						.Title(FText::FromString(TEXT("Procedural Map Generation")))
						.ClientSize(FVector2D(1920, 1080))
						.SupportsMaximize(true)
						.SupportsMinimize(true)
					[
						ProGenWidget
					];

					FSlateApplication::Get().AddWindow(ProGenWindow);
				});
			}
		}
	});
	return SNew(SWindow);
}


AActor* FProceduralMapGenerationModule::GetSceneCapActor(UClass* SceneCapClass)
{
	const UPluginSettings* PluginSettings = GetDefault<UPluginSettings>();
	UClass* ObjectClass = PluginSettings->SceneCapActor.Get();

	// Check if this class is a subclass of AActor
	if (!ObjectClass->IsChildOf(AActor::StaticClass())) return nullptr;

	// Spawn an instance of this actor class
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return nullptr;

	TArray<AActor*> SceneCapActors;
	UGameplayStatics::GetAllActorsOfClass(World, ObjectClass, SceneCapActors);
	if (SceneCapActors.IsEmpty())
	{
		AActor* SceneCapActorInst = World->SpawnActor<AActor>(ObjectClass, FVector(0, 0, 1848), FRotator(-90, 0, -90));
		PluginSettings->SceneCapActorInst = SceneCapActorInst;
		return SceneCapActorInst;
	}
	else
	{
		PluginSettings->SceneCapActorInst = SceneCapActors[0];
		return SceneCapActors[0];
	}
}

void FProceduralMapGenerationModule::OnTabClosed(const TSharedRef<SDockTab> Tab)
{
	if (ShouldRemoveSceneCapActor)
	{
		// GetSceneCapActor()->Destroy();
	}
	UE_LOG(LogTemp, Display, TEXT("Tab closed"));
}
#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FProceduralMapGenerationModule, ProceduralMapGeneration)
