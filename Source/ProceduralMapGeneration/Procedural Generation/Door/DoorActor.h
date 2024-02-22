// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

class UBoxComponent;

UCLASS()
class PROCEDURALMAPGENERATION_API ADoorActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADoorActor();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USceneComponent* RootComp;
	
	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// UPaperFlipbookComponent* DoorFBRight;
	//
	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// UPaperFlipbookComponent* DoorFBLeft;	

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UBoxComponent* BoxComponent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoorEndOverlapDelegate);
	UPROPERTY(BlueprintAssignable)
	FDoorEndOverlapDelegate OnDoorEndOverlap;

protected:
	UFUNCTION()
	void OnBoxComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
