//TODO: Expose a Vector variable that will make entire map centered at that location. 
//TODO: Consider changing UnscaledBoxExtent to ScaledBoxExtent. 
//TODO: WHat's the reason RoomConnections must be array rather than a variable. As soon as the room is overlapped.First I need to spawn that room and make a corridor. ASAP. Otherwise, it could make lots and lots of complications
//TODO: Make MapSizeX and MapSizeY one variable as they cannot be different than each other anymore
#include "ProceduralGen.h"

#include "Components/BoxComponent.h"
#include "Door/DoorActor.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/Test/MakeAllCorridorScenarioTest.h"

class UMakeAllCorridorScenarioTest;

//if ApplyTurnPenalty is not true disable TurnPenaltyAmount textbox
bool AProceduralGen::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AProceduralGen, TurnPenaltyAmount))
	{
		return ApplyTurnPenalty ? true : false;
	}

	return Super::CanEditChange(InProperty);
}

AProceduralGen::AProceduralGen()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AProceduralGen::BeginPlay()
{
	Super::BeginPlay();

	//Assign TileBoxExtends of the assigned tiles
	TileBoxExtends = FVector(TileSizeX / 2, TileSizeY / 2, TileSizeY / 2);

	//Cast all room designs.
	for (auto RoomDesign : RoomDesigns)
	{
		ARoomActor* CastedRoom = Cast<ARoomActor>(RoomDesign->GetDefaultObject());
		CastedRoomDesigns.Add(CastedRoom);
	}
	CastedTurnCorridor = Cast<ARoomActor>(TurnCorridorClass->GetDefaultObject());


	//All these tests in this class are so stupid. I will somehow remove all and make all tests in slate
	if (OnlyMakeCorridorCheck)
	{
		//Body
		UMakeAllCorridorScenarioTest* Test = NewObject<UMakeAllCorridorScenarioTest>(this, UMakeAllCorridorScenarioTest::StaticClass());
		Test->Initialize(this, FixedTwoRoomActor);
		Test->Start();

		if (VisualizeOverlaps || VisualizeVisited || VisualizeOverlappedRoomTiles)
		{
			VisualizeTiles();
		}

		return;
	}

	GenerateMap();

	if (VisualizeOverlaps || VisualizeVisited || VisualizeOverlappedRoomTiles)
	{
		VisualizeTiles();
	}
}

void AProceduralGen::GenerateMap()
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateMap);
	InitWorldTiles();
	SpawnTestCollisionObjects(); //If any
	SpawnFirstRoom();

	//Entire main logic is this while loop.
	while (SpawnedRoomCount < NumberOfRooms)
	{
		if (RoomSpawning(LastSpawnedRoom->ExitSocketDirection)) continue;

		if (LastSpawnedRoom->NoExit) //TODO: I don't remember what's the reason this if block written BY ME
		{
			UE_LOG(LogTemp, Display, TEXT("Aborted all room spawning due to NoExit room encountered"));
			NumberOfRooms = -1;
		}
	}

	//Large Room logic
	if (!LargeRooms.IsEmpty())
	{
		MakeSideBranchFromLargeRoom();
		LargeRooms.Empty();
	}
	if (!LargeRoomsToBeAdded.IsEmpty()) //TODO: Probably there's a room to refactor here  
	{
		MakeSideBranchFromLargeRoom();
	}
}

bool AProceduralGen::RoomSpawning(const Direction EndSocketDirection)
{
	SCOPE_CYCLE_COUNTER(STAT_RoomSpawning);
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	FVector NextRoomLocation = LastSpawnedRoom->DoorSocketExit->GetComponentLocation();
	bool CanSpawnLargeRoom = LargeRoomCounter < MaxLargeRoomCount ? true : false;

	ARoomActor* NextRoom = SelectRoomWithDirection(ExpectedDirection(EndSocketDirection), CanSpawnLargeRoom, false);
	NextRoom->Rotation = Rotation;

	//TODO: You know this 2 line has to gone. 
	NextRoomExitTag = NextRoom->ExitSocketDirection;
	NextRoomEnterTag = NextRoom->EnterSocketDirection; //Just useful to spawning corridors. Nothing to do with rooms

	if (!IsColliding(NextRoom, NextRoomLocation, Rotation))
	{
		SpawnNonOverlappedRoom(Rotation, NextRoomLocation, NextRoom);
	}
	else
	{
		return SpawnOverlappedRoom(Rotation, NextRoomLocation, NextRoom);
	}

	return true;
}

bool AProceduralGen::CanMakeCorridorPathBeforeSpawning(ARoomActor*& NextRoom, const FVector& NextRoomLocation)
{
	TArray<FIntPoint> BlockedTiles = SetTilesBlocked(NextRoom, NextRoomLocation, NextRoom->Rotation); //TODO: How will I implement 
	SetSocketExclusion(NextRoom, NextRoomLocation);

	SCOPE_CYCLE_COUNTER(STAT_CanMakeCorridorPathBeforeSpawning);
	FRoomConnection Connection;
	Connection.StartPoint = LastSpawnedRoom->DoorSocketExit->GetComponentLocation();
	Connection.EndPoint = NextRoomLocation;
	Connection.PathEndOffset = NextRoom->PathEndOffset;
	Connection.PathStartOffset = LastSpawnedRoom->PathStartOffset;
	Connection.MaxCheckAmount = FindCorridorPathLimit;
	Connection.RoomName = NextRoom->GetName();
	FIntPoint StartIndex = WorldToIndex(Connection.StartPoint);
	FIntPoint EndIndex = WorldToIndex(Connection.EndPoint);
	ResetAllVisited();
	if (!FindCorridorPath(StartIndex.X, StartIndex.Y, EndIndex.X, EndIndex.Y, Connection.PathStartOffset, Connection.PathEndOffset, false, Connection.MaxCheckAmount, NextRoom))
	{
		UnBlockTiles(BlockedTiles);
		return false;
	}
	UnBlockTiles(BlockedTiles);
	return true;
}

TSharedPtr<TArray<ARoomActor*>> AProceduralGen::GetBPSpecificBranchRooms(ARoomActor*& NextRoom, const FString& SocketName)
{
	TSharedPtr<TArray<ARoomActor*>> RoomActors = MakeShared<TArray<ARoomActor*>>();

	// Iterate over the properties of the actor's class
	for (TFieldIterator<FProperty> PropIt(NextRoom->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		FString PropertyName = Prop->GetFName().ToString();
		FString NameItShouldBe = (SocketName + TEXT("_Spawn"));
		if (PropertyName != NameItShouldBe) continue;
		// Check if the property is an array
		if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			FProperty* InnerProp = ArrayProp->Inner;
			UE_LOG(LogTemp, Warning, TEXT("Inner Property Class: %s"), *InnerProp->GetClass()->GetName());

			// Cast the Inner of the ArrayProp to FObjectProperty
			FClassProperty* ClassInnerProp = CastField<FClassProperty>(ArrayProp->Inner);

			// Check if the Inner property is of ARoomActor type or its subclass
			if (ClassInnerProp && ClassInnerProp->MetaClass->IsChildOf(ARoomActor::StaticClass()))
			{
				UE_LOG(LogTemp, Warning, TEXT("ObjectInnerProp is valid for %s"), *Prop->GetName());

				// This is an array of ARoomActor. Access the array here.
				FScriptArrayHelper_InContainer ArrayHelper(ArrayProp, NextRoom);

				for (int i = 0; i < ArrayHelper.Num(); ++i)
				{
					UClass* RoomActorClass = Cast<UClass>(ClassInnerProp->GetObjectPropertyValue(ArrayHelper.GetRawPtr(i)));
					if (RoomActorClass && RoomActorClass->IsChildOf(ARoomActor::StaticClass()))
					{
						if (ARoomActor* RoomActor = NewObject<ARoomActor>(NextRoom, RoomActorClass))
						{
							RoomActors->Add(RoomActor);
						}
					}
				}
				return RoomActors;
			}
		}
	}
	return RoomActors;
}

