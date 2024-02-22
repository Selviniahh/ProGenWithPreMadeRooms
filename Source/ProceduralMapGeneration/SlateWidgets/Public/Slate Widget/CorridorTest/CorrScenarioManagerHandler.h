// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "UObject/Object.h"
#include "CorrScenarioManagerHandler.generated.h"

class UProGenSubsystem;
enum class EDirection : uint8;
enum EDirection2 : uint8;
class UPluginSettings;
class ARoomActor;
/**
 * 
 */
UCLASS()
class PROCEDURALMAPGENERATION_API UCorrScenarioManagerHandler : public UObject
{
	GENERATED_BODY()
	
public:
	bool HandleTileSelection(ARoomActor* RoomToIgnore);
	void Initialize(ARoomActor* InFirstRoom, ARoomActor* InSecondRoom, const TArray<ARoomActor*>& PreviousSpawnedRooms);
	AActor* MakeRayCast(AActor* RoomToIgnore);
	ARoomActor* SpawnAndVisualizeRoom(ARoomActor*& Room, const FVector& SpawnLoc);
	void SpawnTiles();
	FReply HandleSecondRoomSpawning(bool IsButtonClicked = false);
	void Destruct();
	void UndoTiles();
	void DestroyPreviousExistingActors(const TArray<ARoomActor*>& PreviousSpawnedRooms);
	void SaveGivenCorrPaths();
	
	//For tile selection
	UPROPERTY()
	UMaterialInterface* TileHoverMat;
	UPROPERTY()
	UMaterialInterface* TileUnhoverMat;
	UPROPERTY()
	UMaterialInterface* TileSelectMat;
	UPROPERTY()
	AActor* LastHitTile;
	UPROPERTY()
	TArray<AActor*> SelectedTiles;
	UPROPERTY()
	AActor* SceneCapInst;
	UPROPERTY()
	UWorld* World;
	UPROPERTY()
	TArray<ARoomActor*> SpawnedRooms;
	UPROPERTY()
	TArray<ARoomActor*> PrevSpawnedRooms;
	UPROPERTY()
	const UPluginSettings* PlugSetting;
	UPROPERTY()
	ARoomActor* FirstRoom;
	UPROPERTY()
	ARoomActor* SecondRoom;
	UPROPERTY()
	TArray<AActor*> SpawnedTiles;
	
	FSlateFontInfo PropertyTextFont;
	FVector RayCastEndLoc;
	TArray<FVector> SpawnedLocations;
	TArray<FIntPoint> SelectedTilePoints;
	bool DoOnce = true;

	UPROPERTY()
	UProGenSubsystem* ProGenSubsystem;
	
	//Declare 3x3 grid of tiles
	TArray<FIntPoint> SpawnTileOffsets = {FIntPoint(0,0), FIntPoint(0,1), FIntPoint(-1,1), FIntPoint(-1,0),
		FIntPoint(-1,-1), FIntPoint(-0,-1), FIntPoint(1,-1),FIntPoint(1,0), FIntPoint(1,-1)};

	static EDirection2 GetTheTilesDirection(const FIntPoint& PreviousPoint, const FIntPoint& CurrentPoint)
	{
		FIntPoint Result = PreviousPoint - CurrentPoint;

		if (Result == FIntPoint(0, 1))
			return EDirection2::Dir_Up;
		if (Result == FIntPoint(0, -1))
			return EDirection2::Dir_Down;
		if (Result == FIntPoint(1, 0))
			return EDirection2::Dir_Right;
		if (Result == FIntPoint(-1, 0))
			return EDirection2::Dir_Left;
		
		return EDirection2::Dir_Down;
	}

	static EDirection2 NormalDirToEDirection2(TEnumAsByte<Direction> Direction)
	{
		if (Direction == HorizontalLeft)
			return Dir_Left;
		if (Direction == HorizontalRight)
			return Dir_Right;
		if (Direction == VerticalUp)
			return Dir_Up;
		if (Direction == VerticalDown)
			return Dir_Down;
		return Dir_None;
	}

	TSharedPtr<FSlateBrush>GetDirectionMaps(TEnumAsByte<Direction> Direction, TSharedPtr<FSlateBrush>& Image) const;

	
};
