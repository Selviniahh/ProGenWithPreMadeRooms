// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomActor.h"
#include "GameFramework/Actor.h"
#include "Stats/Stats.h"
#include "ProceduralGen.generated.h"

//Todo: decide to delete this struct and their uses later. 
struct FCorridorPathParams
{
	int StartX;
	int StartY;
	int GoalX;
	int GoalY;
	FIntPoint StartOffset;
	FIntPoint EndOffset;
	bool SpawnCorr;
	int MaxIterationAmount;
	ARoomActor* OverlappedRoom;

	// Constructor to initialize with initial values
	FCorridorPathParams(int startX, int startY, int goalX, int goalY, FIntPoint startOffset, FIntPoint endOffset, bool spawnCorr, int maxIterationAmount, ARoomActor* overlappedRoom)
		: StartX(startX), StartY(startY), GoalX(goalX), GoalY(goalY), StartOffset(startOffset), EndOffset(endOffset), SpawnCorr(spawnCorr), MaxIterationAmount(maxIterationAmount), OverlappedRoom(overlappedRoom)
	{}
};

USTRUCT()
struct FRoomConnection
{
	GENERATED_BODY()

	FVector StartPoint;  // The exit point of a room
	FVector EndPoint;    // The entry point of the next room
	FIntPoint PathStartOffset; 
	FIntPoint PathEndOffset;
	int MaxCheckAmount;

	UPROPERTY()
	ARoomActor* OverlappedRoom;
	FString RoomName;
};

//TODO: There's a major discrepancy. There's EDirection2 and EDirection same direction enum but different names. Remove either of them and leave single direction enum for both room and this class 
UENUM()
enum EDirection2 : uint8
{
	Dir_Left,
	Dir_Right,
	Dir_Up, 
	Dir_Down,
	Dir_None
};

USTRUCT()
struct FPathNode
{
	GENERATED_BODY()
	bool Visited = false;
	int X = 0;
	int Y = 0;
	int HCost = 0;
	int GCost = 0;
	FPathNode* Parent = nullptr;
	EDirection2 Direction = EDirection2::Dir_None;
	FRotator Rotation = FRotator(0,0,0);
	static int GetHCost(int StartX, int StartY, int GoalX, int GoalY)
	{
		return FMath::Abs(StartX - GoalX) + FMath::Abs(StartY - GoalY);
	}
	int FCost()
	{
		return GCost + HCost;
	}
};

//REMIND: This struct in total 104 bytes.
//Merged Path Into Tile
USTRUCT()
struct FTileStruct
{
	GENERATED_BODY()
	bool Blocked = false;
	bool Path = false;
	FVector Location = FVector(0,0,0); //Rest is for pathfinding
	
	bool Visited = false;
	int X = 0;
	int Y = 0;
	int HCost = 0;
	int GCost = 0;
	FTileStruct* Parent;
	EDirection2 Direction = EDirection2::Dir_None;
	FRotator Rotation = FRotator(0,0,0);
	bool IsTurnCorridor = false;

	static int GetHCost(int StartX, int StartY, int GoalX, int GoalY)
	{
		return FMath::Abs(StartX - GoalX) + FMath::Abs(StartY - GoalY);
	}
	int FCost()
	{
		return GCost + HCost;
	}
};

struct FRoomConnection;
enum Direction : int;
struct FTileStruct;
class ARoomActor;