void AProceduralGen::InitAndSpawnRoom(ARoomActor*& NextRoom, const FVector& NextRoomLocation, const FRotator& Rotation, const bool IsOverlapped, bool AddLargeRoomTempArray)
{
	UE_LOG(LogTemp, Display, TEXT("SpawnedRoom: %s at this loc: %s"), *NextRoom->GetName(), *NextRoomLocation.ToString());

	NextRoom = GetWorld()->SpawnActor<ARoomActor>(NextRoom->GetClass(), NextRoomLocation, Rotation);
	SetTilesBlocked(NextRoom, NextRoomLocation, Rotation);
	SpawnDoors(Rotation, NextRoomLocation, NextRoom, true);
	SetSocketExclusion(NextRoom, NextRoomLocation);

	if (IsOverlapped)
	{
		ConnectRoomsWithCorridors(NextRoom);
	}

	if (NextRoom->LargeRoom)
	{
		AddLargeRoomTempArray ? LargeRoomsToBeAdded.Add(NextRoom) : LargeRooms.Add(NextRoom);
		LargeRoomCounter++;
	}

	SpawnedRoomCount++;
	LastSpawnedRoom = NextRoom;
}

void AProceduralGen::MakeSideBranchFromLargeRoom()
{
	SCOPE_CYCLE_COUNTER(STAT_MakeSideBranchFromLargeRoom);
	TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms;
	int SpawnCounter = 0;
	bool EndBranch = false;
	auto IsSocketAvailable = [&](const USceneComponent* Socket)
	{
		TArray<FName> AllTags = Socket->ComponentTags;
		return (!AllTags.Contains("Enter") && !AllTags.Contains("Exit") && !AllTags.IsEmpty()
			&& NumOfSideBranchRoom < MaxSideBranchRoom);
	};

	auto ProcessLargeRoomSockets = [&](ARoomActor* LargeRoom)
	{
		TArray<USceneComponent*> Sockets;
		LargeRoom->GetComponents<USceneComponent>(Sockets);
		for (auto Socket : Sockets) //Each large room's each socket
		{
			if (IsSocketAvailable(Socket)) //After here only available sockets will be selected to make side branch
			{
				ManualBranchRooms = GetBPSpecificBranchRooms(LargeRoom, Socket->GetName());
				if (ARoomActor* FirstSpawnedBranchRoom = SpawnFirstBranchRoom(TagToDirection(Socket->ComponentTags[0]), Socket->GetComponentLocation(), SpawnCounter, ManualBranchRooms))
				{
					while (FirstSpawnedBranchRoom && SpawnCounter < BranchLength && NumOfSideBranchRoom < MaxSideBranchRoom && !EndBranch)
					{
						SpawnBranchRoom(LastSpawnedRoom->ExitSocketDirection, LastSpawnedRoom->DoorSocketExit->GetComponentLocation(), SpawnCounter, EndBranch, Socket->ComponentTags, TagToDirection(Socket->ComponentTags[0]), ManualBranchRooms);
						SpawnCounter++;
					}
				}
				else
				{
					SpawnNoExitDoor(LargeRoom, Socket->ComponentTags[0], Socket->GetComponentLocation());
				}
			}
			SpawnedRooms.Empty();
		}
	};

	TArray<ARoomActor*> IterateLargeRooms = LargeRooms.IsEmpty() ? LargeRoomsToBeAdded : LargeRooms;
	for (auto LargeRoom : IterateLargeRooms)
	{
		SelectedLargeRoom = LargeRoom;
		ProcessLargeRoomSockets(LargeRoom);
	}
}

void AProceduralGen::UnBlockLastRoomItsCorridorAndDestroy()
{
	SCOPE_CYCLE_COUNTER(STAT_UnBlockLastRoomItsCorridorAndDestroy);

	//Unblock last spawned room
	for (auto BlockedTile : SpawnedRooms.Last()->BlockedRoomTiles)
		Tiles[BlockedTile.X][BlockedTile.Y].Blocked = false;

	//Unblock the tiles
	for (auto Corridor : SpawnedRooms.Last()->OwnerCorridors)
	{
		if (!Corridor)
		{
			return;
		}
		for (auto BlockedCorr : Corridor->BlockedCorTiles)
		{
			Tiles[BlockedCorr.X][BlockedCorr.Y].Blocked = false;
		}
	}
	for (auto OwnerCorridor : SpawnedRooms.Last()->OwnerCorridors)
	{
		OwnerCorridor->Destroy();
	}

	SpawnedRooms.Pop()->Destroy();
	if (!SpawnedRooms.IsEmpty())
		LastSpawnedRoom = SpawnedRooms.Last();
}

void AProceduralGen::DestroyLastRoomSpawnNoExit(FVector& SpawnLocation, const FRotator& Rotation, int& SpawnCounter, bool CanSpawnLargeRoom, TArray<ARoomActor*>& CustomRoomDesigns, bool& OnlySpawnNoExit, ARoomActor*& NextRoom)
{
	CustomRoomDesigns = CastedRoomDesigns; //Refill the array again
	//Before destroying select the NoExit room of last spawned room's enter direction. 
	NextRoom = SelectRoomWithDirection(SpawnedRooms.Last()->EnterSocketDirection, CanSpawnLargeRoom, true, &CustomRoomDesigns);
	SpawnLocation = SpawnedRooms.Last()->DoorSocketEnter->GetComponentLocation();
	SpawnCounter = BranchLength;
	OnlySpawnNoExit = true;
	UnBlockLastRoomItsCorridorAndDestroy();

	for (auto& CustomRoomDesign : CustomRoomDesigns)
	{
		NextRoom = CustomRoomDesign;

		if (!IsColliding(NextRoom, SpawnLocation, Rotation))
		{
			break;
		}
		else
		{
			if (MoveOverlappedRoom(NextRoom, SpawnLocation) || OverlappedRoomMoverCounter >= 20)
				break;
		}
		//we unblocked and destroyed last spawned room, then we tried to spawn no exit room but it's failed, it's colliding and moveOverlappedRoom also returned false. So we need to
		// Destroy again and repeat the process
		if (&CustomRoomDesign == &CustomRoomDesigns.Last())
		{
			DestroyLastRoomSpawnNoExit(SpawnLocation, Rotation, SpawnCounter, CanSpawnLargeRoom, CustomRoomDesigns, OnlySpawnNoExit, NextRoom);
		}
	}
}

