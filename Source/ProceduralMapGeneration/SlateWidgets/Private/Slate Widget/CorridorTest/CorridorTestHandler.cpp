// Fill out your copyright notice in the Description page of Project Settings.
//NOTE: AllCorridorTestWidget will invoke this class
//NOTE: It's name si All Corridor Scenarios Test 
//REMIND: TEMPORARY NOT USED


#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest/CorridorTestHandler.h"
#include "Components/BoxComponent.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/ProGenSubsystem.h"

UCorridorTestHandler::UCorridorTestHandler()
{
	
}

void UCorridorTestHandler::Initialize(ARoomActor* FirstSelectedRoom, ARoomActor* SecondSelectedRoom)
{
	this->FirstRoom = FirstSelectedRoom;
	this->SecondRoom = SecondSelectedRoom;
	Setting = GetDefault<UPluginSettings>();

	FirstSecRoom.Add(FirstRoom);
	FirstSecRoom.Add(SecondRoom);
	
	World = GEditor->GetEditorWorldContext().World();
}

void UCorridorTestHandler::Start()
{
	//TODO: Based on the first and sec room, I didn't do yet but filter the scenarios for only first and sec room directions and return those scenarios
	UProGenSubsystem* ProGenSubsystem = GEditor->GetEditorSubsystem<UProGenSubsystem>();
	Setting->LoadData("CorridorTestScenarios", ProGenSubsystem->CorridorTestScenarios);
	
	TArray<TArray<FIntPoint>> AllScenarios;
	
	auto Key = TPair<TEnumAsByte<Direction>,TEnumAsByte<Direction>>(FirstRoom->ExitSocketDirection, SecondRoom->EnterSocketDirection);
	for (auto& Scenario : ProGenSubsystem->CorridorTestScenarios)
	{
		if (Scenario.Key == Key)
		{
			AllScenarios = Scenario.Value;
		}
	}

	if (AllScenarios.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("There's no saved scenarios. Click corridor scenarios button and save some scenarios first!"));
		return;
	}
	
	InitWorldChunks(AllScenarios);

	for (auto AllScenario : AllScenarios)
	{
		FVector InitialFirstRoomLoc;
		FIntPoint Result = FIntPoint(0,0);

		for (int i = 1; i < AllScenario.Num(); i++)
		{
			Result += AllScenario[i] - AllScenario[i - 1];
		}
		
		InitialFirstRoomLoc = CenterOfEachChunk[0];
		CenterOfEachChunk.RemoveAt(0);
		
		FVector NextRoomOffset = Setting->ProGenInst->IndexToWorld(Result.X,Result.Y);

		//I has to assign this room here first
		Setting->ProGenInst->LastSpawnedRoom = FirstSecRoom[0];
		MakePathScenario(InitialFirstRoomLoc, NextRoomOffset, AllScenario);
	}
}