DECLARE_STATS_GROUP(TEXT("CustomStats"), STATGROUP_CustomStats, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("GenerateMap"), STAT_GenerateMap, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("InitWorldTiles"), STAT_InitWorldTiles, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("SpawnTestCollisionObjects"), STAT_SpawnTestCollisionObjects, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("SpawnFirstRoom"), STAT_SpawnFirstRoom, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("RoomSpawning"), STAT_RoomSpawning, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("MakeSideBranchFromLargeRoom"), STAT_MakeSideBranchFromLargeRoom, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("SpawnFirstBranchRoom"), STAT_SpawnFirstBranchRoom, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("SpawnBranchRoom"), STAT_SpawnBranchRoom, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("SpawnNoExitDoor"), STAT_SpawnNoExitDoor, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("IsColliding"), STAT_IsColliding, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("IsEndSocketOverlapping"), STAT_IsEndSocketOverlapping, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("ForEachTileInRoom"), STAT_ForEachTileInRoom, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("SelectRoomWithDirection"), STAT_SelectRoomWithDirection, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("MoveOverlappedRoom"), STAT_MoveOverlappedRoom, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("UnBlockLastRoomItsCorridorAndDestroy"), STAT_UnBlockLastRoomItsCorridorAndDestroy, STATGROUP_CustomStats);
DECLARE_CYCLE_STAT(TEXT("CanMakeCorridorPathBeforeSpawning"), STAT_CanMakeCorridorPathBeforeSpawning, STATGROUP_CustomStats);