ARoomActor* AProceduralGen::SpawnBranchRoom(Direction ExpDirection, FVector SpawnLocation, int& SpawnCounter, bool& EndBranch, TArray<FName>& SocketComps, Direction LargeRoomSceneComp, TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms)
{
	SCOPE_CYCLE_COUNTER(STAT_SpawnBranchRoom);
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	bool CanSpawnLargeRoom = LargeRoomCounter < MaxLargeRoomCount ? true : false;
	TArray<ARoomActor*> CustomRoomDesigns = CastedRoomDesigns;
	bool OnlySpawnNoExit = false;
	bool IsManualBranchGiven = false;

	//Now in here check that there's a manual room given for that comp or not.


	ARoomActor* NextRoom = nullptr; //TODO: Pass custom array here. I don't know how but in here try to select and spawn all the rooms that not overlapping and can make corridor path  
	if (SpawnCounter + 1 != BranchLength || !ManualBranchRooms->IsEmpty())
	{
		NextRoom = SelectRoomWithDirection(ExpectedDirection(ExpDirection), CanSpawnLargeRoom, false, nullptr, ManualBranchRooms);
	}
	else
	{
		NextRoom = SelectRoomWithDirection(ExpectedDirection(ExpDirection), CanSpawnLargeRoom, true);
	} //TODO: HAndle the case no exit room is unable to be path connected. in line 217 handle the case based on the initial selected room type 

	//TODO: You know this 2 line has to gone. 
	if (NextRoom)
	{
		NextRoomExitTag = NextRoom->ExitSocketDirection;
		NextRoomEnterTag = NextRoom->EnterSocketDirection;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Nextroom is null for some reason"));
		return nullptr;
	}
	//Try to select all the available rooms if it colliding, if the chosen room is not colliding we don't need to move overlapped room anyway. 

	while (!NextRoom || IsColliding(NextRoom, SpawnLocation, Rotation))
	{
		//If this returns, that means we successfully chosen a new location with a room not colliding. Keep trying until the array is emptied. 
		if (NextRoom && MoveOverlappedRoom(NextRoom, SpawnLocation))
		{
			OnlySpawnNoExit = true;
			break;
		}

		//Room selecting
		UE_LOG(LogTemp, Warning, TEXT("Selected Room: %s overlapped and couldn't find an available position. Other rooms will be tried to selected "), *NextRoom->GetName());

		bool ShouldSpawnNoExit = NextRoom->NoExit || SpawnCounter + 1 == BranchLength;
		NextRoom = SelectRoomWithDirection(ExpectedDirection(ExpDirection), CanSpawnLargeRoom, ShouldSpawnNoExit, &CustomRoomDesigns);
		CustomRoomDesigns.Remove(NextRoom);

		//If couldn't find a valid room and array got emptied try to spawn no exit room destroying last spawned room 
		if (CustomRoomDesigns.IsEmpty() && IsColliding(NextRoom, SpawnLocation, Rotation))
		{
			DestroyLastRoomSpawnNoExit(SpawnLocation, NextRoom->Rotation, SpawnCounter, CanSpawnLargeRoom, CustomRoomDesigns, OnlySpawnNoExit, NextRoom);

			//First time destroying last room and trying to spawn no exit room has been failed. Now repeat the process again. 
			while (!NextRoom || IsColliding(NextRoom, SpawnLocation, Rotation))
			{
				if (SpawnedRooms.IsEmpty())
				{
					SpawnCounter = BranchLength;
					return nullptr;
				}
				NextRoom = SelectRoomWithDirection(SpawnedRooms.Last()->EnterSocketDirection, false, true, &CustomRoomDesigns);
				CustomRoomDesigns.Remove(NextRoom);

				if (NextRoom && MoveOverlappedRoom(NextRoom, SpawnLocation))
				{
					OnlySpawnNoExit = true;
					break;
				}

				//Unfortunately this destroying last spawned room trying to spawn no exit failed so I need to repeat this process again. Destroy again last spawned room, and repeat. 
				if (CustomRoomDesigns.IsEmpty() || IsColliding(NextRoom, SpawnLocation, Rotation))
				{
					// return nullptr; //In while loop again select all again like in while loop above
					CustomRoomDesigns = CastedRoomDesigns;
					SpawnLocation = SpawnedRooms.Last()->DoorSocketEnter->GetComponentLocation();

					UnBlockLastRoomItsCorridorAndDestroy();

					if (SpawnedRooms.IsEmpty()) //LargeRoomSceneComp
					{
						NextRoom = SelectRoomWithDirection(LargeRoomSceneComp, false, true, &CustomRoomDesigns);
					}
					else
					{
						NextRoom = SelectRoomWithDirection(SpawnedRooms.Last()->EnterSocketDirection, false, true, &CustomRoomDesigns);
						SpawnedRooms.Pop()->Destroy();
					}

					//Last resort
					if (!IsColliding(NextRoom, SpawnLocation, Rotation))
					{
						OnlySpawnNoExit = true;
						break;
					}
					else if (MoveOverlappedRoom(NextRoom, SpawnLocation))
					{
						OnlySpawnNoExit = true;
						break;
					}
					if (SpawnedRooms.IsEmpty())
					{
					}
				}
			}
		}
	}
	InitAndSpawnRoom(NextRoom, SpawnLocation, Rotation, OnlySpawnNoExit, true);
	SpawnedRooms.Add(LastSpawnedRoom);

	return NextRoom;
}

ARoomActor* AProceduralGen::SpawnFirstBranchRoom(Direction Direction, FVector SpawnLoc, int& SpawnCounter, TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms)
{
	SpawnCounter = 0;
	TArray<ARoomActor*> CustomRoomDesigns = CastedRoomDesigns;
	bool CanSpawnLargeRoom = LargeRoomCounter < MaxLargeRoomCount ? true : false;
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	FVector NextRoomLocation = SpawnLoc;
	ARoomActor* NextRoom = nullptr;

	//Try to select all the available rooms
	while (!NextRoom || IsColliding(NextRoom, NextRoomLocation, Rotation))
	{
		NextRoom = SelectRoomWithDirection(ExpectedDirection(Direction), CanSpawnLargeRoom, false, &CustomRoomDesigns, ManualBranchRooms);
		CustomRoomDesigns.Remove(NextRoom);

		if (CustomRoomDesigns.IsEmpty() && IsColliding(NextRoom, NextRoomLocation, Rotation))
		{
			return nullptr;
		}
	}

	InitAndSpawnRoom(NextRoom, NextRoomLocation, Rotation, false, true);
	SpawnedRooms.Add(LastSpawnedRoom);

	SpawnCounter++;
	return NextRoom;
}

void AProceduralGen::InitWorldTiles()
{
	SCOPE_CYCLE_COUNTER(STAT_InitWorldTiles);

	Tiles.Empty();

	//Init world
	World = GetWorld();
	if (!World) World = GEditor->GetEditorWorldContext().World();

	//REMIND: Previous unefficient way of initializing tiles array took 45 second and 21GB ram on slate 
	//resize tiles array
	// Tiles.SetNum(MapSizeX);
	// for (int i = 0; i < MapSizeX; ++i)
	// 	Tiles[i].SetNum(MapSizeY);

	//REMIND: This one took 25 seconds
	Tiles.Reserve(MapSizeX);
	for (int i = 0; i < MapSizeX; ++i)
	{
		TArray<FTileStruct> TempRow;
		TempRow.SetNumZeroed(MapSizeY);
		Tiles.Add(TempRow);
	}

	//TODO: Idea in here to specify exactly the center of the map worked good but everything messed up when rooms spawned. Handle this case later 
	//Calculate half sizes for centering
	float HalfMapWidth = (MapSizeX * TileSizeX) / 2;
	float HalfMapHeight = (MapSizeY * TileSizeY) / 2;

	//Calculate the starting position (bottom left corner of the map)
	FVector StartPosition = MapCenter - FVector(HalfMapWidth, HalfMapHeight, 0);

	for (int x = 0; x < MapSizeX; ++x)
		for (int y = 0; y < MapSizeY; ++y)
		{
			Tiles[x][y].Location = IndexToWorld(x, y);
			Tiles[x][y].X = x;
			Tiles[x][y].Y = y;
			Tiles[x][y].Parent = nullptr;
			Tiles[x][y].Blocked = false;
			Tiles[x][y].Direction = Dir_None;
		}
}

void AProceduralGen::SpawnTestCollisionObjects()
{
	SCOPE_CYCLE_COUNTER(STAT_SpawnTestCollisionObjects);
	FRotator Rotation(0.0f, 0.0f, -90.0f);
	for (int i = 0; i < BlockRoomLocations.Num(); ++i)
	{
		ARoomActor* TestRoom = World->SpawnActor<ARoomActor>(BlockRoom, BlockRoomLocations[i], Rotation);
		SetTilesBlocked(TestRoom, BlockRoomLocations[i], Rotation);
		SetSocketExclusion(TestRoom, BlockRoomLocations[i]);
	}
}

void AProceduralGen::SpawnFirstRoom()
{
	SCOPE_CYCLE_COUNTER(STAT_SpawnFirstRoom);
	const FRotator Rotation(0.0f, 0.0f, -90.0f);
	ARoomActor* NextRoom = nullptr;
	//TODO: Later for first room spawning make first room array will pick random index from 4 directions. There won't be enter only exit.  
	if (DebugRoomSequence.IsEmpty())
	{
		NextRoom = Cast<ARoomActor>(CastedRoomDesigns[0]); //Later on we will make here completely random as well. Just for now spawn first index as first room as always.
	}
	else
	{
		NextRoom = Cast<ARoomActor>(DebugRoomSequence[0]->GetDefaultObject());
		CurrentIndex++;
	}

	//TODO: You know this 2 line has to gone. 
	NextRoomExitTag = NextRoom->ExitSocketDirection;
	NextRoomEnterTag = NextRoom->EnterSocketDirection; //Just useful to spawning corridors. Nothing to do with rooms

	FVector FirstRoomStartLoc = Tiles[MapSizeX / 2][MapSizeY / 2].Location + ZOffset;
	ARoomActor* FirstRoom = World->SpawnActor<ARoomActor>(NextRoom->GetClass(), FirstRoomStartLoc, Rotation);
	SetTilesBlocked(FirstRoom, FirstRoomStartLoc, Rotation);
	SetSocketExclusion(FirstRoom, FirstRoomStartLoc);
	LastSpawnedRoom = FirstRoom;
	SpawnedRoomCount++;

	if (VisualizeEndSocketOverlapCheck) //Written to only make visualization for first room as well. 
	{
		IsEndSocketOverlapping(FirstRoom, FirstRoomStartLoc);
	}

	if (LastSpawnedRoom->LargeRoom)
	{
		LargeRooms.Add(LastSpawnedRoom);
	}
}