void UCorridorTestHandler::InitWorldChunks(TArray<TArray<FIntPoint>>& AllScenarios)
{
	int TotalChunkCount = AllScenarios.Num();
	int MaxRoomBoxExtent = 0;
	int MaxRoomEndSocketOffset = 0;
	int MaxCorrBoxExtent = 0;
	int LongestScenarioLength = 0;

	//Find the longest extends of the rooms and end socket relative among all the rooms
	for (auto RoomActor : FirstSecRoom)
	{
		//Box extends
		FVector BoxExtent = RoomActor->BoxComponent->GetScaledBoxExtent();
		MaxRoomBoxExtent = FMath::Max(MaxRoomBoxExtent, static_cast<int>(FMath::Max(BoxExtent.X, BoxExtent.Z)));

		//Relative offset
		FVector DoorSocketExit = RoomActor->DoorSocketExit->GetComponentLocation() - RoomActor->GetActorLocation();
		int X = FMath::Abs(DoorSocketExit.X);
		int Y = FMath::Abs(DoorSocketExit.Y);
		int Z = FMath::Abs(DoorSocketExit.Z);
		MaxRoomEndSocketOffset = FMath::Max(MaxRoomEndSocketOffset,(FMath::Max(X, FMath::Max(Y, Z))));
		RoundMaxBoxExtentToNearestTileSize(MaxRoomEndSocketOffset);
	}

	//Find the longest extends among all the corridors
	FVector FirstCorrExtend = Cast<ARoomActor>(Setting->ProGenInst->TurnCorridorClass.GetDefaultObject())->BoxComponent->GetScaledBoxExtent();
	FVector SecondCorrExtend = Cast<ARoomActor>(Setting->ProGenInst->StraightCorrClass.GetDefaultObject())->BoxComponent->GetScaledBoxExtent();
	TArray<FVector> AllCorrExtends = {FirstCorrExtend, SecondCorrExtend};
	for (auto CorrExtend : AllCorrExtends)
	{
		MaxCorrBoxExtent = FMath::Max(MaxCorrBoxExtent, static_cast<int>(FMath::Max(CorrExtend.X, CorrExtend.Z)));
		RoundMaxBoxExtentToNearestTileSize(MaxCorrBoxExtent);
	}

	//Find the longest length of the scenarios 
	for (auto Scenario : AllScenarios)
	{
		// LongestScenarioLength = FMath::Max(LongestScenarioLength, Scenario.Num());
		LongestScenarioLength = FMath::Max(LongestScenarioLength,GetScenariosResultLength(Scenario));
	}
	
	int ChunkDimension = (MaxRoomBoxExtent * 2) + ((LongestScenarioLength * MaxCorrBoxExtent) * 2) + MaxRoomEndSocketOffset; //TODO: Inspect why *2 in second paranthesis
	int ChunksPerRow = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(TotalChunkCount))); //If there's 10 scenario then sqrt of 10 is 3.16. So 4x4 chunks will be created

	//TODO: Here's the causing the performance problem. If ChunkDimension is high, this will cause the performance impact
	Setting->ProGenInst->MapSizeX = ChunksPerRow * ChunkDimension; // Total width of the map //TODO: I am unsure to change revert back the changes later on 
	Setting->ProGenInst->MapSizeY = ChunksPerRow * ChunkDimension; // Total height of the map

	//Now say ChunkDimension is 500. So width and height is 500x500. Readjust the Map Size to cover this scenario
	//REMIND: This line takes 21GB RAM!!!!!!!!!!!!!!!!!!!!
	double StartTime = FPlatformTime::Seconds();
	Setting->ProGenInst->InitWorldTiles();
	size_t sa = sizeof(Setting->ProGenInst);
	UE_LOG(LogTemp, Warning, TEXT("Size of FTileStruct: %llu bytes"), sizeof(FTileStruct));
	UE_LOG(LogTemp, Warning, TEXT("Size of FTileStruct: %llu bytes"), sa);

	double EndTime = FPlatformTime::Seconds();
	double ElapsedTime = EndTime - StartTime;
	UE_LOG(LogTemp, Warning, TEXT("Elapsed Time: %f"), ElapsedTime);
	
	// Clear any existing data
	CenterOfEachChunk.Empty();

	for (int ChunkX = 0; ChunkX < ChunksPerRow; ++ChunkX)
	{
		for (int ChunkY = 0; ChunkY < ChunksPerRow; ++ChunkY)
		{
			// Calculate the top-left corner of the chunk, Study later on how to calculate all these 4 corners
			FVector ChunkTopLeft = FVector(ChunkX * ChunkDimension,ChunkY * ChunkDimension, 0);
			FVector ChunkTopRight = ChunkTopLeft + FVector(ChunkDimension, 0.0f, 0.0f);
			FVector ChunkBottomLeft = ChunkTopLeft + FVector(0.0f, ChunkDimension, 0.0f);
			FVector ChunkBottomRight = ChunkTopLeft + FVector(ChunkDimension, ChunkDimension, 0.0f);

			// Calculate the center of the chunk
			FVector ChunkCenter = ChunkTopLeft + FVector(ChunkDimension / 2, ChunkDimension / 2, 0);
			FVector ChunkTopCenter = ChunkTopLeft + FVector(ChunkDimension / 2, 0,0);
			
			RoundNearestTilePos(ChunkCenter);
			
			// Draw lines for each side of the chunk
			DrawDebugLine(World, ChunkTopLeft, ChunkTopRight, FColor::Red, true, -1, 0, 5);
			DrawDebugLine(World, ChunkTopRight, ChunkBottomRight, FColor::Red, true, -1, 0, 5);
			DrawDebugLine(World, ChunkBottomRight, ChunkBottomLeft, FColor::Red, true, -1, 0, 5);
			DrawDebugLine(World, ChunkBottomLeft, ChunkTopLeft, FColor::Red, true, -1, 0, 5);
			DrawDebugBox(World, ChunkCenter, FVector(Setting->ProGenInst->TileSizeX,Setting->ProGenInst->TileSizeX,Setting->ProGenInst->TileSizeX),
				FColor::Red, true, -1, 0, 1);

			// Add the chunk center to the array
			CenterOfEachChunk.Add(ChunkTopCenter);
		}
	}

	//Set Render View first
	FVector Center = FVector(0,0,0);
	for (auto Chunk : CenterOfEachChunk)
		Center += Chunk;
	
	Center = Center / CenterOfEachChunk.Num();
	Setting->SceneCapActorInst.Get()->SetActorLocation(FVector(Center.X, Center.Y, 500));
}

