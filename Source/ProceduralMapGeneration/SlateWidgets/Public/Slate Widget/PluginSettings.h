// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define DEFAULT_PATH FPaths::ProjectPluginsDir() + TEXT("ProceduralMapGeneration/Config/Saves/")
#define EXTENSION_NAME TEXT(".bin")

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "Serialization/BufferArchive.h"
#include "UObject/SoftObjectPtr.h"
#include "PluginSettings.generated.h"


UENUM()
enum ETestCase : uint8
{
	Undefined,
	RoomBlockValidated,
	RoomCorridorValidated,
};

/* */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Procedural Map Generation"))
class PROCEDURALMAPGENERATION_API UPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	FSoftObjectPath Up = FSoftObjectPath(TEXT("ProceduralMapGeneration/Resources/Up.Up"));

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	FSoftObjectPath Down = FSoftObjectPath(TEXT("/ProceduralMapGeneration/Resources/Down.Down"));

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	FSoftObjectPath Right = FSoftObjectPath(TEXT("/ProceduralMapGeneration/Resources/Right.Right"));

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	FSoftObjectPath Left = FSoftObjectPath(TEXT("/ProceduralMapGeneration/Resources/Left/Left"));

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	FSoftObjectPath NoExit = FSoftObjectPath(TEXT("/ProceduralMapGeneration/Resources/NoExit.NoExit"));

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	// TSoftObjectPtr<UMaterialInterface> SceneCapMaterial = FSoftObjectPath(TEXT("/Game/ProceduralMapGeneration/Resources/RenderTarget/M_UIRenderTarget.M_UIRenderTarget"));
	TSoftObjectPtr<UMaterialInterface> SceneCapMaterial;

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	TSoftClassPtr<AActor> SceneCapActor;

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	TSoftClassPtr<AProceduralGen> ProGenClass;

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	TSoftClassPtr<AActor> TilePlaneActor;
	
	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	TSoftObjectPtr<UMaterialInterface> TileMaterial;

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	TSoftObjectPtr<UMaterialInterface> TileHoverMaterial;

	UPROPERTY(Config, EditAnywhere, Category= "My Plugin")
	TSoftObjectPtr<UMaterialInterface> TileSelectionMaterial;

	//TODO: Later on as the value, it should take an enum containing the all test names
	UPROPERTY()
	mutable TMap<FString, TEnumAsByte<ETestCase>> AllTestCases;
	
	UPROPERTY()
	mutable TWeakObjectPtr<AActor> SceneCapActorInst;
	mutable TWeakObjectPtr<AProceduralGen> ProGenInst;
	
	//TODO: Delete this later on
	bool IsRoomsGenerated = false;
	
	template<typename... Arguments>
	static bool SaveData(FString SaveName, Arguments&... Args);

	template<typename... Arguments>
	static bool LoadData(FString Name, Arguments&... Args);
};

#pragma region Saving
/*Save given argument to the given path name*/
template<typename... Arguments>
bool UPluginSettings::SaveData(FString SaveName, Arguments&... Args)
{
	// Get the path to the plugins directory relative to the project
	FString DirectoryPath = DEFAULT_PATH;

	// Ensure the saves directory exists
	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*DirectoryPath))
	{
		FileManager.MakeDirectory(*DirectoryPath, true);
	}
	
	FString FileExtension = TEXT(".bin"); // or another appropriate extension
	FString FullFilePath = DirectoryPath + SaveName +  FileExtension;
	
	//Create a buffer for binary data
	FBufferArchive ToBinary;
	
	// Serialize each argument. Similar to foreach element
	(ToBinary << ... << Args);
	
	//No data were saved
	if (ToBinary.Num() <= 0 ) return false;
	
	//Save binaries to disk
	bool Result = FFileHelper::SaveArrayToFile(ToBinary,*FullFilePath);
	

	//TODO: Handle this case much later on
	// //Lastly save the path name as variable it's important when I want to load all the data ever saved
	// if (!AllSavePaths.Contains(FullFilePath))
	// {
	// 	AllSavePaths.Add(FullFilePath);
	//
	// 	FBufferArchive NewBinary;
	// 	NewBinary << AllSavePaths;
	//
	// 	FString SavePath = DirectoryPath + FString("AllSavedPaths") + FileExtension;
	// 	FFileHelper::SaveArrayToFile(NewBinary,*SavePath);
	// }
	
	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();
	
	return Result;
}

/*Load given argument to the given path name*/
template<typename... Arguments>
bool UPluginSettings::LoadData(FString Name, Arguments&... Args)
{
	TArray<uint8> BinaryArray;
	FString FinalPath = DEFAULT_PATH + *Name + EXTENSION_NAME;
	
	//Load disk data to binary array
	if (FFileHelper::LoadFileToArray(BinaryArray,*FinalPath))
	{
		//Load data
		FMemoryReader FromBinary = FMemoryReader(BinaryArray,true);
		FromBinary.Seek(0);

		//Deserialize each argument
		(FromBinary << ... << Args); 
		
		//Empty the buffer's contents and close the stream
		FromBinary.FlushCache();
		BinaryArray.Empty();
		FromBinary.Close();
		return true;
	}
	
	return false;
}

#pragma endregion Saving 


