// Fill out your copyright notice in the Description page of Project Settings.
//REMIND: TEMPORARY NOT USED
#pragma once

#include "CoreMinimal.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "UObject/Object.h"
#include "CorridorTestHandler.generated.h"

class UPluginSettings;
class ARoomActor;
/**
 * 
 */
UCLASS()
class PROCEDURALMAPGENERATION_API UCorridorTestHandler : public UObject
{
	GENERATED_BODY()
public:
	TArray<FIntPoint> VerticalUpToVerticalUp = {FIntPoint(0,-1),FIntPoint(0,-1), FIntPoint(0,-1), FIntPoint(0,-1),
		FIntPoint(0,-1), FIntPoint(0,-1), FIntPoint(0,-1),FIntPoint(0,-1), FIntPoint(0,-1), FIntPoint(0,-1)};

	TArray<FIntPoint> DownDown = {FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)
	,FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)
	,FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)};

	TArray<FIntPoint> DownDown2 = {FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)
	,FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)
	,FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)};

	TArray<FIntPoint> DownDown3 = {FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)
	,FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)
	,FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1),FIntPoint(0,1)};

	TArray<FIntPoint> VerticalUpTurnRightToVerticalUp = {FIntPoint(0,-1),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0)
	,FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(0,-1),
		FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0)
	,FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(0,-1)};

	TArray<FIntPoint> VerticalUpTurnLeftToVerticalUp = {FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(0,-1),
	FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(0,-1)};

	
	UPROPERTY()
	ARoomActor* FirstRoom;
	UPROPERTY()
	ARoomActor* SecondRoom;

	UPROPERTY()
	TArray<ARoomActor*> FirstSecRoom;

	UPROPERTY()
	TArray<ARoomActor*> SpawnedRooms;

	TArray<FVector> CenterOfEachChunk;

	UPROPERTY()
	UWorld* World;

	UPROPERTY()
	const UPluginSettings* Setting;
	
	UCorridorTestHandler();
	void Initialize(ARoomActor* FirstSelectedRoom, ARoomActor* SecondSelectedRoom);
	void Start();
	void InitWorldChunks(TArray<TArray<FIntPoint>>& AllScenarios);
	void MakePathScenario(const FVector& FirstRoomLoc, FVector& NextRoomLocation, TArray<FIntPoint>& CurrentPattern);
	ARoomActor* SpawnSecondRoom(const FVector& NextRoomLocation, ARoomActor*& NextRoom) const;
	ARoomActor* SpawnFirstRoom(const FVector& FirstRoomLoc);
	static bool MakeGivenPathFinding(TArray<FIntPoint>& CurrentPattern, ARoomActor* FirstRoomOne, ARoomActor* SecondRoomOne, const FVector& NextRoomLoc, const FVector& FirstRoomLoc);
	static FTileStruct* FillGivenCorrPattern(TArray<FTileStruct*>& RoomList, TArray<FIntPoint>& CurrentPattern, UPluginSettings* PlugSetting);
	void DestroySpawnedRooms();
	static int GetScenariosResultLength(TArray<FIntPoint> Scenario);
	
	int RoundMaxBoxExtentToNearestTileSize(int& MaxExtend) const
	{
		return MaxExtend = FMath::RoundToInt(static_cast<float>(MaxExtend) / Setting->ProGenInst->TileSizeX) * Setting->ProGenInst->TileSizeX;
	}

	/*If the found location is not placed at tile size's exponents, round it*/
	void RoundNearestTilePos(FVector& VectorToRound)
	{
		int X = FMath::RoundToInt(static_cast<float>(VectorToRound.X) / Setting->ProGenInst->TileSizeX) * Setting->ProGenInst->TileSizeX;
		int Y = FMath::RoundToInt(static_cast<float>(VectorToRound.Y) / Setting->ProGenInst->TileSizeY) * Setting->ProGenInst->TileSizeY;
		VectorToRound = FVector(X, Y, 0);
	}

	//TODO: Fix this shit tomorrow
	static inline EDirection2 DetermineFirstDirection(const TArray<FIntPoint>& CurrentPattern)
	{
		if (CurrentPattern[1] - CurrentPattern[0] == FIntPoint(0,-1)) return Dir_Up;
		if (CurrentPattern[1] - CurrentPattern[0] == FIntPoint(0,1)) return Dir_Down;
		if (CurrentPattern[1] - CurrentPattern[0] == FIntPoint(1,0)) return Dir_Right;
		if (CurrentPattern[1] - CurrentPattern[0] == FIntPoint(-1,0)) return Dir_Left;
		return Dir_None;
	}
};
