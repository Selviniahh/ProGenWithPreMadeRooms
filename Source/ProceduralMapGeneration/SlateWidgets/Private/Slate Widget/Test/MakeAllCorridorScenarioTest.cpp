// Fill out your copyright notice in the Description page of Project Settings.

//TODO: Delete All later probably I will delete all 
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/Test/MakeAllCorridorScenarioTest.h"

#include "ProceduralMapGeneration/Procedural Generation/ProceduralGen.h"
#include "Components/BoxComponent.h"

UMakeAllCorridorScenarioTest::UMakeAllCorridorScenarioTest()
{
}

void UMakeAllCorridorScenarioTest::Initialize(AProceduralGen* InProceduralGenInstance, TArray<TSubclassOf<ARoomActor>> FixedTwoRoomActors)
{
	//Assign all the elements of the array
	for (int i = 0; i < 2; ++i)
	{
		FixedRoomActor.Add(Cast<ARoomActor>(FixedTwoRoomActors[i]->GetDefaultObject()));
	}

	ProGen = InProceduralGenInstance;
}

//TODO: I Guess I will need to entirely delete here
void UMakeAllCorridorScenarioTest::Start()
{
	TArray<TArray<FIntPoint>> AllScenarios;
	AllScenarios.Add(ProGen->VerticalUpToVerticalUp);
	AllScenarios.Add(ProGen->VerticalUpTurnRightToVerticalUp);
	AllScenarios.Add(ProGen->VerticalUpTurnLeftToVerticalUp);
	
	InitWorldChunks(AllScenarios);
	
	for (auto AllScenario : AllScenarios)
	{
		FVector FirstRoomLoc;
		FIntPoint Result = FIntPoint(0,0);
		for (auto Pattern : AllScenario)
		{
			Result += Pattern;
		}

		if (LargestExtends != FIntPoint(0,0)) //TODO: Completely remove LargestExtends
		{
			FirstRoomLoc = CenterOfEachChunk[0];
			CenterOfEachChunk.RemoveAt(0);
		}
		else
		{
			FirstRoomLoc = CenterOfEachChunk[0];
			CenterOfEachChunk.RemoveAt(0);
		}
	
		FVector NextRoomLoc = ProGen->SwapZYaxis(FixedRoomActor[0]->DoorSocketExit->GetRelativeLocation(),FirstRoomLoc) + ProGen->IndexToWorld(Result.X,Result.Y);
		// FVector NextRoomLoc = FirstRoomLoc + ProGen->IndexToWorld(Result.X,Result.Y);
		MakePathScenario(FirstRoomLoc,NextRoomLoc, AllScenario);
	}
	
	
}