void UCorridorTestHandler::MakePathScenario(const FVector& FirstRoomLoc, FVector& NextRoomLocation, TArray<FIntPoint>& CurrentPattern)
{
	SpawnedRooms.Add(SpawnFirstRoom(FirstRoomLoc));
	NextRoomLocation += FirstSecRoom[0]->DoorSocketExit->GetComponentLocation();
	SpawnedRooms.Add(SpawnSecondRoom(NextRoomLocation, FirstSecRoom[1]));
	MakeGivenPathFinding(CurrentPattern, FirstSecRoom[0],FirstSecRoom[1], NextRoomLocation, FirstRoomLoc);
}

ARoomActor* UCorridorTestHandler::SpawnFirstRoom(const FVector& FirstRoomLoc)
{
	if (!Setting->ProGenInst.Get()) return nullptr;
	
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	// FVector FirstRoomLoc = ProGen->Tiles[ProGen->MapSizeX / 2][ProGen->MapSizeY / 2].Location + ProGen->ZOffset; //TODO: Later decide how to determine first room loc when you spawn all the other scenarios
	ARoomActor* FirstLocalRoom = World->SpawnActor<ARoomActor>(FirstSecRoom[0]->GetClass(), FirstRoomLoc, Rotation);
	FirstSecRoom[0] = FirstLocalRoom;
	
	//TODO: You know this 2 line has to gone. 
	Setting->ProGenInst->NextRoomExitTag = FirstLocalRoom->ExitSocketDirection;
	Setting->ProGenInst->NextRoomEnterTag = FirstLocalRoom->EnterSocketDirection; //Just useful to spawning corridors. Nothing to do with rooms

	Setting->ProGenInst->SetTilesBlocked(FirstLocalRoom, FirstRoomLoc, Rotation);
	Setting->ProGenInst->SetSocketExclusion(FirstLocalRoom, FirstRoomLoc);
	Setting->ProGenInst->LastSpawnedRoom = FirstLocalRoom;
	Setting->ProGenInst->SpawnedRoomCount++;
	

	if (Setting->ProGenInst->VisualizeEndSocketOverlapCheck) //Written to only make visualization for first room as well.
		Setting->ProGenInst->IsEndSocketOverlapping(FirstLocalRoom, FirstRoomLoc);

	return FirstLocalRoom;
}

