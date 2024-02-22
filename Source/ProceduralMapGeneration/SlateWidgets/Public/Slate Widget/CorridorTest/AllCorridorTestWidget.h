// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMapGeneration/Procedural Generation/RoomActor.h"
#include "Widgets/SCompoundWidget.h"

class UCorridorTestHandler;
class UPluginSettings;
class ARoomActor;
/**
 * 
 */
class PROCEDURALMAPGENERATION_API SAllCorridorTestWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAllCorridorTestWidget) {}
	SLATE_ARGUMENT(ARoomActor*, FirstRoom);
	SLATE_ARGUMENT(ARoomActor*, SecondRoom);
	SLATE_END_ARGS()
	
	//Variables
	ARoomActor* FirstRoom;
	ARoomActor* SecondRoom;
	const UPluginSettings* PluginSetting;
	TSharedPtr<FSlateBrush> SceneCapImgBrush;
	UWorld* World;
	UCorridorTestHandler* CorrTestHandler;
	
	FVector CurrentInputVelocity;
	AActor* SceneCapInst;
	USceneCaptureComponent2D* SceneCapComp;

	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	FReply Click();
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual ~SAllCorridorTestWidget() override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	

	TSharedRef<SVerticalBox> ConstructButtonAndItsTextBox(const FSlateFontInfo& PropertyTextFont, const FString& TextBoxText, const FString& ButtonText, const TFunction<FReply()>& ButtonClick);
	TSharedRef<SWidget> ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, const FSlateColor* Color = nullptr);
	TSharedRef<SButton> ConstructButton(const FText& Text, const TFunction<FReply()>& ButtonClick);
	TSharedRef<SHorizontalBox> HorizontalField(std::initializer_list<TSharedRef<SWidget>> Content);
	TSharedRef<SCheckBox> ConstructCheckBox();



};