UCLASS()
class PROCEDURALMAPGENERATION_API AProceduralGen : public AActor
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	TArray<ARoomActor*> CastedRoomDesigns;

	TArray<TArray<FTileStruct>> Tiles;

	Direction NextRoomExitTag;
	Direction NextRoomEnterTag;

	UPROPERTY()
	UWorld* World;
	
	/*Add all the rooms to be randomly select and spawned.*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 2))
	TArray<TSubclassOf<ARoomActor>> RoomDesigns;
	
	/*Total number of rooms to be spawned*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 3))
	int NumberOfRooms = 10;
	
	/*Given amount of tiles will be included to make all the algorithm checks. Make sure it's long enough if you planning to spawn large amount of rooms*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 4))
	int MapSizeX = 1000;
	
	/*Given amount of tiles will be included to make all the algorithm checks. Make sure it's long enough if you planning to spawn large amount of rooms*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 5))
	int MapSizeY = 1000;
	
	/*The tile size for entire map. It's important to give precise small amount otherwise there will be tile offset issues when a path trying to be found for connecting two rooms with corridors.*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 6))
	int TileSizeX = 16;
	
	/*The tile size for entire map. It's important to give precise small amount otherwise there will be tile offset issues when a path trying to be found for connecting two rooms with corridors.*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 7))
	int TileSizeY = 16;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 7))
	FVector MapCenter = FVector(0,0,0);

	UPROPERTY()
	ARoomActor* LastSelectedOverlappingRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int MaxSideBranchRoom = 50;

	int OverlappedRoomMoverCounter = 0;

	//I know this is so stupid. I first need to learn transformations and then apply those. Workarounds are just workarounds 
	FRotator DefaultRotation = FRotator(0.0f, 0.0f, -90.0f);
	
	TArray<FIntPoint> BlockedTileHolder;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ARoomActor> StraightCorrClass;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ARoomActor> TurnCorridorClass;

	UPROPERTY()
	ARoomActor* LastSpawnedHorizontalStraightCorr;
	
	/*Which room set as blocked*/
	// TMap<TWeakObjectPtr<ARoomActor>, TArray<FIntPoint>> RoomExclusions;

	FVector ZOffset = FVector(0,0,3);

	UPROPERTY()
    	TArray<ARoomActor*> LargeRooms;

	/*Make sure Room sequence length is equal to -1 Number of rooms. Secondly, give index numbers of Room designs. */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 1))
	TArray<TSubclassOf<ARoomActor>> DebugRoomSequence;
	
	/*For debug purposes the location for each given BlockRoom's location. Block room will be spawned at given location*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 2))
	TArray<FVector> BlockRoomLocations;
	
	/*Solely for debug purposes. The added room will be used to check overlaps with original rooms. Intended to force rooms to have overlaps. */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 3))
	TSubclassOf<ARoomActor> BlockRoom;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 4))
	bool VisualizeOverlaps = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 5))
	bool VisualizeVisited = false;

	/*When a room overlapped, this option will visualize all the areas it searched to spawn overlapped room. To find out how much you need to increase MaxOverlappedRoomIterate, enable this option*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 6))
	bool VisualizeOverlappedRoomTiles = false;

	/*Begin and End socket visualization color is Cyan*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 7))
	bool VisualizeBeginAndEndTiles = true;

	/*Path visualization color is red*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 8))
	bool VisualizeCorridorPath = true;
	
	/*All excluded tiles are purple*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 9))
	bool VisualizeAllExclusions = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 10))
	bool VisualizeEndSocketOverlapCheck = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Debugging", meta=(DisplayPriority = 11))
	bool SpawnCorridor = true;

	/* Stored to make calculations for Last Spawned Room. (Connect LastSpawnedRoom's exit socket with new room)*/
	UPROPERTY()
	ARoomActor* LastSpawnedRoom;

	/*Room spawn counter. It will increment as new rooms are spawned*/
	int SpawnedRoomCount;
	int LargeRoomCounter = 0;

	FVector TileBoxExtends;

	int CurrentIndex = 0;
	int CurrentManualSideBranchIndex = 0;

	/* Determines the buffer size when a room is overlapped and moved to a non-overlapping position.A smaller value might cause the overlapped room to be squeezed closer to other rooms.
		 * A larger value will create a larger distance from other rooms.*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 8))
	int BufferSize = 6;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 10))
	int FindCorridorPathLimit = 5000;
	
	int NumOfSideBranchRoom;

	int FoundPathCost;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 3))
	int MaxLargeRoomCount = 5;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="General Map settings", meta=(DisplayPriority = 4))
	int BranchLength = 4;

	//TODO: This is the early return for MoveOverlappedRoom. It's not a good practice to use like this but I don't have any other option.
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int MaxMoveOverlappedRoomIterateNormalRoom = 100000;

	//Same as the above. I decided to use much smaller amount for branch rooms to increase performance little more 
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int MaxMoveOverlappedRoomIterateBranchRoom = 15000;

	UPROPERTY()
	ARoomActor* SelectedLargeRoom;

	UPROPERTY()
	TArray<ARoomActor*> SpawnedRooms;
	
	UPROPERTY()
	TArray<ARoomActor*> LargeRoomsToBeAdded;
	UPROPERTY()
	TArray<ARoomActor*> RoomsToBeRemoved;

	TArray<FIntPoint> MoveOverlapRoomLocationTiles;
	UPROPERTY()
	ARoomActor* CastedTurnCorridor;

	UPROPERTY()
	TArray<ARoomActor*> AllSpawnedCorridors;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Corridor path finding", meta=(DisplayPriority = 1))
	bool ApplyTurnPenalty = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Corridor path finding", meta=(DisplayPriority = 2))
	int TurnPenaltyAmount = 5;
	
	UPROPERTY(EditAnywhere)
	bool OnlyMakeCorridorCheck;


	//Now I need to give custom pattern but I really not sure how to implement this
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Make custom path scenarios", meta=(DisplayPriority = 1))
	bool DisplayCustomPathScenarios = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Make custom path scenarios", meta=(DisplayPriority = 2))
	TArray<TSubclassOf<ARoomActor>> FixedTwoRoomActor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Make custom path scenarios", meta=(EditCondition = "DisplayCustomPathScenarios", EditConditionHides, DisplayPriority = 3))
	TArray<FIntPoint> VerticalUpToVerticalUp = {FIntPoint(0,-1),FIntPoint(0,-1), FIntPoint(0,-1), FIntPoint(0,-1),
		FIntPoint(0,-1), FIntPoint(0,-1), FIntPoint(0,-1),FIntPoint(0,-1), FIntPoint(0,-1), FIntPoint(0,-1)};

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Make custom path scenarios", meta=(EditCondition = "DisplayCustomPathScenarios", EditConditionHides, DisplayPriority = 4))
	TArray<FIntPoint> VerticalUpTurnRightToVerticalUp = {FIntPoint(0,-1),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0)
	,FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(1,0),FIntPoint(0,-1),
		FIntPoint(0,-1),FIntPoint(0,-1)};

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Make custom path scenarios", meta=(EditCondition = "DisplayCustomPathScenarios", EditConditionHides, DisplayPriority = 5))
	TArray<FIntPoint> VerticalUpTurnLeftToVerticalUp = {FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(0,-1),
	FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(-1,0),FIntPoint(0,-1),FIntPoint(0,-1),FIntPoint(0,-1)};

	
	
	
	virtual bool CanEditChange(const FProperty* InProperty) const override;

	TArray<FIntPoint> AddVisitedTiles;
	TArray<FTileStruct*> Path;
	AProceduralGen();
	virtual void Tick(float DeltaTime) override;
	ARoomActor* SpawnBranchRoom(Direction ExpDirection,  FVector SpawnLocation, int& SpawnCounter, bool& EndBranch, TArray<FName>& SocketComps, Direction LargeRoomSceneComp, TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms = nullptr);
	ARoomActor* SpawnFirstBranchRoom(Direction Direction, FVector SpawnLoc, int& SpawnCounter, TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms = nullptr);
	void MakeSideBranchFromLargeRoom();
	void UnBlockLastRoomItsCorridorAndDestroy();
	void GenerateMap();
	void InitWorldTiles();
	TArray<FIntPoint> SetTilesBlocked(ARoomActor* Room, const FVector& SpawnLoc, const FRotator& SpawnRot);
	bool IsColliding(ARoomActor* Room, const FVector& SpawnLoc, const FRotator& SpawnRot);
	static FVector CalculateTopLeftCorner(const ARoomActor* Room, const FVector& WorldLoc, const FRotator& Rotation, FVector& BoxExtends);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	//TODO: This is the function needs very heavy optimization
	bool MoveOverlappedRoom(ARoomActor* NextRoom, FVector& NextRoomLocation);
	
	bool ConnectRoomsWithCorridors(ARoomActor*& Room);
	void SocketExclusionForLargeRoom(ARoomActor* Room);
	
	/*After pathfinding is finished, it will spawn the corridors on the found path*/
	bool SpawnCorridors(int GoalX, int GoalY, ARoomActor* OverlappedRoom, const bool& CheckZigZag);

	/**Checks if the buffer zone around a given room collides in the world. //Same as IsColliding. Just setting given indexes to be blocked.*/
	bool IsBufferZoneColliding(ARoomActor* Room, FVector SpawnLoc);
	int MakeCorridorPathVisualization(ARoomActor* OverlappedRoom, FTileStruct* Current);

	/*After a room is overlapped and it moved to another position where it doesn't overlap anymore, a path will be try to be found from previous room's door socket exit to new room's door socket enter with using a* pathfinding. */
	bool FindCorridorPath(int StartX, int StartY, int GoalX, int GoalY, FIntPoint StartOffset, FIntPoint EndOffset, bool SpawnCorr, int MaxIterationAmount, ARoomActor* OverlappedRoom);

	void VisualizeBeginEndTiles(ARoomActor* NextRoom, const FRoomConnection& Connection);
	bool RoomSpawning(Direction EndSocketDirection);
	bool CanMakeCorridorPathBeforeSpawning(ARoomActor*& NextRoom, const FVector& NextRoomLocation);
	static TSharedPtr<TArray<ARoomActor*>> GetBPSpecificBranchRooms(ARoomActor*& NextRoom, const FString& SocketName);
	void InitAndSpawnRoom(ARoomActor*& NextRoom, const FVector& NextRoomLocation, const FRotator& Rotation, const bool IsOverlapped, bool AddLargeRoomTempArray = false);

	/**For Socket enter and exit, given amounts will be discarded from tiles and unmark as blocked.*/
	void SetSocketExclusion(ARoomActor* Room, FVector SpawnLoc);
	void ResetAllVisited();

	void SetExclusion(ARoomActor* Room, const FIntPoint& Index, const FIntPoint& ExclusionOffset, TArray<FIntPoint> Exclusions);

	/*There might be always edge cases where a room is spawned without overlap but there's not enough space left to make a connection from end socket to another room's enter socket. To prevent this problem,
	 *it will be checked end socket has minimum 3 available tile buffer width. If not, the room will be acted as overlapped*/
	bool IsEndSocketOverlapping(ARoomActor* NextRoom, const FVector& SpawnLoc);

	void SpawnTestCollisionObjects();
	void SpawnFirstRoom();
	void DestroyLastRoomSpawnNoExit(FVector& SpawnLocation,const FRotator& Rotation, int& SpawnCounter, bool CanSpawnLargeRoom, TArray<ARoomActor*>& CustomRoomDesigns, bool& OnlySpawnNoExit, ARoomActor*& NextRoom);
	virtual void BeginPlay() override;
	FRoomConnection CalculatePathInfo(ARoomActor* NextRoom);
	void SpawnNonOverlappedRoom(const FRotator& Rotation, const FVector& NextRoomLocation, ARoomActor*& NextRoom);
	void SpawnDoors(const FRotator& Rotation, const FVector& NextRoomLocation, ARoomActor*& NextRoom, bool OnlySpawnEnterDoor);
	void SpawnNoExitDoor(ARoomActor* LargeRoom, const FName& SceneTag, const FVector& SocketLocation);
	bool SpawnOverlappedRoom(const FRotator& Rotation, FVector NextRoomLocation, ARoomActor*& NextRoom);
	
	/*Room actors are 90 degree rotated to be top down in BP. Therefore swapping is required*/
	static void SwapInvZYaxis(FVector& VectorToSwap);
	static FVector SwapZYaxis(FVector VectorToSwap, const FVector& SpawnLoc)
	{
		float X = SpawnLoc.X + VectorToSwap.X;
		float Y = SpawnLoc.Y - VectorToSwap.Z;
		return VectorToSwap = FVector(X,Y,SpawnLoc.Z);
	};

	ARoomActor* SelectRoomWithDirection(Direction EndSocketDirection, bool CanSpawnLargeRoom, bool OnlySpawnNoExit, TArray<ARoomActor*>* CustomArray = nullptr, TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms = nullptr);

	/*Iterate over all the tiles covered by the box component. Return all the relative tile indexes */
	void ForEachTileInRoom(const ARoomActor* Room, const FVector& SpawnLoc,const FRotator& Rotation, const TFunction<void(int X, int Z)>& TileAction);

	void VisualizeTiles();
	
	void UnBlockTiles(TArray<FIntPoint> BlockedTiles)
	{
		for (auto BlockedTile : BlockedTiles)
		{
			Tiles[BlockedTile.X][BlockedTile.Y].Blocked = false; 
		}
	}
    
	/*Convert given world position to relative index points */
	inline FVector IndexToWorld(const int X, const int Y) const
	{
		return FVector(X * TileSizeX, Y * TileSizeY, 0);
	}
	
	/*Convert given relative index points to world location. Z AXIS IGNORED */
	inline FIntPoint WorldToIndex(const FVector& WorldLocation) const
	{
		int TileX = FMath::RoundToInt(WorldLocation.X / TileSizeX);
		int TileY = FMath::RoundToInt(WorldLocation.Y / TileSizeY);
		
		return FIntPoint(TileX,TileY);
	}

	inline bool IsValid(const int X, const int Y) const
	{
		return X >= 0 && X < MapSizeX && Y >= 0 && Y < MapSizeY;
	}

	static inline Direction ExpectedDirection(const Direction Tag)
	{
		if (Tag == HorizontalRight) return HorizontalLeft;
		if (Tag == HorizontalLeft) return HorizontalRight;
		if (Tag == VerticalUp) return VerticalDown;
		if (Tag == VerticalDown) return VerticalUp;
		return {};
	}

	static inline Direction TagToDirection(const FName Tag)
	{
		if (Tag == "HorizontalRight" || Tag == "SideRight") return HorizontalRight;
		if (Tag == "HorizontalLeft" || Tag == "SideLeft") return HorizontalLeft;
		if (Tag == "VerticalUp" || Tag == "StraightUp") return VerticalUp;
		if (Tag == "VerticalDown" || Tag == "StraightDown") return VerticalDown;
		return {};
	}

	//These all for spawning corridors for found path. 
	inline void DeterminePathDirection(FTileStruct* Current, int X, int Y, FTileStruct* NewNode)
	{
		if (X > Current->X)
		{
			NewNode->Direction = EDirection2::Dir_Right;
			NewNode->Rotation = FRotator(0, 0.0f, -90.0f);
			return;
		}

		else if (X < Current->X)
		{
			NewNode->Direction = EDirection2::Dir_Left;
			NewNode->Rotation = FRotator(0, 0.0f, -90.0f);
			return;
		}

		// Moving vertically
		else if (Y > Current->Y)
		{
			NewNode->Direction = EDirection2::Dir_Down;
			NewNode->Rotation = FRotator(0, -90.0f, -90.0f);
			return;
		}
		else if (Y < Current->Y)
		{
			NewNode->Direction = EDirection2::Dir_Up;
			NewNode->Rotation = FRotator(0, 90.0f, -90.0f); 
			return;
		}
		
		NewNode->Rotation = FRotator(50,50,50);
	}
	inline FRotator DetermineFirstTurnRotation(EDirection2 NextDir)
	{
		if (LastSpawnedRoom->ExitSocketDirection == HorizontalRight) 
		{
			if (NextDir == Dir_Up) return FRotator(0, 0, -90);
			if (NextDir == Dir_Down) return FRotator(0, -90, -90);
		}
		else if (LastSpawnedRoom->ExitSocketDirection == HorizontalLeft) 
		{
			if (NextDir == Dir_Down) return FRotator(0, -180, -90);
			if (NextDir == Dir_Up) return FRotator(0, -90, -90);
			if (NextDir == Dir_Left) return FRotator(0, 0, -90); //(Pitch=0.000000,Yaw=0.000000,Roll=-90.000000)
		}

		else if (LastSpawnedRoom->ExitSocketDirection == VerticalUp) 
		{
			if (NextDir == Dir_Right) return FRotator(0, 0, -90);
			if (NextDir == Dir_Left) 	return FRotator(0, -90, -90);
		}

		else if (LastSpawnedRoom->ExitSocketDirection == VerticalDown) 
		{
			if (NextDir == Dir_Right)	return  FRotator(0, -270, -90);
			if (NextDir == Dir_Left) 	return FRotator(0, 0, -90);
		}
		return FRotator(31,31,31);
	}
	inline EDirection2 DetermineFirstDirection(EDirection2 NextDir)
	{
		if (LastSpawnedRoom->ExitSocketDirection == HorizontalRight)
		{
			if (NextDir == Dir_Right) return Dir_Right;
			if (NextDir == Dir_Down) return Dir_Left;
			if (NextDir == Dir_Up) return Dir_Down;
			if (NextDir == Dir_Down) return Dir_Up;
		}
		return Dir_None;
	}
	
	inline FRotator DetermineMiddleTurnRotation(EDirection2 CurrDir, EDirection2 NextDir)
	{
		if (CurrDir == Dir_Up && NextDir == Dir_Left)		return FRotator(0,270,-90);
		if (CurrDir == Dir_Down && NextDir == Dir_Right) 	return  FRotator(0,90,-90);
		if (CurrDir == Dir_Left && NextDir == Dir_Up) 		return  FRotator(0,-270,-90);
		if (CurrDir == Dir_Up && NextDir == Dir_Right) 		return  FRotator(0,-180,-90);
		if (CurrDir == Dir_Right && NextDir == Dir_Up) 		return  FRotator(0,0,-90);
		if (CurrDir == Dir_Right && NextDir == Dir_Down) 	return  FRotator(0,270,-90);
		if (CurrDir == Dir_Down && NextDir == Dir_Left) 	return  FRotator(0,0,-90);
		if (CurrDir == Dir_Left && NextDir == Dir_Down) 	return  FRotator(0,180,-90);
		return FRotator(31,31,31);
	}
	inline FRotator DetermineLastCorrRotation(EDirection2 LastDir)
	{
		if (LastDir == EDirection2::Dir_Up && NextRoomEnterTag == HorizontalLeft) return FRotator(0, 180, -90); 
		if (LastDir == EDirection2::Dir_Down && NextRoomEnterTag == HorizontalRight) return FRotator(0, 0, -90);
		if (LastDir == EDirection2::Dir_Left && NextRoomEnterTag == VerticalUp) return FRotator(0, -180, -90);
		if (LastDir == EDirection2::Dir_Up && NextRoomEnterTag == HorizontalRight) return FRotator(0, -90, -90); //This has been changed
		if (LastDir == EDirection2::Dir_Right && NextRoomEnterTag == VerticalUp) return FRotator(0, -90, -90);
		if (LastDir == EDirection2::Dir_Right && NextRoomEnterTag == VerticalDown) return FRotator(0, 0, -90); 
		if (LastDir == EDirection2::Dir_Right && NextRoomEnterTag == HorizontalLeft) return FRotator(0, 0, -90);
		if (LastDir == EDirection2::Dir_Left && NextRoomEnterTag == HorizontalRight) return FRotator(0, 0, -90);
		if (LastDir == EDirection2::Dir_Down && NextRoomEnterTag == VerticalUp) return FRotator(0, 180, -90);
		if (LastDir == EDirection2::Dir_Down && NextRoomEnterTag == HorizontalLeft) return FRotator(0, 90, -90); 
		if (LastDir == EDirection2::Dir_Up && NextRoomEnterTag == VerticalDown) return FRotator(0, 90, -90);
		if (LastDir == EDirection2::Dir_Left && NextRoomEnterTag == VerticalDown) return FRotator(0, 90, -90);
		return FRotator(31, 31, 31);
	}
	inline EDirection2 ConvertNextRoomExitTagToDirection()
	{
		if (NextRoomExitTag == HorizontalRight) return EDirection2::Dir_Right;
		if (NextRoomExitTag == HorizontalLeft)	return EDirection2::Dir_Left;
		if (NextRoomExitTag == VerticalUp) return EDirection2::Dir_Up;
		if (NextRoomExitTag == VerticalDown) return EDirection2::Dir_Down;
		return EDirection2::Dir_None;
	}
	inline EDirection2 ConvertNextRoomEnterTagToDirection()
	{
		if (NextRoomEnterTag == HorizontalRight) return EDirection2::Dir_Right;
		if (NextRoomEnterTag == HorizontalLeft)	return EDirection2::Dir_Left;
		if (NextRoomEnterTag == VerticalUp) return EDirection2::Dir_Up;
		if (NextRoomEnterTag == VerticalDown) return EDirection2::Dir_Down;
		return EDirection2::Dir_None;
	}

	//TODO: Later on change this name to Expection 
	static inline bool LastCorrException(EDirection2 CurrDir, EDirection2 NextRoomEnterDir)
	{
		if (CurrDir == Dir_Down && NextRoomEnterDir == Dir_Up) return true;
		if (CurrDir == Dir_Up && NextRoomEnterDir == Dir_Down) return true;
		if (CurrDir == Dir_Right && NextRoomEnterDir == Dir_Left) return true;
		if (CurrDir == Dir_Left && NextRoomEnterDir == Dir_Right) return true;

		return false;
	}

	static inline FName DirectionToTag(EDirection2 Direction)
	{
		if (Direction == Dir_Right) return "SideRight";
		if (Direction == Dir_Left) return "SideLeft";
		if (Direction == Dir_Up) return "StraightUp";
		if (Direction == Dir_Down) return "StraightDown";
		return "Null";
	}

	static inline EDirection2 DetermineFirstDirection(Direction ExitSocDir)
	{
		switch (ExitSocDir)
		{
			case HorizontalLeft : return Dir_Left;
			case HorizontalRight : return Dir_Right;
			case VerticalUp : return Dir_Up;
			case VerticalDown : return Dir_Down;
		}
		return {};
	}

protected:
	// Called when the game starts or when spawned
	

private:
};