bool UCorridorTestHandler::MakeGivenPathFinding(TArray<FIntPoint>& CurrentPattern, ARoomActor* FirstRoomOne, ARoomActor* SecondRoomOne, const FVector& NextRoomLoc, const FVector& FirstRoomLoc)
{
	UPluginSettings* PlugSetting = GetMutableDefault<UPluginSettings>();
	auto SetFirstCorrRotation = [&](FTileStruct* Start)
	{
		if (PlugSetting->ProGenInst->LastSpawnedRoom->ExitSocketDirection == HorizontalRight || PlugSetting->ProGenInst->LastSpawnedRoom->ExitSocketDirection == HorizontalLeft)
			Start->Rotation = FRotator(0, 0, -90);

		else if (PlugSetting->ProGenInst->LastSpawnedRoom->ExitSocketDirection == VerticalUp || PlugSetting->ProGenInst->LastSpawnedRoom->ExitSocketDirection == VerticalDown)
		{
			Start->Rotation = FRotator(0, 90, -90); //(Pitch=0.000000,Yaw=90.000000,Roll=-90.000000)
		}
	};

	//TODO: his is required for a reason I don't know as well I have to find it's reason and fix this shit 
	int Offset = -1;
	
	//Pathfinding logic is here
	FRoomConnection Connection = PlugSetting->ProGenInst->CalculatePathInfo(SecondRoomOne);
	Connection.StartPoint = FirstRoomLoc;
	Connection.EndPoint = NextRoomLoc;

	//TODO: Later on disable this shit. If necessary
	Connection.PathStartOffset = FIntPoint(0,0);
	Connection.PathEndOffset = FIntPoint(0,0);
	
	TArray<FTileStruct*> OpenList;
	
	FIntPoint FirstRoomEnd = PlugSetting->ProGenInst->WorldToIndex(FirstRoomOne->DoorSocketExit->GetComponentLocation());
	// FIntPoint OverlappedRoomLoc = PlugSetting->ProGenInst->WorldToIndex(NextRoom->DoorSocketEnter->GetComponentLocation());

	int StartX = Connection.PathStartOffset.X + FirstRoomEnd.X + Offset;
	int StartY = Connection.PathStartOffset.Y + FirstRoomEnd.Y;
	int GoalX = Connection.PathEndOffset.X + PlugSetting->ProGenInst->WorldToIndex(NextRoomLoc).X + Offset;
	int GoalY = Connection.PathEndOffset.Y + PlugSetting->ProGenInst->WorldToIndex(NextRoomLoc).Y;

	FTileStruct* Start = &PlugSetting->ProGenInst->Tiles[StartX][StartY];
	Start->Direction = PlugSetting->ProGenInst->DetermineFirstDirection(Start->Direction);
	
	//Compare This Start->Direction with CurrentPattern
	Start->Direction = DetermineFirstDirection(CurrentPattern);
	
	SetFirstCorrRotation(Start);
	OpenList.Add(Start);

	while (OpenList.Num() > 0)
	{
		FTileStruct* Current = FillGivenCorrPattern(OpenList, CurrentPattern, PlugSetting);
		if (Current->X == GoalX && Current->Y == GoalY || CurrentPattern.IsEmpty()) //I added is empty so I can inspact further 
		{
			PlugSetting->ProGenInst->VisualizeCorridorPath = true;
			PlugSetting->ProGenInst->MakeCorridorPathVisualization(PlugSetting->ProGenInst->LastSpawnedRoom, Current);
			PlugSetting->ProGenInst->NextRoomEnterTag = SecondRoomOne->EnterSocketDirection;
			PlugSetting->ProGenInst->SpawnCorridors(GoalX, GoalY, PlugSetting->ProGenInst->LastSpawnedRoom, false);

			//For some reason, Vertical Up-> Vertical Down, Current pattern never gets empty but in other directions there's always
			//One element left in the array. So I have to clear it here
			CurrentPattern.Empty();
			return true;
		}
	}
	
	return false;
}