void UMakeAllCorridorScenarioTest::InitWorldChunks(TArray<TArray<FIntPoint>>& AllScenarios)
{
	// Calculate the number of chunks in each dimension
	// int TotalChunkCount = ProGen->MapSizeX / AllScenarios.Num(); //TODO: Later consider why GPT4 using sqrt for here	

	int TotalChunkCount = AllScenarios.Num();
	int MaxBoxExtent = 0;
	int LongestScenarioLength = 0;
	int Offset = 4;

	//Find the longest extends among all the rooms
	for (auto RoomActor : FixedRoomActor)
	{
		FVector BoxExtent = RoomActor->BoxComponent->GetScaledBoxExtent();
		MaxBoxExtent = FMath::Max(MaxBoxExtent, static_cast<int>(FMath::Max(BoxExtent.X, BoxExtent.Z)));
	}

	//Find the longest length of the scenarios 
	for (auto Scenario : AllScenarios)
	{
		LongestScenarioLength = FMath::Max(LongestScenarioLength, Scenario.Num());
	}

	//Step 2: Calculate exact chunk dimension
	int ChunkDimension = (MaxBoxExtent * 4) + (LongestScenarioLength * ProGen->TileSizeX);
	ChunkDimension += (ChunkDimension); // needed to add half of it because we already make all the spawning at the center of the chunk 
	int ChunksPerRow = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(TotalChunkCount))); //If there's 10 scenario then sqrt of 10 is 3.16. So 4x4 chunks will be created
	ProGen->MapSizeX = ChunksPerRow * ChunkDimension; // Total width of the map
	ProGen->MapSizeY = ChunksPerRow * ChunkDimension; // Total height of the map

	//Now say ChunkDimension is 500. So width and height is 500x500. Readjust the Map Size to cover this scenario
	ProGen->InitWorldTiles();

	CenterOfEachChunk.Empty(); // Clear any existing data

	for (int ChunkX = Offset; ChunkX < ChunksPerRow + Offset; ++ChunkX)
	{
		for (int ChunkY = Offset; ChunkY < ChunksPerRow + Offset; ++ChunkY)
		{
			// Calculate the top-left corner of the chunk, Study later on how to calculate all these 4 corners
			FVector ChunkTopLeft = FVector(ChunkX * ChunkDimension,ChunkY * ChunkDimension, 0);
			FVector ChunkTopRight = ChunkTopLeft + FVector(ChunkDimension, 0.0f, 0.0f);
			FVector ChunkBottomLeft = ChunkTopLeft + FVector(0.0f, ChunkDimension, 0.0f);
			FVector ChunkBottomRight = ChunkTopLeft + FVector(ChunkDimension, ChunkDimension, 0.0f);
			
			// Calculate the center of the chunk
			FVector ChunkCenter = ChunkTopLeft + FVector(ChunkDimension / 2, ChunkDimension / 2, 0);
			RoundNearestTilePos(ChunkCenter);
			
			// Draw lines for each side of the chunk
			DrawDebugLine(GetWorld(), ChunkTopLeft, ChunkTopRight, FColor::Red, true, -1, 0, 5);
			DrawDebugLine(GetWorld(), ChunkTopRight, ChunkBottomRight, FColor::Red, true, -1, 0, 5);
			DrawDebugLine(GetWorld(), ChunkBottomRight, ChunkBottomLeft, FColor::Red, true, -1, 0, 5);
			DrawDebugLine(GetWorld(), ChunkBottomLeft, ChunkTopLeft, FColor::Red, true, -1, 0, 5);
			DrawDebugBox(GetWorld(), ChunkCenter, FVector(ProGen->TileSizeX,ProGen->TileSizeX,ProGen->TileSizeX), FColor::Red, true, -1, 0, 5);

			// Add the chunk center to the array
			CenterOfEachChunk.Add(ChunkCenter);
		}
	}
}

void UMakeAllCorridorScenarioTest::SpawnSecondRoom(const FVector& NextRoomLocation, ARoomActor*& NextRoom)
{
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	NextRoom = FixedRoomActor[1];

	//TODO: You know this 2 line has to gone. 
	ProGen->NextRoomExitTag = NextRoom->ExitSocketDirection;
	ProGen->NextRoomEnterTag = NextRoom->EnterSocketDirection; //Just useful to spawning corridors. Nothing to do with rooms

	//First cover horizontal up to horizontal up
	NextRoom = GetWorld()->SpawnActor<ARoomActor>(NextRoom->GetClass(), NextRoomLocation, Rotation);
	FIntPoint BoxExtend = FIntPoint(NextRoom->BoxComponent->GetScaledBoxExtent().X, NextRoom->BoxComponent->GetScaledBoxExtent().Y);
	LargestExtends = FIntPoint(FMath::Max(BoxExtend.X, LargestExtends.X), FMath::Max(BoxExtend.Y, LargestExtends.Y));
	
	ProGen->SetTilesBlocked(NextRoom, NextRoomLocation, NextRoom->Rotation);
	ProGen->SetSocketExclusion(NextRoom, NextRoomLocation);
}

