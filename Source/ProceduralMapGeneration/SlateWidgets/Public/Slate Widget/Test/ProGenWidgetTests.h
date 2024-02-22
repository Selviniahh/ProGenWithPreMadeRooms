// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "UObject/Object.h"
#include "ProGenWidgetTests.generated.h"

class UPluginSettings;
class ARoomActor;
/**
 * 
 */
UCLASS()
class PROCEDURALMAPGENERATION_API UProGenWidgetTests : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ARoomActor* FirstRoom;

	UPROPERTY()
	ARoomActor* SecondRoom;

	UPROPERTY()
	AProceduralGen* ProGen;

	UPROPERTY()
	const UPluginSettings* PluginSettings;
	
	void Initialize(ARoomActor* First, ARoomActor* Second);
#pragma region OverlapTest
	ARoomActor* MakeOverlapTest();
	//Not implemented yet
	FVector DetermineSecondRoomSpawnLocation();
#pragma endregion 
};
