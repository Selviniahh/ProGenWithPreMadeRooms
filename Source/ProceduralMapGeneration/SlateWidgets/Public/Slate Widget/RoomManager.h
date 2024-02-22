// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SAllCorridorTestWidget;
class SBlockRoomWidget;
class UPluginSettings;
class UProGenWidgetTests;
class UMakeAllCorridorScenarioTest;
class ARoomActor;
/**
 * 
 */
class PROCEDURALMAPGENERATION_API SRoomManager : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRoomManager){}
	SLATE_ARGUMENT(ARoomActor*, RoomFirst)
	SLATE_ARGUMENT(ARoomActor*, RoomSecond)
	SLATE_END_ARGS()
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual ~SRoomManager() override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	bool IsFirstRoomSpawned;
	ARoomActor* FirstRoom;
	ARoomActor* SecondRoom;
	AActor* SceneCapInst;

	//TODO: Later on I am planning to instead expose this in plugin settings as "initial Movement Speed" and then retrieve it here
	FVector CurrentVelocity; // Member variable to store current velocity
	
	TSharedPtr<STextBlock> TextBlock, TextBlock2;
	TArray<TSharedPtr<STextBlock>> TextBlocks;

	TSharedRef<STextBlock> ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, TSharedPtr<STextBlock>& MemberTextBlock);
	
	TSharedPtr<FSlateBrush> SceneCapImgBrush;

	/*The class that contains all test scenarios. The widget will always call it's functions to handle test cases */
	UProGenWidgetTests* AllRoomTests;
	
	/*When this widget closed by the user (When destructor called) destroy all the rooms*/
	TArray<ARoomActor*> AllSpawnedRooms;
	
	const UPluginSettings* PluginSetting;
	USceneCaptureComponent2D* SceneCapComp;
	TSharedPtr<FButtonStyle> ButtonStyle;
	// TSharedPtr<SBlockRoomWidget> BlockRoomWidget;
	// TSharedPtr<SAllCorridorTestWidget> AllCorridorTestWidget;
	
#pragma region VisualizeBlocked
	void OnVisBlockedHovered(TSharedPtr<STextBlock>* MemberTextBlock);
	void OnVisBlockedUnHovered(TSharedPtr<STextBlock>* MemberTextBlock);
	FReply OnButtonVisBlockedClicked();
	FReply OnButtonOpenTileSelectorWidget();
#pragma endregion
	
};
