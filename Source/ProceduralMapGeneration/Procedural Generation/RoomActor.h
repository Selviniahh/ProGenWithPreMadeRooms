// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomActor.generated.h"


class AProceduralGen;
class ADoorActor;
class UBoxComponent;

UENUM()
enum Direction
{
	HorizontalRight,
	HorizontalLeft,
	VerticalUp,
	VerticalDown
};

UCLASS()
class PROCEDURALMAPGENERATION_API ARoomActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoomActor();
	bool CheckFirstTagValid(USceneComponent* SceneComp) const;

	UPROPERTY()
	int PathCost = 0;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool LargeRoom;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool NoExit;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Door", meta=(DisplayPriority = 1))
	TSubclassOf<ADoorActor> EnterDoor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Door", meta=(DisplayPriority = 2))
	TSubclassOf<ADoorActor> ExitDoor;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Door", meta=(DisplayPriority = 3))
	TSubclassOf<ADoorActor> NoExitVerticalUp;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Door", meta=(DisplayPriority = 4))
	TSubclassOf<ADoorActor> NoExitVerticalDown;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Door", meta=(DisplayPriority = 5))
	TSubclassOf<ADoorActor> NoExitHorizontalRight;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Door", meta=(DisplayPriority = 5))
	TSubclassOf<ADoorActor> NoExitHorizontalLeft;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USceneComponent* RootScene;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USceneComponent* DoorSocketEnter;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USceneComponent* DoorSocketExit;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FIntPoint PathStartOffset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FIntPoint PathEndOffset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Exclusions", meta=(DisplayPriority = 1))
	TArray<FIntPoint> EnterExclusions;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Exclusions", meta=(DisplayPriority = 2))
	TArray<FIntPoint> ExitExclusions;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Exclusions", meta=(DisplayPriority = 3))
	FIntPoint EnterExclusionOffset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Exclusions", meta=(DisplayPriority = 4))
	FIntPoint ExitExclusionOffset;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Exclusions", meta=(DisplayPriority = 5))
	TArray<FIntPoint> ExitSocketChecks;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Exclusions", meta=(DisplayPriority = 6))
	FIntPoint ExitSocketCheckOffset;
	
	TArray<FString> ValidTags;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool VisualizeBlocked = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TEnumAsByte<Direction> EnterSocketDirection;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TEnumAsByte<Direction> ExitSocketDirection;
	
	UFUNCTION(BlueprintCallable)
	void VisualizeAllBlocked();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool VisualizeBeginAndEndTiles = false;

	UFUNCTION(BlueprintCallable)
	void VisualizeBeginEndTiles();

	UFUNCTION(Blueprintable)
	void CategorizeAndSortSceneCompsByTag();
	
	//Just will be used with Branch Connection
	bool IsOverlapped;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int WaveAmount = 1;

	TArray<FIntPoint> BlockedRoomTiles; 
	TArray<FIntPoint> BlockedCorTiles;
	
	bool DoOnce = true;
	bool IsHorizontalStraightCorr = false;

	UPROPERTY()
	ADoorActor* EnterDoorActor;

	TMap<int32, TArray<USceneComponent*>> TaggedSceneComponents;

	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// TArray<TSubclassOf<AEnemyBase>> EnemyClass;

	// TMap<ARoomActor*, FVector >
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<ARoomActor*> OwnerCorridors;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	ARoomActor* IfCorridorOwnerRoom;

	//Simulation
	UPROPERTY(BlueprintReadWrite)
	AProceduralGen* ProceduralGen;

	bool IsCorridor = false;

	UPROPERTY(EditAnywhere)
	FRotator Rotation;

	/*For some reason GetActorLocation returns 0, so I need to pass Location explicitly*/
	FVector Location;
	

	void SetEnterDoorActor(ADoorActor* DoorActor);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Components, meta=(AllowPrivateAccess="true"))
	// class UPaperTileMapComponent* TileMapComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
