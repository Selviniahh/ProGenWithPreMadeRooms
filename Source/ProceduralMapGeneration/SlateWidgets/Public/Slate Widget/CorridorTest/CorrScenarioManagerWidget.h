// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "Widgets/SCompoundWidget.h"

class UCorrScenarioManagerHandler;
class UPluginSettings;
/**
 * 
 */
class PROCEDURALMAPGENERATION_API SCorrScenarioManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCorrScenarioManagerWidget) {}
	SLATE_ARGUMENT(ARoomActor*, FirstRoom);
	SLATE_ARGUMENT(ARoomActor*, SecondRoom);
	SLATE_ARGUMENT(TArray<ARoomActor*>, SpawnedRooms);
	SLATE_END_ARGS()


	//Variables
	ARoomActor* FirstRoom;
	ARoomActor* SecondRoom;
	const UPluginSettings* PlugSetting;
	TSharedPtr<FSlateBrush> SceneCapImgBrush;
	FVector CurrentInputVelocity;
	USceneCaptureComponent2D* SceneCapComp;
	TArray<ARoomActor*> SpawnedRooms;
	TSharedPtr<FSlateBrush> FirstImage;
	TSharedPtr<FSlateBrush> SecImage;

	//For tile selection
	FVector RayCastEndLoc;
	
	//Declare 3x3 grid of tiles
	TArray<FIntPoint> SpawnTileOffsets = {FIntPoint(0,0), FIntPoint(0,1), FIntPoint(-1,1), FIntPoint(-1,0),
		FIntPoint(-1,-1), FIntPoint(-0,-1), FIntPoint(1,-1),FIntPoint(1,0), FIntPoint(1,-1)}; 

	virtual ~SCorrScenarioManagerWidget() override;
	TSharedRef<SWidget> ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, const FSlateColor* Color = nullptr, const FText& ToolTipText = FText::FromString(""));
	TSharedRef<SButton> ConstructButton(const FText& Text, const TFunction<FReply()>& ButtonClick);
	FReply OnBorderMouseMove(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	TSharedRef<SWidget> ConstructImage(TEnumAsByte<Direction> Direction, TSharedPtr<FSlateBrush>& Image, const FText& Text);
	TSharedRef<SHorizontalBox> HorizontalField(std::initializer_list<TSharedRef<SWidget>> Content);
	
	UCorrScenarioManagerHandler* CorrScenarioHandler;

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};




