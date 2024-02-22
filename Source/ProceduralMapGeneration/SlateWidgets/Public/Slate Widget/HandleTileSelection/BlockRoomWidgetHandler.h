// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BlockRoomWidgetHandler.generated.h"

class UProGenWidgetTests;
class AProceduralGen;
class ARoomActor;
class UPluginSettings;
/**
 * 
 */
UCLASS()
class PROCEDURALMAPGENERATION_API UBlockRoomWidgetHandler : public UObject
{
	GENERATED_BODY()

	

public:
	void Initialize();
	void OpenSelectedRoomBP(const ARoomActor* FirstRoom);
	void ChangeSelectedRoom(ARoomActor* FirstRoom);
	FReply SpawnAllTiles(ARoomActor* FirstRoom);
	bool HandleTileSelection(ARoomActor* FirstRoom);
	FReply TestOverlapWithSecondSelectedRoom(ARoomActor* FirstRoom, ARoomActor* SecondRoom);
	virtual ~UBlockRoomWidgetHandler() override;

	//Variables
	UPROPERTY()
	AActor* SceneCapInst;
	UPROPERTY()
	UMaterialInterface* TileHoverMat;
	UPROPERTY()
	UMaterialInterface* TileUnhoverMat;
	UPROPERTY()
	UMaterialInterface* TileSelectMat;
	UPROPERTY()
	AActor* LastHitTile;

	UPROPERTY()
	TArray<AActor*> SpawnedTiles;

	UPROPERTY()
	const UPluginSettings* PluginSetting;
	TSharedPtr<FSlateBrush> SceneCapImgBrush;
	UPROPERTY()
	UBlockRoomWidgetHandler* BlockRoomWidgetHandler;

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapComp;

	UPROPERTY()
	TArray<AActor*> SelectedActors;

	UPROPERTY()
	AProceduralGen* ProGenInst;

	TArray<FIntPoint> ExclusionTiles;

	UPROPERTY()
	UProGenWidgetTests* ProGenWidgetTest;

};

class ARoomActor;