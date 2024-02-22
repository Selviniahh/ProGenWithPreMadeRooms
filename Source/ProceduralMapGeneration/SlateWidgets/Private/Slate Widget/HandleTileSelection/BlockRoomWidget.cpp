// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/HandleTileSelection/BlockRoomWidget.h"
#include "SlateMaterialBrush.h"
#include "SlateOptMacros.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/ProGenWidget.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/HandleTileSelection/BlockRoomWidgetHandler.h"

#include "UObject/UObjectGlobals.h"
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SBlockRoomWidget::Construct(const FArguments& InArgs)
{
	FirstRoom = InArgs._FirstRoom;
	SecondRoom = InArgs._SecondRoom;
	PluginSetting = GetDefault<UPluginSettings>();

	if (UMaterialInterface* Material = PluginSetting->SceneCapMaterial.Get())
		SceneCapImgBrush = MakeShared<FSlateMaterialBrush>(*Material, FVector2D(100, 100)); // Initialize member variable

	FSlateFontInfo PropertyTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	PropertyTextFont.Size = 12;

	TSharedPtr<STextBlock> LocalTextBlock;

	BlockRoomWidgetHandler = NewObject<UBlockRoomWidgetHandler>(GetTransientPackage());
	BlockRoomWidgetHandler->Initialize();

	ConstructButtonAndItsTextBox(PropertyTextFont, "If box component extends are incorrect", "Open " + FirstRoom->GetName(), [this]() mutable -> FReply
	{
		BlockRoomWidgetHandler->OpenSelectedRoomBP(FirstRoom);
		return FReply::Handled();
	});

	ChildSlot
	[
		// Parent Horizontal Box
		SNew(SHorizontalBox)

		// Image Slot
		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .FillWidth(1.0)
		[
			SNew(SImage)
			.Image(SceneCapImgBrush.Get()) // Initialize member variable
		]

		// Vertical Box Slot for Buttons and TextBoxes
		+ SHorizontalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .FillWidth(1.0)
		[
			SNew(SVerticalBox) // This Vertical Box will contain all the buttons and textboxes

			// First Button and TextBox
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			  .Padding(FMargin(0, 15, 0, 0))
			[
				ConstructButtonAndItsTextBox(PropertyTextFont, "If box component extends are incorrect", "Open " + FirstRoom->GetName(), [this]() -> FReply
				{
					return OpenSelectedRoom();
				})
			]

			// Second Button and TextBox
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			  .Padding(FMargin(0, 15, 0, 0))
			[
				ConstructButtonAndItsTextBox(PropertyTextFont, "Start exclusion", "Click me", [this]() -> FReply
				{
					return BlockRoomWidgetHandler->SpawnAllTiles(FirstRoom);
				})
			]

			// Third Button and TextBox
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			  .Padding(FMargin(0, 15, 0, 0))
			[
				//Save the exclusions
				ConstructButtonAndItsTextBox(PropertyTextFont, "Save exclusions", "Click me", [this]() -> FReply
				{
					//If tile not selected, warn the user  
					if (BlockRoomWidgetHandler->SelectedActors.IsEmpty())
					{
						FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("no tile selected"));
						return FReply::Unhandled();
					}

					BlockRoomWidgetHandler->ChangeSelectedRoom(FirstRoom);

					//Reset SelectedActors. 
					BlockRoomWidgetHandler->SelectedActors.Empty();

					return FReply::Handled();
				})
			]

			// 4. slot 
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			  .Padding(FMargin(0, 15, 0, 0))
			[
				ConstructButtonAndItsTextBox(PropertyTextFont, "Test overlap with second selected room", "Click me", [this, PropertyTextFont]() -> FReply
				{
					FReply Reply = BlockRoomWidgetHandler->TestOverlapWithSecondSelectedRoom(FirstRoom, SecondRoom);
					IsReplyHandled = Reply.IsEventHandled();
					IsTestOverlapBtnClicked = true;

					return FReply::Handled();
				})
			]

			// 5. slot. Signify if there's overlap or not with the second selected room after the above test overlap with second selected room button is clicked 
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .VAlign(VAlign_Center)
			  .HAlign(HAlign_Left)
			  .Padding(FMargin(0, 15, 0, 0))
			[
				SNew(STextBlock)
			   .Justification(ETextJustify::Center)
			   	.Text_Lambda([this]()
				                {
					                return IsReplyHandled
						                       ? FText::FromString("There's no overlap. If you content with the tile blocking and exclusion, check the checkbox")
						                       : FText::FromString("There's overlap");
				                })

			   .Font(PropertyTextFont)
			   .RenderOpacity(0.5)
			   .Visibility_Lambda([this]() { return IsTestOverlapBtnClicked ? EVisibility::Visible : EVisibility::Hidden; })
			   .ColorAndOpacity_Lambda([this]() { return IsReplyHandled ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red); })
			]

			// 6. slot for checkbox to signify this room's overlap check is completely done. When checkbox selected, set the room's test case to RoomBlockValidated
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .VAlign(VAlign_Center)
			  .HAlign(HAlign_Left)
			  .Padding(FMargin(15, 15, 0, 0))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				  .VAlign(VAlign_Center)
				  .HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Everything Checks out"))
					.Font(PropertyTextFont)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				  .VAlign(VAlign_Center)
				  .HAlign(HAlign_Left)
				  .Padding(15)
				[
					SNew(SCheckBox)
					.IsChecked(ECheckBoxState::Unchecked) //TODO: I am unsure to allow user to click this checkbox or disable it and check it myself if the button clicked and there's no overlap
					.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckBoxState)
					{
						FString ClassName = FirstRoom->GetClass()->GetName(); //It has to be L"Room23" or something like that

						if (CheckBoxState == ECheckBoxState::Checked)
						{
							PluginSetting->AllTestCases[ClassName] = RoomBlockValidated;
						}
						else
						{
							PluginSetting->AllTestCases[ClassName] = Undefined;
						}

						CheckBoxState == ECheckBoxState::Checked ? PluginSetting->AllTestCases[ClassName] = RoomBlockValidated :
						PluginSetting->AllTestCases[ClassName] = Undefined;
						PluginSetting->SaveData(FString("AllTestCases"),PluginSetting->AllTestCases);

					})
				]
			]
		]
	];
}

void SBlockRoomWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SProGenWidget::HandleRenderViewMovement(InDeltaTime, SceneCapInst, SceneCapComp, CurrentVelocity);
	BlockRoomWidgetHandler->HandleTileSelection(FirstRoom);

	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}


FReply SBlockRoomWidget::OpenSelectedRoom()
{
	if (!FirstRoom) return FReply::Unhandled();

	BlockRoomWidgetHandler->OpenSelectedRoomBP(FirstRoom);

	return FReply::Handled();
}


//Now Handle the function binding
TSharedRef<SVerticalBox> SBlockRoomWidget::ConstructButtonAndItsTextBox(const FSlateFontInfo& PropertyTextFont, const FString& TextBoxText, const FString& ButtonText, const TFunction<FReply()>& ButtonClick)
{
	TSharedPtr<STextBlock> LocalTextBlock;

	TSharedRef<SVerticalBox> Slot = SNew(SVerticalBox)

		//2. slot textbox and button
		+ SVerticalBox::Slot()
		  .HAlign(HAlign_Fill)
		  .VAlign(VAlign_Fill)
		  .AutoHeight()
		  .Padding(FMargin(10, 0, 0, 0))
		[
			SNew(SHorizontalBox)

			//1. test button textbox
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			  .HAlign(HAlign_Center)
			[
				ConstructTextBlock(PropertyTextFont, FText::FromString(TextBoxText), LocalTextBlock) //Let's see how it will be null init
			] //"If box component extends are incorrect"

			//Make test button
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			  .HAlign(HAlign_Center)
			  .Padding(FMargin(10, 0, 0, 0))
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateLambda(ButtonClick))
				[
					ConstructTextBlock(PropertyTextFont, FText::FromString(ButtonText), TextBlock)
				]
			]
		];

	return Slot;
}

SBlockRoomWidget::~SBlockRoomWidget()
{
	for (auto& SpawnedTile : BlockRoomWidgetHandler->SpawnedTiles)
	{
		if (SpawnedTile->IsValidLowLevelFast())
			SpawnedTile->Destroy();
	}
	SecondRoom->Destroy();
}

TSharedRef<STextBlock> SBlockRoomWidget::ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, TSharedPtr<STextBlock>& MemberTextBlock, const FSlateColor* Color)
{
	MemberTextBlock = SNew(STextBlock)
	   .Justification(ETextJustify::Center)
	   .Text(Text)
	   .Font(FontInfo)
	   .RenderOpacity(0.5);

	if (Color)
	{
		MemberTextBlock.Get()->SetColorAndOpacity(*Color);
	}
	return MemberTextBlock.ToSharedRef();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
