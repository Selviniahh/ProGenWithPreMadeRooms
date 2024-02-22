// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/Test/ProGenWidgetTests.h"

#include "Components/BoxComponent.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"

void UProGenWidgetTests::Initialize(ARoomActor* First, ARoomActor* Second)
{
	PluginSettings = GetDefault<UPluginSettings>();
	
	this->FirstRoom = First;
	this->SecondRoom = Second;
	ProGen = PluginSettings->ProGenInst.Get();
	ProGen->InitWorldTiles();
	ProGen->TileBoxExtends = FVector(ProGen->TileSizeX / 2, ProGen->TileSizeY / 2, ProGen->TileSizeY / 2);
}

ARoomActor* UProGenWidgetTests::MakeOverlapTest()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return nullptr;

	//Spawn the selected first room always at the center
	FVector SpawnLoc = ProGen->Tiles[ProGen->MapSizeX /2][ProGen->MapSizeY /2].Location;
	FirstRoom = World->SpawnActor<ARoomActor>(FirstRoom->GetClass(),SpawnLoc,ProGen->DefaultRotation);
	FirstRoom->Location = SpawnLoc;

	FVector BoxComp = FirstRoom->BoxComponent->GetScaledBoxExtent() * 2;
	double MaxDouble = FMath::Max3(BoxComp.X, BoxComp.Y, BoxComp.Z);

	//TODO: I need to set render target exactly center of the room not the one at room's specified origin. Calculate the box comp center and set it to render target's location
	PluginSettings->SceneCapActorInst->SetActorLocation(FirstRoom->GetActorLocation() + FVector(0,0,MaxDouble));

	//now it's time to make box overlap
	ProGen->VisualizeOverlaps = true;
	ProGen->SetTilesBlocked(FirstRoom,SpawnLoc,ProGen->DefaultRotation);
	ProGen->SetSocketExclusion(FirstRoom,SpawnLoc);
	ProGen->VisualizeTiles();

	return FirstRoom;
}

FVector UProGenWidgetTests::DetermineSecondRoomSpawnLocation()
{
	FVector Extends = FirstRoom->BoxComponent->GetScaledBoxExtent() * 4;
	FVector SecRoomOffset = FVector::Zero();
	
	switch (FirstRoom->ExitSocketDirection)
	{
	case HorizontalRight:
		SecRoomOffset.X = Extends.X;
		break;
	case HorizontalLeft:
		SecRoomOffset.X = -Extends.X;
		break;
	case VerticalUp:
		SecRoomOffset.Y = Extends.Y;
		break;
	case VerticalDown:
		SecRoomOffset.Y = -Extends.Y;
		break;
	default: ;
	}

	return SecRoomOffset;
}