FTileStruct* UCorridorTestHandler::FillGivenCorrPattern(TArray<FTileStruct*>& RoomList, TArray<FIntPoint>& CurrentPattern, UPluginSettings* PlugSetting)
{
	//For now let's just go 10 times up
	FIntPoint CurrentPatternIndex;
	CurrentPattern.Num() > 1 ? CurrentPatternIndex =  CurrentPattern[1] - CurrentPattern[0] : CurrentPatternIndex = CurrentPattern[0];

	if (CurrentPattern.Num() > 1)
	{
		CurrentPatternIndex =  CurrentPattern[1] - CurrentPattern[0];
	}
	else
	{
		CurrentPatternIndex.X = CurrentPattern[0].X - RoomList[RoomList.Num()-2]->X;
		CurrentPatternIndex.Y = CurrentPattern[0].Y - RoomList[RoomList.Num()-2]->Y;
	}

	CurrentPattern.RemoveAt(0);

	//previous selected tile + current one's offset index
	FIntPoint NewIndex = FIntPoint(RoomList.Top()->X + CurrentPatternIndex.X, RoomList.Top()->Y + CurrentPatternIndex.Y);
	
	FTileStruct* LastTile = RoomList.Top();
	FTileStruct* Neighbour = &PlugSetting->ProGenInst->Tiles[NewIndex.X][NewIndex.Y];
	Neighbour->Parent = LastTile;
	PlugSetting->ProGenInst->DeterminePathDirection(LastTile, NewIndex.X, NewIndex.Y, Neighbour);
	RoomList.Add(Neighbour);
	return Neighbour;
}

void UCorridorTestHandler::DestroySpawnedRooms()
{
	for (ARoomActor* SpawnedRoom : SpawnedRooms)
	{
		SpawnedRoom->Destroy();
	}

	for (ARoomActor* AllSpawnedCorridor : Setting->ProGenInst->AllSpawnedCorridors)
	{
		AllSpawnedCorridor->Destroy();
	}
	
	FlushPersistentDebugLines(World);
}

ARoomActor* UCorridorTestHandler::SpawnSecondRoom(const FVector& NextRoomLocation, ARoomActor*& NextRoom) const
{
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	//TODO: You know this 2 line has to gone. 
	Setting->ProGenInst->NextRoomExitTag = NextRoom->ExitSocketDirection;
	Setting->ProGenInst->NextRoomEnterTag = NextRoom->EnterSocketDirection; //Just useful to spawning corridors. Nothing to do with rooms

	NextRoom = World->SpawnActor<ARoomActor>(NextRoom->GetClass(), NextRoomLocation, Rotation);
	Setting->ProGenInst->SetTilesBlocked(NextRoom, NextRoomLocation, NextRoom->Rotation);
	Setting->ProGenInst->SetSocketExclusion(NextRoom, NextRoomLocation);
	return NextRoom;
}

int UCorridorTestHandler::GetScenariosResultLength(TArray<FIntPoint> Scenario)
{
	TArray<FIntPoint> Result;

	//subtract each element from the next one so I can get raw direction elements
	for (int i = 1; i < Scenario.Num(); ++i)
	{
		FIntPoint CurrentPatternIndex;

		if (i < Scenario.Num())
		{
			CurrentPatternIndex =  Scenario[i] - Scenario[i - 1];
		}
		else //REMIND: I guess it will never hit here
		{
			CurrentPatternIndex.X = Scenario[Scenario.Last(0).X].X - Scenario[Scenario.Last(1).X].X;
			CurrentPatternIndex.Y = Scenario[Scenario.Last(0).Y].Y - Scenario[Scenario.Last(1).Y].Y;
		}
		Result.Add(CurrentPatternIndex);
	}

	//Find total distance it will go. For example Destination will be (25,20) which means it will be diagonal right down.  
	FIntPoint Destination = FIntPoint(0,0);
	
	for (int i = 0; i < Result.Num(); ++i)
	{
		Destination += Result[i];
	}

	int X = FMath::Abs(Destination.X);
	int Y = FMath::Abs(Destination.Y);

	return FMath::Max(X,Y);
}
