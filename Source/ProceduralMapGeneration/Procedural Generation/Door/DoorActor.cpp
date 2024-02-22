// Fill out your copyright notice in the Description page of Project Settings.

#include "DoorActor.h"
#include "Components/BoxComponent.h"


// Sets default values
ADoorActor::ADoorActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;
	
	// DoorFBRight = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DoorFBRight"));
	// DoorFBRight->SetupAttachment(RootComponent);
	// DoorFBRight->SetLooping(false);
	// DoorFBRight->SetPlayRate(0);
	//
	// DoorFBLeft = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DoorFBLeft"));
	// DoorFBLeft->SetupAttachment(RootComponent);
	// DoorFBLeft->SetLooping(false);
	// DoorFBLeft->SetPlayRate(0);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADoorActor::BeginPlay()
{
	Super::BeginPlay();
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnBoxComponentBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::OnBoxComponentEndOverlap);
	
}

void ADoorActor::OnBoxComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// UE_LOG(LogTemp, Warning, TEXT("End Overlap"));
	// if (Cast<AHero>(OtherActor))
	// {
	// 	OnDoorEndOverlap.Broadcast();
	// }

}

void ADoorActor::OnBoxComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Begin Overlap"));

	// if (AHero* Hero = Cast<AHero>(OtherActor))
	// {
	// 	FVector2D DoorSize;
	// 	if (DoorFBRight)
	// 	{
	// 		DoorSize = DoorFBRight->GetFlipbook()->GetSpriteAtFrame(0)->GetSourceSize();
	// 	}
	// 	else
	// 	{
	// 		DoorSize = DoorFBLeft->GetFlipbook()->GetSpriteAtFrame(0)->GetSourceSize();
	// 	}
	//
	// 	FVector HeroLoc = Hero->GetActorLocation();
	// 	FVector2D SpriteSize = Hero->FlipBook->GetFlipbook()->GetSpriteAtFrame(0)->GetSourceSize();
	//
	// 	HeroLoc.X -= (SpriteSize.X / 2);
	// 	HeroLoc.Y -= (SpriteSize.Y / 2);
	//
	// 	FVector DoorLoc = GetActorLocation();
	// 	DoorLoc.X -= (DoorSize.X / 2);
	// 	DoorLoc.Y -= (DoorSize.Y / 2);
	//
	// 	FVector Impact = HeroLoc - DoorLoc;
	// 	Impact.Normalize();
	// 	UE_LOG(LogTemp, Display, TEXT("Impact: %s Normal: %s"), *Impact.ToString(), *SweepResult.ImpactNormal.ToString());
	//
	// 	UPaperFlipbookComponent* DoorFB = DoorFBLeft ? DoorFBLeft : DoorFBRight;
	// 	if (Impact.X > 0)
	// 	{
	// 		SetActorScale3D(FVector(-1,1,1));
	// 		
	// 		FVector ActorLoc = GetActorLocation();
	// 		SetActorLocation(FVector(DoorSize.X + ActorLoc.X,ActorLoc.Y,ActorLoc.Z));
	// 	}
	// }
}


// Called every frame
void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

