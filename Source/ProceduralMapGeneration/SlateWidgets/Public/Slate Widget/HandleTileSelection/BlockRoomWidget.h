// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UBlockRoomWidgetHandler;
class UPluginSettings;
class ARoomActor;
/**
 * 
 */
class PROCEDURALMAPGENERATION_API SBlockRoomWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBlockRoomWidget){}
	SLATE_ARGUMENT(ARoomActor*, FirstRoom);
	SLATE_ARGUMENT(ARoomActor*, SecondRoom);
	SLATE_END_ARGS()

	virtual ~SBlockRoomWidget() override;

	ARoomActor* FirstRoom;
	ARoomActor* SecondRoom;
	
	AActor* SceneCapInst;
	FVector CurrentVelocity;
	TSharedPtr<FSlateBrush> SceneCapImgBrush;
	const UPluginSettings* PluginSetting;
	USceneCaptureComponent2D* SceneCapComp;
	TSharedPtr<STextBlock> TextBlock,TextBlock2;

	bool IsReplyHandled;
	bool IsTestOverlapBtnClicked;

	UBlockRoomWidgetHandler* BlockRoomWidgetHandler;
	
	UMaterialInterface* TileHoverMat;
	UMaterialInterface* TileUnhoverMat;
	UMaterialInterface* TileSelectMat;
	
	AActor* LastHitTile;



	
	TSharedRef<STextBlock> ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, TSharedPtr<STextBlock>& MemberTextBlock, const FSlateColor* Color = nullptr);
	FReply OpenSelectedRoom();
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	TSharedRef<SVerticalBox> ConstructButtonAndItsTextBox(const FSlateFontInfo& PropertyTextFont, const FString& TextBoxText, const FString& ButtonText, const TFunction<FReply()>& ButtonClick);
};

 