FRoomConnection AProceduralGen::CalculatePathInfo(ARoomActor* NextRoom)
{
	FRoomConnection Connection;
	Connection.StartPoint = LastSpawnedRoom->DoorSocketExit->GetComponentLocation();
	Connection.EndPoint = NextRoom->DoorSocketEnter->GetComponentLocation();
	Connection.PathEndOffset = NextRoom->PathEndOffset;
	Connection.PathStartOffset = LastSpawnedRoom->PathStartOffset;
	Connection.MaxCheckAmount = FindCorridorPathLimit;
	Connection.RoomName = NextRoom->GetName();

	if (VisualizeBeginAndEndTiles)
	{
		VisualizeBeginEndTiles(NextRoom, Connection);
	}

	return Connection;
}

void AProceduralGen::SpawnNonOverlappedRoom(const FRotator& Rotation, const FVector& NextRoomLocation, ARoomActor*& NextRoom)
{
	InitAndSpawnRoom(NextRoom, NextRoomLocation, Rotation, false);
}

bool AProceduralGen::SpawnOverlappedRoom(const FRotator& Rotation, FVector NextRoomLocation, ARoomActor*& NextRoom)
{
	if (MoveOverlappedRoom(NextRoom, NextRoomLocation)) //TODO: I believe if available space couldn't be found we need to do something in here.
	{
		InitAndSpawnRoom(NextRoom, NextRoomLocation, Rotation, true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't find available location in overlapped room %s aborted. If it seem possible to make a path connection, increase MaxOverlappedRoomIterate or decrease BufferSize "), *NextRoom->GetName());
		SpawnedRoomCount++;
		return false;
	}
	return true;
}

void AProceduralGen::SpawnDoors(const FRotator& Rotation, const FVector& NextRoomLocation, ARoomActor*& NextRoom, const bool OnlySpawnEnterDoor)
{
	ADoorActor* EnterDoor = World->SpawnActor<ADoorActor>(NextRoom->EnterDoor, NextRoomLocation + ZOffset, Rotation);
	if (!OnlySpawnEnterDoor) World->SpawnActor<ADoorActor>(NextRoom->ExitDoor, NextRoom->DoorSocketExit->GetComponentLocation() + ZOffset, Rotation);
	NextRoom->SetEnterDoorActor(EnterDoor);
}

//TODO: LATER MAKE ROTATIONS PROPER


void AProceduralGen::SpawnNoExitDoor(ARoomActor* LargeRoom, const FName& SceneTag, const FVector& SocketLocation)
{
	SCOPE_CYCLE_COUNTER(STAT_SpawnNoExitDoor);
	Direction TagDirection = TagToDirection(SceneTag);

	if (LastSpawnedRoom->NoExitHorizontalLeft || LastSpawnedRoom->NoExitHorizontalRight || LastSpawnedRoom->NoExitVerticalUp || LastSpawnedRoom->NoExitVerticalDown)
	{
		UE_LOG(LogTemp, Error, TEXT("No Exit Door is not given in the Blueprint"));
		return;
	}
	TMap<Direction, TPair<TSubclassOf<ADoorActor>, FRotator>> DoorMap;
	DoorMap.Add(HorizontalLeft, TPair<TSubclassOf<ADoorActor>, FRotator>(LargeRoom->NoExitHorizontalLeft, FRotator(0.0f, 0.0f, -90.0f)));
	DoorMap.Add(HorizontalRight, TPair<TSubclassOf<ADoorActor>, FRotator>(LargeRoom->NoExitHorizontalRight, FRotator(0.0f, 0.0f, -90.0f)));
	DoorMap.Add(VerticalUp, TPair<TSubclassOf<ADoorActor>, FRotator>(LargeRoom->NoExitVerticalUp, FRotator(0.0f, 0.0f, -90.0f)));
	DoorMap.Add(VerticalDown, TPair<TSubclassOf<ADoorActor>, FRotator>(LargeRoom->NoExitVerticalDown, FRotator(0.0f, 0.0f, -90.0f)));


	if (auto DoorPair = DoorMap.Find(TagDirection))
	{
		World->SpawnActor<ADoorActor>(DoorPair->Key, SocketLocation + ZOffset, DoorPair->Value);
	}
}

bool AProceduralGen::IsEndSocketOverlapping(ARoomActor* NextRoom, const FVector& SpawnLoc)
{
	SCOPE_CYCLE_COUNTER(STAT_IsEndSocketOverlapping);
	FVector ExitLoc = NextRoom->DoorSocketExit->GetRelativeLocation();
	SwapInvZYaxis(ExitLoc);
	FVector EndSocketWorldLoc = SpawnLoc + ExitLoc;
	FIntPoint Index = WorldToIndex(EndSocketWorldLoc);
	Index += NextRoom->ExitSocketCheckOffset;

	for (auto Tile : NextRoom->ExitSocketChecks)
	{
		int CurrentX = Index.X + Tile.X;
		int CurrentY = Index.Y + Tile.Y;

		if (IsValid(CurrentX, CurrentY) && VisualizeEndSocketOverlapCheck)
			DrawDebugBox(World, Tiles[CurrentX][CurrentY].Location + TileBoxExtends, TileBoxExtends, FColor::Magenta, true);

		if (IsValid(CurrentX, CurrentY) && Tiles[CurrentX][CurrentY].Blocked)
		{
			return true;
		}
	}
	return false; // The area around the End Socket is fre
}

bool AProceduralGen::IsColliding(ARoomActor* Room, const FVector& SpawnLoc, const FRotator& SpawnRot)
{
	SCOPE_CYCLE_COUNTER(STAT_IsColliding);
	bool IsColliding = false;

	if (IsEndSocketOverlapping(Room, SpawnLoc)) return true;
	ForEachTileInRoom(Room, SpawnLoc, SpawnRot, [&](const int X, const int Z)
	{
		if (Tiles[X][Z].Blocked)
		{
			IsColliding = true;
		}
	});


	return IsColliding;
}

TArray<FIntPoint> AProceduralGen::SetTilesBlocked(ARoomActor* Room, const FVector& SpawnLoc, const FRotator& SpawnRot)
{
	TArray<FIntPoint> BlockedTiles;
	ForEachTileInRoom(Room, SpawnLoc, SpawnRot, [&](const int X, const int Z)
	{
		Tiles[X][Z].Blocked = true;
		Tiles[X][Z].Visited = true;
		BlockedTileHolder.Add(FIntPoint(X, Z)); //TODO: Not sure enough if this BlockedTileHolder must be removed
		BlockedTiles.Add(FIntPoint(X, Z));
	});

	if (Room->IsA(TurnCorridorClass) || Room->IsA(StraightCorrClass)) //TODO: i Changed here take a look later
	{
		Room->BlockedCorTiles = BlockedTiles;
	}
	else
	{
		Room->BlockedRoomTiles = BlockedTiles;
	}

	return BlockedTiles;
}

void AProceduralGen::SetSocketExclusion(ARoomActor* Room, FVector SpawnLoc)
{
	if (!Room) return;

	//Enter exclusions
	FIntPoint StartIndex = WorldToIndex(SpawnLoc);
	SetExclusion(Room, StartIndex, Room->EnterExclusionOffset, Room->EnterExclusions);
	// SwapZYaxis(Test);

	FVector Test = SwapZYaxis(Room->DoorSocketExit->GetRelativeLocation(), SpawnLoc);
	// SwapZYaxis(Test, SpawnLoc); //TODO: TEst thoruguhly 
	//Exit Exclusions
	FIntPoint EndIndex = WorldToIndex(Test);
	SetExclusion(Room, EndIndex, Room->ExitExclusionOffset, Room->ExitExclusions);

	if (Room->LargeRoom)
	{
		SocketExclusionForLargeRoom(Room);
	}
}

//TODO: This deserves a real extracted method or lambd it's name must be like "GetEachFIntPointBPArray" or something like that


void AProceduralGen::SocketExclusionForLargeRoom(ARoomActor* Room)
{
	TArray<USceneComponent*> SceneComponents;
	Room->GetComponents<USceneComponent*>(SceneComponents);

	auto RemoveEnterExitSocketFromPool = [&]()
	{
		//Remove Enter and Exit sockets from the array pool
		TArray<USceneComponent*> CompsToRemove;
		for (auto SceneComp : SceneComponents)
		{
			if (SceneComp->ComponentHasTag("Enter") || SceneComp->ComponentHasTag("Exit"))
			{
				CompsToRemove.Add(SceneComp);
			}
		}

		//Remove the stored ones from scene comp list.
		for (auto Component : CompsToRemove)
		{
			SceneComponents.Remove(Component);
		}
	};

	auto GetAllSceneCompNameAddOffsetName = [&]()
	{
		TArray<FName> SceneCompNames;
		for (auto SceneComp : SceneComponents)
		{
			FString Name = SceneComp->GetFName().ToString();
			Name.Append(TEXT("_Exclude"));
			SceneCompNames.Add(FName(*Name));
		}
		return SceneCompNames;
	};

	auto GetOffsetFromGivenVarName = [&]()
	{
		for (TFieldIterator<FProperty> PropIt(Room->GetClass()); PropIt; ++PropIt)
		{
			FProperty* Prop = *PropIt;
			FStructProperty* StructProp = CastField<FStructProperty>(Prop);
			if (StructProp && StructProp->Struct == TBaseStructure<FIntPoint>::Get())
			{
				TArray<USceneComponent*> SceneComp;
				Room->GetComponents<USceneComponent*>(SceneComp);

				FIntPoint* IntPoint = StructProp->ContainerPtrToValuePtr<FIntPoint>(Room);
				FName ChoppedName = FName(*StructProp->GetName().LeftChop(7));
				for (auto SceneComponent : SceneComp)
				{
					if (ChoppedName == SceneComponent->GetFName())
					{
						return *IntPoint;
					}
				}
			}
		}
		return FIntPoint(0, 0);
	};

	RemoveEnterExitSocketFromPool();
	TArray<FName> SceneCompNames = GetAllSceneCompNameAddOffsetName();

	//Get the class of the Room
	UClass* RoomClass = Room->GetClass();

	// Iterate through all properties of the Room
	for (TFieldIterator<FProperty> PropIt(RoomClass); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			FStructProperty* StructInnerProp = CastField<FStructProperty>(ArrayProp->Inner);
			if (StructInnerProp && StructInnerProp->Struct == TBaseStructure<FIntPoint>::Get())
			{
				for (auto Name : SceneCompNames)
				{
					if (StructInnerProp->GetFName() == Name)
					{
						// This is an array of FIntPoints. Do your logic here.
						FScriptArrayHelper_InContainer ArrayHelper(ArrayProp, Room);
						if (ArrayHelper.Num() > 0)
						{
							for (int i = 0; i < ArrayHelper.Num(); ++i)
							{
								FIntPoint* IntPoint = reinterpret_cast<FIntPoint*>(ArrayHelper.GetRawPtr(i));

								//1. Looks harder than it is. Foreach SceneComponents check if it's name is equal to the name of the array. But before we need to scrap the _Exclude from the name. suffix
								FName ChoppedName = FName(*Name.ToString().LeftChop(8));
								for (auto SceneComp : SceneComponents)
								{
									if (ChoppedName == SceneComp->GetFName())
									{
										FIntPoint Index = WorldToIndex(SceneComp->GetComponentLocation());
										FIntPoint Offset = GetOffsetFromGivenVarName();

										int CurrentX = Index.X + Offset.X + IntPoint->X;
										int CurrentY = Index.Y + Offset.Y + IntPoint->Y;

										if (IsValid(CurrentX, CurrentY))
										{
											Tiles[CurrentX][CurrentY].Blocked = false;
											Tiles[CurrentX][CurrentY].Visited = false;
										}

										if (IsValid(CurrentX, CurrentY) && VisualizeAllExclusions)
										{
											DrawDebugBox(World, Tiles[CurrentX][CurrentY].Location + FVector(TileSizeX / 2, TileSizeY / 2, 0), FVector(TileSizeX / 2, TileSizeY / 2, 0), FColor::Purple, true);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

bool AProceduralGen::ConnectRoomsWithCorridors(ARoomActor*& Room)
{
	FRoomConnection Connection = CalculatePathInfo(Room);

	FIntPoint StartIndex = WorldToIndex(Connection.StartPoint);
	FIntPoint EndIndex = WorldToIndex(Connection.EndPoint);

	// Find a path from StartIndex to EndIndex:
	{
		ResetAllVisited();
		return FindCorridorPath(StartIndex.X, StartIndex.Y, EndIndex.X, EndIndex.Y, Connection.PathStartOffset, Connection.PathEndOffset, true, Connection.MaxCheckAmount, Room);
	}
	return true;
}

void AProceduralGen::SetExclusion(ARoomActor* Room, const FIntPoint& Index, const FIntPoint& ExclusionOffset, TArray<FIntPoint> Exclusions)
{
	for (const FIntPoint& Point : Exclusions)
	{
		int CurrentX = Index.X + Point.X + ExclusionOffset.X;
		int CurrentY = Index.Y + Point.Y + ExclusionOffset.Y;

		if (IsValid(CurrentX, CurrentY))
		{
			Tiles[CurrentX][CurrentY].Blocked = false;
			Tiles[CurrentX][CurrentY].Visited = false;
		}

		if (IsValid(CurrentX, CurrentY) && VisualizeAllExclusions)
			DrawDebugBox(World, Tiles[CurrentX][CurrentY].Location + TileBoxExtends, TileBoxExtends, FColor::Purple, true);
	}
}

void AProceduralGen::SwapInvZYaxis(FVector& VectorToSwap)
{
	VectorToSwap.Y = -VectorToSwap.Z;
	VectorToSwap.Z = 0;
}

ARoomActor* AProceduralGen::SelectRoomWithDirection(const Direction EndSocketDirection, const bool CanSpawnLargeRoom, const bool OnlySpawnNoExit, TArray<ARoomActor*>* CustomArray, TSharedPtr<TArray<ARoomActor*>> ManualBranchRooms)
{
	SCOPE_CYCLE_COUNTER(STAT_SelectRoomWithDirection);
	//Lambda functions for condition checks
	auto IsLargeRoomAllowed = [&](const ARoomActor* Room) { return CanSpawnLargeRoom || !Room->LargeRoom; };
	auto IsNoExitStatusMatching = [&](const ARoomActor* Room) { return OnlySpawnNoExit == Room->NoExit; };

	TArray<ARoomActor*> IterateRoomDesigns = (CustomArray != nullptr) ? *CustomArray : CastedRoomDesigns;

	//Selection for side branch manual room selection
	if (ManualBranchRooms && !ManualBranchRooms->IsEmpty())
	{
		TArray<ARoomActor*> FirstDownCast = ManualBranchRooms.Get()[0];
		ARoomActor* FirstRoom = (FirstDownCast[0]);
		ManualBranchRooms->RemoveAt(0);
		if (CustomArray)
		{
			*CustomArray = *ManualBranchRooms.Get(); //Custom Array suppossed to be assigned by ref but not working. 
		}

		return FirstRoom;
	}

	//If DebugRoomSequence is given the sequence has to be selected in order. 
	if (!DebugRoomSequence.IsEmpty() && CurrentIndex < DebugRoomSequence.Num())
	{
		ARoomActor* SequenceRoom = Cast<ARoomActor>(DebugRoomSequence[CurrentIndex]->GetDefaultObject());
		CurrentIndex++;

		if (SequenceRoom->LargeRoom)
		{
			LargeRoomCounter++;
		}

		return SequenceRoom;
	}

	//Determine all the valid rooms could be selected 
	TArray<ARoomActor*> ValidRooms;
	for (auto RoomDesign : IterateRoomDesigns)
	{
		ARoomActor* RoomCandidate = Cast<ARoomActor>(RoomDesign);
		if (RoomCandidate && RoomCandidate->EnterSocketDirection == EndSocketDirection && IsLargeRoomAllowed(RoomCandidate) && IsNoExitStatusMatching(RoomCandidate))
		{
			ValidRooms.Add(RoomCandidate);
		}
		else if (CustomArray && !CustomArray->IsEmpty()) //For large room spawning. 
		{
			CustomArray->Remove(RoomCandidate);
		}
	}

	if (ValidRooms.Num() > 0)
	{
		int32 RoomIndex = FMath::RandRange(0, ValidRooms.Num() - 1);

		UE_LOG(LogTemp, Display, TEXT("Selected Room: %s"), *ValidRooms[RoomIndex]->GetName());
		return ValidRooms[RoomIndex];
	}

	return nullptr;
}

FVector AProceduralGen::CalculateTopLeftCorner(const ARoomActor* Room, const FVector& WorldLoc, const FRotator& Rotation, FVector& BoxExtends)
{
	FVector TopLeftCorner;
	//TODO: NOte that still I don't cover the rotations turn corridor might have, it's best to cover all of them once I learn little math 
	if (Rotation == FRotator(-0.000000, 90.000000, -90.000000)) //This means only vertical corridor. //TODO: LATER calculate everything taking rotation in account. To do that, I need to learn lots of cool math quats etc 
	{
		FVector BoxCompCenter = Room->BoxComponent->GetComponentLocation();
		TopLeftCorner.X = BoxCompCenter.X - BoxExtends.Z;
		TopLeftCorner.Y = BoxCompCenter.Y - BoxExtends.X;
		TopLeftCorner.Z = 0;

		//Swap BoxExtends. Because that's how it should be.
		BoxExtends = FVector(BoxExtends.Z, BoxExtends.Y, BoxExtends.X);
		return TopLeftCorner;
	}

	//And these just for //(Pitch=0.000000,Yaw=0.000000,Roll=-90.000000) 
	TopLeftCorner.X = WorldLoc.X - BoxExtends.X;
	TopLeftCorner.Y = WorldLoc.Y - BoxExtends.Z;
	TopLeftCorner.Z = 0;
	return TopLeftCorner;
}

void AProceduralGen::ForEachTileInRoom(const ARoomActor* Room, const FVector& SpawnLoc, const FRotator& Rotation, const TFunction<void(int X, int Z)>& TileAction)
{
	SCOPE_CYCLE_COUNTER(STAT_ForEachTileInRoom);
	if (!Room) return;

	FVector RelativeLoc = Room->BoxComponent->GetRelativeLocation();
	SwapInvZYaxis(RelativeLoc);

	FVector WorldLoc = SpawnLoc + RelativeLoc;
	FVector BoxExtends = Room->BoxComponent->GetUnscaledBoxExtent();

	//TODO: I don't have any idea what's going on here I guess I have to fix all these bullshits with proper transformation math for entire class
	if (Room->IsHorizontalStraightCorr)
	{
		int X = BoxExtends.X;
		int Z = BoxExtends.Z;
		BoxExtends = FVector(Z, BoxExtends.Y, X);
		WorldLoc -= FVector(TileSizeX, 0, 0);
	}

	FVector TopLeftCorner = CalculateTopLeftCorner(Room, WorldLoc, Rotation, BoxExtends);
	FIntPoint StartIndex = WorldToIndex(TopLeftCorner);

	// Amount of tile the room has
	int TilesX = FMath::CeilToInt(BoxExtends.X * 2 / TileSizeX);
	int TilesZ = FMath::CeilToInt(BoxExtends.Z * 2 / TileSizeY);

	for (int x = 0; x < TilesX; ++x)
	{
		for (int z = 0; z < TilesZ; ++z)
		{
			int CurrentTileX = (StartIndex.X + x);
			int CurrentTileZ = (StartIndex.Y + z);
			if (IsValid(CurrentTileX, CurrentTileZ))
			{
				TileAction(CurrentTileX, CurrentTileZ);
			}
		}
	}
}

bool AProceduralGen::SpawnCorridors(const int GoalX, const int GoalY, ARoomActor* OverlappedRoom, const bool& CheckZigZag)
{
	TArray<FVector> LocStack;
	TArray<FRotator> RotStack;
	TArray<EDirection2> DirStack;
	FTileStruct* Current = &Tiles[GoalX][GoalY];
	TSet<FTileStruct*> VisitedNodes;

	TArray<FVector> SpawnLocations;
	TArray<FRotator> SpawnRotations;
	TArray<ARoomActor*> TurnCorridors;

	struct FTurnCorridorProps
	{
		FVector Location;
		FRotator Rotation;

		FTurnCorridorProps(const FVector& Location, const FRotator& Rotation): Location(Location), Rotation(Rotation)
		{
		}
	};

	TArray<FTurnCorridorProps> TurnCorridorStruct;

	while (Current != nullptr)
	{
		if (VisitedNodes.Contains(Current))
		{
			// Detected a cycle
			UE_LOG(LogTemp, Warning, TEXT("Detected a cycle in path traversal at node (%d, %d)"), Current->X, Current->Y);
			break; // Break out of the loop
		}
		VisitedNodes.Add(Current);

		Tiles[Current->X][Current->Y].Path = true;
		LocStack.Add(Tiles[Current->X][Current->Y].Location);
		DirStack.Push(Tiles[Current->X][Current->Y].Direction);
		RotStack.Push(Tiles[Current->X][Current->Y].Rotation);
		Current = Current->Parent;
	}

	//First assign CurrDir and pop it from DirStack
	EDirection2 CurrDir = DirStack.Top();
	DirStack.Pop();

	//Secondly peek next dir after popping CurrDir previously
	if (DirStack.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Dirstack empty"));
		return false;
	}
	EDirection2 NextDir = DirStack.Top();
	CurrDir = DetermineFirstDirection(NextDir);

	if (CurrDir == Dir_None)
		CurrDir = DetermineFirstDirection(LastSpawnedRoom->ExitSocketDirection);

	// First corridor logic
	if (!LocStack.IsEmpty())
	{
		//Check first current and next corr dir and spawn straight if not spawn turned in else 
		// FVector SpawnLoc = LocStack[LocStack.Num()-1];
		FVector SpawnLoc = LocStack.Top();
		if (CurrDir == NextDir)
		{
			SpawnLocations.Add(SpawnLoc);
			SpawnRotations.Add(RotStack.Top());
		}
		else
		{
			//First corridor is not straight. It will be turned. 
			FRotator FirstCorrRot = DetermineFirstTurnRotation(NextDir);
			SpawnLoc.Z += 1;

			//Assigning rot is done. Now spawn turned corr
			if (FirstCorrRot != FRotator(31, 31, 31))
			{
				if (FirstCorrRot == FRotator(0, 0, -90) || FirstCorrRot == FRotator(0, 90, -90))
				{
					SpawnLocations.Add(SpawnLoc);
					SpawnRotations.Add(FirstCorrRot);
				}
				else
				{
					TurnCorridorStruct.Add(FTurnCorridorProps(SpawnLoc, FirstCorrRot));
				}
			}
		}

		LocStack.Pop();
		RotStack.Pop();
		CurrDir = DirStack.Top();
		DirStack.Pop();
	}

	//Middle corridors between first and last 
	while (LocStack.Num() > 1)
	{
		// Set the next direction if available
		if (!DirStack.IsEmpty())
		{
			NextDir = DirStack.Top();
			DirStack.Pop();
		}
		else
		{
			NextDir = Dir_None;
		}

		FVector SpawnLoc = LocStack.Top();
		if (CurrDir == NextDir)
		{
			SpawnLocations.Add(SpawnLoc);
			SpawnRotations.Add(RotStack.Top());
		}
		else
		{
			// Assuming TurnCorridorClass has the right rotation logic inside it
			FRotator TurnCorrRot = DetermineMiddleTurnRotation(CurrDir, NextDir);
			SpawnLoc.Z += 1;
			if (TurnCorrRot != FRotator(31, 31, 31))
			{
				TurnCorridorStruct.Add(FTurnCorridorProps(SpawnLoc, TurnCorrRot));
			}
		}
		LocStack.Pop();
		RotStack.Pop();
		CurrDir = NextDir;
	}

	//Last corridor logic. After while loop over last corridor will be left
	FVector SpawnLoc = LocStack.Top();
	FRotator LastCorrRot = DetermineLastCorrRotation(CurrDir);

	EDirection2 NextRoomEnterDir = ConvertNextRoomEnterTagToDirection();

	if ((CurrDir == NextRoomEnterDir) || LastCorrException(CurrDir, NextRoomEnterDir))
	{
		SpawnLocations.Add(SpawnLoc);
		SpawnRotations.Add(RotStack.Top());
	}
	else
	{
		SpawnLoc.Z += 1;
		if (LastCorrRot != FRotator(31, 31, 31))
		{
			TurnCorridorStruct.Add(FTurnCorridorProps(SpawnLoc, LastCorrRot));
		}
	}

	LocStack.Pop();
	RotStack.Pop();
	CurrDir = NextDir;

	//First check if two turn corridor overlapping with each other
	if (CheckZigZag)
	{
		TArray<ARoomActor*> SpawnedTurnedCorr;
		for (auto TurnCorr : TurnCorridorStruct)
		{
			ARoomActor* TurnCorridor = World->SpawnActor<ARoomActor>(TurnCorridorClass, TurnCorr.Location, TurnCorr.Rotation);
			SpawnedTurnedCorr.Add(TurnCorridor);
			AllSpawnedCorridors.Add(TurnCorridor);
		}
		for (auto TurnCorr : SpawnedTurnedCorr)
		{
			TArray<AActor*> OverlappingActors;
			TurnCorr->GetOverlappingActors(OverlappingActors);
			for (auto OverlappingActor : OverlappingActors)
			{
				if (OverlappingActor->IsA(TurnCorridorClass))
				{
					for (auto TurnCorridor : SpawnedTurnedCorr)
					{
						TurnCorridor->Destroy();
						//TODO: I didn't called called destroy for AllSpawnedCorridors later on come check here
					}
					return false;
				}
			}
		}

		//If true, this function call is only meant to be check whether there's zigzag. There should not be corridor spawning
		for (auto TurnedCorr : SpawnedTurnedCorr)
		{
			TurnedCorr->Destroy();
		}
		return true;
	}


	for (auto TurnCorr : TurnCorridorStruct)
	{
		ARoomActor* TurnCorridor = World->SpawnActor<ARoomActor>(TurnCorridorClass, TurnCorr.Location, TurnCorr.Rotation);
		TurnCorridors.Add(TurnCorridor);
		TurnCorridor->IsCorridor = true;
		SetTilesBlocked(TurnCorridor, SpawnLoc, TurnCorr.Rotation);
		OverlappedRoom->OwnerCorridors.Add(TurnCorridor);
		TurnCorridor->IfCorridorOwnerRoom = OverlappedRoom;
		AllSpawnedCorridors.Add(TurnCorridor);
	}

	//Spawn straight corridors at the end
	for (int i = 0; i < SpawnLocations.Num(); ++i)
	{
		ARoomActor* NormalCorr = World->SpawnActor<ARoomActor>(StraightCorrClass, SpawnLocations[i], SpawnRotations[i]);
		NormalCorr->IsCorridor = true;
		NormalCorr->IfCorridorOwnerRoom = OverlappedRoom;
		if (SpawnRotations[i] == FRotator(0, -90, -90)) //TODO: I donno this but I am sure this has to gone sooner or later
		{
			NormalCorr->IsHorizontalStraightCorr = true;
		}
		SetTilesBlocked(NormalCorr, SpawnLocations[i], SpawnRotations[i]);
		OverlappedRoom->OwnerCorridors.Add(NormalCorr);
		AllSpawnedCorridors.Add(NormalCorr);
	}

	//Destroy overlapped ones
	for (auto TurnCorridor : TurnCorridors)
	{
		TArray<AActor*> OverlappingActors;
		TurnCorridor->GetOverlappingActors(OverlappingActors);
		for (auto OverlappingActor : OverlappingActors)
		{
			if (OverlappingActor->IsA(StraightCorrClass))
			{
				OverlappingActor->Destroy();
			}
		}
	}
	return true;
}

void AProceduralGen::VisualizeTiles()
{
	//Since I calling world in editor, I need to access world first
	for (int X = 0; X < MapSizeX; ++X)
	{
		for (int Y = 0; Y < MapSizeY; ++Y)
		{
			bool Condition = VisualizeOverlaps ? Tiles[X][Y].Blocked : Tiles[X][Y].Visited;
			FColor Color = VisualizeOverlaps ? FColor::Yellow : FColor::Magenta;
			if (Condition)
			{
				DrawDebugBox(World, Tiles[X][Y].Location + TileBoxExtends, TileBoxExtends, Color, true);
			}
		}
	}

	//For MoveOverlapRoomLocationTiles visualization
	if (VisualizeOverlappedRoomTiles)
	{
		for (int i = 0; i < MoveOverlapRoomLocationTiles.Num(); ++i)
		{
			FColor Color = (i == MoveOverlapRoomLocationTiles.Num() - 1) ? FColor::Red : FColor::White;
			const FIntPoint& Tile = MoveOverlapRoomLocationTiles[i];
			DrawDebugBox(World, Tiles[Tile.X][Tile.Y].Location + TileBoxExtends, TileBoxExtends, Color, true);
		}
	}
}

void AProceduralGen::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//Get the name of the property that was changed
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//Check if the property changed is VisualizeOverlaps
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProceduralGen, VisualizeOverlaps))
	{
		if (VisualizeOverlaps)
		{
			VisualizeVisited = false;
		}
		return;
	}
	//Check if the property changed is VisualizeOverlaps
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AProceduralGen, VisualizeVisited))
	{
		if (VisualizeVisited)
		{
			VisualizeOverlaps = false;
		}
		return;
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProceduralGen, FixedTwoRoomActor))
	{
		if (FixedTwoRoomActor.Num() > 2)
		{
			while (FixedTwoRoomActor.Num() > 2)
			{
				FixedTwoRoomActor.RemoveAt(FixedTwoRoomActor.Num() - 1);
			}
		}
	}
}

bool AProceduralGen::MoveOverlappedRoom(ARoomActor* NextRoom, FVector& NextRoomLocation)
{
	if (LastSelectedOverlappingRoom && LastSelectedOverlappingRoom == NextRoom)
	{
		OverlappedRoomMoverCounter++;
	}
	SCOPE_CYCLE_COUNTER(STAT_MoveOverlappedRoom);
	TSet<FIntPoint> VisitedTiles;
	int Attempts = 0;
	int InnerAttempts = 0;

	int MaxMoveOverlappedRoomIterate = NextRoom->LargeRoom ? MaxMoveOverlappedRoomIterateBranchRoom : MaxMoveOverlappedRoomIterateNormalRoom;
	
	// Get the starting point from the NextRoomLocation's tile indexes
	FIntPoint StartIndex = WorldToIndex(NextRoomLocation);
	int StartX = StartIndex.X;
	int StartY = StartIndex.Y;
	
	int MaxDistance = FMath::Max(MapSizeX, MapSizeY);
	
	for (int Dist = 1; Dist <= MaxDistance; ++Dist)
	{
		for (int x = StartX - Dist; x <= StartX + Dist; ++x)
		{
			for (int y = StartY - Dist; y <= StartY + Dist; ++y)
			{
				// Ensure the determined position is suitable to be spawned
				if (IsValid(x, y) && !VisitedTiles.Contains(FIntPoint(x, y)) &&
					(!IsColliding(NextRoom, Tiles[x][y].Location, NextRoom->Rotation)) &&
					(!IsBufferZoneColliding(NextRoom, Tiles[x][y].Location)))
				{
					InnerAttempts++;
					if (CanMakeCorridorPathBeforeSpawning(NextRoom, Tiles[x][y].Location))
					{
						NextRoomLocation = Tiles[x][y].Location;
						OverlappedRoomMoverCounter = 0;
						
						UE_LOG(LogTemp, Display, TEXT("total attemps: %i"), Attempts);
						UE_LOG(LogTemp, Display, TEXT("Inner attemps: %i"), InnerAttempts);

						return true;
					}
				}
				++Attempts;
				// UE_LOG(LogTemp, Display, TEXT("total attemps: %i"), Attempts);
				// UE_LOG(LogTemp, Display, TEXT("Inner attemps: %i"), InnerAttempts);

				if (Attempts > MaxMoveOverlappedRoomIterate)
				{
					LastSelectedOverlappingRoom = NextRoom;
					return false;
				}

				VisitedTiles.Add(FIntPoint(x, y));
				MoveOverlapRoomLocationTiles.Add(FIntPoint(x, y)); //if VisualizeOverlappedRoomTiles then make it work but before I need to compare their timings 
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("Nothing has been found"));
	return false;
}

bool AProceduralGen::IsBufferZoneColliding(ARoomActor* Room, FVector SpawnLoc)
{
	int BufferX = BufferSize * TileSizeX;
	int BufferY = BufferSize * TileSizeY;
	//Box comp's World Location
	FVector RelativeLoc = Room->BoxComponent->GetRelativeLocation();
	SwapInvZYaxis(RelativeLoc);

	FVector WorldLoc = SpawnLoc + RelativeLoc;
	FVector BoxExtends = Room->BoxComponent->GetUnscaledBoxExtent() + FVector(BufferX, 0, BufferY);
	FVector TopLeftCorner = CalculateTopLeftCorner(Room, WorldLoc, Room->Rotation, BoxExtends);
	FIntPoint StartIndex = WorldToIndex(TopLeftCorner);

	// Dimensions of each tile
	int TilesX = FMath::CeilToInt(BoxExtends.X * 2 / TileSizeX);
	int TilesZ = FMath::CeilToInt(BoxExtends.Z * 2 / TileSizeY);

	for (int x = 0; x < TilesX; ++x)
	{
		for (int z = 0; z < TilesZ; ++z)
		{
			int CurrentTileX = (StartIndex.X + x);
			int CurrentTileZ = (StartIndex.Y + z);
			if (IsValid(CurrentTileX, CurrentTileZ))
			{
				if (Tiles[CurrentTileX][CurrentTileZ].Blocked)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int AProceduralGen::MakeCorridorPathVisualization(ARoomActor* OverlappedRoom, FTileStruct* Current)
{
	FTileStruct* PrevNode = nullptr; // Keep track of the previous node
	FTileStruct* PathNode = Current;
	int PathCost = 0;

	while (PathNode != nullptr)
	{
		PathCost++;
		if (PathNode->Parent == PrevNode)
		{
			UE_LOG(LogTemp, Error, TEXT("Room %s Circular reference detected between nodes. Breaking out of loop."), *OverlappedRoom->GetName());
			break;
		}
		if (VisualizeCorridorPath)
		{
			DrawDebugBox(World, Tiles[PathNode->X][PathNode->Y].Location + FVector(TileSizeX / 2, TileSizeY / 2, 0), FVector(TileSizeX / 2, TileSizeY / 2, TileSizeY), FColor::Red, true, -1, 0, 0);
		}
		PrevNode = PathNode;
		PathNode = PathNode->Parent;
	}
	return PathCost;
}

EDirection2 LastDirection = EDirection2::Dir_None;
int DirectionCounter = 0;
TArray<FTileStruct*> Path;

bool AProceduralGen::FindCorridorPath(int StartX, int StartY, int GoalX, int GoalY, FIntPoint StartOffset, FIntPoint EndOffset, bool SpawnCorr, int MaxIterationAmount, ARoomActor* OverlappedRoom)
{
	TArray<int> Row = {-1, 0, 0, 1};
	TArray<int> Col = {0, -1, 1, 0};
	TArray<FTileStruct*> OpenList;
	int SafeCheck = 0;
	int SecondCounter = 0;
	FoundPathCost = INT_MAX;


	auto ApplyOffsets = [&]()
	{
		StartX += StartOffset.X;
		StartY += StartOffset.Y;
		GoalX += EndOffset.X;
		GoalY += EndOffset.Y;
	};

	auto ShouldEarlyReturn = [&]()
	{
		if (!IsValid(StartX, StartY))
		{
			UE_LOG(LogTemp, Display, TEXT("Room %s Out of bound so I returned: %i, %i "), *OverlappedRoom->GetName(), StartX, StartY);
			return true;
		}
		if (!IsValid(GoalX, GoalY) || Tiles[GoalX][GoalY].Blocked)
		{
			UE_LOG(LogTemp, Error, TEXT("Room %s Goal tile is blocked or not valid. Disable visualize overlaps. Enable Visualize Begin and End tiles. Manually offset the Path End Offset."), *OverlappedRoom->GetName());
			return true;
		}
		return false;
	};

	auto SetFirstCorrRotation = [&](FTileStruct* Start)
	{
		if (LastSpawnedRoom->ExitSocketDirection == HorizontalRight || LastSpawnedRoom->ExitSocketDirection == HorizontalLeft)
			Start->Rotation = FRotator(0, 0, -90);
		else
		{
			Start->Rotation = FRotator(0, 90, -90); //(Pitch=0.000000,Yaw=90.000000,Roll=-90.000000)
		}
	};

	auto InitializeStartNode = [&]()
	{
		// StartX++;
		FTileStruct* Start = &Tiles[StartX][StartY];
		Start->Visited = true;
		Start->HCost = FPathNode::GetHCost(StartX, StartY, GoalX, GoalY);
		SetFirstCorrRotation(Start);
		OpenList.Add(Start);
	};

	//No sorting involved! 
	auto GetMinimumFCostTile = [&]()
	{
		// Find the node with the smallest FCost
		int minIndex = 0;
		for (int i = 1; i < OpenList.Num(); ++i)
		{
			if (OpenList[i]->FCost() < OpenList[minIndex]->FCost())
				minIndex = i;
		}

		FTileStruct* Current = OpenList[minIndex];
		OpenList.RemoveAt(minIndex);
		return Current;
	};

	auto FindNeighbor = [&](FTileStruct* Current)
	{
		for (int i = 0; i < 4; i++)
		{
			SecondCounter++;
			int NewX = Current->X + Row[i];
			int NewY = Current->Y + Col[i];
			FVector TileWorldPos = IndexToWorld(NewX, NewY);
			ARoomActor* Corridor = Cast<ARoomActor>(TurnCorridorClass->GetDefaultObject()); //TODO: WHy just casting and checking colliding based on TurnCorridor
			Corridor->IsCorridor = true;
			if (IsValid(NewX, NewY) && !Tiles[NewX][NewY].Visited && !IsColliding(Corridor, TileWorldPos, OverlappedRoom->Rotation))
			{
				FTileStruct* Neighbour = &Tiles[NewX][NewY];
				int TurnPenalty = 0;

				//If next considered Neighbour is going to turn, add turn penalty
				if (Current->Parent != nullptr && (Current->X - Current->Parent->X != Row[i] || Current->Y - Current->Parent->Y != Col[i]))
				{
					TurnPenalty = TurnPenaltyAmount;
				}

				int NewMovementCostToNeighbour = Current->GCost + 1 + TurnPenalty; // assuming each move costs 1

				if (NewMovementCostToNeighbour < Neighbour->GCost || !OpenList.Contains(Neighbour))
				{
					Neighbour->GCost = NewMovementCostToNeighbour;
					Neighbour->HCost = FPathNode::GetHCost(NewX, NewY, GoalX, GoalY);
					Neighbour->Parent = Current;
					DeterminePathDirection(Current, NewX, NewY, Neighbour);
					OpenList.Add(Neighbour);
				}
				Tiles[NewX][NewY].Visited = true;
			}
		}
	};


	ApplyOffsets();
	// if (ShouldEarlyReturn()) return false;
	InitializeStartNode();

	while (OpenList.Num() > 0 && SafeCheck < MaxIterationAmount)
	{
		FTileStruct* Current = GetMinimumFCostTile();

		if (Current->X == GoalX && Current->Y == GoalY)
		{
			if (SpawnCorr)
			{
				SpawnCorridors(GoalX, GoalY, OverlappedRoom, false);
			}
			else
			{
				ResetAllVisited();
				if (!SpawnCorridors(GoalX, GoalY, OverlappedRoom, true)) return false;
			}

			OverlappedRoom->PathCost = MakeCorridorPathVisualization(OverlappedRoom, Current);
			UE_LOG(LogTemp, Error, TEXT("Safecheck: %i"), SafeCheck);
			return true;
		}
		else
		{
			FindNeighbor(Current);
			SafeCheck++;
		}
	}
	
	return false;
}

void AProceduralGen::VisualizeBeginEndTiles(ARoomActor* NextRoom, const FRoomConnection& Connection)
{
	FIntPoint StartIndex = WorldToIndex(LastSpawnedRoom->DoorSocketExit->GetComponentLocation());
	DrawDebugBox(World, Tiles[StartIndex.X + Connection.PathStartOffset.X][StartIndex.Y + Connection.PathStartOffset.Y].Location + FVector(TileSizeX / 2, TileSizeY / 2, 0), FVector(TileSizeX / 2, TileSizeY / 2, TileSizeY / 2), FColor::Cyan,
	             true);
	FIntPoint EndIndex = WorldToIndex(NextRoom->DoorSocketEnter->GetComponentLocation());
	DrawDebugBox(World, Tiles[EndIndex.X + Connection.PathEndOffset.X][EndIndex.Y + Connection.PathEndOffset.Y].Location + FVector(TileSizeX / 2, TileSizeY / 2, 0), FVector(TileSizeX / 2, TileSizeY / 2, TileSizeY / 2), FColor::Cyan, true);
}

void AProceduralGen::ResetAllVisited()
{
	for (int x = 0; x < MapSizeX; ++x)
		for (int y = 0; y < MapSizeY; ++y)
			Tiles[x][y].Visited = false;
}

void AProceduralGen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