bool UMakeAllCorridorScenarioTest::MakeGivenPathFinding(TArray<FIntPoint>& CurrentPattern, ARoomActor* NextRoom, AProceduralGen* ProceduralGen)
{
	auto SetFirstCorrRotation = [&](FTileStruct* Start)
	{
		if (ProceduralGen->LastSpawnedRoom->ExitSocketDirection == HorizontalRight || ProceduralGen->LastSpawnedRoom->ExitSocketDirection == HorizontalLeft)
			Start->Rotation = FRotator(0, 0, -90);

		else if (ProceduralGen->LastSpawnedRoom->ExitSocketDirection == VerticalUp || ProceduralGen->LastSpawnedRoom->ExitSocketDirection == VerticalDown)
		{
			Start->Rotation = FRotator(0, 90, -90); //(Pitch=0.000000,Yaw=90.000000,Roll=-90.000000)
		}
	};
	
	//Pathfinding logic is here
	FRoomConnection Connection = ProceduralGen->CalculatePathInfo(NextRoom);

	TArray<FTileStruct*> OpenList;
	
	FIntPoint LastRoomEnd = ProceduralGen->WorldToIndex(ProceduralGen->LastSpawnedRoom->DoorSocketExit->GetComponentLocation());
	FIntPoint OverlappedRoomLoc = ProceduralGen->WorldToIndex(NextRoom->DoorSocketEnter->GetComponentLocation());

	int StartX = Connection.PathStartOffset.X + LastRoomEnd.X;
	int StartY = Connection.PathStartOffset.Y + LastRoomEnd.Y;
	int GoalX = Connection.PathEndOffset.X + OverlappedRoomLoc.X;
	int GoalY = Connection.PathEndOffset.Y + OverlappedRoomLoc.Y;

	FTileStruct* Start = &ProceduralGen->Tiles[StartX][StartY];
	SetFirstCorrRotation(Start);
	OpenList.Add(Start);

	while (OpenList.Num() > 0)
	{
		FTileStruct* Current = FillGivenCorrPattern(OpenList, CurrentPattern, ProceduralGen);
		if (Current->X == GoalX && Current->Y == GoalY)
		{
			ProceduralGen->MakeCorridorPathVisualization(ProceduralGen->LastSpawnedRoom, Current);
			ProceduralGen->SpawnCorridors(GoalX, GoalY, ProceduralGen->LastSpawnedRoom, false);
			return true;
		}
	}
	
	return false;
}

void UMakeAllCorridorScenarioTest::MakePathScenario(const FVector& FirstRoomLoc, const FVector& NextRoomLocation, TArray<FIntPoint>& CurrentPattern)
{
	SpawnFirstRoom(FirstRoomLoc);

	ARoomActor* NextRoom;
	SpawnSecondRoom(NextRoomLocation, NextRoom);
	
	MakeGivenPathFinding(CurrentPattern, NextRoom, ProGen);
}

void UMakeAllCorridorScenarioTest::SpawnFirstRoom(const FVector& FirstRoomLoc)
{
	if (!ProGen) return;
	
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	// FVector FirstRoomLoc = ProGen->Tiles[ProGen->MapSizeX / 2][ProGen->MapSizeY / 2].Location + ProGen->ZOffset; //TODO: Later decide how to determine first room loc when you spawn all the other scenarios
	ARoomActor* FirstRoom = GetWorld()->SpawnActor<ARoomActor>(FixedRoomActor[0]->GetClass(), FirstRoomLoc, Rotation);
	LargestExtends = FIntPoint(FirstRoom->BoxComponent->GetScaledBoxExtent().X, FirstRoom->BoxComponent->GetScaledBoxExtent().Z); //TODO: If I were doing this in side scrolling, this axises would be highly inverted. I don't have any idea how but later make these stuff based on criterias to determine only option based on the rotation of the actor etc, (Top down is different side scrolling is different in this case)
	
	//TODO: You know this 2 line has to gone. 
	ProGen->NextRoomExitTag = FirstRoom->ExitSocketDirection;
	ProGen->NextRoomEnterTag = FirstRoom->EnterSocketDirection; //Just useful to spawning corridors. Nothing to do with rooms

	ProGen->SetTilesBlocked(FirstRoom, FirstRoomLoc, Rotation);
	ProGen->SetSocketExclusion(FirstRoom, FirstRoomLoc);
	ProGen->LastSpawnedRoom = FirstRoom;
	ProGen->SpawnedRoomCount++;
	

	if (ProGen->VisualizeEndSocketOverlapCheck) //Written to only make visualization for first room as well.
		ProGen->IsEndSocketOverlapping(FirstRoom, FirstRoomLoc);
}

FTileStruct* UMakeAllCorridorScenarioTest::FillGivenCorrPattern(TArray<FTileStruct*>& RoomList, TArray<FIntPoint>& CurrentPattern, AProceduralGen* ProceduralGen)
{
	//For now let's just go 10 times up
	FIntPoint CurrentPatternIndex = CurrentPattern[0];
	CurrentPattern.RemoveAt(0);

	FIntPoint NewIndex = FIntPoint(RoomList.Top()->X + CurrentPatternIndex.X, RoomList.Top()->Y + CurrentPatternIndex.Y); //Go up each iteration 
	FTileStruct* LastTile = RoomList.Top();
	FTileStruct* Neighbour = &ProceduralGen->Tiles[NewIndex.X][NewIndex.Y];
	Neighbour->Parent = LastTile;
	ProceduralGen->DeterminePathDirection(LastTile, NewIndex.X, NewIndex.Y, Neighbour);
	RoomList.Add(Neighbour);
	return Neighbour;
}
