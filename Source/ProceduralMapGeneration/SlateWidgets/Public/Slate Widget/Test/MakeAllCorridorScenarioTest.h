// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "UObject/Object.h"
#include "MakeAllCorridorScenarioTest.generated.h"

class AProceduralGen;
class ARoomActor;
/**
 * 
 */
UCLASS()
class PROCEDURALMAPGENERATION_API UMakeAllCorridorScenarioTest : public UObject
{
	GENERATED_BODY()
public:
	UMakeAllCorridorScenarioTest();
	void Initialize(AProceduralGen* InProceduralGenInstance, TArray<TSubclassOf<ARoomActor>> FixedTwoRoomActors);
	void Start();
	void SpawnSecondRoom(const FVector& NextRoomLocation, ARoomActor*& NextRoom);
	void SpawnFirstRoom(const FVector& FirstRoomLoc);
	void MakePathScenario(const FVector& FirstRoomLoc, const FVector& NextRoomLocation, TArray<FIntPoint>& CurrentPattern);
	FTileStruct* FillGivenCorrPattern(TArray<FTileStruct*>& RoomList, TArray<FIntPoint>& CurrentPattern, AProceduralGen* ProceduralGen);
	bool MakeGivenPathFinding(TArray<FIntPoint>& CurrentPattern, ARoomActor* NextRoom, AProceduralGen* ProceduralGen);
	void InitWorldChunks(TArray<TArray<FIntPoint>>& AllScenarios);
	
	FVector RoundBoxExtentToNearestTileSize(const FIntPoint& BoxExtent)
	{
		int X = FMath::RoundToInt(static_cast<float>(BoxExtent.X * 2) / ProGen->TileSizeX) * ProGen->TileSizeX;
		int Y = FMath::RoundToInt(static_cast<float>(BoxExtent.Y * 2) / ProGen->TileSizeY) * ProGen->TileSizeY;
		return FVector(X, Y, 0);
	}

	void RoundNearestTilePos(FVector& VectorToRound)
	{
		int X = FMath::RoundToInt(static_cast<float>(VectorToRound.X) / ProGen->TileSizeX) * ProGen->TileSizeX;
		int Y = FMath::RoundToInt(static_cast<float>(VectorToRound.Y) / ProGen->TileSizeY) * ProGen->TileSizeY;
		VectorToRound = FVector(X, Y, 0);
	}

	UPROPERTY()
	AProceduralGen* ProGen;

	UPROPERTY()
	TArray<ARoomActor*> FixedRoomActor;

	FIntPoint LargestExtends = FIntPoint(0,0);
	TArray<FVector> CenterOfEachChunk;
};



