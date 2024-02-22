// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/RoomManager.h"
#include "SlateMaterialBrush.h"
#include "SlateOptMacros.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/PluginSettings.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/ProGenWidget.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/CorridorTest/AllCorridorTestWidget.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/HandleTileSelection/BlockRoomWidget.h"
#include "ProceduralMapGeneration/SlateWidgets/Public/Slate Widget/Test/ProGenWidgetTests.h"

class SBlockRoomWidget;
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SRoomManager::Construct(const FArguments& InArgs)
{
	FirstRoom = InArgs._RoomFirst;
	SecondRoom = InArgs._RoomSecond;
	PluginSetting = GetDefault<UPluginSettings>();
	SceneCapInst = PluginSetting->SceneCapActorInst.Get();

	AllRoomTests = NewObject<UProGenWidgetTests>(GetTransientPackage(), UProGenWidgetTests::StaticClass());
	AllRoomTests->Initialize(FirstRoom, SecondRoom);

	if (UMaterialInterface* Material = PluginSetting->SceneCapMaterial.Get())
	{
		SceneCapImgBrush = MakeShared<FSlateMaterialBrush>(*Material, FVector2D(100, 100)); // Initialize member variable
	}

	FSlateFontInfo PropertyTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	PropertyTextFont.Size = 12;

	ButtonStyle = MakeShared<FButtonStyle>();
	ButtonStyle.Get()->SetNormal(FSlateColorBrush(FColor(0, 0, 0, 0))); //Make everything 0 later on
	ButtonStyle.Get()->SetHovered(FSlateColorBrush(FColor(0, 0, 0, 0))); //Make everything 0 later on
	ButtonStyle.Get()->SetPressed(FSlateColorBrush(FColor(0, 0, 0, 0))); //Make everything 0 later on

	//Access Scene capture component. THe reason not creating actor from C++, Scene capture component details are not exposed in BP editor
	TArray<USceneCaptureComponent2D*> SceneCapCompArray;
	SceneCapInst->GetComponents(SceneCapCompArray);
	SceneCapComp = SceneCapCompArray[0];
	TSharedPtr<STextBlock> LocalTextBlock;

	ChildSlot
	[
		//Parent
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		[
			SNew(SHorizontalBox)

			//1. Slot
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			  .FillWidth(1.0)
			[
				SNew(SImage)
				.Image(SceneCapImgBrush.Get()) // Initialize member variable)
			]

			//NOTE: First slot for horizontally 
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)

				//2. slot Spawn room and open block selection widget
				+ SVerticalBox::Slot()
				  .HAlign(HAlign_Fill)
				  .VAlign(VAlign_Fill)
				  .AutoHeight()
				  .Padding(FMargin(10, 20, 0, 0))
				[
					SNew(SHorizontalBox)

					//1. Slot: test button textbox
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					[
						ConstructTextBlock(PropertyTextFont, FText::FromString("Make Block Check"), LocalTextBlock) //Let's see how it will be null init
					]

					//2. Slot: Spawn Selected Room button
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					  .Padding(FMargin(10, 0, 0, 0))
					[
						SNew(SButton)
						.OnClicked(this, &SRoomManager::OnButtonVisBlockedClicked)
						.OnHovered(this, &SRoomManager::OnVisBlockedHovered, &TextBlock)
						.OnUnhovered(this, &SRoomManager::OnVisBlockedUnHovered, &TextBlock)
						[
							ConstructTextBlock(PropertyTextFont, FText::FromString("Spawn Selected Room"), TextBlock)
						]
					]

					//3. Slot: Open Block selection widget room
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					  .Padding(FMargin(10, 0, 0, 0))
					[
						SNew(SButton)
						.OnClicked(this, &SRoomManager::OnButtonOpenTileSelectorWidget)
						.OnHovered(this, &SRoomManager::OnVisBlockedHovered, &TextBlock2)
						.OnUnhovered(this, &SRoomManager::OnVisBlockedUnHovered, &TextBlock2)
						[
							ConstructTextBlock(PropertyTextFont, FText::FromString("Start Test"), TextBlock2)
						]
					]

					// 4. Slot: CheckBox to signify if block selection is correctly configured
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					  .Padding(FMargin(10, 0, 0, 0))
					[
						SNew(SCheckBox)
					 	.IsEnabled(false)
					 	.IsChecked_Lambda([this]() -> ECheckBoxState
						               {
							               //TODO: Refactor here later on
							               PluginSetting->LoadData(FString("AllTestCases"), PluginSetting->AllTestCases);
							               if (PluginSetting->AllTestCases.Contains(FirstRoom->GetClass()->GetName()))
							               {
								               if (PluginSetting->AllTestCases[FirstRoom->GetClass()->GetName()] == RoomBlockValidated)
								               {
									               PluginSetting->SaveData(FString("AllTestCases"), PluginSetting->AllTestCases);
									               return ECheckBoxState::Checked;
								               }
								               PluginSetting->SaveData(FString("AllTestCases"), PluginSetting->AllTestCases);
								               return ECheckBoxState::Unchecked;
							               }
							               PluginSetting->SaveData(FString("AllTestCases"), PluginSetting->AllTestCases);
							               return ECheckBoxState::Unchecked;
						               })
					]
				]

				//NOTE: Second Slot Horizontally 
				+ SVerticalBox::Slot()
				  .HAlign(HAlign_Fill)
				  .VAlign(VAlign_Fill)
				  .AutoHeight()
				  .Padding(FMargin(10, 20, 0, 0))
				[
					SNew(SHorizontalBox)

					//1. Slot: test button textbox
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					[
						ConstructTextBlock(PropertyTextFont, FText::FromString("Make Corridor test"), LocalTextBlock) //Let's see how it will be null init
					]

					//2. Slot: Spawn the corridor scenario test window 
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					  .Padding(FMargin(10, 0, 0, 0))
					[
						SNew(SButton)
						.OnHovered(this, &SRoomManager::OnVisBlockedHovered, &TextBlock)
						.OnUnhovered(this, &SRoomManager::OnVisBlockedUnHovered, &TextBlock)
						.OnClicked(FOnClicked::CreateLambda([this]() -> FReply
				             {
					             //TODO: Destroy any spawned room first.
					             for (auto SpawnedRoom : AllSpawnedRooms)
					             {
						             SpawnedRoom->Destroy();
					             }
					
					             //Init the widget window first
					             TSharedRef<SAllCorridorTestWidget> AllCorridorTestWidget = SNew(SAllCorridorTestWidget)
								.FirstRoom(FirstRoom).SecondRoom(SecondRoom);
					             //Fill the widget window with content
					             TSharedRef<SWindow> CorridorTestWidget = SNew(SWindow)
								.Title(FText::FromString("All Corridor scenarios Test"))
								.ClientSize(FVector2D(1920, 1080))
								.SupportsMaximize(true)
								.SupportsMinimize(true)
					             [
						             AllCorridorTestWidget
					             ];
							
					             FSlateApplication::Get().AddWindow(CorridorTestWidget);
							
					             return FReply::Handled();
				             }))
						[
							ConstructTextBlock(PropertyTextFont, FText::FromString("Make corridor test with selected two rooms"), TextBlock)
						]
					]
					
					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .HAlign(HAlign_Center)
					  .Padding(FMargin(10, 0, 0, 0))
					[
						//TODO: Handle here much more later on for scenario test 
						SNew(SCheckBox)
					 	.IsEnabled(false)
					 	.IsChecked_Lambda([this]() -> ECheckBoxState
			               {
				               PluginSetting->LoadData(FString("AllTestCases"), PluginSetting->AllTestCases);
				               if (PluginSetting->AllTestCases.Contains(FirstRoom->GetClass()->GetName()))
				               {
					               if (PluginSetting->AllTestCases[FirstRoom->GetClass()->GetName()] == RoomBlockValidated)
					               {
						               PluginSetting->SaveData(FString("AllTestCases"), PluginSetting->AllTestCases);
						               return ECheckBoxState::Checked;
					               }
					               PluginSetting->SaveData(FString("AllTestCases"), PluginSetting->AllTestCases);
					               return ECheckBoxState::Unchecked;
				               }
				               PluginSetting->SaveData(FString("AllTestCases"), PluginSetting->AllTestCases);
				               return ECheckBoxState::Unchecked;
			               })
					]
				]
			]
		]
	];
}

