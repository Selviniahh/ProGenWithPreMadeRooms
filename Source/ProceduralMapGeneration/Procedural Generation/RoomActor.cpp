// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomActor.h"
#include "Components/BoxComponent.h"
#include "Door/DoorActor.h"


// Sets default values

ARoomActor::ARoomActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootScene->SetRelativeLocation(FVector(0,0,0));
	RootComponent = RootScene;
	
	//No need to give something in BP for now
	// TileMapComponent = CreateDefaultSubobject<UPaperTileMapComponent>(TEXT("TileMapComponent"));
	// TileMapComponent->SetupAttachment(RootComponent);

	DoorSocketEnter = CreateDefaultSubobject<USceneComponent>(TEXT("DoorSocketEnter"));
	DoorSocketEnter->SetupAttachment(RootComponent);
	DoorSocketEnter->ComponentTags.Add("Enter");

	DoorSocketExit = CreateDefaultSubobject<USceneComponent>(TEXT("DoorSocketExit"));
	DoorSocketExit->SetupAttachment(RootComponent);
	DoorSocketExit->ComponentTags.Add("Exit");

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	
	ValidTags = { TEXT("SideRight"), TEXT("SideLeft"), TEXT("StraightUp"), TEXT("StraightDown") };

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false);
}

void ARoomActor::SetEnterDoorActor(ADoorActor* DoorActor)
{
	EnterDoorActor = DoorActor;
	if (EnterDoorActor)
	{
		EnterDoorActor->OnDoorEndOverlap.AddDynamic(this, &ARoomActor::CategorizeAndSortSceneCompsByTag);
	}
}

void ARoomActor::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add("RoomActor");
	DoorSocketEnter->ComponentTags.Add("Enter");
	DoorSocketExit->ComponentTags.Add("Exit");
}

void ARoomActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoomActor::VisualizeAllBlocked()
{
}

void ARoomActor::VisualizeBeginEndTiles()
{
	
}

void ARoomActor::CategorizeAndSortSceneCompsByTag()
{
	// if (!DoOnce) return;
	// TArray<USceneComponent*> SceneComp;
	// this->GetComponents<USceneComponent>(SceneComp);
	//
	// //Clear previous categorization
	// TaggedSceneComponents.Empty();
	//
	// for (auto Comp : SceneComp)
	// {
	// 	if (Comp->ComponentTags.Num() > 0)
	// 	{
	// 		FString Tag = Comp->ComponentTags[0].ToString();
	// 		int32 TagAsInt = FCString::Atoi(*Tag);
	// 		if (TagAsInt != 0)
	// 		{
	// 			//Check if the TMap already contains this tag
	// 			if (!TaggedSceneComponents.Contains(TagAsInt))
	// 			{
	// 				TaggedSceneComponents.Add(TagAsInt, TArray<USceneComponent*>());
	// 			}
	//
	// 			TaggedSceneComponents[TagAsInt].Add(Comp);
	// 		}
	// 	}
	// }
	//
	// for (auto KeyValue : TaggedSceneComponents)
	// {
	// 	Algo::Sort(KeyValue.Value, [](const USceneComponent* A, const USceneComponent* B)
	// 	{
	// 		if (A->ComponentTags.Num() >= 1 && B->ComponentTags.Num() >= 1)
	// 			return FCString::Atoi(*A->ComponentTags[0].ToString()) < FCString::Atoi(*B->ComponentTags[0].ToString());
	// 		
	// 		return false;
	// 	});
	// }
	//
	// // Spawn enemies
	//  for (auto KeyVale : TaggedSceneComponents)
	//  {
	//  	for (auto Comp : KeyVale.Value)
	//  	{
	//  		// // GetWorld()->SpawnActor<AEnemyBase>(EnemyClass[KeyVale], Comp->GetComponentLocation(), FRotator::ZeroRotator);
	//  		GetWorld()->SpawnActor<AEnemyBase>(EnemyClass[FCString::Atoi(*Comp->ComponentTags[1].ToString())], Comp->GetComponentLocation() + FVector(0,0,2), FRotator::ZeroRotator);
	//  		// // SpawnedEnemies.Add(SpawnedEnemy);
	//  	}
	//  }
	// DoOnce = false;
}

bool ARoomActor::CheckFirstTagValid(USceneComponent* SceneComp) const
{
	if (SceneComp->ComponentTags.Num() > 0 && ValidTags.Contains(SceneComp->ComponentTags[0].ToString()))
	{
		return true;
	}
	return false;
}

void ARoomActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARoomActor, LargeRoom))
	{
		if (LargeRoom)
		{
			NoExit = false;
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ARoomActor, NoExit))
	{
		if (NoExit)
		{
			LargeRoom = false;
		}
	}
}