void SRoomManager::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SProGenWidget::HandleRenderViewMovement(InDeltaTime, SceneCapInst, SceneCapComp, CurrentVelocity);
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

TSharedRef<STextBlock> SRoomManager::ConstructTextBlock(FSlateFontInfo FontInfo, FText Text, TSharedPtr<STextBlock>& MemberTextBlock)
{
	MemberTextBlock = SNew(STextBlock)
	   .Justification(ETextJustify::Center)
	   .Text(Text)
	   .Font(FontInfo)
	   .RenderOpacity(0.5);

	return MemberTextBlock.ToSharedRef();
}

FReply SRoomManager::OnButtonVisBlockedClicked()
{
	if (!IsFirstRoomSpawned)
	{
		ARoomActor* SpawnedFirstRoom = AllRoomTests->MakeOverlapTest();
		AllSpawnedRooms.Add(SpawnedFirstRoom);
		IsFirstRoomSpawned = true;
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("First selected room already spawned"));
	}

	return FReply::Handled();
}

FReply SRoomManager::OnButtonOpenTileSelectorWidget()
{
	//Return early if the first room is not spawned yet
	if (!IsFirstRoomSpawned)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("First selected room is not spawned yet What are you trying to do? "));
		return FReply::Unhandled();
	}
	//Init the widget
	TSharedRef<SBlockRoomWidget> BlockRoomWidget = SNew(SBlockRoomWidget)
	.FirstRoom(AllSpawnedRooms[0])
	.SecondRoom(AllRoomTests->SecondRoom);

	//Create the widget as SWindow
	TSharedRef<SWindow> RoomManagerWindow = SNew(SWindow)
	.Title(FText::FromString(TEXT("Block tile selection for ") + FirstRoom->GetName()))
	.ClientSize(FVector2D(1920, 1080)) //TODO: Later on for all window sizes, make what engine's window was already has
	.SupportsMaximize(true)
	.SupportsMinimize(true)
	[ //It's content
		BlockRoomWidget
	];

	FSlateApplication::Get().AddWindow(RoomManagerWindow);

	return FReply::Handled();
}

SRoomManager::~SRoomManager()
{
	FlushPersistentDebugLines(GEditor->GetEditorWorldContext().World());

	//Destroy all spawned rooms when the room manager widget closed by the user
	for (auto SpawnedRoom : AllSpawnedRooms)
	{
		SpawnedRoom->Destroy();
	}
}

#pragma region VisualizeBlocked
void SRoomManager::OnVisBlockedHovered(TSharedPtr<STextBlock>* MemberTextBlock)
{
	MemberTextBlock->Get()->SetRenderOpacity(1);
}

void SRoomManager::OnVisBlockedUnHovered(TSharedPtr<STextBlock>* MemberTextBlock)
{
	MemberTextBlock->Get()->SetRenderOpacity(0.5);
}
#pragma endregion


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
